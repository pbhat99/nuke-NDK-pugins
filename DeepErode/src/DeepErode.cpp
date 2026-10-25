// SPDX-License-Identifier: MIT
//
// ============================================================================
//
//  DeepErode — 3D & Screen-Space Normal-Weighted Deep Erode / Dilate for Nuke
//
//  - Screen Space: Normal-weighted extrusion / erosion. Points facing the camera
//    move 0%, points facing away from the camera erode 100%. Z depth is 100%
//    UNTOUCHED.
//  - 3D Space: Full extrusion / contraction in ALL directions (X, Y, and Z).
//    Extrudes depth towards/away from camera along viewing rays.
//
//  Features:
//  - Camera Required: Throws an error if no Camera node is connected.
//  - Softness: Edge feathering control (0 = crisp/hard, 1 = maximum soft).
//  - Mix: Dissolve slider between unprocessed input and eroded result.
//  - Background Infill: Automatically synthesizes missing background deep samples
//    from neighboring background layers when inward erosion occurs, eliminating holes.
//
// ============================================================================

#include "DDImage/DeepFilterOp.h"
#include "DDImage/DeepPixel.h"
#include "DDImage/DeepPlane.h"
#include "DDImage/Knobs.h"
#include "DDImage/CameraOp.h"
#include "DDImage/Box.h"
#include "DDImage/Vector3.h"
#include "DDImage/ChannelSet.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <cfloat>

using namespace DD::Image;

static const char* const CLASS = "DeepErode";
static const char* const HELP =
    "DeepErode: Deep Erode / Dilate Node.\n\n"
    "Modes:\n"
    "- Screen Space: Extrusion and erosion occur according to surface normals\n"
    "  (points facing camera move 0%, points facing away erode 100%). Z depth is UNTOUCHED.\n"
    "- 3D Space: Expands/contracts in ALL directions (X, Y, and Z).\n"
    "  Extrudes depth towards/away from camera along viewing rays.\n\n"
    "Controls:\n"
    "- size: Positive = Inflate / Fat (px), Negative = Deflate / Thinner (px).\n"
    "- softness: Edge feathering (0 = hard/sharp edge, 1 = maximum feathering).\n"
    "- depth_extrude: Active in 3D Space mode only (shifts Z along viewing ray).\n"
    "- fill_background: Automatically synthesizes missing background deep samples\n"
    "  from neighboring background layers when inward erosion occurs, eliminating holes.\n"
    "- mix: Dissolve between unprocessed input (0) and fully eroded output (1).\n"
    "- cam input: REQUIRED. Connect a CameraOp to provide camera optics and projection.";

enum ErodeMode {
    MODE_SCREENSPACE = 0,
    MODE_3DSPACE = 1
};

static const char* const modeNames[] = {
    "Screen Space",
    "3D Space",
    nullptr
};

struct SampleRec {
    int   originalIndex = -1; // -1 if newly synthesized / dilated
    float zFront = 0.0f;
    float zBack = 0.0f;
    float alpha = 0.0f;
    std::vector<float> channels;
};

struct SampleCluster {
    float minZ;
    float maxZ;
    std::vector<SampleRec> samples;
};

class DeepErode : public DeepFilterOp
{
    float _size;
    float _softness;
    int   _mode;
    float _depthExtrude;
    bool  _fillBackground;
    float _bgLayerGap;
    float _mix;

    // Camera parameters (extracted from connected CameraOp)
    float _focalLength;
    float _haperture;
    float _vaperture;
    int   _actualRadius;

public:
    DeepErode(Node* node) : DeepFilterOp(node),
        _size(0.0f),
        _softness(0.5f),
        _mode(MODE_SCREENSPACE),
        _depthExtrude(0.5f),
        _fillBackground(true),
        _bgLayerGap(1.0f),
        _mix(1.0f),
        _focalLength(50.0f),
        _haperture(36.0f),
        _vaperture(24.0f),
        _actualRadius(0)
    {
        inputs(2);
    }

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }
    Op* op() override { return this; }

    int minimum_inputs() const override { return 2; } // Camera input is REQUIRED
    int maximum_inputs() const override { return 2; }

    bool test_input(int n, Op* op) const override
    {
        if (n == 0)
            return DeepFilterOp::test_input(n, op);
        if (n == 1)
            return dynamic_cast<CameraOp*>(op) != nullptr;
        return false;
    }

    Op* default_input(int input) const override
    {
        if (input == 0)
            return DeepFilterOp::default_input(input);
        return nullptr;
    }

    const char* input_label(int input, char* /*buffer*/) const override
    {
        switch (input) {
            case 0: return "deep";
            case 1: return "cam";
            default: return "";
        }
    }

    void knobs(Knob_Callback f) override
    {
        Float_knob(f, &_size, "size", "size / extrude");
        SetRange(f, -30.0f, 30.0f);
        Tooltip(f, "Extrusion amount in pixels. Positive = Inflate / Fat, Negative = Deflate / Thinner.");

        Float_knob(f, &_softness, "softness", "softness");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Edge softness for extruded or eroded edges. 0 = hard/sharp edge, 1 = maximum feathering.");

        Enumeration_knob(f, &_mode, modeNames, "mode", "mode");
        Tooltip(f, "Screen Space: Normal-weighted X/Y (facing camera = 0% movement, facing away = 100% erode). Z depth is untouched.\n"
                   "3D Space: Expands/contracts in all directions (X, Y, and Z).");

        Float_knob(f, &_depthExtrude, "depth_extrude", "depth extrusion (3D)");
        SetRange(f, 0.0f, 2.0f);
        Tooltip(f, "3D Space mode only: Amount by which depth front/back shifts along viewing ray.\n"
                   "In Screen Space mode, depth is untouched.");

        Divider(f, "Inward Infill");
        Bool_knob(f, &_fillBackground, "fill_background", "fill background layer");
        Tooltip(f, "When deflating (size < 0), fills in the missing background deep layer behind eroded edges.");

        Float_knob(f, &_bgLayerGap, "bg_layer_gap", "layer depth gap");
        SetRange(f, 0.1f, 20.0f);
        Tooltip(f, "Depth separation threshold used to classify discrete layers.");

        Divider(f, "");
        Float_knob(f, &_mix, "mix", "mix");
        SetRange(f, 0.0f, 1.0f);
        Tooltip(f, "Dissolve between the unprocessed input (0) and the fully eroded/dilated result (1).");
    }

    int knob_changed(Knob* k) override
    {
        if (k->is("mode")) {
            Knob* deKnob = knob("depth_extrude");
            if (deKnob)
                deKnob->enable(_mode == MODE_3DSPACE);
            return 1;
        }
        return DeepFilterOp::knob_changed(k);
    }

    void _validate(bool for_real) override
    {
        // Enforce CameraOp connection: throw error if missing
        CameraOp* cam = dynamic_cast<CameraOp*>(Op::input(1));
        if (!cam) {
            error("No camera connected to 'cam' input! A Camera node is required.");
            return;
        }

        cam->validate(for_real);
        _focalLength = cam->focal_length();
        _haperture   = cam->film_width();
        _vaperture   = cam->film_height();

        if (_focalLength <= 0.001f) _focalLength = 50.0f;
        if (_haperture <= 0.001f)   _haperture   = 36.0f;
        if (_vaperture <= 0.001f)   _vaperture   = 24.0f;

        DeepFilterOp::_validate(for_real);

        const float absSize = std::fabs(_size);
        if (absSize < 0.001f || _mix <= 0.0001f) {
            _actualRadius = 0;
            return;
        }

        _actualRadius = std::min(100, std::max(1, static_cast<int>(std::ceil(absSize))));

        // For outward inflation, expand output bounding box
        if (_actualRadius > 0 && _size > 0.0f) {
            Box box = _deepInfo.box();
            _deepInfo.box().set(box.x() - _actualRadius,
                                box.y() - _actualRadius,
                                box.r() + _actualRadius,
                                box.t() + _actualRadius);
        }
    }

    void getDeepRequests(Box box, const ChannelSet& channels, int count,
                         std::vector<RequestData>& requests) override
    {
        if (!input0())
            return;

        const int rad = _actualRadius;
        Box padded(box.x() - rad,
                   box.y() - rad,
                   box.r() + rad,
                   box.t() + rad);

        ChannelSet reqChannels = channels;
        reqChannels += Chan_DeepFront;
        reqChannels += Chan_DeepBack;
        reqChannels += Chan_Alpha;

        requests.push_back(RequestData(input0(), padded, reqChannels, count));
    }

    // Compute dilation falloff weight with softness (0 = hard edge, 1 = soft feather)
    inline float computeDilationWeight(float dist, float radius) const
    {
        if (radius <= 0.0f || dist > radius)
            return 0.0f;

        const float s = std::min(1.0f, std::max(0.0f, _softness));
        if (s <= 0.001f)
            return 1.0f; // Hard / crisp edge

        const float innerR = radius * (1.0f - s);
        if (dist <= innerR)
            return 1.0f;

        const float u = (radius - dist) / (radius - innerR);
        return u * u * (3.0f - 2.0f * u); // Smoothstep
    }

    // Compute erosion factor with softness (0 = hard cutoff, 1 = soft feather)
    inline float computeErosionFactor(float minEdgeDist, float radius) const
    {
        if (radius <= 0.0f)
            return 1.0f;

        const float s = std::min(1.0f, std::max(0.0f, _softness));
        if (s <= 0.001f)
            return (minEdgeDist >= radius) ? 1.0f : 0.0f; // Sharp / hard boundary

        const float innerR = radius * (1.0f - s);
        if (minEdgeDist >= radius)
            return 1.0f; // Deep interior
        if (minEdgeDist <= innerR)
            return 0.0f; // Outer feather boundary

        const float u = (minEdgeDist - innerR) / (radius - innerR);
        return u * u * (3.0f - 2.0f * u); // Smoothstep
    }

    // Unproject pixel to 3D point in camera space
    inline Vector3 camPoint(int px, int py, float Z, int imgW, int imgH) const
    {
        const float u = (static_cast<float>(px) + 0.5f) / static_cast<float>(imgW) * 2.0f - 1.0f;
        const float v = (static_cast<float>(py) + 0.5f) / static_cast<float>(imgH) * 2.0f - 1.0f;
        const float x = (_haperture * 0.5f / _focalLength) * u * Z;
        const float y = (_vaperture * 0.5f / _focalLength) * v * Z;
        const float z = -Z;
        return Vector3(x, y, z);
    }

    // Compute normal weight: 0.0 = facing camera (0% erode/move), 1.0 = facing away (100% erode)
    inline float computeNormalWeight(int outX, int outY, float Z, const DeepPlane& inPlane, int imgW, int imgH) const
    {
        const Box& inBox = inPlane.box();
        const Vector3 P = camPoint(outX, outY, Z, imgW, imgH);
        Vector3 V = -P; // Viewing vector towards camera
        const float vLen = V.length();
        if (vLen > 1e-6f)
            V /= vLen;
        else
            V.set(0.0f, 0.0f, 1.0f);

        // Sample neighbors to compute tangent vectors
        Vector3 Tx(1.0f, 0.0f, 0.0f);
        Vector3 Ty(0.0f, 1.0f, 0.0f);

        bool hasRight = false;
        bool hasLeft = false;
        float zRight = Z, zLeft = Z;

        if (outX + 1 < inBox.r()) {
            DeepPixel rPx = inPlane.getPixel(outY, outX + 1);
            for (size_t s = 0; s < rPx.getSampleCount(); ++s) {
                float zf = rPx.getUnorderedSample(s, Chan_DeepFront);
                if (std::fabs(zf - Z) <= _bgLayerGap) {
                    zRight = zf;
                    hasRight = true;
                    break;
                }
            }
        }

        if (outX - 1 >= inBox.x()) {
            DeepPixel lPx = inPlane.getPixel(outY, outX - 1);
            for (size_t s = 0; s < lPx.getSampleCount(); ++s) {
                float zf = lPx.getUnorderedSample(s, Chan_DeepFront);
                if (std::fabs(zf - Z) <= _bgLayerGap) {
                    zLeft = zf;
                    hasLeft = true;
                    break;
                }
            }
        }

        if (hasRight && hasLeft) {
            Tx = camPoint(outX + 1, outY, zRight, imgW, imgH) - camPoint(outX - 1, outY, zLeft, imgW, imgH);
        } else if (hasRight) {
            Tx = (camPoint(outX + 1, outY, zRight, imgW, imgH) - P) * 2.0f;
        } else if (hasLeft) {
            Tx = (P - camPoint(outX - 1, outY, zLeft, imgW, imgH)) * 2.0f;
        }

        bool hasTop = false;
        bool hasBottom = false;
        float zTop = Z, zBottom = Z;

        if (outY + 1 < inBox.t()) {
            DeepPixel tPx = inPlane.getPixel(outY + 1, outX);
            for (size_t s = 0; s < tPx.getSampleCount(); ++s) {
                float zf = tPx.getUnorderedSample(s, Chan_DeepFront);
                if (std::fabs(zf - Z) <= _bgLayerGap) {
                    zTop = zf;
                    hasTop = true;
                    break;
                }
            }
        }

        if (outY - 1 >= inBox.y()) {
            DeepPixel bPx = inPlane.getPixel(outY - 1, outX);
            for (size_t s = 0; s < bPx.getSampleCount(); ++s) {
                float zf = bPx.getUnorderedSample(s, Chan_DeepFront);
                if (std::fabs(zf - Z) <= _bgLayerGap) {
                    zBottom = zf;
                    hasBottom = true;
                    break;
                }
            }
        }

        if (hasTop && hasBottom) {
            Ty = camPoint(outX, outY + 1, zTop, imgW, imgH) - camPoint(outX, outY - 1, zBottom, imgW, imgH);
        } else if (hasTop) {
            Ty = (camPoint(outX, outY + 1, zTop, imgW, imgH) - P) * 2.0f;
        } else if (hasBottom) {
            Ty = (P - camPoint(outX, outY - 1, zBottom, imgW, imgH)) * 2.0f;
        }

        Vector3 N = Tx.cross(Ty);
        const float nLen = N.length();
        if (nLen > 1e-6f) {
            N /= nLen;
            if (N.dot(V) < 0.0f)
                N = -N;
        } else {
            N = V;
        }

        // Dot product with viewing ray: 1.0 = facing camera, 0.0 = facing away / grazing silhouette
        float facing = std::max(0.0f, std::min(1.0f, N.dot(V)));

        // Silhouette boundaries curve away from the camera
        if (!hasRight || !hasLeft || !hasTop || !hasBottom) {
            facing *= 0.5f;
        }

        // Normal weight: 0.0 facing camera (0% move), 1.0 facing away (100% erode)
        return std::max(0.0f, std::min(1.0f, 1.0f - facing));
    }

    // Cluster sorted samples into discrete depth layers
    static std::vector<SampleCluster> clusterLayers(const std::vector<SampleRec>& samples, float gap)
    {
        std::vector<SampleCluster> clusters;
        if (samples.empty())
            return clusters;

        SampleCluster cur;
        cur.minZ = samples[0].zFront;
        cur.maxZ = samples[0].zBack;
        cur.samples.push_back(samples[0]);

        for (size_t i = 1; i < samples.size(); ++i) {
            if (samples[i].zFront <= cur.maxZ + gap) {
                cur.maxZ = std::max(cur.maxZ, samples[i].zBack);
                cur.samples.push_back(samples[i]);
            } else {
                clusters.push_back(std::move(cur));
                cur = SampleCluster();
                cur.minZ = samples[i].zFront;
                cur.maxZ = samples[i].zBack;
                cur.samples.push_back(samples[i]);
            }
        }
        clusters.push_back(std::move(cur));
        return clusters;
    }

    bool doDeepEngine(Box box, const ChannelSet& channels, DeepOutputPlane& plane) override
    {
        DeepOp* in = input0();
        if (!in)
            return true;

        const float absSize = std::fabs(_size);
        const int rad = _actualRadius;

        // Pass through if zero size or mix == 0
        if (absSize < 0.001f || rad == 0 || _mix <= 0.0001f) {
            DeepPlane inPlane;
            if (!in->deepEngine(box, channels, inPlane))
                return false;
            plane = DeepOutputPlane(channels, box, DeepPixel::eUnordered);
            for (Box::iterator it = box.begin(); it != box.end(); ++it) {
                DeepPixel px = inPlane.getPixel(it);
                const int nSamples = static_cast<int>(px.getSampleCount());
                if (nSamples == 0) {
                    plane.addHole();
                    continue;
                }
                DeepOutPixel outPixel;
                outPixel.reserve(nSamples * channels.size());
                for (int s = 0; s < nSamples; ++s) {
                    foreach(z, channels) {
                        outPixel.push_back(px.getUnorderedSample(s, z));
                    }
                }
                plane.addPixel(outPixel);
            }
            return true;
        }

        // Fetch padded input region
        Box inputBox(box.x() - rad,
                     box.y() - rad,
                     box.r() + rad,
                     box.t() + rad);

        DeepPlane inPlane;
        if (!in->deepEngine(inputBox, channels, inPlane))
            return false;

        const int nChans = channels.size();
        std::vector<bool> isDepthChan(nChans, false);
        std::vector<bool> isAlphaChan(nChans, false);
        {
            int ci = 0;
            foreach(z, channels) {
                if (z == Chan_DeepFront || z == Chan_DeepBack)
                    isDepthChan[ci] = true;
                if (z == Chan_Alpha)
                    isAlphaChan[ci] = true;
                ci++;
            }
        }

        plane = DeepOutputPlane(channels, box, DeepPixel::eUnordered);
        const Box& inBox = inPlane.box();
        const int imgW = _deepInfo.box().w() > 0 ? _deepInfo.box().w() : 2048;
        const int imgH = _deepInfo.box().h() > 0 ? _deepInfo.box().h() : 1556;

        // In Screen Space: Z is UNTOUCHED (pushZ = false), X/Y is normal-weighted
        // In 3D Space: Z pushes towards/away from camera (pushZ = true)
        const bool pushZ = (_mode == MODE_3DSPACE);

        DeepOutPixel outPixel;
        std::vector<SampleRec> outputSamples;
        std::vector<SampleRec> curSamples;

        for (Box::iterator it = box.begin(); it != box.end(); ++it) {
            if (Op::aborted())
                return false;

            const int outX = it.x;
            const int outY = it.y;

            outputSamples.clear();
            curSamples.clear();

            // Read current pixel samples
            DeepPixel curPx = inPlane.getPixel(outY, outX);
            const int curSampleCount = static_cast<int>(curPx.getSampleCount());

            for (int s = 0; s < curSampleCount; ++s) {
                SampleRec rec;
                rec.originalIndex = s;
                rec.zFront = curPx.getUnorderedSample(s, Chan_DeepFront);
                rec.zBack  = curPx.getUnorderedSample(s, Chan_DeepBack);
                rec.alpha  = curPx.getUnorderedSample(s, Chan_Alpha);
                rec.channels.resize(nChans);
                int ci = 0;
                foreach(z, channels) {
                    rec.channels[ci] = curPx.getUnorderedSample(s, z);
                    ci++;
                }
                curSamples.push_back(std::move(rec));
            }

            // Sort current samples by zFront
            std::sort(curSamples.begin(), curSamples.end(),
                      [](const SampleRec& a, const SampleRec& b) {
                          return a.zFront < b.zFront;
                      });

            // Group current samples into discrete depth layers
            std::vector<SampleCluster> curLayers = clusterLayers(curSamples, _bgLayerGap);

            // ================================================================
            // CASE 1: INFLATE / OUTWARD EXTRUSION (size > 0)
            // ================================================================
            if (_size > 0.0f) {
                // 1. Process current pixel's existing layers
                for (const auto& layer : curLayers) {
                    for (const auto& s : layer.samples) {
                        SampleRec rec = s;
                        if (pushZ) {
                            // 3D Space: Extrude Z towards camera
                            rec.zFront = std::max(0.001f, s.zFront - _depthExtrude * _size * 0.1f);
                            rec.zBack  = std::max(rec.zFront + 0.001f, s.zBack + _depthExtrude * _size * 0.05f);
                            for (size_t ci = 0; ci < rec.channels.size(); ++ci) {
                                if (isDepthChan[ci])
                                    rec.channels[ci] = (ci == 0) ? rec.zFront : rec.zBack;
                            }
                        }
                        // Screen Space: Z screenspace is 100% UNTOUCHED
                        outputSamples.push_back(rec);
                    }
                }

                // 2. Search neighbors for dilation into this pixel
                for (int dy = -rad; dy <= rad; ++dy) {
                    const int ny = outY + dy;
                    if (ny < inBox.y() || ny >= inBox.t())
                        continue;

                    for (int dx = -rad; dx <= rad; ++dx) {
                        if (dx == 0 && dy == 0)
                            continue; // Center pixel already handled

                        const int nx = outX + dx;
                        if (nx < inBox.x() || nx >= inBox.r())
                            continue;

                        DeepPixel nbPx = inPlane.getPixel(ny, nx);
                        const int nbCount = static_cast<int>(nbPx.getSampleCount());
                        if (nbCount == 0)
                            continue;

                        const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                        if (dist > static_cast<float>(rad))
                            continue;

                        const float weight = computeDilationWeight(dist, static_cast<float>(rad));
                        if (weight <= 0.001f)
                            continue;

                        // Read and cluster neighbor samples
                        std::vector<SampleRec> nbSamples;
                        for (int s = 0; s < nbCount; ++s) {
                            SampleRec rec;
                            rec.originalIndex = -1; // Dilated from neighbor
                            rec.zFront = nbPx.getUnorderedSample(s, Chan_DeepFront);
                            rec.zBack  = nbPx.getUnorderedSample(s, Chan_DeepBack);
                            rec.alpha  = nbPx.getUnorderedSample(s, Chan_Alpha);
                            rec.channels.resize(nChans);
                            int ci = 0;
                            foreach(z, channels) {
                                rec.channels[ci] = nbPx.getUnorderedSample(s, z);
                                ci++;
                            }
                            nbSamples.push_back(std::move(rec));
                        }

                        std::sort(nbSamples.begin(), nbSamples.end(),
                                  [](const SampleRec& a, const SampleRec& b) {
                                      return a.zFront < b.zFront;
                                  });

                        std::vector<SampleCluster> nbLayers = clusterLayers(nbSamples, _bgLayerGap);

                        for (const auto& nbLayer : nbLayers) {
                            // Check if current pixel already has a layer matching this depth
                            bool alreadyHasLayer = false;
                            for (const auto& curL : curLayers) {
                                if (std::fabs(curL.minZ - nbLayer.minZ) <= _bgLayerGap) {
                                    alreadyHasLayer = true;
                                    break;
                                }
                            }

                            // If not already present, expand neighbor layer into current pixel
                            if (!alreadyHasLayer) {
                                // Normal-based weighting in Screen Space:
                                // Facing camera moves 0%, facing away moves 100%
                                float normWeight = 1.0f;
                                if (!pushZ) {
                                    normWeight = computeNormalWeight(nx, ny, nbLayer.minZ, inPlane, imgW, imgH);
                                }

                                const float finalWeight = weight * normWeight;
                                if (finalWeight <= 0.001f)
                                    continue;

                                for (const auto& nbSample : nbLayer.samples) {
                                    SampleRec dilated;
                                    dilated.originalIndex = -1;

                                    if (pushZ) {
                                        // 3D Space: Extrude Z towards camera
                                        dilated.zFront = std::max(0.001f, nbSample.zFront - _depthExtrude * _size * 0.1f * finalWeight);
                                        dilated.zBack  = std::max(dilated.zFront + 0.001f, nbSample.zBack + _depthExtrude * _size * 0.05f * finalWeight);
                                    } else {
                                        // Screen Space: Z depth is 100% UNTOUCHED
                                        dilated.zFront = nbSample.zFront;
                                        dilated.zBack  = nbSample.zBack;
                                    }

                                    dilated.alpha = nbSample.alpha * finalWeight;
                                    dilated.channels.resize(nChans);
                                    int ci = 0;
                                    foreach(z, channels) {
                                        if (isDepthChan[ci])
                                            dilated.channels[ci] = (z == Chan_DeepFront) ? dilated.zFront : dilated.zBack;
                                        else if (isAlphaChan[ci])
                                            dilated.channels[ci] = dilated.alpha;
                                        else
                                            dilated.channels[ci] = nbSample.channels[ci] * finalWeight;
                                        ci++;
                                    }
                                    outputSamples.push_back(dilated);
                                }
                            }
                        }
                    }
                }
            }
            // ================================================================
            // CASE 2: DEFLATE / INWARD EROSION (size < 0)
            // ================================================================
            else {
                if (!curLayers.empty()) {
                    for (size_t lIdx = 0; lIdx < curLayers.size(); ++lIdx) {
                        const auto& layer = curLayers[lIdx];
                        const float layerZ = layer.minZ;

                        // Find distance to the nearest boundary of this layer
                        float minEdgeDist = FLT_MAX;
                        for (int dy = -rad; dy <= rad; ++dy) {
                            const int ny = outY + dy;
                            for (int dx = -rad; dx <= rad; ++dx) {
                                const int nx = outX + dx;
                                bool isOutside = false;

                                if (ny < inBox.y() || ny >= inBox.t() || nx < inBox.x() || nx >= inBox.r()) {
                                    isOutside = true;
                                } else {
                                    DeepPixel testPx = inPlane.getPixel(ny, nx);
                                    const size_t testCount = testPx.getSampleCount();
                                    if (testCount == 0) {
                                        isOutside = true;
                                    } else {
                                        bool neighborHasLayer = false;
                                        for (size_t s = 0; s < testCount; ++s) {
                                            float zf = testPx.getUnorderedSample(s, Chan_DeepFront);
                                            if (std::fabs(zf - layerZ) <= _bgLayerGap) {
                                                neighborHasLayer = true;
                                                break;
                                            }
                                        }
                                        if (!neighborHasLayer)
                                            isOutside = true;
                                    }
                                }

                                if (isOutside) {
                                    const float d = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                                    minEdgeDist = std::min(minEdgeDist, d);
                                }
                            }
                        }

                        // Compute raw geometric erosion factor from distance to edge (0.0 = eroded, 1.0 = solid)
                        const float baseErodeFactor = computeErosionFactor(minEdgeDist, static_cast<float>(rad));

                        // In Screen Space: Normal-weighted erosion
                        // Points facing camera (normWeight = 0.0) move 0% (erodeFactor = 1.0, untouched)
                        // Points facing away (normWeight = 1.0) erode 100% (erodeFactor = baseErodeFactor)
                        float normWeight = 1.0f;
                        if (!pushZ) {
                            normWeight = computeNormalWeight(outX, outY, layerZ, inPlane, imgW, imgH);
                        }

                        const float erodeFactor = 1.0f - (1.0f - baseErodeFactor) * normWeight;

                        // If layer survives, add it
                        if (erodeFactor > 0.005f) {
                            for (const auto& s : layer.samples) {
                                SampleRec rec = s;
                                rec.alpha *= erodeFactor;

                                if (pushZ) {
                                    // 3D Space: Push Z away from camera on deflate
                                    rec.zFront += _depthExtrude * absSize * (1.0f - erodeFactor) * 0.1f;
                                    rec.zBack  += _depthExtrude * absSize * (1.0f - erodeFactor) * 0.1f;
                                }
                                // Screen Space: Z screenspace is 100% UNTOUCHED

                                for (size_t ci = 0; ci < rec.channels.size(); ++ci) {
                                    if (isDepthChan[ci])
                                        rec.channels[ci] = (ci == 0) ? rec.zFront : rec.zBack;
                                    else
                                        rec.channels[ci] *= erodeFactor;
                                }
                                outputSamples.push_back(rec);
                            }
                        }

                        // BACKGROUND INFILL:
                        // If this layer was eroded (erodeFactor < 0.98f) and there is no deeper
                        // background layer in this pixel behind it:
                        const bool hasDeeperLayer = (lIdx + 1 < curLayers.size());
                        if (_fillBackground && !hasDeeperLayer && erodeFactor < 0.98f) {
                            float bestBgDist = FLT_MAX;
                            SampleRec synthesizedBg;
                            bool foundBg = false;

                            for (int dy = -rad; dy <= rad; ++dy) {
                                const int ny = outY + dy;
                                if (ny < inBox.y() || ny >= inBox.t())
                                    continue;

                                for (int dx = -rad; dx <= rad; ++dx) {
                                    const int nx = outX + dx;
                                    if (nx < inBox.x() || nx >= inBox.r())
                                        continue;

                                    DeepPixel nbPx = inPlane.getPixel(ny, nx);
                                    const int nbCount = static_cast<int>(nbPx.getSampleCount());
                                    for (int s = 0; s < nbCount; ++s) {
                                        float zf = nbPx.getUnorderedSample(s, Chan_DeepFront);
                                        // Valid background sample deeper than the eroded layer
                                        if (zf > layer.maxZ + _bgLayerGap) {
                                            const float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                                            if (dist < bestBgDist) {
                                                bestBgDist = dist;
                                                foundBg = true;

                                                // Background Z is ALWAYS untouched
                                                synthesizedBg.originalIndex = -1;
                                                synthesizedBg.zFront = zf;
                                                synthesizedBg.zBack  = nbPx.getUnorderedSample(s, Chan_DeepBack);
                                                const float fillAlpha = (1.0f - erodeFactor) * nbPx.getUnorderedSample(s, Chan_Alpha);
                                                synthesizedBg.alpha = fillAlpha;

                                                synthesizedBg.channels.resize(nChans);
                                                int ci = 0;
                                                foreach(z, channels) {
                                                    if (isDepthChan[ci])
                                                        synthesizedBg.channels[ci] = (z == Chan_DeepFront) ? synthesizedBg.zFront : synthesizedBg.zBack;
                                                    else if (isAlphaChan[ci])
                                                        synthesizedBg.channels[ci] = fillAlpha;
                                                    else
                                                        synthesizedBg.channels[ci] = nbPx.getUnorderedSample(s, z) * (1.0f - erodeFactor);
                                                    ci++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (foundBg) {
                                outputSamples.push_back(synthesizedBg);
                            }
                        }
                    }
                }
            }

            // ================================================================
            // MIX DISSOLVE (0.0 = original input, 1.0 = eroded output)
            // ================================================================
            if (_mix < 0.999f) {
                const float m = std::max(0.0f, std::min(1.0f, _mix));
                std::vector<bool> origVisited(curSamples.size(), false);

                // Blend surviving/dilated samples
                for (auto& sr : outputSamples) {
                    if (sr.originalIndex >= 0 && sr.originalIndex < static_cast<int>(curSamples.size())) {
                        const auto& orig = curSamples[sr.originalIndex];
                        origVisited[sr.originalIndex] = true;
                        sr.zFront = orig.zFront + (sr.zFront - orig.zFront) * m;
                        sr.zBack  = orig.zBack  + (sr.zBack  - orig.zBack)  * m;
                        sr.alpha  = orig.alpha  + (sr.alpha  - orig.alpha)  * m;
                        for (size_t ci = 0; ci < sr.channels.size(); ++ci) {
                            if (!isDepthChan[ci])
                                sr.channels[ci] = orig.channels[ci] + (sr.channels[ci] - orig.channels[ci]) * m;
                            else
                                sr.channels[ci] = (ci == 0) ? sr.zFront : sr.zBack;
                        }
                    } else {
                        // Newly dilated or infilled sample fades in with mix
                        sr.alpha *= m;
                        for (size_t ci = 0; ci < sr.channels.size(); ++ci) {
                            if (!isDepthChan[ci])
                                sr.channels[ci] *= m;
                        }
                    }
                }

                // Add back any original samples that were completely eroded away, weighted by (1 - m)
                for (size_t i = 0; i < curSamples.size(); ++i) {
                    if (!origVisited[i]) {
                        SampleRec faded = curSamples[i];
                        faded.alpha *= (1.0f - m);
                        for (size_t ci = 0; ci < faded.channels.size(); ++ci) {
                            if (!isDepthChan[ci])
                                faded.channels[ci] *= (1.0f - m);
                        }
                        if (faded.alpha > 0.001f)
                            outputSamples.push_back(faded);
                    }
                }
            }

            // Output plane packing
            if (outputSamples.empty()) {
                plane.addHole();
                continue;
            }

            // Sort samples by zFront
            std::sort(outputSamples.begin(), outputSamples.end(),
                      [](const SampleRec& a, const SampleRec& b) {
                          return a.zFront < b.zFront;
                      });

            outPixel.clear();
            outPixel.reserve(static_cast<int>(outputSamples.size()) * nChans);

            for (const auto& sr : outputSamples) {
                int ci = 0;
                foreach(z, channels) {
                    if (z == Chan_DeepFront)
                        outPixel.push_back(sr.zFront);
                    else if (z == Chan_DeepBack)
                        outPixel.push_back(sr.zBack);
                    else
                        outPixel.push_back(sr.channels[ci]);
                    ci++;
                }
            }

            plane.addPixel(outPixel);
        }

        return true;
    }

    static const Op::Description d;
};

static Op* build(Node* node) { return new DeepErode(node); }
const Op::Description DeepErode::d(::CLASS, "Deep/DeepErode", build);
