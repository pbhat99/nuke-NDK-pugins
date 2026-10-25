# Nuke Plugin Builds

A curated collection of high-performance **Nuke NDK (C++) and Python plugins** ported, optimized, and built for Windows and modern Foundry Nuke (Nuke 17+ / VFX Reference Platform CY2024).

---

## Included Plugins & Upstream Links

| Tool / Plugin | Description | Upstream Repository |
|---|---|---|
| **[AlphaToRoto](AlphaToRoto/)** | Potrace-powered alpha contour vectorization with automated keyframed animated Roto shape generation. | [petermercell/AlphaToRoto](https://github.com/petermercell/AlphaToRoto) |
| **[ColorGrade](colorgrade/)** | NDK ColorGrade operator with black/white points, lift, gain, multiply, offset, and gamma. | In-House / Workspace |
| **[DeepC](DeepC/)** | 31+ volumetric deep compositing plugins including DeepThinner, DeepCAdd, DeepCExpression, and DeepCPNoise. | [charlesangus/DeepC](https://github.com/charlesangus/DeepC) |
| **[DeepErode](DeepErode/)** | Morphological erosion and dilation for deep sample data. | In-House / Workspace |
| **[FastNoiseLite](FastNoiseLite/)** | Procedural multi-fractal noise generator node (Perlin, Simplex, Cellular, ValueCubic, PingPong). | [Auburn/FastNoiseLite](https://github.com/Auburn/FastNoiseLite) |
| **[ldpk](ldpk/)** | 3DEqualizer Lens Distortion Plugin Kit (18 distortion models including Anamorphic, Radial, Fisheye, and OpenCV). | Science.D.Visions LDPK |
| **[NukeShenanigans](NukeShenanigans/)** | Production utility suite including wildcard/regex channel-removing `RemoveUnlimited`, `ContactSheetAll`, and deep operators. | [zeemzoet/NukeShenanigans](https://github.com/zeemzoet/NukeShenanigans) |
| **[ODIN Denoiser](ODIN%20Denoiser/)** | Intel Open Image Denoise (OIDN) deep learning image denoiser with albedo and normal guide buffers. | [twins-annning/NukeOIDNDenoise](https://github.com/twins-annning/NukeOIDNDenoise) |
| **[OptixDenoiser](OptixDenoiser/)** | NVIDIA OptiX AI image denoiser with RTX hardware acceleration and motion vector support. | [petermercell/OptiXDenoiser](https://github.com/petermercell/OptiXDenoiser) |
| **[rotoSimplifier](rotoSimplifier/)** | Fast Ramer-Douglas-Peucker spline simplification node and companion interactive PySide GUI. | [bratgot/RotoSimplifier](https://github.com/bratgot/RotoSimplifier) |
| **[USDsubD](USDsubD/)** | OpenSubdiv Catmull-Clark and Loop mesh subdivision for Nuke 17 USD Stage geometry. | [bratgot/USDSubD](https://github.com/bratgot/USDSubD) |

---

## Directory Structure

```text
nuke-plugin-builds/
├── AGENTS.md                  # Workspace build instructions & rules
├── README.md                  # Project overview and upstream directory
├── .gitignore                 # Excludes build caches, object files, and temporary outputs
├── COMPILED/                  # Precompiled binaries (managed manually)
│   ├── init.py                # Root version router (detects running Nuke version)
│   └── Nuke17.1/              # Nuke 17.1 production build
│       ├── init.py            # Dynamic tool discovery & Win32 DLL loader
│       ├── AlphaToRoto/
│       ├── ColorGrade/
│       ├── DeepC/
│       ├── DeepErode/
│       ├── FastNoiseLite/
│       ├── ldpk/
│       ├── NukeShenanigans/
│       ├── oidnDenoise/
│       ├── OptixDenoiser/
│       ├── RotoSimplifier/
│       └── USDsubD/
└── <ToolSourceFolders>/       # C++ source code & CMake build scripts
```

---

## Loading Plugins in Nuke

Add either the root `COMPILED` directory or the specific version directory to your Nuke plugin path:

```python
# Option A: Auto-detects running Nuke version (e.g. routes to Nuke17.1)
nuke.pluginAddPath("F:/Tools dev/Nuke plugin builds/COMPILED")

# Option B: Direct version load
nuke.pluginAddPath("F:/Tools dev/Nuke plugin builds/COMPILED/Nuke17.1")
```

The loaders automatically register each tool folder, configure Windows DLL search paths, and register node menu entries.
