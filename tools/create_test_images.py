#!/usr/bin/env python3
"""
Create test images for demonstrating the puzzle image preparation tool.
Generates colorful test patterns with borders.
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_gradient_image(size=(800, 600), with_border=True):
    """Create a gradient test image."""
    img = Image.new('RGB', size)
    draw = ImageDraw.Draw(img)

    # Draw gradient
    for y in range(size[1]):
        for x in range(size[0]):
            r = int(255 * x / size[0])
            g = int(255 * y / size[1])
            b = int(255 * (1 - x / size[0]) * (1 - y / size[1]))
            draw.point((x, y), (r, g, b))

    if with_border:
        # Add white border
        border_width = 50
        draw.rectangle([0, 0, size[0]-1, border_width], fill=(255, 255, 255))
        draw.rectangle([0, size[1]-border_width, size[0]-1, size[1]-1], fill=(255, 255, 255))
        draw.rectangle([0, 0, border_width, size[1]-1], fill=(255, 255, 255))
        draw.rectangle([size[0]-border_width, 0, size[0]-1, size[1]-1], fill=(255, 255, 255))

    return img


def create_colorful_pattern(size=(800, 600), with_border=True):
    """Create a colorful geometric pattern."""
    img = Image.new('RGB', size)
    draw = ImageDraw.Draw(img)

    # Background
    draw.rectangle([0, 0, size[0], size[1]], fill=(20, 20, 40))

    # Draw colorful shapes
    colors = [
        (255, 100, 100),  # Red
        (100, 255, 100),  # Green
        (100, 100, 255),  # Blue
        (255, 255, 100),  # Yellow
        (255, 100, 255),  # Magenta
        (100, 255, 255),  # Cyan
    ]

    step = size[0] // 6
    for i, color in enumerate(colors):
        x = i * step
        y = 100 + (i % 3) * 100
        draw.ellipse([x+20, y, x+step-20, y+150], fill=color, outline=(255, 255, 255), width=3)

    # Add some geometric shapes
    for i in range(5):
        x = 100 + i * 140
        y = 400
        points = []
        for j in range(5):
            import math
            angle = j * 2 * math.pi / 5
            px = x + 50 * math.cos(angle)
            py = y + 50 * math.sin(angle)
            points.append((px, py))
        draw.polygon(points, fill=colors[i], outline=(255, 255, 255), width=2)

    # Add text
    try:
        font = ImageFont.truetype("arial.ttf", 60)
    except:
        font = ImageFont.load_default()

    text = "TEST PUZZLE"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_x = (size[0] - text_width) // 2

    draw.text((text_x, 30), text, font=font, fill=(255, 255, 255))

    if with_border:
        # Add black border
        border_width = 40
        draw.rectangle([0, 0, size[0]-1, border_width], fill=(0, 0, 0))
        draw.rectangle([0, size[1]-border_width, size[0]-1, size[1]-1], fill=(0, 0, 0))
        draw.rectangle([0, 0, border_width, size[1]-1], fill=(0, 0, 0))
        draw.rectangle([size[0]-border_width, 0, size[0]-1, size[1]-1], fill=(0, 0, 0))

    return img


def create_photo_mockup(size=(1000, 750), with_border=True):
    """Create a mockup that looks like a photo with frame."""
    img = Image.new('RGB', size)
    draw = ImageDraw.Draw(img)

    # Sky gradient
    for y in range(size[1] // 2):
        blue = int(100 + 155 * (1 - y / (size[1] // 2)))
        draw.rectangle([0, y, size[0], y+1], fill=(135, 206, blue))

    # Ground gradient
    for y in range(size[1] // 2, size[1]):
        green = int(50 + 100 * (y - size[1]//2) / (size[1] // 2))
        draw.rectangle([0, y, size[0], y+1], fill=(34, green, 34))

    # Sun
    draw.ellipse([size[0]-200, 50, size[0]-50, 200], fill=(255, 255, 100))

    # Mountains
    points = [
        (0, size[1]//2),
        (150, size[1]//2 - 100),
        (300, size[1]//2),
        (0, size[1]//2)
    ]
    draw.polygon(points, fill=(100, 100, 100))

    points = [
        (200, size[1]//2),
        (400, size[1]//2 - 150),
        (600, size[1]//2),
        (200, size[1]//2)
    ]
    draw.polygon(points, fill=(80, 80, 80))

    # Trees
    for i in range(10):
        x = 50 + i * 100
        y = size[1] // 2 + 50
        # Trunk
        draw.rectangle([x-5, y, x+5, y+80], fill=(101, 67, 33))
        # Foliage
        draw.ellipse([x-30, y-30, x+30, y+30], fill=(34, 139, 34))

    # Add label
    try:
        font = ImageFont.truetype("arial.ttf", 40)
    except:
        font = ImageFont.load_default()

    draw.text((size[0]//2 - 100, size[1] - 100), "LANDSCAPE", font=font, fill=(255, 255, 255))

    if with_border:
        # Add thick gray frame border
        border_width = 60
        draw.rectangle([0, 0, size[0]-1, border_width], fill=(180, 180, 180))
        draw.rectangle([0, size[1]-border_width, size[0]-1, size[1]-1], fill=(180, 180, 180))
        draw.rectangle([0, 0, border_width, size[1]-1], fill=(180, 180, 180))
        draw.rectangle([size[0]-border_width, 0, size[0]-1, size[1]-1], fill=(180, 180, 180))

    return img


def main():
    """Generate test images."""
    print("Creating test images...")

    os.makedirs("test_images", exist_ok=True)

    # Create images with borders
    print("  Creating gradient_with_border.png...")
    img1 = create_gradient_image(with_border=True)
    img1.save("test_images/gradient_with_border.png")

    print("  Creating pattern_with_border.png...")
    img2 = create_colorful_pattern(with_border=True)
    img2.save("test_images/pattern_with_border.png")

    print("  Creating landscape_with_border.png...")
    img3 = create_photo_mockup(with_border=True)
    img3.save("test_images/landscape_with_border.png")

    # Create images without borders
    print("  Creating gradient_no_border.png...")
    img4 = create_gradient_image(with_border=False)
    img4.save("test_images/gradient_no_border.png")

    print("  Creating pattern_no_border.png...")
    img5 = create_colorful_pattern(with_border=False)
    img5.save("test_images/pattern_no_border.png")

    print("\n✓ Test images created in 'test_images/' folder")
    print("\nYou can now test the tool with:")
    print("  python prepare_puzzle_image.py test_images/gradient_with_border.png")
    print("  python prepare_puzzle_image.py test_images/pattern_with_border.png")
    print("  python prepare_puzzle_image.py test_images/landscape_with_border.png")


if __name__ == "__main__":
    main()
