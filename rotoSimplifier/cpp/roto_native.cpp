/* ROTO SIMPLIFIER NATIVE ACCELERATOR
   Compile this against Nuke's python include directory.
*/

// Add the Python include directory to your project's include paths.
// For example, if Python is installed at C:\Python314, add:
// C:\Python314\include
// to your project's C/C++ "Additional Include Directories" in Visual Studio.

// The code itself does not need to change, but ensure this line remains:
#include <Python.h>
#include <vector>

// --- CORE MATH STRUCTS ---
struct Point { double x, y; int index; };

// --- 1. OPTIMIZED MATH FUNCTIONS ---
inline double get_sq_dist(Point p, Point a, Point b) {
    double px = b.x - a.x;
    double py = b.y - a.y;
    double norm = px * px + py * py;

    // FIX: Prevent division by zero when points are identical
    if (norm < 1e-10) return 0.0;

    double u = ((p.x - a.x) * px + (p.y - a.y) * py) / norm;
    if (u > 1) u = 1; else if (u < 0) u = 0;
    double x = a.x + u * px;
    double y = a.y + u * py;
    double dx = x - p.x;
    double dy = y - p.y;
    return dx * dx + dy * dy;
}

void rdp_recursive(const std::vector<Point>& points, double epsilon, std::vector<Point>& out) {
    if (points.size() < 3) {
        for (const auto& p : points) out.push_back(p);
        return;
    }

    double dmax = 0.0;
    int index = 0;
    int end = points.size() - 1;

    for (int i = 1; i < end; i++) {
        double d = get_sq_dist(points[i], points[0], points[end]);
        if (d > dmax) {
            index = i;
            dmax = d;
        }
    }

    if (dmax > epsilon) {
        std::vector<Point> res1, res2;
        // Recursive calls
        std::vector<Point> left(points.begin(), points.begin() + index + 1);
        std::vector<Point> right(points.begin() + index, points.end());

        rdp_recursive(left, epsilon, res1);
        rdp_recursive(right, epsilon, res2);

        out.insert(out.end(), res1.begin(), res1.end() - 1);
        out.insert(out.end(), res2.begin(), res2.end());
    }
    else {
        out.push_back(points[0]);
        out.push_back(points[end]);
    }
}

// --- 2. PYTHON BINDING INTERFACE ---

// Python: roto_native.simplify( list_of_tuples, epsilon )
static PyObject* py_simplify(PyObject* self, PyObject* args) {
    PyObject* pList;
    double epsilon;

    // Parse Input: A List and a Float
    if (!PyArg_ParseTuple(args, "O!d", &PyList_Type, &pList, &epsilon)) {
        return NULL;
    }

    // Convert Python List -> C++ Vector
    Py_ssize_t num_points = PyList_Size(pList);  // FIX: Use Py_ssize_t instead of int
    std::vector<Point> points;
    points.reserve(num_points);

    for (Py_ssize_t i = 0; i < num_points; i++) {
        PyObject* item = PyList_GetItem(pList, i); // Tuple (index, x, y)

        // FIX: Validate tuple structure (supports 3 or more elements, e.g. index, x, y, fx, fy)
        if (!PyTuple_Check(item) || PyTuple_Size(item) < 3) {
            PyErr_SetString(PyExc_TypeError, "Expected list of tuples with at least (index, x, y)");
            return NULL;
        }

        // FIX: Extract index as long (not float) with error checking
        long idx = PyLong_AsLong(PyTuple_GetItem(item, 0));
        if (idx == -1 && PyErr_Occurred()) {
            return NULL;
        }

        // FIX: Extract x with error checking
        double x = PyFloat_AsDouble(PyTuple_GetItem(item, 1));
        if (x == -1.0 && PyErr_Occurred()) {
            return NULL;
        }

        // FIX: Extract y with error checking
        double y = PyFloat_AsDouble(PyTuple_GetItem(item, 2));
        if (y == -1.0 && PyErr_Occurred()) {
            return NULL;
        }

        points.push_back({ x, y, (int)idx });
    }

    // RUN ALGORITHM (Squared epsilon for fast dist comparison)
    std::vector<Point> result;
    result.reserve(num_points);  // FIX: Pre-allocate to avoid reallocations
    rdp_recursive(points, epsilon * epsilon, result);

    // Convert C++ Vector -> Python List
    PyObject* pyResult = PyList_New(result.size());
    if (!pyResult) {  // FIX: Check for allocation failure
        return NULL;
    }

    for (size_t i = 0; i < result.size(); i++) {  // FIX: Use size_t for consistency
        PyObject* tuple = PyTuple_New(3);
        if (!tuple) {  // FIX: Check for allocation failure
            Py_DECREF(pyResult);
            return NULL;
        }

        // FIX: Return index as PyLong (not PyFloat)
        PyTuple_SetItem(tuple, 0, PyLong_FromLong(result[i].index));
        PyTuple_SetItem(tuple, 1, PyFloat_FromDouble(result[i].x));
        PyTuple_SetItem(tuple, 2, PyFloat_FromDouble(result[i].y));
        PyList_SetItem(pyResult, i, tuple);
    }

    return pyResult;
}

// Module Definition
static PyMethodDef RotoMethods[] = {
    {"simplify", py_simplify, METH_VARARGS, "Runs RDP simplification on points."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef rotomodule = {
    PyModuleDef_HEAD_INIT, "roto_native", NULL, -1, RotoMethods
};

PyMODINIT_FUNC PyInit_roto_native(void) {
    return PyModule_Create(&rotomodule);
}

PyMODINIT_FUNC PyInit_RotoSimplifier(void) {
    return PyModule_Create(&rotomodule);
}