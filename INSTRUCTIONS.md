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

### Key Insights from Snake Project

**PlatformIO Configuration:**
```ini
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.partitions = huge_app.csv
build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=0
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
lib_deps = lovyan03/LovyanGFX@1.2.7
```

**Pin Definitions (from DisplayConfig.hpp):**
- **RGB Interface (16-bit):** D0-D15 mapped to B0-B4, G0-G5, R0-R4
  - Blue: GPIOs 4,5,6,7,15
  - Green: GPIOs 8,20,3,46,9,10
  - Red: GPIOs 11,12,13,14,0
- **Sync/Control:**
  - HSYNC: GPIO 16
  - VSYNC: GPIO 17
  - DE (Data Enable): GPIO 18
  - PCLK: GPIO 21
  - Backlight: GPIO 38
- **3-Wire SPI (for ST7701S init only):**
  - CS: GPIO 39
  - SCK: GPIO 48
  - SDA/MOSI: GPIO 47
- **Touch (GT911 I2C):**
  - SDA: GPIO 19
  - SCL: GPIO 45
  - I2C Address: 0x14 (or 0x5D)
  - Frequency: 400kHz

**Display Initialization Sequence:**
1. Manual ST7701S initialization via 3-wire SPI (run before LGFX init)
2. ST7701S uses 9-bit SPI: first bit is D/C flag (0=cmd, 1=data)
3. Init sequence sets up timing, gamma, power, and enables RGB666 mode
4. After SPI init, LGFX configures RGB parallel bus for actual rendering
5. Set backlight brightness (0-255)

**LovyanGFX Configuration Pattern:**
```cpp
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7701 _panel_instance;
    lgfx::Bus_RGB      _bus_instance;
    lgfx::Light_PWM    _light_instance;
    lgfx::Touch_GT911  _touch_instance;
    // Configure bus, panel, touch, light in constructor
};
```

**Touch Handling Pattern:**
```cpp
// In loop():
int32_t x, y;
if (tft.getTouch(&x, &y)) {
    handleTouch(x, y);  // Process touch
} else {
    notifyTouchEnd();   // Handle touch release
}
```

**Display Timing (RGB Interface):**
- Write frequency: 11 MHz
- HSYNC: polarity=1, front=10, pulse=8, back=50
- VSYNC: polarity=1, front=10, pulse=8, back=20
- PCLK active neg: 0
- Resolution: 480x480 (RGB666 color mode)

**Memory Considerations:**
- PSRAM is available and required for sprites/image storage
- Use `ESP.getPsramSize()` to verify PSRAM availability
- Sprite buffers for smooth animations require PSRAM

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

**Implementation Best Practices (from Snake):**
- Use separate files for configuration (DisplayConfig.hpp, LGFX_Setup.hpp)
- Create game class with state machine (START, PLAYING, OPTIONS, WIN)
- Touch handling: detect press vs. release with debouncing for menus
- Incremental rendering: only redraw changed tiles (not full screen)
- Game loop structure:
  ```cpp
  void loop() {
      game.update();        // Game logic/timing
      int32_t x, y;
      if (tft.getTouch(&x, &y)) {
          game.handleTouch(x, y);
      } else {
          game.notifyTouchEnd();
      }
      delay(5);  // Prevent CPU hogging
  }
  ```
- Use textdatum for text alignment (top_left, middle_center, etc.)
- Button hit detection: check if touch X,Y falls within button bounds
- State-specific touch handling: different behavior per game state

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

### Phase 1: Hardware Setup
1. Copy platformio.ini from SnakePIO (exact configuration works)
2. Create DisplayConfig.hpp with pin definitions (copy from SnakePIO)
3. Create LGFX_Setup.hpp with LovyanGFX configuration
4. Implement ST7701S manual initialization (3-wire SPI sequence)
5. Test basic display output and touch input

### Phase 2: Core Game Structure
6. Create PuzzleGame class with state machine
7. Implement game board data structure:
   ```cpp
   struct Tile { int number; int x; int y; };
   Tile board[4][4];  // 4x4 grid
   Point emptyPos;    // Track empty tile position
   ```
8. Create tile rendering functions:
   - drawTile(int x, int y, int number)
   - drawBoard() - initial full render
   - updateTile(int x, int y) - incremental update

### Phase 3: Game Logic
9. Implement tile movement validation:
   - Check if tile is adjacent to empty space (up/down/left/right)
   - Swap tile with empty space
10. Build touch-to-tile mapping:
    ```cpp
    int getTileX(int32_t touchX) { return touchX / TILE_SIZE; }
    int getTileY(int32_t touchY) { return touchY / TILE_SIZE; }
    ```
11. Implement shuffle algorithm:
    - Perform 100-200 random valid moves from solved state
    - Guarantees solvable puzzle
12. Win detection: check if all tiles are in order

### Phase 4: UI & Polish
13. Create menu screens (start, options, win)
14. Add move counter and timer
15. Implement tile slide animations (optional but recommended)
16. Add image mode with PSRAM sprite storage (advanced)

### Phase 5: Testing
17. Test touch responsiveness and accuracy
18. Verify puzzle always solvable after shuffle
19. Test win detection
20. Performance testing (smooth 60fps rendering)

## Useful Code Patterns

### Grid Layout Calculations
```cpp
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480
#define GRID_SIZE 4
#define TILE_SIZE (SCREEN_WIDTH / GRID_SIZE)  // 120px per tile
#define TILE_GAP 2  // Border between tiles
#define TILE_DRAW_SIZE (TILE_SIZE - TILE_GAP)  // 118px

// Calculate screen position from grid position
int screenX = gridX * TILE_SIZE;
int screenY = gridY * TILE_SIZE;

// Calculate grid position from touch position
int gridX = touchX / TILE_SIZE;
int gridY = touchY / TILE_SIZE;
```

### Color Scheme Suggestions
```cpp
// Clean modern look
#define COLOR_BG        0x1082      // Dark blue-gray
#define COLOR_TILE      0x4208      // Medium blue
#define COLOR_EMPTY     TFT_BLACK   // Empty space
#define COLOR_TEXT      TFT_WHITE   // Tile numbers
#define COLOR_BORDER    0x2104      // Darker border

// Or use Snake's proven palette
#define COLOR_BG        TFT_BLACK
#define COLOR_TILE      TFT_DARKGREY
#define COLOR_HIGHLIGHT TFT_GREEN   // For valid moves
#define COLOR_TEXT      TFT_WHITE
```

### Animation Pattern (Smooth Tile Slide)
```cpp
void animateTile(int fromX, int fromY, int toX, int toY, int number) {
    const int steps = 10;
    int dx = (toX - fromX) * TILE_SIZE / steps;
    int dy = (toY - fromY) * TILE_SIZE / steps;

    for (int i = 0; i <= steps; i++) {
        int x = fromX * TILE_SIZE + (dx * i);
        int y = fromY * TILE_SIZE + (dy * i);

        // Clear previous position
        tft.fillRect(x - dx, y - dy, TILE_DRAW_SIZE, TILE_DRAW_SIZE, COLOR_EMPTY);

        // Draw at new position
        drawTileAt(x, y, number);

        delay(20);  // Animation speed
    }
}
```

### Shuffle Algorithm (Guaranteed Solvable)
```cpp
void shufflePuzzle() {
    const int moves = 200;  // More moves = harder
    int lastDir = -1;  // Prevent immediate undo

    for (int i = 0; i < moves; i++) {
        // Find valid moves (tiles adjacent to empty)
        std::vector<int> validDirs;
        if (emptyX > 0 && lastDir != 0) validDirs.push_back(0);  // Can move from left
        if (emptyX < 3 && lastDir != 1) validDirs.push_back(1);  // Can move from right
        if (emptyY > 0 && lastDir != 2) validDirs.push_back(2);  // Can move from up
        if (emptyY < 3 && lastDir != 3) validDirs.push_back(3);  // Can move from down

        int dir = validDirs[random(validDirs.size())];
        // Move tile into empty space based on direction
        // Update emptyX, emptyY
        lastDir = (dir + 2) % 4;  // Opposite direction
    }
}
```

## Notes
- 480x480 display works perfectly for 4x4 grid (120px tiles with 2px gaps)
- Shuffle should make 100-200 random valid moves to properly scramble
- Image mode requires PSRAM for storing tile sprites (use `sprite.createSprite()`)
- Consider adding sound effects for tile slides and win condition
- Test with different grid sizes for difficulty options:
  - Easy: 3x3 (160px tiles)
  - Medium: 4x4 (120px tiles)
  - Hard: 5x5 (96px tiles)
- Touch debouncing is critical for menu buttons (use `notifyTouchEnd()` pattern)
- LovyanGFX text datum modes: top_left, middle_center, bottom_right, etc.
- Use `setTextDatum()` before `drawString()` for proper text alignment
