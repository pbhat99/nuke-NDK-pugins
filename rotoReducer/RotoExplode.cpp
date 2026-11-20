//
// RotoExplode - Dedicated Nuke C++ Plugin Node
// Explodes layers from the connected or selected Roto node into separate, individual Roto nodes.
//

#include <Python.h>
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"

#include <string>
#include <sstream>

using namespace DD::Image;

static const char* const CLASS = "RotoExplode";
static const char* const HELP = 
    "RotoExplode - Explodes layers and folders inside the connected or selected Roto node into separate, individual Roto nodes.";

// ==============================================================================
// EMBEDDED PYTHON LOGIC FOR ROTO EXPLODE
// ==============================================================================

static const char* kEmbeddedPythonCode = R"PY(
import os
import tempfile
import nuke
import nuke.rotopaint as rp

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

def _do_run_explode(caller_node_name):
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
        nuke.message("No layers found at this depth inside '" + roto_node.name() + "'.")
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

    nuke.message(f"RotoExplode: Successfully exploded {created_count} node(s).")

def run_explode_from_node(caller_node_name):
    try:
        _do_run_explode(caller_node_name)
    except Exception as e:
        import traceback
        traceback.print_exc()
        nuke.message(f"RotoExplode Error:\n{e}")
)PY";

static void ensure_python_initialized() {
    static bool s_initialized = false;
    if (s_initialized) return;

    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(kEmbeddedPythonCode);
    s_initialized = true;
    PyGILState_Release(gstate);
}

// ==============================================================================
// NUKE NODE IMPLEMENTATION (RotoExplode)
// ==============================================================================

class RotoExplode : public NoIop {
    int depth_mode_;
    int custom_depth_;

public:
    explicit RotoExplode(Node* node)
        : NoIop(node)
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
    return new RotoExplode(node);
}

const Op::Description RotoExplode::description(CLASS, build);

void RotoExplode::_validate(bool for_real) {
    copy_info();
}

void RotoExplode::knobs(Knob_Callback f) {
    ensure_python_initialized();

    // Do NOT declare an initial Tab_knob so Nuke titles the primary tab after the node class
    static const char* const depths[] = { "Top Level Only (Depth 1)", "Infinite Recursive (All Sub-Folders)", "Custom Depth Level", nullptr };
    Enumeration_knob(f, &depth_mode_, depths, "depth_mode", "depth level");
    SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Select how deep in the folder hierarchy layers will be exploded.");

    Int_knob(f, &custom_depth_, "custom_depth", "custom depth");
    SetRange(f, 1, 10);
    Tooltip(f, "Specific level depth when using Custom Depth Level.");

    Divider(f, "");

    PyScript_knob(f, "run_explode_from_node(nuke.thisNode().name())", "btn_explode", "Explode Layers to Nodes");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Splits all layers in the connected or active Roto node into separate, disconnected Roto nodes.");
}

int RotoExplode::knob_changed(Knob* k) {
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
