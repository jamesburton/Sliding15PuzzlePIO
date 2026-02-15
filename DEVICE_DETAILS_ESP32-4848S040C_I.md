# ESP32-4848S040C_I Device Specifications

## Overview
**Manufacturer:** Sunton
**Product Name:** ESP32-4848S040C_I
**Display Size:** 4.0 inches
**Resolution:** 480x480 pixels
**Type:** IPS LCD with Capacitive Touch

---

## Hardware Specifications

### Microcontroller
- **Chip:** ESP32-S3-WROOM-1
- **CPU:** Dual-core Xtensa LX7 @ 240 MHz
- **Flash:** 16 MB
- **PSRAM:** 8 MB (OPI PSRAM)
- **Wi-Fi:** 802.11 b/g/n
- **Bluetooth:** BLE 5.0

### Display Panel
- **Driver IC:** ST7701S
- **Interface:** RGB565/RGB666 (16-bit parallel)
- **Resolution:** 480 x 480 pixels
- **Colors:** 262K (RGB666) / 65K (RGB565)
- **Viewing Angle:** IPS (178°)
- **Backlight:** PWM controllable LED

### Touch Controller
- **IC:** GT911
- **Type:** Capacitive multi-touch (up to 5 points)
- **Interface:** I2C
- **I2C Address:** 0x14 or 0x5D (configurable)

### Display Interface Details
- **Type:** 16-bit RGB parallel bus
- **Init Interface:** 3-wire SPI (for ST7701S initialization only)
- **Rendering Interface:** RGB parallel (for actual drawing)

---

## GPIO Pin Mapping

### RGB Parallel Interface (16-bit)

#### Blue Channel (5 bits: B0-B4)
| Signal | GPIO | Description |
|--------|------|-------------|
| B0     | 4    | Blue bit 0  |
| B1     | 5    | Blue bit 1  |
| B2     | 6    | Blue bit 2  |
| B3     | 7    | Blue bit 3  |
| B4     | 15   | Blue bit 4  |

#### Green Channel (6 bits: G0-G5)
| Signal | GPIO | Description |
|--------|------|-------------|
| G0     | 8    | Green bit 0 |
| G1     | 20   | Green bit 1 |
| G2     | 3    | Green bit 2 |
| G3     | 46   | Green bit 3 |
| G4     | 9    | Green bit 4 |
| G5     | 10   | Green bit 5 |

#### Red Channel (5 bits: R0-R4)
| Signal | GPIO | Description |
|--------|------|-------------|
| R0     | 11   | Red bit 0   |
| R1     | 12   | Red bit 1   |
| R2     | 13   | Red bit 2   |
| R3     | 14   | Red bit 3   |
| R4     | 0    | Red bit 4   |

### Sync and Control Signals
| Signal | GPIO | Description |
|--------|------|-------------|
| HSYNC  | 16   | Horizontal sync |
| VSYNC  | 17   | Vertical sync |
| DE     | 18   | Data enable |
| PCLK   | 21   | Pixel clock |
| BL     | 38   | Backlight PWM |

### 3-Wire SPI (ST7701S Init Only)
| Signal | GPIO | Description |
|--------|------|-------------|
| CS     | 39   | Chip select |
| SCK    | 48   | Serial clock |
| SDA    | 47   | Serial data (MOSI) |

**Note:** This SPI interface is ONLY used for initializing the ST7701S chip. After initialization, the display uses the RGB parallel interface for all rendering.

### Touch Controller (GT911 I2C)
| Signal | GPIO | Description |
|--------|------|-------------|
| SDA    | 19   | I2C data |
| SCL    | 45   | I2C clock |
| INT    | -1   | Interrupt (not used) |
| RST    | -1   | Reset (not used/shared) |

**I2C Configuration:**
- **Address:** 0x14 (primary) or 0x5D (alternate)
- **Frequency:** 400 kHz (fast mode)
- **Port:** Wire (I2C port 0)

---

## Display Timing Parameters

### RGB Interface Timing
```cpp
// Pixel Clock
#define WRITE_FREQ_HZ     11000000  // 11 MHz

// Horizontal Timing
#define HSYNC_POLARITY    1         // Active high
#define HSYNC_FRONT_PORCH 10
#define HSYNC_PULSE_WIDTH 8
#define HSYNC_BACK_PORCH  50

// Vertical Timing
#define VSYNC_POLARITY    1         // Active high
#define VSYNC_FRONT_PORCH 10
#define VSYNC_PULSE_WIDTH 8
#define VSYNC_BACK_PORCH  20

// Clock Polarity
#define PCLK_ACTIVE_NEG   0         // Data latched on rising edge
```

### Color Mode
- **Configured Mode:** RGB666 (18-bit, 262K colors)
- **Register Value:** 0x60 (in ST7701S command 0x3A)

---

## PlatformIO Configuration

### Complete platformio.ini
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino

; Memory Configuration
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.partitions = huge_app.csv

; Build Flags
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=0
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue

; Library Dependency Mode
lib_ldf_mode = deep

; Library Dependencies
lib_deps =
    lovyan03/LovyanGFX@1.2.7

; Serial Monitor
monitor_speed = 115200
```

### Partition Scheme (huge_app.csv)
Uses the built-in `huge_app.csv` partition table:
- **App:** ~3 MB
- **OTA:** Enabled (for firmware updates)
- **SPIFFS/FATFS:** Remaining flash space

---

## LovyanGFX Library Usage

### Complete Setup Example

#### 1. DisplayConfig.hpp
```cpp
#pragma once

// RGB Interface Pins
#define PIN_D0    4   // B0
#define PIN_D1    5   // B1
#define PIN_D2    6   // B2
#define PIN_D3    7   // B3
#define PIN_D4    15  // B4

#define PIN_D5    8   // G0
#define PIN_D6    20  // G1
#define PIN_D7    3   // G2
#define PIN_D8    46  // G3
#define PIN_D9    9   // G4
#define PIN_D10   10  // G5

#define PIN_D11   11  // R0
#define PIN_D12   12  // R1
#define PIN_D13   13  // R2
#define PIN_D14   14  // R3
#define PIN_D15   0   // R4

// Sync & Control
#define PIN_HSYNC 16
#define PIN_VSYNC 17
#define PIN_DE    18
#define PIN_PCLK  21
#define PIN_BL    38

// 3-Wire SPI for ST7701S Init
#define PIN_SPI_CS  39
#define PIN_SPI_SCK 48
#define PIN_SPI_SDA 47

// Touch (GT911)
#define PIN_TOUCH_SDA 19
#define PIN_TOUCH_SCL 45
#define PIN_TOUCH_INT -1
#define PIN_TOUCH_RST -1

#define TOUCH_I2C_ADDR 0x14
#define TOUCH_I2C_PORT 0
#define TOUCH_I2C_FREQ 400000

// Timing
#define WRITE_FREQ_HZ     11000000
#define HSYNC_POLARITY    1
#define HSYNC_FRONT_PORCH 10
#define HSYNC_PULSE_WIDTH 8
#define HSYNC_BACK_PORCH  50
#define VSYNC_POLARITY    1
#define VSYNC_FRONT_PORCH 10
#define VSYNC_PULSE_WIDTH 8
#define VSYNC_BACK_PORCH  20
#define PCLK_ACTIVE_NEG   0

// ST7701S Initialization Sequence
static const uint8_t ST7701_INIT_SEQUENCE[] = {
    0xFF, 5, 0x77, 0x01, 0x00, 0x00, 0x10,
    0xC0, 2, 0x3B, 0x00,
    0xC1, 2, 0x0D, 0x02,
    0xC2, 2, 0x31, 0x05,
    0xCD, 1, 0x00,
    0xB0, 16, 0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18,
    0xB1, 16, 0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18,
    0xFF, 5, 0x77, 0x01, 0x00, 0x00, 0x11,
    0xB0, 1, 0x60,
    0xB1, 1, 0x32,
    0xB2, 1, 0x07,
    0xB3, 1, 0x80,
    0xB5, 1, 0x49,
    0xB7, 1, 0x85,
    0xB8, 1, 0x21,
    0xC1, 1, 0x78,
    0xC2, 1, 0x78,
    0xE0, 3, 0x00, 0x1B, 0x02,
    0xE1, 11, 0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44,
    0xE2, 12, 0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00,
    0xE3, 4, 0x00, 0x00, 0x11, 0x11,
    0xE4, 2, 0x44, 0x44,
    0xE5, 16, 0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0,
    0xE6, 4, 0x00, 0x00, 0x11, 0x11,
    0xE7, 2, 0x44, 0x44,
    0xE8, 16, 0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0,
    0xEB, 7, 0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40,
    0xEC, 2, 0x3C, 0x00,
    0xED, 16, 0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA,
    0xFF, 5, 0x77, 0x01, 0x00, 0x00, 0x13,
    0xE5, 1, 0xE4,
    0xFF, 5, 0x77, 0x01, 0x00, 0x00, 0x00,
    0x3A, 1, 0x60, // RGB666
    0x11, 0,       // Sleep Out
    0x29, 0,       // Display On
    0x00           // End of sequence
};
```

#### 2. LGFX_Setup.hpp
```cpp
#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include "DisplayConfig.hpp"

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7701 _panel_instance;
    lgfx::Bus_RGB      _bus_instance;
    lgfx::Light_PWM    _light_instance;
    lgfx::Touch_GT911  _touch_instance;

public:
    LGFX(void)
    {
        // Configure RGB Bus
        {
            auto cfg = _bus_instance.config();
            cfg.panel = &_panel_instance;

            // RGB Data Pins
            cfg.pin_d0  = PIN_D0;
            cfg.pin_d1  = PIN_D1;
            cfg.pin_d2  = PIN_D2;
            cfg.pin_d3  = PIN_D3;
            cfg.pin_d4  = PIN_D4;
            cfg.pin_d5  = PIN_D5;
            cfg.pin_d6  = PIN_D6;
            cfg.pin_d7  = PIN_D7;
            cfg.pin_d8  = PIN_D8;
            cfg.pin_d9  = PIN_D9;
            cfg.pin_d10 = PIN_D10;
            cfg.pin_d11 = PIN_D11;
            cfg.pin_d12 = PIN_D12;
            cfg.pin_d13 = PIN_D13;
            cfg.pin_d14 = PIN_D14;
            cfg.pin_d15 = PIN_D15;

            // Sync Pins
            cfg.pin_henable = PIN_DE;
            cfg.pin_vsync   = PIN_VSYNC;
            cfg.pin_hsync   = PIN_HSYNC;
            cfg.pin_pclk    = PIN_PCLK;

            cfg.freq_write  = WRITE_FREQ_HZ;

            // Horizontal Timing
            cfg.hsync_polarity = HSYNC_POLARITY;
            cfg.hsync_front_porch = HSYNC_FRONT_PORCH;
            cfg.hsync_pulse_width = HSYNC_PULSE_WIDTH;
            cfg.hsync_back_porch  = HSYNC_BACK_PORCH;

            // Vertical Timing
            cfg.vsync_polarity = VSYNC_POLARITY;
            cfg.vsync_front_porch = VSYNC_FRONT_PORCH;
            cfg.vsync_pulse_width = VSYNC_PULSE_WIDTH;
            cfg.vsync_back_porch  = VSYNC_BACK_PORCH;

            // Clock Settings
            cfg.pclk_active_neg = PCLK_ACTIVE_NEG;
            cfg.de_idle_high    = 0;
            cfg.pclk_idle_high  = 0;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // Configure Panel
        {
            auto cfg = _panel_instance.config();
            cfg.memory_width  = 480;
            cfg.memory_height = 480;
            cfg.panel_width   = 480;
            cfg.panel_height  = 480;
            cfg.offset_x      = 0;
            cfg.offset_y      = 0;
            cfg.rgb_order     = true;  // BGR order

            _panel_instance.config(cfg);
        }

        // Configure Touch
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 0;
            cfg.x_max      = 479;
            cfg.y_min      = 0;
            cfg.y_max      = 479;
            cfg.pin_int    = PIN_TOUCH_INT;
            cfg.pin_rst    = PIN_TOUCH_RST;
            cfg.bus_shared = false;
            cfg.pin_sda    = PIN_TOUCH_SDA;
            cfg.pin_scl    = PIN_TOUCH_SCL;
            cfg.i2c_port   = TOUCH_I2C_PORT;
            cfg.i2c_addr   = TOUCH_I2C_ADDR;
            cfg.freq       = TOUCH_I2C_FREQ;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        // Configure Backlight
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = PIN_BL;
            cfg.invert      = false;
            cfg.freq        = 12000;
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};
```

#### 3. ST7701S Manual Initialization
```cpp
// 3-wire SPI protocol for ST7701S
// 9-bit SPI: First bit is D/C flag (0=Command, 1=Data)

void st7701_send(uint8_t data, bool is_cmd) {
    digitalWrite(PIN_SPI_CS, LOW);

    // Send D/C bit
    digitalWrite(PIN_SPI_SCK, LOW);
    digitalWrite(PIN_SPI_SDA, is_cmd ? LOW : HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_SPI_SCK, HIGH);
    delayMicroseconds(1);

    // Send 8 data bits (MSB first)
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_SPI_SCK, LOW);
        digitalWrite(PIN_SPI_SDA, (data & 0x80) ? HIGH : LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_SPI_SCK, HIGH);
        delayMicroseconds(1);
        data <<= 1;
    }

    digitalWrite(PIN_SPI_CS, HIGH);
    delayMicroseconds(1);
}

void st7701_write_command(uint8_t cmd) {
    st7701_send(cmd, true);
}

void st7701_write_data(uint8_t data) {
    st7701_send(data, false);
}

void run_st7701_init_sequence() {
    Serial.println("ST7701: Starting Manual Init...");

    // Configure SPI pins as outputs
    pinMode(PIN_SPI_CS, OUTPUT);
    pinMode(PIN_SPI_SDA, OUTPUT);
    pinMode(PIN_SPI_SCK, OUTPUT);

    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, HIGH);
    digitalWrite(PIN_SPI_SDA, HIGH);

    delay(120);  // Power-on delay

    // Process initialization sequence
    const size_t n = sizeof(ST7701_INIT_SEQUENCE);
    size_t i = 0;

    while (i < n) {
        const uint8_t cmd = ST7701_INIT_SEQUENCE[i++];

        if (cmd == 0x00 && i >= n) break;  // End of sequence

        st7701_write_command(cmd);

        if (i >= n) break;
        uint8_t len = ST7701_INIT_SEQUENCE[i++];

        // Send data bytes
        for (uint8_t j = 0; j < len; j++) {
            if (i >= n) break;
            st7701_write_data(ST7701_INIT_SEQUENCE[i++]);
        }

        // Required delays for specific commands
        if (cmd == 0x11) delay(120);  // Sleep Out
        if (cmd == 0x29) delay(50);   // Display On
        if (cmd == 0xFF) delay(10);   // Page Select
    }

    Serial.println("ST7701: Manual Init Done.");
}
```

#### 4. Main Application Setup
```cpp
#include <Arduino.h>
#include "DisplayConfig.hpp"
#include "LGFX_Setup.hpp"

LGFX tft;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting...");
    Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());

    // CRITICAL: Run ST7701S init BEFORE LGFX init
    run_st7701_init_sequence();

    // Initialize LGFX
    tft.init();
    tft.setBrightness(255);  // 0-255

    // Test display
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("Display Ready!", 240, 240);

    Serial.println("Setup Complete");
}

void loop() {
    // Handle touch input
    int32_t x, y;
    if (tft.getTouch(&x, &y)) {
        Serial.printf("Touch: (%d, %d)\n", x, y);

        // Draw at touch position
        tft.fillCircle(x, y, 10, TFT_GREEN);
    }

    delay(10);
}
```

---

## Common LovyanGFX Operations

### Drawing Primitives
```cpp
// Fill screen
tft.fillScreen(TFT_BLACK);

// Draw shapes
tft.fillCircle(x, y, radius, color);
tft.fillRect(x, y, width, height, color);
tft.fillRoundRect(x, y, width, height, radius, color);
tft.drawRect(x, y, width, height, color);
tft.drawLine(x1, y1, x2, y2, color);

// Draw text
tft.setTextSize(2);
tft.setTextColor(TFT_WHITE);
tft.setTextDatum(textdatum_t::middle_center);
tft.drawString("Hello", 240, 240);
```

### Text Alignment (textdatum)
```cpp
textdatum_t::top_left
textdatum_t::top_center
textdatum_t::top_right
textdatum_t::middle_left
textdatum_t::middle_center
textdatum_t::middle_right
textdatum_t::bottom_left
textdatum_t::bottom_center
textdatum_t::bottom_right
```

### Touch Input
```cpp
// Basic touch reading
int32_t x, y;
if (tft.getTouch(&x, &y)) {
    // Touch detected at (x, y)
}

// Multi-touch
lgfx::touch_point_t tp[5];
int count = tft.getTouchRaw(tp, 5);
for (int i = 0; i < count; i++) {
    Serial.printf("Touch %d: (%d, %d)\n", i, tp[i].x, tp[i].y);
}
```

### Sprites (for Animations)
```cpp
LGFX_Sprite sprite(&tft);

// Create sprite (requires PSRAM)
sprite.createSprite(100, 100);

// Draw to sprite
sprite.fillScreen(TFT_BLACK);
sprite.drawString("Sprite", 50, 50);

// Push sprite to display
sprite.pushSprite(x, y);

// Delete when done
sprite.deleteSprite();
```

### Backlight Control
```cpp
tft.setBrightness(255);  // Full brightness
tft.setBrightness(128);  // Half brightness
tft.setBrightness(0);    // Off
```

---

## Troubleshooting

### Display Not Working
1. Verify ST7701S init runs BEFORE `tft.init()`
2. Check 3-wire SPI pins are correct (CS=39, SCK=48, SDA=47)
3. Verify RGB pin connections match DisplayConfig.hpp
4. Add debug prints in `run_st7701_init_sequence()`

### Touch Not Responding
1. Check I2C address (try 0x14 and 0x5D)
2. Verify SDA=19, SCL=45
3. Check I2C pull-ups (may need external 4.7kΩ resistors)
4. Test with: `tft.getTouchRaw()` for raw values

### Display Corruption/Flickering
1. Reduce `WRITE_FREQ_HZ` (try 8-10 MHz)
2. Check power supply (needs stable 5V, 1A+)
3. Verify timing parameters match device specs

### PSRAM Not Available
1. Check `board_build.arduino.memory_type = qio_opi`
2. Verify `-DBOARD_HAS_PSRAM` in build_flags
3. Add `-mfix-esp32-psram-cache-issue` flag
4. Check with `ESP.getPsramSize()` (should return 8388608)

### Serial Monitor Garbled
1. Set `monitor_speed = 115200`
2. If still garbled, add `-DARDUINO_USB_CDC_ON_BOOT=0`
3. Press reset button after upload completes

---

## Performance Tips

1. **Use Sprites for Animation** - Draw to sprite, then push to display
2. **Incremental Updates** - Only redraw changed areas
3. **PSRAM for Large Buffers** - Store images/sprites in PSRAM
4. **DMA Transfers** - LovyanGFX handles this automatically for RGB bus
5. **Optimize Loop** - Add small `delay(5-10)` to prevent CPU hogging
6. **Batch Operations** - Group drawing operations to minimize overhead

---

## Color Reference

### Standard Colors (16-bit RGB565)
```cpp
TFT_BLACK       0x0000
TFT_WHITE       0xFFFF
TFT_RED         0xF800
TFT_GREEN       0x07E0
TFT_BLUE        0x001F
TFT_CYAN        0x07FF
TFT_MAGENTA     0xF81F
TFT_YELLOW      0xFFE0
TFT_ORANGE      0xFC00
TFT_DARKGREY    0x7BEF
TFT_LIGHTGREY   0xC618
```

### Custom Colors
```cpp
// RGB565: 5 bits red, 6 bits green, 5 bits blue
uint16_t color = tft.color565(r, g, b);  // r, g, b = 0-255

// RGB888 to RGB565 conversion
uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
```

---

## Additional Resources

- **LovyanGFX Documentation:** https://github.com/lovyan03/LovyanGFX
- **ESP32-S3 Datasheet:** https://www.espressif.com/en/products/socs/esp32-s3
- **ST7701S Datasheet:** https://www.newhavendisplay.com/app_notes/ST7701S.pdf
- **GT911 Touch IC:** https://github.com/goodix/gt9xx

---

## License & Credits

**Device:** ESP32-4848S040C_I by Sunton
**Library:** LovyanGFX by lovyan03
**Platform:** ESP32 Arduino Core by Espressif

This document is based on working configurations from successful projects.
