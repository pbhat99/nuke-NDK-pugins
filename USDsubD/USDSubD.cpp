// Copyright 2025 Marten Blumen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FN_SHARED_EXPORT
#define FN_SHARED_EXPORT __declspec(dllexport)
#endif

#include <DDImage/GeomOp.h>
#include <DDImage/Knobs.h>
#include <DDImage/Op.h>

#include <usg/engine/GeomEngine.h>
#include <usg/engine/GeomSceneContext.h>
#include <usg/engine/EngineTokens.h>
#include <usg/geom/MeshPrim.h>
#include <usg/geom/Stage.h>
#include <usg/geom/PrimRange.h>
#include <usg/geom/Attribute.h>
#include <usg/tools/Subdivide.h>

#include <string>
#include <vector>

using namespace DD::Image;

// ============================================================
// ENGINE
// ============================================================
class USDSubDEngine : public GeomOpEngine
{
public:
    USDSubDEngine(GeomOpNode* parent) : GeomOpEngine(parent) {}
    std::string name() const override { return "USDSubDEngine"; }

protected:
    void processScenegraph(usg::GeomSceneContext& context) override
    {
        GeomOpEngine::processScenegraph(context);

        int iterations = getValue<int>(1, "iterations");
        bool onlySubdMeshes = getValue<bool>(1, "only_subd_meshes");

        if (iterations <= 0) return;

        usg::StageRef stage = editableStage(context);
        if (!stage) return;

        std::vector<usg::MeshPrim> meshes;
        usg::PrimRange range = stage->traverse();
        for (auto it = range.begin(); it != range.end(); ++it)
        {
            usg::Prim prim = *it;
            if (!prim.isA<usg::MeshPrim>()) continue;
            usg::MeshPrim mesh(prim);
            if (onlySubdMeshes && mesh.getSubdivisionScheme() == usg::GeomTokens.none) continue;
            meshes.push_back(mesh);
        }

        if (meshes.empty()) return;

        Op* op = firstOp();
        if (op) op->progressFraction(0.0);
        float totalMeshes = static_cast<float>(meshes.size());

        int meshesWithoutUVs = 0;

        for (size_t i = 0; i < meshes.size(); ++i)
        {
            // Thread-safe abort check
            if (aborted()) break;

            usg::MeshPrim mesh = meshes[i];
            usg::Prim prim(mesh);

            usg::IntArray faceVertexCounts = mesh.getFaceVertexCounts();
            usg::IntArray vertIndices = mesh.getFaceVertexIndices();
            usg::Vec3fArray points = mesh.getPoints();

            usg::MeshTessellateContext tessCtx;
            tessCtx.faceVertexCounts = &faceVertexCounts;
            tessCtx.vert_position_indices = &vertIndices;
            tessCtx.position_lists.push_back(&points);

            usg::Attribute stAttr;
            usg::Attribute stIdxAttr;
            std::string stAttrName;

            std::vector<usg::Token> attrNames = prim.getAttributes();
            for (const usg::Token& attrToken : attrNames)
            {
                std::string name = attrToken.asString();
                if (name.rfind("primvars:", 0) != 0) continue;
                if (name.size() >= 8 && name.substr(name.size() - 8) == ":indices") continue;

                usg::Attribute candidate = prim.getAttr(attrToken);
                if (!candidate.hasValue()) continue;

                usg::Vec2fArray testVals;
                candidate.getValue(testVals);
                if (!testVals.empty())
                {
                    stAttr = candidate;
                    stAttrName = name;
                    break;
                }
            }

            bool hasUVs = stAttr.hasValue();
            if (!hasUVs) meshesWithoutUVs++;

            if (hasUVs) {
                std::string targetIdxName = stAttrName + ":indices";
                for (const usg::Token& attrToken : attrNames) {
                    if (attrToken.asString() == targetIdxName) {
                        stIdxAttr = prim.getAttr(attrToken);
                        break;
                    }
                }
            }

            usg::Vec2fArray flatUVs;
            if (hasUVs)
            {
                usg::Vec2fArray uvValues;
                stAttr.getValue(uvValues);
                usg::IntArray uvIndices;
                if (stIdxAttr.hasValue()) stIdxAttr.getValue(uvIndices);

                if (!uvIndices.empty() && uvIndices.size() == vertIndices.size())
                {
                    flatUVs.resize(vertIndices.size());
                    for (size_t v = 0; v < vertIndices.size(); ++v)
                        flatUVs[v] = uvValues[uvIndices[v]];
                }
                else if (uvValues.size() == vertIndices.size())
                {
                    flatUVs = uvValues;
                }
                else if (uvValues.size() == points.size())
                {
                    flatUVs.resize(vertIndices.size());
                    for (size_t v = 0; v < vertIndices.size(); ++v)
                        flatUVs[v] = uvValues[vertIndices[v]];
                }

                if (!flatUVs.empty())
                    tessCtx.vert_vec2_attribs.push_back(&flatUVs);
            }

            usg::ArgSet args;
            usg::subdivideUniform(
                args,
                static_cast<int32_t>(faceVertexCounts.size()),
                static_cast<int32_t>(vertIndices.size()),
                static_cast<int32_t>(points.size()),
                static_cast<int32_t>(tessCtx.vert_vec2_attribs.size()),
                iterations,
                tessCtx
            );

            if (tessCtx.position_lists[0]->empty() || tessCtx.faceVertexCounts->empty())
            {
                if (op) op->progressFraction((i + 1) / totalMeshes);
                continue;
            }

            mesh.setPoints(*tessCtx.position_lists[0]);
            mesh.setFaceVertexCounts(*tessCtx.faceVertexCounts);
            mesh.setFaceVertexIndices(*tessCtx.vert_position_indices);
            mesh.setSubdivisionScheme(usg::GeomTokens.none);

            if (hasUVs && !tessCtx.vert_vec2_attribs.empty())
            {
                usg::Vec2fArray& subdUVs = *tessCtx.vert_vec2_attribs[0];
                stAttr.setValue(subdUVs);
                stAttr.setInterpolation(usg::GeomTokens.faceVarying);
                if (stIdxAttr.hasValue()) {
                    usg::IntArray newIndices;
                    newIndices.resize(subdUVs.size());
                    for (size_t v = 0; v < subdUVs.size(); ++v)
                        newIndices[v] = static_cast<int>(v);
                    stIdxAttr.setValue(newIndices);
                }
            }
            if (op) op->progressFraction((i + 1) / totalMeshes);
        }

        // Thread-Safe native Nuke warning system
        if (op && meshesWithoutUVs > 0)
        {
            op->warning("%d mesh(es) are missing UV primvars. Textures may break.", meshesWithoutUVs);
        }
    }
};

// ============================================================
// GEOMOP
// ============================================================
class USDSubD : public GeomOp
{
    int  _iterations = 0;
    bool _onlySubdMeshes = true;

public:
    USDSubD(Node* node) : GeomOp(node, BuildEngine<USDSubDEngine>()) {}
    static const GeomOp::Description description;
    const char* Class()     const override { return "USDSubD"; }
    const char* node_help() const override { return "Bakes subdivision into USD meshes."; }

    int minimum_inputs() const override { return 1; }
    int maximum_inputs() const override { return 1; }

    void knobs(Knob_Callback f) override
    {
        // 1. HEADER
        Text_knob(f, "<font size='6' color='#666'><b>USDSubD</b></font>");
        Text_knob(f, "<font color='#888'>High-performance Refinement</font>");
        Divider(f);

        // 2. CORE SETTINGS
        Int_knob(f, &_iterations, "iterations", "Iterations");
        Tooltip(f, "Refinement levels. Careful: 4+ can be extremely heavy.");

        Bool_knob(f, &_onlySubdMeshes, "only_subd_meshes", "Only subd tagged meshes");
        Tooltip(f, "Filters for meshes where subdivisionScheme is not 'none'.");

        // 3. USAGE INFO
        Divider(f, "Information");

        const char* infoText =
            "<b>Artist Notes:</b><br/>"
            "&bull; UVs are promoted to <b>Face-Varying</b> to prevent seam tearing.<br/>"
            "&bull; Levels > 3 will impact UI responsiveness.<br/><br/>"
            "<b>Algorithm:</b> Catmull-Clark via OpenSubdiv.";

        Text_knob(f, infoText);

        Divider(f);
        Text_knob(f, "<font color='#777' size='1'>Created by Marten Blumen</font>");
    }

    void append(DD::Image::Hash& hash) override
    {
        GeomOp::append(hash);
        hash.append(_iterations);
        hash.append(_onlySubdMeshes);
    }
};

// ============================================================
// NDK REGISTRATION
// ============================================================
extern "C" {
    FN_SHARED_EXPORT Op* build(Node* node) { return new USDSubD(node); }
    FN_SHARED_EXPORT void USDSubD_entry() {}
}
const GeomOp::Description USDSubD::description("USDSubD", build);