# ImageTool

A high-performance Nuke NDK C++ plugin that computes live image statistics across an input image — similar to CurveTool but without requiring an offline pre-render pass. It outputs a solid color image representing the Average, Median, Low (Min), or High (Max) value of the input image (similar to the Stat node in Shake).

- **Original Author**: Gerard Benjamin Pierre ([IMDb](http://www.imdb.com/name/nm0682633/))
- **Original Nukepedia**: [ImageTool on Nukepedia](https://www.nukepedia.com/tools/plugins/colour/imagetool/)
- **Upstream Repository**: [petermercell/ImageTool](https://github.com/petermercell/ImageTool)

---

## Features & Optimizations

- **Pure Native C++ NDK**: Statically linked (/MT) with zero external runtime dependencies.
- **Hardware Generic & Multithreaded**: Dynamically utilizes system VRAM throughput and CPU cores with generic GPU fallback controls (`use GPU`).
- **Clean Nuke Tab Hierarchy**: No spurious initial Tab knobs, preserving native Nuke `Node` tabs (`label`, `disable`, etc.).
- **Standard Routing & Controls**:
  - `output` ChannelSet knob with backward-compatible `channels` mapping.
  - Channel passthrough: untargeted channels pass through untouched from `input(0)`.
  - Standard `mix` knob (0..1) on the primary tab at the bottom of the main controls.
  - `alpha only`: Restricts sampling to pixels where `alpha > 0` (weighted for Average, filtered for Median/Low/High).
- **Algorithmic Improvements**:
  - $O(N)$ streaming pass for Min, Max, and Average (no heap allocations or full-frame sorting).
  - $O(N)$ `std::nth_element` selection for exact Median computation.
  - Fixed channel array indexing boundary bug present in original upstream code.

---

## Building from Source (Windows)

Use the central workspace interactive builder:
```cmd
build.bat 1 "17.1" "ImageTool"
build.bat 1 "16.0" "ImageTool"
```

---

## License

BSD 3-Clause License — Copyright (c) 2011 Gerard Benjamin Pierre. See [LICENSE](LICENSE) for details.
