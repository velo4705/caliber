#!/usr/bin/env bash
# Caliber — Flatpak package builder
# Produces: dist/com.caliber.app.flatpak
# Requirements:
#   flatpak-builder, org.kde.Platform//6.7, org.kde.Sdk//6.7
#
# Install requirements:
#   sudo dnf install flatpak-builder          # Fedora
#   sudo apt install flatpak-builder          # Ubuntu
#   flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
#   flatpak install flathub org.kde.Platform//6.7 org.kde.Sdk//6.7
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$REPO/dist"
BUILD_DIR="$REPO/build_flatpak"
REPO_DIR="$REPO/build_flatpak_repo"
APP_ID="com.caliber.app"

mkdir -p "$OUT"

echo "==> Building Flatpak..."
flatpak-builder \
    --force-clean \
    --repo="$REPO_DIR" \
    "$BUILD_DIR" \
    "$REPO/package/com.caliber.app.json"

echo "==> Bundling .flatpak..."
flatpak build-bundle \
    "$REPO_DIR" \
    "$OUT/${APP_ID}-1.1.0.flatpak" \
    "$APP_ID"

echo ""
echo "Done: $OUT/${APP_ID}-1.1.0.flatpak"
echo ""
echo "Install with:"
echo "  flatpak install --user $OUT/${APP_ID}-1.1.0.flatpak"
