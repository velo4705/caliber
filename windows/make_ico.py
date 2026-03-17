#!/usr/bin/env python3
"""
Convert caliber.svg → caliber.ico (multi-resolution Windows icon).

Requirements:
    pip install cairosvg Pillow

Usage:
    python windows/make_ico.py
"""

import sys
import os

try:
    import cairosvg
    from PIL import Image
    import io
except ImportError:
    print("Missing dependencies. Run:  pip install cairosvg Pillow")
    sys.exit(1)

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SVG_PATH  = os.path.join(REPO_ROOT, "resources", "icons", "caliber.svg")
ICO_PATH  = os.path.join(os.path.dirname(os.path.abspath(__file__)), "caliber.ico")

SIZES = [16, 24, 32, 48, 64, 128, 256]

def main():
    if not os.path.exists(SVG_PATH):
        print(f"SVG not found: {SVG_PATH}")
        sys.exit(1)

    images = []
    for size in SIZES:
        png_data = cairosvg.svg2png(url=SVG_PATH, output_width=size, output_height=size)
        img = Image.open(io.BytesIO(png_data)).convert("RGBA")
        images.append(img)
        print(f"  Rendered {size}x{size}")

    # Export each size as a temporary PNG, then combine with icotool
    import subprocess
    import tempfile

    tmpdir = tempfile.mkdtemp()
    png_files = []
    for img, size in zip(images, SIZES):
        resized = img.resize((size, size), Image.LANCZOS)
        png_path = os.path.join(tmpdir, f"icon_{size}.png")
        resized.save(png_path, format="PNG")
        png_files.append(png_path)
        print(f"  Exported {size}x{size} PNG")

    # Use icotool to combine all PNGs into a proper multi-size .ico
    result = subprocess.run(
        ["icotool", "--create", "--output", ICO_PATH] + png_files,
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print(f"icotool error: {result.stderr}")
        sys.exit(1)

    # Cleanup temp files
    for f in png_files:
        os.remove(f)
    os.rmdir(tmpdir)

    print(f"\nSaved: {ICO_PATH}")

if __name__ == "__main__":
    main()
