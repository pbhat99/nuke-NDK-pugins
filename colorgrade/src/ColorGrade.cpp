// SPDX-License-Identifier: MIT
// ============================================================================
//
//  ColorGrade — Enhanced Color Grading Node for Foundry Nuke
//
//  Extends Nuke's native Grade node with:
//  - Exposure in photographic stops (2^exposure), with per-channel RGB/A control
//  - Saturation in linear color space with per-channel RGB control
//  - All standard Grade controls (blackpoint, whitepoint, lift, gain, multiply,
//    offset, gamma, reverse, black clamp, white clamp)
//  - Mathematically exact forward and reverse grading
//  - Standard Nuke wrapper knobs (channels, mask, premult, mix, and Node tab)
//
// ============================================================================

#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/ArrayKnobI.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/DDMath.h"

#include <cmath>
#include <algorithm>

using namespace DD::Image;

static const char* const CLASS = "ColorGrade";
static const char* const HELP =
    "ColorGrade: Enhanced color grading node with exposure (in stops) and saturation.\n\n"
    "Combines all native Grade controls with:\n"
    "- Exposure in photographic stops (factor = 2^exposure), 4-channel control similar to multiply.\n"
    "- Saturation in linear color space.\n"
    "- Mathematically exact forward and reverse grading pipeline.\n"
    "- Standard Nuke wrapper controls (channels, mask input, (un)premult, mix).";

class ColorGrade : public PixelIop
{
    float _blackpoint[4];
    float _whitepoint[4];
    float _black[4];      // lift
    float _white[4];      // gain
    float _multiply[4];
    float _multSource[4]; // source for multiply matching
    float _multTarget[4]; // target for multiply matching
    float _exposure[4];   // in stops
    float _add[4];        // offset
    float _addSource[4];  // source for offset matching
    float _addTarget[4];  // target for offset matching
    float _gamma[4];
    float _saturation[4]; // saturation factor

    bool  _reverse;
    bool  _blackClamp;
    bool  _whiteClamp;
    bool  _isInternalUpdating;

public:
    explicit ColorGrade(Node* node) : PixelIop(node),
        _reverse(false),
        _blackClamp(true),
        _whiteClamp(false),
        _isInternalUpdating(false)
    {
        for (int n = 0; n < 4; ++n) {
            _blackpoint[n] = 0.0f;
            _whitepoint[n] = 1.0f;
            _black[n]      = 0.0f; // lift
            _white[n]      = 1.0f; // gain
            _multiply[n]   = 1.0f;
            _multSource[n] = 1.0f;
            _multTarget[n] = 1.0f;
            _exposure[n]   = 0.0f; // 0 stops = identity (2^0 = 1.0)
            _add[n]        = 0.0f; // offset
            _addSource[n]  = 0.0f;
            _addTarget[n]  = 0.0f;
            _gamma[n]      = 1.0f;
            _saturation[n] = 1.0f; // 1.0 = identity
        }
    }

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }
    bool pass_transform() const override { return true; }

    void in_channels(int input_number, ChannelSet& channels) const override
    {
        // When saturation is applied, make sure brother RGB channels are fetched together
        ChannelSet done;
        foreach (z, channels) {
            if (colourIndex(z) < 3) {
                if (!(done & z)) {
                    done.addBrothers(z, 3);
                }
            }
        }
        channels += done;
    }

    void knobs(Knob_Callback f) override
    {
        AColor_knob(f, _blackpoint, IRange(-1, 1), "blackpoint");
        Tooltip(f, "Color turned into black (0.0).");

        AColor_knob(f, _whitepoint, IRange(0, 4), "whitepoint");
        Tooltip(f, "Color turned into white (1.0).");

        AColor_knob(f, _black, IRange(-1, 1), "black", "lift");
        Tooltip(f, "Black is turned into this color.");

        AColor_knob(f, _white, IRange(0, 4), "white", "gain");
        Tooltip(f, "White is turned into this color.");

        AColor_knob(f, _multiply, IRange(0, 4), "multiply");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Linear multiplier.");

        // Foldable group for matching multiply
        BeginClosedGroup(f, "match_multiply_group", "match multiply");
        AColor_knob(f, _multSource, IRange(0, 4), "mult_source", "source");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Source color for multiply matching (multiply = target / source). Updating this calculates the absolute multiply slider.");
        AColor_knob(f, _multTarget, IRange(0, 4), "mult_target", "target");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Target color for multiply matching (multiply = target / source). Updating this calculates the absolute multiply slider.");
        EndGroup(f);

        AColor_knob(f, _exposure, IRange(-10, 10), "exposure");
        SetFlags(f, Knob::SLIDER);
        Tooltip(f, "Exposure adjustment in photographic stops (factor = 2^exposure). 3/4 channel control similar to multiply.");

        AColor_knob(f, _add, IRange(-1, 1), "add", "offset");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Constant added to result (raises both black and white).");

        // Foldable group for matching add / offset
        BeginClosedGroup(f, "match_add_group", "match offset");
        AColor_knob(f, _addSource, IRange(-1, 1), "add_source", "source");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Source color for offset matching (offset = target - source). Updating this calculates the absolute offset slider.");
        AColor_knob(f, _addTarget, IRange(-1, 1), "add_target", "target");
        SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);
        Tooltip(f, "Target color for offset matching (offset = target - source). Updating this calculates the absolute offset slider.");
        EndGroup(f);

        AColor_knob(f, _gamma, IRange(0.2, 5), "gamma");
        Tooltip(f, "Gamma correction exponent applied to values.");

        AColor_knob(f, _saturation, IRange(0, 4), "saturation");
        SetFlags(f, Knob::SLIDER);
        Tooltip(f, "Color saturation in linear space (0 = monochrome, 1 = unchanged, >1 = saturated).");

        Bool_knob(f, &_reverse, "reverse");
        SetFlags(f, Knob::STARTLINE);
        Tooltip(f, "Invert the math to precisely undo the color grade.");

        Bool_knob(f, &_blackClamp, "black_clamp", "black clamp");
        Tooltip(f, "Clamps output values less than zero to zero.");

        Bool_knob(f, &_whiteClamp, "white_clamp", "white clamp");
        Tooltip(f, "Clamps output values greater than one to one.");
    }

    int knob_changed(Knob* k) override
    {
        if (_isInternalUpdating) {
            return PixelIop::knob_changed(k);
        }

        // Multiply match: source or target adjusted
        if (k->is("mult_source") || k->is("mult_target")) {
            _isInternalUpdating = true;
            Knob* srcK = knob("mult_source");
            Knob* tgtK = knob("mult_target");
            Knob* multK = knob("multiply");
            if (srcK && tgtK && multK && srcK->arrayKnob() && tgtK->arrayKnob()) {
                const auto& sVec = srcK->arrayKnob()->array_at();
                const auto& tVec = tgtK->arrayKnob()->array_at();
                for (int i = 0; i < 4; ++i) {
                    const double s = (i < (int)sVec.size()) ? sVec[i] : 1.0;
                    const double t = (i < (int)tVec.size()) ? tVec[i] : 1.0;
                    const double m = (std::fabs(s) > 1e-6) ? (t / s) : t;
                    _multiply[i] = (float)m;
                    _multSource[i] = (float)s;
                    _multTarget[i] = (float)t;
                    multK->set_value(m, i);
                }
            }
            _isInternalUpdating = false;
            return 1;
        }

        // Direct adjustment of main multiply slider -> resets source and target to default (1.0)
        if (k->is("multiply")) {
            _isInternalUpdating = true;
            Knob* srcK = knob("mult_source");
            Knob* tgtK = knob("mult_target");
            for (int i = 0; i < 4; ++i) {
                _multSource[i] = 1.0f;
                _multTarget[i] = 1.0f;
                if (srcK) srcK->set_value(1.0, i);
                if (tgtK) tgtK->set_value(1.0, i);
            }
            if (ArrayKnobI* mk = k->arrayKnob()) {
                const auto& mVec = mk->array_at();
                for (int i = 0; i < 4 && i < (int)mVec.size(); ++i) {
                    _multiply[i] = (float)mVec[i];
                }
            }
            _isInternalUpdating = false;
            return 1;
        }

        // Add / offset match: source or target adjusted
        if (k->is("add_source") || k->is("add_target")) {
            _isInternalUpdating = true;
            Knob* srcK = knob("add_source");
            Knob* tgtK = knob("add_target");
            Knob* addK = knob("add");
            if (srcK && tgtK && addK && srcK->arrayKnob() && tgtK->arrayKnob()) {
                const auto& sVec = srcK->arrayKnob()->array_at();
                const auto& tVec = tgtK->arrayKnob()->array_at();
                for (int i = 0; i < 4; ++i) {
                    const double s = (i < (int)sVec.size()) ? sVec[i] : 0.0;
                    const double t = (i < (int)tVec.size()) ? tVec[i] : 0.0;
                    const double a = t - s;
                    _add[i] = (float)a;
                    _addSource[i] = (float)s;
                    _addTarget[i] = (float)t;
                    addK->set_value(a, i);
                }
            }
            _isInternalUpdating = false;
            return 1;
        }

        // Direct adjustment of main add slider -> resets source and target to default (0.0)
        if (k->is("add")) {
            _isInternalUpdating = true;
            Knob* srcK = knob("add_source");
            Knob* tgtK = knob("add_target");
            for (int i = 0; i < 4; ++i) {
                _addSource[i] = 0.0f;
                _addTarget[i] = 0.0f;
                if (srcK) srcK->set_value(0.0, i);
                if (tgtK) tgtK->set_value(0.0, i);
            }
            if (ArrayKnobI* ak = k->arrayKnob()) {
                const auto& aVec = ak->array_at();
                for (int i = 0; i < 4 && i < (int)aVec.size(); ++i) {
                    _add[i] = (float)aVec[i];
                }
            }
            _isInternalUpdating = false;
            return 1;
        }

        return PixelIop::knob_changed(k);
    }

    void _validate(bool for_real) override
    {
        bool change_any = _blackClamp || _whiteClamp;
        bool change_zero = false;

        for (int z = 0; z < 4; ++z) {
            const float expMult = std::exp2(_exposure[z]);
            float A = _whitepoint[z] - _blackpoint[z];
            A = (A != 0.0f) ? (_white[z] - _black[z]) / A : 10000.0f;
            A *= _multiply[z] * expMult;
            float B = _add[z] + _black[z] - _blackpoint[z] * A;

            if (A != 1.0f || B != 0.0f || _gamma[z] != 1.0f || _saturation[z] != 1.0f) {
                change_any = true;
                if (B != 0.0f)
                    change_zero = true;
            }
        }

        set_out_channels(change_any ? Mask_All : Mask_None);
        PixelIop::_validate(for_real);
        if (change_zero)
            info_.black_outside(false);
    }

    // Forward gamma evaluation (Nuke standard with linear highlight rolloff)
    static inline float applyGamma(float x, float G)
    {
        if (G <= 0.0f) {
            if (x < 0.0f) return 0.0f;
            if (x > 1.0f) return 1.0f;
            return x;
        }
        if (G != 1.0f) {
            float invG = 1.0f / G;
            if (x <= 0.0f)
                return x;
            if (x < 1.0f)
                return std::pow(x, invG);
            return 1.0f + (x - 1.0f) * invG;
        }
        return x;
    }

    // Reverse gamma evaluation
    static inline float reverseGamma(float x, float G)
    {
        if (G <= 0.0f) {
            return (x > 0.0f) ? 1.0f : 0.0f;
        }
        if (G != 1.0f) {
            if (x <= 0.0f)
                return x;
            if (x < 1.0f)
                return std::pow(x, G);
            return 1.0f + (x - 1.0f) * G;
        }
        return x;
    }

    // Linear saturation application (Rec.709 weights)
    static inline void applySaturation(float& r, float& g, float& b,
                                       float sr, float sg, float sb)
    {
        if (sr == 1.0f && sg == 1.0f && sb == 1.0f) return;
        const float Y = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        r = Y + sr * (r - Y);
        g = Y + sg * (g - Y);
        b = Y + sb * (b - Y);
    }

    // Exact inverse saturation
    static inline void reverseSaturation(float& r, float& g, float& b,
                                         float sr, float sg, float sb)
    {
        if (sr == 1.0f && sg == 1.0f && sb == 1.0f) return;

        // Uniform saturation (common case)
        if (sr == sg && sg == sb) {
            if (sr != 0.0f) {
                const float Y = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                const float invS = 1.0f / sr;
                r = Y + (r - Y) * invS;
                g = Y + (g - Y) * invS;
                b = Y + (b - Y) * invS;
            }
            return;
        }

        // Per-channel saturation inversion
        const float eps = 1e-6f;
        const float invSr = (std::fabs(sr) > eps) ? (1.0f / sr) : 0.0f;
        const float invSg = (std::fabs(sg) > eps) ? (1.0f / sg) : 0.0f;
        const float invSb = (std::fabs(sb) > eps) ? (1.0f / sb) : 0.0f;

        const float denom = 0.2126f * invSr + 0.7152f * invSg + 0.0722f * invSb;
        if (std::fabs(denom) > eps) {
            const float Y = (0.2126f * r * invSr + 0.7152f * g * invSg + 0.0722f * b * invSb) / denom;
            r = Y + (r - Y) * invSr;
            g = Y + (g - Y) * invSg;
            b = Y + (b - Y) * invSb;
        }
    }

    void pixel_engine(const Row& in, int y, int x, int r,
                      ChannelMask channels, Row& out) override
    {
        // Compute linear grade coefficients A and B for all 4 channels
        float A[4];
        float B[4];

        for (int z = 0; z < 4; ++z) {
            const float expMult = std::exp2(_exposure[z]);
            float denom = _whitepoint[z] - _blackpoint[z];
            float a = (denom != 0.0f) ? (_white[z] - _black[z]) / denom : 10000.0f;
            a *= _multiply[z] * expMult;
            float b = _add[z] + _black[z] - _blackpoint[z] * a;

            if (_reverse) {
                float invA = (a != 0.0f) ? (1.0f / a) : 1.0f;
                b = -b * invA;
                a = invA;
            }
            A[z] = a;
            B[z] = b;
        }

        ChannelSet done;

        foreach (z, channels) {
            if (done & z)
                continue;

            const unsigned cIdx = colourIndex(z);

            // RGB color processing: group R, G, B together for saturation
            if (cIdx < 3) {
                Channel rchan = brother(z, 0);
                Channel gchan = brother(z, 1);
                Channel bchan = brother(z, 2);

                done += rchan;
                done += gchan;
                done += bchan;

                const float* rIn = in[rchan] + x;
                const float* gIn = in[gchan] + x;
                const float* bIn = in[bchan] + x;

                const bool writeR = channels.contains(rchan);
                const bool writeG = channels.contains(gchan);
                const bool writeB = channels.contains(bchan);

                float* rOut = writeR ? (out.writable(rchan) + x) : nullptr;
                float* gOut = writeG ? (out.writable(gchan) + x) : nullptr;
                float* bOut = writeB ? (out.writable(bchan) + x) : nullptr;

                const int w = r - x;

                if (!_reverse) {
                    // Forward pipeline: Linear -> Gamma -> Saturation -> Clamp
                    for (int i = 0; i < w; ++i) {
                        float rVal = rIn[i] * A[0] + B[0];
                        float gVal = gIn[i] * A[1] + B[1];
                        float bVal = bIn[i] * A[2] + B[2];

                        rVal = applyGamma(rVal, _gamma[0]);
                        gVal = applyGamma(gVal, _gamma[1]);
                        bVal = applyGamma(bVal, _gamma[2]);

                        applySaturation(rVal, gVal, bVal, _saturation[0], _saturation[1], _saturation[2]);

                        if (_blackClamp) {
                            rVal = std::max(rVal, 0.0f);
                            gVal = std::max(gVal, 0.0f);
                            bVal = std::max(bVal, 0.0f);
                        }
                        if (_whiteClamp) {
                            rVal = std::min(rVal, 1.0f);
                            gVal = std::min(gVal, 1.0f);
                            bVal = std::min(bVal, 1.0f);
                        }

                        if (rOut) rOut[i] = rVal;
                        if (gOut) gOut[i] = gVal;
                        if (bOut) bOut[i] = bVal;
                    }
                } else {
                    // Reverse pipeline: Clamp -> InvSaturation -> InvGamma -> InvLinear
                    for (int i = 0; i < w; ++i) {
                        float rVal = rIn[i];
                        float gVal = gIn[i];
                        float bVal = bIn[i];

                        if (_blackClamp) {
                            rVal = std::max(rVal, 0.0f);
                            gVal = std::max(gVal, 0.0f);
                            bVal = std::max(bVal, 0.0f);
                        }
                        if (_whiteClamp) {
                            rVal = std::min(rVal, 1.0f);
                            gVal = std::min(gVal, 1.0f);
                            bVal = std::min(bVal, 1.0f);
                        }

                        reverseSaturation(rVal, gVal, bVal, _saturation[0], _saturation[1], _saturation[2]);

                        rVal = reverseGamma(rVal, _gamma[0]);
                        gVal = reverseGamma(gVal, _gamma[1]);
                        bVal = reverseGamma(bVal, _gamma[2]);

                        rVal = rVal * A[0] + B[0];
                        gVal = gVal * A[1] + B[1];
                        bVal = bVal * A[2] + B[2];

                        if (rOut) rOut[i] = rVal;
                        if (gOut) gOut[i] = gVal;
                        if (bOut) bOut[i] = bVal;
                    }
                }
            } else {
                // Non-RGB channels (Alpha, etc.)
                done += z;

                const float* inPtr = in[z] + x;
                float* outPtr = out.writable(z) + x;
                const int w = r - x;

                const int comp = std::min(static_cast<int>(cIdx), 3);
                const float aCoeff = A[comp];
                const float bCoeff = B[comp];
                const float gCoeff = _gamma[comp];

                if (!_reverse) {
                    for (int i = 0; i < w; ++i) {
                        float val = inPtr[i] * aCoeff + bCoeff;
                        val = applyGamma(val, gCoeff);
                        if (_blackClamp) val = std::max(val, 0.0f);
                        if (_whiteClamp) val = std::min(val, 1.0f);
                        outPtr[i] = val;
                    }
                } else {
                    for (int i = 0; i < w; ++i) {
                        float val = inPtr[i];
                        if (_blackClamp) val = std::max(val, 0.0f);
                        if (_whiteClamp) val = std::min(val, 1.0f);
                        val = reverseGamma(val, gCoeff);
                        val = val * aCoeff + bCoeff;
                        outPtr[i] = val;
                    }
                }
            }
        }
    }

    static const Op::Description d;
};

static Op* build(Node* node)
{
    return (new NukeWrapper(new ColorGrade(node)))->channelsRGBoptionalAlpha()->mixLuminance();
}

const Op::Description ColorGrade::d(CLASS, build);
