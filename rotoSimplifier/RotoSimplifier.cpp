//
// RotoSimplifier - Self-contained Nuke C++ Plugin Node
// Merges RDP native C++ accelerator, Nuke rotopaint logic, and interactive knobs into a single DLL.
//

#include <Python.h>
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"
#include "DDImage/Row.h"

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>

using namespace DD::Image;

static const char* const CLASS = "RotoSimplifier";
static const char* const HELP = 
    "RotoSimplifier - Reduces control points on Roto shapes using the Ramer-Douglas-Peucker algorithm,\n"
    "recalculating smooth tangents, baking animations, and exploding layers into separate nodes.";

// ==============================================================================
// 1. C++ RDP CORE ALGORITHM
// ==============================================================================

struct RotoPoint {
    double x;
    double y;
    int index;
};

inline double get_sq_dist(const RotoPoint& p, const RotoPoint& a, const RotoPoint& b) {
    double px = b.x - a.x;
    double py = b.y - a.y;
    double norm = px * px + py * py;

    if (norm < 1e-10) {
        double dx = p.x - a.x;
        double dy = p.y - a.y;
        return dx * dx + dy * dy;
    }

    double u = ((p.x - a.x) * px + (p.y - a.y) * py) / norm;
    if (u > 1.0) u = 1.0;
    else if (u < 0.0) u = 0.0;

    double x = a.x + u * px;
    double y = a.y + u * py;
    double dx = x - p.x;
    double dy = y - p.y;
    return dx * dx + dy * dy;
}

static void rdp_recursive(const std::vector<RotoPoint>& points, double epsilon_sq, std::vector<RotoPoint>& out) {
    if (points.size() < 3) {
        for (const auto& p : points) out.push_back(p);
        return;
    }

    double dmax = 0.0;
    size_t index = 0;
    size_t end = points.size() - 1;

    for (size_t i = 1; i < end; ++i) {
        double d = get_sq_dist(points[i], points[0], points[end]);
        if (d > dmax) {
            index = i;
            dmax = d;
        }
    }

    if (dmax > epsilon_sq) {
        std::vector<RotoPoint> res1, res2;
        std::vector<RotoPoint> left(points.begin(), points.begin() + index + 1);
        std::vector<RotoPoint> right(points.begin() + index, points.end());

        rdp_recursive(left, epsilon_sq, res1);
        rdp_recursive(right, epsilon_sq, res2);

        out.insert(out.end(), res1.begin(), res1.end() - 1);
        out.insert(out.end(), res2.begin(), res2.end());
    } else {
        out.push_back(points[0]);
        out.push_back(points[end]);
    }
}

// ==============================================================================
// 2. EMBEDDED PYTHON MODULE (roto_native)
// ==============================================================================

static PyObject* py_simplify(PyObject* self, PyObject* args) {
    PyObject* pList = nullptr;
    double epsilon = 0.0;

    if (!PyArg_ParseTuple(args, "O!d", &PyList_Type, &pList, &epsilon)) {
        return nullptr;
    }

    Py_ssize_t num_points = PyList_Size(pList);
    std::vector<RotoPoint> points;
    points.reserve(num_points);

    for (Py_ssize_t i = 0; i < num_points; ++i) {
        PyObject* item = PyList_GetItem(pList, i);
        if (!PyTuple_Check(item) || PyTuple_Size(item) < 3) {
            PyErr_SetString(PyExc_TypeError, "Expected list of tuples with at least (index, x, y)");
            return nullptr;
        }

        long idx = PyLong_AsLong(PyTuple_GetItem(item, 0));
        if (idx == -1 && PyErr_Occurred()) return nullptr;

        double x = PyFloat_AsDouble(PyTuple_GetItem(item, 1));
        if (x == -1.0 && PyErr_Occurred()) return nullptr;

        double y = PyFloat_AsDouble(PyTuple_GetItem(item, 2));
        if (y == -1.0 && PyErr_Occurred()) return nullptr;

        points.push_back({ x, y, static_cast<int>(idx) });
    }

    std::vector<RotoPoint> result;
    result.reserve(num_points);
    rdp_recursive(points, epsilon * epsilon, result);

    PyObject* pyResult = PyList_New(result.size());
    if (!pyResult) return nullptr;

    for (size_t i = 0; i < result.size(); ++i) {
        PyObject* tuple = PyTuple_New(3);
        if (!tuple) {
            Py_DECREF(pyResult);
            return nullptr;
        }
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(result[i].index));
        PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(result[i].x));
        PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(result[i].y));
        PyList_SetItem(pyResult, i, tuple);
    }

    return pyResult;
}

static PyMethodDef RotoNativeMethods[] = {
    {"simplify", py_simplify, METH_VARARGS, "Runs RDP simplification on points."},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef roto_native_module = {
    PyModuleDef_HEAD_INIT, "roto_native", nullptr, -1, RotoNativeMethods
};

static PyObject* init_roto_native_in_memory() {
    return PyModule_Create(&roto_native_module);
}

// ==============================================================================
// 3. EMBEDDED PYTHON LOGIC FOR ROTOPAINT & UI
// ==============================================================================

static const char* kEmbeddedPythonCode = R"PY(
import sys
import math
import nuke
import nuke.rotopaint as rp
import roto_native

def perpendicular_distance(point, start, end):
    p_x, p_y = point[1], point[2]
    s_x, s_y = start[1], start[2]
    e_x, e_y = end[1], end[2]
    if (s_x == e_x) and (s_y == e_y):
        return math.sqrt((p_x - s_x)**2 + (p_y - s_y)**2)
    n = abs((e_y - s_y) * p_x - (e_x - s_x) * p_y + e_x * s_y - e_y * s_x)
    d = math.sqrt((e_y - s_y)**2 + (e_x - s_x)**2)
    return n / d if d > 1e-10 else 0.0

def ramer_douglas_peucker_py(points, epsilon):
    if len(points) >= 3:
        try:
            return roto_native.simplify(points, float(epsilon))
        except Exception:
            pass
    dmax = 0.0
    index = 0
    end = len(points) - 1
    for i in range(1, end):
        d = perpendicular_distance(points[i], points[0], points[end])
        if d > dmax:
            index = i
            dmax = d
    if dmax > epsilon:
        rec1 = ramer_douglas_peucker_py(points[:index+1], epsilon)
        rec2 = ramer_douglas_peucker_py(points[index:], epsilon)
        return rec1[:-1] + rec2
    else:
        return [points[0], points[end]]

def solve_epsilon_for_target(points, target_count, is_closed=True):
    current_count = (len(points) - 1) if (is_closed and len(points) > 1) else len(points)
    if target_count >= current_count:
        return 0.0
    target_rdp_len = (target_count + 1) if is_closed else target_count
    low = 0.0
    high = 4000.0
    best_epsilon = low
    best_diff = float('inf')
    for _ in range(30):
        mid = (low + high) / 2.0
        simplified = ramer_douglas_peucker_py(points, mid)
        count = len(simplified)
        diff = abs(count - target_rdp_len)
        if diff < best_diff:
            best_diff = diff
            best_epsilon = mid
        if count == target_rdp_len:
            return mid
        if count > target_rdp_len:
            low = mid
        else:
            high = mid
    return best_epsilon

def calculate_smooth_tangents(points, tension=0.35, is_closed=True):
    tangents = []
    n = len(points)
    if n < 2:
        return [((0.0, 0.0), (0.0, 0.0))] * n
    for i in range(n):
        if is_closed:
            prev_pt = points[(i - 1 + n) % n]
            next_pt = points[(i + 1) % n]
        else:
            prev_pt = points[0] if i == 0 else points[i - 1]
            next_pt = points[n - 1] if i == n - 1 else points[i + 1]
        vec_x = next_pt[0] - prev_pt[0]
        vec_y = next_pt[1] - prev_pt[1]
        tan_x = vec_x * tension
        tan_y = vec_y * tension
        tangents.append(((-float(tan_x), -float(tan_y)), (float(tan_x), float(tan_y))))
    return tangents

def find_parent_layer(shape, layer):
    for item in layer:
        if item.name == shape.name:
            return layer
        if isinstance(item, rp.Layer):
            found = find_parent_layer(shape, item)
            if found:
                return found
    return None

def find_target_roto_node(node_name=None):
    if node_name:
        n = nuke.toNode(node_name)
        if n:
            inp = n.input(0)
            if inp and inp.Class() in ('Roto', 'RotoPaint'):
                return inp
    selected = nuke.selectedNodes()
    for n in selected:
        if n.Class() in ('Roto', 'RotoPaint'):
            return n
    all_nodes = nuke.allNodes()
    for n in all_nodes:
        if n.Class() in ('Roto', 'RotoPaint') and n.isSelected():
            return n
    roto_nodes = [n for n in all_nodes if n.Class() in ('Roto', 'RotoPaint')]
    if len(roto_nodes) == 1:
        return roto_nodes[0]
    return None

def _collect_shapes(items):
    shapes = []
    for item in items:
        if isinstance(item, rp.Shape):
            shapes.append(item)
        elif isinstance(item, rp.Layer):
            shapes.extend(_collect_shapes(item))
    return shapes

def _do_run_simplify(caller_node_name):
    caller = nuke.toNode(caller_node_name)
    if not caller:
        nuke.message("Error: Could not locate calling RotoSimplifier node.")
        return

    strategy = int(caller['strategy'].getValue())
    target_val = int(caller['target_count'].getValue())
    percent_val = float(caller['percentage'].getValue())
    epsilon_val = float(caller['epsilon'].getValue())
    tension = float(caller['tension'].getValue())
    do_bake = bool(caller['bake'].getValue())
    f_start = int(caller['start_frame'].getValue())
    f_end = int(caller['end_frame'].getValue())
    do_new_node = bool(caller['new_node'].getValue())

    roto_node = find_target_roto_node(caller_node_name)
    if not roto_node:
        nuke.message("Please select or connect a Roto or RotoPaint node.")
        return

    curves_knob = roto_node['curves']
    root_layer = curves_knob.rootLayer

    selected_items = curves_knob.getSelected()
    shapes_to_process = _collect_shapes(selected_items)
    if not shapes_to_process:
        all_root_shapes = _collect_shapes(root_layer)
        if len(all_root_shapes) == 1:
            shapes_to_process = all_root_shapes
        else:
            nuke.message(f"No shape selected inside '{roto_node.name()}'.\nPlease select one or more shapes in the Roto curve editor or viewer.")
            return

    current_frame = nuke.frame()
    frame_range = range(f_start, f_end + 1) if (do_bake and f_end >= f_start) else [current_frame]

    target_knob = curves_knob
    target_layer = root_layer

    if do_new_node:
        for n in nuke.allNodes():
            n.setSelected(False)
        new_roto = nuke.createNode("Roto")
        new_roto.setName(roto_node.name() + "_Simplified")
        new_roto.setXYpos(roto_node.xpos() + 120, roto_node.ypos())
        target_knob = new_roto['curves']
        target_layer = target_knob.rootLayer

    count_processed = 0
    use_progress = (do_bake and len(frame_range) > 1)
    task = nuke.ProgressTask("Simplifying Roto Shapes") if use_progress else None

    try:
        for idx, shape in enumerate(shapes_to_process):
            if task and task.isCancelled():
                break

            points_data = []
            for i in range(len(shape)):
                pt_main = shape[i].center.getPosition(current_frame)
                pt_feather = shape[i].featherCenter.getPosition(current_frame)
                points_data.append((i, float(pt_main[0]), float(pt_main[1]), float(pt_feather[0]), float(pt_feather[1])))

            if len(points_data) < 3:
                continue

            try:
                is_open_val = shape.getAttributes().getValue(current_frame, "op")
            except Exception:
                is_open_val = 0.0
            is_closed = (is_open_val == 0.0)

            if is_closed:
                points_data.append(points_data[0])

            if strategy == 0:  # Target Count
                calc_epsilon = solve_epsilon_for_target(points_data, target_val, is_closed)
                simplified_data = ramer_douglas_peucker_py(points_data, calc_epsilon)
            elif strategy == 1:  # Percentage
                original_count = len(shape)
                keep_ratio = max(0.01, 1.0 - (percent_val / 100.0))
                calc_target = max(3, int(round(original_count * keep_ratio)))
                calc_epsilon = solve_epsilon_for_target(points_data, calc_target, is_closed)
                simplified_data = ramer_douglas_peucker_py(points_data, calc_epsilon)
            else:  # Epsilon
                simplified_data = ramer_douglas_peucker_py(points_data, epsilon_val)

            if is_closed and len(simplified_data) > 2:
                simplified_data.pop()

            indices_to_track = [p[0] for p in simplified_data]
            if len(indices_to_track) < 2:
                continue

            parent = target_layer
            if not do_new_node:
                parent = find_parent_layer(shape, root_layer) or root_layer

            new_shape = rp.Shape(target_knob)
            new_shape.name = shape.name + ("_simplified" if do_new_node else "_simp")

            for _ in indices_to_track:
                new_shape.append(rp.AnimControlPoint(0, 0))

            total_frames = len(frame_range)
            for f_idx, frame in enumerate(frame_range):
                if task:
                    progress = int(((f_idx + 1) / total_frames) * 100)
                    task.setMessage(f"Baking frame {frame} ({idx+1}/{len(shapes_to_process)})")
                    task.setProgress(progress)

                frame_main_pts = []
                frame_feather_pts = []
                for i in indices_to_track:
                    orig_pt = shape[i].center.getPosition(frame)
                    orig_f_pt = shape[i].featherCenter.getPosition(frame)
                    frame_main_pts.append((orig_pt[0], orig_pt[1]))
                    frame_feather_pts.append((orig_f_pt[0], orig_f_pt[1]))

                frame_tangents = calculate_smooth_tangents(frame_main_pts, tension, is_closed)

                for k, new_pt in enumerate(new_shape):
                    mx, my = frame_main_pts[k]
                    fx, fy = frame_feather_pts[k]
                    left_tan = frame_tangents[k][0]
                    right_tan = frame_tangents[k][1]

                    new_pt.center.addKey(frame, (mx, my))
                    new_pt.featherCenter.addKey(frame, (fx, fy))
                    try:
                        new_pt.leftTangent.addKey(frame, left_tan)
                        new_pt.rightTangent.addKey(frame, right_tan)
                    except Exception:
                        pass

            try:
                new_shape.getAttributes().set(current_frame, "op", is_open_val)
            except Exception:
                pass

            parent.append(new_shape)
            count_processed += 1
    finally:
        if task:
            del task

    target_knob.changed()
    nuke.message(f"RotoSimplifier: Successfully simplified {count_processed} shape(s).")

def run_simplify_from_node(caller_node_name):
    try:
        _do_run_simplify(caller_node_name)
    except Exception as e:
        import traceback
        traceback.print_exc()
        nuke.message(f"RotoSimplifier Error:\n{e}")

def _do_run_explode(caller_node_name):
    import tempfile
    import os

    caller = nuke.toNode(caller_node_name)
    roto_node = find_target_roto_node(caller_node_name)
    if not roto_node:
        nuke.message("Please select or connect a Roto or RotoPaint node.")
        return

    curves_knob = roto_node['curves']
    root_layer = curves_knob.rootLayer

    mode = int(caller['depth_mode'].getValue()) if caller else 0
    if mode == 0:
        max_depth = 1
    elif mode == 1:
        max_depth = 99
    else:
        max_depth = int(caller['custom_depth'].getValue()) if caller else 2

    target_layers = []
    def scan_layers(layer, current_depth):
        if current_depth > max_depth:
            return
        if isinstance(layer, rp.Layer):
            if current_depth > 0:
                target_layers.append(layer.name)
            for item in layer:
                scan_layers(item, current_depth + 1)

    scan_layers(root_layer, 0)
    target_layers = sorted(list(set(target_layers)))
    if not target_layers:
        nuke.message("No layers found at this depth.")
        return

    fd, tmp_file = tempfile.mkstemp(suffix=".nk")
    os.close(fd)

    use_progress = len(target_layers) > 1
    task = nuke.ProgressTask("Exploding Layers") if use_progress else None
    x_pos = roto_node.xpos()
    y_pos = roto_node.ypos()
    created_count = 0

    try:
        for n in nuke.allNodes():
            n.setSelected(False)
        roto_node.setSelected(True)
        nuke.nodeCopy(tmp_file)
        roto_node.setSelected(False)

        for i, layer_name in enumerate(target_layers):
            if task and task.isCancelled():
                break
            if task:
                task.setMessage(f"Exploding {layer_name}...")
                task.setProgress(int(((i + 1) / len(target_layers)) * 100))

            for n in nuke.allNodes():
                n.setSelected(False)

            nuke.nodePaste(tmp_file)
            pasted = [n for n in nuke.selectedNodes() if n.Class() in ('Roto', 'RotoPaint')]
            if not pasted:
                continue
            new_node = pasted[0]
            new_node.setXYpos(x_pos + ((i + 1) * 120), y_pos)
            new_node.setName(f"{layer_name}_Node")
            new_node.setInput(0, None)

            new_knob = new_node['curves']
            new_root = new_knob.rootLayer

            def layer_contains_target(layer, target_name):
                for item in layer:
                    if item.name == target_name:
                        return True
                    if isinstance(item, rp.Layer):
                        if layer_contains_target(item, target_name):
                            return True
                return False

            def filter_tree(layer_obj):
                for j in range(len(layer_obj) - 1, -1, -1):
                    item = layer_obj[j]
                    if item.name == layer_name:
                        continue
                    if isinstance(item, rp.Layer):
                        if layer_contains_target(item, layer_name):
                            filter_tree(item)
                            continue
                    try:
                        layer_obj.remove(j)
                    except Exception:
                        pass

            filter_tree(new_root)
            new_knob.changed()
            new_node.setSelected(False)
            created_count += 1
    finally:
        if task:
            del task
        if os.path.exists(tmp_file):
            try:
                os.remove(tmp_file)
            except Exception:
                pass

    nuke.message(f"RotoSimplifier: Successfully exploded {created_count} node(s).")

def run_explode_from_node(caller_node_name):
    try:
        _do_run_explode(caller_node_name)
    except Exception as e:
        import traceback
        traceback.print_exc()
        nuke.message(f"RotoSimplifier Explode Error:\n{e}")
)PY";

// Helper to ensure Python environment is initialized with roto_native & embedded logic
static void ensure_python_initialized() {
    static bool s_initialized = false;
    if (s_initialized) return;

    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject* sys_modules = PyImport_GetModuleDict();
    if (sys_modules && !PyDict_GetItemString(sys_modules, "roto_native")) {
        PyObject* mod = init_roto_native_in_memory();
        if (mod) {
            PyDict_SetItemString(sys_modules, "roto_native", mod);
            Py_DECREF(mod);
        }
    }

    PyRun_SimpleString(kEmbeddedPythonCode);
    s_initialized = true;

    PyGILState_Release(gstate);
}

// ==============================================================================
// 4. NUKE NODE IMPLEMENTATION (RotoSimplifier)
// ==============================================================================

class RotoSimplifier : public NoIop {
    int strategy_;
    int target_count_;
    double percent_;
    double epsilon_;
    double tension_;
    bool bake_animation_;
    int start_frame_;
    int end_frame_;
    bool new_node_;

    int depth_mode_;
    int custom_depth_;

public:
    explicit RotoSimplifier(Node* node)
        : NoIop(node)
        , strategy_(0)
        , target_count_(10)
        , percent_(50.0)
        , epsilon_(2.0)
        , tension_(0.35)
        , bake_animation_(false)
        , start_frame_(1)
        , end_frame_(100)
        , new_node_(false)
        , depth_mode_(0)
        , custom_depth_(2)
    {
    }

    void knobs(Knob_Callback) override;
    int knob_changed(Knob*) override;
    void _validate(bool) override;

    [[nodiscard]] const char* Class() const override { return CLASS; }
    [[nodiscard]] const char* node_help() const override { return HELP; }
    static const Op::Description description;
};

static Op* build(Node* node) {
    return new RotoSimplifier(node);
}

const Op::Description RotoSimplifier::description(CLASS, build);

void RotoSimplifier::_validate(bool for_real) {
    copy_info();
}

void RotoSimplifier::knobs(Knob_Callback f) {
    ensure_python_initialized();

    static const char* const strategies[] = { "Target Count", "Percentage", "Epsilon", nullptr };
    Enumeration_knob(f, &strategy_, strategies, "strategy", "strategy");
    SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Choose reduction method: target control point count, percentage reduction, or pixel distance epsilon.");

    Int_knob(f, &target_count_, "target_count", "target count");
    SetRange(f, 3, 200);
    Tooltip(f, "Desired number of points on the simplified shape.");

    Double_knob(f, &percent_, "percentage", "percentage");
    SetRange(f, 1.0, 99.0);
    Tooltip(f, "Percentage of points to remove.");

    Double_knob(f, &epsilon_, "epsilon", "epsilon");
    SetRange(f, 0.1, 50.0);
    Tooltip(f, "Max pixel tolerance distance for Ramer-Douglas-Peucker reduction.");

    Divider(f, "Smoothing & Output");

    Double_knob(f, &tension_, "tension", "tension");
    SetRange(f, 0.0, 1.0);
    Tooltip(f, "Smooth tangent tension (0.0 = sharp corners, 0.35-0.5 = smooth curves).");

    Bool_knob(f, &bake_animation_, "bake", "bake animation");
    SetFlags(f, Knob::STARTLINE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "When enabled, evaluates and bakes the simplified point indices across the specified frame range.");

    Int_knob(f, &start_frame_, "start_frame", "start frame");
    Tooltip(f, "First frame of range to bake animation.");

    Int_knob(f, &end_frame_, "end_frame", "end frame");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Last frame of range to bake animation.");

    Bool_knob(f, &new_node_, "new_node", "output to new node");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Create a separate new Roto node instead of modifying the existing shape.");

    PyScript_knob(f, "run_simplify_from_node(nuke.thisNode().name())", "btn_simplify", "Simplify Selected Shapes");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Executes the C++ RDP reduction on the currently selected shapes in the connected or active Roto node.");

    // Explode Tab
    Tab_knob(f, "Explode");

    static const char* const depths[] = { "Top Level Only (Depth 1)", "Infinite Recursive (All Sub-Folders)", "Custom Depth Level", nullptr };
    Enumeration_knob(f, &depth_mode_, depths, "depth_mode", "depth level");
    SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Select how deep in the folder hierarchy layers will be exploded.");

    Int_knob(f, &custom_depth_, "custom_depth", "custom depth");
    SetRange(f, 1, 10);
    Tooltip(f, "Specific level depth when using Custom Depth Level.");

    PyScript_knob(f, "run_explode_from_node(nuke.thisNode().name())", "btn_explode", "Explode Layers");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Splits all layers in the Roto node into separate, disconnected Roto nodes.");
}

int RotoSimplifier::knob_changed(Knob* k) {
    if (k == &Knob::showPanel || k->is("strategy")) {
        int strat = static_cast<int>(k->is("strategy") ? k->get_value() : (knob("strategy") ? knob("strategy")->get_value() : strategy_));
        if (Knob* k_count = knob("target_count")) k_count->enable(strat == 0);
        if (Knob* k_perc = knob("percentage")) k_perc->enable(strat == 1);
        if (Knob* k_eps = knob("epsilon")) k_eps->enable(strat == 2);
        if (k->is("strategy")) return 1;
    }

    if (k == &Knob::showPanel || k->is("bake")) {
        bool bake = (k->is("bake") ? (k->get_value() != 0.0) : (knob("bake") ? (knob("bake")->get_value() != 0.0) : bake_animation_));
        if (Knob* k_sf = knob("start_frame")) k_sf->enable(bake);
        if (Knob* k_ef = knob("end_frame")) k_ef->enable(bake);
        if (k->is("bake")) return 1;
    }

    if (k == &Knob::showPanel || k->is("depth_mode")) {
        int depth = static_cast<int>(k->is("depth_mode") ? k->get_value() : (knob("depth_mode") ? knob("depth_mode")->get_value() : depth_mode_));
        if (Knob* k_cd = knob("custom_depth")) k_cd->enable(depth == 2);
        if (k->is("depth_mode")) return 1;
    }

    if (k == &Knob::showPanel) {
        return 1;
    }

    return NoIop::knob_changed(k);
}
