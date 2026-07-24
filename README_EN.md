This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/uWire.svg?color=brightgreen)](https://github.com/GyverLibs/uWire/releases/latest/download/uWire.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/uWire.svg)](https://registry.platformio.org/libraries/gyverlibs/uWire)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/uWire?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# uWire
Compact wrapper on AVR TWI (I2C) with Arduino Wire-like API

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

## Contents
- [Use of use](#usage)
- [Versions](#versions)
- [Installation](#install)
- [Bugs and feedback](#feedback)

<a id="usage"></a>

## Use of use
### Initialization
Global facility declared`uWire`.

### Description of classes
```cpp
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

uint8_t available();
Error getError();

enum Error {
    ERROR_NONE = 0,
    ERROR_ADDRESS_NACK = 2,
    ERROR_DATA_NACK = 3,
    ERROR_OTHER = 4,
};
```

## Examples
```cpp
#include <Arduino.h>
#include <uWire.h>

void setup() {
    Serial.begin(115200);
    uWire.begin();
}

void loop() {
    Serial.println("Begin scan");

    for (uint8_t addr = 1; addr <= 127; addr++) {
        uWire.beginTransmission(addr);
        if (!uWire.endTransmission()) {
            Serial.print("0x");
            Serial.println(addr, HEX);
        }
    }

    Serial.println("End scan");
    Serial.println();
    delay(1000);
}
```

<a id="versions"></a>

## Versions
- v1.0

<a id="install"></a>
## Installation
- The library can be found under the name **uWire** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/uWire/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="feedback"></a>

## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
