#!/usr/bin/env python3
"""
Puzzle Image Preparation Tool
Prepares images for ESP32-4848S040C_I Sliding Puzzle Game

Features:
- Detects and removes solid color borders
- Rescales and crops to 480x480
- Generates tile preview grid
- Converts to C/C++ header file format
- Creates visual preview of tiles

Usage:
    python prepare_puzzle_image.py input.jpg [output_name]
    python prepare_puzzle_image.py input.png --preview
    python prepare_puzzle_image.py input.jpg --tiles 4 --format rgb565
"""

import sys
import os
from pathlib import Path
from typing import Tuple, Optional
import argparse

try:
    from PIL import Image, ImageDraw, ImageFont
    import numpy as np
except ImportError:
    print("ERROR: Required libraries not found!")
    print("Install with: pip install Pillow numpy")
    sys.exit(1)


def detect_border_color(img: Image.Image, threshold: int = 10) -> Tuple[int, int, int, int]:
    """
    Detect solid color borders by checking edges.
    Returns (top, bottom, left, right) border sizes in pixels.

    Args:
        img: PIL Image
        threshold: Maximum color variance to consider "solid" (0-255)
    """
    width, height = img.size
    pixels = img.load()

    def is_uniform_row(y: int) -> bool:
        """Check if a row has uniform color."""
        colors = [pixels[x, y] for x in range(width)]
        if len(colors) == 0:
            return False
        avg_color = np.mean(colors, axis=0)
        return all(np.linalg.norm(np.array(c) - avg_color) < threshold for c in colors)

    def is_uniform_col(x: int) -> bool:
        """Check if a column has uniform color."""
        colors = [pixels[x, y] for y in range(height)]
        if len(colors) == 0:
            return False
        avg_color = np.mean(colors, axis=0)
        return all(np.linalg.norm(np.array(c) - avg_color) < threshold for c in colors)

    # Detect top border
    top = 0
    for y in range(height):
        if is_uniform_row(y):
            top = y + 1
        else:
            break

    # Detect bottom border
    bottom = 0
    for y in range(height - 1, -1, -1):
        if is_uniform_row(y):
            bottom = height - y
        else:
            break

    # Detect left border
    left = 0
    for x in range(width):
        if is_uniform_col(x):
            left = x + 1
        else:
            break

    # Detect right border
    right = 0
    for x in range(width - 1, -1, -1):
        if is_uniform_col(x):
            right = width - x
        else:
            break

    return (top, bottom, left, right)


def crop_borders(img: Image.Image, threshold: int = 10) -> Image.Image:
    """Remove solid color borders from image."""
    top, bottom, left, right = detect_border_color(img, threshold)

    if top == 0 and bottom == 0 and left == 0 and right == 0:
        print("  No borders detected")
        return img

    width, height = img.size
    print(f"  Detected borders: top={top}px, bottom={bottom}px, left={left}px, right={right}px")

    # Crop the image
    crop_box = (left, top, width - right, height - bottom)
    return img.crop(crop_box)


def resize_and_crop_center(img: Image.Image, target_size: int = 480) -> Image.Image:
    """
    Resize image to fill target size, then center crop to square.
    Maintains aspect ratio and ensures no black bars.
    """
    width, height = img.size
    aspect = width / height

    # Calculate new dimensions to fill the square
    if aspect > 1:
        # Image is wider than tall
        new_height = target_size
        new_width = int(target_size * aspect)
    else:
        # Image is taller than wide
        new_width = target_size
        new_height = int(target_size / aspect)

    # Resize with high quality
    img_resized = img.resize((new_width, new_height), Image.Resampling.LANCZOS)

    # Center crop to target_size x target_size
    left = (new_width - target_size) // 2
    top = (new_height - target_size) // 2
    right = left + target_size
    bottom = top + target_size

    return img_resized.crop((left, top, right, bottom))


def create_tile_preview(img: Image.Image, grid_size: int = 4, gap: int = 2) -> Image.Image:
    """
    Create a preview showing how the image will be split into tiles.

    Args:
        img: Source image (should be 480x480)
        grid_size: Number of tiles per row/column (default 4 for 4x4)
        gap: Gap between tiles in pixels (default 2)
    """
    size = img.size[0]
    tile_size = size // grid_size

    # Create new image with gaps
    preview_size = size + (gap * (grid_size - 1))
    preview = Image.new('RGB', (preview_size, preview_size), (0, 0, 0))
    draw = ImageDraw.Draw(preview)

    # Draw tiles with gaps
    for row in range(grid_size):
        for col in range(grid_size):
            # Skip bottom-right tile (empty space)
            if row == grid_size - 1 and col == grid_size - 1:
                continue

            # Extract tile from source
            x1 = col * tile_size
            y1 = row * tile_size
            x2 = x1 + tile_size
            y2 = y1 + tile_size
            tile = img.crop((x1, y1, x2, y2))

            # Paste with gap offset
            paste_x = col * (tile_size + gap)
            paste_y = row * (tile_size + gap)
            preview.paste(tile, (paste_x, paste_y))

            # Draw tile number
            tile_num = row * grid_size + col + 1
            try:
                font = ImageFont.truetype("arial.ttf", 24)
            except:
                font = ImageFont.load_default()

            text = str(tile_num)
            # Get text bounding box
            bbox = draw.textbbox((0, 0), text, font=font)
            text_width = bbox[2] - bbox[0]
            text_height = bbox[3] - bbox[1]

            text_x = paste_x + (tile_size - text_width) // 2
            text_y = paste_y + (tile_size - text_height) // 2

            # Draw text with outline for visibility
            for dx, dy in [(-1,-1), (-1,1), (1,-1), (1,1)]:
                draw.text((text_x+dx, text_y+dy), text, font=font, fill=(0, 0, 0))
            draw.text((text_x, text_y), text, font=font, fill=(255, 255, 255))

    return preview


def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    """Convert RGB888 (24-bit) to RGB565 (16-bit)."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def image_to_c_array(img: Image.Image, var_name: str = "puzzle_image",
                     format: str = "rgb565") -> str:
    """
    Convert image to C/C++ array format.

    Args:
        img: PIL Image (should be 480x480 RGB)
        var_name: Variable name for the array
        format: 'rgb565' or 'rgb888'
    """
    width, height = img.size
    pixels = img.load()

    lines = []
    lines.append(f"// Image: {width}x{height}, Format: {format.upper()}")
    lines.append(f"// Generated by prepare_puzzle_image.py")
    lines.append("")

    if format == "rgb565":
        lines.append(f"const uint16_t {var_name}[{width * height}] PROGMEM = {{")

        for y in range(height):
            row_data = []
            for x in range(width):
                r, g, b = pixels[x, y][:3]  # Get RGB, ignore alpha if present
                rgb565 = rgb888_to_rgb565(r, g, b)
                row_data.append(f"0x{rgb565:04X}")

            lines.append("    " + ", ".join(row_data) + ",")

        lines.append("};")

    elif format == "rgb888":
        lines.append(f"const uint8_t {var_name}[{width * height * 3}] PROGMEM = {{")

        for y in range(height):
            row_data = []
            for x in range(width):
                r, g, b = pixels[x, y][:3]
                row_data.extend([f"0x{r:02X}", f"0x{g:02X}", f"0x{b:02X}"])

            lines.append("    " + ", ".join(row_data) + ",")

        lines.append("};")

    else:
        raise ValueError(f"Unknown format: {format}")

    lines.append("")
    lines.append(f"const int {var_name}_width = {width};")
    lines.append(f"const int {var_name}_height = {height};")

    return "\n".join(lines)


def process_image(input_path: str, output_name: Optional[str] = None,
                  target_size: int = 480, grid_size: int = 4,
                  border_threshold: int = 10, generate_preview: bool = True,
                  generate_header: bool = False, header_format: str = "rgb565") -> dict:
    """
    Main processing function.

    Returns:
        dict with paths to generated files
    """
    print(f"Processing: {input_path}")

    # Load image
    try:
        img = Image.open(input_path)
    except Exception as e:
        print(f"ERROR: Could not load image: {e}")
        return {}

    print(f"  Original size: {img.size[0]}x{img.size[1]}")

    # Convert to RGB if needed
    if img.mode != 'RGB':
        print(f"  Converting from {img.mode} to RGB")
        img = img.convert('RGB')

    # Step 1: Remove borders
    print("  Removing borders...")
    img_cropped = crop_borders(img, threshold=border_threshold)
    print(f"  After border removal: {img_cropped.size[0]}x{img_cropped.size[1]}")

    # Step 2: Resize and crop to target size
    print(f"  Resizing and cropping to {target_size}x{target_size}...")
    img_final = resize_and_crop_center(img_cropped, target_size)

    # Determine output name
    if output_name is None:
        input_stem = Path(input_path).stem
        output_name = input_stem

    # Create output directory
    output_dir = Path("output")
    output_dir.mkdir(exist_ok=True)

    results = {}

    # Save final processed image
    final_path = output_dir / f"{output_name}_480x480.png"
    img_final.save(final_path, "PNG", optimize=True)
    print(f"  ✓ Saved: {final_path}")
    results['final'] = str(final_path)

    # Generate tile preview
    if generate_preview:
        print(f"  Generating {grid_size}x{grid_size} tile preview...")
        preview = create_tile_preview(img_final, grid_size)
        preview_path = output_dir / f"{output_name}_preview.png"
        preview.save(preview_path, "PNG", optimize=True)
        print(f"  ✓ Saved: {preview_path}")
        results['preview'] = str(preview_path)

    # Generate C header file
    if generate_header:
        print(f"  Generating C header file ({header_format})...")
        header_content = image_to_c_array(img_final, output_name, header_format)
        header_path = output_dir / f"{output_name}.h"
        with open(header_path, 'w') as f:
            f.write(f"#pragma once\n\n")
            f.write(header_content)
        print(f"  ✓ Saved: {header_path}")
        results['header'] = str(header_path)

    # Print summary
    print("\n" + "="*60)
    print("PROCESSING COMPLETE")
    print("="*60)
    print(f"Final image size: {img_final.size[0]}x{img_final.size[1]}")
    print(f"Tile size: {target_size // grid_size}x{target_size // grid_size}")
    print(f"Grid: {grid_size}x{grid_size} = {grid_size * grid_size} tiles ({grid_size * grid_size - 1} + empty)")
    print("\nGenerated files:")
    for key, path in results.items():
        print(f"  {key}: {path}")

    return results


def main():
    parser = argparse.ArgumentParser(
        description="Prepare images for ESP32 Sliding Puzzle Game",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python prepare_puzzle_image.py photo.jpg
  python prepare_puzzle_image.py photo.jpg my_puzzle
  python prepare_puzzle_image.py photo.jpg --tiles 5 --no-preview
  python prepare_puzzle_image.py photo.jpg --header --format rgb565
  python prepare_puzzle_image.py photo.jpg --threshold 20
        """
    )

    parser.add_argument('input', help='Input image file')
    parser.add_argument('output_name', nargs='?', default=None,
                        help='Output name (default: same as input filename)')
    parser.add_argument('--size', type=int, default=480,
                        help='Target size in pixels (default: 480)')
    parser.add_argument('--tiles', type=int, default=4, choices=[3, 4, 5, 6],
                        help='Grid size (3x3, 4x4, 5x5, 6x6) (default: 4)')
    parser.add_argument('--threshold', type=int, default=10,
                        help='Border detection threshold 0-255 (default: 10)')
    parser.add_argument('--no-preview', action='store_true',
                        help='Skip generating tile preview')
    parser.add_argument('--header', action='store_true',
                        help='Generate C/C++ header file')
    parser.add_argument('--format', choices=['rgb565', 'rgb888'], default='rgb565',
                        help='Header file format (default: rgb565)')

    args = parser.parse_args()

    # Validate input file
    if not os.path.exists(args.input):
        print(f"ERROR: Input file not found: {args.input}")
        sys.exit(1)

    # Process image
    results = process_image(
        input_path=args.input,
        output_name=args.output_name,
        target_size=args.size,
        grid_size=args.tiles,
        border_threshold=args.threshold,
        generate_preview=not args.no_preview,
        generate_header=args.header,
        header_format=args.format
    )

    if not results:
        print("ERROR: Processing failed")
        sys.exit(1)

    print("\nReady to use in your puzzle game!")


if __name__ == "__main__":
    main()
