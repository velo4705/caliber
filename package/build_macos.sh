#!/usr/bin/env bash
# Caliber — macOS package builder
# Produces: Caliber-1.0.0.dmg
# Requirements:
#   - Qt6 installed (e.g. via Homebrew: brew install qt)
#   - create-dmg: brew install create-dmg
#   - Xcode Command Line Tools
# Usage: bash package/build_macos.sh [Qt6 cmake dir]
#   e.g. bash package/build_macos.sh /opt/homebrew/opt/qt/lib/cmake/Qt6
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$REPO/build_mac"
OUT="$REPO/dist"
APP_NAME="Caliber"
VERSION="1.0.0"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"

QT_CMAKE_DIR="${1:-$(brew --prefix qt 2>/dev/null)/lib/cmake/Qt6}"

if [ ! -d "$QT_CMAKE_DIR" ]; then
    echo "[ERROR] Qt6 cmake dir not found: $QT_CMAKE_DIR"
    echo "        Pass it as argument or install Qt via: brew install qt"
    exit 1
fi

echo "==> Configuring (Qt: $QT_CMAKE_DIR)..."
mkdir -p "$BUILD"
cmake -S "$REPO" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_CMAKE_DIR" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    > /dev/null

echo "==> Building..."
cmake --build "$BUILD" --parallel "$(sysctl -n hw.logicalcpu)"

# ── Create .app bundle ────────────────────────────────────────────────────────
APP_DIR="$BUILD/${APP_NAME}.app"
mkdir -p "$APP_DIR/Contents/MacOS"
mkdir -p "$APP_DIR/Contents/Resources"

cp "$BUILD/caliber" "$APP_DIR/Contents/MacOS/${APP_NAME}"

# Generate Info.plist
cat > "$APP_DIR/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
    "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>             <string>${APP_NAME}</string>
    <key>CFBundleDisplayName</key>      <string>${APP_NAME}</string>
    <key>CFBundleIdentifier</key>       <string>com.caliber.app</string>
    <key>CFBundleVersion</key>          <string>${VERSION}</string>
    <key>CFBundleShortVersionString</key><string>${VERSION}</string>
    <key>CFBundleExecutable</key>       <string>${APP_NAME}</string>
    <key>CFBundleIconFile</key>         <string>caliber</string>
    <key>CFBundlePackageType</key>      <string>APPL</string>
    <key>NSHighResolutionCapable</key>  <true/>
    <key>NSRequiresAquaSystemAppearance</key><false/>
</dict>
</plist>
PLIST

# Convert SVG icon to .icns if possible
ICNS="$APP_DIR/Contents/Resources/caliber.icns"
SVG="$REPO/resources/icons/caliber.svg"
if command -v rsvg-convert &>/dev/null && command -v iconutil &>/dev/null; then
    echo "==> Generating .icns..."
    ICONSET="$BUILD/caliber.iconset"
    mkdir -p "$ICONSET"
    for SIZE in 16 32 64 128 256 512; do
        rsvg-convert -w $SIZE -h $SIZE "$SVG" -o "$ICONSET/icon_${SIZE}x${SIZE}.png"
        rsvg-convert -w $((SIZE*2)) -h $((SIZE*2)) "$SVG" -o "$ICONSET/icon_${SIZE}x${SIZE}@2x.png"
    done
    iconutil -c icns "$ICONSET" -o "$ICNS"
else
    echo "[WARN] rsvg-convert or iconutil not found — skipping .icns (install: brew install librsvg)"
fi

# ── macdeployqt ───────────────────────────────────────────────────────────────
MACDEPLOYQT="$(dirname "$QT_CMAKE_DIR")/../../bin/macdeployqt"
if [ ! -f "$MACDEPLOYQT" ]; then
    MACDEPLOYQT="$(brew --prefix qt)/bin/macdeployqt"
fi

echo "==> Running macdeployqt..."
"$MACDEPLOYQT" "$APP_DIR" -no-strip

# ── Build .dmg ────────────────────────────────────────────────────────────────
mkdir -p "$OUT"
DMG_PATH="$OUT/$DMG_NAME"

if command -v create-dmg &>/dev/null; then
    echo "==> Building .dmg..."
    create-dmg \
        --volname "$APP_NAME" \
        --window-pos 200 120 \
        --window-size 600 400 \
        --icon-size 100 \
        --icon "${APP_NAME}.app" 175 190 \
        --hide-extension "${APP_NAME}.app" \
        --app-drop-link 425 190 \
        "$DMG_PATH" \
        "$APP_DIR"
    echo "    -> $DMG_PATH"
else
    echo "[WARN] create-dmg not found. Install with: brew install create-dmg"
    echo "       Falling back to hdiutil..."
    hdiutil create -volname "$APP_NAME" -srcfolder "$APP_DIR" \
        -ov -format UDZO "$DMG_PATH"
    echo "    -> $DMG_PATH"
fi

echo ""
echo "Done: $DMG_PATH"
