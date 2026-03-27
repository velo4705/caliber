#!/usr/bin/env bash
# Caliber — iOS package builder
# Produces: Caliber.ipa
#
# Requirements:
#   - macOS with Xcode
#   - Qt6 for iOS installed (e.g. via Qt Online Installer or: brew install qt)
#   - An Apple Developer certificate and provisioning profile (for device builds)
#
# Usage: bash package/build_ios.sh [Qt6 iOS cmake dir]
#   e.g. bash package/build_ios.sh ~/Qt/6.8.0/ios/lib/cmake/Qt6
#   or:  bash package/build_ios.sh $(brew --prefix qt)/lib/cmake/Qt6  (simulator only)
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$REPO/build_ios"
OUT="$REPO/dist"
APP_NAME="Caliber"

QT_CMAKE_DIR="${1:-}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

if [ -z "$QT_CMAKE_DIR" ]; then
    # Try Homebrew
    QT_CMAKE_DIR="$(brew --prefix qt 2>/dev/null)/lib/cmake/Qt6"
fi

if [ ! -d "$QT_CMAKE_DIR" ]; then
    echo "[ERROR] Qt6 cmake dir not found: $QT_CMAKE_DIR"
    echo "  Pass it as argument or install Qt6 for iOS"
    exit 1
fi

echo "==> Configuring (Qt: $QT_CMAKE_DIR)..."
mkdir -p "$BUILD"
cmake -S "$REPO" -B "$BUILD" \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
    -DCMAKE_PREFIX_PATH="$QT_CMAKE_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -GXcode

echo "==> Building..."
cmake --build "$BUILD" --config "$BUILD_TYPE"

# Find the .app
APP_PATH="$BUILD/$BUILD_TYPE-iphoneos/${APP_NAME}.app"
if [ ! -d "$APP_PATH" ]; then
    APP_PATH="$BUILD/${APP_NAME}.app"
fi

if [ ! -d "$APP_PATH" ]; then
    echo "[ERROR] .app not found. Build may have failed."
    exit 1
fi

echo "==> Creating .ipa..."
mkdir -p "$OUT"
PAYLOAD="$BUILD/Payload"
mkdir -p "$PAYLOAD"
cp -R "$APP_PATH" "$PAYLOAD/"
cd "$BUILD"
zip -r "$OUT/${APP_NAME}-2.0.0-iOS.ipa" Payload/

echo ""
echo "Done: $OUT/${APP_NAME}-2.0.0-iOS.ipa"
echo ""
echo "To install on simulator:"
echo "  xcrun simctl install booted '$APP_PATH'"
echo ""
echo "To install on device, sign with your provisioning profile in Xcode."
