# Sliding 15-Puzzle Game

ESP32-based sliding tile puzzle game for ESP32-4848S040C_I development board.

## Hardware

- **Board**: ESP32-4848S040C_I (Sunton 4.0")
- **Display**: ST7701S 480x480 RGB LCD
- **Touch**: GT911 capacitive touch
- **Flash**: 8MB
- **PSRAM**: 8MB

## Features

- 15 unique puzzle images across 3 difficulty levels
- Easy (3x3), Medium (4x4), Hard (5x5) grids
- Touch-based tile sliding mechanics
- Move counter and timer
- AI-generated puzzle artwork

## Build & Upload

```bash
# Convert images (if modified)
python tools/convert_to_rgb565.py

# Build firmware
pio run

# Upload filesystem
pio run --target uploadfs

# Upload firmware
pio run --target upload
```

## Current Status: v0.9

✅ Display initialization
✅ Image loading and display
✅ Touch detection
✅ 15 puzzle images (RGB565 format)
✅ Correct color rendering

🚧 Game mechanics (in progress)

## RGB565 Color Format

This hardware requires **byte-swapped RGB565** format. See `docs/COLOR_FORMAT.md` for details.

## License

Educational project.
