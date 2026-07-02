//
//  TPMHardware.cpp
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#include "TPMHardware.h"
#include "TPMFwd.h"
#include <IOKit/IOLib.h>

TPMHardware::TPMHardware() : tpmBase(nullptr), commandLock(nullptr) {}

TPMHardware::~TPMHardware() {
    cleanup();
}

bool TPMHardware::initialize(volatile uint8_t *baseAddress) {
    commandLock = IOLockAlloc();
    if (!commandLock) {
        IOLog("TPMHardware: Failed to allocate IOLock\n");
        return false;
    }
    
    if (!baseAddress) {
        IOLog("TPMHardware: Invalid base address\n");
        return false;
    }
    tpmBase = baseAddress;
    return true;
}

void TPMHardware::cleanup() {
    if (commandLock) {
        IOLockFree(commandLock);
        commandLock = nullptr;
    }
    
    tpmBase = nullptr;
}

uint8_t TPMHardware::readRegister(uint32_t offset) {
    if (tpmBase == nullptr) {
        IOLog("TPMHardware: Attempt to read register with null tpmBase. offset: %d", offset);
        return 0;
    }
    
    return tpmBase[offset];
}

void TPMHardware::writeRegister(uint32_t offset, uint8_t value) {
    if (tpmBase == nullptr) {
        IOLog("TPMHardware: Attempt to write register with null tpmBase. offset: %d, value: %d", offset, value);
        return;
    }
    
    tpmBase[offset] = value;
}

uint16_t TPMHardware::readBurstCount() {
    uint16_t burst = readRegister(TPMRegisters::BURST_COUNT);
    burst |= (readRegister(TPMRegisters::BURST_COUNT + 1) << 8);
    return burst;
}

bool TPMHardware::requestLocality(uint8_t locality) {
    writeRegister(TPMRegisters::ACCESS, TPMAccess::REQUEST_USE);
    
    for (int i = 0; i < 1000; i++) {
        uint8_t access = readRegister(TPMRegisters::ACCESS);
        if (access & TPMAccess::ACTIVE_LOCALITY) {
            return true;
        }
        IODelay(1000);
    }
    
    IOLog("TPMHardware: Timeout waiting for locality %u\n", locality);
    return false;
}

void TPMHardware::releaseLocality(uint8_t locality) {
    writeRegister(TPMRegisters::ACCESS, TPMAccess::ACTIVE_LOCALITY);
}

void TPMHardware::abortCommand() {
    writeRegister(TPMRegisters::STS, TPMStatus::COMMAND_READY);
    IODelay(1000);
}

bool TPMHardware::waitForCommandReady() {
    abortCommand();
    
    for (int i = 0; i < 10000; i++) {
        uint8_t sts = readRegister(TPMRegisters::STS);
        if (sts & TPMStatus::COMMAND_READY) {
            return true;
        }
        IODelay(100);
    }
    
    IOLog("TPMHardware: Timeout waiting for COMMAND_READY (STS=0x%02x)\n", readRegister(TPMRegisters::STS));
    return false;
}

bool TPMHardware::waitForDataAvail() {
    for (int i = 0; i < 10000; i++) {
        uint8_t sts = readRegister(TPMRegisters::STS);
        if (sts & TPMStatus::DATA_AVAIL) {
            return true;
        }
        IODelay(100);
    }
    IOLog("TPMHardware: Timeout waiting for DATA_AVAIL\n");
    return false;
}

bool TPMHardware::sendCommand(const uint8_t *cmd, size_t cmdSize, uint8_t *resp, size_t *respSize, uint32_t *returnCode) {
    if (!cmd || cmdSize == 0 || !respSize || !returnCode || !commandLock) {
        return false;
    }

    bool success = false;
    size_t bytesWritten = 0;
    uint8_t header[10] = {};
    size_t bytesRead = 0;
    uint16_t tag = 0;
    uint32_t respLen = 0;
    uint32_t retCode = 0;
    size_t remaining = 0;
    IOLockLock(commandLock);
    
    if (!waitForCommandReady()) {
        goto done;
    }
    
    // Записываем команду в FIFO
    while (bytesWritten < cmdSize) {
        uint16_t burst = readBurstCount();
        if (burst == 0) {
            if (TPMFwd::TPM_DEBUG){
                IOLog("TPMHardware: Burst count is 0\n");
            }
            goto done;
        }
        
        size_t toWrite = (cmdSize - bytesWritten < burst) ?
                         (cmdSize - bytesWritten) : burst;
        for (size_t i = 0; i < toWrite; i++) {
            writeRegister(TPMRegisters::DATA_FIFO, cmd[bytesWritten + i]);
        }
        bytesWritten += toWrite;
        
        if (bytesWritten < cmdSize) {
            uint8_t sts = readRegister(TPMRegisters::STS);
            if (!(sts & TPMStatus::DATA_EXPECT)) {
                if (TPMFwd::TPM_DEBUG){
                    IOLog("TPMHardware: TPM not expecting more data\n");
                }
                goto done;
            }
        }
    }
    
    // Запускаем выполнение
    writeRegister(TPMRegisters::STS, TPMStatus::GO);
    
    // Ждем ответа
    if (!waitForDataAvail()) {
        goto done;
    }
    
    // Читаем заголовок ответа
    while (bytesRead < 10) {
        uint16_t burst = readBurstCount();
        if (burst == 0) {
            if (TPMFwd::TPM_DEBUG){
                IOLog("TPMHardware: Burst count is 0 while reading response\n");
            }
            goto done;
        }
        
        size_t toRead = (10 - bytesRead < burst) ? (10 - bytesRead) : burst;
        for (size_t i = 0; i < toRead; i++) {
            header[bytesRead + i] = readRegister(TPMRegisters::DATA_FIFO);
        }
        bytesRead += toRead;
    }
    
    // Парсим заголовок
    tag = (header[0] << 8) | header[1];
    respLen = (header[2] << 24) | (header[3] << 16) |
                       (header[4] << 8) | header[5];
    retCode = (header[6] << 24) | (header[7] << 16) |
                       (header[8] << 8) | header[9];
    
    *returnCode = retCode;
    
    if (TPMFwd::TPM_DEBUG){
        IOLog("TPMHardware: Response tag: 0x%04x, length: %u, return code: 0x%08x\n", tag, respLen, retCode);
    }
    
    if (respLen < 10) {
        if (TPMFwd::TPM_DEBUG){
            IOLog("TPMHardware: Invalid response length: %u\n", respLen);
        }
        goto done;
    }
    
    // Копируем заголовок
    if (resp && *respSize >= 10) {
        memcpy(resp, header, 10);
    }
    
    // Читаем остаток ответа
    remaining = respLen - 10;
    if (resp && remaining > 0 && *respSize > 10) {
        size_t toRead = (remaining < *respSize - 10) ? remaining : *respSize - 10;
        bytesRead = 0;
        
        while (bytesRead < toRead) {
            uint16_t burst = readBurstCount();
            if (burst == 0) {
                break;
            }
            
            size_t chunk = (toRead - bytesRead < burst) ?
                          (toRead - bytesRead) : burst;
            for (size_t i = 0; i < chunk; i++) {
                resp[10 + bytesRead + i] = readRegister(TPMRegisters::DATA_FIFO);
            }
            bytesRead += chunk;
        }
        
        *respSize = 10 + bytesRead;
    } else {
        *respSize = 10;
    }

    success = true;
    
done:
    // unlocking
    IOLockUnlock(commandLock);
    
    return success;
}

uint8_t TPMHardware::readAccessRegister() {
    return readRegister(TPMRegisters::ACCESS);
}
