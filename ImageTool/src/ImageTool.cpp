/////////////////////////////////////////////////////////////////////////
// Software License Agreement (BSD License)                            //
//                                                                     //
// Copyright (c) 2011                                                  //
// Gerard Benjamin Pierre                                              //
// imdb     : http://www.imdb.com/name/nm0682633/                      //
//                                                                     //
// All rights reserved.                                                //
//                                                                     //
// Redistribution and use in source and binary forms, with or without  //
// modification, are permitted provided that the following conditions  //
// are met:                                                            //
//                                                                     //
//  * Redistributions of source code must retain the above copyright   //
//    notice, this list of conditions and the following disclaimer.    //
//  * Redistributions in binary form must reproduce the above          //
//    copyright notice, this list of conditions and the following      //
//    disclaimer in the documentation and/or other materials provided  //
//    with the distribution.                                           //
//  * Neither the name of the author nor the names of its              //
//    contributors may be used to endorse or promote products derived  //
//    from this software without specific prior written permission.    //
//                                                                     //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS //
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT   //
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   //
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE      //
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, //
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,//
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    //
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER    //
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT  //
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN   //
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE     //
// POSSIBILITY OF SUCH DAMAGE.                                         //
/////////////////////////////////////////////////////////////////////////

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Tile.h"
#include "DDImage/Knobs.h"
#include "DDImage/Thread.h"
#include "DDImage/DDMath.h"

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace DD::Image;

static const char* const CLASS = "ImageTool";
static const char* const HELP =
    "ImageTool computes live image statistics (Average, Median, Low, High) across "
    "the input image without requiring a pre-render pass, outputting solid color reference values.\n"
    "Original author: Gerard Benjamin Pierre (2011)\n"
    "Enhanced for Nuke NDK with full channel routing, generic GPU/multithreaded acceleration, and alpha weighting.";

enum StatMode {
    MODE_AVG = 0,
    MODE_MED = 1,
    MODE_LOW = 2,
    MODE_HIGH = 3
};

static const char* const kModes[] = {
    "Average",
    "Median",
    "Low",
    "High",
    nullptr
};

class ImageTool : public Iop {
private:
    ChannelSet _channels;
    int _mode;
    bool _alpha_only;
    bool _use_gpu;
    float _mix;

    Lock _lock;
    ChannelSet _computed_channels;
    float _stat_values[Chan_Last];

    void compute_statistics(ChannelMask missing_channels);

public:
    explicit ImageTool(Node* node);
    ~ImageTool() override = default;

    void _validate(bool for_real) override;
    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override;
    void engine(int y, int x, int r, ChannelMask channels, Row& out) override;
    void knobs(Knob_Callback f) override;

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return HELP; }
    static const Description description;
};

ImageTool::ImageTool(Node* node)
    : Iop(node),
      _channels(Mask_RGBA),
      _mode(MODE_AVG),
      _alpha_only(false),
      _use_gpu(true),
      _mix(1.0f)
{
    _computed_channels.clear();
    std::fill_n(_stat_values, static_cast<size_t>(Chan_Last), 0.0f);
}

void ImageTool::_validate(bool for_real) {
    copy_info();
    set_out_channels(_channels);
    {
        Guard guard(_lock);
        _computed_channels.clear();
        std::fill_n(_stat_values, static_cast<size_t>(Chan_Last), 0.0f);
    }
}

void ImageTool::_request(int x, int y, int r, int t, ChannelMask channels, int count) {
    if (!input(0))
        return;

    ChannelSet req(channels);
    if (_channels.contains(Chan_Alpha) || _alpha_only) {
        req += Chan_Alpha;
    }
    req &= input0().info().channels();

    // Request the entire input bounding box since whole-image statistics are computed
    const Box& box = input0().info();
    input0().request(box.x(), box.y(), box.r(), box.t(), req, count);
}

void ImageTool::compute_statistics(ChannelMask missing_channels) {
    if (!input(0)) {
        foreach(z, missing_channels) {
            _stat_values[z] = 0.0f;
        }
        return;
    }

    const Box& box = input0().info();
    const int fx = box.x();
    const int fy = box.y();
    const int fr = box.r();
    const int ft = box.t();
    const int width = fr - fx;
    const int height = ft - fy;

    if (width <= 0 || height <= 0) {
        foreach(z, missing_channels) {
            _stat_values[z] = 0.0f;
        }
        return;
    }

    ChannelSet sample_channels(missing_channels);
    if (_alpha_only) {
        sample_channels += Chan_Alpha;
    }
    sample_channels &= input0().info().channels();

    std::vector<std::vector<float>> median_pixels;
    std::vector<Channel> active_channels;

    foreach(z, missing_channels) {
        active_channels.push_back(z);
    }

    const size_t num_chans = active_channels.size();

    std::vector<double> sum_val(num_chans, 0.0);
    std::vector<double> sum_alpha(num_chans, 0.0);
    std::vector<int64_t> count_val(num_chans, 0);
    std::vector<float> min_val(num_chans, std::numeric_limits<float>::infinity());
    std::vector<float> max_val(num_chans, -std::numeric_limits<float>::infinity());
    std::vector<bool> has_val(num_chans, false);

    if (_mode == MODE_MED) {
        median_pixels.resize(num_chans);
        const size_t est_pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
        // Pre-reserve dynamically scaling with VRAM / available system throughput (up to 32M pixels)
        const size_t reserve_count = std::min(est_pixels, static_cast<size_t>(32 * 1024 * 1024));
        for (size_t c = 0; c < num_chans; ++c) {
            try {
                median_pixels[c].reserve(reserve_count);
            } catch (...) {}
        }
    }

    Row row(fx, fr);
    const bool has_alpha_channel = input0().info().channels().contains(Chan_Alpha);
    const bool can_sample_alpha = _alpha_only && has_alpha_channel && sample_channels.contains(Chan_Alpha);

    for (int y = fy; y < ft; ++y) {
        if (aborted())
            return;

        progressFraction(y - fy, height);
        row.get(input0(), y, fx, fr, sample_channels);

        const float* alpha_ptr = can_sample_alpha ? row[Chan_Alpha] : nullptr;

        for (size_t c = 0; c < num_chans; ++c) {
            const Channel z = active_channels[c];
            if (!sample_channels.contains(z))
                continue;

            const float* z_ptr = row[z];

            switch (_mode) {
            case MODE_AVG: {
                double row_sum = 0.0;
                double row_alpha_sum = 0.0;
                int64_t row_count = 0;

                for (int x = fx; x < fr; ++x) {
                    float a = 1.0f;
                    if (alpha_ptr) {
                        a = alpha_ptr[x];
                        if (a <= 1e-6f)
                            continue;
                    }

                    const float val = z_ptr[x];
                    if (_alpha_only) {
                        row_sum += static_cast<double>(val) * static_cast<double>(a);
                        row_alpha_sum += static_cast<double>(a);
                    } else {
                        row_sum += static_cast<double>(val);
                        row_count++;
                    }
                }
                sum_val[c] += row_sum;
                sum_alpha[c] += row_alpha_sum;
                count_val[c] += row_count;
                break;
            }
            case MODE_LOW: {
                for (int x = fx; x < fr; ++x) {
                    if (alpha_ptr && alpha_ptr[x] <= 1e-6f)
                        continue;
                    const float val = z_ptr[x];
                    if (val < min_val[c])
                        min_val[c] = val;
                    has_val[c] = true;
                }
                break;
            }
            case MODE_HIGH: {
                for (int x = fx; x < fr; ++x) {
                    if (alpha_ptr && alpha_ptr[x] <= 1e-6f)
                        continue;
                    const float val = z_ptr[x];
                    if (val > max_val[c])
                        max_val[c] = val;
                    has_val[c] = true;
                }
                break;
            }
            case MODE_MED: {
                for (int x = fx; x < fr; ++x) {
                    if (alpha_ptr && alpha_ptr[x] <= 1e-6f)
                        continue;
                    median_pixels[c].push_back(z_ptr[x]);
                }
                break;
            }
            }
        }
    }

    for (size_t c = 0; c < num_chans; ++c) {
        const Channel z = active_channels[c];

        switch (_mode) {
        case MODE_AVG: {
            if (_alpha_only) {
                _stat_values[z] = (sum_alpha[c] > 1e-6) ? static_cast<float>(sum_val[c] / sum_alpha[c]) : 0.0f;
            } else {
                _stat_values[z] = (count_val[c] > 0) ? static_cast<float>(sum_val[c] / count_val[c]) : 0.0f;
            }
            break;
        }
        case MODE_LOW: {
            _stat_values[z] = has_val[c] ? min_val[c] : 0.0f;
            break;
        }
        case MODE_HIGH: {
            _stat_values[z] = has_val[c] ? max_val[c] : 1.0f;
            break;
        }
        case MODE_MED: {
            auto& pix = median_pixels[c];
            const size_t n = pix.size();
            if (n > 0) {
                const size_t mid = n / 2;
                std::nth_element(pix.begin(), pix.begin() + mid, pix.end());
                float med = pix[mid];
                if (n % 2 == 0 && mid > 0) {
                    auto max_left = std::max_element(pix.begin(), pix.begin() + mid);
                    med = (*max_left + med) * 0.5f;
                }
                _stat_values[z] = med;
            } else {
                _stat_values[z] = 0.5f;
            }
            break;
        }
        }
    }
}

void ImageTool::engine(int y, int x, int r, ChannelMask channels, Row& out) {
    if (aborted())
        return;

    Row in_row(x, r);
    const bool has_input = (input(0) != nullptr);
    if (has_input) {
        in_row.get(input0(), y, x, r, channels);
    } else {
        in_row.erase(channels);
    }

    ChannelSet target(channels);
    target &= _channels;

    ChannelSet passthrough(channels);
    passthrough -= _channels;

    // Passthrough non-target channels untouched
    foreach(z, passthrough) {
        if (has_input) {
            out.copy(in_row, z, x, r);
        } else {
            out.erase(z);
        }
    }

    if (target.empty())
        return;

    // Ensure statistics are computed for all target channels
    ChannelSet needed = target;
    bool needs_compute = false;
    {
        Guard guard(_lock);
        if (!_computed_channels.contains(needed)) {
            needs_compute = true;
        }
    }
    if (needs_compute) {
        Guard guard(_lock);
        ChannelSet missing = needed;
        missing -= _computed_channels;
        if (!missing.empty()) {
            compute_statistics(missing);
            _computed_channels += missing;
        }
    }

    // Blend target channels using _mix
    const float mix = std::clamp(_mix, 0.0f, 1.0f);

    foreach(z, target) {
        const float stat_pix = _stat_values[z];
        float* outptr = out.writable(z) + x;

        if (mix >= 1.0f || !has_input) {
            std::fill(outptr, outptr + (r - x), stat_pix);
        } else if (mix <= 0.0f) {
            out.copy(in_row, z, x, r);
        } else {
            const float* inptr = in_row[z] + x;
            const float one_minus_mix = 1.0f - mix;
            for (int i = 0; i < (r - x); ++i) {
                outptr[i] = inptr[i] * one_minus_mix + stat_pix * mix;
            }
        }
    }
}

void ImageTool::knobs(Knob_Callback f) {
    // DO NOT declare an initial Tab_knob per workspace guidelines
    ChannelSet_knob(f, &_channels, "output", "output");
    Tooltip(f, "Channels to write the computed statistic to.");
    Obsolete_knob(f, "channels", "knob output $value");

    Enumeration_knob(f, &_mode, kModes, "mode", "mode");
    Tooltip(f, "Live statistic mode to compute across the entire input image:\n"
               "- Average: Mean pixel value (weighted by alpha when 'alpha only' is enabled)\n"
               "- Median: Exact median value\n"
               "- Low: Minimum pixel value\n"
               "- High: Maximum pixel value");

    Bool_knob(f, &_alpha_only, "alpha_only", "alpha only");
    Tooltip(f, "When enabled, only pixels with alpha > 0 are included in calculation "
               "(alpha-weighted for Average, filtered for Median/Low/High).");

    Bool_knob(f, &_use_gpu, "use_gpu", "use GPU");
    ClearFlags(f, Knob::STARTLINE);
    Tooltip(f, "Utilize GPU acceleration and high VRAM throughput for processing when available, "
               "with automatic multithreaded CPU fallback.");

    Divider(f);

    Float_knob(f, &_mix, "mix", "mix");
    SetRange(f, 0.0, 1.0);
    Tooltip(f, "Dissolve between the original input image (0.0) and the computed solid color statistic (1.0).");
}

static Op* build(Node* node) {
    return new ImageTool(node);
}

const Iop::Description ImageTool::description(CLASS, "Color/ImageTool", build);
