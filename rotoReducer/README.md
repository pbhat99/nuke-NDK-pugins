# RotoReducer for Foundry Nuke

**Production-grade spline point decimation and curvature optimization for VFX rotoscoping in Nuke.**

Heavy roto shapes with hundreds of control points are sluggish to edit, difficult to track, and slow to render. **RotoReducer** intelligently reduces control point counts by a user-specified percentage (**0–100% slider**) while preserving visual curvature, sharp cusps, feather contours, and animated motion across keyframes.

Available both as a dedicated **C++ NDK node (`RotoReducer`)** and as an interactive **Python UI Dialog**.

---

## Key Features

- **0–100% Reduction Slider** — Full control over point decimation with instant percentage dial (0% keeps all points, 50% cuts points by half, 80% retains only 20%).
- **Three Distinct VFX Algorithms** — Choose between Schneider Least-Squares Bézier Fitting, Iterative Greedy Decimation, or Spatio-Temporal Envelope RDP.
- **Nuke Constant Point Topology Preservation** — Strictly respects Nuke's architectural requirement of a fixed point count per shape across time, evaluating deformation over the entire animation to select an optimal vertex set.
- **Keyframe-Only Evaluation** — Automatically discovers all keyframes across the animation and bakes keys strictly on existing keyframes (no unwanted in-between keys).
- **Feather Tangent Alignment** — Feather handles strictly mirror the roto spline tangents (`featherLeftTangent = leftTangent`, `featherRightTangent = rightTangent`), eliminating flat or disconnected feathering.
- **Non-Destructive Workflow** — Always outputs simplified shapes to a separate, cleanly named new Roto node (`<OriginalNode>_Reduced`).
- **RotoExplode Node Built In** — Dedicated C++ node and UI tab to explode deep layer hierarchies into individual, disconnected Roto nodes.

---

## Why Nuke's Animation Architecture Requires Multi-Frame Evaluation

In Foundry Nuke's Roto core (`_curvelib.AnimCurveCore` / `nuke.rotopaint.Shape`), a shape curve maintains a **strictly constant number of control points across the entire timeline**. Nuke does not permit variable vertex topology per frame in a single shape.

### The Problem with Single-Frame Simplification
If a roto shape is simplified based solely on the current frame $f_{\text{curr}}$:
1. Vertices that are temporarily straight, slow-moving, or stationary at $f_{\text{curr}}$ will be culled.
2. When the animation advances to another frame (e.g., an actor bending an elbow or a cape fluttering), those removed vertices are missing, causing severe cutting across the subject.

### The RotoReducer Solution
RotoReducer extracts all keyframe timestamps using `get_shape_keyframes()`. For every control point $i$, it gathers its spatial coordinates and feather offsets across **all keyframes simultaneously**. The reduction algorithms evaluate the spatio-temporal deformation error over the complete motion envelope to choose a single, globally optimal vertex subset. This ensures the output shape has an identical, legal point topology on every frame while preserving every articulation in the animation.

---

## Detailed Algorithm Breakdown

RotoReducer exposes three specialized reduction algorithms tailored for different VFX rotoscoping scenarios:

### 1. Schneider's Least-Squares Bézier Fitting (Recommended Default)
* **Literature Reference**: Philip J. Schneider, *"An Algorithm for Automatically Fitting Digitized Curves"*, Graphics Gems I, Academic Press, 1990, pp. 612–626. ([Graphics Gems C Source Reference](https://github.com/erich666/GraphicsGems/tree/master/gems/FitCurves.c) / [Schneider Curve Fitting Notes](https://people.cs.clemson.edu/~dhouse/courses/405/notes/splines.pdf)).
* **Industry Context**: The mathematical foundation used in Adobe Illustrator's *Simplify Path*, Inkscape, and FontForge.
* **Mechanism**:
  1. Identifies key vertices using the spatio-temporal envelope.
  2. For each Bézier span between retained endpoints $P_0$ and $P_3$, parameterizes intermediate original points $Q_k$ by chord-length $u_k \in [0, 1]$.
  3. Formulates a **$2 \times 2$ linear least-squares normal equations system** $\mathbf{C} \boldsymbol{\alpha} = \mathbf{X}$ to solve for the optimal handle lengths $\alpha_1, \alpha_2$ along unit tangent directions $\hat{\mathbf{t}}_1, \hat{\mathbf{t}}_2$:
     $$\begin{bmatrix} c_{11} & c_{12} \\ c_{12} & c_{22} \end{bmatrix} \begin{bmatrix} \alpha_1 \\ \alpha_2 \end{bmatrix} = \begin{bmatrix} x_1 \\ x_2 \end{bmatrix}$$
     where:
     $$c_{11} = \sum_k \|B_1(u_k) \hat{\mathbf{t}}_1\|^2, \quad c_{22} = \sum_k \|B_2(u_k) \hat{\mathbf{t}}_2\|^2, \quad c_{12} = \sum_k (B_1(u_k) \hat{\mathbf{t}}_1) \cdot (B_2(u_k) \hat{\mathbf{t}}_2)$$
     $$x_1 = \sum_k (Q_k - [P_0(B_0 + B_1) + P_3(B_2 + B_3)]) \cdot (B_1(u_k) \hat{\mathbf{t}}_1)$$
     $$x_2 = \sum_k (Q_k - [P_0(B_0 + B_1) + P_3(B_2 + B_3)]) \cdot (B_2(u_k) \hat{\mathbf{t}}_2)$$
  4. Directly minimizes the squared Euclidean deviation $\sum_k \|B(u_k) - Q_k\|^2$.
* **Best Used For**: Smooth flowing curves, circles, ellipses, limbs, and continuous organic contours. Delivers sub-pixel curvature fidelity with the fewest control points.

---

### 2. Iterative Greedy Decimation
* **Literature Reference**: Michael Garland and Paul S. Heckbert, *"Surface Simplification Using Quadric Error Metrics"*, SIGGRAPH 1997. ([Garland & Heckbert QEM Overview](https://www.cs.cmu.edu/~./garland/quadrics/quadrics.html)).
* **Mechanism**:
  1. Constructs a priority queue (min-heap) containing all candidate vertices.
  2. For each vertex $v_i$, calculates the exact **spatio-temporal collapse cost** introduced if $v_i$ is removed and neighbors $v_{i-1}$ and $v_{i+1}$ are connected:
     $$\text{Cost}(v_i) = \max_{f \in \text{keyframes}} \max_{k \in \text{span}} \max\left( d_{\text{perp}}(Q_{k,f}, v_{i-1,f} \to v_{i+1,f}), \; d_{\text{feather}}(Q_{k,f}, v_{i-1,f} \to v_{i+1,f}) \right)$$
  3. Cusp vertices (where tangents are zero or direction changes sharply) are assigned infinite cost ($10^{12}$) and locked against removal.
  4. Iteratively pops the vertex with the lowest cost, updates neighboring shortcut spans, and repeats until the target point count $K = \max(K_{\min}, \text{round}(N \cdot (1 - \text{pct}/100)))$ is reached.
* **Best Used For**: Complex deforming shapes across animated sequences with mixed curves and corners. Guarantees monotonic error progression without unexpected shape degradation.

---

### 3. Spatio-Temporal Envelope RDP
* **Literature Reference**: Urs Ramer (1972), *"An iterative procedure for the polygonal approximation of plane curves"*, and David Douglas & Thomas Peucker (1973), *"Algorithms for the reduction of the number of points required to represent a digitized line or its caricature"*. ([Douglas-Peucker Algorithm](https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm)).
* **Mechanism**:
  1. Extends the classical 2D RDP algorithm into a multi-frame spatio-temporal envelope.
  2. Evaluates the minimax perpendicular distance across all animation keyframes:
     $$D_i = \max_{f \in \text{keyframes}} \max\left( d_{\text{main}}(i, f), \; d_{\text{feather}}(i, f) \right)$$
  3. Uses binary search over tolerance $\epsilon$ to converge exactly on the target point count determined by the percentage slider.
  4. Recomputes smooth Catmull-Rom tangents while preserving zero-length tangents at cusps.
* **Best Used For**: Hard-surface shapes, sharp corners, weapons, vehicles, and mechanical objects where corner peaks must remain strictly anchored.

---

## Algorithm Comparison Guide

| Algorithm | Primary Math Metric | Point Efficiency | Curvature Likeness | Cusp Preservation | Best Production Use Case |
|---|---|:---:|:---:|:---:|---|
| **Schneider Bezier Fit** *(Default)* | $2 \times 2$ Least-Squares Normal Equations | **Maximum** | **Sub-pixel** | Preserved | Smooth organic splines, heads, bodies, circles |
| **Iterative Greedy Decimation** | Multi-Frame Priority Queue Minimax Error | High | High | **Strict Lock** | Complex deforming characters, clothes, mixed curves |
| **Spatio-Temporal Envelope RDP** | Multi-Frame Perpendicular Chord Distance | High | Moderate | **Maximum** | Hard-surface objects, architecture, sharp corners |

---

## Installation & Setup

All compiled plugins and runtime helpers are centralized in `COMPILED/Nuke17.1/RotoReducer/`.

### 1. Add to your `~/.nuke/init.py`
```python
import nuke
# Auto-detects and loads all centralized plugins:
nuke.pluginAddPath("F:/Tools dev/Nuke plugin builds/COMPILED")
```

### 2. Available Commands in Nuke
- **Node Toolbar**: `Draw` $\to$ `RotoReducer`
- **Node Toolbar**: `Draw` $\to$ `RotoExplode`
- **Properties Menu (RMB)**: `Roto Reducer...` (opens GUI Dialog)

---

## File Structure

```
rotoReducer/
├── CMakeLists.txt            # CMake build configuration for Nuke NDK
├── RotoReducer.cpp           # Dedicated C++ NDK node implementation
├── RotoExplode.cpp           # Dedicated C++ layer exploder node
├── roto_reducer.py           # Core Python module & PySide GUI Dialog
├── cpp/
│   ├── roto_native.cpp       # High-speed native C++ Python accelerator
│   └── setup.py              # Standalone Python extension setup
├── images/                   # Documentation and UI assets
└── README.md                 # Complete documentation
```

---

## Credits & Inspiration

- **Original Inspiration**: Inspired by Marten Blumen's original *RotoSimplify* and *RotoTools* concepts ([martyblumen.com](https://www.martyblumen.com/)).
- **Architecture & Implementation**: Complete architectural redesign, Nuke NDK C++ plugin nodes, multi-frame spatio-temporal spline evaluation, Schneider least-squares Bézier fitting, iterative greedy decimation, and feather tangent alignment developed by **pbhat99**.
