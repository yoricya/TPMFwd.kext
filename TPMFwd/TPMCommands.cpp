//
//  TPMCommands.cpp
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#include "TPMCommands.h"
#include <IOKit/IOLib.h>

TPMCommands::TPMCommands(TPMHardware *hw) : hardware(hw) {}

void TPMCommands::writeBE16(uint8_t *buffer, uint16_t value) {
    buffer[0] = (value >> 8) & 0xFF;
    buffer[1] = value & 0xFF;
}

void TPMCommands::writeBE32(uint8_t *buffer, uint32_t value) {
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;
}

uint16_t TPMCommands::readBE16(const uint8_t *buffer) {
    return (buffer[0] << 8) | buffer[1];
}

uint32_t TPMCommands::readBE32(const uint8_t *buffer) {
    return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

bool TPMCommands::startup(uint16_t startupType) {
    uint8_t cmd[12];
    writeBE16(cmd, TPMTag::NO_SESSIONS);
    writeBE32(cmd + 2, 12);
    writeBE32(cmd + 6, TPMCommand::STARTUP);
    writeBE16(cmd + 10, startupType);
    
    uint8_t resp[256];
    size_t respSize = sizeof(resp);
    uint32_t returnCode = 0;
    
    if (!hardware->sendCommand(cmd, sizeof(cmd), resp, &respSize, &returnCode)) {
        return false;
    }
    
    if (returnCode == TPMReturnCode::INITIALIZE) {
        IOLog("TPMCommands: TPM already initialized\n");
        return true;
    }
    
    return (returnCode == TPMReturnCode::SUCCESS);
}

bool TPMCommands::selfTest(bool fullTest) {
    uint8_t cmd[11];
    writeBE16(cmd, TPMTag::NO_SESSIONS);
    writeBE32(cmd + 2, 11);
    writeBE32(cmd + 6, TPMCommand::SELF_TEST);
    cmd[10] = fullTest ? 0x01 : 0x00;
    
    uint8_t resp[256];
    size_t respSize = sizeof(resp);
    uint32_t returnCode = 0;
    
    if (!hardware->sendCommand(cmd, sizeof(cmd), resp, &respSize, &returnCode)) {
        return false;
    }
    
    return (returnCode == TPMReturnCode::SUCCESS);
}
