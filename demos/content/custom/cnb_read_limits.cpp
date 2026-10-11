// SPDX-License-Identifier: MS-PL
// A tiny tile layer shows why container budgets and schema allocation limits are distinct.
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

using namespace CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentLoadException;

namespace
{
    constexpr auto GridChunk = MakeChunkId('G', 'R', 'I', 'D');
    constexpr auto NoteChunk = MakeChunkId('N', 'O', 'T', 'E');
    constexpr const char* TileType = "CnaDemos.BoundedTileLayer";

    void ReadLimits(CnaDemos::DemoContext& ctx)
    {
        std::vector<std::uint8_t> tiles(64);
        for (std::size_t i = 0; i < tiles.size(); ++i) tiles[i] = static_cast<std::uint8_t>((i / 8 + i % 8) % 2);
        CnbByteWriter grid;
        grid.WriteU32(8); grid.WriteU32(8);
        grid.WriteU32(static_cast<std::uint32_t>(tiles.size()));
        grid.WriteBytes(tiles);
        CnbByteWriter note;
        const std::string authorNote(80, 'L');
        note.WriteString(authorNote);
        CnbWriter writer(CnbAssetTypeIdFromName(TileType), 1);
        writer.SetMetadata(TileType, "tile_layer");
        writer.AddChunk(GridChunk, grid.Take(), CnbChunkFlags::Mandatory);
        writer.AddChunk(NoteChunk, note.Take(), CnbChunkFlags::None);
        const auto image = writer.Build();
        const auto document = CnbDocument::Parse(image, "8x8 tile layer");
        auto input = document.OpenChunk(document.RequireSingle(GridChunk));
        const auto width = input.ReadU32(), height = input.ReadU32();
        const auto count = input.ReadCount(1, "tiles");
        const auto decoded = input.ReadBytes(count);
        input.RequireExhausted();
        ctx.Check(width == 8 && height == 8 && count == 64 && std::equal(decoded.begin(), decoded.end(), tiles.begin(), tiles.end()),
                  "the bounded grid reader recovers the 8x8 checker tile layer");
        std::uint64_t total = 0, largest = 0;
        for (std::size_t i = 0; i < document.ChunkCount(); ++i)
        {
            const auto& entry = document.ChunkAt(i);
            total += entry.uncompressedSize;
            largest = std::max(largest, entry.uncompressedSize);
            ctx.Print("{}: {} stored bytes, {} logical bytes", ChunkIdToString(entry.type), entry.storedSize, entry.uncompressedSize);
        }
        ctx.Print("Container: {} bytes; total logical chunks: {} bytes", image.size(), total);

        // Each run changes one bound, keeping allocations tiny. The same complete file is
        // accepted above, so refusal below cannot be attributed to a malformed fixture.
        for (int budget = 0; budget < 3; ++budget)
        {
            auto limits = DefaultCnbReadLimits();
            const char* label;
            if (budget == 0) { limits.maxFileSize = image.size() - 1; label = "file byte budget"; }
            else if (budget == 1) { limits.maxChunkSize = largest - 1; label = "single-chunk byte budget"; }
            else { limits.maxTotalUncompressedSize = total - 1; label = "aggregate logical-byte budget"; }
            bool readerRejected = false, writerRejected = false;
            try { (void)CnbDocument::Parse(image, label, limits); }
            catch (const ContentLoadException& error) { readerRejected = true; ctx.Print("Reader: {}", error.what()); }
            writer.SetLimits(limits);
            try { (void)writer.Build(); }
            catch (const ContentLoadException& error) { writerRejected = true; ctx.Print("Writer: {}", error.what()); }
            ctx.Check(readerRejected && writerRejected, std::string(label) + " is enforced by both reader and writer");
        }
        writer.SetLimits(DefaultCnbReadLimits());
        ctx.Check(writer.Build() == image, "restoring the writer's limits preserves the accepted image");

        // A custom payload is opaque to the container. Parse accepts it; the bounded cursor
        // enforces schema lengths before a game reserves an array or copies a string.
        auto arrayLimits = DefaultCnbReadLimits();
        arrayLimits.maxArrayElementCount = 16;
        const auto arrayDocument = CnbDocument::Parse(image, "tile count budget", arrayLimits);
        auto arrayInput = arrayDocument.OpenChunk(arrayDocument.RequireSingle(GridChunk));
        (void)arrayInput.ReadU32(); (void)arrayInput.ReadU32();
        bool arrayRejected = false;
        try { (void)arrayInput.ReadCount(1, "tiles"); }
        catch (const ContentLoadException& error) { arrayRejected = true; ctx.Print("Schema: {}", error.what()); }
        ctx.Check(arrayRejected, "the schema refuses 64 tiles against a 16-element limit before reserving storage");

        auto stringLimits = DefaultCnbReadLimits();
        stringLimits.maxStringBytes = 64; // metadata names fit, but the 80-byte game note does not
        const auto stringDocument = CnbDocument::Parse(image, "author note budget", stringLimits);
        auto stringInput = stringDocument.OpenChunk(stringDocument.RequireSingle(NoteChunk));
        bool stringRejected = false;
        try { (void)stringInput.ReadString(); }
        catch (const ContentLoadException& error) { stringRejected = true; ctx.Print("Schema: {}", error.what()); }
        ctx.Check(stringRejected, "the schema string limit is enforced after the container parses successfully");

        auto damaged = image;
        damaged.back() ^= 1u;
        bool corruptionRejected = false;
        try { (void)CnbDocument::Parse(damaged, "damaged tile layer"); }
        catch (const ContentLoadException& error) { corruptionRejected = true; ctx.Print("Corruption: {}", error.what()); }
        ctx.Check(CnbDocument::HasMagic(damaged) && corruptionRejected,
                  "a recognizable CNB signature does not establish container validity");
    }
}

CNA_DEMOS_CONSOLE(ReadLimits, {
    .id = "content.custom.cnb_read_limits",
    .title = "CNB container budgets and schema limits (CNAEXT)",
    .description = "Inspects a tiny tile-layer container and checks file, per-chunk and aggregate byte budgets on "
                   "both writer and reader, then enforces schema array/string limits before allocating game data.",
    .apis = {"CNA::Content::Cnb::CnbWriter::SetLimits", "CNA::Content::Cnb::CnbWriter::Build",
             "CNA::Content::Cnb::CnbDocument::Parse", "CNA::Content::Cnb::CnbDocument::ChunkCount",
             "CNA::Content::Cnb::CnbDocument::ChunkAt", "CNA::Content::Cnb::CnbDocument::OpenChunk",
             "CNA::Content::Cnb::CnbDocument::HasMagic", "CNA::Content::Cnb::CnbByteReader::ReadCount",
             "CNA::Content::Cnb::CnbByteReader::ReadString", "CNA::Content::Cnb::DefaultCnbReadLimits"},
    .tags = {"content", "custom", "cnb", "limits", "validation", "cnaext"},
    .restrictions = "Small in-memory uncompressed fixtures; no claim of every limit, compressed expansion or allocation profiling.",
})
