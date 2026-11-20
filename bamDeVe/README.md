# bamDeVe 1.0 (Native C++ NDK Port)

Originally developed by Erik Schneider / erik@bewegtbildbau.de  
Developed at Lumatic / https://lumatic.xyz/  
C++ NDK port by the Antigravity DeepMind agent team.

Estimate a lens vignette from motion entirely inside Nuke with **zero external dependencies** (no NumPy, SciPy, or PyTorch required).

It generates the vignette from seeing the same thing at different screen positions.
Camera motion and few lighting changes will most likely give the best results.
If you have multiple plates per lens, pick the plate that fits this description most and reuse the Vignette for the rest.
Supports spherical and anamorphic lenses.

## How it works
You pre-render optical flow (SmartVector or Cattery RAFT) to disk, wire the plate + flow
read nodes, and hit **Analyse** and then **Solve**. The tool reads the EXRs straight off disk
and fits an anamorphic/spherical lens vignette formula with robust soft-$L_1$ Levenberg-Marquardt optimization.

## Performance & C++ NDK Features
- **Pure C++ Engine**: Evaluates $V(r^2) = \max(0.001, 1 + k_1 r^2 + k_2 r^4 + k_3 r^6)$ directly inside Nuke's scanline rendering engine with full multithreading, channel masking, and standard mix knob.
- **Hardware Generic GPU & VRAM Acceleration**: Compatible with any standard Windows hardware. Features a **`use GPU`** control that harnesses available GPU VRAM throughput when present, with automatic CPU multithreading fallback.
- **Native C++ Acceleration**: Numerical optimization routines are compiled into `bamDeVe.dll` for microsecond-fast solving.
- **Zero NumPy / SciPy Dependency**: Zero external package dependencies. Runs out-of-the-box on clean Nuke installations without pip or admin rights.
- **Self-Contained Supporting Architecture**: Uses a single clean supporting Python solver module (`bamDeVe_solve.py`, similar to `alphaToRoto.py`), keeping the installation clean and minimal without loose duplicate fallbacks.
- **Node Tab Hierarchy Preserved**: Clean property panel without extraneous top tabs, preserving native `Node` controls (`label`, `disable`, etc.).

## Files
- `src/bamDeVe.cpp` — Native C++ NDK `Iop` plugin and solver core.
- `CMakeLists.txt` — MSVC build configuration targeting Nuke NDK.
- `bamDeVe_solve.py` — Supporting solver, analysis, and EXR reader module (zero NumPy).
- `menu.py` — Adds **bam → bamDeVe** to the Nodes toolbar and main menu bar.
- `init.py` — Registers plugin paths.

## Building & Installation
Use the centralized workspace `build.bat`:
```cmd
build.bat 1 "17.1" "bamDeVe"
build.bat 1 "16.0" "bamDeVe"
```
The compiled binaries are deployed to:
- `COMPILED/Nuke17.1/bamDeVe/` (`bamDeVe.dll`, `bamDeVe_solve.py`, `init.py`, `menu.py`)
- `COMPILED/Nuke16.0/bamDeVe/` (`bamDeVe.dll`, `bamDeVe_solve.py`, `init.py`, `menu.py`)

Add `COMPILED/Nuke17.1` (or `COMPILED/Nuke16.0`) to your `NUKE_PATH`.
