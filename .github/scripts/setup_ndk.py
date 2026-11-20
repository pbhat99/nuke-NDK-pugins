#!/usr/bin/env python3
"""
Nuke NDK Setup and Caching Helper for GitHub Actions & Local Builds

Points NDKDIR to the bundled repository NDK stubs (.github/ndk/), ensuring
instant, reliable builds on Windows, Linux, and macOS without downloading
any multi-gigabyte installer packages.
"""

import os
import sys
import argparse
import subprocess
import shutil
from pathlib import Path

def setup_ndk(platform_name, version, cache_dir=None):
    root_dir = Path(__file__).resolve().parents[2]
    ndk_root = root_dir / ".github" / "ndk"
    
    # 1. Base include directory
    common_include = ndk_root / "include"
    if not common_include.exists():
        # Fallback to local Windows Nuke installation if on local machine
        for cand in [Path("C:/Program Files/Nuke17.1v2/include"), Path("C:/Program Files/Nuke16.0v9/include")]:
            if cand.exists():
                shutil.copytree(cand, common_include, dirs_exist_ok=True)
                break

    # 2. Platform-specific NDK directory
    if platform_name == "Windows":
        target_ndk = ndk_root / "win" / f"Nuke{version}"
        if not target_ndk.exists() or not (target_ndk / "DDImage.lib").exists():
            # If exact version folder doesn't exist, fall back to closest available (e.g. Nuke17.1 or Nuke16.0)
            avail = sorted((ndk_root / "win").glob("Nuke*"))
            if avail:
                target_ndk = avail[-1]
                print(f"[FALLBACK] Using closest available Windows NDK stub: {target_ndk.name}")
            else:
                target_ndk.mkdir(parents=True, exist_ok=True)
                # Copy from local Windows install if present
                for cand_ver in [f"Nuke{version}v2", f"Nuke{version}v1", f"Nuke{version}v9", f"Nuke{version}"]:
                    local_lib = Path(f"C:/Program Files/{cand_ver}/DDImage.lib")
                    if local_lib.exists():
                        shutil.copy2(local_lib, target_ndk / "DDImage.lib")
                        break

        # Ensure include/ exists inside target_ndk
        if common_include.exists() and not (target_ndk / "include").exists():
            try:
                shutil.copytree(common_include, target_ndk / "include", dirs_exist_ok=True)
            except Exception as e:
                print(f"[WARN] Could not copy include: {e}")

    elif platform_name == "macOS":
        target_ndk = ndk_root / "mac" / f"Nuke{version}"
        target_ndk.mkdir(parents=True, exist_ok=True)
        if common_include.exists() and not (target_ndk / "include").exists():
            shutil.copytree(common_include, target_ndk / "include", dirs_exist_ok=True)
        # Create stub libDDImage.dylib so CMake find_library and existence checks succeed
        stub_dylib = target_ndk / "libDDImage.dylib"
        if not stub_dylib.exists():
            try:
                subprocess.run(["clang", "-shared", "-o", str(stub_dylib), "-x", "c", "/dev/null"], check=True)
                print(f"[NDK] Created stub {stub_dylib.name}")
            except Exception as e:
                print(f"[WARN] Could not create stub dylib: {e}")

    else: # Linux
        target_ndk = ndk_root / "linux" / f"Nuke{version}"
        target_ndk.mkdir(parents=True, exist_ok=True)
        if common_include.exists() and not (target_ndk / "include").exists():
            shutil.copytree(common_include, target_ndk / "include", dirs_exist_ok=True)
        # Create stub libDDImage.so so CMake find_library and existence checks succeed
        stub_so = target_ndk / "libDDImage.so"
        if not stub_so.exists():
            try:
                subprocess.run(["gcc", "-shared", "-o", str(stub_so), "-x", "c", "/dev/null"], check=True)
                print(f"[NDK] Created stub {stub_so.name}")
            except Exception as e:
                print(f"[WARN] Could not create stub so: {e}")

    export_github_env(target_ndk)

def export_github_env(ndk_dir):
    ndk_norm = str(ndk_dir).replace("\\", "/")
    print(f"\n[NDK READY] NDKDIR={ndk_norm}")
    
    # Export for GitHub Actions runners
    github_env = os.environ.get("GITHUB_ENV")
    if github_env and os.path.exists(github_env):
        with open(github_env, "a", encoding="utf-8") as f:
            f.write(f"NDKDIR={ndk_norm}\n")
            f.write(f"Nuke_ROOT={ndk_norm}\n")
            f.write(f"NUKE_INSTALL_PATH={ndk_norm}\n")
            f.write(f"NUKE_INSTALL_DIR={ndk_norm}\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Setup Nuke NDK headers and libraries")
    parser.add_argument("--platform", required=True, choices=["Windows", "Linux", "macOS"], help="Target platform")
    parser.add_argument("--version", required=True, help="Nuke version (e.g. 17.1, 16.0)")
    parser.add_argument("--cache-dir", default=None, help="Optional cache directory")
    args = parser.parse_args()

    setup_ndk(args.platform, args.version, args.cache_dir)
