#!/usr/bin/env python3
"""
Cross-Platform Plugin Build Orchestrator for GitHub Actions & Local CLI

Builds Nuke NDK C++ plugins across Windows, Linux, and macOS, deploying
compiled artifacts directly to COMPILED/<PLATFORM>/Nuke<version>/<Tool>/.
"""

import os
import sys
import argparse
import subprocess
import shutil
from pathlib import Path

PLUGIN_REGISTRY = [
    {
        "id": "ImageTool",
        "name": "ImageTool",
        "source": "ImageTool",
        "target": "ImageTool",
        "binaries": ["ImageTool"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "bamDeVe",
        "name": "bamDeVe",
        "source": "bamDeVe",
        "target": "bamDeVe",
        "binaries": ["bamDeVe"],
        "extra_files": ["bamDeVe_solve.py", "init.py", "menu.py"]
    },
    {
        "id": "cropMore",
        "name": "cropMore",
        "source": "cropMore",
        "target": "cropMore",
        "binaries": ["CropMore"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "rotoReducer",
        "name": "rotoReducer",
        "source": "rotoReducer",
        "target": "RotoReducer",
        "binaries": ["RotoReducer", "RotoExplode"],
        "extra_files": ["roto_reducer.py", "menu.py"]
    },
    {
        "id": "AlphaToRoto",
        "name": "AlphaToRoto",
        "source": "AlphaToRoto",
        "target": "AlphaToRoto",
        "binaries": ["AlphaToRoto"],
        "extra_files": ["alphaToRoto.py", "menu.py"]
    },
    {
        "id": "ColorGrade",
        "name": "ColorGrade",
        "source": "colorgrade",
        "target": "ColorGrade",
        "binaries": ["ColorGrade"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "DeepC",
        "name": "DeepC",
        "source": "DeepC",
        "target": "DeepC",
        "binaries": ["DeepThinner", "DeepCAdd", "DeepCExpression"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "DeepErode",
        "name": "DeepErode",
        "source": "DeepErode",
        "target": "DeepErode",
        "binaries": ["DeepErode"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "FastNoiseLite",
        "name": "FastNoiseLite",
        "source": "FastNoiseLite",
        "target": "FastNoiseLite",
        "binaries": ["FastNoiseLite"],
        "extra_files": ["init.py", "menu.py"]
    },
    {
        "id": "NukeShenanigans",
        "name": "NukeShenanigans",
        "source": "NukeShenanigans",
        "target": "NukeShenanigans",
        "binaries": ["MirrorExtend", "DummyCamera", "DeepHistogram", "ContactSheetAll", "RemoveUnlimited"],
        "extra_files": ["menu.py"]
    },
    {
        "id": "USDsubD",
        "name": "USDsubD",
        "source": "USDsubD",
        "target": "USDsubD",
        "binaries": ["USDSubD"],
        "extra_files": ["menu.py"]
    }
]

def get_platform_tag(platform_name):
    if platform_name.lower().startswith("win"):
        return "WIN", [".dll"]
    elif platform_name.lower().startswith("mac") or platform_name.lower() == "darwin":
        return "MAC", [".dylib", ".so"]
    else:
        return "LINUX", [".so"]

def build_single_plugin(plugin, platform_name, nuke_version, ndk_dir, root_dir):
    root = Path(root_dir).resolve()
    src_dir = root / plugin["source"]
    platform_tag, valid_exts = get_platform_tag(platform_name)
    
    v_clean = nuke_version.lower().replace("nuke", "")
    folder_tag = f"Nuke{v_clean}"

    deploy_dir = root / "COMPILED" / platform_tag / folder_tag / plugin["target"]
    deploy_dir.mkdir(parents=True, exist_ok=True)
    
    tmp_build = root / "_build_tmp" / f"{plugin['id']}_{platform_tag}_{v_clean}"
    if tmp_build.exists():
        shutil.rmtree(tmp_build, ignore_errors=True)
    tmp_build.mkdir(parents=True, exist_ok=True)

    print(f"\n{'='*70}")
    print(f"Building [{plugin['name']}] for {platform_name} (Nuke {v_clean})")
    print(f"Source : {src_dir}")
    print(f"Deploy : {deploy_dir}")
    print(f"{'='*70}")

    ndk_norm = str(Path(ndk_dir).resolve()).replace("\\", "/")
    cmake_module_dir = str(root / "cmake").replace("\\", "/")
    cmake_args = [
        "cmake",
        "-B", str(tmp_build),
        "-S", str(src_dir),
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DNUKE_VERSION={v_clean}",
        f"-DNDKDIR={ndk_norm}",
        f"-DNuke_ROOT={ndk_norm}",
        f"-DNUKE_INSTALL_PATH={ndk_norm}",
        f"-DNUKE_INSTALL_DIR={ndk_norm}",
        f"-DCMAKE_MODULE_PATH={cmake_module_dir}"
    ]

    # Platform-specific generator & flags
    if platform_tag == "WIN":
        if "CMAKE_GENERATOR" not in os.environ:
            cmake_args.extend(["-G", "Visual Studio 17 2022", "-A", "x64"])
    elif platform_tag == "MAC":
        cmake_args.extend([
            "-G", "Unix Makefiles",
            "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64",
            "-DCMAKE_SHARED_LINKER_FLAGS=-undefined dynamic_lookup",
            "-DCMAKE_MODULE_LINKER_FLAGS=-undefined dynamic_lookup",
            "-DCMAKE_CXX_FLAGS=-fdeclspec"
        ])
    else: # LINUX
        cmake_args.extend([
            "-G", "Unix Makefiles",
            "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,--unresolved-symbols=ignore-all",
            "-DCMAKE_MODULE_LINKER_FLAGS=-Wl,--unresolved-symbols=ignore-all"
        ])

    if plugin["id"] == "DeepC":
        cmake_args.append("-DDEEPC_BUILD_TESTS=OFF")

    # 1. CMake Configure
    print(f"[1/2] Configuring CMake...")
    res = subprocess.run(cmake_args)
    if res.returncode != 0:
        print(f"[ERROR] CMake configure failed for {plugin['name']}")
        return False

    # 2. CMake Build
    print(f"[2/2] Compiling plugin...")
    build_cmd = ["cmake", "--build", str(tmp_build), "--config", "Release", "--parallel"]
    res = subprocess.run(build_cmd)
    if res.returncode != 0:
        print(f"[ERROR] Build failed for {plugin['name']}")
        return False

    # 3. Deploy Artifacts
    print(f"[3/3] Deploying artifacts to {deploy_dir} ...")
    found_any = False
    for ext_candidate in valid_exts:
        for f in tmp_build.rglob(f"*{ext_candidate}"):
            if f.name.endswith(".lib") or f.name.endswith(".exp"):
                continue
            shutil.copy2(f, deploy_dir)
            print(f"  + Deployed: {f.name}")
            found_any = True

    # Also deploy companion .pyd files if present
    for f in tmp_build.rglob("*.pyd"):
        shutil.copy2(f, deploy_dir)
        print(f"  + Deployed: {f.name}")
        found_any = True

    # Copy extra files (init.py, menu.py, companion scripts)
    for extra in plugin.get("extra_files", []):
        src_extra = src_dir / extra
        if src_extra.exists():
            shutil.copy2(src_extra, deploy_dir)
            print(f"  + Copied: {extra}")

    # Ensure target Nuke version folder has dynamic init.py
    ensure_version_init(root / "COMPILED" / platform_tag / folder_tag, platform_tag)

    # Clean intermediate build dir to save disk space
    shutil.rmtree(tmp_build, ignore_errors=True)
    return found_any

def ensure_version_init(version_dir, platform_tag):
    version_dir.mkdir(parents=True, exist_ok=True)
    init_path = version_dir / "init.py"
    if not init_path.exists():
        content = f'''"""
Nuke {version_dir.name} ({platform_tag}) Plugin Auto-Discovery Initializer
Registers all tool subdirectories with Nuke's pluginPath.
"""
import os
import sys
import nuke

_root = os.path.dirname(os.path.abspath(__file__))
for _item in sorted(os.listdir(_root)):
    _sub = os.path.join(_root, _item)
    if os.path.isdir(_sub) and not _item.startswith((".", "_")):
        if _sub not in sys.path:
            sys.path.insert(0, _sub)
        nuke.pluginAddPath(_sub)
'''
        with open(init_path, "w", encoding="utf-8") as f:
            f.write(content)

def main():
    parser = argparse.ArgumentParser(description="Build Nuke NDK Plugins")
    parser.add_argument("--platform", required=True, help="Target platform (Windows, Linux, macOS)")
    parser.add_argument("--version", required=True, help="Nuke version (e.g. 17.1, 16.0)")
    parser.add_argument("--plugin", default="ALL", help="Plugin ID or ALL")
    parser.add_argument("--ndk", required=True, help="Path to Nuke NDK directory")
    args = parser.parse_args()

    root_dir = Path(__file__).resolve().parents[2]
    plugins_to_build = []

    if args.plugin.upper() == "ALL":
        plugins_to_build = PLUGIN_REGISTRY
    else:
        for p in PLUGIN_REGISTRY:
            if p["id"].lower() == args.plugin.lower() or p["name"].lower() == args.plugin.lower():
                plugins_to_build.append(p)
                break
        if not plugins_to_build:
            print(f"[ERROR] Plugin not found matching '{args.plugin}'")
            sys.exit(1)

    success_count = 0
    fail_count = 0

    for plugin in plugins_to_build:
        ok = build_single_plugin(plugin, args.platform, args.version, args.ndk, root_dir)
        if ok:
            success_count += 1
        else:
            fail_count += 1

    print(f"\n{'='*70}")
    print(f"BUILD SUMMARY: {success_count} succeeded, {fail_count} failed")
    print(f"{'='*70}")

    if fail_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
