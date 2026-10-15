// SPDX-License-Identifier: MS-PL
// A Curve authored as CNJ (CNA's JSON content format), compiled to CNB and loaded as a Curve.
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbCurveCodec.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveContinuity.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveLoopType.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"

using namespace Microsoft::Xna::Framework;
using Microsoft::Xna::Framework::Content::ContentManager;
namespace Cnb = CNA::Content::Cnb;

namespace
{
    bool SameCurve(const Curve& left, const Curve& right)
    {
        if (left.getPreLoopProperty() != right.getPreLoopProperty() ||
            left.getPostLoopProperty() != right.getPostLoopProperty() ||
            left.getKeysProperty().getCountProperty() !=
                right.getKeysProperty().getCountProperty())
        {
            return false;
        }
        for (int index = 0; index < left.getKeysProperty().getCountProperty(); ++index)
        {
            if (left.getKeysProperty()[index] != right.getKeysProperty()[index]) return false;
        }
        return true;
    }

    template<class Function>
    bool Refuses(Function&& function)
    {
        try
        {
            function();
            return false;
        }
        catch (const std::exception&)
        {
            return true;
        }
    }

    std::vector<std::uint8_t> CurveHeader(const std::uint32_t preLoop,
                                          const std::uint32_t postLoop,
                                          const std::uint32_t keyCount)
    {
        Cnb::CnbByteWriter bytes;
        bytes.WriteU32(preLoop);
        bytes.WriteU32(postLoop);
        bytes.WriteU32(keyCount);
        return bytes.Take();
    }

    std::vector<std::uint8_t> OneKey(const std::uint32_t continuity)
    {
        Cnb::CnbByteWriter bytes;
        bytes.WriteF32(0.0f);
        bytes.WriteF32(1.0f);
        bytes.WriteF32(-2.0f);
        bytes.WriteF32(3.0f);
        bytes.WriteU32(continuity);
        return bytes.Take();
    }

    Cnb::CnbDocument CurveDocument(const std::uint32_t assetType,
                                   const std::uint32_t schema,
                                   const std::vector<std::uint8_t>& header,
                                   const std::vector<std::uint8_t>& keys,
                                   const int headerCopies = 1,
                                   const int keyCopies = 1)
    {
        Cnb::CnbWriter writer(assetType, schema);
        writer.SetMetadata("Microsoft.Xna.Framework.Curve", "synthetic/curve");
        for (int copy = 0; copy < headerCopies; ++copy)
        {
            writer.AddChunk(Cnb::CnbCurveChunk::Header, header,
                            Cnb::CnbChunkFlags::Mandatory);
        }
        for (int copy = 0; copy < keyCopies; ++copy)
        {
            writer.AddChunk(Cnb::CnbCurveChunk::Keys, keys,
                            Cnb::CnbChunkFlags::Mandatory);
        }
        return Cnb::CnbDocument::Parse(writer.Build(), "synthetic curve CNB");
    }

    void CurveAsset(CnaDemos::DemoContext& ctx)
    {
        // content/curves/bounce.cnj describes one bounce: keys at 0, 0.5 and 1 with tangents,
        // repeated by postLoop Cycle. XNA authored curves like this as XML in a content project;
        // CNA's pipeline compiles the JSON into curves/bounce.cnb. A Curve needs no graphics
        // device, so a ContentManager without services is enough.
        ContentManager content(nullptr, "assets");
        const Curve bounce = content.Load<Curve>("curves/bounce");

        ctx.Print("{} keys, PreLoop {}, PostLoop {}", bounce.getKeysProperty().getCountProperty(),
                  static_cast<int>(bounce.getPreLoopProperty()), static_cast<int>(bounce.getPostLoopProperty()));
        ctx.Check(bounce.getKeysProperty().getCountProperty() == 3, "the curve has the three authored keys");
        ctx.Check(bounce.getPostLoopProperty() == CurveLoopType::Cycle, "PostLoop is Cycle, as authored");

        for (const float t : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f})
            ctx.Print("Evaluate({:.2f}) = {:.3f}", t, bounce.Evaluate(t));
        ctx.CheckNear(bounce.Evaluate(0.5f), 1.0, 1e-5, "the top of the bounce is at 0.5");
        ctx.CheckNear(bounce.Evaluate(1.25f), bounce.Evaluate(0.25f), 1e-5, "after the last key the bounce cycles");
        ctx.Check(bounce.Evaluate(-1.0f) == 0.0f, "before the first key PreLoop Constant holds the first value");

        ctx.Check(Cnb::CnbCurveChunk::Header == Cnb::MakeChunkId('C', 'R', 'V', 'H') &&
                      Cnb::CnbCurveChunk::Keys == Cnb::MakeChunkId('C', 'R', 'V', 'K') &&
                      Cnb::CnbCurveSchemaVersion == 1u && Cnb::CnbCurveKeyStride == 20u,
                  "the Curve schema freezes its two chunk IDs, version and five-field key stride");

        const std::filesystem::path compiledPath =
            std::filesystem::path(TitleLocation::Path()) / "assets/curves/bounce.cnb";
        const Cnb::CnbDocument compiled = Cnb::CnbDocument::ParseFile(
            compiledPath.generic_string());
        const Curve directlyDecoded = Cnb::DecodeCurveFromCnb(compiled);
        ctx.Check(compiled.AssetTypeId() == Cnb::CnbAssetTypeId::Curve &&
                      compiled.AssetSchemaVersion() == Cnb::CnbCurveSchemaVersion &&
                      compiled.Metadata().present &&
                      compiled.Metadata().assetTypeName == "Microsoft.Xna.Framework.Curve" &&
                      compiled.Metadata().contentName == "curves/bounce" &&
                      compiled.FindAll(Cnb::CnbCurveChunk::Header).size() == 1u &&
                      compiled.FindAll(Cnb::CnbCurveChunk::Keys).size() == 1u &&
                      SameCurve(directlyDecoded, bounce),
                  "the project-built CNB declares the exact schema and direct decoding matches ContentManager");

        const auto encoded = Cnb::EncodeCurveToCnb(bounce, "Curves/roundtrip");
        const auto repeated = Cnb::EncodeCurveToCnb(bounce, "Curves/roundtrip");
        const Cnb::CnbDocument roundTripDocument =
            Cnb::CnbDocument::Parse(encoded, "round-trip curve CNB");
        const Curve roundTrip = Cnb::DecodeCurveFromCnb(roundTripDocument);
        ctx.Check(encoded == repeated && SameCurve(roundTrip, bounce) &&
                      roundTripDocument.Metadata().contentName == "Curves/roundtrip",
                  "encoding is byte-deterministic and preserves every authored key field and logical name");

        const std::array<CurveLoopType, 5> loopTypes{
            CurveLoopType::Constant, CurveLoopType::Cycle, CurveLoopType::CycleOffset,
            CurveLoopType::Oscillate, CurveLoopType::Linear};
        bool allEnums = true;
        for (std::size_t index = 0; index < loopTypes.size(); ++index)
        {
            Curve typed;
            typed.setPreLoopProperty(loopTypes[index]);
            typed.setPostLoopProperty(loopTypes[loopTypes.size() - index - 1u]);
            typed.getKeysProperty().Add(CurveKey(-1.0f, 2.0f, -3.0f, 4.0f,
                                                 CurveContinuity::Step));
            typed.getKeysProperty().Add(CurveKey(2.0f, -5.0f, 6.0f, -7.0f,
                                                 CurveContinuity::Smooth));
            const Curve restored = Cnb::DecodeCurveFromCnb(Cnb::CnbDocument::Parse(
                Cnb::EncodeCurveToCnb(typed), "enumerated curve CNB"));
            allEnums = allEnums && SameCurve(restored, typed);
        }
        ctx.Check(allEnums,
                  "all five loop values and both continuity values survive exact key-table round trips");

        const Curve empty;
        const Curve restoredEmpty = Cnb::DecodeCurveFromCnb(Cnb::CnbDocument::Parse(
            Cnb::EncodeCurveToCnb(empty), "empty curve CNB"));
        ctx.Check(SameCurve(restoredEmpty, empty) &&
                      restoredEmpty.getKeysProperty().getCountProperty() == 0,
                  "an empty Curve remains a valid zero-row key table with its safe loop defaults");

        const auto validHeader = CurveHeader(0u, 1u, 0u);
        const std::vector<std::uint8_t> noKeys;
        ctx.Check(
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Texture2D, 1u, validHeader, noKeys));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 2u, validHeader, noKeys));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, validHeader, noKeys, 0, 1));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, validHeader, noKeys, 2, 1));
            }),
            "the decoder rejects wrong identity/version and missing or duplicate mandatory tables");

        ctx.Check(
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, CurveHeader(5u, 0u, 0u), noKeys));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, CurveHeader(0u, 0u, 1u), noKeys));
            }) &&
            Refuses([&]
            {
                (void)Cnb::DecodeCurveFromCnb(CurveDocument(
                    Cnb::CnbAssetTypeId::Curve, 1u, CurveHeader(0u, 0u, 1u), OneKey(2u)));
            }),
            "invalid loop/continuity ordinals and a key-count byte mismatch are refused independently");
    }
}

CNA_DEMOS_CONSOLE(CurveAsset, {
    .id = "content.cnj.curve",
    .title = "Curve from CNJ content",
    .description = "Loads and evaluates a Curve authored in CNA's JSON format, then audits the public CNB Curve codec through exact deterministic round trips, every persisted loop/continuity value and representative schema refusals.",
    .apis = {"ContentManager::Load", "Curve::Evaluate", "Curve::Keys", "Curve::PreLoop", "Curve::PostLoop",
             "CNA::Content::Cnb::CnbCurveChunk::Header",
             "CNA::Content::Cnb::CnbCurveChunk::Keys",
             "CNA::Content::Cnb::CnbCurveSchemaVersion",
             "CNA::Content::Cnb::CnbCurveKeyStride",
             "CNA::Content::Cnb::EncodeCurveToCnb",
             "CNA::Content::Cnb::DecodeCurveFromCnb"},
    .tags = {"content", "cnj", "json", "curve", "animation"},
    .assets = {"curves/bounce.cnb"},
})
