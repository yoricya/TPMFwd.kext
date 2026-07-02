//
//  TPMUserClient.cpp
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#include "TPMUserClient.h"
#include "TPMFwd.h"

#define super IOUserClient
OSDefineMetaClassAndStructors(TPMUserClient, IOUserClient);

bool TPMUserClient::start(IOService *provider) {
    if (!super::start(provider)) return false;
    fDriver = OSDynamicCast(TPMFwd, provider);
    return (fDriver != nullptr);
}

void TPMUserClient::stop(IOService *provider) {
    super::stop(provider);
}

void TPMUserClient::setDriver(TPMFwd *driver) {
    fDriver = driver;
}

IOReturn TPMUserClient::clientClose() {
    if (!isInactive()) {
        terminate();
    }
    
    return kIOReturnSuccess;
}

IOReturn TPMUserClient::clientDied() {
    return clientClose();
}

IOReturn TPMUserClient::externalMethod(uint32_t selector,
                                       IOExternalMethodArguments *args,
                                       IOExternalMethodDispatch *dispatch,
                                       OSObject *target,
                                       void *reference) {
    if (!args) {
        return kIOReturnBadArgument;
    }

    switch (selector) {
        // Send cmd
        case 0: {
            
            // Check
            if (!fDriver || !fDriver->hardware) {
                IOLog("TPMUserClient: Driver or hardware is not ready!\n");
                return kIOReturnNotReady;
            }
            
            if (!args->structureInput || args->structureInputSize < 10) {
                if (TPMFwd::TPM_DEBUG) {
                    IOLog("TPMUserClient: Invalid input (min 10 bytes)\n");
                }
                return kIOReturnBadArgument;
            }
                        
            if (!args->structureOutput || args->structureOutputSize == 0) {
                if (TPMFwd::TPM_DEBUG) {
                    IOLog("TPMUserClient: No output buffer\n");
                }
                return kIOReturnBadArgument;
            }
            
            // Prepare cmd input buffer
            const uint8_t *cmd = (const uint8_t *)args->structureInput;
            size_t cmdSize = args->structureInputSize;
            
            // Prepare response buffer
            uint8_t *respBuffer = (uint8_t *)args->structureOutput;
            size_t respSize = args->structureOutputSize;
            
            // Send cmd
            if (TPMFwd::TPM_DEBUG) {
                IOLog("TPMUserClient: Sending %zu bytes to TPM\n", cmdSize);
            }
            
            uint32_t returnCode = 0;
            bool succesed = fDriver->hardware->sendCommand(cmd, cmdSize, respBuffer, &respSize, &returnCode);
            
            if (TPMFwd::TPM_DEBUG) {
                IOLog("TPMUserClient: TPM returned %zu bytes, returnCode=0x%08x\n", respSize, returnCode);
            }

            args->structureOutputSize = (uint32_t) respSize;
            
            // Return response
            if (args->scalarOutput && args->scalarOutputCount >= 1) {
                args->scalarOutput[0] = (uint64_t) returnCode;
            }
            
            return succesed ? kIOReturnSuccess : kIOReturnError;
        }
        default:
            return kIOReturnUnsupported;
    }
}
