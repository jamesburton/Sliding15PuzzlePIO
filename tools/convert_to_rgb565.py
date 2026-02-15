#!/usr/bin/env python3
"""
Convert PNG images to raw RGB565 format for ESP32
Reduces file size and eliminates need for PNG decoding
"""

import sys
import os
from pathlib import Path
import struct

try:
    from PIL import Image
    import numpy as np
except ImportError:
    print("ERROR: Required libraries not found!")
    print("Install with: pip install Pillow numpy")
    sys.exit(1)


def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 (24-bit) to RGB565 (16-bit) for ESP32-4848S040C_I hardware."""
    r5 = (r >> 3) & 0x1F  # 5 bits
    g6 = (g >> 2) & 0x3F  # 6 bits
    b5 = (b >> 3) & 0x1F  # 5 bits

    # Standard RGB565 format with byte swap
    # Hardware expects: (r5 << 11) | (g6 << 5) | b5, but bytes swapped
    rgb565 = (r5 << 11) | (g6 << 5) | b5

    # Swap bytes (big-endian to little-endian)
    return ((rgb565 & 0xFF) << 8) | ((rgb565 >> 8) & 0xFF)


def convert_png_to_rgb565(input_path, output_path):
    """Convert PNG image to raw RGB565 binary file."""
    print(f"Converting: {input_path}")

    # Load image
    try:
        img = Image.open(input_path)
    except Exception as e:
        print(f"  ERROR: Could not load image: {e}")
        return False

    # Verify size
    if img.size != (480, 480):
        print(f"  WARNING: Image size is {img.size}, expected (480, 480)")
        print(f"  Resizing...")
        img = img.resize((480, 480), Image.Resampling.LANCZOS)

    # Convert to RGB if needed
    if img.mode != 'RGB':
        print(f"  Converting from {img.mode} to RGB")
        img = img.convert('RGB')

    # Convert to RGB565
    pixels = img.load()
    rgb565_data = bytearray()

    for y in range(480):
        for x in range(480):
            r, g, b = pixels[x, y][:3]  # Get RGB, ignore alpha
            rgb565 = rgb888_to_rgb565(r, g, b)
            # Store as little-endian 16-bit value
            rgb565_data.extend(struct.pack('<H', rgb565))

    # Write to file
    try:
        with open(output_path, 'wb') as f:
            f.write(rgb565_data)

        input_size = os.path.getsize(input_path)
        output_size = os.path.getsize(output_path)
        compression = (1 - output_size / input_size) * 100

        print(f"  [OK] Saved: {output_path}")
        print(f"    Input:  {input_size:,} bytes ({input_size/1024:.1f} KB)")
        print(f"    Output: {output_size:,} bytes ({output_size/1024:.1f} KB)")
        print(f"    Saved:  {compression:+.1f}%")

        return True

    except Exception as e:
        print(f"  ERROR: Could not write file: {e}")
        return False


def convert_directory(input_dir, output_dir):
    """Convert all PNG files in directory."""
    input_path = Path(input_dir)
    output_path = Path(output_dir)

    if not input_path.exists():
        print(f"ERROR: Input directory not found: {input_dir}")
        return 0

    # Create output directory
    output_path.mkdir(parents=True, exist_ok=True)

    # Find all PNG files
    png_files = list(input_path.glob("*.png"))

    if not png_files:
        print(f"No PNG files found in {input_dir}")
        return 0

    print(f"\nProcessing {len(png_files)} files from {input_dir}")
    print("=" * 70)

    success_count = 0
    for png_file in sorted(png_files):
        output_file = output_path / (png_file.stem + ".rgb565")
        if convert_png_to_rgb565(str(png_file), str(output_file)):
            success_count += 1
        print()

    return success_count


def main():
    print("=" * 70)
    print("  PNG to RGB565 Converter for ESP32")
    print("  480x480 Images Only")
    print("=" * 70)
    print()

    # Convert all difficulty levels
    base_input = "images/sliding_puzzle_images_with_originals_480x480_final"
    base_output = "data/puzzles"

    total_converted = 0

    # Easy
    converted = convert_directory(
        f"{base_input}/Easy",
        f"{base_output}/easy"
    )
    total_converted += converted

    # Medium
    converted = convert_directory(
        f"{base_input}/Medium",
        f"{base_output}/medium"
    )
    total_converted += converted

    # Hard
    converted = convert_directory(
        f"{base_input}/Hard",
        f"{base_output}/hard"
    )
    total_converted += converted

    print("=" * 70)
    print(f"CONVERSION COMPLETE")
    print(f"Successfully converted: {total_converted} images")
    print("=" * 70)
    print()
    print("Expected file size per image: 460,800 bytes (450 KB)")
    print(f"Total for {total_converted} images: {total_converted * 450} KB (~{total_converted * 450 / 1024:.1f} MB)")
    print()
    print("Next steps:")
    print("  1. pio run")
    print("  2. pio run --target uploadfs")
    print("  3. pio run --target upload")
    print()


if __name__ == "__main__":
    main()
