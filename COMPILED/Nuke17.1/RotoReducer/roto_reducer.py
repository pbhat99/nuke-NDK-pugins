import nuke
import nuke.rotopaint as rp
import math
import heapq
import os
import tempfile

try:
    from PySide6 import QtWidgets, QtCore, QtGui
except ImportError:
    from PySide2 import QtWidgets, QtCore, QtGui

# ==============================================================================
# 1. MATHEMATICAL CORE & ALGORITHMS
# ==============================================================================

def perpendicular_distance_2d(px, py, sx, sy, ex, ey):
    if (sx == ex) and (sy == ey):
        return math.hypot(px - sx, py - sy)
    n = abs((ey - sy) * px - (ex - sx) * py + ex * sy - ey * sx)
    d = math.hypot(ey - sy, ex - sx)
    return n / d if d > 1e-10 else 0.0

def rdp_point_distance(point, start, end):
    """Calculates combined distance for static or spatio-temporal animated points."""
    if isinstance(point[1], (list, tuple)):
        max_d = 0.0
        p_samples = point[1]
        s_samples = start[1]
        e_samples = end[1]
        for s_idx in range(len(p_samples)):
            p = p_samples[s_idx]
            s = s_samples[s_idx]
            e = e_samples[s_idx]
            dm = perpendicular_distance_2d(p[0], p[1], s[0], s[1], e[0], e[1])
            df = perpendicular_distance_2d(p[2], p[3], s[2], s[3], e[2], e[3]) if len(p) >= 4 else dm
            d = max(dm, df)
            if d > max_d:
                max_d = d
        return max_d

    dm = perpendicular_distance_2d(point[1], point[2], start[1], start[2], end[1], end[2])
    if len(point) >= 5 and len(start) >= 5 and len(end) >= 5:
        df = perpendicular_distance_2d(point[3], point[4], start[3], start[4], end[3], end[4])
        return max(dm, df)
    return dm

def ramer_douglas_peucker(points, epsilon):
    """Recursive algorithm to reduce point count based on tolerance for open paths."""
    dmax = 0.0
    index = 0
    end = len(points) - 1

    for i in range(1, end):
        d = rdp_point_distance(points[i], points[0], points[end])
        if d > dmax:
            index = i
            dmax = d

    if dmax > epsilon:
        rec1 = ramer_douglas_peucker(points[:index+1], epsilon)
        rec2 = ramer_douglas_peucker(points[index:], epsilon)
        return rec1[:-1] + rec2
    else:
        return [points[0], points[end]]

def rdp_closed_py(points, epsilon):
    """Closed loop RDP algorithm splitting at antipodal chord to prevent single-point collapse."""
    if len(points) <= 3:
        return list(points)
    k = 0
    max_d = -1.0
    is_anim = isinstance(points[0][1], (list, tuple))
    for i in range(1, len(points)):
        if is_anim:
            d_chord = 0.0
            for s_idx in range(len(points[i][1])):
                p = points[i][1][s_idx]
                p0 = points[0][1][s_idx]
                dm = math.hypot(p[0] - p0[0], p[1] - p0[1])
                df = math.hypot(p[2] - p0[2], p[3] - p0[3]) if len(p) >= 4 else dm
                d_chord = max(d_chord, max(dm, df))
            d = d_chord
        else:
            dm = math.hypot(points[i][1] - points[0][1], points[i][2] - points[0][2])
            df = math.hypot(points[i][3] - points[0][3], points[i][4] - points[0][4]) if len(points[i]) >= 5 and len(points[0]) >= 5 else dm
            d = max(dm, df)
        if d > max_d:
            max_d = d
            k = i
    if k == 0 or max_d < 1e-10:
        return list(points)

    chain1 = points[:k+1]
    chain2 = points[k:] + [points[0]]
    res1 = ramer_douglas_peucker(chain1, epsilon)
    res2 = ramer_douglas_peucker(chain2, epsilon)
    out = res1[:-1] + res2[:-1]
    if len(out) < 3:
        best_third = 1 if k > 1 else 2
        best_d = -1.0
        for i in range(1, len(points)):
            if i == k: continue
            d = rdp_point_distance(points[i], points[0], points[k])
            if d > best_d:
                best_d = d
                best_third = i
        if best_third < k:
            out = [points[0], points[best_third], points[k]]
        else:
            out = [points[0], points[k], points[best_third]]
    return out

def simplify_points_rdp(points, epsilon, is_closed=True):
    if len(points) < 3:
        return list(points)
    if is_closed:
        return rdp_closed_py(points, float(epsilon))
    else:
        return ramer_douglas_peucker(points, float(epsilon))

def solve_rdp_for_target(points, target_count, is_closed=True):
    """Binary search to find the epsilon that yields a specific point count."""
    min_count = 3 if is_closed else 2
    target_count = max(min_count, min(target_count, len(points)))
    if target_count >= len(points):
        return list(points)

    low = 0.0
    high = 5000.0
    best_epsilon = low
    best_diff = 99999

    for _ in range(30):
        mid = (low + high) / 2.0
        simplified = simplify_points_rdp(points, mid, is_closed)
        count = len(simplified)
        diff = abs(count - target_count)
        if diff < best_diff:
            best_diff = diff
            best_epsilon = mid
        if count == target_count:
            return simplified
        if count > target_count:
            low = mid
        else:
            high = mid
    return simplify_points_rdp(points, best_epsilon, is_closed)

def eval_span_error(pts, orig_indices, p_start, p_end):
    """
    Measures the maximum spatio-temporal deviation of collapsed intermediate points
    to the shortcut chord connecting p_start and p_end across all keyframe samples.
    """
    max_d = 0.0
    for s_idx in range(len(p_start[1])):
        s = p_start[1][s_idx]
        e = p_end[1][s_idx]
        for idx in orig_indices:
            p = pts[idx][1][s_idx]
            dm = perpendicular_distance_2d(p[0], p[1], s[0], s[1], e[0], e[1])
            df = perpendicular_distance_2d(p[2], p[3], s[2], s[3], e[2], e[3]) if len(p) >= 4 else dm
            d = max(dm, df)
            if d > max_d:
                max_d = d
    return max_d

def iterative_greedy_decimation(points, target_count, is_closed=True, cusps=None):
    """
    Iterative Greedy Spline Decimation:
    Priority-queue driven decimation that iteratively removes the vertex with the lowest
    spatio-temporal deformation error across all animated keyframes. Cusps are locked and protected.
    """
    n = len(points)
    min_count = 3 if is_closed else 2
    target_count = max(min_count, min(target_count, n))
    if n <= target_count:
        return list(points)

    prev_idx = [(i - 1 + n) % n if is_closed else (i - 1 if i > 0 else -1) for i in range(n)]
    next_idx = [(i + 1) % n if is_closed else (i + 1 if i < n - 1 else -1) for i in range(n)]

    collapsed_after = [[] for _ in range(n)]

    def compute_removal_cost(i):
        prv = prev_idx[i]
        nxt = next_idx[i]
        if prv == -1 or nxt == -1:
            return 1e12
        merged = collapsed_after[prv] + [i] + collapsed_after[i]
        return eval_span_error(points, merged, points[prv], points[nxt])

    heap = []
    entry_finder = {}
    REMOVED = '<removed>'

    for i in range(n):
        if not is_closed and (i == 0 or i == n - 1):
            continue
        cost = 1e12 if (cusps and cusps[i]) else compute_removal_cost(i)
        entry = [cost, i]
        entry_finder[i] = entry
        heapq.heappush(heap, entry)

    remaining_count = n
    active = [True] * n

    while remaining_count > target_count and heap:
        while heap and heap[0] is REMOVED:
            heapq.heappop(heap)
        if not heap:
            break
        cost, i = heapq.heappop(heap)
        if entry_finder.get(i) is REMOVED or not active[i]:
            continue
        if cusps and cusps[i]:
            continue

        active[i] = False
        remaining_count -= 1

        prv = prev_idx[i]
        nxt = next_idx[i]
        next_idx[prv] = nxt
        prev_idx[nxt] = prv

        collapsed_after[prv] = collapsed_after[prv] + [i] + collapsed_after[i]
        collapsed_after[i] = []

        for nb in (prv, nxt):
            if not is_closed and (nb == 0 or nb == n - 1):
                continue
            if not active[nb]:
                continue
            if cusps and cusps[nb]:
                continue
            if nb in entry_finder and entry_finder[nb] is not REMOVED:
                entry_finder[nb][0] = -1  # invalidate
            new_cost = compute_removal_cost(nb)
            new_entry = [new_cost, nb]
            entry_finder[nb] = new_entry
            heapq.heappush(heap, new_entry)

    return [points[i] for i in range(n) if active[i]]

# --- Tangent Calculators: Catmull-Rom & Schneider Least-Squares ---

def calculate_smooth_tangents(pts, is_closed=True, tension=1.0, is_smooth_mask=None):
    """
    Computes length-scaled cubic Bezier tangents matching Catmull-Rom curvature.
    Preserves cusps and straight edges by keeping tangent at (0, 0) when not smooth.
    """
    n = len(pts)
    if n < 2 or tension <= 0.0:
        return [((0.0, 0.0), (0.0, 0.0))] * n

    tangents = []
    for i in range(n):
        if is_smooth_mask is not None and not is_smooth_mask[i]:
            tangents.append(((0.0, 0.0), (0.0, 0.0)))
            continue

        prev_pt = pts[(i - 1 + n) % n] if is_closed else (pts[0] if i == 0 else pts[i - 1])
        curr_pt = pts[i]
        next_pt = pts[(i + 1) % n] if is_closed else (pts[n - 1] if i == n - 1 else pts[i + 1])

        v_in_x = curr_pt[0] - prev_pt[0]
        v_in_y = curr_pt[1] - prev_pt[1]
        d_in = math.hypot(v_in_x, v_in_y)

        v_out_x = next_pt[0] - curr_pt[0]
        v_out_y = next_pt[1] - curr_pt[1]
        d_out = math.hypot(v_out_x, v_out_y)

        if d_in < 1e-6 and d_out < 1e-6:
            tangents.append(((0.0, 0.0), (0.0, 0.0)))
            continue

        u_in_x = v_in_x / d_in if d_in > 1e-6 else 0.0
        u_in_y = v_in_y / d_in if d_in > 1e-6 else 0.0
        u_out_x = v_out_x / d_out if d_out > 1e-6 else 0.0
        u_out_y = v_out_y / d_out if d_out > 1e-6 else 0.0

        tx = u_in_x + u_out_x
        ty = u_in_y + u_out_y
        t_len = math.hypot(tx, ty)

        if t_len < 1e-6:
            tangents.append(((0.0, 0.0), (0.0, 0.0)))
            continue

        tx /= t_len
        ty /= t_len

        left_len = (d_in / 3.0) * tension
        right_len = (d_out / 3.0) * tension

        left_tan = (-tx * left_len, -ty * left_len)
        right_tan = (tx * right_len, ty * right_len)
        tangents.append((left_tan, right_tan))

    return tangents

def fit_least_squares_tangents(pts, orig_pts, kept_indices, is_closed=True, is_smooth_mask=None):
    """
    Fits cubic Bezier handle lengths alpha1, alpha2 along Catmull-Rom tangent directions
    using Schneider's 2x2 linear least-squares normal equations on original intermediate points.
    Maximizes shape likeness and curve fullness with minimal control points.
    """
    n = len(pts)
    base_tans = calculate_smooth_tangents(pts, is_closed, 1.0, is_smooth_mask)
    if len(orig_pts) == n:
        return base_tans

    refined_tans = list(base_tans)
    spans = n if is_closed else n - 1

    for s in range(spans):
        idx_a = s
        idx_b = (s + 1) % n
        orig_idx_a = kept_indices[idx_a]
        orig_idx_b = kept_indices[idx_b]

        if is_smooth_mask is not None:
            if not is_smooth_mask[idx_a] and not is_smooth_mask[idx_b]:
                continue

        intermediate = []
        if orig_idx_b > orig_idx_a:
            for k in range(orig_idx_a + 1, orig_idx_b):
                intermediate.append(orig_pts[k])
        elif is_closed and orig_idx_b < orig_idx_a:
            for k in range(orig_idx_a + 1, len(orig_pts)):
                intermediate.append(orig_pts[k])
            for k in range(0, orig_idx_b):
                intermediate.append(orig_pts[k])

        if not intermediate:
            continue

        p0 = pts[idx_a]
        p3 = pts[idx_b]
        chord_len = math.hypot(p3[0] - p0[0], p3[1] - p0[1])
        if chord_len < 1e-4:
            continue

        r_dir = base_tans[idx_a][1]
        r_mag = math.hypot(r_dir[0], r_dir[1])
        l_dir = base_tans[idx_b][0]
        l_mag = math.hypot(l_dir[0], l_dir[1])

        t1_unit = (r_dir[0] / r_mag, r_dir[1] / r_mag) if r_mag > 1e-4 else (0.0, 0.0)
        t2_unit = (l_dir[0] / l_mag, l_dir[1] / l_mag) if l_mag > 1e-4 else (0.0, 0.0)

        all_pts = [p0] + intermediate + [p3]
        total_d = sum(math.hypot(all_pts[i][0] - all_pts[i-1][0], all_pts[i][1] - all_pts[i-1][1]) for i in range(1, len(all_pts)))
        if total_d < 1e-4:
            continue

        curr_accum = 0.0
        u_vals = []
        for i in range(1, len(all_pts) - 1):
            curr_accum += math.hypot(all_pts[i][0] - all_pts[i-1][0], all_pts[i][1] - all_pts[i-1][1])
            u_vals.append(curr_accum / total_d)

        c11 = 0.0; c12 = 0.0; c22 = 0.0
        x1 = 0.0; x2 = 0.0

        for idx_u, u in enumerate(u_vals):
            q = intermediate[idx_u]
            b0 = (1.0 - u) ** 3
            b1 = 3.0 * ((1.0 - u) ** 2) * u
            b2 = 3.0 * (1.0 - u) * (u ** 2)
            b3 = u ** 3

            base_x = p0[0] * (b0 + b1) + p3[0] * (b2 + b3)
            base_y = p0[1] * (b0 + b1) + p3[1] * (b2 + b3)

            a1_x = b1 * t1_unit[0]
            a1_y = b1 * t1_unit[1]
            a2_x = b2 * t2_unit[0]
            a2_y = b2 * t2_unit[1]

            diff_x = q[0] - base_x
            diff_y = q[1] - base_y

            c11 += a1_x * a1_x + a1_y * a1_y
            c12 += a1_x * a2_x + a1_y * a2_y
            c22 += a2_x * a2_x + a2_y * a2_y

            x1 += diff_x * a1_x + diff_y * a1_y
            x2 += diff_x * a2_x + diff_y * a2_y

        det = c11 * c22 - c12 * c12
        if abs(det) > 1e-8:
            alpha1 = (x1 * c22 - x2 * c12) / det
            alpha2 = (x2 * c11 - x1 * c12) / det
            if 0.05 * chord_len <= alpha1 <= 1.8 * chord_len and r_mag > 1e-4:
                new_rx = t1_unit[0] * alpha1
                new_ry = t1_unit[1] * alpha1
                refined_tans[idx_a] = (refined_tans[idx_a][0], (new_rx, new_ry))
            if 0.05 * chord_len <= alpha2 <= 1.8 * chord_len and l_mag > 1e-4:
                new_lx = t2_unit[0] * alpha2
                new_ly = t2_unit[1] * alpha2
                refined_tans[idx_b] = ((new_lx, new_ly), refined_tans[idx_b][1])

    return refined_tans

def get_shape_keyframes(shape, current_frame):
    """
    Extracts all keyframed frames for a shape across control points and attributes.
    If the shape is static, returns [current_frame].
    """
    keys = set()
    for pt in shape:
        for attr_name in ('center', 'featherCenter', 'leftTangent', 'rightTangent', 'featherLeftTangent', 'featherRightTangent'):
            attr = getattr(pt, attr_name, None)
            if attr:
                try:
                    for t in attr.getControlPointKeyTimes():
                        keys.add(int(round(t)) if abs(t - round(t)) < 1e-3 else t)
                except Exception:
                    pass
    if not keys:
        return [current_frame]
    return sorted(list(keys))

def collect_shapes_with_paths(layer, current_path=(), parent_selected=False):
    """
    Recursively scans the layer hierarchy and returns a list of dictionaries:
    [{'shape': Shape, 'path': (0, 1, 2), 'selected': bool}, ...]
    """
    shapes_info = []
    for idx, item in enumerate(layer):
        path = current_path + (idx,)
        is_sel = parent_selected or bool(item.getFlag(rp.FlagType.eSelectedFlag))
        if isinstance(item, rp.Shape):
            shapes_info.append({
                'shape': item,
                'path': path,
                'selected': is_sel
            })
        elif isinstance(item, rp.Layer):
            shapes_info.extend(collect_shapes_with_paths(item, path, is_sel))
    return shapes_info

def get_item_by_path(root_layer, path):
    """Retrieves an item from a root layer using its integer index path tuple."""
    curr = root_layer
    for idx in path:
        curr = curr[idx]
    return curr

def clone_roto_node(orig_node):
    """
    Clones a Roto node via script serialization to perfectly preserve the complete
    hierarchy (folders and subfolders), layer/shape transforms, tracks, pivot points,
    extraMatrix, and expressions without spatial offsets.
    """
    with tempfile.NamedTemporaryFile(suffix='.nk', delete=False) as tmp:
        tmp_path = tmp.name.replace('\\', '/')
    try:
        for n in nuke.allNodes():
            n.setSelected(False)
        orig_node.setSelected(True)
        nuke.nodeCopy(tmp_path)
        for n in nuke.allNodes():
            n.setSelected(False)
        new_node = nuke.nodePaste(tmp_path)
    finally:
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except Exception:
                pass
    new_node.setName(orig_node.name() + "_Reduced")
    new_node.setXYpos(orig_node.xpos() + 120, orig_node.ypos())
    if orig_node.input(0):
        new_node.setInput(0, orig_node.input(0))
    return new_node

def find_parent_layer(shape, layer):
    """Finds which layer a shape belongs to."""
    for item in layer:
        if item.name == shape.name: return layer
        if isinstance(item, rp.Layer):
            found = find_parent_layer(shape, item)
            if found: return found
    return None

def _collect_shapes(items):
    shapes = []
    if items is None:
        return shapes
    for item in items:
        if isinstance(item, rp.Shape):
            shapes.append(item)
        elif isinstance(item, rp.Layer):
            shapes.extend(_collect_shapes(item))
    return shapes

# ==============================================================================
# 2. UI INTERFACE
# ==============================================================================

class RotoReducerDialog(QtWidgets.QDialog):
    def __init__(self):
        super(RotoReducerDialog, self).__init__()
        self.setWindowTitle("RotoReducer v1.0")
        self.resize(420, 520)
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint) 

        self.setStyleSheet("""
            QGroupBox { font-weight: bold; border: 1px solid #444; margin-top: 6px; padding-top: 10px; border-radius: 4px; }
            QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 4px; }
            QLabel { font-size: 11px; }
            QRadioButton { padding: 4px; }
            QPushButton { padding: 10px; font-weight: bold; background-color: #444; border-radius: 4px; }
            QPushButton:hover { background-color: #555; }
            QTabWidget::pane { border: 1px solid #444; }
            QTabBar::tab { background: #333; padding: 8px 20px; border-top-left-radius: 4px; border-top-right-radius: 4px; margin-right: 1px; }
            QTabBar::tab:selected { background: #555; }
        """)

        self.main_layout = QtWidgets.QVBoxLayout()
        self.setLayout(self.main_layout)

        # Tabs
        self.tabs = QtWidgets.QTabWidget()
        self.main_layout.addWidget(self.tabs)

        self.tab_reduce = QtWidgets.QWidget()
        self.setup_reduce_tab()
        self.tabs.addTab(self.tab_reduce, "Reduce")

        self.tab_explode = QtWidgets.QWidget()
        self.setup_explode_tab()
        self.tabs.addTab(self.tab_explode, "Explode")

        # Footer
        self.info_label = QtWidgets.QLabel("Ready.")
        self.info_label.setAlignment(QtCore.Qt.AlignCenter)
        self.info_label.setStyleSheet("color: #aaa; font-style: italic; margin-top: 5px;")
        self.main_layout.addWidget(self.info_label)

        self.credit_label = QtWidgets.QLabel("inspired by Marten Blumen | enhanced by pbhat99")
        self.credit_label.setAlignment(QtCore.Qt.AlignCenter)
        self.credit_label.setStyleSheet("color: #555; font-size: 9px; margin-bottom: 2px;")
        self.main_layout.addWidget(self.credit_label)

    # --- REDUCE TAB SETUP ---
    def setup_reduce_tab(self):
        layout = QtWidgets.QVBoxLayout()
        layout.setSpacing(10)
        self.tab_reduce.setLayout(layout)

        # 1. Algorithm Selection Group
        self.algo_group = QtWidgets.QGroupBox("Reduction Algorithm")
        algo_layout = QtWidgets.QVBoxLayout()
        algo_layout.setSpacing(8)

        self.radio_schneider = QtWidgets.QRadioButton("Schneider Bezier Fit (Least-Squares) [Recommended]")
        self.radio_schneider.setChecked(True)
        self.radio_schneider.setToolTip(
            "Best for smooth flowing curves, circles, ellipses, and continuous splines.\n"
            "Fits cubic Bezier handles using mathematical 2x2 linear least-squares against original curve points to maximize curvature match."
        )

        self.radio_greedy = QtWidgets.QRadioButton("Iterative Greedy Decimation")
        self.radio_greedy.setToolTip(
            "Best for complex deforming shapes with mixed curves and corners.\n"
            "Iteratively simulates removing each point, evaluates deformation error across all keyframes, and removes vertices with lowest error while preserving cusps."
        )

        self.radio_rdp = QtWidgets.QRadioButton("Spatio-Temporal Envelope RDP")
        self.radio_rdp.setToolTip(
            "Best for hard-surface shapes, sharp corners, and cusps.\n"
            "Uses minimax chord distance to strictly preserve corner peaks and peak articulators across keyframes."
        )

        algo_layout.addWidget(self.radio_schneider)
        algo_layout.addWidget(self.radio_greedy)
        algo_layout.addWidget(self.radio_rdp)
        self.algo_group.setLayout(algo_layout)
        layout.addWidget(self.algo_group)

        # 2. Percentage Slider & SpinBox (0 - 100%)
        self.percent_group = QtWidgets.QGroupBox("Reduction Amount")
        pct_layout = QtWidgets.QHBoxLayout()
        pct_layout.setSpacing(8)

        self.slider_percent = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.slider_percent.setRange(0, 100)
        self.slider_percent.setValue(50)
        self.slider_percent.setToolTip("Target percentage of points to reduce (0% keeps all points, 50% cuts points by half, 80% retains 20%).")

        self.spin_percent = QtWidgets.QSpinBox()
        self.spin_percent.setRange(0, 100)
        self.spin_percent.setValue(50)
        self.spin_percent.setSuffix("%")
        self.spin_percent.setFixedWidth(65)
        self.spin_percent.setToolTip("Target percentage of points to reduce (0-100%).")

        self.slider_percent.valueChanged.connect(self.spin_percent.setValue)
        self.spin_percent.valueChanged.connect(self.slider_percent.setValue)

        pct_layout.addWidget(self.slider_percent)
        pct_layout.addWidget(self.spin_percent)
        self.percent_group.setLayout(pct_layout)
        layout.addWidget(self.percent_group)

        # 3. Information Box
        self.info_box = QtWidgets.QGroupBox("Nuke Animation Topology")
        info_layout = QtWidgets.QVBoxLayout()
        info_lbl = QtWidgets.QLabel(
            "• Constant Point Count: Nuke requires a fixed number of control points across animation. RotoReducer evaluates shape deformation across all keyframes simultaneously to retain an optimal vertex set.\n"
            "• Keyframe Processing: Evaluates and bakes keys only on existing keyframed frames.\n"
            "• Tangent & Feather Alignment: Recalculates smooth Bézier tangents and mirrors them onto feather points.\n"
            "• Output: Always copies and outputs reduced shapes to a separate new Roto node."
        )
        info_lbl.setStyleSheet("color: #ccc; font-size: 11px;")
        info_lbl.setWordWrap(True)
        info_layout.addWidget(info_lbl)
        self.info_box.setLayout(info_layout)
        layout.addWidget(self.info_box)

        layout.addStretch()

        # 4. Action Button
        self.btn_process = QtWidgets.QPushButton("REDUCE SHAPES")
        self.btn_process.setStyleSheet("background-color: #d98016; color: white; padding: 14px; font-size: 12px; font-weight: bold;")
        self.btn_process.setToolTip("Reduces control points by the specified percentage across all keyframed frames, preserving Nuke constant point topology and outputting to a new Roto node.")
        self.btn_process.clicked.connect(self.run_process)
        layout.addWidget(self.btn_process)

    # --- EXPLODE TAB SETUP ---
    def setup_explode_tab(self):
        layout = QtWidgets.QVBoxLayout()
        layout.setSpacing(10)
        self.tab_explode.setLayout(layout)
        
        lbl = QtWidgets.QLabel("Splits folders into separate, disconnected Roto nodes.")
        lbl.setWordWrap(True)
        lbl.setStyleSheet("color: #aaa; padding: 10px;")
        layout.addWidget(lbl)
        
        self.explode_group = QtWidgets.QGroupBox("Explode Options")
        group_layout = QtWidgets.QVBoxLayout()
        
        group_layout.addWidget(QtWidgets.QLabel("Depth Level:"))
        self.combo_depth = QtWidgets.QComboBox()
        self.combo_depth.addItems(["Top Level Only (Depth 1)", "Infinite Recursive (All Sub-Folders)", "Custom Depth Level"])
        self.combo_depth.currentIndexChanged.connect(self.toggle_explode_inputs)
        group_layout.addWidget(self.combo_depth)

        self.spin_depth = QtWidgets.QSpinBox()
        self.spin_depth.setRange(1, 10)
        self.spin_depth.setValue(2)
        self.spin_depth.setVisible(False)
        group_layout.addWidget(self.spin_depth)
        
        self.explode_group.setLayout(group_layout)
        layout.addWidget(self.explode_group)
        
        layout.addStretch()
        
        self.btn_explode = QtWidgets.QPushButton("EXPLODE LAYERS")
        self.btn_explode.setStyleSheet("background-color: #4477aa; color: white; padding: 15px;")
        self.btn_explode.clicked.connect(self.run_explode)
        layout.addWidget(self.btn_explode)
        layout.addStretch()

    def toggle_explode_inputs(self):
        self.spin_depth.setVisible(self.combo_depth.currentIndex() == 2)

    # ==============================================================================
    # 3. LOGIC EXECUTION
    # ==============================================================================

    def run_process(self):
        node = nuke.selectedNode()
        if not node or node.Class() not in ('Roto', 'RotoPaint'):
            self.info_label.setText("Error: Select a Roto node."); return

        curves_knob = node['curves']
        root_layer = curves_knob.rootLayer
        shapes_info = collect_shapes_with_paths(root_layer)
        if not shapes_info:
            self.info_label.setText("Error: No shapes found in Roto node."); return

        has_selection = any(info['selected'] for info in shapes_info)
        targets_info = [info for info in shapes_info if info['selected']] if has_selection else shapes_info
        if not targets_info:
            self.info_label.setText("Error: No shapes to process."); return

        current_frame = nuke.frame()

        # Clone node to preserve exact folder/subfolder hierarchy, transforms, tracks, pivot points, and offsets
        new_roto_node = clone_roto_node(node)
        target_knob = new_roto_node['curves']
        target_root = target_knob.rootLayer

        if self.radio_schneider.isChecked():
            algo_mode = 0
        elif self.radio_greedy.isChecked():
            algo_mode = 1
        else:
            algo_mode = 2

        percent = float(self.spin_percent.value())
        count_processed = 0
        task = nuke.ProgressTask(f"Reducing Roto Shapes ({int(percent)}%)")

        try:
            for idx, info in enumerate(targets_info):
                if task.isCancelled(): break

                shape = info['shape']
                target_shape = get_item_by_path(target_root, info['path'])

                eval_frames = get_shape_keyframes(shape, current_frame)
                points_data = []
                orig_is_smooth = []

                for i in range(len(shape)):
                    is_sm = False
                    samples = []
                    for f in eval_frames:
                        pt_main = shape[i].center.getPosition(f)
                        pt_f_off = shape[i].featherCenter.getPosition(f)
                        mx = float(pt_main[0])
                        my = float(pt_main[1])
                        fx_off = float(pt_f_off[0])
                        fy_off = float(pt_f_off[1])
                        samples.append((mx, my, mx + fx_off, my + fy_off))

                        lt = shape[i].leftTangent.getPosition(f)
                        rt = shape[i].rightTangent.getPosition(f)
                        flt = shape[i].featherLeftTangent.getPosition(f)
                        frt = shape[i].featherRightTangent.getPosition(f)
                        if (lt[0]*lt[0] + lt[1]*lt[1] > 1e-4) or (rt[0]*rt[0] + rt[1]*rt[1] > 1e-4) or \
                           (flt[0]*flt[0] + flt[1]*flt[1] > 1e-4) or (frt[0]*frt[0] + frt[1]*frt[1] > 1e-4):
                            is_sm = True

                    points_data.append((i, samples))
                    orig_is_smooth.append(is_sm)

                if len(points_data) < 3: continue

                shape_has_any_tangent = any(orig_is_smooth)

                try: is_open_val = shape.getAttributes().getValue(current_frame, "op")
                except: is_open_val = 0.0
                is_closed = (is_open_val == 0.0)

                # Target percentage reduction (preserves constant point topology across keyframes)
                original_count = len(points_data)
                min_pts = 3 if is_closed else 2
                keep_fraction = (100.0 - percent) / 100.0
                target_k = max(min_pts, min(original_count, int(round(original_count * keep_fraction))))

                if algo_mode == 0:
                    simplified_data = solve_rdp_for_target(points_data, target_k, is_closed)
                elif algo_mode == 1:
                    simplified_data = iterative_greedy_decimation(
                        points_data, target_k, is_closed,
                        cusps=[not sm for sm in orig_is_smooth] if shape_has_any_tangent else None
                    )
                else:
                    simplified_data = solve_rdp_for_target(points_data, target_k, is_closed)

                indices_to_track = [p[0] for p in simplified_data]
                if len(indices_to_track) < min_pts: continue

                has_tan_mask = [orig_is_smooth[i] for i in indices_to_track] if shape_has_any_tangent else [False] * len(indices_to_track)

                # In-place point reduction on target_shape (preserves exact folder hierarchy and name)
                while len(target_shape) > 0:
                    target_shape.remove(0)

                for _ in indices_to_track:
                    target_shape.append(rp.AnimControlPoint(0, 0))

                # Bake only keyframed frames
                total_frames = len(eval_frames)
                for f_idx, frame in enumerate(eval_frames):
                    progress = int(((f_idx / total_frames) * 100))
                    task.setMessage(f"Processing keyframe {frame} ({idx+1}/{len(targets_info)})")
                    task.setProgress(progress)

                    frame_main_pts = []
                    frame_feather_offsets = []

                    for i in indices_to_track:
                        orig_pt = shape[i].center.getPosition(frame)
                        orig_f_pt = shape[i].featherCenter.getPosition(frame)
                        mx = float(orig_pt[0])
                        my = float(orig_pt[1])
                        fx_off = float(orig_f_pt[0])
                        fy_off = float(orig_f_pt[1])
                        frame_main_pts.append((mx, my))
                        frame_feather_offsets.append((fx_off, fy_off))

                    if algo_mode == 0:  # Schneider Least-Squares
                        orig_frame_pts = []
                        for i in range(len(shape)):
                            pt_m = shape[i].center.getPosition(frame)
                            orig_frame_pts.append((float(pt_m[0]), float(pt_m[1])))
                        main_tangents = fit_least_squares_tangents(frame_main_pts, orig_frame_pts, indices_to_track, is_closed, has_tan_mask)
                    else:  # Greedy Decimation or Spatio-Temporal Envelope RDP
                        main_tangents = calculate_smooth_tangents(frame_main_pts, is_closed, 1.0, has_tan_mask)

                    for k, new_pt in enumerate(target_shape):
                        mx, my = frame_main_pts[k]
                        fx_off, fy_off = frame_feather_offsets[k]
                        lt_m, rt_m = main_tangents[k]

                        flt = lt_m
                        frt = rt_m

                        new_pt.center.addKey(frame, (mx, my))
                        new_pt.featherCenter.addKey(frame, (fx_off, fy_off))
                        new_pt.leftTangent.addKey(frame, lt_m)
                        new_pt.rightTangent.addKey(frame, rt_m)
                        new_pt.featherLeftTangent.addKey(frame, flt)
                        new_pt.featherRightTangent.addKey(frame, frt)

                        if frame == current_frame or len(eval_frames) == 1:
                            try:
                                new_pt.center.setPosition((mx, my, 0.0))
                                new_pt.featherCenter.setPosition((fx_off, fy_off, 0.0))
                                new_pt.leftTangent.setPosition((lt_m[0], lt_m[1], 0.0))
                                new_pt.rightTangent.setPosition((rt_m[0], rt_m[1], 0.0))
                                new_pt.featherLeftTangent.setPosition((flt[0], flt[1], 0.0))
                                new_pt.featherRightTangent.setPosition((frt[0], frt[1], 0.0))
                            except Exception: pass

                try:
                    target_shape.getAttributes().set(current_frame, "op", is_open_val)
                except Exception: pass

                count_processed += 1
        finally:
            if task:
                del task

        target_knob.changed()
        self.info_label.setText(f"Success: Reduced {count_processed} shape(s) by {int(percent)}%.")

    def run_explode(self):
        node = nuke.selectedNode()
        if not node or node.Class() not in ('Roto', 'RotoPaint'): 
            self.info_label.setText("Error: Select a Roto node."); return
        
        curves_knob = node['curves']
        root_layer = curves_knob.rootLayer
        
        mode = self.combo_depth.currentIndex()
        if mode == 0: max_depth = 1
        elif mode == 1: max_depth = 99
        else: max_depth = self.spin_depth.value()

        target_layers = []
        
        def scan_layers(layer, current_depth):
            if current_depth > max_depth: return
            if isinstance(layer, rp.Layer):
                if current_depth > 0:
                    target_layers.append(layer.name)
                for item in layer:
                    scan_layers(item, current_depth + 1)
        
        scan_layers(root_layer, 0)
        target_layers = sorted(list(set(target_layers)))
        
        if not target_layers:
            self.info_label.setText("No layers found at this depth."); return

        with tempfile.NamedTemporaryFile(suffix='.nk', delete=False) as tmp:
            tmp_path = tmp.name.replace('\\', '/')
        try:
            for n in nuke.allNodes(): n.setSelected(False)
            node.setSelected(True)
            nuke.nodeCopy(tmp_path)
            for n in nuke.allNodes(): n.setSelected(False)
            
            x_pos = node.xpos()
            y_pos = node.ypos()
            created_count = 0
            
            task = nuke.ProgressTask("Exploding Layers")
            
            for i, layer_name in enumerate(target_layers):
                if task.isCancelled(): break
                task.setMessage(f"Exploding {layer_name}")
                task.setProgress(int((i / len(target_layers)) * 100))
                
                new_node = nuke.nodePaste(tmp_path)
                new_node.setName(f"{node.name()}_{layer_name}")
                new_node.setXYpos(x_pos + ((i + 1) * 120), y_pos)
                
                new_curves = new_node['curves']
                new_root = new_curves.rootLayer
                
                def prune_layers(layer):
                    for item in list(layer):
                        if isinstance(item, rp.Layer):
                            if item.name == layer_name:
                                continue
                            elif any(sub.name == layer_name for sub in item if isinstance(sub, rp.Layer)):
                                prune_layers(item)
                            else:
                                layer.remove(item)
                        elif isinstance(item, rp.Shape):
                            if layer.name != layer_name:
                                layer.remove(item)
                
                prune_layers(new_root)
                new_curves.changed()
                created_count += 1
                
            del task
            self.info_label.setText(f"Exploded into {created_count} nodes.")
        finally:
            if os.path.exists(tmp_path):
                try: os.remove(tmp_path)
                except Exception: pass

# ==============================================================================
# 4. ENTRY POINTS & NODE INTEGRATION
# ==============================================================================

def find_target_roto_node(node_name=None):
    if node_name:
        n = nuke.toNode(node_name)
        if n:
            curr = n.input(0)
            while curr:
                if curr.Class() in ('Roto', 'RotoPaint'):
                    return curr
                curr = curr.input(0)
    for n in nuke.selectedNodes():
        if n.Class() in ('Roto', 'RotoPaint'):
            return n
    roto_nodes = [n for n in nuke.allNodes() if n.Class() in ('Roto', 'RotoPaint')]
    if len(roto_nodes) == 1:
        return roto_nodes[0]
    return None

def run_reduce_from_node(caller_node_name):
    caller = nuke.toNode(caller_node_name)
    if not caller:
        nuke.message("Error: Could not locate calling RotoReducer node.")
        return

    algo_mode = int(caller['algorithm'].getValue()) if 'algorithm' in caller.knobs() else 0
    percent = float(caller['percent'].getValue()) if 'percent' in caller.knobs() else 50.0

    roto_node = find_target_roto_node(caller_node_name)
    if not roto_node:
        nuke.message("Please select or connect a Roto or RotoPaint node.")
        return

    curves_knob = roto_node['curves']
    root_layer = curves_knob.rootLayer

    shapes_info = collect_shapes_with_paths(root_layer)
    if not shapes_info:
        nuke.message(f"No shapes found inside '{roto_node.name()}'.")
        return

    targets_info = shapes_info

    current_frame = nuke.frame()

    # Clone node to preserve exact folder/subfolder hierarchy, transforms, tracks, pivot points, and offsets
    new_roto = clone_roto_node(roto_node)
    target_knob = new_roto['curves']
    target_root = target_knob.rootLayer

    count_processed = 0
    task = nuke.ProgressTask(f"Reducing Roto Shapes ({int(percent)}%)")

    try:
        for idx, info in enumerate(targets_info):
            if task and task.isCancelled():
                break

            shape = info['shape']
            target_shape = get_item_by_path(target_root, info['path'])

            eval_frames = get_shape_keyframes(shape, current_frame)
            points_data = []
            orig_is_smooth = []

            for i in range(len(shape)):
                is_sm = False
                samples = []
                for f in eval_frames:
                    pt_main = shape[i].center.getPosition(f)
                    pt_f_off = shape[i].featherCenter.getPosition(f)
                    mx = float(pt_main[0])
                    my = float(pt_main[1])
                    fx_off = float(pt_f_off[0])
                    fy_off = float(pt_f_off[1])
                    samples.append((mx, my, mx + fx_off, my + fy_off))

                    lt = shape[i].leftTangent.getPosition(f)
                    rt = shape[i].rightTangent.getPosition(f)
                    flt = shape[i].featherLeftTangent.getPosition(f)
                    frt = shape[i].featherRightTangent.getPosition(f)
                    if (lt[0]*lt[0] + lt[1]*lt[1] > 1e-4) or (rt[0]*rt[0] + rt[1]*rt[1] > 1e-4) or \
                       (flt[0]*flt[0] + flt[1]*flt[1] > 1e-4) or (frt[0]*frt[0] + frt[1]*frt[1] > 1e-4):
                        is_sm = True

                points_data.append((i, samples))
                orig_is_smooth.append(is_sm)

            if len(points_data) < 3:
                continue

            shape_has_any_tangent = any(orig_is_smooth)

            try:
                is_open_val = shape.getAttributes().getValue(current_frame, "op")
            except Exception:
                is_open_val = 0.0
            is_closed = (is_open_val == 0.0)

            # Target percentage reduction (preserves constant point topology across keyframes)
            original_count = len(points_data)
            min_pts = 3 if is_closed else 2
            keep_fraction = (100.0 - percent) / 100.0
            target_k = max(min_pts, min(original_count, int(round(original_count * keep_fraction))))

            if algo_mode == 0:
                simplified_data = solve_rdp_for_target(points_data, target_k, is_closed)
            elif algo_mode == 1:
                simplified_data = iterative_greedy_decimation(
                    points_data, target_k, is_closed,
                    cusps=[not sm for sm in orig_is_smooth] if shape_has_any_tangent else None
                )
            else:
                simplified_data = solve_rdp_for_target(points_data, target_k, is_closed)

            indices_to_track = [p[0] for p in simplified_data]
            if len(indices_to_track) < min_pts:
                continue

            has_tan_mask = [orig_is_smooth[i] for i in indices_to_track] if shape_has_any_tangent else [False] * len(indices_to_track)

            # In-place point reduction on target_shape (preserves exact folder hierarchy and name)
            while len(target_shape) > 0:
                target_shape.remove(0)

            for _ in indices_to_track:
                target_shape.append(rp.AnimControlPoint(0, 0))

            total_frames = len(eval_frames)
            for f_idx, frame in enumerate(eval_frames):
                if task:
                    progress = int(((f_idx + 1) / total_frames) * 100)
                    task.setMessage(f"Processing keyframe {frame} ({idx+1}/{len(targets_info)})")
                    task.setProgress(progress)

                frame_main_pts = []
                frame_feather_offsets = []
                for i in indices_to_track:
                    orig_pt = shape[i].center.getPosition(frame)
                    orig_f_pt = shape[i].featherCenter.getPosition(frame)
                    mx = float(orig_pt[0])
                    my = float(orig_pt[1])
                    fx_off = float(orig_f_pt[0])
                    fy_off = float(orig_f_pt[1])
                    frame_main_pts.append((mx, my))
                    frame_feather_offsets.append((fx_off, fy_off))

                if algo_mode == 0:  # Schneider Least-Squares
                    orig_frame_pts = []
                    for i in range(len(shape)):
                        pt_m = shape[i].center.getPosition(frame)
                        orig_frame_pts.append((float(pt_m[0]), float(pt_m[1])))
                    main_tangents = fit_least_squares_tangents(frame_main_pts, orig_frame_pts, indices_to_track, is_closed, has_tan_mask)
                else:  # Greedy Decimation or Spatio-Temporal Envelope RDP
                    main_tangents = calculate_smooth_tangents(frame_main_pts, is_closed, 1.0, has_tan_mask)

                for k, new_pt in enumerate(target_shape):
                    mx, my = frame_main_pts[k]
                    fx_off, fy_off = frame_feather_offsets[k]
                    lt_m, rt_m = main_tangents[k]

                    flt = lt_m
                    frt = rt_m

                    new_pt.center.addKey(frame, (mx, my))
                    new_pt.featherCenter.addKey(frame, (fx_off, fy_off))
                    new_pt.leftTangent.addKey(frame, lt_m)
                    new_pt.rightTangent.addKey(frame, rt_m)
                    new_pt.featherLeftTangent.addKey(frame, flt)
                    new_pt.featherRightTangent.addKey(frame, frt)

                    if frame == current_frame or len(eval_frames) == 1:
                        try:
                            new_pt.center.setPosition((mx, my, 0.0))
                            new_pt.featherCenter.setPosition((fx_off, fy_off, 0.0))
                            new_pt.leftTangent.setPosition((lt_m[0], lt_m[1], 0.0))
                            new_pt.rightTangent.setPosition((rt_m[0], rt_m[1], 0.0))
                            new_pt.featherLeftTangent.setPosition((flt[0], flt[1], 0.0))
                            new_pt.featherRightTangent.setPosition((frt[0], frt[1], 0.0))
                        except Exception:
                            pass

            try:
                target_shape.getAttributes().set(current_frame, "op", is_open_val)
            except Exception:
                pass

            count_processed += 1
    finally:
        if task:
            del task

    target_knob.changed()
    nuke.message(f"RotoReducer: Successfully reduced {count_processed} shape(s) by {int(percent)}%.")

_dialog_instance = None

def show_dialog():
    global _dialog_instance
    if _dialog_instance is None:
        _dialog_instance = RotoReducerDialog()
    _dialog_instance.show()
    _dialog_instance.raise_()
    _dialog_instance.activateWindow()

if __name__ == '__main__':
    show_dialog()