//
//  TPMKext.h
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#ifndef TPMFwd_h
#define TPMFwd_h

#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "TPMHardware.h"
#include "TPMCommands.h"

class TPMFwd : public IOService {
    OSDeclareDefaultStructors(TPMFwd);
    
private:
    IOACPIPlatformDevice *acpiDevice;
    IOMemoryMap *memoryMap;
    volatile uint8_t *tpmBase;
    bool initialized;
    bool mapTPMMemory();
    
public:
    static constexpr bool TPM_DEBUG = true;

    TPMHardware *hardware;
    TPMCommands *commands;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual IOReturn newUserClient(task_t task, void *securityID, UInt32 type, IOUserClient **handler) override;
    virtual IOReturn newUserClient(task_t task, void *securityID, UInt32 type, OSDictionary *properties, IOUserClient **handler) override;
};

#endif /* TPMFwd_h */
