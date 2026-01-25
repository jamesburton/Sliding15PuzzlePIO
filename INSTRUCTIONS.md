# Sliding 15-Puzzle - Implementation Instructions

## Target Hardware
**ESP32-4848S040C_I** - Sunton 4.0" 480x480 Touch Display
- **Board:** ESP32-S3-DevKitC-1
- **Display:** 4.0" 480x480 IPS (ST7701S Driver via 3-wire SPI + RGB)
- **Touch Controller:** GT911 Capacitive Touch
- **Memory:** PSRAM enabled

## Reference Project
Use **c:\Development\PlatformIO\SnakePIO** as the reference implementation for:
- PlatformIO configuration (platformio.ini)
- Library dependencies (LovyanGFX 1.2.7)
- Platform version (espressif32@6.9.0)
- Display initialization and pinout (ST7701S setup)
- Touch controller setup (GT911)
- Memory configuration (PSRAM settings)
- Build flags and partition scheme

## Game Overview
Classic sliding puzzle game where players:
- Rearrange numbered tiles (1-15) into correct order
- Move tiles into the empty space by tapping adjacent tiles
- Can use numbers or a custom image split into tiles
- Win when all tiles are in correct sequence

## Implementation Requirements

### Core Features
1. **Game Board**
   - 4x4 grid with 15 tiles + 1 empty space
   - Tile size: ~110-115px per tile for 480x480 display
   - Touch to move tiles adjacent to empty space
   - Smooth sliding animations

2. **Game Mechanics**
   - Tap tile adjacent to empty space to slide it
   - Only allow valid moves (tiles next to empty space)
   - Track move counter
   - Shuffle algorithm that ensures solvable puzzle
   - Win detection (all tiles in correct position)
   - Timer (optional)

3. **Display Modes**
   - **Number Mode:** Tiles numbered 1-15
   - **Image Mode:** Upload/embed image, split into 15 pieces
   - Preview button to show solved state

4. **UI Components**
   - Title screen with mode selection
   - Move counter display
   - Timer (optional)
   - Shuffle/New Game button
   - Solve preview overlay
   - Win screen with stats

### Technical Considerations
- Tile movement validation: check if clicked tile is adjacent to empty space
- Shuffle algorithm: perform random valid moves (not random arrangement - prevents unsolvable states)
- Image mode: split source image into 4x4 grid, store tile sprites
- Animation: smooth slide from current position to empty space
- Touch detection: map touch coordinates to grid position
- Solvability: ensure puzzle is always solvable after shuffle

### Suggested Features
- **Difficulty levels:**
  - Easy: 3x3 grid (8 tiles)
  - Medium: 4x4 grid (15 tiles)
  - Hard: 5x5 grid (24 tiles)
- **Image selection:**
  - Preloaded images (scenic, patterns, etc.)
  - Custom image upload via SD card or WiFi
- **Visual aids:**
  - Show numbers on image pieces
  - Highlight valid movable tiles
  - Peek at solution

## Getting Started
1. Copy platformio.ini from SnakePIO and adapt as needed
2. Set up LovyanGFX display configuration matching SnakePIO
3. Implement game board data structure (grid array)
4. Create tile movement logic and validation
5. Build touch-to-tile mapping system
6. Implement shuffle algorithm (random valid moves)
7. Create number tile rendering
8. Add image mode with tile sprites (optional)
9. Implement animations
10. Create UI and game states

## Notes
- 480x480 display works perfectly for 4x4 grid (~115px tiles with borders)
- Shuffle should make 100+ random valid moves to properly scramble
- Image mode requires PSRAM for storing tile sprites
- Consider adding sound effects for tile slides and win condition
- Test with different grid sizes for difficulty options
