# USDSubD

A Nuke GeomOp plugin that bakes Catmull-Clark subdivision into USD meshes using OpenSubdiv.

![Nuke](https://img.shields.io/badge/Nuke-16.0-yellow)
![License](https://img.shields.io/badge/license-Apache--2.0-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)

## Features

- **Catmull-Clark subdivision** via OpenSubdiv on USD mesh primitives
- **UV-aware** — UVs are promoted to face-varying interpolation to prevent seam tearing
- **Selective processing** — optionally subdivide only meshes tagged with a subdivision scheme
- **Progress & cancellation** — reports progress to Nuke's UI and respects abort signals
- **Warnings** — alerts when meshes are missing UV primvars

## Requirements

| Dependency | Version |
|---|---|
| Nuke (NDK) | 16.0+ |
| CMake | 3.16+ |
| C++ compiler | C++17 support |

The plugin links against Nuke's bundled USD (USG) and OpenSubdiv libraries — no separate USD build is needed.

## Building

### 1. Clone

```bash
git clone https://github.com/<your-username>/USDSubD.git
cd USDSubD
```

### 2. Configure

Point CMake at your Nuke installation:

```bash
# Linux
cmake -B build -DNUKE_INSTALL_DIR=/usr/local/Nuke16.0v8

# Windows (Visual Studio)
cmake -B build -G "Visual Studio 17 2022" -DNUKE_INSTALL_DIR="C:/Program Files/Nuke16.0v8"
```

Or set the environment variable instead:

```bash
export NUKE_INSTALL_DIR=/usr/local/Nuke16.0v8
cmake -B build
```

### 3. Build

```bash
cmake --build build --config Release
```

### 4. Install

```bash
cmake --install build --config Release
```

By default this copies the plugin into `~/.nuke` so Nuke picks it up on next launch. Override with `-DCMAKE_INSTALL_PREFIX=/your/path`.

## Usage

1. Open Nuke and create a 3D scene with USD geometry.
2. Add the **USDSubD** node after your geometry source.
3. Set **Iterations** (1–3 recommended; 4+ is very heavy).
4. Optionally enable **Only subd tagged meshes** to skip meshes whose `subdivisionScheme` is `none`.

## How It Works

The engine traverses the USD stage, collects eligible `MeshPrim` objects, flattens their UV data into face-varying layout, and calls `usg::subdivideUniform` (Catmull-Clark) for the requested number of iterations. After subdivision the mesh topology, points, and UVs are written back to the stage.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is licensed under the Apache License 2.0 — see [LICENSE](LICENSE) for details.

## Author

Created by **Marten Blumen**.
