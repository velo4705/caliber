#!/usr/bin/env bash
# Caliber — one-line uninstaller
# Usage: bash uninstall.sh
set -e

echo "==> Uninstalling Caliber (requires sudo)..."

sudo rm -f /usr/local/bin/caliber
sudo rm -f /usr/local/share/icons/hicolor/scalable/apps/caliber.svg
sudo rm -f /usr/local/share/applications/caliber.desktop

command -v gtk-update-icon-cache &>/dev/null && \
    sudo gtk-update-icon-cache -f /usr/local/share/icons/hicolor 2>/dev/null || true

command -v update-desktop-database &>/dev/null && \
    sudo update-desktop-database /usr/local/share/applications 2>/dev/null || true

echo "✓ Caliber uninstalled."
