# Puzzle Image Preparation Tool - Summary

## What It Does

This tool prepares any image for use in the ESP32-4848S040C_I Sliding Puzzle Game by:

1. **Detecting and Removing Borders** - Automatically identifies solid color borders (like photo frames) and crops them away
2. **Intelligent Resizing** - Scales images to fill 480x480 while maintaining aspect ratio
3. **Center Cropping** - Ensures the most important part of the image is preserved
4. **Tile Preview** - Shows exactly how the puzzle will look when split into tiles
5. **Optional C Header** - Generates embeddable code for including images in firmware

## Files Created

| File | Description |
|------|-------------|
| `prepare_puzzle_image.py` | Main Python script (400+ lines) |
| `requirements.txt` | Python dependencies (Pillow, numpy) |
| `prepare_image.bat` | Windows batch file for easy use |
| `create_test_images.py` | Generates test images for demonstration |
| `README.md` | Complete documentation |
| `TOOL_SUMMARY.md` | This file |

## Quick Start

### Installation (One-Time)

```bash
# Install Python dependencies
pip install -r requirements.txt
```

### Windows Users (Easiest)

```cmd
# Drag and drop your image onto prepare_image.bat
# OR run from command line:
prepare_image.bat my_photo.jpg
```

### Command Line (All Platforms)

```bash
# Basic usage
python prepare_puzzle_image.py photo.jpg

# With custom name
python prepare_puzzle_image.py photo.jpg mountain_scene

# Different grid size
python prepare_puzzle_image.py photo.jpg --tiles 5

# Generate C header for embedding
python prepare_puzzle_image.py photo.jpg --header
```

## Output Examples

For input `vacation.jpg`:

```
output/
├── vacation_480x480.png      ← Final image ready for ESP32
├── vacation_preview.png      ← Shows tile grid with numbers
└── vacation.h                ← C header (if --header used)
```

## Features Explained

### 1. Border Detection

**Problem:** Many images have solid color borders (white frames, black letterboxing, etc.)

**Solution:** The tool scans edges and detects uniform color regions, then crops them:

```
[==============================]
[  #####################       ]  ← White border detected
[  #     PHOTO       #        ]
[  #                 #        ]
[  #####################      ]
[==============================]

After processing:
#####################
#     PHOTO       #
#                 #
#####################
```

**Threshold Control:**
- `--threshold 5` = Very sensitive (detects subtle borders)
- `--threshold 10` = Default (good for most cases)
- `--threshold 20` = Less sensitive (only obvious borders)

### 2. Smart Resizing

**Problem:** Images come in all sizes and aspect ratios

**Solution:** The tool:
1. Calculates the best scaling to fill 480x480 (no black bars!)
2. Uses high-quality Lanczos resampling
3. Center-crops to exactly 480x480

```
Original: 1920x1080 (landscape)
↓
Resize to: 853x480 (maintains aspect)
↓
Center crop: 480x480 (crops 186px from each side)
```

### 3. Tile Preview

Shows exactly how the puzzle will look:

```
For 4x4 grid:
┌────┬────┬────┬────┐
│ 1  │ 2  │ 3  │ 4  │
├────┼────┼────┼────┤
│ 5  │ 6  │ 7  │ 8  │
├────┼────┼────┼────┤
│ 9  │ 10 │ 11 │ 12 │
├────┼────┼────┼────┤
│ 13 │ 14 │ 15 │    │  ← Empty space
└────┴────┴────┴────┘

Each tile: 120x120 pixels
Gap: 2 pixels (for visual separation)
```

### 4. C Header Generation

**For embedding small images directly in firmware:**

```cpp
// vacation.h (generated)
const uint16_t vacation[230400] PROGMEM = {
    0x1234, 0x5678, 0x9ABC, ...  // RGB565 pixels
};
const int vacation_width = 480;
const int vacation_height = 480;
```

**Usage in code:**
```cpp
#include "vacation.h"

void drawPuzzleImage() {
    for (int y = 0; y < vacation_height; y++) {
        for (int x = 0; x < vacation_width; x++) {
            int idx = y * vacation_width + x;
            uint16_t color = pgm_read_word(&vacation[idx]);
            tft.drawPixel(x, y, color);
        }
    }
}
```

**Note:** 480x480 RGB565 image = ~450KB. Only use for 1-2 embedded images. For multiple images, use SD card or SPIFFS.

## Grid Size Reference

| Grid | Tiles | Tile Size | Difficulty | Memory (sprites) |
|------|-------|-----------|------------|------------------|
| 3x3  | 8     | 160x160px | Easy       | ~400KB           |
| 4x4  | 15    | 120x120px | Medium     | ~650KB           |
| 5x5  | 24    | 96x96px   | Hard       | ~900KB           |
| 6x6  | 35    | 80x80px   | Expert     | ~1.1MB           |

**Note:** ESP32-4848S040C_I has 8MB PSRAM, so even 6x6 fits comfortably.

## Command Line Reference

### Basic Commands

```bash
# Process with defaults (4x4 grid)
python prepare_puzzle_image.py input.jpg

# Custom output name
python prepare_puzzle_image.py input.jpg my_puzzle

# Different grid size
python prepare_puzzle_image.py input.jpg --tiles 3
python prepare_puzzle_image.py input.jpg --tiles 5
```

### Advanced Options

```bash
# Adjust border detection
python prepare_puzzle_image.py input.jpg --threshold 15

# Skip preview generation
python prepare_puzzle_image.py input.jpg --no-preview

# Generate C header (RGB565)
python prepare_puzzle_image.py input.jpg --header

# Generate C header (RGB888)
python prepare_puzzle_image.py input.jpg --header --format rgb888

# Custom size (if not 480x480)
python prepare_puzzle_image.py input.jpg --size 512
```

### Batch Processing (Linux/Mac)

```bash
# Process all JPGs in a folder
for img in *.jpg; do
    python prepare_puzzle_image.py "$img"
done

# Process with custom names
for img in *.jpg; do
    name=$(basename "$img" .jpg)
    python prepare_puzzle_image.py "$img" "${name}_puzzle"
done
```

### Batch Processing (Windows PowerShell)

```powershell
# Process all JPGs in a folder
Get-ChildItem *.jpg | ForEach-Object {
    python prepare_puzzle_image.py $_.FullName
}
```

## Testing the Tool

Generate test images to verify the tool works:

```bash
# Create test patterns
python create_test_images.py

# Process them
python prepare_puzzle_image.py test_images/gradient_with_border.png
python prepare_puzzle_image.py test_images/pattern_with_border.png
python prepare_puzzle_image.py test_images/landscape_with_border.png
```

## Image Guidelines for Best Results

### ✅ Good Images

- **Landscapes** - Mountains, beaches, cityscapes
- **Portraits** - Clear facial features, good lighting
- **Artwork** - Paintings, illustrations with distinct areas
- **Objects** - Cars, animals, buildings with clear shapes
- **Colorful scenes** - Markets, festivals, gardens

### ❌ Challenging Images

- **Solid colors** - Not much to solve
- **Repetitive patterns** - Hard to differentiate tiles
- **Text-heavy** - Uncomfortable to solve
- **Very dark/light** - Low contrast makes it hard
- **Abstract patterns** - No reference points

### 📏 Technical Specs

- **Minimum resolution:** 480x480 (larger is better for quality)
- **Recommended:** 1000x1000 or larger
- **Format:** JPEG, PNG, BMP, GIF (any Pillow-supported format)
- **Aspect ratio:** Any (will be center-cropped)
- **Color space:** RGB (will convert automatically)

## Integration with ESP32 Game

### Method 1: Load from File System

```cpp
// Load from SPIFFS/SD card
File f = SPIFFS.open("/puzzles/vacation_480x480.png", "r");
if (f) {
    // Use LovyanGFX to decode and draw
    tft.drawPngFile(f, 0, 0);
    f.close();
}
```

### Method 2: Embed in Firmware (Small Images)

```cpp
// Include generated header
#include "puzzles/vacation.h"

// Draw from PROGMEM
void drawEmbeddedPuzzle() {
    for (int i = 0; i < vacation_width * vacation_height; i++) {
        uint16_t color = pgm_read_word(&vacation[i]);
        int x = i % vacation_width;
        int y = i / vacation_width;
        tft.drawPixel(x, y, color);
    }
}
```

### Method 3: Load into Sprites (Best for Game)

```cpp
LGFX_Sprite tiles[15];  // For 4x4 (15 tiles + empty)

void loadPuzzleIntoTiles(const char* filename) {
    // Load full 480x480 image
    tft.drawPngFile(filename, 0, 0);

    // Split into tiles
    for (int i = 0; i < 15; i++) {
        tiles[i].createSprite(120, 120);

        int row = i / 4;
        int col = i % 4;

        // Copy tile from screen
        tiles[i].readRect(col * 120, row * 120, 120, 120, &tiles[i]);
    }
}
```

## Performance & Memory

### Processing Time

- **Small image (800x600):** ~1 second
- **Medium image (2000x1500):** ~2 seconds
- **Large image (4000x3000):** ~3-5 seconds

### Memory Usage (During Processing)

- **Peak RAM:** 50-200 MB (depends on input size)
- **Disk space:** Output ~500KB per image

### Output Sizes

| Format | Size |
|--------|------|
| 480x480 PNG (optimized) | ~200-500 KB |
| Preview PNG | ~300-600 KB |
| C header (RGB565) | ~900 KB |
| C header (RGB888) | ~1.4 MB |

## Troubleshooting

### "No module named 'PIL'"

```bash
pip install Pillow
```

### "No module named 'numpy'"

```bash
pip install numpy
```

### Borders Not Removed

Try increasing threshold:
```bash
python prepare_puzzle_image.py image.jpg --threshold 20
```

### Image Too Dark/Light After Processing

The tool preserves original colors. Adjust your source image first with photo editing software.

### Font Not Found (Preview Numbers)

The script tries to use Arial. If not found, it falls back to default font. This is cosmetic only.

## Future Enhancements (Possible)

- [ ] Batch processing mode (process folder of images)
- [ ] Auto-contrast adjustment
- [ ] Sharpen/enhance filter
- [ ] Generate multiple difficulty levels from one image
- [ ] Web interface (drag & drop)
- [ ] Direct upload to ESP32 via WiFi
- [ ] Animated GIF support (extract frames)

## Support

For issues or questions:
1. Check README.md for detailed documentation
2. Verify Python and dependencies are installed
3. Test with generated test images
4. Check output/ folder permissions

## License

Free to use and modify for your projects.

---

**Created for:** ESP32-4848S040C_I Sliding Puzzle Game
**Author:** Generated by Claude
**Version:** 1.0
