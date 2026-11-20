//
// CropMore.cpp
// All-in-one compiled Nuke C++ NDK Crop Plugin with integrated GetBounds & CurveTool AutoCrop.
//
// Features:
// - Full native Crop performance (scanline Iop)
// - Top Buttons: format, bBox, bBox animated, autoCrop
// - Integrated GetBounds planar scan (merged directly into CropMore.dll)
// - Integrated CurveTool AutoCrop analysis
// - Aspect-locked 2D padding (-100 to 100+)
// - Reverse crop restoring into selectable target format canvas
// - Preconfigured defaults (intersect=True, black outside=True)
// - Zero external Python dependencies (no crop_more.py needed)
//

#include "DDImage/Iop.h"
#include "DDImage/PlanarIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/Format.h"
#include "DDImage/Box.h"
#include "DDImage/DDMath.h"
#include "DDImage/Interest.h"
#include "DDImage/Thread.h"

#include <cmath>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using namespace DD::Image;

// ============================================================================
// Inline Python Action Scripts (Self-contained, no external .py file required)
// ============================================================================

static const char* const kFormatScript = R"PY(import nuke
n = nuke.thisNode()
rf = nuke.root().format()
n['box'].setValue([0.0, 0.0, float(rf.width()), float(rf.height())])
)PY";

static const char* const kBBoxScript = R"PY(import nuke
n = nuke.thisNode()
inp = n.input(0)
if not inp:
    nuke.message("CropMore: Please connect an input node.")
else:
    bb = inp.bbox()
    n['box'].setValue([float(bb.x()), float(bb.y()), float(bb.x() + bb.w()), float(bb.y() + bb.h())])
)PY";

static const char* const kBBoxAnimScript = R"PY(import nuke
n = nuke.thisNode()
inp = n.input(0)
if not inp:
    nuke.message("CropMore: Please connect an input node.")
else:
    first = int(nuke.root().firstFrame())
    last = int(nuke.root().lastFrame())
    try:
        first = int(inp.firstFrame())
        last = int(inp.lastFrame())
    except Exception:
        pass
    test_fr = n.knobs().get('_test_frames')
    if test_fr and test_fr.value():
        fr_str = test_fr.value()
    else:
        fr_str = nuke.getInput("Frame Range for BBox Animation", "%d-%d" % (first, last))
    if fr_str:
        try:
            ranges = nuke.FrameRanges(fr_str).toFrameList()
        except Exception:
            ranges = [int(p.strip()) for p in fr_str.split('-') if p.strip()]
            if len(ranges) == 2:
                ranges = list(range(ranges[0], ranges[1] + 1))
        if ranges:
            cur_f = nuke.frame()
            bk = n['box']
            bk.setAnimated()
            task = nuke.ProgressTask("CropMore: BBox Animated")
            total = len(ranges)
            try:
                for idx, f in enumerate(ranges):
                    if task.isCancelled(): break
                    task.setMessage("Sampling BBox at frame %d" % f)
                    task.setProgress(int((float(idx) / float(total)) * 100))
                    nuke.frame(f)
                    bb = inp.bbox()
                    bk.setValueAt(float(bb.x()), f, 0)
                    bk.setValueAt(float(bb.y()), f, 1)
                    bk.setValueAt(float(bb.x() + bb.w()), f, 2)
                    bk.setValueAt(float(bb.y() + bb.h()), f, 3)
            finally:
                del task
                nuke.frame(cur_f)
)PY";

static const char* const kAutoCropScript = R"PY(import nuke
n = nuke.thisNode()
inp = n.input(0)
if not inp:
    nuke.message("CropMore: Please connect an input node.")
else:
    first = int(nuke.root().firstFrame())
    last = int(nuke.root().lastFrame())
    try:
        first = int(inp.firstFrame())
        last = int(inp.lastFrame())
    except Exception:
        pass

    channel_list = ['alpha', 'rgba.alpha', 'rgba.red', 'rgba.green', 'rgba.blue', 'depth.Z']
    try:
        chans = inp.channels()
        if chans:
            ordered = []
            if 'alpha' in chans: ordered.append('alpha')
            if 'rgba.alpha' in chans and 'rgba.alpha' not in ordered: ordered.append('rgba.alpha')
            for c in chans:
                if c not in ordered: ordered.append(c)
            if ordered: channel_list = ordered
    except Exception:
        pass

    test_action = n.knobs().get('_test_action')
    test_fr = n.knobs().get('_test_frames')
    test_chan = n.knobs().get('_test_channel')

    if test_action and test_action.value():
        ret = 1 if test_action.value() == 'getbounds' else 2
        fr_str = test_fr.value() if (test_fr and test_fr.value()) else ("%d-%d" % (first, last))
        channel = test_chan.value() if (test_chan and test_chan.value()) else 'alpha'
    else:
        p = nuke.Panel("CropMore: AutoCrop")
        p.addSingleLineInput("Frame Range", "%d-%d" % (first, last))
        p.addEnumerationPulldown("Channel", " ".join(channel_list))
        p.addButton("Calculate with GetBounds")
        p.addButton("Calculate with CurveTool")
        p.addButton("Cancel")
        ret = p.show()
        if ret not in (1, 2):
            ret = 0
        else:
            fr_str = p.value("Frame Range")
            channel = p.value("Channel") or "alpha"

    if ret in (1, 2):
        try:
            ranges = nuke.FrameRanges(fr_str).toFrameList()
        except Exception:
            ranges = list(range(first, last + 1))
        if ranges:
            cur_f = nuke.frame()
            bk = n['box']
            bk.setAnimated()
            if ret == 1:
                # Calculate using GetBounds (integrated directly in CropMore.dll)
                try:
                    gb = nuke.nodes.GetBounds(inputs=[inp])
                    if 'channels' in gb.knobs(): gb['channels'].setValue(channel)
                    if 'black outside' in gb.knobs(): gb['black outside'].setValue(True)
                    task = nuke.ProgressTask("AutoCrop (GetBounds)")
                    tot = len(ranges)
                    for i, f in enumerate(ranges):
                        if task.isCancelled(): break
                        task.setMessage("Scanning bounds at frame %d" % f)
                        task.setProgress(int((float(i) / float(tot)) * 100))
                        nuke.frame(f)
                        bb = gb.bbox()
                        bk.setValueAt(float(bb.x()), f, 0)
                        bk.setValueAt(float(bb.y()), f, 1)
                        bk.setValueAt(float(bb.x() + bb.w()), f, 2)
                        bk.setValueAt(float(bb.y() + bb.h()), f, 3)
                    del task
                    nuke.delete(gb)
                except Exception as e:
                    nuke.message("GetBounds error: %s" % str(e))
            elif ret == 2:
                # Calculate using CurveTool AutoCrop (integrated)
                try:
                    ct = nuke.nodes.CurveTool(inputs=[inp])
                    ct['operation'].setValue('Auto Crop')
                    try: ct['channels'].setValue(channel)
                    except Exception: pass
                    w = float(inp.width())
                    h = float(inp.height())
                    ct['ROI'].setValue([0.0, 0.0, w, h])
                    f_min = min(ranges)
                    f_max = max(ranges)
                    nuke.execute(ct, f_min, f_max)
                    for f in ranges:
                        for idx in range(4):
                            bk.setValueAt(float(ct['autocropdata'].valueAt(f, idx)), f, idx)
                    nuke.delete(ct)
                except Exception as e:
                    nuke.message("CurveTool error: %s" % str(e))
            nuke.frame(cur_f)
)PY";

static const char* const kPresetResetScript = R"PY(import nuke
n = nuke.thisNode()
presets = {
    'format': None,
    'square': 1.0,
    '4:3': 4.0 / 3.0,
    '16:9': 16.0 / 9.0,
    '14:9': 14.0 / 9.0,
    '1.66:1': 1.66,
    '1.85:1': 1.85,
    '2.35:1': 2.35
}
p = n['preset'].value()
if p == 'format':
    rf = nuke.root().format()
    n['box'].setValue([0.0, 0.0, float(rf.width()), float(rf.height())])
elif p in presets:
    target_aspect = presets[p]
    bx, by, br, bt = [float(v) for v in n['box'].value()]
    cur_w = br - bx
    cur_h = bt - by
    cx = (bx + br) * 0.5
    cy = (by + bt) * 0.5
    if (cur_w / max(cur_h, 1e-4)) > target_aspect:
        new_w = cur_h * target_aspect
        new_h = cur_h
    else:
        new_w = cur_w
        new_h = cur_w / target_aspect
    n['box'].setValue([cx - new_w * 0.5, cy - new_h * 0.5, cx + new_w * 0.5, cy + new_h * 0.5])
)PY";


// ============================================================================
// CropMore Class Declaration & Implementation
// ============================================================================

static const char* const CLASS = "CropMore";
static const char* const HELP =
    "CropMore - High-performance C++ NDK Crop operator with automated bounding box,\n"
    "animated tracking, aspect-aware padding, and reverse crop.";

class CropMore : public Iop {
    int preset_;
    double box_[4];
    double softness_[2];
    double padding_[2];
    double last_padding_[2];
    bool keep_aspect_;
    bool reverse_;
    FormatPair target_format_;
    bool reformat_;
    bool intersect_;
    bool black_outside_;

    int dx_, dy_;
    Box crop_box_;
    Box in_bbox_;
    bool updating_padding_;

public:
    explicit CropMore(Node* node);
    ~CropMore() override = default;

    void _validate(bool for_real) override;
    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override;
    void engine(int y, int x, int r, ChannelMask channels, Row& row) override;
    void knobs(Knob_Callback f) override;
    int knob_changed(Knob* k) override;

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }
    static const Iop::Description description;

private:
    void apply_softness(int y, int x, int r, ChannelMask channels, Row& row);
};

CropMore::CropMore(Node* node) : Iop(node) {
    preset_ = 0; // "format"
    box_[0] = 0.0;
    box_[1] = 0.0;
    box_[2] = 2048.0;
    box_[3] = 1556.0;

    softness_[0] = softness_[1] = 0.0;
    padding_[0] = padding_[1] = 0.0;
    last_padding_[0] = last_padding_[1] = 0.0;

    keep_aspect_ = false;
    reverse_ = false;
    reformat_ = false;

    // Always ticked by default per specification
    intersect_ = true;
    black_outside_ = true;

    dx_ = 0;
    dy_ = 0;
    updating_padding_ = false;
}

static const char* const preset_names[] = {
    "format",
    "square",
    "4:3",
    "16:9",
    "14:9",
    "1.66:1",
    "1.85:1",
    "2.35:1",
    nullptr
};

void CropMore::knobs(Knob_Callback f) {
    // Preset dropdown + reset button on top row
    Enumeration_knob(f, &preset_, preset_names, "preset", "preset");
    Tooltip(f, "Standard aspect ratio preset for the crop box.");

    PyScript_knob(f, kPresetResetScript, "reset_preset", "reset");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Reset box to selected preset aspect ratio.");

    // 4 Action buttons next to the preset controls
    PyScript_knob(f, kFormatScript, "btn_format", "format");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Set crop box to root project format.");

    PyScript_knob(f, kBBoxScript, "btn_bbox", "bBox");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Set crop box to current frame bounding box.");

    PyScript_knob(f, kBBoxAnimScript, "btn_bbox_anim", "bBox animated");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Animate crop box across frame range based on incoming bounding box.");

    PyScript_knob(f, kAutoCropScript, "btn_autocrop", "autoCrop");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Automatically detect non-zero pixels across frame range using GetBounds or CurveTool.");

    BBox_knob(f, box_, "box");
    SetFlags(f, Knob::EARLY_STORE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Crop coordinates: x, y, r, t.");

    WH_knob(f, softness_, "softness");
    Tooltip(f, "Edge softness in pixels (X and Y).");

    WH_knob(f, padding_, "padding");
    SetRange(f, -100.0, 100.0);
    SetFlags(f, Knob::EARLY_STORE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Crop padding in pixels (X and Y). Can be negative to shrink crop.");

    Bool_knob(f, &keep_aspect_, "keep_aspect", "keep aspect ratio");
    ClearFlags(f, Knob::STARTLINE);
    SetFlags(f, Knob::EARLY_STORE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Proportionally expand or shrink padding to match crop box aspect ratio.");

    Bool_knob(f, &reverse_, "reverse", "reverse");
    SetFlags(f, Knob::EARLY_STORE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Reverse the crop, placing the cropped image back into the target format canvas.");

    Format_knob(f, &target_format_, "target_format", "reformat to");
    ClearFlags(f, Knob::STARTLINE);
    if (!reverse_) {
        SetFlags(f, Knob::DISABLED);
    }
    Tooltip(f, "Target format canvas used when reverse is ticked.");

    Bool_knob(f, &reformat_, "reformat", "reformat");
    Tooltip(f, "Reformat incoming canvas to the crop box bounds.");

    Bool_knob(f, &intersect_, "intersect", "intersect");
    Tooltip(f, "Intersect crop rectangle with the incoming bounding box.");

    Bool_knob(f, &black_outside_, "crop", "black outside");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Black outside crop bounds.");
}

int CropMore::knob_changed(Knob* k) {
    if (k->is("reverse")) {
        Hash h;
        k->store(BoolPtr, &reverse_, h, uiContext());
        Knob* tf = knob("target_format");
        if (tf) {
            tf->enable(reverse_);
            tf->set_flag(Knob::DISABLED, !reverse_);
        }
        return 1;
    }
    if (k->is("padding") || k->is("keep_aspect")) {
        if (updating_padding_) return 1;
        Hash h;
        if (k->is("keep_aspect")) {
            k->store(BoolPtr, &keep_aspect_, h, uiContext());
        } else {
            k->store(DoublePtr, padding_, h, uiContext());
        }
        if (keep_aspect_) {
            double bw = box_[2] - box_[0];
            double bh = box_[3] - box_[1];
            double aspect = (bh > 1e-4) ? (bw / bh) : 1.0;

            double px = padding_[0];
            double py = padding_[1];

            updating_padding_ = true;
            Knob* pk = knob("padding");
            if (std::abs(px - last_padding_[0]) > 1e-4) {
                double new_py = (aspect > 1e-4) ? (px / aspect) : px;
                padding_[1] = new_py;
                if (pk) pk->set_value(new_py, 1);
                last_padding_[0] = px;
                last_padding_[1] = new_py;
            } else if (std::abs(py - last_padding_[1]) > 1e-4) {
                double new_px = py * aspect;
                padding_[0] = new_px;
                if (pk) pk->set_value(new_px, 0);
                last_padding_[0] = new_px;
                last_padding_[1] = py;
            } else {
                last_padding_[0] = px;
                last_padding_[1] = py;
            }
            updating_padding_ = false;
            return 1;
        } else {
            last_padding_[0] = padding_[0];
            last_padding_[1] = padding_[1];
        }
    }
    if (k->is("box")) {
        Hash h;
        k->store(DoublePtr, box_, h, uiContext());
        return 1;
    }
    if (k->is("showPanel")) {
        Knob* tf = knob("target_format");
        if (tf) {
            tf->enable(reverse_);
            tf->set_flag(Knob::DISABLED, !reverse_);
        }
        return 1;
    }
    return Iop::knob_changed(k);
}

void CropMore::_validate(bool for_real) {
    copy_info();

    double eff_x = box_[0] - padding_[0];
    double eff_y = box_[1] - padding_[1];
    double eff_r = box_[2] + padding_[0];
    double eff_t = box_[3] + padding_[1];

    if (eff_x > eff_r) std::swap(eff_x, eff_r);
    if (eff_y > eff_t) std::swap(eff_y, eff_t);

    int bx = static_cast<int>(std::floor(eff_x));
    int by = static_cast<int>(std::floor(eff_y));
    int br = static_cast<int>(std::ceil(eff_r));
    int bt = static_cast<int>(std::ceil(eff_t));

    crop_box_ = Box(bx, by, br, bt);
    in_bbox_ = Box(info_.x(), info_.y(), info_.r(), info_.t());

    if (!reverse_) {
        if (intersect_) {
            crop_box_.intersect(in_bbox_);
        }

        if (reformat_) {
            int w = std::max(1, crop_box_.w());
            int h = std::max(1, crop_box_.h());
            Format fmt(w, h);
            info_.format(fmt);
            dx_ = -crop_box_.x();
            dy_ = -crop_box_.y();
            if (black_outside_) {
                info_.set(-1, -1, w + 1, h + 1);
                info_.black_outside(true);
            } else {
                info_.set(0, 0, w, h);
                info_.black_outside(false);
            }
        } else {
            dx_ = 0;
            dy_ = 0;
            if (black_outside_) {
                info_.set(crop_box_.x() - 1, crop_box_.y() - 1, crop_box_.r() + 1, crop_box_.t() + 1);
                info_.black_outside(true);
            } else {
                info_.set(crop_box_);
                info_.black_outside(false);
            }
        }
    } else {
        const Format* tf = target_format_.format();
        if (!tf || tf == &Format::None) {
            tf = &input_format();
        }
        info_.format(*tf);

        int in_fx = input0().format().x();
        int in_fy = input0().format().y();

        dx_ = crop_box_.x() - in_fx;
        dy_ = crop_box_.y() - in_fy;

        info_.move(dx_, dy_);

        if (intersect_) {
            info_.intersect(0, 0, tf->width(), tf->height());
        }

        if (black_outside_) {
            info_.set(info_.x() - 1, info_.y() - 1, info_.r() + 1, info_.t() + 1);
            info_.black_outside(true);
        } else {
            info_.black_outside(false);
        }
    }
}

void CropMore::_request(int x, int y, int r, int t, ChannelMask channels, int count) {
    if (!reverse_) {
        if (reformat_) {
            int in_x = std::max(0, x) - dx_;
            int in_y = std::max(0, y) - dy_;
            int in_r = std::min(crop_box_.w(), r) - dx_;
            int in_t = std::min(crop_box_.h(), t) - dy_;
            if (in_x < in_r && in_y < in_t) {
                input0().request(in_x, in_y, in_r, in_t, channels, count);
            }
        } else {
            int req_x = std::max(x, crop_box_.x());
            int req_y = std::max(y, crop_box_.y());
            int req_r = std::min(r, crop_box_.r());
            int req_t = std::min(t, crop_box_.t());
            if (req_x < req_r && req_y < req_t) {
                input0().request(req_x, req_y, req_r, req_t, channels, count);
            }
        }
    } else {
        int in_x = std::max(crop_box_.x(), x) - dx_;
        int in_y = std::max(crop_box_.y(), y) - dy_;
        int in_r = std::min(crop_box_.r(), r) - dx_;
        int in_t = std::min(crop_box_.t(), t) - dy_;
        if (in_x < in_r && in_y < in_t) {
            input0().request(in_x, in_y, in_r, in_t, channels, count);
        }
    }
}

void CropMore::engine(int y, int x, int r, ChannelMask channels, Row& row) {
    if (!reverse_) {
        if (reformat_) {
            int in_y = y - dy_;
            int in_x = x - dx_;
            int in_r = r - dx_;

            if (y < 0 || y >= crop_box_.h()) {
                if (black_outside_) {
                    row.erase(channels);
                } else {
                    Row in_row(in_x, in_r);
                    in_row.get(input0(), in_y, in_x, in_r, channels);
                    foreach(z, channels) {
                        float* out = row.writable(z);
                        const float* in = in_row[z];
                        for (int cur_x = x; cur_x < r; ++cur_x) {
                            out[cur_x] = in[cur_x - dx_];
                        }
                    }
                }
                return;
            }

            Row in_row(in_x, in_r);
            in_row.get(input0(), in_y, in_x, in_r, channels);

            foreach(z, channels) {
                float* out = row.writable(z);
                const float* in = in_row[z];
                for (int cur_x = x; cur_x < r; ++cur_x) {
                    if (black_outside_ && (cur_x < 0 || cur_x >= crop_box_.w())) {
                        out[cur_x] = 0.0f;
                    } else {
                        out[cur_x] = in[cur_x - dx_];
                    }
                }
            }
        } else {
            // Normal forward crop (not reformatted)
            if (y < crop_box_.y() || y >= crop_box_.t()) {
                if (black_outside_) {
                    row.erase(channels);
                } else {
                    row.get(input0(), y, x, r, channels);
                }
                return;
            }

            if (black_outside_) {
                Row in_row(x, r);
                in_row.get(input0(), y, x, r, channels);

                foreach(z, channels) {
                    float* out = row.writable(z);
                    const float* in = in_row[z];
                    for (int cur_x = x; cur_x < r; ++cur_x) {
                        if (cur_x < crop_box_.x() || cur_x >= crop_box_.r()) {
                            out[cur_x] = 0.0f;
                        } else {
                            out[cur_x] = in[cur_x];
                        }
                    }
                }
            } else {
                row.get(input0(), y, x, r, channels);
            }
        }

        if (softness_[0] > 0.0 || softness_[1] > 0.0) {
            apply_softness(y, x, r, channels, row);
        }
    } else {
        // Reverse mode: restore cropped image back onto target canvas
        if (y < crop_box_.y() || y >= crop_box_.t()) {
            row.erase(channels);
            return;
        }

        int in_y = y - dy_;
        int in_x = x - dx_;
        int in_r = r - dx_;

        Row in_row(in_x, in_r);
        in_row.get(input0(), in_y, in_x, in_r, channels);

        foreach(z, channels) {
            float* out = row.writable(z);
            const float* in = in_row[z];
            for (int cur_x = x; cur_x < r; ++cur_x) {
                if (cur_x < crop_box_.x() || cur_x >= crop_box_.r()) {
                    out[cur_x] = 0.0f;
                } else {
                    out[cur_x] = in[cur_x - dx_];
                }
            }
        }
    }
}

void CropMore::apply_softness(int y, int x, int r, ChannelMask channels, Row& row) {
    double sx = std::max(0.001, softness_[0]);
    double sy = std::max(0.001, softness_[1]);

    int box_x = reformat_ ? 0 : crop_box_.x();
    int box_y = reformat_ ? 0 : crop_box_.y();
    int box_r = reformat_ ? crop_box_.w() : crop_box_.r();
    int box_t = reformat_ ? crop_box_.h() : crop_box_.t();

    double dist_bottom = y - box_y;
    double dist_top = box_t - 1 - y;
    double min_dist_y = std::min(dist_bottom, dist_top);

    double falloff_y = 1.0;
    if (min_dist_y < sy) {
        if (min_dist_y <= 0) {
            falloff_y = 0.0;
        } else {
            falloff_y = min_dist_y / sy;
            falloff_y = falloff_y * falloff_y * (3.0 - 2.0 * falloff_y); // smoothstep
        }
    }

    foreach(z, channels) {
        float* ptr = row.writable(z);
        for (int cur_x = x; cur_x < r; ++cur_x) {
            double dist_left = cur_x - box_x;
            double dist_right = box_r - 1 - cur_x;
            double min_dist_x = std::min(dist_left, dist_right);

            double falloff_x = 1.0;
            if (min_dist_x < sx) {
                if (min_dist_x <= 0) {
                    falloff_x = 0.0;
                } else {
                    falloff_x = min_dist_x / sx;
                    falloff_x = falloff_x * falloff_x * (3.0 - 2.0 * falloff_x); // smoothstep
                }
            }

            float factor = static_cast<float>(falloff_x * falloff_y);
            ptr[cur_x] *= factor;
        }
    }
}

static Iop* build_cropmore(Node* node) {
    return new CropMore(node);
}

const Iop::Description CropMore::description(CLASS, "Transform/CropMore", build_cropmore);


// ============================================================================
// GetBounds Class Declaration & Implementation (Merged Internally)
// ============================================================================

static const char* const GB_CLASS = "GetBounds";
static const char* const GB_HELP = "GetBounds - Scans non-zero pixel bounds across selected channels.";

class GetBounds : public PlanarIop {
    ChannelSet channels;
    Box bbox;
    int p[4];
    bool _blackOutside;
    bool _ignoreNegative;
    bool _firstTime;
    Lock _lock;
    Hash _lastHash;
    int _extra;

public:
    explicit GetBounds(Node* node) : PlanarIop(node) {
        Box inputFormat = input_format();
        p[0] = inputFormat.x();
        p[1] = inputFormat.y();
        p[2] = inputFormat.r();
        p[3] = inputFormat.t();

        _blackOutside = true;
        _ignoreNegative = false;
        _lastHash = Hash();
        _extra = 0;
    }
    ~GetBounds() override = default;

    void _validate(bool for_real) override;
    void getRequests(const Box& box, const ChannelSet& channelsSet, int count, RequestOutput& reqData);
    void renderStripe(ImagePlane& imagePlane) override;
    virtual bool renderFullPlanes() { return true; }

    void getBounds(ImagePlane& imagePlane, Format& format, Interest& interest);

    const char* Class() const override { return GB_CLASS; }
    const char* node_help() const override { return GB_HELP; }
    const char* input_label(int n, char*) const override { return "1"; }

    void knobs(Knob_Callback f) override;
    static const PlanarIop::Description dGet;
};

void GetBounds::_validate(bool for_real) {
    copy_info();
    info_.black_outside(_blackOutside);

    Format format = input0().format();
    Box inputbounds = Box(format.x(), format.y(), format.r(), format.t());
    ImagePlane imagePlane = ImagePlane(inputbounds, true, channels);

    RequestOutput reqData = RequestOutput();
    getRequests(inputbounds, channels, 0, reqData);
    input0().request(channels, 0);

    Interest interest(input0(), inputbounds, imagePlane.channels(), true);
    interest.unlock();

    if (input0().real_valid() && !input0().inErrorState() && !input0().inInvalidState()) {
        if (input0().tryValidate() && interest.valid()) {
            running(true);
            getBounds(imagePlane, format, interest);

            Box bounds(p[0], p[1], p[2], p[3]);
            if (bounds.area() >= 1) {
                info_.set(bounds);
            }
            _lastHash = input0().hash();
            running(false);
        }
    }
}

void GetBounds::getRequests(const Box& box, const ChannelSet& channelsSet, int count, RequestOutput& reqData) {
    PlanarIop::getRequests(box, channelsSet, count, reqData);
}

void GetBounds::getBounds(ImagePlane& imagePlane, Format& format, Interest& interest) {
    const int bx = imagePlane.bounds().x();
    const int by = imagePlane.bounds().y();
    const int br = imagePlane.bounds().r();
    const int bt = imagePlane.bounds().t();

    bool yMinFound = false;
    bool xMinFound = false;
    bool xMaxFound = false;

    foreach(z, channels) {
        for (int y = by; y < bt; y++) {
            float maxValue = 0;
            for (int x = bx; x < br; x++) {
                try {
                    float currentPixel = interest.at(x, y, z);
                    if (currentPixel > 0) {
                        if (currentPixel > maxValue) maxValue = currentPixel;
                        if (!xMinFound) {
                            p[0] = x - _extra;
                            xMinFound = true;
                        } else if (x - _extra < p[0]) {
                            p[0] = x - _extra;
                        }
                        break;
                    }
                } catch (...) {
                    abort();
                }
            }

            for (int x = br; x-- > bx;) {
                try {
                    float currentPixel = interest.at(x, y, z);
                    if (currentPixel > 0) {
                        if (currentPixel > maxValue) maxValue = currentPixel;
                        if (!xMaxFound) {
                            p[2] = x + 1 + _extra;
                            xMaxFound = true;
                        } else if (x + 1 + _extra > p[2]) {
                            p[2] = x + 1 + _extra;
                        }
                        break;
                    }
                } catch (...) {
                    abort();
                }
            }

            if (maxValue > 0) {
                if (!yMinFound) {
                    p[1] = y - _extra;
                    yMinFound = true;
                }
                p[3] = y + 1 + _extra;
            }
        }
    }
}

void GetBounds::renderStripe(ImagePlane& imagePlane) {
    input0().fetchPlane(imagePlane);
    imagePlane.makeUnique();
}

void GetBounds::knobs(Knob_Callback f) {
    ChannelSet_knob(f, &channels, "channels");
    SetFlags(f, Knob::EARLY_STORE);
    Bool_knob(f, &_blackOutside, "black outside");
    SetFlags(f, Knob::STARTLINE | Knob::ALWAYS_SAVE | Knob::EARLY_STORE);
    Bool_knob(f, &_ignoreNegative, "ignore <0");
    SetFlags(f, Knob::EARLY_STORE);
    Int_knob(f, &_extra, "extra");
}

static Iop* build_getbounds(Node* node) {
    return new GetBounds(node);
}

const PlanarIop::Description GetBounds::dGet("GetBounds", "GetBounds", build_getbounds);
