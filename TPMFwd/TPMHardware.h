//
//  TPMHardware.h
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#ifndef TPMHardware_h
#define TPMHardware_h

#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>
#include "TPMTypes.h"

class TPMHardware {
private:
    volatile uint8_t *tpmBase;
    IOLock *commandLock;
    
    uint8_t readRegister(uint32_t offset);
    void writeRegister(uint32_t offset, uint8_t value);
    uint16_t readBurstCount();
    
    bool waitForCommandReady();
    bool waitForDataAvail();
    void abortCommand();

public:
    TPMHardware();
    ~TPMHardware();
    
    bool initialize(volatile uint8_t *baseAddress);
    bool requestLocality(uint8_t locality);
    void releaseLocality(uint8_t locality);
    void cleanup();
    
    bool sendCommand(const uint8_t *cmd, size_t cmdSize,
                     uint8_t *resp, size_t *respSize,
                     uint32_t *returnCode);
    
    uint8_t readAccessRegister();
};

#endif /* TPMHardware_h */
