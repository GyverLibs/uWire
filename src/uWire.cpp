#include "uWire.h"

void uWireClass::begin() {
    TWCR = 0;
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    _requested = 0;
    _error = ERROR_NONE;
    _flags = 0;
    setClock(100000UL);
    TWCR = _BV(TWEN);
}

void uWireClass::end() {
    if (_flags & FLAG_ACTIVE) _stop();
    TWCR = 0;
    pinMode(SDA, INPUT);
    pinMode(SCL, INPUT);
    _requested = 0;
    _error = ERROR_NONE;
    _flags = 0;
}

void uWireClass::setClock(uint32_t clock) {
    if (!clock) return;
    TWSR = 0;
    uint32_t divider = F_CPU / clock;
    divider = divider > 16 ? (divider - 16) / 2 : 0;
    TWBR = (uint8_t)(divider > 255 ? 255 : divider);
}

void uWireClass::beginTransmission(uint8_t address) {
    _requested = 0;
    _error = ERROR_NONE;
    _flags &= (uint8_t)~FLAG_STOP_AFTER_READ;
    _start((address << 1) | TW_WRITE);
}

uint8_t uWireClass::endTransmission(bool sendStop) {
    if ((sendStop || _error != ERROR_NONE) && (_flags & FLAG_ACTIVE)) _stop();
    uint8_t error = _error;
    _error = ERROR_NONE;
    return error;
}

size_t uWireClass::write(uint8_t data) {
    if (_error != ERROR_NONE || !(_flags & FLAG_ACTIVE)) return 0;

    TWDR = data;
    TWCR = _BV(TWEN) | _BV(TWINT);
    if (!_wait(_BV(TWINT), true)) return 0;

    switch (TW_STATUS) {
        case TW_MT_DATA_ACK:
            return 1;
        case TW_MT_DATA_NACK:
            _error = ERROR_DATA_NACK;
            break;
        case TW_MT_ARB_LOST:
            _flags &= (uint8_t)~FLAG_ACTIVE;
            _error = ERROR_OTHER;
            break;
        default:
            _error = ERROR_OTHER;
            break;
    }
    return 0;
}

size_t uWireClass::write(const uint8_t* data, size_t len) {
    if (!data && len) return 0;

    size_t written = 0;
    while (written < len && write(data[written])) written++;
    return written;
}

uint8_t uWireClass::requestFrom(uint8_t address, uint8_t length, bool sendStop) {
    _requested = 0;
    _error = ERROR_NONE;

    if (sendStop) _flags |= FLAG_STOP_AFTER_READ;
    else _flags &= (uint8_t)~FLAG_STOP_AFTER_READ;

    if (!length) {
        if (sendStop && (_flags & FLAG_ACTIVE)) _stop();
        return 0;
    }

    if (!_start((address << 1) | TW_READ)) {
        if (_flags & FLAG_ACTIVE) _stop();
        return 0;
    }

    _requested = length;
    return length;
}

uint8_t uWireClass::requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop) {
    if (isize) {
        beginTransmission(address);
        if (isize > 3) isize = 3;
        while (isize && _error == ERROR_NONE) {
            isize--;
            write((uint8_t)(iaddress >> (isize * 8)));
        }
        if (endTransmission(false)) return 0;
    }
    return requestFrom(address, quantity, sendStop);
}

int uWireClass::read() {
    if (!_requested || _error != ERROR_NONE || !(_flags & FLAG_ACTIVE)) return -1;

    bool last = _requested == 1;
    TWCR = _BV(TWEN) | _BV(TWINT) | (last ? 0 : _BV(TWEA));
    if (!_wait(_BV(TWINT), true)) return -1;

    uint8_t status = TW_STATUS;
    if (status != (last ? TW_MR_DATA_NACK : TW_MR_DATA_ACK)) {
        _requested = 0;
        _error = ERROR_OTHER;
        if (_flags & FLAG_ACTIVE) _stop();
        return -1;
    }

    uint8_t data = TWDR;
    _requested--;
    if (last && (_flags & FLAG_STOP_AFTER_READ)) _stop();
    return data;
}

size_t uWireClass::read(uint8_t* data, size_t len) {
    if (!data && len) return 0;

    size_t readCount = 0;
    while (readCount < len && _requested) {
        int value = read();
        if (value < 0) break;
        data[readCount++] = (uint8_t)value;
    }
    return readCount;
}

bool uWireClass::_wait(uint8_t mask, bool state) {
#if UWIRE_TIMEOUT > 0
    uint16_t timeout = UWIRE_TIMEOUT_LOOPS;
    while (((TWCR & mask) != 0) != state) {
        if (!--timeout) {
            _requested = 0;
            _flags = 0;
            _error = ERROR_TIMEOUT;
            _reset();
            return false;
        }
    }
#else
    while (((TWCR & mask) != 0) != state) {
    }
#endif
    return true;
}

bool uWireClass::_start(uint8_t address) {
    TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT);
    if (!_wait(_BV(TWINT), true)) return false;

    uint8_t status = TW_STATUS;
    if (status != TW_START && status != TW_REP_START) {
        _flags &= (uint8_t)~FLAG_ACTIVE;
        _error = ERROR_OTHER;
        _reset();
        return false;
    }

    _flags |= FLAG_ACTIVE;
    TWDR = address;
    TWCR = _BV(TWEN) | _BV(TWINT);
    if (!_wait(_BV(TWINT), true)) return false;

    status = TW_STATUS;
    if (status == ((address & TW_READ) ? TW_MR_SLA_ACK : TW_MT_SLA_ACK)) return true;

    if (status == TW_MT_SLA_NACK || status == TW_MR_SLA_NACK) _error = ERROR_ADDRESS_NACK;
    else _error = ERROR_OTHER;

    if (status == TW_MT_ARB_LOST) _flags &= (uint8_t)~FLAG_ACTIVE;
    return false;
}

void uWireClass::_stop() {
    TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);
    _wait(_BV(TWSTO), false);
    _flags = 0;
    _requested = 0;
}

void uWireClass::_reset() {
    TWCR = 0;
    TWCR = _BV(TWEN);
}

uWireClass uWire;
