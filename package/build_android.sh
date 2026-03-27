#!/usr/bin/env bash
# Caliber — Android package builder
# Produces: caliber.apk
#
# Requirements:
#   - Qt6 for Android installed (e.g. via Qt Online Installer)
#   - Android SDK + NDK (API 34)
#   - ANDROID_SDK_ROOT and ANDROID_NDK_ROOT set
#
# Usage: bash package/build_android.sh [Qt6 Android cmake dir]
#   e.g. bash package/build_android.sh ~/Qt/6.8.0/android_arm64_v8a/lib/cmake/Qt6
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$REPO/build_android"
OUT="$REPO/dist"
APP_NAME="Caliber"

QT_CMAKE_DIR="${1:-}"
ANDROID_API="${ANDROID_API:-34}"
ANDROID_ABIS="${ANDROID_ABIS:-arm64-v8a}"

if [ -z "$QT_CMAKE_DIR" ]; then
    echo "[ERROR] Pass Qt6 Android cmake dir as argument."
    echo "  e.g. bash package/build_android.sh ~/Qt/6.8.0/android_arm64_v8a/lib/cmake/Qt6"
    exit 1
fi

if [ -z "$ANDROID_SDK_ROOT" ]; then
    echo "[ERROR] ANDROID_SDK_ROOT not set."
    exit 1
fi

if [ -z "$ANDROID_NDK_ROOT" ]; then
    echo "[ERROR] ANDROID_NDK_ROOT not set."
    exit 1
fi

echo "==> Configuring (Qt: $QT_CMAKE_DIR)..."
mkdir -p "$BUILD"
cmake -S "$REPO" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
    -DCMAKE_FIND_ROOT_PATH="$QT_CMAKE_DIR/../../" \
    -DANDROID_ABI="$ANDROID_ABIS" \
    -DANDROID_PLATFORM="android-$ANDROID_API" \
    -DCMAKE_PREFIX_PATH="$QT_CMAKE_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    > /dev/null

echo "==> Building..."
cmake --build "$BUILD" --parallel "$(nproc 2>/dev/null || echo 4)"

echo "==> Deploying..."
"$QT_CMAKE_DIR/../../bin/androiddeployqt" \
    --input "$BUILD/android-caliber-deployment-settings.json" \
    --output "$BUILD/android-build" \
    --apk "$BUILD/caliber.apk" \
    --aab

mkdir -p "$OUT"
cp "$BUILD/caliber.apk" "$OUT/"
cp "$BUILD/android-build/build/outputs/bundle/release/android-build-release.aab" "$OUT/caliber.aab" 2>/dev/null || true

echo ""
echo "Done: $OUT/caliber.apk"
[ -f "$OUT/caliber.aab" ] && echo "Done: $OUT/caliber.aab"
