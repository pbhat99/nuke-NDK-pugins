//
// RotoReducer - Dedicated Nuke C++ Plugin Node
// Reduces control points on Roto shapes using Schneider Bezier Fitting,
// Iterative Greedy Decimation, or Spatio-Temporal Envelope RDP,
// strictly preserving constant point topology across animation and matching feather tangents.
//

#include <Python.h>
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>

using namespace DD::Image;

static const char* const CLASS = "RotoReducer";
static const char* const HELP = 
    "RotoReducer - Reduces control points on Roto shapes using Schneider Bezier Fitting,\n"
    "Iterative Greedy Decimation, or Spatio-Temporal Envelope RDP,\n"
    "preserving constant point topology across animation, matching feather tangents, and creating a new Roto node.";

// ==============================================================================
// 1. C++ RDP CORE ALGORITHM (Main + Feather Aware)
// ==============================================================================

struct RotoPoint {
    double x;
    double y;
    double fx;
    double fy;
    int index;
};

inline double get_point_line_sq_dist(double px, double py, double ax, double ay, double bx, double by) {
    double sx = bx - ax;
    double sy = by - ay;
    double norm = sx * sx + sy * sy;

    if (norm < 1e-10) {
        double dx = px - ax;
        double dy = py - ay;
        return dx * dx + dy * dy;
    }

    double u = ((px - ax) * sx + (py - ay) * sy) / norm;
    if (u > 1.0) u = 1.0;
    else if (u < 0.0) u = 0.0;

    double x = ax + u * sx;
    double y = ay + u * sy;
    double dx = x - px;
    double dy = y - py;
    return dx * dx + dy * dy;
}

inline double get_sq_dist(const RotoPoint& p, const RotoPoint& a, const RotoPoint& b) {
    double d_main = get_point_line_sq_dist(p.x, p.y, a.x, a.y, b.x, b.y);
    double d_feather = get_point_line_sq_dist(p.fx, p.fy, a.fx, a.fy, b.fx, b.fy);
    return (d_main > d_feather) ? d_main : d_feather;
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

static void rdp_closed(const std::vector<RotoPoint>& points, double epsilon_sq, std::vector<RotoPoint>& out) {
    if (points.size() <= 3) {
        out = points;
        return;
    }

    size_t k = 0;
    double max_dist_sq = -1.0;
    for (size_t i = 1; i < points.size(); ++i) {
        double dx = points[i].x - points[0].x;
        double dy = points[i].y - points[0].y;
        double dfx = points[i].fx - points[0].fx;
        double dfy = points[i].fy - points[0].fy;
        double d_main = dx * dx + dy * dy;
        double d_feather = dfx * dfx + dfy * dfy;
        double d = (d_main > d_feather) ? d_main : d_feather;
        if (d > max_dist_sq) {
            max_dist_sq = d;
            k = i;
        }
    }

    if (k == 0 || max_dist_sq < 1e-10) {
        out = points;
        return;
    }

    std::vector<RotoPoint> chain1(points.begin(), points.begin() + k + 1);
    std::vector<RotoPoint> chain2(points.begin() + k, points.end());
    chain2.push_back(points[0]);

    std::vector<RotoPoint> res1, res2;
    rdp_recursive(chain1, epsilon_sq, res1);
    rdp_recursive(chain2, epsilon_sq, res2);

    out.clear();
    out.reserve(res1.size() + res2.size());
    out.insert(out.end(), res1.begin(), res1.end() - 1);
    out.insert(out.end(), res2.begin(), res2.end() - 1);

    if (out.size() < 3) {
        size_t best_third = (k > 1) ? 1 : 2;
        double best_d = -1.0;
        for (size_t i = 1; i < points.size(); ++i) {
            if (i == k) continue;
            double d = get_sq_dist(points[i], points[0], points[k]);
            if (d > best_d) {
                best_d = d;
                best_third = i;
            }
        }
        out.clear();
        out.push_back(points[0]);
        if (best_third < k) {
            out.push_back(points[best_third]);
            out.push_back(points[k]);
        } else {
            out.push_back(points[k]);
            out.push_back(points[best_third]);
        }
    }
}

// Binary search for target count
static std::vector<RotoPoint> solve_for_count(const std::vector<RotoPoint>& points, int target_count, bool is_closed) {
    int min_count = is_closed ? 3 : 2;
    if (target_count < min_count) target_count = min_count;
    if (target_count >= (int)points.size()) return points;

    double low = 0.0;
    double high = 5000.0;
    double best_eps = low;
    int best_diff = 999999;
    std::vector<RotoPoint> best_result;

    for (int iter = 0; iter < 32; ++iter) {
        double mid = (low + high) * 0.5;
        double eps_sq = mid * mid;
        std::vector<RotoPoint> current;

        if (is_closed) {
            rdp_closed(points, eps_sq, current);
        } else {
            rdp_recursive(points, eps_sq, current);
        }

        int count = (int)current.size();
        int diff = std::abs(count - target_count);

        if (diff < best_diff) {
            best_diff = diff;
            best_eps = mid;
            best_result = current;
        }

        if (count == target_count) {
            return current;
        }

        if (count > target_count) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return best_result;
}

// ==============================================================================
// 2. EMBEDDED PYTHON MODULE DEFINITION (roto_native)
// ==============================================================================

static PyObject* py_simplify(PyObject* self, PyObject* args) {
    PyObject* pyPoints = nullptr;
    int mode = 0;
    double target = 0.0;
    int isClosed = 1;

    if (!PyArg_ParseTuple(args, "Oidi", &pyPoints, &mode, &target, &isClosed)) {
        return nullptr;
    }

    if (!PyList_Check(pyPoints)) {
        PyErr_SetString(PyExc_TypeError, "points must be a list");
        return nullptr;
    }

    Py_ssize_t n = PyList_Size(pyPoints);
    std::vector<RotoPoint> points;
    points.reserve(n);

    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject* item = PyList_GetItem(pyPoints, i);
        if (!PyTuple_Check(item) && !PyList_Check(item)) continue;

        RotoPoint p;
        p.index = (int)PyLong_AsLong(PySequence_GetItem(item, 0));
        p.x = PyFloat_AsDouble(PySequence_GetItem(item, 1));
        p.y = PyFloat_AsDouble(PySequence_GetItem(item, 2));

        if (PySequence_Size(item) >= 5) {
            p.fx = PyFloat_AsDouble(PySequence_GetItem(item, 3));
            p.fy = PyFloat_AsDouble(PySequence_GetItem(item, 4));
        } else {
            p.fx = p.x;
            p.fy = p.y;
        }
        points.push_back(p);
    }

    if (points.size() < 3) {
        Py_INCREF(pyPoints);
        return pyPoints;
    }

    std::vector<RotoPoint> result;

    if (mode == 0) { // Tolerance (epsilon)
        double eps_sq = target * target;
        if (isClosed) {
            rdp_closed(points, eps_sq, result);
        } else {
            rdp_recursive(points, eps_sq, result);
        }
    } else if (mode == 1) { // Target Count
        result = solve_for_count(points, (int)target, isClosed != 0);
    } else if (mode == 2) { // Percentage
        double pct = std::max(0.0, std::min(100.0, target));
        int target_k = (int)std::round(points.size() * ((100.0 - pct) / 100.0));
        result = solve_for_count(points, target_k, isClosed != 0);
    }

    PyObject* pyResult = PyList_New(result.size());
    for (size_t i = 0; i < result.size(); ++i) {
        PyObject* tuple = PyTuple_New(5);
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(result[i].index));
        PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(result[i].x));
        PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(result[i].y));
        PyTuple_SetItem(tuple, 3, PyFloat_FromDouble(result[i].fx));
        PyTuple_SetItem(tuple, 4, PyFloat_FromDouble(result[i].fy));
        PyList_SetItem(pyResult, i, tuple);
    }

    return pyResult;
}

static PyMethodDef RotoNativeMethods[] = {
    {"simplify", py_simplify, METH_VARARGS, "Runs combined main+feather RDP simplification on points."},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef roto_native_module = {
    PyModuleDef_HEAD_INIT, "roto_native", nullptr, -1, RotoNativeMethods
};

static PyObject* init_roto_native_in_memory() {
    return PyModule_Create(&roto_native_module);
}

// ==============================================================================
// 3. EMBEDDED PYTHON LOGIC FOR ROTOPAINT & REDUCE
// ==============================================================================

static const char* kEmbeddedPythonCode = R"PY(
try:
    import roto_reducer
except Exception:
    pass
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
// 4. NUKE NODE IMPLEMENTATION (RotoReducer)
// ==============================================================================

class RotoReducer : public NoIop {
    int algorithm_;
    float percent_;

public:
    explicit RotoReducer(Node* node)
        : NoIop(node)
        , algorithm_(0)
        , percent_(50.0f)
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
    return new RotoReducer(node);
}

const Op::Description RotoReducer::description(CLASS, build);

void RotoReducer::_validate(bool for_real) {
    copy_info();
}

void RotoReducer::knobs(Knob_Callback f) {
    ensure_python_initialized();

    static const char* const kAlgorithms[] = {
        "Schneider Bezier Fit (Least-Squares)",
        "Iterative Greedy Decimation",
        "Spatio-Temporal Envelope RDP",
        nullptr
    };
    Enumeration_knob(f, &algorithm_, kAlgorithms, "algorithm", "algorithm");
    Tooltip(f, "Select reduction algorithm:\n\n"
               "• Schneider Bezier Fit (Least-Squares) [Recommended]: Best for smooth curves, circles, ellipses, and continuous organic contours. Fits cubic Bezier handles using mathematical 2x2 linear least-squares against original curve points to maximize curvature fidelity.\n\n"
               "• Iterative Greedy Decimation: Best for complex deforming shapes with mixed curves and corners. Iteratively simulates removing each point, evaluates deformation error across all keyframes, and removes vertices with lowest error while preserving cusps.\n\n"
               "• Spatio-Temporal Envelope RDP: Best for hard-surface shapes, sharp corners, and cusps. Uses minimax chord distance across all keyframes to strictly preserve corner peaks and peak articulators.");

    Float_knob(f, &percent_, "percent", "reduction (%)");
    SetRange(f, 0.0, 100.0);
    Tooltip(f, "Percentage of points to reduce (0-100%). For example, 50% reduces points by half, 80% retains only 20% of points, 0% keeps all points.");

    Divider(f, "");

    PyScript_knob(f, "import roto_reducer\nroto_reducer.run_reduce_from_node(nuke.thisNode().name())", "btn_reduce", "Reduce Shapes");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Reduces control points by the specified percentage across all keyframed frames using the selected algorithm, preserving Nuke's constant point topology, and always creating a new Roto node.");
}

int RotoReducer::knob_changed(Knob* k) {
    if (k == &Knob::showPanel) {
        return 1;
    }
    return NoIop::knob_changed(k);
}
