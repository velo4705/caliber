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
HOST_QT="$HOME/Qt/6.7.3/gcc_64"

if [ -z "$QT_CMAKE_DIR" ]; then
    echo "[ERROR] Pass Qt6 Android cmake dir as argument."
    echo "  e.g. bash package/build_android.sh ~/Qt/6.7.3/android_arm64_v8a/lib/cmake/Qt6"
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

# Qt 6.7 needs JDK 17 — check JAVA_HOME or fallback to system java
if [ -n "$JAVA_HOME" ] && [ -f "$JAVA_HOME/bin/java" ]; then
    echo "Using JDK: $JAVA_HOME"
else
    echo "[WARN] JAVA_HOME not set. Qt 6.7 requires JDK 17 (JDK 25+ will fail)."
    echo "       Set JAVA_HOME to JDK 17, e.g.:"
    echo "       export JAVA_HOME=~/jdk-17.0.13+11"
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
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH \
    -DANDROID_ABI="$ANDROID_ABIS" \
    -DANDROID_PLATFORM="android-$ANDROID_API" \
    -DCMAKE_PREFIX_PATH="$QT_CMAKE_DIR" \
    -DQT_HOST_PATH="$HOME/Qt/6.7.3/gcc_64" \
    -DQT_HOST_PATH_CMAKE_DIR="$HOME/Qt/6.7.3/gcc_64/lib/cmake" \
    -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
    -DCMAKE_BUILD_TYPE=Release

echo "==> Building..."
cmake --build "$BUILD" --target caliber --parallel "$(nproc 2>/dev/null || echo 4)"

echo "==> Deploying..."
mkdir -p "$BUILD/android-build/libs/$ANDROID_ABIS"
cp "$BUILD/libcaliber_arm64-v8a.so" "$BUILD/android-build/libs/$ANDROID_ABIS/"

"$HOST_QT/bin/androiddeployqt" \
    --input "$BUILD/android-caliber-deployment-settings.json" \
    --output "$BUILD/android-build" \
    --apk "$BUILD/Caliber.apk"

mkdir -p "$OUT"
cp "$BUILD/Caliber.apk" "$OUT/"

echo ""
echo "Done: $OUT/Caliber.apk"
