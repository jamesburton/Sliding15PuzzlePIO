# RGB565 Color Format Discovery

## Hardware: ESP32-4848S040C_I (ST7701S Display)

### Problem
Initial image displays showed incorrect colors - reds appeared as greens, color channels were mixed/shifted.

### Investigation Process

1. **Compared with working project (WhackAMolePIO)**
   - WhackAMole uses `drawPng()` → LovyanGFX handles color conversion
   - Our project uses `writePixels()` → requires exact RGB565 format match

2. **Created systematic test pattern**
   - Generated 3x3 color grid (RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, GRAY, BLACK)
   - Tested bit arrangement to identify channel mapping

3. **Generated 8 test formats**
   - Standard RGB565
   - BGR565
   - Various bit reversals
   - Byte-swapped versions

### Solution

**Format 5 (RGB565 + ByteSwap) was correct:**

```python
def rgb888_to_rgb565(r, g, b):
    r5 = (r >> 3) & 0x1F  # 5 bits
    g6 = (g >> 2) & 0x3F  # 6 bits
    b5 = (b >> 3) & 0x1F  # 5 bits

    # Standard RGB565 format
    rgb565 = (r5 << 11) | (g6 << 5) | b5

    # Swap bytes (big-endian ↔ little-endian)
    return ((rgb565 & 0xFF) << 8) | ((rgb565 >> 8) & 0xFF)
```

### Why This Works

The ESP32-4848S040C_I hardware expects:
- **Bit arrangement**: Standard RGB565 → `Red[15:11] | Green[10:5] | Blue[4:0]`
- **Byte order**: Swapped (big-endian format)

This is an endianness issue. When `writePixels()` writes 16-bit values to the display:
1. Without byte swap: `[Low Byte, High Byte]` → Wrong colors
2. With byte swap: `[High Byte, Low Byte]` → Correct colors

### Configuration

**LGFX_Setup.hpp:**
```cpp
cfg.rgb_order = true;  // BGR order flag (misleading but required)
```

**Pin Mapping** (from DisplayConfig.hpp):
- D0-D4: Labeled as "Blue" but hardware interpretation varies
- D5-D10: Labeled as "Green"
- D11-D15: Labeled as "Red"

**Note:** Pin labels in documentation are misleading. The actual color mapping is determined by the RGB565 byte-swapped format, not the pin labels.

### Key Learnings

1. **Never assume pin labels are accurate** - always test with color patterns
2. **`drawPng()` vs `writePixels()`** - PNG decoding handles color format automatically
3. **Endianness matters** - 16-bit color values require byte swapping for this hardware
4. **Systematic testing** - Test grids with primary/secondary/grayscale colors identify issues quickly

### References

- Working project: `c:\Development\PlatformIO\WhackAMolePIO\`
- Test pattern generator: `tools/test_color_formats.py`
- Color test program: `src/main.cpp` (color grid test - backed up)

### Date
2026-02-15
