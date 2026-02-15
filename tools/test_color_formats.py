#!/usr/bin/env python3
"""
Generate puppy image in multiple RGB565 formats for systematic testing
"""

import sys
import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("ERROR: PIL not found! Install with: pip install Pillow")
    sys.exit(1)


# Define different RGB565 conversion formats to test
FORMATS = {
    # Standard formats
    "fmt1_rgb565_std": lambda r5, g6, b5: (r5 << 11) | (g6 << 5) | b5,
    "fmt2_bgr565_std": lambda r5, g6, b5: (b5 << 11) | (g6 << 5) | r5,

    # Reversed positions (what we tried)
    "fmt3_grb_current": lambda r5, g6, b5: g6 | (r5 << 6) | (b5 << 11),
    "fmt4_brg": lambda r5, g6, b5: b5 | (r5 << 5) | (g6 << 11),
    "fmt5_gbr": lambda r5, g6, b5: g6 | (b5 << 6) | (r5 << 11),
    "fmt6_rbg": lambda r5, g6, b5: r5 | (b5 << 5) | (g6 << 11),

    # Byte swapped versions of standard
    "fmt7_rgb565_swapped": lambda r5, g6, b5: swap_bytes((r5 << 11) | (g6 << 5) | b5),
    "fmt8_bgr565_swapped": lambda r5, g6, b5: swap_bytes((b5 << 11) | (g6 << 5) | r5),

    # Different bit arrangements
    "fmt9_rgb_reversed": lambda r5, g6, b5: r5 | (g6 << 5) | (b5 << 11),
    "fmt10_bgr_reversed": lambda r5, g6, b5: b5 | (g6 << 5) | (r5 << 11),
}


def swap_bytes(val):
    """Swap bytes in 16-bit value"""
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF)


def convert_image(input_path, output_dir, format_name, convert_func):
    """Convert image using specific format function"""
    print(f"\nConverting with {format_name}...")

    img = Image.open(input_path)

    if img.size != (480, 480):
        print(f"  Resizing from {img.size} to (480, 480)")
        img = img.resize((480, 480), Image.Resampling.LANCZOS)

    if img.mode != 'RGB':
        print(f"  Converting from {img.mode} to RGB")
        img = img.convert('RGB')

    pixels = img.load()
    rgb565_data = bytearray()

    for y in range(480):
        for x in range(480):
            r, g, b = pixels[x, y][:3]

            # Convert to 5-6-5 bit components
            r5 = (r >> 3) & 0x1F  # 5 bits
            g6 = (g >> 2) & 0x3F  # 6 bits
            b5 = (b >> 3) & 0x1F  # 5 bits

            # Apply format function
            rgb565 = convert_func(r5, g6, b5)

            # Store as little-endian 16-bit value
            rgb565_data.extend(struct.pack('<H', rgb565))

    output_file = output_dir / f"{format_name}.rgb565"
    with open(output_file, 'wb') as f:
        f.write(rgb565_data)

    print(f"  Saved: {output_file} ({len(rgb565_data)} bytes)")
    return str(output_file)


def main():
    input_file = "images/sliding_puzzle_images_with_originals_480x480_final/Easy/puppy_in_toy_car.png"
    output_dir = Path("data/test_formats")

    output_dir.mkdir(parents=True, exist_ok=True)

    print("=" * 70)
    print("RGB565 Format Testing - Puppy Image")
    print("=" * 70)

    if not Path(input_file).exists():
        print(f"ERROR: Input file not found: {input_file}")
        return 1

    results = []
    for fmt_name, fmt_func in FORMATS.items():
        try:
            output = convert_image(input_file, output_dir, fmt_name, fmt_func)
            results.append(fmt_name)
        except Exception as e:
            print(f"  ERROR: {e}")

    print("\n" + "=" * 70)
    print("CONVERSION COMPLETE")
    print("=" * 70)
    print(f"\nGenerated {len(results)} test formats:")
    for i, fmt in enumerate(results, 1):
        print(f"  {i:2d}. {fmt}")

    print("\n" + "=" * 70)
    print("TESTING INSTRUCTIONS:")
    print("=" * 70)
    print("1. Copy these files to data/puzzles/easy/")
    print("2. Modify PuzzleManager.hpp to load test formats")
    print("3. Upload filesystem and firmware")
    print("4. Touch screen to cycle through formats")
    print("5. Note which format shows correct colors")
    print("=" * 70)

    return 0


if __name__ == "__main__":
    sys.exit(main())
