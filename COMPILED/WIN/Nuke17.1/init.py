"""
Nuke 17.1 Plugin Initializer

Automatically discovers each tool subfolder in this directory
(e.g., AlphaToRoto, ColorGrade, DeepC, DeepErode, FastNoiseLite,
ldpk, NukeShenanigans, oidnDenoise, OptixDenoiser, RotoReducer, USDsubD)
and registers it with Nuke's plugin path and Windows DLL search directory.
"""
import os
import sys
import ctypes
import nuke

def _init_nuke17_1_plugins():
    guard_flag = "_COMPILED_NUKE17_1_LOADED"
    if getattr(nuke, guard_flag, False):
        return
    setattr(nuke, guard_flag, True)

    this_dir = os.path.dirname(os.path.abspath(__file__))
    aux_subdirs = ("icons", "plugins", "lib")

    dll_paths = []

    for entry in sorted(os.listdir(this_dir)):
        tool_dir = os.path.join(this_dir, entry)
        if not os.path.isdir(tool_dir):
            continue
        # Skip hidden or special dirs
        if entry.startswith(('.', '_')):
            continue

        # 1. Add tool directory to Nuke plugin path
        nuke.pluginAddPath(tool_dir)

        # 2. Add to Python sys.path so helper modules are importable
        if tool_dir not in sys.path:
            sys.path.append(tool_dir)

        dll_paths.append(tool_dir)

        # 3. Check for auxiliary subdirectories
        for sub in aux_subdirs:
            sub_path = os.path.join(tool_dir, sub)
            if os.path.isdir(sub_path):
                nuke.pluginAddPath(sub_path)
                dll_paths.append(sub_path)

        # 4. If the tool has its own init.py, run it
        tool_init = os.path.join(tool_dir, "init.py")
        if os.path.isfile(tool_init):
            try:
                with open(tool_init, "r", encoding="utf-8") as f:
                    code = compile(f.read(), tool_init, "exec")
                    exec(code, {"__file__": tool_init, "__name__": "__main__"})
            except Exception as e:
                print(f"[COMPILED] Error executing {tool_init}: {e}")

    # 5. On Windows, register directories with the Win32 DLL loader so
    # dependent DLLs (e.g. OpenImageDenoise, tbb12, CUDA) are found by LoadLibrary.
    if sys.platform == "win32":
        current_path = os.environ.get("PATH", "")
        prepend_entries = [p for p in dll_paths if p not in current_path]
        if prepend_entries:
            new_path = os.pathsep.join(prepend_entries) + os.pathsep + current_path
            os.environ["PATH"] = new_path
            try:
                ctypes.windll.kernel32.SetEnvironmentVariableW("PATH", new_path)
            except Exception:
                pass

        if hasattr(os, "add_dll_directory"):
            for p in dll_paths:
                try:
                    os.add_dll_directory(p)
                except Exception:
                    pass

_init_nuke17_1_plugins()
