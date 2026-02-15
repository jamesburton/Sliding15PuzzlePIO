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

## Current Status: v1.1 - Polished Experience

✅ Display initialization
✅ Image loading and display
✅ Touch detection
✅ 15 puzzle images (RGB565 format)
✅ Correct color rendering
✅ **Complete game mechanics**
✅ **4 game screens** (main menu, puzzle select, gameplay, win screen)
✅ **Touch-based tile sliding**
✅ **Move counter and timer**
✅ **Win detection**
✅ **Smooth tile animations** (180ms slide animation)
✅ **Visual touch feedback** (valid/invalid move indicators)
✅ **Optional sound effects** (PWM buzzer support)

## How to Play

1. **Main Menu**: Select difficulty (Easy 3x3, Medium 4x4, Hard 5x5)
2. **Puzzle Select**: Choose from 5 unique images per difficulty
3. **Play**: Touch tiles adjacent to empty space to slide them
4. **Win**: Arrange all tiles in correct order
5. **Stats**: View moves and time on completion

## Game Features

- **Solvable Puzzles**: Shuffle algorithm guarantees solvable configurations
- **Touch Controls**: Intuitive tile sliding with 250ms debouncing
- **Smooth Animations**: 180ms interpolated tile sliding for polished feel
- **Visual Feedback**: White border on valid tiles, red flash on invalid moves
- **Performance**: Efficient PSRAM-based image slicing for smooth rendering
- **Memory Management**: Automatic cleanup on screen transitions
- **Timer**: Starts on first move, tracks completion time
- **Optional Audio**: PWM buzzer support for slide, error, and win sounds (disabled by default)

## Technical Details

- **Core Engine**: `src/SlidingPuzzle.hpp` - Puzzle logic and validation
- **Implementation**: `src/main.cpp` - Full game with 4 screens
- **Memory**: PSRAM allocation for 480x480 puzzle images (~450KB each)
- **Image Format**: RGB565 with byte swapping (see `docs/COLOR_FORMAT.md`)

## Optional Audio Setup

Sound effects are disabled by default. To enable:

1. Connect a **passive buzzer** to **GPIO2** (or GPIO1/GPIO40)
2. Edit `src/main.cpp` line 11: Uncomment `#define ENABLE_SOUND`
3. Rebuild and upload firmware

**Note**: GPIO2 is shared with relay on this board. Choose GPIO1 or GPIO40 if you need relay functionality.

## Future Enhancements

See `docs/IMPLEMENTATION_PLAN.md` for planned features:
- v1.2: Best time/moves tracking, image preview
- v1.3: Auto-solve hints, statistics screen

## RGB565 Color Format

This hardware requires **byte-swapped RGB565** format. See `docs/COLOR_FORMAT.md` for details.

## License

Educational project.
