import nuke
import nuke.rotopaint as rp
import math

try:
    from PySide6 import QtWidgets, QtCore, QtGui
except ImportError:
    from PySide2 import QtWidgets, QtCore, QtGui

# ==============================================================================
# 1. MATHEMATICAL CORE
# ==============================================================================

def get_dist(p1, p2):
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

def perpendicular_distance(point, start, end):
    """Calculates distance from a point to a line segment."""
    p_x, p_y = point[1], point[2]
    s_x, s_y = start[1], start[2]
    e_x, e_y = end[1], end[2]

    if (s_x == e_x) and (s_y == e_y):
        return math.sqrt((p_x - s_x)**2 + (p_y - s_y)**2)
    
    n = abs((e_y - s_y) * p_x - (e_x - s_x) * p_y + e_x * s_y - e_y * s_x)
    d = math.sqrt((e_y - s_y)**2 + (e_x - s_x)**2)
    return n / d

try:
    import roto_native
    HAVE_NATIVE = True
except ImportError:
    HAVE_NATIVE = False

def ramer_douglas_peucker(points, epsilon):
    """Recursive algorithm to reduce point count based on tolerance."""
    if HAVE_NATIVE and len(points) >= 3:
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
        rec_results1 = ramer_douglas_peucker(points[:index+1], epsilon)
        rec_results2 = ramer_douglas_peucker(points[index:], epsilon)
        return rec_results1[:-1] + rec_results2
    else:
        return [points[0], points[end]]

def solve_epsilon_for_target(points, target_count):
    """Binary search to find the epsilon that yields a specific point count."""
    low = 0.0
    high = 5000.0 
    best_epsilon = low
    best_diff = float('inf')
    
    if target_count >= len(points): return 0.0

    for _ in range(20): 
        mid = (low + high) / 2.0
        simplified = ramer_douglas_peucker(points, mid)
        count = len(simplified)
        
        diff = abs(count - target_count)
        if diff < best_diff:
            best_diff = diff
            best_epsilon = mid
        if count == target_count: return mid 
        if count > target_count: low = mid
        else: high = mid
    return best_epsilon

def calculate_smooth_tangents(points, tension=0.35, is_closed=True):
    """Generates smooth Bezier handles for points."""
    tangents = []
    n = len(points)
    if n < 2: return [((0.0,0.0), (0.0,0.0))] * n

    for i in range(n):
        if i == 0: prev_pt = points[n-2] if is_closed else points[0]
        else: prev_pt = points[i-1]
            
        if i == n - 1: next_pt = points[1] if is_closed else points[n-1]
        else: next_pt = points[i+1]
            
        vec_x = next_pt[0] - prev_pt[0]
        vec_y = next_pt[1] - prev_pt[1]
        
        tan_x = vec_x * tension
        tan_y = vec_y * tension
        
        tangents.append( ((-float(tan_x), -float(tan_y)), (float(tan_x), float(tan_y))) )
        
    return tangents

def find_parent_layer(shape, layer):
    """Finds which layer a shape belongs to."""
    for item in layer:
        if item.name == shape.name: return layer
        if isinstance(item, rp.Layer):
            found = find_parent_layer(shape, item)
            if found: return found
    return None

# ==============================================================================
# 2. UI INTERFACE
# ==============================================================================

class RotoSimplifierDialog(QtWidgets.QDialog):
    def __init__(self):
        super(RotoSimplifierDialog, self).__init__()
        self.setWindowTitle("Roto Tools v6.3")
        self.resize(380, 650)
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint) 

        self.setStyleSheet("""
            QGroupBox { font-weight: bold; border: 1px solid #444; margin-top: 6px; padding-top: 8px; border-radius: 4px; }
            QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; }
            QLabel { font-size: 11px; }
            QDoubleSpinBox, QSpinBox { padding: 4px; min-height: 20px; }
            QPushButton { padding: 8px; font-weight: bold; background-color: #444; border-radius: 3px; }
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

        self.tab_simplify = QtWidgets.QWidget()
        self.setup_simplify_tab()
        self.tabs.addTab(self.tab_simplify, "Simplify")

        self.tab_explode = QtWidgets.QWidget()
        self.setup_explode_tab()
        self.tabs.addTab(self.tab_explode, "Explode")

        # Footer
        self.info_label = QtWidgets.QLabel("Ready.")
        self.info_label.setAlignment(QtCore.Qt.AlignCenter)
        self.info_label.setStyleSheet("color: #aaa; font-style: italic; margin-top: 5px;")
        self.main_layout.addWidget(self.info_label)

        self.credit_label = QtWidgets.QLabel("created by Marten Blumen")
        self.credit_label.setAlignment(QtCore.Qt.AlignCenter)
        self.credit_label.setStyleSheet("color: #555; font-size: 9px; margin-bottom: 2px;")
        self.main_layout.addWidget(self.credit_label)

    # --- SIMPLIFY TAB SETUP ---
    def setup_simplify_tab(self):
        layout = QtWidgets.QVBoxLayout()
        layout.setSpacing(8)
        self.tab_simplify.setLayout(layout)

        # Strategy
        self.mode_group = QtWidgets.QGroupBox("Reduction Strategy")
        mode_layout = QtWidgets.QVBoxLayout()
        self.radio_epsilon = QtWidgets.QRadioButton("Epsilon (Visual Fidelity)")
        self.radio_target = QtWidgets.QRadioButton("Target Point Count")
        self.radio_percent = QtWidgets.QRadioButton("Percentage Reduction")
        self.radio_epsilon.setChecked(True)
        
        mode_layout.addWidget(self.radio_epsilon)
        mode_layout.addWidget(self.radio_target)
        mode_layout.addWidget(self.radio_percent)
        self.mode_group.setLayout(mode_layout)
        layout.addWidget(self.mode_group)

        # Settings
        self.settings_group = QtWidgets.QGroupBox("Parameters")
        form_layout = QtWidgets.QFormLayout()
        
        self.epsilon_spin = QtWidgets.QDoubleSpinBox()
        self.epsilon_spin.setRange(0.1, 500.0)
        self.epsilon_spin.setValue(5.0)
        form_layout.addRow("Epsilon:", self.epsilon_spin)
        
        self.count_spin = QtWidgets.QSpinBox()
        self.count_spin.setRange(3, 1000)
        self.count_spin.setValue(20)
        self.count_spin.setEnabled(False) 
        form_layout.addRow("Target Count:", self.count_spin)

        self.percent_spin = QtWidgets.QSpinBox()
        self.percent_spin.setRange(1, 99)
        self.percent_spin.setValue(50)
        self.percent_spin.setSuffix("%")
        self.percent_spin.setEnabled(False)
        form_layout.addRow("Reduce By:", self.percent_spin)

        self.tension_spin = QtWidgets.QDoubleSpinBox()
        self.tension_spin.setRange(0.0, 1.5)
        self.tension_spin.setSingleStep(0.05)
        self.tension_spin.setValue(0.35)
        form_layout.addRow("Smoothing:", self.tension_spin)
        self.settings_group.setLayout(form_layout)
        layout.addWidget(self.settings_group)

        # Animation
        self.anim_group = QtWidgets.QGroupBox("Animation Baking")
        anim_layout = QtWidgets.QGridLayout()
        
        self.chk_bake = QtWidgets.QCheckBox("Bake Animation Range")
        self.chk_bake.setChecked(False)
        self.chk_bake.toggled.connect(self.toggle_anim)
        anim_layout.addWidget(self.chk_bake, 0, 0, 1, 2)
        
        self.start_frame = QtWidgets.QSpinBox()
        self.start_frame.setRange(-99999, 99999)
        self.start_frame.setValue(int(nuke.root()['first_frame'].value()))
        self.start_frame.setEnabled(False)
        
        self.end_frame = QtWidgets.QSpinBox()
        self.end_frame.setRange(-99999, 99999)
        self.end_frame.setValue(int(nuke.root()['last_frame'].value()))
        self.end_frame.setEnabled(False)
        
        anim_layout.addWidget(QtWidgets.QLabel("Start:"), 1, 0)
        anim_layout.addWidget(self.start_frame, 1, 1)
        anim_layout.addWidget(QtWidgets.QLabel("End:"), 2, 0)
        anim_layout.addWidget(self.end_frame, 2, 1)
        
        self.anim_group.setLayout(anim_layout)
        layout.addWidget(self.anim_group)

        # Execution
        self.chk_new_node = QtWidgets.QCheckBox("Output to New Node")
        self.chk_new_node.setChecked(False)
        layout.addWidget(self.chk_new_node)

        self.btn_process = QtWidgets.QPushButton("SIMPLIFY SHAPE")
        self.btn_process.setStyleSheet("background-color: #d98016; color: white;")
        self.btn_process.clicked.connect(self.run_process)
        layout.addWidget(self.btn_process)
        
        self.radio_epsilon.toggled.connect(self.toggle_mode)
        self.radio_target.toggled.connect(self.toggle_mode)
        self.radio_percent.toggled.connect(self.toggle_mode)
        layout.addStretch()

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

    # --- TOGGLES ---
    def toggle_mode(self):
        self.epsilon_spin.setEnabled(self.radio_epsilon.isChecked())
        self.count_spin.setEnabled(self.radio_target.isChecked())
        self.percent_spin.setEnabled(self.radio_percent.isChecked())

    def toggle_anim(self):
        self.start_frame.setEnabled(self.chk_bake.isChecked())
        self.end_frame.setEnabled(self.chk_bake.isChecked())

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
        selected_items = curves_knob.getSelected()
        if not selected_items:
            self.info_label.setText("Error: No shape selected."); return

        current_frame = nuke.frame()
        tension = self.tension_spin.value()
        
        # Determine Frame Range
        do_bake = self.chk_bake.isChecked()
        f_start = self.start_frame.value()
        f_end = self.end_frame.value()
        frame_range = range(f_start, f_end + 1) if do_bake else [current_frame]

        # Determine Target Knob
        target_knob = curves_knob
        target_layer = root_layer
        
        if self.chk_new_node.isChecked():
            # Deselect current to avoid connecting
            for n in nuke.allNodes(): n.setSelected(False)
            new_roto_node = nuke.createNode("Roto")
            new_roto_node.setName(node.name() + "_Simplified")
            new_roto_node.setXYpos(node.xpos() + 100, node.ypos())
            target_knob = new_roto_node['curves']
            target_layer = target_knob.rootLayer

        count_processed = 0
        task = nuke.ProgressTask("Simplifying Roto")

        for idx, shape in enumerate(list(selected_items)):
            if task.isCancelled(): break
            
            if isinstance(shape, rp.Shape):
                # 1. Collect Points
                points_data = []
                for i in range(len(shape)):
                    pt_main = shape[i].center.getPosition(current_frame)
                    pt_feather = shape[i].featherCenter.getPosition(current_frame)
                    points_data.append((i, float(pt_main[0]), float(pt_main[1]), float(pt_feather[0]), float(pt_feather[1])))
                
                try: is_open_val = shape.getAttributes().getValue(current_frame, "op")
                except: is_open_val = 0.0
                is_closed = (is_open_val == 0.0)

                if is_closed and len(points_data) > 2:
                    points_data.append(points_data[0])

                if len(points_data) < 3: continue

                # 2. Solve Epsilon
                if self.radio_target.isChecked():
                    target_val = self.count_spin.value()
                    calc_epsilon = solve_epsilon_for_target(points_data, target_val)
                    simplified_data = ramer_douglas_peucker(points_data, calc_epsilon)
                elif self.radio_percent.isChecked():
                    percent_val = self.percent_spin.value()
                    original_count = len(points_data)
                    keep_ratio = 1.0 - (percent_val / 100.0)
                    calc_target = int(original_count * keep_ratio)
                    if calc_target < 3: calc_target = 3
                    calc_epsilon = solve_epsilon_for_target(points_data, calc_target)
                    simplified_data = ramer_douglas_peucker(points_data, calc_epsilon)
                else:
                    epsilon_val = self.epsilon_spin.value()
                    simplified_data = ramer_douglas_peucker(points_data, epsilon_val)

                if is_closed and len(simplified_data) > 2:
                    simplified_data.pop()
                
                indices_to_track = [p[0] for p in simplified_data]

                # 3. Create Shape
                parent = target_layer
                if not self.chk_new_node.isChecked():
                    parent = find_parent_layer(shape, root_layer) or root_layer

                new_shape = rp.Shape(target_knob)
                new_shape.name = shape.name + "_simplified"
                parent.append(new_shape)

                for _ in indices_to_track:
                    new_shape.append(rp.AnimControlPoint(0, 0))
                
                # 4. Bake Animation
                total_frames = len(frame_range)
                for f_idx, frame in enumerate(frame_range):
                    progress = int(((f_idx / total_frames) * 100))
                    task.setMessage(f"Baking frame {frame}")
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
                        except: pass

                try:
                    new_shape.getAttributes().set(current_frame, "op", is_open_val)
                except: pass
                
                count_processed += 1
                target_knob.changed()

        del task
        self.info_label.setText(f"Success: Processed {count_processed} shapes.")

    def run_explode(self):
        node = nuke.selectedNode()
        if not node or node.Class() not in ('Roto', 'RotoPaint'): 
            self.info_label.setText("Error: Select a Roto node."); return
        
        curves_knob = node['curves']
        root_layer = curves_knob.rootLayer
        
        # Determine Depth
        mode = self.combo_depth.currentIndex()
        if mode == 0: max_depth = 1  # Top Level
        elif mode == 1: max_depth = 99 # Infinite
        else: max_depth = self.spin_depth.value() # Custom

        # Scan for Target Layers
        target_layers = []
        
        def scan_layers(layer, current_depth):
            if current_depth > max_depth: return
            if isinstance(layer, rp.Layer):
                # Add to list if it's not the root itself
                if current_depth > 0:
                    target_layers.append(layer.name)
                for item in layer:
                    scan_layers(item, current_depth + 1)
        
        scan_layers(root_layer, 0)
        
        # Deduplicate
        target_layers = sorted(list(set(target_layers)))
        
        if not target_layers:
            self.info_label.setText("No layers found at this depth."); return

        # Copy original
        node.setSelected(True)
        nuke.nodeCopy("%clipboard%")
        
        for n in nuke.allNodes(): n.setSelected(False)
        
        x_pos = node.xpos()
        y_pos = node.ypos()
        created_count = 0
        
        task = nuke.ProgressTask("Exploding Layers")
        
        for i, layer_name in enumerate(target_layers):
            if task.isCancelled(): break
            task.setMessage(f"Exploding {layer_name}...")
            task.setProgress(int((i / len(target_layers)) * 100))

            nuke.nodePaste("%clipboard%")
            new_node = nuke.selectedNode()
            new_node.setXYpos(x_pos + ((i+1) * 110), y_pos)
            new_node.setName(f"{layer_name}_Node")
            new_node.setInput(0, None)
            
            new_knob = new_node['curves']
            new_root = new_knob.rootLayer
            
            # Helper to check if a layer contains our target (so we don't delete parents)
            def layer_contains_target(layer, target_name):
                for item in layer:
                    if item.name == target_name: return True
                    if isinstance(item, rp.Layer):
                        if layer_contains_target(item, target_name): return True
                return False

            # Recursive Cleanup
            def filter_tree(layer_obj):
                # Reverse iteration for safe deletion
                for j in range(len(layer_obj)-1, -1, -1):
                    item = layer_obj[j]
                    
                    # Keep the target itself
                    if item.name == layer_name:
                        continue
                    
                    # If it's a folder containing target, recurse inside
                    if isinstance(item, rp.Layer):
                        if layer_contains_target(item, layer_name):
                            filter_tree(item)
                            continue
                    
                    # Otherwise delete
                    try: layer_obj.remove(item)
                    except: 
                        try: layer_obj.remove(j)
                        except: pass

            filter_tree(new_root)
            new_node.setSelected(False)
            created_count += 1
        
        del task
        self.info_label.setText(f"Success: Exploded {created_count} nodes.")

# --- STARTUP ---
roto_simplifier_panel = None

def start():
    global roto_simplifier_panel
    if roto_simplifier_panel is not None:
        roto_simplifier_panel.close()
        roto_simplifier_panel.deleteLater()
    roto_simplifier_panel = RotoSimplifierDialog()
    roto_simplifier_panel.show()