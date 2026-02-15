# Puzzle Image Preparation Tools

Tools for preparing images for the ESP32-4848S040C_I Sliding Puzzle Game.

## Installation

Install required Python libraries:

```bash
pip install Pillow numpy
```

Or use the provided requirements file:

```bash
pip install -r requirements.txt
```

## Quick Start

### Basic Usage

```bash
python prepare_puzzle_image.py your_photo.jpg
```

This will:
1. Detect and remove solid color borders
2. Resize and crop to 480x480 centered
3. Generate a tile preview showing the puzzle split
4. Save outputs to `output/` directory

### Specify Output Name

```bash
python prepare_puzzle_image.py landscape.jpg mountain_scene
```

Outputs: `mountain_scene_480x480.png` and `mountain_scene_preview.png`

### Generate C Header File

For embedding images directly in firmware:

```bash
python prepare_puzzle_image.py photo.jpg --header
```

This creates a `.h` file with the image as a C array in RGB565 format.

### Different Grid Sizes

```bash
python prepare_puzzle_image.py photo.jpg --tiles 3   # 3x3 grid (8 tiles)
python prepare_puzzle_image.py photo.jpg --tiles 4   # 4x4 grid (15 tiles) - default
python prepare_puzzle_image.py photo.jpg --tiles 5   # 5x5 grid (24 tiles)
```

## Advanced Options

### Adjust Border Detection Sensitivity

```bash
# More sensitive (detect subtle borders)
python prepare_puzzle_image.py photo.jpg --threshold 5

# Less sensitive (only detect very solid borders)
python prepare_puzzle_image.py photo.jpg --threshold 20
```

### Skip Preview Generation

```bash
python prepare_puzzle_image.py photo.jpg --no-preview
```

### RGB888 Format

```bash
python prepare_puzzle_image.py photo.jpg --header --format rgb888
```

## Output Files

### Standard Output

- `{name}_480x480.png` - Final processed image ready for use
- `{name}_preview.png` - Visual preview showing tile grid with numbers

### With --header Flag

- `{name}.h` - C/C++ header file with image data array

## Examples

### Example 1: Simple Photo Processing

```bash
python prepare_puzzle_image.py vacation_photo.jpg
```

**Output:**
- `output/vacation_photo_480x480.png`
- `output/vacation_photo_preview.png`

### Example 2: Create Embedded Image

```bash
python prepare_puzzle_image.py logo.png company_logo --header --tiles 3
```

**Output:**
- `output/company_logo_480x480.png`
- `output/company_logo_preview.png`
- `output/company_logo.h` (can be included in C++ code)

### Example 3: High-Difficulty Puzzle

```bash
python prepare_puzzle_image.py complex_scene.jpg challenge --tiles 6
```

**Output:**
- `output/challenge_480x480.png` (480x480)
- `output/challenge_preview.png` (6x6 grid = 35 tiles)

## Using Generated Images in Game

### Option 1: Load from SD Card / SPIFFS

1. Convert PNG to raw RGB565:
   ```bash
   # Use external tool or modify script
   ```

2. Copy to ESP32 filesystem

3. Load in game code:
   ```cpp
   File f = SPIFFS.open("/puzzle.rgb565", "r");
   tft.drawPngFile(f, 0, 0);
   ```

### Option 2: Embed in Firmware (Small Images)

1. Generate header:
   ```bash
   python prepare_puzzle_image.py image.jpg --header
   ```

2. Include in your code:
   ```cpp
   #include "image.h"

   // Draw from PROGMEM
   for (int i = 0; i < puzzle_image_width * puzzle_image_height; i++) {
       uint16_t color = pgm_read_word(&puzzle_image[i]);
       // Draw pixel...
   }
   ```

### Option 3: Split into Tiles

For better memory management, split the 480x480 image into tiles:

```cpp
// 4x4 grid = 120x120 per tile
#define TILE_SIZE 120

LGFX_Sprite tiles[15];  // 15 tiles (4x4 minus empty)

void loadTiles() {
    for (int i = 0; i < 15; i++) {
        tiles[i].createSprite(TILE_SIZE, TILE_SIZE);

        int row = i / 4;
        int col = i % 4;
        int x = col * TILE_SIZE;
        int y = row * TILE_SIZE;

        // Load and draw tile from source image
        tiles[i].pushImage(0, 0, TILE_SIZE, TILE_SIZE,
                          &source_image[y * 480 + x]);
    }
}
```

## Image Guidelines

### Best Results

- **Format:** JPEG or PNG
- **Minimum Size:** 480x480 (larger is better)
- **Aspect Ratio:** Any (will be center-cropped)
- **Content:** High contrast, clear subjects work best
- **Avoid:** Very dark images, pure text, repetitive patterns

### Recommended Image Types

✅ **Good for Puzzles:**
- Landscapes with distinct features
- Portraits with clear facial features
- Colorful artwork
- Photos with varied colors/textures
- Images with recognizable landmarks

❌ **Challenging:**
- Solid colors or gradients
- Repetitive patterns (tiles, bricks)
- Very dark or very light images
- Abstract art with no clear features
- Text-heavy images

## Troubleshooting

### "No module named 'PIL'"

Install Pillow:
```bash
pip install Pillow
```

### "No module named 'numpy'"

Install numpy:
```bash
pip install numpy
```

### Borders Not Detected

Increase threshold:
```bash
python prepare_puzzle_image.py image.jpg --threshold 20
```

### Image Quality Issues

The script uses Lanczos resampling (highest quality). If you need different quality:
- Edit `Image.Resampling.LANCZOS` in the script
- Options: `NEAREST`, `BILINEAR`, `BICUBIC`, `LANCZOS`

### Preview Numbers Not Visible

The script tries to use Arial font. If not available, it uses default font.
Install Arial or modify the script to use a different font.

## Script Workflow

```
Input Image
    ↓
Detect & Remove Borders
    ↓
Resize (maintain aspect ratio)
    ↓
Center Crop to 480x480
    ↓
Save Final Image
    ↓
Generate Tile Preview (optional)
    ↓
Generate C Header (optional)
```

## Performance Notes

- **Processing Time:** 1-5 seconds per image (depends on size)
- **Memory Usage:** ~50-200 MB during processing
- **Output Size:**
  - 480x480 PNG: ~200-500 KB
  - C header (RGB565): ~900 KB
  - Preview PNG: ~300-600 KB

## License

Free to use and modify for your projects.
