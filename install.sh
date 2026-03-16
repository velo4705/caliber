#!/usr/bin/env bash
# Caliber — one-line installer
# Usage: bash install.sh
set -e

INSTALL_BIN="/usr/local/bin/caliber"
INSTALL_ICON="/usr/local/share/icons/hicolor/scalable/apps/caliber.svg"
INSTALL_DESKTOP="/usr/local/share/applications/caliber.desktop"
BUILD_DIR="$(dirname "$0")/build"

echo "==> Building Caliber..."
mkdir -p "$BUILD_DIR"
cmake -S "$(dirname "$0")" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local > /dev/null
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

echo "==> Installing (requires sudo)..."
sudo install -Dm755 "$BUILD_DIR/caliber"        "$INSTALL_BIN"
sudo install -Dm644 "$(dirname "$0")/resources/icons/caliber.svg" "$INSTALL_ICON"

sudo tee "$INSTALL_DESKTOP" > /dev/null <<EOF
[Desktop Entry]
Name=Caliber
Comment=Graphical calculator — Basic, Scientific, Programming, Graphing and more
Exec=caliber
Icon=caliber
Terminal=false
Type=Application
Categories=Utility;Calculator;
Keywords=calculator;math;graphing;scientific;programming;
StartupWMClass=caliber
EOF

sudo chmod 644 "$INSTALL_DESKTOP"

# Refresh icon cache if possible
command -v gtk-update-icon-cache &>/dev/null && \
    sudo gtk-update-icon-cache -f /usr/local/share/icons/hicolor 2>/dev/null || true

command -v update-desktop-database &>/dev/null && \
    sudo update-desktop-database /usr/local/share/applications 2>/dev/null || true

echo ""
echo "✓ Caliber installed successfully."
echo "  Binary : $INSTALL_BIN"
echo "  Desktop: $INSTALL_DESKTOP"
echo "  Run with: caliber"
