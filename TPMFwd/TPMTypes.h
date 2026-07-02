//
//  TPMTypes.h
//  TPMFwd
//
//  Created by Yoricya on 02.07.2026.
//

#ifndef TPMTypes_h
#define TPMTypes_h

#include <stdint.h>

// TPM TIS регистры
namespace TPMRegisters {
    constexpr uint32_t ACCESS = 0x0000;
    constexpr uint32_t INT_ENABLE = 0x0008;
    constexpr uint32_t INT_VECTOR = 0x0009;
    constexpr uint32_t INT_STATUS = 0x0010;
    constexpr uint32_t INTF_CAPS = 0x0014;
    constexpr uint32_t STS = 0x0018;
    constexpr uint32_t BURST_COUNT = 0x0019;
    constexpr uint32_t DATA_FIFO = 0x0024;
}

// ACCESS биты
namespace TPMAccess {
    constexpr uint8_t VALID = 0x80;
    constexpr uint8_t ACTIVE_LOCALITY = 0x20;
    constexpr uint8_t REQUEST_PENDING = 0x02;
    constexpr uint8_t REQUEST_USE = 0x02;
}

// STS биты
namespace TPMStatus {
    constexpr uint8_t VALID = 0x80;
    constexpr uint8_t COMMAND_READY = 0x40;
    constexpr uint8_t GO = 0x20;
    constexpr uint8_t DATA_AVAIL = 0x10;
    constexpr uint8_t DATA_EXPECT = 0x08;
    constexpr uint8_t SELF_TEST_DONE = 0x04;
    constexpr uint8_t RESPONSE_RETRY = 0x02;
    constexpr uint8_t ERR = 0x01;
}

// TPM 2.0 теги
namespace TPMTag {
    constexpr uint16_t NO_SESSIONS = 0x8001;
    constexpr uint16_t SESSIONS = 0x8002;
}

// TPM 2.0 команды
namespace TPMCommand {
    constexpr uint32_t STARTUP = 0x00000144;
    constexpr uint32_t SELF_TEST = 0x00000143;
    constexpr uint32_t SHUTDOWN = 0x00000145;
}

// TPM return коды
namespace TPMReturnCode {
    constexpr uint32_t SUCCESS = 0x000;
    constexpr uint32_t INITIALIZE = 0x100;
    constexpr uint32_t FAILURE = 0x101;
    constexpr uint32_t SIZE = 0x095;
    constexpr uint32_t POLICY_FAIL = 0x12F;
}

// TPM Startup типы
namespace TPMStartupType {
    constexpr uint16_t CLEAR = 0x0000;
    constexpr uint16_t STATE = 0x0001;
}

#endif
