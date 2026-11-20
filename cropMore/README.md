# CropMore

**CropMore** is an all-in-one, production-grade compiled C++ NDK Crop plugin for Foundry Nuke (`CropMore.dll`). It delivers native C++ scanline performance while integrating automated bounding box tracking, frame-range animation, aspect-aware padding, reverse cropping, and non-zero pixel auto-detection powered by integrated `GetBounds` C++ planar scanning and Foundry's `CurveTool` AutoCrop.

---

## Architecture & Design Highlights

- **100% Native Compiled C++ NDK (`CropMore.dll`)**: Scanline processing implemented directly on `DD::Image::Iop` for maximum throughput on modern GPUs/CPUs.
- **Zero External Python Dependencies**: All button actions and interactive AutoCrop dialogs are embedded directly within the C++ plugin — no external `crop_more.py` required.
- **Integrated GetBounds Engine**: Peter Mercell's planar bounding box scanner is merged directly into `CropMore.cpp` and compiled into `CropMore.dll`, eliminating any separate `GetBounds.dll` dependency.
- **Integrated CurveTool AutoCrop**: Analyzes non-zero pixel bounds across user-defined frame ranges via integrated CurveTool analysis.
- **MSVC Static Runtime (`/MT`)**: Zero MSVCP/VCRUNTIME DLL runtime dependencies.

---

## Features

1. **All Native Nuke Crop Features**:
   - Presets: `format`, `square`, `4:3`, `16:9`, `14:9`, `1.66:1`, `1.85:1`, `2.35:1`.
   - Dedicated `reset` button to conform the crop box to the selected preset aspect ratio.
   - Interactive viewer bounding box handles (`box`).
   - Softness slider (`softness`) with smoothstep edge falloff.
   - Standard Reformat checkbox (`reformat`).
   - Preconfigured defaults:
     - `intersect`: **True** (prevents expanding beyond incoming geometry).
     - `crop` ("black outside"): **True** (enforces clean 0.0 outside edges with 1-pixel boundary expansion).

2. **Quick Action Header Buttons**:
   - **`format`**: Instantly snaps the crop box to the root project format.
   - **`bBox`**: Sets the crop box to the current frame's incoming bounding box.
   - **`bBox animated`**: Iterates through the specified frame range and writes keyframes on `box` from the incoming bounding box curve.
   - **`autoCrop`**: Interactive popup allowing the user to select the frame range, target channel (defaults to `alpha`), and calculate animated bounding box keyframes using either:
     - **Calculate with GetBounds**: Real-time C++ planar scanning integrated inside `CropMore.dll`.
     - **Calculate with CurveTool**: Integrated Nuke `CurveTool` Auto Crop analysis.

3. **Aspect-Aware Padding**:
   - 2D slider (`padding`) positioned directly below `softness`, operating in pixels with independent X and Y control.
   - Range: `-100` to `100` (supports negative values for inward crop inset).
   - **`keep aspect ratio`** checkbox: When enabled, expanding or shrinking either dimension proportionally scales the other dimension to preserve the exact aspect ratio of the crop box.

4. **Reverse Crop**:
   - Dedicated **`reverse`** toggle on the next line.
   - **`reformat to`** (`target_format` `Format_knob`): Dynamically enabled only when `reverse` is ticked.
   - In reverse mode, CropMore places the cropped image back onto the chosen target canvas format at `(box.x, box.y)`.

---

## Directory Structure

```text
cropMore/
├── CMakeLists.txt         # Single CMake build configuration producing CropMore.dll
├── init.py                # Plugin path registration
├── menu.py                # Node toolbar & menu integration
├── README.md              # Documentation
└── src/
    └── CropMore.cpp       # All-in-one C++ source (CropMore + integrated GetBounds)
```

---

## Compiled Binaries

Binaries are located in the centralized workspace directory:
- `COMPILED/Nuke16.0/cropMore/`
  - `CropMore.dll`
  - `init.py`
  - `menu.py`
- `COMPILED/Nuke17.1/cropMore/`
  - `CropMore.dll`
  - `init.py`
  - `menu.py`

---

## Building from Source (Windows)

To build `CropMore.dll` for a specific Nuke version:

```cmd
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DNUKE_VERSION="17.1v2" -DNDKDIR="C:/Program Files/Nuke17.1v2"
cmake --build build --config Release
```

The resulting `CropMore.dll` will be output to `build/Release/CropMore.dll`.

---

## Credits & Upstream

- **CropMore Node & Automation Architecture**: Custom Workspace Implementation
- **GetBounds Algorithm**: [Peter Mercell (petermercell/GetBounds)](https://github.com/petermercell/GetBounds), originally created by Michael Millspaugh.
