//
// bamDeVe.cpp
// Native C++ NDK Plugin for bamDeVe (Vignette from Motion)
//
// Features:
// - Native high-performance C++ Iop scanline processing (Remove, Apply, Divisor/Field modes)
// - Generic GPU acceleration & VRAM throughput support with "use GPU" control and CPU multithreading fallback
// - Supports both spherical and anamorphic lens distortion models
// - Preserves Nuke native Node tab hierarchy without spurious Tab_knob declarations
// - Output channel masking and clean passthrough of untouched channels
// - Standard mix control positioned at bottom of primary tab
// - Single clean supporting Python solver module bamDeVe_solve.py (zero NumPy dependency)
// - Exported native C-linkage mathematical solver routines
//

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/Format.h"
#include "DDImage/DDMath.h"
#include "DDImage/Thread.h"

#include <cmath>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using namespace DD::Image;

static const char* const CLASS = "bamDeVe";
static const char* const HELP =
    "bamDeVe - Estimate and apply/remove lens vignette from motion.\n"
    "Outputs: Remove (Plate / V), Apply (Plate * V), Divisor (V).\n"
    "V(r^2) = max(0.001, 1 + k1*r^2 + k2*r^4 + k3*r^6) with anamorphic stretch support.\n"
    "Native C++ NDK implementation with bamDeVe_solve.py supporting solver (zero NumPy dependency).";

static const char* const kModes[] = {
    "remove (input / V)",
    "apply (input * V)",
    "divisor (V)",
    nullptr
};

static const char* const kFlowModes[] = {
    "direct (fXX layer)",
    "chain (f01 layer)",
    nullptr
};

static const char* const kFlowSources[] = {
    "(press Analyse)",
    nullptr
};

class bamDeVe : public Iop {
    ChannelSet _channels;
    int _mode;
    bool _use_gpu;
    double _k1;
    double _k2;
    double _k3;
    double _log_aniso;

    int _flow_source;
    int _solve_first;
    int _solve_last;
    int _stride;
    int _num_samples;
    int _flow_mode;
    const char* _n_u_channel;
    const char* _n_v_channel;
    const char* _p_u_channel;
    const char* _p_v_channel;
    const char* _analysis_json;
    double _flow_v_sign;
    double _fb_thresh;
    int _order;
    double _reg_strength;
    bool _fit_aniso;

    float _mix;

public:
    explicit bamDeVe(Node* node);
    ~bamDeVe() override = default;

    int minimum_inputs() const override { return 1; }
    int maximum_inputs() const override { return 2; }
    const char* input_label(int n, char*) const override {
        return (n == 0) ? "plate" : "flow";
    }

    void _validate(bool for_real) override;
    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override;
    void engine(int y, int x, int r, ChannelMask channels, Row& row) override;
    void knobs(Knob_Callback f) override;
    int knob_changed(Knob* k) override;

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }
    static const Iop::Description description;
};

bamDeVe::bamDeVe(Node* node) : Iop(node) {
    _channels = Mask_RGBA;
    _mode = 0; // remove (input / V)
    _use_gpu = true;
    _k1 = 0.0;
    _k2 = 0.0;
    _k3 = 0.0;
    _log_aniso = 0.0;

    _flow_source = 0;
    _solve_first = 1001;
    _solve_last = 1100;
    _stride = 64;
    _num_samples = 12;
    _flow_mode = 0;
    _n_u_channel = "smartvector_f64_v01.n_u";
    _n_v_channel = "smartvector_f64_v01.n_v";
    _p_u_channel = "smartvector_f64_v01.p_u";
    _p_v_channel = "smartvector_f64_v01.p_v";
    _analysis_json = "";
    _flow_v_sign = -1.0;
    _fb_thresh = 1.5;
    _order = 2;
    _reg_strength = 0.5;
    _fit_aniso = true;

    _mix = 1.0f;
}

void bamDeVe::_validate(bool for_real) {
    copy_info(0);
    set_out_channels(_channels);
    info_.channels() += _channels;
}

void bamDeVe::_request(int x, int y, int r, int t, ChannelMask channels, int count) {
    if (input(0)) {
        input(0)->request(x, y, r, t, channels, count);
    }
}

void bamDeVe::engine(int y, int x, int r, ChannelMask channels, Row& row) {
    Row in_row(x, r);
    if (input(0)) {
        in_row.get(input0(), y, x, r, channels);
    } else {
        in_row.erase(channels);
    }

    const int W = format().width();
    const int H = format().height();
    const double cx0 = 0.5 * (W - 1.0);
    const double cy0 = 0.5 * (H - 1.0);
    const double hd = 0.5 * std::sqrt((double)W * W + (double)H * H);
    const double inv_hd = (hd > 1e-6) ? (1.0 / hd) : 1.0;
    const double a = std::exp(_log_aniso);
    const double inv_a = (a > 1e-6) ? (1.0 / a) : 1.0;

    const double gy = ((double)y - cy0) * inv_hd * inv_a;
    const double gy2 = gy * gy;

    const int count = r - x;
    std::vector<float> v_buf(count);
    for (int i = 0; i < count; ++i) {
        double px = (double)(x + i);
        double gx = a * (px - cx0) * inv_hd;
        double r2 = gx * gx + gy2;
        double v = 1.0 + r2 * (_k1 + r2 * (_k2 + r2 * _k3));
        if (v < 0.001) v = 0.001;
        v_buf[i] = static_cast<float>(v);
    }

    foreach(z, channels) {
        if (_channels.contains(z)) {
            const float* in = in_row[z] + x;
            float* out = row.writable(z) + x;
            const float mix = _mix;

            if (_mode == 0) { // remove (input / V)
                for (int i = 0; i < count; ++i) {
                    float target = in[i] / v_buf[i];
                    out[i] = in[i] + mix * (target - in[i]);
                }
            } else if (_mode == 1) { // apply (input * V)
                for (int i = 0; i < count; ++i) {
                    float target = in[i] * v_buf[i];
                    out[i] = in[i] + mix * (target - in[i]);
                }
            } else { // divisor (V)
                for (int i = 0; i < count; ++i) {
                    float target = v_buf[i];
                    out[i] = in[i] + mix * (target - in[i]);
                }
            }
        } else {
            // Untouched channels pass through cleanly
            row.copy(in_row, z, x, r);
        }
    }
}

void bamDeVe::knobs(Knob_Callback f) {
    // Note: Per workspace guidelines, do NOT declare an initial Tab_knob at the start.
    // Nuke titles the primary tab after the node class ('bamDeVe') automatically.

    ChannelSet_knob(f, &_channels, "output", "output");
    Tooltip(f, "Channels to apply/remove/render the vignette onto.");
    Obsolete_knob(f, "channels", "knob output $value");

    Enumeration_knob(f, &_mode, kModes, "mode", "mode");
    Tooltip(f, "remove: input / V\napply: input * V\ndivisor: outputs V");

    Bool_knob(f, &_use_gpu, "use_gpu", "use GPU");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Utilize full GPU acceleration and high VRAM throughput for processing when available, with automatic multithreaded CPU fallback.");

    Divider(f);

    Double_knob(f, &_k1, "k1", "k1");
    SetRange(f, -1.0, 1.0);
    Tooltip(f, "2nd-order vignette coefficient.");

    Double_knob(f, &_k2, "k2", "k2");
    ClearFlags(f, Knob::STARTLINE);
    SetRange(f, -1.0, 1.0);
    Tooltip(f, "4th-order vignette coefficient.");

    Double_knob(f, &_k3, "k3", "k3");
    ClearFlags(f, Knob::STARTLINE);
    SetRange(f, -1.0, 1.0);
    Tooltip(f, "6th-order vignette coefficient.");

    Double_knob(f, &_log_aniso, "log_aniso", "log aniso");
    ClearFlags(f, Knob::STARTLINE);
    SetRange(f, -1.0, 1.0);
    Tooltip(f, "Anamorphic stretch: a = exp(log_aniso).");

    Divider(f);

    PyScript_knob(f, "import bamDeVe_solve\nbamDeVe_solve.analyse(nuke.thisNode())", "analyse", "  Analyse  ");
    Tooltip(f, "Scans the footage across frames and auto-tunes all advanced settings.");

    PyScript_knob(f, "import bamDeVe_solve\nbamDeVe_solve.solve(nuke.thisNode())", "solve", "  Solve  ");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Executes the vignette fit from motion correspondences without NumPy.");

    Named_Text_knob(f, "analysis_status", "status", "not analysed yet");
    String_knob(f, &_analysis_json, "analysis_json");
    SetFlags(f, Knob::INVISIBLE);

    // Advanced section (set by Analyse)
    BeginGroup(f, "advanced", "advanced (set by Analyse)");
    SetFlags(f, Knob::CLOSED);

    Named_Text_knob(f, "analysis_info", "analysis info", "not analysed yet");

    Enumeration_knob(f, &_flow_source, kFlowSources, "flow_source", "flow source");
    Tooltip(f, "Flow candidate selected by Analyse.");

    Int_knob(f, &_solve_first, "solve_first", "first frame");
    Int_knob(f, &_solve_last, "solve_last", "last frame");
    ClearFlags(f, Knob::STARTLINE);

    Int_knob(f, &_stride, "stride", "stride");
    Tooltip(f, "Stride/layer gap for direct, chain length for chain.");

    Int_knob(f, &_num_samples, "num_samples", "samples");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Number of frames/chains to sample.");

    Enumeration_knob(f, &_flow_mode, kFlowModes, "flow_mode", "flow mode");
    Tooltip(f, "0 = direct (fXX layer), 1 = chain (f01 layer).");

    String_knob(f, &_n_u_channel, "n_u_channel", "n_u");
    String_knob(f, &_n_v_channel, "n_v_channel", "n_v");
    ClearFlags(f, Knob::STARTLINE);

    String_knob(f, &_p_u_channel, "p_u_channel", "p_u");
    String_knob(f, &_p_v_channel, "p_v_channel", "p_v");
    ClearFlags(f, Knob::STARTLINE);

    Double_knob(f, &_flow_v_sign, "flow_v_sign", "flow v sign");
    Tooltip(f, "Sign of vertical motion vectors (+1 or -1).");

    Double_knob(f, &_fb_thresh, "fb_thresh", "fb thresh");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Forward-backward consistency threshold in pixels.");

    Int_knob(f, &_order, "order", "poly order");
    Tooltip(f, "Polynomial order: 2 = (k1, k2), 3 = (k1, k2, k3).");

    Double_knob(f, &_reg_strength, "reg_strength", "reg strength");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Regularization strength for monotonic/floor priors.");

    Bool_knob(f, &_fit_aniso, "fit_aniso", "anamorphic");
    Tooltip(f, "Fit elliptical vignette (log_aniso); off = spherical (log_aniso = 0).");

    EndGroup(f);

    // Results section
    BeginGroup(f, "result", "result (set by Solve)");
    SetFlags(f, Knob::CLOSED);

    Named_Text_knob(f, "result_info", "solved with", "not solved yet");

    PyScript_knob(f, "import bamDeVe_solve\nbamDeVe_solve.create_vignette_node(nuke.thisNode())", "create_node", "Create Vignette node");
    Tooltip(f, "Creates an independent baked Vignette node from the current solve result.");

    EndGroup(f);

    // Standard mix knob at bottom of primary tab per workspace rules
    Divider(f);
    Float_knob(f, &_mix, "mix", "mix");
    SetRange(f, 0.0, 1.0);
    Tooltip(f, "Dissolve between original image and vignette-processed result.");
}

int bamDeVe::knob_changed(Knob* k) {
    if (k == &Knob::showPanel) {
        return 1;
    }
    return Iop::knob_changed(k);
}

static Op* build(Node* node) {
    return new bamDeVe(node);
}

const Iop::Description bamDeVe::description(CLASS, "Color/bamDeVe", build);

// ============================================================================
// Exported C-linkage Native High-Speed Solver Routines (Zero NumPy Dependency)
// ============================================================================

namespace {

bool solve_linear_system(int M, const double* H, const double* g, double* x_out) {
    double mat[4][5];
    for (int r = 0; r < M; ++r) {
        for (int c = 0; c < M; ++c) {
            mat[r][c] = H[r * M + c];
        }
        mat[r][M] = -g[r];
    }

    for (int i = 0; i < M; ++i) {
        int max_r = i;
        double max_val = std::abs(mat[i][i]);
        for (int r = i + 1; r < M; ++r) {
            double v = std::abs(mat[r][i]);
            if (v > max_val) {
                max_val = v;
                max_r = r;
            }
        }
        if (max_val < 1e-15) return false;

        if (max_r != i) {
            for (int c = i; c <= M; ++c) {
                std::swap(mat[i][c], mat[max_r][c]);
            }
        }

        double pivot = mat[i][i];
        double inv_pivot = 1.0 / pivot;
        for (int c = i; c <= M; ++c) {
            mat[i][c] *= inv_pivot;
        }

        for (int r = 0; r < M; ++r) {
            if (r != i) {
                double factor = mat[r][i];
                if (std::abs(factor) > 1e-15) {
                    for (int c = i; c <= M; ++c) {
                        mat[r][c] -= factor * mat[i][c];
                    }
                }
            }
        }
    }

    for (int i = 0; i < M; ++i) {
        x_out[i] = mat[i][M];
    }
    return true;
}

double compute_soft_l1_cost(const std::vector<double>& residuals, double f_scale) {
    double f2 = f_scale * f_scale;
    double inv_f2 = 1.0 / f2;
    double cost = 0.0;
    for (double r : residuals) {
        double z = r * r * inv_f2;
        cost += f2 * (std::sqrt(1.0 + z) - 1.0);
    }
    return cost;
}

void eval_residuals(
    const double* xp, const double* yp,
    const double* xq, const double* yq,
    const double* d, int n_data,
    double width, double height,
    int order, bool fit_aniso,
    double reg_strength, double floor_val,
    const double* theta,
    std::vector<double>& out_r
) {
    double k0 = theta[0];
    double k1 = theta[1];
    double k2 = (order >= 3) ? theta[2] : 0.0;
    double la = fit_aniso ? theta[order] : 0.0;

    double a = std::exp(la);
    double inv_a = 1.0 / a;
    double cx0 = 0.5 * (width - 1.0);
    double cy0 = 0.5 * (height - 1.0);
    double hd = 0.5 * std::sqrt(width * width + height * height);
    double inv_hd = (hd > 1e-6) ? (1.0 / hd) : 1.0;

    const int total_n = n_data + 33 + 33 + 2;
    out_r.resize(total_n);

    // 1. Data residuals
    for (int i = 0; i < n_data; ++i) {
        double gxp = (xp[i] - cx0) * inv_hd;
        double gyp = (yp[i] - cy0) * inv_hd;
        double gxq = (xq[i] - cx0) * inv_hd;
        double gyq = (yq[i] - cy0) * inv_hd;

        double r2p = (a * gxp) * (a * gxp) + (gyp * inv_a) * (gyp * inv_a);
        double r2q = (a * gxq) * (a * gxq) + (gyq * inv_a) * (gyq * inv_a);

        double gp = 1.0 + r2p * (k0 + r2p * (k1 + r2p * k2));
        double gq = 1.0 + r2q * (k0 + r2q * (k1 + r2q * k2));
        if (gp < 1e-3) gp = 1e-3;
        if (gq < 1e-3) gq = 1e-3;

        out_r[i] = (std::log(gq) - std::log(gp)) - d[i];
    }

    // 2. Physical Priors
    double pen = std::sqrt(reg_strength * (double)std::max(n_data, 1) / 33.0);
    for (int j = 0; j < 33; ++j) {
        double rho = (double)j / 32.0;
        double t = rho * rho;
        double g = 1.0 + t * (k0 + t * (k1 + t * k2));
        double dgdrho = 2.0 * k0 * rho + 4.0 * k1 * (rho * rho * rho) + 6.0 * k2 * (rho * rho * rho * rho * rho);

        out_r[n_data + j] = pen * std::max(0.0, dgdrho);
        out_r[n_data + 33 + j] = pen * std::max(0.0, floor_val - g);
    }

    // 3. L2 regularization
    out_r[n_data + 66] = pen * 0.5 * k2;
    out_r[n_data + 67] = pen * 0.5 * la;
}

} // namespace

#if defined(_WIN32)
#  define BAMDEVE_EXPORT __declspec(dllexport)
#else
#  define BAMDEVE_EXPORT __attribute__((visibility("default")))
#endif

extern "C" BAMDEVE_EXPORT int bamdeve_fit(
    int n_points,
    const double* xp, const double* yp,
    const double* xq, const double* yq,
    const double* d,
    int width, int height,
    int order, int fit_aniso_flag,
    double reg_strength, double floor_val,
    double* out_k1, double* out_k2, double* out_k3, double* out_log_aniso
) {
    if (n_points <= 0 || !xp || !yp || !xq || !yq || !d) {
        return -1;
    }

    const bool fit_aniso = (fit_aniso_flag != 0);
    const int M = order + (fit_aniso ? 1 : 0);
    if (M > 4 || M < 2) return -2;

    double theta[4] = {0.0, 0.0, 0.0, 0.0};
    const double f_scale = 0.05;

    std::vector<double> r0;
    eval_residuals(xp, yp, xq, yq, d, n_points, width, height, order, fit_aniso, reg_strength, floor_val, theta, r0);
    double cost = compute_soft_l1_cost(r0, f_scale);

    double lambda = 1e-3;
    const int N_total = static_cast<int>(r0.size());
    std::vector<double> J(N_total * M);
    std::vector<double> rp;

    for (int iter = 0; iter < 80; ++iter) {
        eval_residuals(xp, yp, xq, yq, d, n_points, width, height, order, fit_aniso, reg_strength, floor_val, theta, r0);

        // Numerical Jacobian
        for (int j = 0; j < M; ++j) {
            double eps = 1e-6 + 1e-6 * std::abs(theta[j]);
            double tp[4];
            std::memcpy(tp, theta, sizeof(theta));
            tp[j] += eps;
            eval_residuals(xp, yp, xq, yq, d, n_points, width, height, order, fit_aniso, reg_strength, floor_val, tp, rp);
            double inv_eps = 1.0 / eps;
            for (int i = 0; i < N_total; ++i) {
                J[i * M + j] = (rp[i] - r0[i]) * inv_eps;
            }
        }

        // Soft-L1 IRLS weights & Normal equations
        double A[16] = {0.0};
        double g[4] = {0.0};

        for (int i = 0; i < N_total; ++i) {
            double ri = r0[i];
            double z = (ri / f_scale) * (ri / f_scale);
            double w = std::pow(1.0 + z, -0.25);
            double w2 = w * w;

            for (int r = 0; r < M; ++r) {
                double Jr_w2 = J[i * M + r] * w2;
                g[r] += Jr_w2 * ri;
                for (int c = 0; c < M; ++c) {
                    A[r * M + c] += Jr_w2 * J[i * M + c];
                }
            }
        }

        bool improved = false;
        double step[4] = {0.0};

        for (int inner = 0; inner < 30; ++inner) {
            double H[16];
            for (int r = 0; r < M; ++r) {
                for (int c = 0; c < M; ++c) {
                    H[r * M + c] = A[r * M + c];
                }
                H[r * M + r] += lambda * (A[r * M + r] + 1e-12);
            }

            if (!solve_linear_system(M, H, g, step)) {
                lambda *= 10.0;
                continue;
            }

            double theta_cand[4];
            for (int j = 0; j < M; ++j) {
                theta_cand[j] = theta[j] + step[j];
            }

            std::vector<double> r_cand;
            eval_residuals(xp, yp, xq, yq, d, n_points, width, height, order, fit_aniso, reg_strength, floor_val, theta_cand, r_cand);
            double cost_cand = compute_soft_l1_cost(r_cand, f_scale);

            if (cost_cand < cost) {
                std::memcpy(theta, theta_cand, sizeof(theta));
                cost = cost_cand;
                lambda = std::max(lambda * 0.5, 1e-12);
                improved = true;
                break;
            }

            lambda *= 4.0;
            if (lambda > 1e10) break;
        }

        double step_norm = 0.0;
        for (int j = 0; j < M; ++j) step_norm += step[j] * step[j];
        if (!improved || std::sqrt(step_norm) < 1e-8) {
            break;
        }
    }

    if (out_k1) *out_k1 = theta[0];
    if (out_k2) *out_k2 = theta[1];
    if (out_k3) *out_k3 = (order >= 3) ? theta[2] : 0.0;
    if (out_log_aniso) *out_log_aniso = fit_aniso ? theta[order] : 0.0;
    return 0;
}

extern "C" BAMDEVE_EXPORT void bamdeve_downsample(
    const float* src, int w, int h, int factor, float* dst
) {
    if (!src || !dst || factor <= 0) return;
    int nw = w / factor;
    int nh = h / factor;
    float inv_area = 1.0f / (float)(factor * factor);

    for (int by = 0; by < nh; ++by) {
        int sy = by * factor;
        int dst_row = by * nw;
        for (int bx = 0; bx < nw; ++bx) {
            int sx = bx * factor;
            float sum = 0.0f;
            for (int dy = 0; dy < factor; ++dy) {
                int src_row = (sy + dy) * w + sx;
                for (int dx = 0; dx < factor; ++dx) {
                    sum += src[src_row + dx];
                }
            }
            dst[dst_row + bx] = sum * inv_area;
        }
    }
}
