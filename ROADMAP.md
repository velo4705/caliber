# Caliber — Milestone Roadmap

## Project Stack
- Language: C++ (C++17)
- GUI Framework: Qt6 (Widgets + Charts + DataVisualization + Network)
- Build System: CMake
- Target Platforms: Linux (first), Windows, macOS

---

## Milestone 1 — Project Scaffold & Basic Calculator ✅
- [x] CMake project setup with Qt6
- [x] Main window with mode switcher (sidebar)
- [x] Basic mode UI: number pad, operators (+, -, *, /)
- [x] Expression parser (recursive descent)
- [x] Display widget (input + result)
- [x] Keyboard input support

---

## Milestone 2 — Scientific Mode ✅
- [x] Trigonometric functions (sin, cos, tan, asin, acos, atan)
- [x] Logarithms (log, ln, log2)
- [x] Power, root, factorial
- [x] Constants (π, e)
- [x] Angle mode toggle (DEG / RAD / GRAD)
- [x] History panel

---

## Milestone 3 — Programming Mode ✅
- [x] Integer arithmetic
- [x] Base display: HEX, DEC, OCT, BIN
- [x] Bitwise operators: AND, OR, XOR, NOT, LSHIFT, RSHIFT
- [x] Bit-width selector: 8, 16, 32, 64-bit
- [x] Byte/bit visualization panel

---

## Milestone 4 — Date Calculation Mode ✅
- [x] Date difference calculator (days, weeks, months, years)
- [x] Add/subtract days from a date
- [x] Day-of-week finder
- [x] Age calculator

---

## Milestone 5 — Conversion Mode ✅
- [x] Local unit conversion (length, mass, temp, speed, area, volume, energy, pressure)
- [x] Currency conversion via live API (open.er-api.com)
- [x] Offline fallback

---

## Milestone 6 — Equations & Matrices ✅
- [x] Linear and quadratic solvers
- [x] System of equations (Gaussian elimination)
- [x] Matrix operations: add, subtract, multiply, transpose, determinant, inverse

---

## Milestone 7 — Graphing Mode ✅
- [x] 2D function plotter with zoom + pan
- [x] Multiple functions, color-coded chips
- [x] Cartesian axes at origin, hairline grid
- [x] 3D surface plotting (Q3DSurface) with rotation
- [x] Height-based gradient coloring on 3D surfaces
- [x] Light/dark chart theme toggle synced to app theme
- [x] Export graph as PNG/JPEG
- [x] Correct operator precedence (-x^2 = -(x^2))
- [x] Radians mode for graphing parser

---

## Milestone 8 — Polish & Themes ✅
- [x] 8 built-in themes: Light, Dark (true black), Midnight, Dracula, Nord, Monokai, Solarized, Follow System
- [x] Custom theme loader (.qss from disk)
- [x] Dynamic portrait/landscape layout
- [x] History panel as slide-in overlay drawer
- [x] Settings persistence (QSettings)
- [x] Keyboard shortcuts (Ctrl+1–7, Ctrl+H)
- [x] App icon and branding

---

## Future — UI/UX Improvements

### Graphing
- [ ] **Trace mode** — hover cursor snaps to the nearest curve and shows (x, y) coordinates in a tooltip
- [ ] **Intersection finder** — detect and mark where two plotted functions cross
- [ ] **Derivative overlay** — toggle to plot f'(x) alongside f(x) automatically
- [ ] **Integral shading** — shade the area under a curve between two x bounds
- [ ] **Polar / parametric mode** — plot r(θ) or (x(t), y(t)) in addition to y(x)
- [ ] **3D color theme sync** — Q3DSurface background and axis colors follow app theme
- [ ] **Animated 3D rotation** — auto-spin toggle so the surface rotates on its own
- [ ] **Multiple 3D surfaces** — plot more than one function at once in 3D

### General UI
- [ ] **Smooth mode transitions** — slide or fade animation when switching between calculator modes
- [ ] **Resizable sidebar** — drag handle to widen or collapse the mode sidebar
- [ ] **Compact mode** — a minimal single-line layout for small windows or embedded use
- [ ] **Font size setting** — user-adjustable display font size in preferences
- [ ] **Accent color picker** — let users pick a single accent color applied across all themes
- [ ] **Onboarding tooltips** — first-run hints pointing out non-obvious features (history drawer, graph pan, etc.)

### History
- [ ] **Search/filter history** — type to filter past calculations in the history panel
- [ ] **Pin entries** — star/pin important results so they stay at the top
- [ ] **Export history** — save history as plain text or CSV

### Accessibility
- [ ] **High contrast theme** — a dedicated WCAG-friendly high contrast option
- [ ] **Keyboard navigation in graphing** — arrow keys to pan, +/- to zoom without mouse

### Platform
- [~] **Windows build + installer** — CMakeLists.txt updated, NSIS script written, `build.bat` one-click script, SVG→ICO converter (`make_ico.py`). Needs a Windows machine to produce the final `Setup.exe`.
- [ ] **macOS build + .dmg packaging** — CMake already cross-platform; needs `macdeployqt`, a `.icns` icon, and a `create-dmg` script.
- [ ] **Wayland-native** — test and fix any X11-specific assumptions (native window containers)

---

## v1.1.0 — Release Packages ✅
- [x] `.deb` package (Ubuntu/Debian) via CPack
- [x] `.rpm` package (Fedora/RHEL) via CPack
- [x] Windows NSIS installer script + `build.bat`
- [x] macOS `.dmg` packaging script
- [x] `LICENSE` file added
- [x] `package/caliber.desktop` for system integration

---

## Dependencies

| Library | Purpose |
|---|---|
| Qt6::Widgets | UI framework |
| Qt6::Charts | 2D graphing |
| Qt6::DataVisualization | 3D surface plotting |
| Qt6::Network | API calls (currency) |
| Qt6::Core | QSettings, QDate, threading |

---

## Current Status
All milestones complete. Active development on Future improvements.
