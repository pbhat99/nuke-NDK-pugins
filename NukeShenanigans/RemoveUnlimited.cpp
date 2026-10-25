//
// Originally Created by Hannes Sap on 28/03/2026.
// wildcard remove by arunvfx (https://github.com/arunvfx/)
// combined and modified by Prasannakumar T Bhat
//
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"

using namespace DD::Image;

static const char* const CLASS = "RemoveUnlimited";
static const char* const HELP = "Remove unlimited amount of channels with wildcard support";

static inline std::string trimString(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static inline bool isWildcardMatch(const std::string& channelName, const std::string& pattern) {
    int channelNameIndex = 0, patternIndex = 0, lastWildcardIndex = -1, channelNameBacktrackIndex = -1, nextWildcardIndex = -1;
    const int cLen = static_cast<int>(channelName.size());
    const int pLen = static_cast<int>(pattern.size());

    while (channelNameIndex < cLen) {
        if (patternIndex < pLen && (pattern[patternIndex] == '?' ||
            std::tolower(static_cast<unsigned char>(pattern[patternIndex])) ==
            std::tolower(static_cast<unsigned char>(channelName[channelNameIndex])))) {
            ++channelNameIndex;
            ++patternIndex;
        }
        else if (patternIndex < pLen && pattern[patternIndex] == '*') {
            lastWildcardIndex = patternIndex;
            nextWildcardIndex = ++patternIndex;
            channelNameBacktrackIndex = channelNameIndex;
        }
        else if (lastWildcardIndex == -1) {
            return false;
        }
        else {
            patternIndex = nextWildcardIndex;
            channelNameIndex = ++channelNameBacktrackIndex;
        }
    }
    for (int i = patternIndex; i < pLen; i++) {
        if (pattern[i] != '*') return false;
    }
    return true;
}

static inline std::vector<std::string> parseWildcardPatterns(const std::string& entry) {
    std::vector<std::string> patterns;
    std::stringstream ss(entry);
    std::string token;
    while (std::getline(ss, token, ',')) {
        std::string t = trimString(token);
        if (t.empty()) continue;
        // If no dot and doesn't end with wildcard, match all channels in the layer
        if (t.find('.') == std::string::npos && t.back() != '*') {
            t.append(".*");
        }
        patterns.push_back(t);
    }
    return patterns;
}

class RemoveUnlimited : public NoIop {
    int operation_; // 0 = remove, 1 = keep
    int channels_amount_;
    bool use_wildcard_;
    std::string wildcard_entry_;

    int knobs_amount_; // number of dynamic channel knobs installed

    std::vector<ChannelSet> dynamic_channels_;
    std::vector<std::string> channel_names_;

    void save_current_channel_names() {
        for (int i = 0; ; ++i) {
            std::string knob_name = "channels" + std::to_string(i + 1);
            const Knob* k = knob(knob_name.c_str());
            if (!k) break;
            std::stringstream ss;
            k->to_script(ss, nullptr, false);
            std::string val = ss.str();
            if (static_cast<size_t>(i) < channel_names_.size()) {
                channel_names_[i] = val;
            } else {
                channel_names_.push_back(val);
            }
        }
    }

    static void add_dynamic_channelknobs(void*, Knob_Callback);

public:
    explicit RemoveUnlimited(Node* node)
        : NoIop(node)
        , operation_(1)
        , channels_amount_(2)
        , use_wildcard_(false)
        , wildcard_entry_("")
        , knobs_amount_(0)
        , dynamic_channels_(2, Mask_None)
        , channel_names_(2, "none")
    {}

    void _validate(bool) override;
    void knobs(Knob_Callback) override;
    int knob_changed(Knob *) override;
    std::vector<ChannelSet>& get_dynamic_channels() { return dynamic_channels_; }

    [[nodiscard]] const char* Class() const override { return CLASS; }
    [[nodiscard]] const char* node_help() const override { return HELP; }
    static const Op::Description description;
};

static Op* build_node(Node* node) { return new RemoveUnlimited(node); }
const Op::Description RemoveUnlimited::description(CLASS, build_node);

void RemoveUnlimited::_validate(bool for_real) {
    copy_info();
    ChannelSet c;

    // 1. Collect channels from dynamic channel dropdowns
    for (const auto& new_channels : dynamic_channels_) {
        c += new_channels;
    }

    // 2. Collect channels from wildcard matching if enabled
    if (use_wildcard_ && !wildcard_entry_.empty()) {
        const std::vector<std::string> patterns = parseWildcardPatterns(wildcard_entry_);
        if (!patterns.empty()) {
            const ChannelMask inputChannels = info_.channels();
            for (Channel chan : inputChannels) {
                const char* ch_name = getName(chan);
                if (!ch_name) continue;
                const std::string name_str(ch_name);
                for (const auto& pat : patterns) {
                    if (isWildcardMatch(name_str, pat)) {
                        c += chan;
                        break;
                    }
                }
            }
        }
    }

    // 3. Apply operation
    if (operation_ == 1) {  // Keep
        info_.channels() &= c;
        set_out_channels(info_.channels());
    }
    else {  // Remove
        info_.turn_off(c);
        set_out_channels(c);
    }
}

static const char* const operation_enums[] = {
    "remove", "keep", nullptr
};

void RemoveUnlimited::knobs(Knob_Callback f) {
    Enumeration_knob(f, &operation_, operation_enums, "operation");

    Int_knob(f, &channels_amount_, "channels_amount", "Amount of Channels:");
    SetFlags(f, Knob::KNOB_CHANGED_ALWAYS);

    Bool_knob(f, &use_wildcard_, "use_wildcard", "use wildcard");
    ClearFlags(f, Knob::STARTLINE);
    SetFlags(f, Knob::EARLY_STORE | Knob::KNOB_CHANGED_ALWAYS);
    Tooltip(f, "Enable wildcard channel matching");

    Knob* wk = String_knob(f, &wildcard_entry_, "wildcard", "wildcard:");
    SetFlags(f, Knob::STARTLINE);
    Tooltip(f, "Wildcard pattern (e.g. crypto*, depth*, rgba.alpha). Comma-separated entries.");
    if (wk && !use_wildcard_) {
        wk->enable(false);
    }

    if (!f.makeKnobs()) {
        add_dynamic_channelknobs(this->firstOp(), f);
    } else {
        knobs_amount_ = add_knobs(add_dynamic_channelknobs, this->firstOp(), f);
    }
}

int RemoveUnlimited::knob_changed(Knob *k) {
    if (!k) return 1;

    if (k->is("use_wildcard")) {
        use_wildcard_ = (k->get_value() != 0.0);
        if (Knob* wk = knob("wildcard")) {
            wk->enable(use_wildcard_);
        }
        return 1;
    }

    if (k->is("showPanel")) {
        if (Knob* wk = knob("wildcard")) {
            wk->enable(use_wildcard_);
        }
        return 1;
    }

    if (k->is("channels_amount")) {
        int target_amount = static_cast<int>(k->get_value());
        if (target_amount < 0) target_amount = 0;
        channels_amount_ = target_amount;

        save_current_channel_names();

        dynamic_channels_.resize(target_amount, Mask_None);
        channel_names_.resize(target_amount, "none");

        knobs_amount_ = replace_knobs(
            knob("wildcard"),
            knobs_amount_,
            add_dynamic_channelknobs,
            this->firstOp()
        );
        return 1;
    }

    const std::string kname = k->name();
    if (kname.rfind("channels", 0) == 0 && kname != "channels_amount") {
        int idx = std::atoi(kname.c_str() + 8) - 1;
        if (idx >= 0) {
            std::stringstream ss;
            k->to_script(ss, nullptr, false);
            std::string val = ss.str();
            if (static_cast<size_t>(idx) >= channel_names_.size()) {
                channel_names_.resize(idx + 1, "none");
            }
            channel_names_[idx] = val;
        }
        return 1;
    }

    return 1;
}

void RemoveUnlimited::add_dynamic_channelknobs(void* p, Knob_Callback f) {
    if (auto* node_op = static_cast<RemoveUnlimited*>(p)) {
        int count = node_op->channels_amount_;
        if (count < 0) count = 0;

        if (node_op->dynamic_channels_.size() != static_cast<size_t>(count)) {
            node_op->dynamic_channels_.resize(count, Mask_None);
        }
        if (node_op->channel_names_.size() != static_cast<size_t>(count)) {
            node_op->channel_names_.resize(count, "none");
        }

        for (int index = 0; index < count; ++index) {
            ChannelSet& channels = node_op->dynamic_channels_[index];
            std::string knob_name = "channels" + std::to_string(index + 1);

            channels = Mask_None;
            Knob* new_knob = Input_ChannelMask_knob(f, &channels, 0, knob_name.c_str());

            if (new_knob && f.makeKnobs()) {
                if (static_cast<size_t>(index) < node_op->channel_names_.size()) {
                    const std::string& val = node_op->channel_names_[index];
                    if (!val.empty()) {
                        new_knob->from_script(val.c_str());
                    }
                }
            }
        }
    }
}