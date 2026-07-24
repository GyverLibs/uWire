#pragma once
#include <Arduino.h>

// Таймаут ожидания аппаратного TWI, мкс. 0 отключает таймаут.
#ifndef MICROWIRE_TIMEOUT
#define MICROWIRE_TIMEOUT 1000ul
#endif

class uWireClass {
   public:
    enum Error : uint8_t {
        ERROR_NONE = 0,
        ERROR_ADDRESS_NACK = 2,
        ERROR_DATA_NACK = 3,
        ERROR_OTHER = 4,
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
    uint8_t _requested = 0;
    Error _error = ERROR_NONE;
    bool _need_stop = true;
    bool _active = false;

    bool _wait(uint8_t mask, bool state);
    bool _start();
    void _stop();
    static void _reset();
};

extern uWireClass uWire;
