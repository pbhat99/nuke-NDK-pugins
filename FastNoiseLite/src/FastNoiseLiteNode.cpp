// SPDX-License-Identifier: MIT
//
// ============================================================================
//
//  FastNoiseLiteNode — Procedural Noise Generator & Filter for Foundry Nuke
//
//  Powered by Auburn's FastNoiseLite library (https://github.com/Auburn/FastNoiseLite)
//  Exposes 100% of FastNoiseLite controls to Nuke artists:
//  - 6 Noise Types (OpenSimplex2, OpenSimplex2S, Cellular, Perlin, ValueCubic, Value)
//  - 6 Fractal Types (FBm, Ridged, PingPong, DomainWarp, etc.) with octaves, lacunarity, gain
//  - Cellular Voronoi with 4 distance functions and 7 return types
//  - Domain Warping (OpenSimplex2, BasicGrid) with amplitude control
//  - 2D Screen Space and 3D Position Channel (P pass) coordinate sources
//  - Monochrome and RGB Color seed modes
//  - Built-in color grading (blackpoint, whitepoint, lift, multiply, gamma)
//
// ============================================================================

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/ChannelSet.h"
#include "DDImage/Format.h"

#include "FastNoiseLite.h"

#include <cmath>
#include <algorithm>
#include <vector>

using namespace DD::Image;

static const char* const CLASS = "FastNoiseLite";
static const char* const HELP =
    "FastNoiseLite: High-performance procedural noise generator.\n\n"
    "Features:\n"
    "- 6 Noise Types: OpenSimplex2, OpenSimplex2S, Cellular (Voronoi), Perlin, ValueCubic, Value.\n"
    "- 6 Fractal Modes: FBm, Ridged, PingPong, DomainWarp, etc.\n"
    "- Cellular / Voronoi with 4 distance metrics and 7 return types.\n"
    "- Domain Warping for organic turbulence and fluid distortion.\n"
    "- Supports 2D screen coordinates and 3D Position pass (P) sampling.\n"
    "- Built-in color grading, monochrome/color seed modes, and mask input.";

// ----------------------------------------------------------------------------
// Enumeration labels
// ----------------------------------------------------------------------------

static const char* const noiseTypeNames[] = {
    "OpenSimplex2",
    "OpenSimplex2S",
    "Cellular",
    "Perlin",
    "ValueCubic",
    "Value",
    nullptr
};

static const char* const rotationTypeNames[] = {
    "None",
    "Improve XY Planes",
    "Improve XZ Planes",
    nullptr
};

static const char* const fractalTypeNames[] = {
    "None",
    "FBm",
    "Ridged",
    "PingPong",
    "DomainWarp Progressive",
    "DomainWarp Independent",
    nullptr
};

static const char* const cellularDistanceNames[] = {
    "Euclidean",
    "EuclideanSq",
    "Manhattan",
    "Hybrid",
    nullptr
};

static const char* const cellularReturnNames[] = {
    "CellValue",
    "Distance",
    "Distance2",
    "Distance2Add",
    "Distance2Sub",
    "Distance2Mul",
    "Distance2Div",
    nullptr
};

static const char* const domainWarpTypeNames[] = {
    "OpenSimplex2",
    "OpenSimplex2Reduced",
    "BasicGrid",
    nullptr
};

static const char* const coordSourceNames[] = {
    "2D Screen Coordinates (XY)",
    "3D Position Channel (P)",
    nullptr
};

static const char* const colorModeNames[] = {
    "Monochrome (R=G=B)",
    "Color (RGB Independent Seeds)",
    nullptr
};

static const char* const outputRangeNames[] = {
    "0 to 1 (Normalized)",
    "-1 to 1 (Raw Noise)",
    nullptr
};

// ----------------------------------------------------------------------------
// FastNoiseLiteNode Class
// ----------------------------------------------------------------------------

class FastNoiseLiteNode : public Iop
{
    // Channels
    ChannelSet _channels;
    ChannelSet _pChannels;

    // Noise parameters
    int   _noiseType;
    int   _seed;
    float _frequency;
    int   _rotationType3D;

    // Transform
    float _offset[3];
    float _scale[3];
    float _zEvolution;

    // Fractal parameters
    int   _fractalType;
    int   _octaves;
    float _lacunarity;
    float _gain;
    float _weightedStrength;
    float _pingPongStrength;

    // Cellular parameters
    int   _cellularDistance;
    int   _cellularReturn;
    float _cellularJitter;

    // Domain Warp
    bool  _enableDomainWarp;
    int   _domainWarpType;
    float _domainWarpAmp;

    // Coordinates & Color
    int   _coordSource;
    int   _colorMode;
    int   _outputRange;
    bool  _invert;

    // Grade
    float _blackpoint;
    float _whitepoint;
    float _lift;
    float _multiply;
    float _gamma;

    // Output
    float _mix;

    // FastNoiseLite engines (primary for mono/R, plus G and B for color mode)
    FastNoiseLite _fnlR;
    FastNoiseLite _fnlG;
    FastNoiseLite _fnlB;

public:
    explicit FastNoiseLiteNode(Node* node) : Iop(node),
        _channels(Mask_RGBA),
        _pChannels(Chan_Black),
        _noiseType(FastNoiseLite::NoiseType_OpenSimplex2),
        _seed(1337),
        _frequency(0.01f),
        _rotationType3D(FastNoiseLite::RotationType3D_None),
        _zEvolution(0.0f),
        _fractalType(FastNoiseLite::FractalType_FBm),
        _octaves(3),
        _lacunarity(2.0f),
        _gain(0.5f),
        _weightedStrength(0.0f),
        _pingPongStrength(2.0f),
        _cellularDistance(FastNoiseLite::CellularDistanceFunction_EuclideanSq),
        _cellularReturn(FastNoiseLite::CellularReturnType_Distance),
        _cellularJitter(1.0f),
        _enableDomainWarp(false),
        _domainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2),
        _domainWarpAmp(30.0f),
        _coordSource(0),
        _colorMode(0),
        _outputRange(0),
        _invert(false),
        _blackpoint(0.0f),
        _whitepoint(1.0f),
        _lift(0.0f),
        _multiply(1.0f),
        _gamma(1.0f),
        _mix(1.0f)
    {
        _offset[0] = _offset[1] = _offset[2] = 0.0f;
        _scale[0]  = _scale[1]  = _scale[2]  = 1.0f;
    }

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }

    int minimum_inputs() const override { return 0; }
    int maximum_inputs() const override { return 2; }

    bool test_input(int n, Op* op) const override
    {
        if (n == 0 || n == 1)
            return dynamic_cast<Iop*>(op) != nullptr;
        return false;
    }

    const char* input_label(int input, char* /*buffer*/) const override
    {
        switch (input) {
            case 0: return "";
            case 1: return "mask";
            default: return "";
        }
    }

    // ------------------------------------------------------------------------
    // Knobs
    // ------------------------------------------------------------------------
    void knobs(Knob_Callback f) override
    {
        // Standard Nuke channel output knob
        ChannelSet_knob(f, &_channels, "output", "output");
        Tooltip(f, "Channels to write the noise into.");
        Obsolete_knob(f, "channels", "knob output $value");

        Enumeration_knob(f, &_noiseType, noiseTypeNames, "noise_type", "type");
        Tooltip(f, "Core noise algorithm:\n"
                   "- OpenSimplex2: Smooth, modern simplex noise.\n"
                   "- OpenSimplex2S: Faster variant with smooth evaluation.\n"
                   "- Cellular: Voronoi / cellular structure.\n"
                   "- Perlin: Classic Perlin gradient noise.\n"
                   "- ValueCubic: Smooth bicubic value noise.\n"
                   "- Value: Standard value noise.");

        Int_knob(f, &_seed, "seed", "seed");
        Tooltip(f, "Random seed used for generating the noise field.");

        Float_knob(f, &_frequency, "frequency", "frequency");
        SetRange(f, 0.0001f, 0.1f);
        Tooltip(f, "Base frequency of the noise pattern.");

        XYZ_knob(f, _scale, "scale", "scale");
        SetRange(f, 0.01f, 100.0f);
        Tooltip(f, "Scale factor in X, Y, and Z.");

        XYZ_knob(f, _offset, "offset", "offset");
        Tooltip(f, "Spatial translation in X, Y, and Z.");

        Float_knob(f, &_zEvolution, "z_evolution", "z / evolution");
        Tooltip(f, "Z slice coordinate for 2D noise. Animate this knob (e.g. frame * 0.1) to create evolving noise.");

        Enumeration_knob(f, &_rotationType3D, rotationTypeNames, "rotation_type_3d", "3D rotation");
        Tooltip(f, "Domain rotation for 3D sampling to reduce directional alignment artifacts.");

        // Fractal Group
        Divider(f, "Fractal");
        Enumeration_knob(f, &_fractalType, fractalTypeNames, "fractal_type", "fractal");
        Tooltip(f, "Method for combining octaves:\n"
                   "- None: Single octave.\n"
                   "- FBm: Fractional Brownian Motion.\n"
                   "- Ridged: Absolute value inverted for sharp ridges.\n"
                   "- PingPong: Bounces between limits for billowy patterns.\n"
                   "- DomainWarp Progressive / Independent: Fractal domain distortion.");

        Int_knob(f, &_octaves, "octaves", "octaves");
        SetRange(f, 1, 10);
        Tooltip(f, "Number of noise octaves to sum.");

        Float_knob(f, &_lacunarity, "lacunarity", "lacunarity");
        SetRange(f, 1.0f, 4.0f);
        Tooltip(f, "Frequency multiplier per octave (typically 2.0).");

        Float_knob(f, &_gain, "gain", "gain");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Amplitude multiplier per octave (typically 0.5).");

        Float_knob(f, &_weightedStrength, "weighted_strength", "weighted strength");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Weighting factor favoring lower octaves.");

        Float_knob(f, &_pingPongStrength, "ping_pong_strength", "ping pong strength");
        SetRange(f, 0.0f, 10.0f);
        Tooltip(f, "Strength of the ping-pong folding effect.");

        // Cellular Group
        Divider(f, "Cellular (Voronoi)");
        Enumeration_knob(f, &_cellularDistance, cellularDistanceNames, "cellular_distance", "distance function");
        Tooltip(f, "Distance metric to nearest Voronoi feature point:\n"
                   "- Euclidean: True radial distance.\n"
                   "- EuclideanSq: Squared Euclidean distance (faster, sharper).\n"
                   "- Manhattan: Diamond / grid aligned distance.\n"
                   "- Hybrid: Combined metric.");

        Enumeration_knob(f, &_cellularReturn, cellularReturnNames, "cellular_return", "return type");
        Tooltip(f, "Value returned from cellular calculation:\n"
                   "- CellValue: Unique random value per Voronoi cell.\n"
                   "- Distance: Distance to nearest cell point.\n"
                   "- Distance2: Distance to second nearest point.\n"
                   "- Distance2Add / Sub / Mul / Div: Mathematical combinations creating borders and facets.");

        Float_knob(f, &_cellularJitter, "cellular_jitter", "jitter");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Maximum displacement of cellular feature points from the grid (0 = regular grid, 1 = fully random).");

        // Domain Warp Group
        Divider(f, "Domain Warp");
        Bool_knob(f, &_enableDomainWarp, "enable_domain_warp", "enable domain warp");
        Tooltip(f, "Enables domain distortion before noise sampling to create swirling, organic turbulence.");

        Enumeration_knob(f, &_domainWarpType, domainWarpTypeNames, "domain_warp_type", "warp type");
        Tooltip(f, "Algorithm used for domain coordinate warping.");

        Float_knob(f, &_domainWarpAmp, "domain_warp_amp", "warp amplitude");
        SetRange(f, 0.0f, 100.0f);
        Tooltip(f, "Maximum displacement distance for domain warping.");

        // Standard Mix Knob
        Divider(f, "");
        Float_knob(f, &_mix, "mix", "mix");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Dissolve between original input (0) and noise output (1).");

        // --- Tab 2: Color & Coordinates ---
        Tab_knob(f, "Color & Coordinates");

        Enumeration_knob(f, &_coordSource, coordSourceNames, "coord_source", "coordinate source");
        Tooltip(f, "Coordinate system for noise generation:\n"
                   "- 2D Screen Coordinates (XY): Generates noise over the image plane.\n"
                   "- 3D Position Channel (P): Samples coordinates from an input 3D position pass.");

        ChannelSet_knob(f, &_pChannels, "p_channels", "P channels");
        Tooltip(f, "Input Position channel set (e.g. P.x, P.y, P.z) when using 3D Position source.");

        Enumeration_knob(f, &_colorMode, colorModeNames, "color_mode", "color mode");
        Tooltip(f, "Monochrome: R, G, and B receive the same noise pattern.\n"
                   "Color: R, G, and B use independent random seeds for full RGB color noise.");

        Enumeration_knob(f, &_outputRange, outputRangeNames, "output_range", "output range");
        Tooltip(f, "Output value mapping:\n"
                   "- 0 to 1: Remaps -1..1 noise into 0..1 range (value * 0.5 + 0.5).\n"
                   "- -1 to 1: Raw noise output.");

        Bool_knob(f, &_invert, "invert", "invert");
        Tooltip(f, "Inverts the noise values.");

        // Grade Controls
        Divider(f, "Grade / Remap");
        Float_knob(f, &_blackpoint, "blackpoint", "blackpoint");
        Tooltip(f, "Color blackpoint.");

        Float_knob(f, &_whitepoint, "whitepoint", "whitepoint");
        Tooltip(f, "Color whitepoint.");

        Float_knob(f, &_lift, "lift", "lift / offset");
        Tooltip(f, "Color lift offset.");

        Float_knob(f, &_multiply, "multiply", "multiply");
        Tooltip(f, "Color multiplier.");

        Float_knob(f, &_gamma, "gamma", "gamma");
        SetRange(f, 0.1f, 4.0f);
        Tooltip(f, "Color gamma curve.");
    }

    int knob_changed(Knob* k) override
    {
        if (k->is("noise_type")) {
            const bool isCellular = (_noiseType == FastNoiseLite::NoiseType_Cellular);
            if (knob("cellular_distance")) knob("cellular_distance")->enable(isCellular);
            if (knob("cellular_return"))   knob("cellular_return")->enable(isCellular);
            if (knob("cellular_jitter"))   knob("cellular_jitter")->enable(isCellular);
            return 1;
        }
        if (k->is("fractal_type")) {
            const bool hasFractal = (_fractalType != FastNoiseLite::FractalType_None);
            if (knob("octaves"))            knob("octaves")->enable(hasFractal);
            if (knob("lacunarity"))         knob("lacunarity")->enable(hasFractal);
            if (knob("gain"))               knob("gain")->enable(hasFractal);
            if (knob("weighted_strength"))  knob("weighted_strength")->enable(hasFractal);
            if (knob("ping_pong_strength")) knob("ping_pong_strength")->enable(_fractalType == FastNoiseLite::FractalType_PingPong);
            return 1;
        }
        if (k->is("enable_domain_warp")) {
            if (knob("domain_warp_type")) knob("domain_warp_type")->enable(_enableDomainWarp);
            if (knob("domain_warp_amp"))  knob("domain_warp_amp")->enable(_enableDomainWarp);
            return 1;
        }
        if (k->is("coord_source")) {
            if (knob("p_channels")) knob("p_channels")->enable(_coordSource == 1);
            return 1;
        }
        return Iop::knob_changed(k);
    }

    // Configure a FastNoiseLite engine instance
    void configureEngine(FastNoiseLite& fnl, int seed) const
    {
        fnl.SetSeed(seed);
        fnl.SetFrequency(_frequency);
        fnl.SetNoiseType(static_cast<FastNoiseLite::NoiseType>(_noiseType));
        fnl.SetRotationType3D(static_cast<FastNoiseLite::RotationType3D>(_rotationType3D));
        fnl.SetFractalType(static_cast<FastNoiseLite::FractalType>(_fractalType));
        fnl.SetFractalOctaves(_octaves);
        fnl.SetFractalLacunarity(_lacunarity);
        fnl.SetFractalGain(_gain);
        fnl.SetFractalWeightedStrength(_weightedStrength);
        fnl.SetFractalPingPongStrength(_pingPongStrength);
        fnl.SetCellularDistanceFunction(static_cast<FastNoiseLite::CellularDistanceFunction>(_cellularDistance));
        fnl.SetCellularReturnType(static_cast<FastNoiseLite::CellularReturnType>(_cellularReturn));
        fnl.SetCellularJitter(_cellularJitter);
        fnl.SetDomainWarpType(static_cast<FastNoiseLite::DomainWarpType>(_domainWarpType));
        fnl.SetDomainWarpAmp(_domainWarpAmp);
    }

    void _validate(bool for_real) override
    {
        // Setup FastNoise engines
        configureEngine(_fnlR, _seed);
        configureEngine(_fnlG, _seed + 101);
        configureEngine(_fnlB, _seed + 202);

        if (!input(0)) {
            // Standalone generator: use default format
            info_.format(input_format());
            info_.set(input_format());
            info_.channels(_channels);
        } else {
            // Filter mode: copy info from input 0
            copy_info();
            set_out_channels(_channels);
            info_.channels() += _channels;
        }
    }

    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override
    {
        if (input(0)) {
            ChannelSet req(channels);
            if (_coordSource == 1 && _pChannels != Chan_Black) {
                req += _pChannels;
            }
            input(0)->request(x, y, r, t, req, count);
        }

        // Mask input
        if (input(1)) {
            input(1)->request(x, y, r, t, Mask_Alpha, count);
        }
    }

    // Apply color grading to raw noise value
    inline float applyGrade(float v) const
    {
        const float range = _whitepoint - _blackpoint;
        if (std::fabs(range) > 1e-6f)
            v = (v - _blackpoint) / range;
        else
            v = 0.0f;

        v *= _multiply;

        if (v > 0.0f && _gamma > 0.0f && _gamma != 1.0f)
            v = std::pow(v, 1.0f / _gamma);

        v += _lift;
        return v;
    }

    void engine(int y, int x, int r, ChannelMask channels, Row& row) override
    {
        // Fetch input rows if connected
        Row inRow(x, r);
        if (input(0)) {
            input(0)->get(y, x, r, channels, inRow);
        }

        // Fetch position channels if coordinate source is 3D Position
        Row pRow(x, r);
        Channel pX = Chan_Black, pY = Chan_Black, pZ = Chan_Black;
        if (_coordSource == 1 && input(0) && _pChannels != Chan_Black) {
            input(0)->get(y, x, r, _pChannels, pRow);
            foreach(c, _pChannels) {
                if (colourIndex(c) == 0) pX = c;
                else if (colourIndex(c) == 1) pY = c;
                else if (colourIndex(c) == 2) pZ = c;
            }
        }

        // Fetch mask row if connected
        Row maskRow(x, r);
        const bool hasMask = (input(1) != nullptr);
        if (hasMask) {
            input(1)->get(y, x, r, Mask_Alpha, maskRow);
        }

        const float invScaleX = (_scale[0] != 0.0f) ? (1.0f / _scale[0]) : 1.0f;
        const float invScaleY = (_scale[1] != 0.0f) ? (1.0f / _scale[1]) : 1.0f;
        const float invScaleZ = (_scale[2] != 0.0f) ? (1.0f / _scale[2]) : 1.0f;

        // Buffers for noise results across the row span [x, r)
        const int span = r - x;
        std::vector<float> noiseR(span);
        std::vector<float> noiseG(span);
        std::vector<float> noiseB(span);

        for (int col = x; col < r; ++col) {
            const int idx = col - x;
            float px, py, pz;

            if (_coordSource == 1 && pX != Chan_Black && pY != Chan_Black) {
                // 3D Position channel source
                px = pRow[pX][col];
                py = pRow[pY][col];
                pz = (pZ != Chan_Black) ? pRow[pZ][col] : _zEvolution;
            } else {
                // 2D Screen space source
                px = static_cast<float>(col);
                py = static_cast<float>(y);
                pz = _zEvolution;
            }

            // Apply spatial offset and scale
            float nx = (px - _offset[0]) * invScaleX;
            float ny = (py - _offset[1]) * invScaleY;
            float nz = (pz - _offset[2]) * invScaleZ;

            // Optional domain warp
            if (_enableDomainWarp) {
                _fnlR.DomainWarp(nx, ny, nz);
            }

            if (_colorMode == 0) {
                // Monochrome: single noise value for all channels
                float val = _fnlR.GetNoise(nx, ny, nz);

                if (_outputRange == 0)
                    val = val * 0.5f + 0.5f;

                if (_invert)
                    val = (_outputRange == 0) ? (1.0f - val) : (-val);

                val = applyGrade(val);
                noiseR[idx] = noiseG[idx] = noiseB[idx] = val;
            } else {
                // Color mode: independent seeds for R, G, B
                float vR = _fnlR.GetNoise(nx, ny, nz);
                float vG = _fnlG.GetNoise(nx, ny, nz);
                float vB = _fnlB.GetNoise(nx, ny, nz);

                if (_outputRange == 0) {
                    vR = vR * 0.5f + 0.5f;
                    vG = vG * 0.5f + 0.5f;
                    vB = vB * 0.5f + 0.5f;
                }

                if (_invert) {
                    vR = (_outputRange == 0) ? (1.0f - vR) : (-vR);
                    vG = (_outputRange == 0) ? (1.0f - vG) : (-vG);
                    vB = (_outputRange == 0) ? (1.0f - vB) : (-vB);
                }

                noiseR[idx] = applyGrade(vR);
                noiseG[idx] = applyGrade(vG);
                noiseB[idx] = applyGrade(vB);
            }
        }

        // Write into output row channels
        foreach(z, channels) {
            float* out = row.writable(z);
            const float* in = input(0) ? inRow[z] : nullptr;

            // If channel is not targeted by output, pass through or erase
            if (!_channels.contains(z)) {
                if (in) {
                    std::copy(in + x, in + r, out + x);
                } else {
                    std::fill(out + x, out + r, 0.0f);
                }
                continue;
            }

            const int cIdx = colourIndex(z);

            for (int col = x; col < r; ++col) {
                const int idx = col - x;
                float noiseVal = 0.0f;

                if (_colorMode == 0) {
                    // Monochrome: all output channels receive the monochrome noise pattern
                    noiseVal = noiseR[idx];
                } else {
                    // Color mode: RGB get independent seeds, alpha/others get noiseR
                    if (cIdx == 0)      noiseVal = noiseR[idx];
                    else if (cIdx == 1) noiseVal = noiseG[idx];
                    else if (cIdx == 2) noiseVal = noiseB[idx];
                    else                noiseVal = noiseR[idx];
                }

                // Effective mix
                float m = _mix;
                if (hasMask) {
                    m *= maskRow[Chan_Alpha][col];
                }

                if (in && m < 0.999f) {
                    out[col] = in[col] + (noiseVal - in[col]) * m;
                } else if (in) {
                    out[col] = noiseVal;
                } else {
                    out[col] = noiseVal * m;
                }
            }
        }
    }

    static const Description d;
};

static Iop* build(Node* node) { return new FastNoiseLiteNode(node); }
const Iop::Description FastNoiseLiteNode::d(::CLASS, "Draw/FastNoiseLite", build);
