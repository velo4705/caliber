# Caliber

A graphical calculator built with C++ and Qt6. Covers every calculation mode you'd need — from basic arithmetic to function graphing — in a clean, themeable interface.

![Caliber](resources/icons/caliber.svg)

---

## Screenshots

| | |
|---|---|
| ![Basic](screenshots/basic.png) | ![Scientific](screenshots/scientific.png) |
| ![Programming](screenshots/programming.png) | ![Date](screenshots/datecalc.png) |
| ![Conversion](screenshots/conversion.png) | ![Equations](screenshots/equations.png) |
| ![Graphing 2D](screenshots/graph2d.png) | ![Graphing 3D](screenshots/graph3d.png) |

---

## Modes

| Mode | Description |
|---|---|
| Basic | Arithmetic with live expression preview |
| Scientific | Trig, logarithms, powers, constants (π, e), DEG/RAD/GRAD |
| Programming | HEX/DEC/OCT/BIN, bitwise ops, 8–64 bit width, bit visualizer |
| Date | Date difference, add/subtract durations, age calculator, day of week |
| Conversion | 13 unit categories + live currency conversion via API |
| Equations | Linear, quadratic, system solver (Gaussian), matrix operations |
| Graphing | 2D function plotter, multiple functions, zoom/pan, export to image |

---

## Installation

Installations are currently made for Linux Only. Windows and MacOS are Coming Soon.

### One-line install (Linux)

```bash
bash <(curl -fsSL https://raw.githubusercontent.com/imloafy/caliber/main/install.sh)
```

Or if you've already cloned the repo:

```bash
bash install.sh
```

This builds from source, installs the binary to `/usr/local/bin/caliber`, adds a `.desktop` entry (shows up in your app launcher), and registers the icon.

### Uninstall

```bash
bash uninstall.sh
```

---

## Building

### Requirements

- Qt6 (Widgets, Charts, Network)
- CMake 3.16+
- C++17 compiler

### Linux (Fedora/Ultramarine)

```bash
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install qt6-base-dev qt6-charts-dev
```

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./caliber
```

---

## Features

- 8 built-in themes: Light, Dark, Midnight Blue, Dracula, Nord, Monokai, Solarized Dark, Follow System
- Custom theme support — load any `.qss` file from disk via View → Theme → Load Custom Theme
- Settings persistence — remembers window size, last mode, and theme across restarts
- Keyboard shortcuts — `Ctrl+1` through `Ctrl+7` to switch modes instantly
- History panel — click any past calculation to restore it
- Currency conversion — fetches live rates from [open.er-api.com](https://open.er-api.com), caches offline
- Graph export — save plots as PNG or JPEG

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+1` – `Ctrl+7` | Switch to mode 1–7 |
| `Ctrl+H` | Toggle history panel |
| `Ctrl+Shift+S` | Theme: Follow System |
| `Ctrl+Shift+L` | Theme: Light |
| `Ctrl+Shift+D` | Theme: Dark |
| `Enter` | Calculate |
| `Backspace` | Delete last character |
| `Escape` | Clear |

---

## Stack

- C++17
- Qt6 (Widgets · Charts · Network)
- CMake
