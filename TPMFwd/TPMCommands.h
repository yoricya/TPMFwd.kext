//
//  TPMCommands.h
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#ifndef TPMCommands_h
#define TPMCommands_h

#include "TPMHardware.h"
#include "TPMTypes.h"

class TPMCommands {
private:
    TPMHardware *hardware;
    
    void writeBE16(uint8_t *buffer, uint16_t value);
    void writeBE32(uint8_t *buffer, uint32_t value);
    uint16_t readBE16(const uint8_t *buffer);
    uint32_t readBE32(const uint8_t *buffer);

public:
    TPMCommands(TPMHardware *hw);
    
    bool startup(uint16_t startupType = TPMStartupType::CLEAR);
    bool selfTest(bool fullTest = false);
};

#endif /* TPMCommands_h */
