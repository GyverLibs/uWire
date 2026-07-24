#include "uWire.h"

void uWireClass::begin() {
    TWCR = 0;
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    _requested = 0;
    _error = ERROR_NONE;
    _active = false;
    setClock(100000ul);
    TWCR = _BV(TWEN);
}

void uWireClass::end() {
    if (_active) _stop();
    TWCR = 0;
    pinMode(SDA, INPUT);
    pinMode(SCL, INPUT);
    _requested = 0;
    _error = ERROR_NONE;
    _active = false;
}

void uWireClass::setClock(uint32_t clock) {
    if (!clock) return;
    TWSR = 0;  // prescaler /1
    uint32_t divider = F_CPU / clock;
    divider = (divider > 16) ? (divider - 16) / 2 : 0;
    TWBR = (divider > 255) ? 255 : (uint8_t)divider;
}

void uWireClass::beginTransmission(uint8_t address) {
    _requested = 0;
    _error = ERROR_NONE;
    if (_start()) write(address << 1);
}

uint8_t uWireClass::endTransmission(bool sendStop) {
    if ((sendStop || _error != ERROR_NONE) && _active) _stop();
    Error error = _error;
    _error = ERROR_NONE;
    return error;
}

size_t uWireClass::write(uint8_t data) {
    if (_error != ERROR_NONE || !_active) return 0;
    TWDR = data;
    TWCR = _BV(TWEN) | _BV(TWINT);
    if (!_wait(_BV(TWINT), true)) return 0;

    switch (TWSR & 0xF8) {
        case 0x18:  // SLA+W ACK
        case 0x28:  // DATA ACK
        case 0x40:  // SLA+R ACK
            return 1;
        case 0x20:  // SLA+W NACK
        case 0x48:  // SLA+R NACK
            _error = ERROR_ADDRESS_NACK;
            break;
        case 0x30:  // DATA NACK
            _error = ERROR_DATA_NACK;
            break;
        case 0x38:  // arbitration lost
            _active = false;
            _error = ERROR_OTHER;
            break;
        default:
            _error = ERROR_OTHER;
            break;
    }
    return 0;
}

int uWireClass::read() {
    if (!_requested || _error != ERROR_NONE || !_active) return -1;

    bool last = !--_requested;
    TWCR = _BV(TWEN) | _BV(TWINT) | (last ? 0 : _BV(TWEA));
    if (!_wait(_BV(TWINT), true)) return -1;

    uint8_t status = TWSR & 0xF8;
    if (status != (last ? 0x58 : 0x50)) {  // DATA+NACK / DATA+ACK
        _requested = 0;
        _error = ERROR_OTHER;
        if (_active) _stop();
        return -1;
    }

    int data = TWDR;
    if (last && _need_stop) _stop();
    return data;
}

size_t uWireClass::read(uint8_t* data, size_t len) {
    if (!data && len) return 0;

    size_t res = 0;
    while (len-- && available()) {
        int value = read();
        if (value < 0) break;
        *data++ = value;
        res++;
    }
    return res;
}

uint8_t uWireClass::requestFrom(uint8_t address, uint8_t length, bool sendStop) {
    _need_stop = sendStop;
    _requested = 0;
    _error = ERROR_NONE;

    if (!length) {
        if (sendStop && _active) _stop();
        return 0;
    }
    if (!_start()) return 0;
    if (!write((address << 1) | 1)) {
        if (_active) _stop();
        return 0;
    }

    _requested = length;
    return length;
}

uint8_t uWireClass::requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop) {
    if (isize) {
        beginTransmission(address);

        // Wire API использует до 3 байт внутреннего адреса
        if (isize > 3) isize = 3;

        // write internal register address - most significant byte first
        while (isize-- > 0) write((uint8_t)(iaddress >> (isize * 8)));
        if (endTransmission(false)) return 0;
    }

    return requestFrom(address, quantity, sendStop);
}

size_t uWireClass::write(const uint8_t* data, size_t len) {
    size_t res = 0;
    while (len--) res += write(*data++);
    return res;
}

bool uWireClass::_wait(uint8_t mask, bool state) {
#if MICROWIRE_TIMEOUT > 0
    uint32_t started = micros();
#endif
    while (!!(TWCR & mask) != state) {
#if MICROWIRE_TIMEOUT > 0
        if ((uint32_t)(micros() - started) >= MICROWIRE_TIMEOUT) {
            _requested = 0;
            _active = false;
            _error = ERROR_OTHER;
            _reset();
            return false;
        }
#endif
    }
    return true;
}

bool uWireClass::_start() {
    TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT);
    if (!_wait(_BV(TWINT), true)) return false;

    uint8_t status = TWSR & 0xF8;
    if (status == 0x08 || status == 0x10) {
        _active = true;
        return true;
    }
    _active = false;
    _error = ERROR_OTHER;
    _reset();
    return false;
}

void uWireClass::_stop() {
    TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);
    _wait(_BV(TWSTO), false);
    _active = false;
    _requested = 0;
}

void uWireClass::_reset() {
    TWCR = 0;
    TWCR = _BV(TWEN);
}

uWireClass uWire = uWireClass();
