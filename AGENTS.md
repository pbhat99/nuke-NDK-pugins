# Workspace Guidelines for Nuke Plugin Builds

The following rules apply universally across all projects in this workspace (`f:/Tools dev/Nuke plugin builds/`):

- **Target Platform**: Windows only. Ignore Linux, macOS, and any other platforms unless explicitly requested.
- **Production Hardware & GPU Target**: The user's production machine is equipped with an **NVIDIA GeForce RTX 4090** (24 GB VRAM, Ada Lovelace architecture, Compute Capability 8.9 / `sm_89`, 4th-Gen Tensor Cores). Never restrict memory limits, tiling sizes, buffer allocations, or compilation targets to lower-spec development machines (e.g., GTX 1650). Always ensure plugins are fully optimized to leverage the RTX 4090's high compute throughput, 24 GB VRAM budget (enabling large 2048/4096 tiles or single-pass un-tiled processing), and Tensor Core acceleration.
- **Working Directory**: Always use the current project directory only. All builds, intermediate files, and downloaded dependencies must stay within the workspace.
- **Nuke Configuration**: Do not modify, append to, or overwrite `init.py` or `menu.py` in `~/.nuke` (the user manages Nuke environment and plugin paths manually).
- **Environment PATH**: Avoid modifying or adding paths to the system or user `PATH` environment variable.
- **Output Artifacts**: Place all compiled plugins, runtime dependencies, and helper scripts in the centralized workspace root directory: `COMPILED/Nuke<version>/<ToolName>/` (e.g., `COMPILED/Nuke17.1/DeepC/`, `COMPILED/Nuke17.1/NukeShenanigans/`, `COMPILED/Nuke17.1/oidnDenoise/`, `COMPILED/Nuke17.1/RotoSimplifier/`). Each Nuke version folder contains a single `init.py` that dynamically registers all tool subfolders to `nuke.pluginPath()`, and the root `COMPILED/` folder contains an `init.py` that auto-detects the running Nuke version and routes to the appropriate version folder.
- **Git & Binary Tracking**:
  - Do not add `COMPILED/` to `.gitignore`; the user manually manages and commits precompiled binaries.
  - Maintain strict `.gitignore` exclusions for Python cache (`__pycache__`, `*.pyc`), virtual environments (`.venv/`), build directories (`build/`), compiler intermediate files (`*.obj`, `*.pdb`, `*.ilk`), and CMake temporary files.
  - Purge intermediate `build/` directories, temporary artifacts, and duplicate `.agents/` or `AGENTS.md` files from tool subfolders before committing.
  - Link upstream source repositories via Git remotes (`upstream-<ToolName>`), document them in `README.md`, and commit tool additions separately per tool.
- **Windows Runtime DLL Resolution**: For plugins with sibling runtime DLL dependencies (e.g., Intel OIDN, TBB, CUDA/OptiX), ensure directories are registered both via `os.add_dll_directory()` and Win32 process `PATH` (`ctypes.windll.kernel32.SetEnvironmentVariableW("PATH", ...)`) so Nuke's C++ `LoadLibrary` can resolve all dependencies without system PATH pollution.
- **Node Tab Hierarchy**: In `knobs(Knob_Callback f)`, do not declare an initial `Tab_knob` at the very start of the function. Let Nuke automatically title the primary tab after the node class so Nuke's native `Node` tab (`label`, `disable`, etc.) is cleanly preserved. Declare secondary tabs (`Tab_knob(f, "...")`) only after the primary controls.
- **Standard Knobs & Placement**:
  - **Channel Knobs**: For generator/draw nodes, name the target channel knob `output` (`ChannelSet_knob(f, &_channels, "output", "output")`) and provide `Obsolete_knob(f, "channels", ...)` where applicable for backward compatibility.
  - **Mix Knob**: Place the standard `mix` knob (`Float_knob(f, &_mix, "mix", "mix")` with range 0..1) on the primary tab at the bottom of the main controls, ensuring immediate access without needing to switch tabs.
- **Engine Channel Pass-Through**: In `engine()`, always check `_channels.contains(z)`. Any channels not targeted by the output knob must pass through untouched from `input(0)` (via `std::copy`) or be cleared if no input is connected. Ensure single-channel targets (e.g., `alpha` or custom mattes) receive the generated pattern rather than arbitrary constants.
