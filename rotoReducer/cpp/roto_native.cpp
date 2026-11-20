/* ROTO SIMPLIFIER NATIVE ACCELERATOR
   Compile this against Nuke's python include directory.
*/

#include <Python.h>
#include <vector>
#include <cmath>
#include <algorithm>

// --- CORE MATH STRUCTS ---
struct Point {
    double x;
    double y;
    double fx;
    double fy;
    int index;
};

// --- 1. OPTIMIZED MATH FUNCTIONS ---
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

inline double get_sq_dist(const Point& p, const Point& a, const Point& b) {
    double d_main = get_point_line_sq_dist(p.x, p.y, a.x, a.y, b.x, b.y);
    double d_feather = get_point_line_sq_dist(p.fx, p.fy, a.fx, a.fy, b.fx, b.fy);
    return (d_main > d_feather) ? d_main : d_feather;
}

void rdp_recursive(const std::vector<Point>& points, double epsilon_sq, std::vector<Point>& out) {
    if (points.size() < 3) {
        for (const auto& p : points) out.push_back(p);
        return;
    }

    double dmax = 0.0;
    int index = 0;
    int end = static_cast<int>(points.size()) - 1;

    for (int i = 1; i < end; i++) {
        double d = get_sq_dist(points[i], points[0], points[end]);
        if (d > dmax) {
            index = i;
            dmax = d;
        }
    }

    if (dmax > epsilon_sq) {
        std::vector<Point> res1, res2;
        std::vector<Point> left(points.begin(), points.begin() + index + 1);
        std::vector<Point> right(points.begin() + index, points.end());

        rdp_recursive(left, epsilon_sq, res1);
        rdp_recursive(right, epsilon_sq, res2);

        out.insert(out.end(), res1.begin(), res1.end() - 1);
        out.insert(out.end(), res2.begin(), res2.end());
    }
    else {
        out.push_back(points[0]);
        out.push_back(points[end]);
    }
}

void rdp_closed(const std::vector<Point>& points, double epsilon_sq, std::vector<Point>& out) {
    if (points.size() <= 3) {
        out = points;
        return;
    }

    // 1. Find antipodal point k maximizing distance on main or feather
    size_t k = 0;
    double max_dist_sq = -1.0;
    for (size_t i = 1; i < points.size(); ++i) {
        double dx = points[i].x - points[0].x;
        double dy = points[i].y - points[0].y;
        double d_sq_m = dx * dx + dy * dy;

        double fdx = points[i].fx - points[0].fx;
        double fdy = points[i].fy - points[0].fy;
        double d_sq_f = fdx * fdx + fdy * fdy;

        double d_sq = (d_sq_m > d_sq_f) ? d_sq_m : d_sq_f;
        if (d_sq > max_dist_sq) {
            max_dist_sq = d_sq;
            k = i;
        }
    }

    if (k == 0 || max_dist_sq < 1e-10) {
        out = points;
        return;
    }

    std::vector<Point> chain1(points.begin(), points.begin() + k + 1);
    std::vector<Point> chain2(points.begin() + k, points.end());
    chain2.push_back(points[0]);

    std::vector<Point> res1, res2;
    rdp_recursive(chain1, epsilon_sq, res1);
    rdp_recursive(chain2, epsilon_sq, res2);

    out.clear();
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
        if (best_third < k) {
            out.push_back(points[0]);
            out.push_back(points[best_third]);
            out.push_back(points[k]);
        } else {
            out.push_back(points[0]);
            out.push_back(points[k]);
            out.push_back(points[best_third]);
        }
    }
}

// --- 2. PYTHON BINDING INTERFACE ---

// Python: roto_native.simplify( list_of_tuples, epsilon, is_closed=False )
static PyObject* py_simplify(PyObject* self, PyObject* args) {
    PyObject* pList = NULL;
    double epsilon = 0.0;
    int is_closed = -1;

    if (!PyArg_ParseTuple(args, "O!d|p", &PyList_Type, &pList, &epsilon, &is_closed)) {
        return NULL;
    }

    Py_ssize_t num_points = PyList_Size(pList);
    if (num_points == 0) {
        return PyList_New(0);
    }

    std::vector<Point> points;
    points.reserve(num_points);

    for (Py_ssize_t i = 0; i < num_points; i++) {
        PyObject* item = PyList_GetItem(pList, i);

        if (!PyTuple_Check(item) || PyTuple_Size(item) < 3) {
            PyErr_SetString(PyExc_TypeError, "Expected list of tuples with at least (index, x, y)");
            return NULL;
        }

        long idx = PyLong_AsLong(PyTuple_GetItem(item, 0));
        if (idx == -1 && PyErr_Occurred()) return NULL;

        double x = PyFloat_AsDouble(PyTuple_GetItem(item, 1));
        if (x == -1.0 && PyErr_Occurred()) return NULL;

        double y = PyFloat_AsDouble(PyTuple_GetItem(item, 2));
        if (y == -1.0 && PyErr_Occurred()) return NULL;

        double fx = x;
        double fy = y;
        if (PyTuple_Size(item) >= 5) {
            fx = PyFloat_AsDouble(PyTuple_GetItem(item, 3));
            if (fx == -1.0 && PyErr_Occurred()) return NULL;

            fy = PyFloat_AsDouble(PyTuple_GetItem(item, 4));
            if (fy == -1.0 && PyErr_Occurred()) return NULL;
        }

        points.push_back({ x, y, fx, fy, static_cast<int>(idx) });
    }

    bool closed_flag = false;
    if (is_closed == 1) {
        closed_flag = true;
    } else if (is_closed == 0) {
        closed_flag = false;
    } else {
        if (points.size() > 2 &&
            points.front().index == points.back().index &&
            std::abs(points.front().x - points.back().x) < 1e-4 &&
            std::abs(points.front().y - points.back().y) < 1e-4) {
            closed_flag = true;
            points.pop_back();
        }
    }

    std::vector<Point> result;
    result.reserve(num_points);

    double eps_sq = epsilon * epsilon;
    if (closed_flag && points.size() > 2) {
        rdp_closed(points, eps_sq, result);
    } else {
        rdp_recursive(points, eps_sq, result);
    }

    PyObject* pyResult = PyList_New(result.size());
    if (!pyResult) return NULL;

    for (size_t i = 0; i < result.size(); i++) {
        PyObject* tuple = PyTuple_New(5);
        if (!tuple) {
            Py_DECREF(pyResult);
            return NULL;
        }

        PyTuple_SetItem(tuple, 0, PyLong_FromLong(result[i].index));
        PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(result[i].x));
        PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(result[i].y));
        PyTuple_SetItem(tuple, 3, PyFloat_FromDouble(result[i].fx));
        PyTuple_SetItem(tuple, 4, PyFloat_FromDouble(result[i].fy));
        PyList_SetItem(pyResult, i, tuple);
    }

    return pyResult;
}

// Module Definition
static PyMethodDef RotoMethods[] = {
    {"simplify", py_simplify, METH_VARARGS, "Runs combined main+feather RDP simplification on points."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef rotomodule = {
    PyModuleDef_HEAD_INIT, "roto_native", NULL, -1, RotoMethods
};

PyMODINIT_FUNC PyInit_roto_native(void) {
    return PyModule_Create(&rotomodule);
}