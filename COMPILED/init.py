"""
Root COMPILED Plugins Initializer

Detects the active OS platform (WIN, LINUX, MAC) and Nuke version,
automatically routing to the matching platform & version folder
(e.g. COMPILED/WIN/Nuke17.1/) in Nuke's plugin path and executing
its version-specific init.py.
"""
import os
import sys
import nuke

def _route_nuke_version():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    major = nuke.NUKE_VERSION_MAJOR
    minor = nuke.NUKE_VERSION_MINOR

    # Determine OS platform subfolder
    if sys.platform.startswith("win"):
        platform_sub = "WIN"
    elif sys.platform.startswith("linux"):
        platform_sub = "LINUX"
    elif sys.platform.startswith("darwin"):
        platform_sub = "MAC"
    else:
        platform_sub = None

    candidates = [
        f"Nuke{major}.{minor}",
        f"Nuke{major}",
    ]

    target_dir = None

    # 1. Look in platform subfolder (e.g. COMPILED/WIN/Nuke17.1/)
    if platform_sub:
        platform_root = os.path.join(root_dir, platform_sub)
        if os.path.isdir(platform_root):
            for cand in candidates:
                cand_path = os.path.join(platform_root, cand)
                if os.path.isdir(cand_path):
                    target_dir = cand_path
                    break
            if not target_dir:
                for name in sorted(os.listdir(platform_root)):
                    full_path = os.path.join(platform_root, name)
                    if os.path.isdir(full_path) and name.lower().startswith(f"nuke{major}"):
                        target_dir = full_path
                        break

    # 2. Fallback to direct root folder for backwards compatibility
    if not target_dir:
        for cand in candidates:
            cand_path = os.path.join(root_dir, cand)
            if os.path.isdir(cand_path):
                target_dir = cand_path
                break
        if not target_dir:
            for name in sorted(os.listdir(root_dir)):
                full_path = os.path.join(root_dir, name)
                if os.path.isdir(full_path) and name.lower().startswith(f"nuke{major}"):
                    target_dir = full_path
                    break

    if target_dir:
        nuke.pluginAddPath(target_dir)
        version_init = os.path.join(target_dir, "init.py")
        if os.path.isfile(version_init):
            with open(version_init, "r", encoding="utf-8") as f:
                code = compile(f.read(), version_init, "exec")
                exec(code, {"__file__": version_init, "__name__": "__main__"})
    else:
        print(f"[COMPILED] No precompiled plugin folder found matching Nuke {major}.{minor} ({platform_sub}) in {root_dir}")

_route_nuke_version()
