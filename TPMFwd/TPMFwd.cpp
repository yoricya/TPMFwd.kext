//
//  TPMFwd.h
//  TPMFwd
//
//  Created by Yoricya on 01.07.2026.
//

#include "TPMFwd.h"
#include <IOKit/IOLib.h>
#include "TPMUserClient.h"

#define super IOService
OSDefineMetaClassAndStructors(TPMFwd, IOService);

bool TPMFwd::start(IOService *provider) {
    IOLog("TPMFwd: start() called\n");
    
    if (!super::start(provider)) {
        IOLog("TPMFwd: super::start() failed\n");
        return false;
    }
    
    acpiDevice = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!acpiDevice) {
        IOLog("TPMFwd: provider is not IOACPIPlatformDevice\n");
        return false;
    }
    
    IOLog("TPMFwd: Found TPM device\n");
    
    if (!mapTPMMemory()) {
        IOLog("TPMFwd: Failed to map TPM memory\n");
        return false;
    }
    
    // Инициализируем hardware abstraction
    hardware = new TPMHardware();
    if (!hardware->initialize(tpmBase)) {
        IOLog("TPMFwd: Failed to initialize hardware\n");
        delete hardware;
        return false;
    }
    
    // Инициализируем commands
    commands = new TPMCommands(hardware);
    
    initialized = false;
    
    uint8_t access = hardware->readAccessRegister();
    IOLog("TPMFwd: Initial TPM_ACCESS = 0x%02x\n", access);
    
    if (!hardware->requestLocality(0)) {
        IOLog("TPMFwd: Failed to request locality 0\n");
        delete commands;
        delete hardware;
        return false;
    }
    
    IOLog("TPMFwd: Locality 0 acquired\n");
    
    // TPM Startup
    if (!commands->startup()) {
        IOLog("TPMFwd: TPM Startup returned error\n");
        delete commands;
        delete hardware;
        return false;
    } else {
        IOLog("TPMFwd: TPM Startup successful\n");
    }
    
    // Self-test
    if (!commands->selfTest(false)) {
        IOLog("TPMFwd: TPM Self-test failed\n");
        delete commands;
        delete hardware;
        return false;
    } else {
        IOLog("TPMFwd: TPM Self-test successful\n");
    }
    
    initialized = true;
    registerService();
    IOLog("TPMFwd: TPM driver loaded successfully!\n");

    return true;
}

void TPMFwd::stop(IOService *provider) {
    IOLog("TPMFwd: stop() called\n");
    
    if (initialized && hardware) {
        hardware->releaseLocality(0);
    }
    
    if (commands) {
        delete commands;
        commands = nullptr;
    }
    
    if (hardware) {
        delete hardware;
        hardware = nullptr;
    }
    
    if (memoryMap) {
        memoryMap->release();
        memoryMap = nullptr;
    }
    
    super::stop(provider);
}

bool TPMFwd::mapTPMMemory() {
    IOMemoryDescriptor *memoryDescriptor =
        IOMemoryDescriptor::withPhysicalAddress(0xFED40000, 0x5000, kIODirectionInOut);
    
    if (!memoryDescriptor) {
        IOLog("TPMFwd: Failed to create memory descriptor\n");
        return false;
    }
    
    memoryMap = memoryDescriptor->map();
    memoryDescriptor->release();
    
    if (!memoryMap) {
        IOLog("TPMFwd: Failed to map memory\n");
        return false;
    }
    
    tpmBase = (volatile uint8_t *)memoryMap->getVirtualAddress();
    IOLog("TPMFwd: TPM mapped at virtual address 0x%lx\n", (uintptr_t)tpmBase);
    
    return true;
}

IOReturn TPMFwd::newUserClient(task_t task, void *securityID, UInt32 type, IOUserClient **handler) {
    if (TPMFwd::TPM_DEBUG) {
        IOLog("TPMFwd: newUserClient called\n");
    }
    
    if (!handler) {
        return kIOReturnBadArgument;
    }
    *handler = nullptr;
    
    TPMUserClient *client = new TPMUserClient();
    if (!client) {
        return kIOReturnNoMemory;
    }

    if (!client->initWithTask(task, securityID, type)) {
        client->release();
        return kIOReturnError;
    }
    
    client->setDriver(this);
    
    if (!client->attach(this)) {
        client->release();
        return kIOReturnError;
    }
    
    if (!client->start(this)) {
        client->detach(this);
        client->release();
        return kIOReturnError;
    }
    
    *handler = client;
    
    return kIOReturnSuccess;
}

IOReturn TPMFwd::newUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOUserClient **handler) {
    return newUserClient(task, securityID, type, handler);
}
