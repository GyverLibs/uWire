#pragma once
#include <Arduino.h>
#include <util/twi.h>

#if !defined(__AVR__) || !defined(TWCR) || !defined(TWSR) || !defined(TWDR) || !defined(TWBR)
#error "uWire requires classic AVR TWI hardware"
#endif

#ifndef UWIRE_TIMEOUT
#define UWIRE_TIMEOUT 1000UL
#endif

#ifndef UWIRE_TIMEOUT_LOOP_CYCLES
#define UWIRE_TIMEOUT_LOOP_CYCLES 8UL
#endif

#if UWIRE_TIMEOUT_LOOP_CYCLES == 0
#error "UWIRE_TIMEOUT_LOOP_CYCLES must be greater than zero"
#endif

#if UWIRE_TIMEOUT > 0
#define UWIRE_TIMEOUT_LOOPS_RAW ((F_CPU / 1000UL) * UWIRE_TIMEOUT / (1000UL * UWIRE_TIMEOUT_LOOP_CYCLES))
#if UWIRE_TIMEOUT_LOOPS_RAW > 65535UL
#error "UWIRE_TIMEOUT is too large for the 16-bit polling counter"
#endif
#define UWIRE_TIMEOUT_LOOPS ((uint16_t)(UWIRE_TIMEOUT_LOOPS_RAW ? UWIRE_TIMEOUT_LOOPS_RAW : 1UL))
#endif

class uWireClass {
   public:
    enum Error : uint8_t {
        ERROR_NONE = 0,
        ERROR_ADDRESS_NACK = 2,
        ERROR_DATA_NACK = 3,
        ERROR_OTHER = 4,
        ERROR_TIMEOUT = 5,
    };

    void begin();
    void end();
    static void setClock(uint32_t clock);

    void beginTransmission(uint8_t address);
    uint8_t endTransmission(bool sendStop = true);

    size_t write(uint8_t data);
    size_t write(const uint8_t* data, size_t len);

    uint8_t requestFrom(uint8_t address, uint8_t length, bool sendStop = true);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop);

    int read();
    size_t read(uint8_t* data, size_t len);

    uint8_t available() const { return _requested; }
    Error getError() const { return _error; }

   private:
    enum Flag : uint8_t {
        FLAG_ACTIVE = 0x01,
        FLAG_STOP_AFTER_READ = 0x02,
    };

    uint8_t _requested = 0;
    Error _error = ERROR_NONE;
    uint8_t _flags = 0;

    bool _wait(uint8_t mask, bool state);
    bool _start(uint8_t address);
    void _stop();
    static void _reset();
};

extern uWireClass uWire;
