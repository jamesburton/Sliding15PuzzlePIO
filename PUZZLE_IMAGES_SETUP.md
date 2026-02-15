# Puzzle Images Setup Guide

## Available Images

You have **15 puzzle images** (5 per difficulty level) ready to use:

### Easy (3x3 grid - 8 tiles)
1. `castle_on_hill_sunset.png`
2. `cute_anime_icecream_park.png`
3. `puppy_in_toy_car.png`
4. `space_planet_big_moons.png`
5. `underwater_turtle_reef_simple.png`

### Medium (4x4 grid - 15 tiles)
1. `autumn_forest_path.png`
2. `fantasy_market_street.png`
3. `robot_and_kids_workshop.png`
4. `sci_fi_hangar_ship.png`
5. `tropical_beach_palm_wave.png`

### Hard (5x5 grid - 24 tiles)
1. `cosmic_swirl_nebula_dense.png`
2. `cyberpunk_city_rain.png`
3. `mechanical_gears_closeup.png`
4. `pile_of_androids_dense.png`
5. `wizard_library_shelves.png`

---

## Image Storage Strategy

### Option 1: LittleFS (RECOMMENDED)
Store images in the ESP32's flash filesystem (LittleFS/SPIFFS).

**Advantages:**
- No SD card required
- Fast access
- Reliable
- Images persist across power cycles

**Limitations:**
- Flash space limited (~3-4 MB available with huge_app partition)
- Each 480x480 PNG is ~200-500 KB
- Can fit ~10-15 images comfortably

**Implementation:**
1. Upload images to LittleFS using PlatformIO
2. Load images on demand
3. Split into sprites for gameplay

### Option 2: SD Card
Store images on microSD card (if available).

**Advantages:**
- Unlimited storage
- Easy to add/remove images
- Can store hundreds of puzzles

**Disadvantages:**
- Requires SD card slot
- Slightly slower access
- SD card may fail/corrupt

### Option 3: Embedded (Small Set Only)
Convert images to C arrays and embed in firmware.

**Advantages:**
- No filesystem needed
- Fastest access
- Most reliable

**Disadvantages:**
- Each image ~450 KB in flash
- Can only embed 2-3 images max
- Firmware size increases

---

## Recommended Approach: LittleFS with Selective Loading

**Strategy:**
- Store all 15 images in LittleFS
- Load one puzzle at a time into PSRAM sprites
- Support puzzle selection menu
- Progressive loading for smooth UX

---

## Implementation Steps

### Step 1: Prepare LittleFS Data Directory

Create the data directory structure:

```
Sliding15PuzzlePIO/
├── data/                          ← LittleFS upload folder
│   └── puzzles/
│       ├── easy/
│       │   ├── castle_on_hill_sunset.png
│       │   ├── cute_anime_icecream_park.png
│       │   ├── puppy_in_toy_car.png
│       │   ├── space_planet_big_moons.png
│       │   └── underwater_turtle_reef_simple.png
│       ├── medium/
│       │   ├── autumn_forest_path.png
│       │   ├── fantasy_market_street.png
│       │   ├── robot_and_kids_workshop.png
│       │   ├── sci_fi_hangar_ship.png
│       │   └── tropical_beach_palm_wave.png
│       └── hard/
│           ├── cosmic_swirl_nebula_dense.png
│           ├── cyberpunk_city_rain.png
│           ├── mechanical_gears_closeup.png
│           ├── pile_of_androids_dense.png
│           └── wizard_library_shelves.png
```

### Step 2: Copy Images to Data Directory

**Windows Command:**
```cmd
cd C:\Development\PlatformIO\Sliding15PuzzlePIO

mkdir data\puzzles\easy data\puzzles\medium data\puzzles\hard

xcopy images\sliding_puzzle_images_with_originals_480x480_final\Easy\*.png data\puzzles\easy\ /Y
xcopy images\sliding_puzzle_images_with_originals_480x480_final\Medium\*.png data\puzzles\medium\ /Y
xcopy images\sliding_puzzle_images_with_originals_480x480_final\Hard\*.png data\puzzles\hard\ /Y
```

**Or manually:**
- Create `data/puzzles/easy/`, `data/puzzles/medium/`, `data/puzzles/hard/`
- Copy corresponding images from `images/sliding_puzzle_images_with_originals_480x480_final/`

### Step 3: Update platformio.ini

Add LittleFS configuration:

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino

board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.partitions = huge_app.csv
board_build.filesystem = littlefs    ; ← Add this

build_flags =
    -DARDUINO_USB_CDC_ON_BOOT=0
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue

lib_ldf_mode = deep

lib_deps =
    lovyan03/LovyanGFX@1.2.7

monitor_speed = 115200
```

### Step 4: Upload Filesystem

**Using PlatformIO:**
```bash
pio run --target uploadfs
```

**Or in PlatformIO IDE:**
- Click "Build Filesystem Image"
- Click "Upload Filesystem Image"

This uploads everything in `data/` folder to the ESP32's LittleFS.

---

## Code Implementation

### Puzzle Image Loader

```cpp
#include <LittleFS.h>
#include <vector>

struct PuzzleInfo {
    String filename;
    String displayName;
    int gridSize;     // 3, 4, or 5
    int difficulty;   // 0=Easy, 1=Medium, 2=Hard
};

class PuzzleManager {
private:
    std::vector<PuzzleInfo> easyPuzzles;
    std::vector<PuzzleInfo> mediumPuzzles;
    std::vector<PuzzleInfo> hardPuzzles;

public:
    void init() {
        if (!LittleFS.begin()) {
            Serial.println("LittleFS mount failed!");
            return;
        }

        // Easy puzzles (3x3)
        easyPuzzles = {
            {"/puzzles/easy/castle_on_hill_sunset.png", "Castle Sunset", 3, 0},
            {"/puzzles/easy/cute_anime_icecream_park.png", "Ice Cream Park", 3, 0},
            {"/puzzles/easy/puppy_in_toy_car.png", "Puppy Car", 3, 0},
            {"/puzzles/easy/space_planet_big_moons.png", "Space Planet", 3, 0},
            {"/puzzles/easy/underwater_turtle_reef_simple.png", "Turtle Reef", 3, 0}
        };

        // Medium puzzles (4x4)
        mediumPuzzles = {
            {"/puzzles/medium/autumn_forest_path.png", "Autumn Path", 4, 1},
            {"/puzzles/medium/fantasy_market_street.png", "Fantasy Market", 4, 1},
            {"/puzzles/medium/robot_and_kids_workshop.png", "Robot Workshop", 4, 1},
            {"/puzzles/medium/sci_fi_hangar_ship.png", "Sci-Fi Hangar", 4, 1},
            {"/puzzles/medium/tropical_beach_palm_wave.png", "Beach Paradise", 4, 1}
        };

        // Hard puzzles (5x5)
        hardPuzzles = {
            {"/puzzles/hard/cosmic_swirl_nebula_dense.png", "Cosmic Nebula", 5, 2},
            {"/puzzles/hard/cyberpunk_city_rain.png", "Cyberpunk City", 5, 2},
            {"/puzzles/hard/mechanical_gears_closeup.png", "Mechanical Gears", 5, 2},
            {"/puzzles/hard/pile_of_androids_dense.png", "Android Pile", 5, 2},
            {"/puzzles/hard/wizard_library_shelves.png", "Wizard Library", 5, 2}
        };

        Serial.println("PuzzleManager initialized");
        Serial.printf("  Easy: %d puzzles\n", easyPuzzles.size());
        Serial.printf("  Medium: %d puzzles\n", mediumPuzzles.size());
        Serial.printf("  Hard: %d puzzles\n", hardPuzzles.size());
    }

    const std::vector<PuzzleInfo>& getPuzzles(int difficulty) {
        switch(difficulty) {
            case 0: return easyPuzzles;
            case 1: return mediumPuzzles;
            case 2: return hardPuzzles;
            default: return mediumPuzzles;
        }
    }

    int getPuzzleCount(int difficulty) {
        return getPuzzles(difficulty).size();
    }

    const PuzzleInfo& getPuzzle(int difficulty, int index) {
        const auto& puzzles = getPuzzles(difficulty);
        return puzzles[index % puzzles.size()];
    }
};
```

### Loading Puzzle into Sprites

```cpp
class PuzzleGame {
private:
    LGFX* tft;
    PuzzleManager puzzleManager;
    std::vector<LGFX_Sprite*> tiles;
    int gridSize;
    int tileSize;

public:
    bool loadPuzzle(int difficulty, int puzzleIndex) {
        // Get puzzle info
        const PuzzleInfo& puzzle = puzzleManager.getPuzzle(difficulty, puzzleIndex);
        gridSize = puzzle.gridSize;
        tileSize = 480 / gridSize;

        Serial.printf("Loading: %s (%dx%d)\n",
                      puzzle.displayName.c_str(), gridSize, gridSize);

        // Open file
        File file = LittleFS.open(puzzle.filename, "r");
        if (!file) {
            Serial.println("Failed to open puzzle file!");
            return false;
        }

        // Draw PNG to screen (temporary)
        tft->drawPngFile(file, 0, 0, 480, 480);
        file.close();

        // Clear old tiles
        for (auto* tile : tiles) {
            tile->deleteSprite();
            delete tile;
        }
        tiles.clear();

        // Create sprites from screen
        int numTiles = gridSize * gridSize - 1;  // -1 for empty space
        tiles.reserve(numTiles);

        for (int i = 0; i < numTiles; i++) {
            int row = i / gridSize;
            int col = i % gridSize;
            int x = col * tileSize;
            int y = row * tileSize;

            // Create sprite
            LGFX_Sprite* sprite = new LGFX_Sprite(tft);
            sprite->createSprite(tileSize, tileSize);

            // Read from screen into sprite
            sprite->readRect(x, y, tileSize, tileSize);

            tiles.push_back(sprite);
        }

        Serial.printf("Loaded %d tiles (%dx%d each)\n", numTiles, tileSize, tileSize);
        return true;
    }

    void drawTile(int tileIndex, int gridX, int gridY) {
        if (tileIndex >= 0 && tileIndex < tiles.size()) {
            tiles[tileIndex]->pushSprite(gridX * tileSize, gridY * tileSize);
        }
    }
};
```

### Puzzle Selection Menu

```cpp
void drawPuzzleSelectionMenu(LGFX* tft, PuzzleManager& pm, int difficulty) {
    tft->fillScreen(TFT_BLACK);

    // Title
    tft->setTextSize(3);
    tft->setTextColor(TFT_WHITE);
    tft->setTextDatum(textdatum_t::middle_center);

    const char* diffNames[] = {"EASY (3x3)", "MEDIUM (4x4)", "HARD (5x5)"};
    tft->drawString(diffNames[difficulty], 240, 30);

    // Draw puzzle selection buttons
    const auto& puzzles = pm.getPuzzles(difficulty);
    int count = puzzles.size();

    tft->setTextSize(2);
    int buttonHeight = 70;
    int startY = 80;

    for (int i = 0; i < count; i++) {
        int y = startY + (i * buttonHeight) + (i * 10);

        // Button background
        uint16_t color = TFT_DARKGREY;
        tft->fillRoundRect(40, y, 400, buttonHeight, 10, color);

        // Button text
        tft->setTextColor(TFT_WHITE);
        tft->drawString(puzzles[i].displayName, 240, y + buttonHeight/2);
    }

    // Back button
    tft->fillRoundRect(140, 440, 200, 40, 10, TFT_RED);
    tft->setTextSize(2);
    tft->drawString("BACK", 240, 460);
}

bool handlePuzzleSelectionTouch(int32_t x, int32_t y, int difficulty,
                                 PuzzleManager& pm, int& selectedIndex) {
    const auto& puzzles = pm.getPuzzles(difficulty);
    int count = puzzles.size();

    int buttonHeight = 70;
    int startY = 80;

    // Check puzzle buttons
    for (int i = 0; i < count; i++) {
        int buttonY = startY + (i * buttonHeight) + (i * 10);

        if (x >= 40 && x <= 440 &&
            y >= buttonY && y <= buttonY + buttonHeight) {
            selectedIndex = i;
            return true;  // Puzzle selected
        }
    }

    // Check back button
    if (x >= 140 && x <= 340 && y >= 440 && y <= 480) {
        return false;  // Go back
    }

    return false;
}
```

---

## Memory Usage Estimate

### Per Difficulty Level

**Easy (3x3):**
- Grid: 3x3 = 8 tiles
- Tile size: 160x160 pixels
- Memory: 8 tiles × 160×160×2 bytes = 409,600 bytes (~400 KB)

**Medium (4x4):**
- Grid: 4x4 = 15 tiles
- Tile size: 120x120 pixels
- Memory: 15 tiles × 120×120×2 bytes = 432,000 bytes (~422 KB)

**Hard (5x5):**
- Grid: 5x5 = 24 tiles
- Tile size: 96x96 pixels
- Memory: 24 tiles × 96×96×2 bytes = 442,368 bytes (~432 KB)

**Total PSRAM Usage:** ~400-450 KB per loaded puzzle

**ESP32-4848S040C_I PSRAM:** 8 MB total → Plenty of room! ✓

---

## Game Flow with Images

```
STARTUP
   ↓
MAIN MENU
├─ Play
├─ Select Difficulty
└─ Exit
   ↓
DIFFICULTY SELECTION
├─ Easy (3x3)
├─ Medium (4x4)
└─ Hard (5x5)
   ↓
PUZZLE SELECTION
├─ Puzzle 1
├─ Puzzle 2
├─ Puzzle 3
├─ Puzzle 4
└─ Puzzle 5
   ↓
LOAD PUZZLE
├─ Load PNG from LittleFS
├─ Split into sprites
└─ Shuffle tiles
   ↓
PLAY GAME
├─ Touch to move tiles
├─ Track moves/time
└─ Detect win
   ↓
WIN SCREEN
├─ Show stats
├─ Play Again
└─ Back to Menu
```

---

## File Size Reference

Actual file sizes for your images:

```bash
# Check file sizes
ls -lh data/puzzles/*/*.png

# Typical sizes:
# Easy puzzles:   200-400 KB each × 5 = ~1.5 MB
# Medium puzzles: 300-500 KB each × 5 = ~2.0 MB
# Hard puzzles:   400-600 KB each × 5 = ~2.5 MB
# Total:          ~6 MB for all 15 images
```

**LittleFS Capacity:** With `huge_app.csv` partition, you have ~3-4 MB available.

**Solution:**
- Option A: Compress PNGs further (reduce to ~150-200 KB each)
- Option B: Use custom partition table with more LittleFS space
- Option C: Store 8-10 images in LittleFS, rest on SD card
- Option D: Use JPEG instead of PNG (smaller file size)

---

## Next Steps

1. **Create data directory structure**
2. **Copy images to data folder**
3. **Update platformio.ini**
4. **Implement PuzzleManager class**
5. **Implement puzzle loading code**
6. **Upload filesystem to ESP32**
7. **Test puzzle selection and loading**

---

## Testing Checklist

- [ ] LittleFS mounts successfully
- [ ] Can list files in `/puzzles/easy/`
- [ ] Can open and read PNG files
- [ ] PNG decoding works with LovyanGFX
- [ ] Sprites created correctly
- [ ] All 15 puzzles load without errors
- [ ] Memory usage stays within limits
- [ ] Puzzle selection menu responsive
- [ ] Touch detection accurate on menu

---

## Troubleshooting

### "LittleFS mount failed"
- Check `board_build.filesystem = littlefs` in platformio.ini
- Verify filesystem was uploaded (`pio run --target uploadfs`)

### "File not found"
- Check file paths (case-sensitive!)
- Verify files uploaded correctly
- Use LittleFS.exists("/puzzles/easy/castle_on_hill_sunset.png")

### "PNG decode failed"
- Check file is valid PNG
- Verify file not corrupted during upload
- Try re-uploading filesystem

### "Out of memory"
- Delete old sprites before loading new puzzle
- Use sprite->deleteSprite() and delete
- Check PSRAM available with ESP.getFreePsram()

---

This setup will give you a professional puzzle game with 15 different puzzles across 3 difficulty levels!
