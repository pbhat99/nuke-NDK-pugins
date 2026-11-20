"""
Root COMPILED Plugins Initializer

Detects the active Nuke version and automatically adds the matching
version folder (e.g. COMPILED/Nuke17.1/) to Nuke's plugin path and
executes its version-specific init.py.
"""
import os
import sys
import nuke

def _route_nuke_version():
    root_dir = os.path.dirname(os.path.abspath(__file__))
    major = nuke.NUKE_VERSION_MAJOR
    minor = nuke.NUKE_VERSION_MINOR

    # Look for exact version match first, then major version fallback
    candidates = [
        f"Nuke{major}.{minor}",
        f"Nuke{major}",
    ]

    target_dir = None
    for cand in candidates:
        cand_path = os.path.join(root_dir, cand)
        if os.path.isdir(cand_path):
            target_dir = cand_path
            break

    if not target_dir:
        # Flexible match for folders starting with Nuke<major>
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
        print(f"[COMPILED] No precompiled plugin folder found matching Nuke {major}.{minor} in {root_dir}")

_route_nuke_version()
