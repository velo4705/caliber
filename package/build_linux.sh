#!/usr/bin/env bash
# Caliber — Linux package builder
# Produces: caliber-1.0.0-Linux.deb  and  caliber-1.0.0-Linux.rpm
# Usage: bash package/build_linux.sh [--deb] [--rpm] [--both]
# Default (no args): builds both.
set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$REPO/build_pkg"
OUT="$REPO/dist"

DO_DEB=true
DO_RPM=true
DO_TGZ=true

for arg in "$@"; do
    case "$arg" in
        --deb)  DO_RPM=false; DO_TGZ=false ;;
        --rpm)  DO_DEB=false; DO_TGZ=false ;;
        --tgz)  DO_DEB=false; DO_RPM=false ;;
        --both) ;;
    esac
done

echo "==> Configuring..."
mkdir -p "$BUILD"
cmake -S "$REPO" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    > /dev/null

echo "==> Building..."
cmake --build "$BUILD" --parallel "$(nproc)"

mkdir -p "$OUT"

if $DO_DEB; then
    echo "==> Packaging .deb..."
    (cd "$BUILD" && cpack -G DEB -B "$OUT")
    echo "    -> $(ls "$OUT"/*.deb 2>/dev/null | tail -1)"
fi

if $DO_RPM; then
    if ! command -v rpmbuild &>/dev/null; then
        echo "[SKIP] rpmbuild not found — skipping .rpm"
        echo "       Install with: sudo dnf install rpm-build"
        DO_RPM=false
    else
        echo "==> Packaging .rpm..."
        (cd "$BUILD" && cpack -G RPM -B "$OUT")
        echo "    -> $(ls "$OUT"/*.rpm 2>/dev/null | tail -1)"
    fi
fi

if $DO_TGZ; then
    echo "==> Packaging .tar.gz..."
    (cd "$BUILD" && cpack -G TGZ -B "$OUT")
    echo "    -> $(ls "$OUT"/*.tar.gz 2>/dev/null | tail -1)"
fi

echo ""
echo "Done. Packages in: $OUT/"
ls "$OUT"/*.deb "$OUT"/*.rpm "$OUT"/*.tar.gz 2>/dev/null || true
