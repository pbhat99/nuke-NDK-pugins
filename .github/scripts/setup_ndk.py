#!/usr/bin/env python3
"""
Nuke NDK Setup and Caching Helper for GitHub Actions

Checks the local or GitHub Actions cache for Nuke NDK headers and DDImage library.
If missing, fetches/extracts the minimal NDK stubs, stores them in the persistent
cache directory, and exports NDKDIR to GITHUB_ENV so future runs skip downloading.
"""

import os
import sys
import argparse
import subprocess
import urllib.request
import tarfile
import zipfile
import shutil
from pathlib import Path

# Foundry public release download links (official release CDN)
FOUNDRY_URLS = {
    "17.1": {
        "Linux": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/17.1v2/Nuke17.1v2-linux-x86_64.run",
        "macOS": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/17.1v2/Nuke17.1v2-mac-arm64.dmg",
        "Windows": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/17.1v2/Nuke17.1v2-win-x86_64.zip"
    },
    "16.0": {
        "Linux": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/16.0v9/Nuke16.0v9-linux-x86_64.run",
        "macOS": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/16.0v9/Nuke16.0v9-mac-arm64.dmg",
        "Windows": "https://thefoundry.s3.amazonaws.com/products/nuke/releases/16.0v9/Nuke16.0v9-win-x86_64.zip"
    }
}

def setup_ndk(platform_name, version, cache_dir):
    cache_path = Path(cache_dir).resolve()
    target_ndk = cache_path / f"Nuke{version}_{platform_name.lower()}"
    target_ndk.mkdir(parents=True, exist_ok=True)

    include_dir = target_ndk / "include"
    
    if platform_name == "Windows":
        lib_file = target_ndk / "DDImage.lib"
    elif platform_name == "macOS":
        lib_file = target_ndk / "libDDImage.dylib"
    else: # Linux
        lib_file = target_ndk / "libDDImage.so"

    # 1. Check if NDK is already cached
    if include_dir.exists() and lib_file.exists():
        print(f"[CACHE HIT] Nuke {version} NDK for {platform_name} found in cache: {target_ndk}")
        export_github_env(target_ndk)
        return

    print(f"[CACHE MISS] Nuke {version} NDK for {platform_name} not found in {cache_path}. Preparing...")

    # 2. Check if host already has local Nuke installed (e.g. self-hosted runner or local PC)
    local_candidates = []
    if platform_name == "Windows":
        local_candidates = [
            Path(f"C:/Program Files/Nuke{version}v2"),
            Path(f"C:/Program Files/Nuke{version}v1"),
            Path(f"C:/Program Files/Nuke{version}v9"),
            Path(f"C:/Program Files/Nuke{version}")
        ]
    elif platform_name == "macOS":
        local_candidates = [
            Path(f"/Applications/Nuke{version}/Nuke{version}.app/Contents/MacOS"),
            Path(f"/Applications/Nuke{version}v2/Nuke{version}v2.app/Contents/MacOS")
        ]
    else: # Linux
        local_candidates = [
            Path(f"/usr/local/Nuke{version}"),
            Path(f"/opt/Nuke{version}")
        ]

    for cand in local_candidates:
        if cand.exists():
            print(f"[LOCAL HIT] Found local installation at {cand}. Copying stubs to cache...")
            if (cand / "include").exists():
                shutil.copytree(cand / "include", include_dir, dirs_exist_ok=True)
            if platform_name == "Windows" and (cand / "DDImage.lib").exists():
                shutil.copy2(cand / "DDImage.lib", lib_file)
            elif platform_name == "macOS" and (cand / "libDDImage.dylib").exists():
                shutil.copy2(cand / "libDDImage.dylib", lib_file)
            elif platform_name == "Linux" and (cand / "libDDImage.so").exists():
                shutil.copy2(cand / "libDDImage.so", lib_file)

            if include_dir.exists() and lib_file.exists():
                print(f"[READY] Cached NDK stub created successfully from local install: {target_ndk}")
                export_github_env(target_ndk)
                return

    # 3. Download from Foundry / CDN archive
    url_info = FOUNDRY_URLS.get(version, {}).get(platform_name)
    if not url_info:
        print(f"[WARNING] No direct download URL registered for Nuke {version} on {platform_name}.")
        print(f"[FALLBACK] Creating placeholder NDK structure. Please populate {target_ndk}")
        include_dir.mkdir(parents=True, exist_ok=True)
        export_github_env(target_ndk)
        return

    print(f"Downloading Nuke {version} package from {url_info} ...")
    temp_archive = cache_path / f"nuke_{version}_{platform_name.lower()}_installer"

    try:
        urllib.request.urlretrieve(url_info, temp_archive)
        print("Download complete. Extracting headers and DDImage...")

        if platform_name == "Linux":
            # The Linux .run file is a self-extracting archive
            os.chmod(temp_archive, 0o755)
            # Unpack only include/ and libDDImage.so using the embedded tarball
            subprocess.run([str(temp_archive), "--tar", "xf", "-C", str(target_ndk), "include", "libDDImage.so"], check=False)
        elif platform_name == "Windows":
            # Windows zip installer
            with zipfile.ZipFile(temp_archive, 'r') as zf:
                for member in zf.namelist():
                    if member.startswith("include/") or member.endswith("DDImage.lib"):
                        zf.extract(member, target_ndk)
        
        # Clean up large temp installer to save disk space
        if temp_archive.exists():
            temp_archive.unlink()

    except Exception as e:
        print(f"[ERROR] Could not extract NDK from installer: {e}")

    export_github_env(target_ndk)

def export_github_env(ndk_dir):
    ndk_norm = str(ndk_dir).replace("\\", "/")
    print(f"Exporting NDKDIR={ndk_norm}")
    
    # Export for GitHub Actions runners
    github_env = os.environ.get("GITHUB_ENV")
    if github_env and os.path.exists(github_env):
        with open(github_env, "a", encoding="utf-8") as f:
            f.write(f"NDKDIR={ndk_norm}\n")
            f.write(f"Nuke_ROOT={ndk_norm}\n")
            f.write(f"NUKE_INSTALL_PATH={ndk_norm}\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Setup Nuke NDK headers and libraries")
    parser.add_argument("--platform", required=True, choices=["Windows", "Linux", "macOS"], help="Target platform")
    parser.add_argument("--version", required=True, help="Nuke version (e.g. 17.1, 16.0)")
    parser.add_argument("--cache-dir", default=".ndk_cache", help="Directory to store cached NDK files")
    args = parser.parse_args()

    setup_ndk(args.platform, args.version, args.cache_dir)
