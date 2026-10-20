// SPDX-License-Identifier: MS-PL
// A tiny tile layer shows why container budgets and schema allocation limits are distinct.
#include <algorithm>
#include <cstdint>
#include <exception>
#include <limits>
#include <string>
#include <vector>
#include "CnaDemos/Demo.hpp"
#include "CNA/Content/Cnb/CnbArithmetic.hpp"
#include "CNA/Content/Cnb/CnbByteReader.hpp"
#include "CNA/Content/Cnb/CnbByteWriter.hpp"
#include "CNA/Content/Cnb/CnbChunkCompression.hpp"
#include "CNA/Content/Cnb/CnbCrc32c.hpp"
#include "CNA/Content/Cnb/CnbWriter.hpp"
#include "Microsoft/Xna/Framework/Content/ContentLoadException.hpp"

using namespace CNA::Content::Cnb;
using Microsoft::Xna::Framework::Content::ContentLoadException;

namespace
{
    constexpr auto GridChunk = MakeChunkId('G', 'R', 'I', 'D');
    constexpr auto NoteChunk = MakeChunkId('N', 'O', 'T', 'E');
    constexpr const char* TileType = "CnaDemos.BoundedTileLayer";

    template <typename Action>
    bool Refuses(Action&& action)
    {
        try { action(); }
        catch (const std::exception&) { return true; }
        return false;
    }

    void ReadLimits(CnaDemos::DemoContext& ctx)
    {
        const CnbReadLimits defaults;
        const auto& sharedDefaults = DefaultCnbReadLimits();
        ctx.Check(defaults.maxFileSize == 512ull * 1024ull * 1024ull &&
                      defaults.maxChunkCount == 65536u &&
                      defaults.maxChunkSize == 384ull * 1024ull * 1024ull &&
                      defaults.maxTotalUncompressedSize == 1024ull * 1024ull * 1024ull &&
                      defaults.maxStringBytes == 1024u * 1024u &&
                      defaults.maxArrayElementCount == 16u * 1024u * 1024u &&
                      defaults.maxChunkAlignment == 4096u &&
                      sharedDefaults.maxFileSize == defaults.maxFileSize,
                  "the public CNB read limits expose the exact documented safe defaults");

        const CnbChunkEntry emptyEntry;
        const CnbExternalReference emptyReference;
        const CnbMetadata emptyMetadata;
        ctx.Check(emptyEntry.type == CnbChunkId{} && emptyEntry.flags == 0u &&
                      emptyEntry.offset == 0u && emptyEntry.storedSize == 0u &&
                      emptyEntry.uncompressedSize == 0u && emptyEntry.checksum == 0u &&
                      emptyEntry.compression == CnbCompression::None &&
                      emptyEntry.alignment == 1u && !emptyEntry.IsMandatory() &&
                      emptyReference.flags == 0u &&
                      emptyReference.expectedAssetTypeId == CnbAssetTypeId::Invalid &&
                      emptyReference.logicalName.empty() && !emptyMetadata.present &&
                      emptyMetadata.flags == 0u && emptyMetadata.assetTypeName.empty() &&
                      emptyMetadata.contentName.empty(),
                  "document carrier values have safe observable default states");

        ctx.Check(Refuses([] { (void)CnbWriter(CnbAssetTypeId::Invalid, 1u); }) &&
                      Refuses([] { (void)CnbWriter(CnbAssetTypeId::Texture2D, 0u); }),
                  "the writer constructor refuses invalid asset identity and schema zero");

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
        ctx.Check(writer.SchemaChunkCount() == 0u &&
                      writer.Limits().maxFileSize == sharedDefaults.maxFileSize &&
                      writer.Limits().maxChunkAlignment == sharedDefaults.maxChunkAlignment,
                  "a new writer has no schema chunks and owns the default read limits");
        writer.SetMetadata(TileType, "tile_layer");
        writer.AddChunk(GridChunk, grid.Take(), CnbChunkFlags::Mandatory);
        writer.AddChunk(NoteChunk, note.Take(), CnbChunkFlags::None, 64u);
        ctx.Check(writer.SchemaChunkCount() == 2u,
                  "schema chunk accounting excludes generated CMET and XREF chunks");

        CnbWriter invalidChunks(CnbAssetTypeId::Texture2D, 1u);
        ctx.Check(Refuses([&] { invalidChunks.AddChunk(CnbChunkId{}, {}); }) &&
                      Refuses([&] { invalidChunks.AddChunk(GridChunk, {}, 2u); }) &&
                      Refuses([&] { invalidChunks.AddChunk(GridChunk, {}, 0u, 3u); }) &&
                      Refuses([&] { invalidChunks.AddChunk(GridChunk, {}, 0u, 8192u); }) &&
                      Refuses([&] { invalidChunks.AddChunk(CnbContainerChunk::Metadata, {}); }) &&
                      Refuses([&] { invalidChunks.AddChunk(CnbContainerChunk::ExternalReferences, {}); }),
                  "AddChunk refuses malformed IDs, unknown flags, bad alignments and container-owned IDs");
        const auto image = writer.Build();

        CnbByteWriter primitives({0xAAu});
        primitives.WriteU8(0x7Fu);
        primitives.WriteU16(0x1234u);
        primitives.WriteU32(0x89ABCDEFu);
        primitives.WriteU64(UINT64_C(0x0123456789ABCDEF));
        primitives.WriteI32(-2);
        primitives.WriteF32(1.5f);
        primitives.WriteF64(-2.25);
        primitives.WriteString(std::string{"A\xC2\xA2", 3u});
        primitives.WriteBytes(std::vector<std::uint8_t>{0xDEu, 0xADu});
        primitives.WriteZeros(3u);
        const std::vector<std::uint8_t> expectedPrimitives{
            0xAAu, 0x7Fu, 0x34u, 0x12u, 0xEFu, 0xCDu, 0xABu, 0x89u,
            0xEFu, 0xCDu, 0xABu, 0x89u, 0x67u, 0x45u, 0x23u, 0x01u,
            0xFEu, 0xFFu, 0xFFu, 0xFFu, 0x00u, 0x00u, 0xC0u, 0x3Fu,
            0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x02u, 0xC0u,
            0x03u, 0x00u, 0x00u, 0x00u, 0x41u, 0xC2u, 0xA2u,
            0xDEu, 0xADu, 0x00u, 0x00u, 0x00u};
        ctx.Check(primitives.Size() == expectedPrimitives.size() &&
                      std::equal(primitives.View().begin(), primitives.View().end(),
                                 expectedPrimitives.begin(), expectedPrimitives.end()),
                  "the byte writer appends initial, integer, float, UTF-8, raw and zero data in exact canonical bytes");

        const std::vector<std::uint8_t> primitiveBytes = primitives.Take();
        ctx.Check(primitiveBytes == expectedPrimitives && primitives.Size() == 0u &&
                      primitives.View().empty(),
                  "Take transfers the complete buffer and leaves the writer observably empty");

        CnbByteReader primitiveReader(primitiveBytes, "primitive matrix");
        const bool readerValues = primitiveReader.Size() == primitiveBytes.size() &&
            primitiveReader.Position() == 0u &&
            primitiveReader.Remaining() == primitiveBytes.size() &&
            primitiveReader.Context() == "primitive matrix" &&
            primitiveReader.ReadU8() == 0xAAu && primitiveReader.ReadU8() == 0x7Fu &&
            primitiveReader.ReadU16() == 0x1234u &&
            primitiveReader.ReadU32() == 0x89ABCDEFu &&
            primitiveReader.ReadU64() == UINT64_C(0x0123456789ABCDEF) &&
            primitiveReader.ReadI32() == -2 && primitiveReader.ReadF32() == 1.5f &&
            primitiveReader.ReadF64() == -2.25 &&
            primitiveReader.ReadString() == std::string{"A\xC2\xA2", 3u};
        const std::span<const std::uint8_t> raw = primitiveReader.ReadBytes(2u);
        primitiveReader.Skip(3u);
        primitiveReader.RequireExhausted();
        ctx.Check(readerValues && raw.size() == 2u && raw[0] == 0xDEu && raw[1] == 0xADu &&
                      primitiveReader.Position() == primitiveBytes.size() &&
                      primitiveReader.Remaining() == 0u,
                  "the bounded reader restores every primitive, exposes raw views, skips exactly and reports its cursor state");

        const std::string validUtf8{"A\xC2\xA2\xE2\x82\xAC\xF0\x9F\x98\x80", 10u};
        const std::string overlongUtf8{"\xC0\xAF", 2u};
        const std::string surrogateUtf8{"\xED\xA0\x80", 3u};
        const std::string truncatedUtf8{"\xE2\x82", 2u};
        const std::string tooLargeUtf8{"\xF4\x90\x80\x80", 4u};
        ctx.Check(CnbByteReader::IsWellFormedUtf8("") &&
                      CnbByteReader::IsWellFormedUtf8(validUtf8) &&
                      !CnbByteReader::IsWellFormedUtf8(overlongUtf8) &&
                      !CnbByteReader::IsWellFormedUtf8(surrogateUtf8) &&
                      !CnbByteReader::IsWellFormedUtf8(truncatedUtf8) &&
                      !CnbByteReader::IsWellFormedUtf8(tooLargeUtf8),
                  "UTF-8 validation accepts every encoded width and rejects overlong, surrogate, truncated and out-of-range sequences");

        bool badWriterString = false;
        try
        {
            CnbByteWriter invalidString;
            invalidString.WriteString(overlongUtf8);
        }
        catch (const ContentLoadException&)
        {
            badWriterString = true;
        }
        bool truncatedRead = false;
        try
        {
            CnbByteReader shortReader(std::span<const std::uint8_t>(primitiveBytes).first(3u),
                                      "short primitive");
            (void)shortReader.ReadU32();
        }
        catch (const ContentLoadException& error)
        {
            const std::string message = error.what();
            truncatedRead = message.find("short primitive") != std::string::npos &&
                message.find("offset 0") != std::string::npos;
        }
        bool explicitFailure = false;
        try
        {
            CnbByteReader failureReader(primitiveBytes, "explicit failure");
            failureReader.Skip(2u);
            failureReader.Fail("sentinel reason");
        }
        catch (const ContentLoadException& error)
        {
            const std::string message = error.what();
            explicitFailure = message.find("explicit failure") != std::string::npos &&
                message.find("offset 2") != std::string::npos &&
                message.find("sentinel reason") != std::string::npos;
        }
        ctx.Check(badWriterString && truncatedRead && explicitFailure,
                  "writer UTF-8, bounded truncation and explicit cursor failures retain their precise context");

        const std::uint64_t max64 = std::numeric_limits<std::uint64_t>::max();
        bool addOverflow = false;
        bool multiplyOverflow = false;
        try { (void)CheckedAdd(max64, 1u, "add probe"); }
        catch (const ContentLoadException&) { addOverflow = true; }
        try { (void)CheckedMultiply(max64, 2u, "multiply probe"); }
        catch (const ContentLoadException&) { multiplyOverflow = true; }
        ctx.Check(CheckedAdd(max64 - 1u, 1u, "exact add") == max64 &&
                      CheckedMultiply(max64, 1u, "exact multiply") == max64 &&
                      CheckedMultiply(max64, 0u, "zero multiply") == 0u &&
                      addOverflow && multiplyOverflow,
                  "checked arithmetic accepts exact uint64 boundaries and refuses both wraparound classes");

        const std::vector<std::uint8_t> crcProbe{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
        const std::uint32_t crc = Crc32c(crcProbe);
        std::uint32_t continued = Crc32cSeed();
        continued = Crc32cContinue(continued,
                                   std::span<const std::uint8_t>(crcProbe).first(4u));
        continued = Crc32cContinue(continued,
                                   std::span<const std::uint8_t>(crcProbe).subspan(4u));
        ctx.Print("CRC-32C path: {}", Crc32cUsesHardwareEXT() ? "hardware" : "portable");
        ctx.Check(Crc32c({}) == 0u && Crc32cSeed() == 0u && crc == 0xE3069283u &&
                      continued == crc && Crc32cPortableEXT(crcProbe) == crc,
                  "CRC-32C matches its standard check vector, incremental fold and portable reference path");

        const auto headerBytes = std::span<const std::uint8_t>(image);
        const std::uint32_t storedHeaderCrc =
            static_cast<std::uint32_t>(image[44]) |
            (static_cast<std::uint32_t>(image[45]) << 8u) |
            (static_cast<std::uint32_t>(image[46]) << 16u) |
            (static_cast<std::uint32_t>(image[47]) << 24u);
        ctx.Check(Crc32c(headerBytes.first(Format::HeaderChecksumCoverage)) == storedHeaderCrc,
                  "the generic writer stores the public CRC-32C over the exact declared header prefix");

        const std::vector<std::uint8_t> compressionProbe(4096u, 0x5Au);
        const auto noneCompressed = CompressCnbChunk(compressionProbe, CnbCompression::None);
        const auto noneRestored = DecompressCnbChunk(noneCompressed, CnbCompression::None,
                                                     noneCompressed.size(), noneCompressed.size(),
                                                     "uncompressed probe");
        ctx.Check(IsCnbCompressionSupported(CnbCompression::None) &&
                      CnbCompressionToString(CnbCompression::None) == "none" &&
                      CnbCompressionToString(CnbCompression::ReservedLz4) == "LZ4" &&
                      CnbCompressionToString(CnbCompression::ReservedZstd) == "Zstandard" &&
                      CnbCompressionToString(CnbCompression::ReservedDeflate) == "Deflate" &&
                      CnbCompressionToString(static_cast<CnbCompression>(99u)) == "unknown codec 99" &&
                      noneCompressed == compressionProbe && noneRestored == compressionProbe,
                  "compression identity, names and the no-compression copy path are exact");

        // Protect the direct helper independently of the container parser's validation.
        const auto refusesNone = [&](std::uint64_t declared, std::uint64_t ceiling)
        {
            try
            {
                (void)DecompressCnbChunk(compressionProbe, CnbCompression::None,
                                        declared, ceiling, "CNA-DEMOS-ISSUE-110 probe");
            }
            catch (const ContentLoadException& error)
            {
                return std::string(error.what()).find("CNA-DEMOS-ISSUE-110 probe") !=
                       std::string::npos;
            }
            return false;
        };
        const bool bothRefused = refusesNone(compressionProbe.size() + 1u, 1u);
        const bool mismatchRefused = refusesNone(compressionProbe.size() + 1u,
                                                compressionProbe.size() + 1u);
        const bool ceilingRefused = refusesNone(compressionProbe.size(), 1u);
        ctx.Check(bothRefused && mismatchRefused && ceilingRefused,
                  "CNA-DEMOS-ISSUE-110: direct None decompression enforces exact size and allocation limits");

        const bool zstdAvailable = IsCnbCompressionSupported(CnbCompression::Zstd);
        bool codecBehavior = !IsCnbCompressionSupported(CnbCompression::ReservedLz4) &&
            !IsCnbCompressionSupported(CnbCompression::ReservedDeflate) &&
            Refuses([&] { (void)CompressCnbChunk(compressionProbe, CnbCompression::ReservedLz4); }) &&
            Refuses([&] { (void)DecompressCnbChunk(compressionProbe, CnbCompression::ReservedDeflate,
                                                   compressionProbe.size(), compressionProbe.size(),
                                                   "unsupported codec probe"); });
        if (zstdAvailable)
        {
            const auto packedProbe = CompressCnbChunk(compressionProbe, CnbCompression::Zstd);
            codecBehavior = codecBehavior && packedProbe.size() < compressionProbe.size() &&
                DecompressCnbChunk(packedProbe, CnbCompression::Zstd, compressionProbe.size(),
                                   compressionProbe.size(), "Zstandard probe") == compressionProbe &&
                Refuses([&] { (void)DecompressCnbChunk(packedProbe, CnbCompression::Zstd,
                                                       compressionProbe.size() + 1u,
                                                       compressionProbe.size() + 1u,
                                                       "wrong decoded size"); }) &&
                Refuses([&] { (void)DecompressCnbChunk(packedProbe, CnbCompression::Zstd,
                                                       compressionProbe.size(),
                                                       compressionProbe.size() - 1u,
                                                       "decoded allocation ceiling"); });
        }
        ctx.Check(codecBehavior,
                  "implemented compression round-trips exactly and refuses size, ceiling and unsupported-codec violations");

        const auto document = CnbDocument::Parse(image, "8x8 tile layer");
        document.RequireAsset(CnbAssetTypeIdFromName(TileType), 1u);
        const auto& metadata = document.Metadata();
        ctx.Check(document.Origin() == "8x8 tile layer" &&
                      document.ContainerMajor() == Format::ContainerMajor &&
                      document.ContainerMinor() == Format::ContainerMinor &&
                      document.AssetTypeId() == CnbAssetTypeIdFromName(TileType) &&
                      document.AssetSchemaVersion() == 1u && metadata.present &&
                      metadata.flags == 0u && metadata.assetTypeName == TileType &&
                      metadata.contentName == "tile_layer" &&
                      document.ExternalReferences().empty() &&
                      document.Limits().maxFileSize == sharedDefaults.maxFileSize,
                  "the parsed document exposes exact origin, versions, asset identity, metadata and limits");

        const auto gridIndex = document.RequireSingle(GridChunk);
        const auto noteIndex = document.RequireSingle(NoteChunk);
        const auto& gridEntry = document.ChunkAt(gridIndex);
        const auto& noteEntry = document.ChunkAt(noteIndex);
        ctx.Check(gridEntry.type == GridChunk && gridEntry.flags == CnbChunkFlags::Mandatory &&
                      gridEntry.storedSize == 76u && gridEntry.uncompressedSize == 76u &&
                      gridEntry.checksum == Crc32c(document.ChunkData(gridIndex)) &&
                      gridEntry.compression == CnbCompression::None && gridEntry.alignment == 4u &&
                      gridEntry.offset % gridEntry.alignment == 0u && gridEntry.IsMandatory() &&
                      noteEntry.type == NoteChunk && noteEntry.flags == CnbChunkFlags::None &&
                      noteEntry.storedSize == 84u && noteEntry.uncompressedSize == 84u &&
                      noteEntry.checksum == Crc32c(document.ChunkData(noteIndex)) &&
                      noteEntry.compression == CnbCompression::None && noteEntry.alignment == 64u &&
                      noteEntry.offset % noteEntry.alignment == 0u && !noteEntry.IsMandatory(),
                  "table entries expose exact IDs, flags, offsets, sizes, checksums, codecs and alignments");

        ctx.Check(document.FindAll(GridChunk) == std::vector<std::size_t>{gridIndex} &&
                      document.FindSingle(GridChunk) == gridIndex &&
                      !document.FindSingle(MakeChunkId('M', 'I', 'S', 'S')).has_value() &&
                      Refuses([&] { (void)document.ChunkAt(document.ChunkCount()); }) &&
                      Refuses([&] { (void)document.ChunkData(document.ChunkCount()); }) &&
                      Refuses([&] { (void)document.OpenChunk(document.ChunkCount()); }) &&
                      Refuses([&] { (void)document.ExternalReferenceAt(0u, "tile atlas"); }),
                  "document lookup distinguishes one, absent and out-of-range chunks and references");

        auto copiedLimits = DefaultCnbReadLimits();
        copiedLimits.maxFileSize = image.size();
        CnbWriter copiedLimitWriter(CnbAssetTypeIdFromName(TileType), 1u);
        copiedLimitWriter.SetMetadata(TileType, "tile_layer");
        copiedLimitWriter.AddChunk(GridChunk, std::vector<std::uint8_t>{1u});
        copiedLimitWriter.SetLimits(copiedLimits);
        copiedLimits.maxFileSize = 1u;
        const auto copiedLimitImage = copiedLimitWriter.Build();
        auto parserLimits = DefaultCnbReadLimits();
        parserLimits.maxFileSize = copiedLimitImage.size();
        const auto copiedLimitDocument = CnbDocument::Parse(copiedLimitImage, "copied limits", parserLimits);
        parserLimits.maxFileSize = 1u;
        ctx.Check(copiedLimitWriter.Limits().maxFileSize == image.size() &&
                      copiedLimitDocument.Limits().maxFileSize == copiedLimitImage.size() &&
                      copiedLimitWriter.Build() == copiedLimitImage,
                  "writers and documents own copied limits and deterministic builds ignore later caller mutation");
        auto input = document.OpenChunk(document.RequireSingle(GridChunk));
        const auto width = input.ReadU32(), height = input.ReadU32();
        const auto count = input.ReadCount(1, "tiles");
        const auto decoded = input.ReadBytes(count);
        input.RequireExhausted();
        ctx.Check(width == 8 && height == 8 && count == 64 && std::equal(decoded.begin(), decoded.end(), tiles.begin(), tiles.end()),
                  "the bounded grid reader recovers the 8x8 checker tile layer");
        const auto& alignedNote = document.ChunkAt(document.RequireSingle(NoteChunk));
        ctx.Check(alignedNote.alignment == 64u && alignedNote.offset % alignedNote.alignment == 0u,
                  "the writer publishes and satisfies the note chunk's 64-byte alignment");
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
        for (int budget = 0; budget < 5; ++budget)
        {
            auto limits = DefaultCnbReadLimits();
            const char* label;
            if (budget == 0) { limits.maxFileSize = image.size() - 1; label = "file byte budget"; }
            else if (budget == 1) { limits.maxChunkSize = largest - 1; label = "single-chunk byte budget"; }
            else if (budget == 2) { limits.maxTotalUncompressedSize = total - 1; label = "aggregate logical-byte budget"; }
            else if (budget == 3) { limits.maxChunkCount = document.ChunkCount() - 1; label = "table entry budget"; }
            else { limits.maxChunkAlignment = 32; label = "chunk alignment budget"; }
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

        // CMET strings and the XREF row count are container data, so these two schema-looking
        // limits must be symmetric writer/reader guards here as well as inside game chunks.
        auto metadataLimits = DefaultCnbReadLimits();
        metadataLimits.maxStringBytes = std::string(TileType).size() - 1;
        bool metadataReader = false, metadataWriter = false;
        try { (void)CnbDocument::Parse(image, "metadata string budget", metadataLimits); }
        catch (const ContentLoadException& error) { metadataReader = true; ctx.Print("Reader: {}", error.what()); }
        writer.SetLimits(metadataLimits);
        try { (void)writer.Build(); }
        catch (const ContentLoadException& error) { metadataWriter = true; ctx.Print("Writer: {}", error.what()); }
        ctx.Check(metadataReader && metadataWriter,
                  "the CMET type name is bounded symmetrically by the container string limit");
        writer.SetLimits(DefaultCnbReadLimits());

        CnbWriter referenced(CnbAssetTypeIdFromName(TileType), 1);
        referenced.SetMetadata(TileType, "referenced_layer");
        CnbExternalReference atlas; atlas.logicalName = "tiles/atlas";
        CnbExternalReference palette; palette.logicalName = "tiles/palette";
        referenced.SetExternalReferences({atlas, palette});
        referenced.AddChunk(GridChunk, std::vector<std::uint8_t>{1}, CnbChunkFlags::Mandatory);
        const auto referencedImage = referenced.Build();
        auto referenceLimits = DefaultCnbReadLimits();
        referenceLimits.maxArrayElementCount = 1;
        bool referencesReader = false, referencesWriter = false;
        try { (void)CnbDocument::Parse(referencedImage, "external-reference count budget", referenceLimits); }
        catch (const ContentLoadException& error) { referencesReader = true; ctx.Print("Reader: {}", error.what()); }
        referenced.SetLimits(referenceLimits);
        try { (void)referenced.Build(); }
        catch (const ContentLoadException& error) { referencesWriter = true; ctx.Print("Writer: {}", error.what()); }
        ctx.Check(referencesReader && referencesWriter,
                  "the XREF row count is bounded symmetrically by the container array limit");

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

        // Compression reduces storage, not the decoded allocation budget. Keep the core example
        // useful in builds without libzstd, and report that this additional path was not run.
        if (!IsCnbCompressionSupported(CnbCompression::Zstd))
        {
            ctx.Print("Zstandard is unavailable: compressed expansion checks were not run.");
            return;
        }
        writer.SetCompression(CnbCompression::Zstd);
        const auto packed = writer.Build();
        const auto compressed = CnbDocument::Parse(packed, "compressed tile layer");
        bool matching = compressed.ChunkCount() == document.ChunkCount();
        bool compressedPayload = false;
        for (std::size_t i = 0; i < document.ChunkCount(); ++i)
        {
            const auto& original = document.ChunkAt(i);
            const auto index = compressed.RequireSingle(original.type);
            const auto& entry = compressed.ChunkAt(index);
            const auto restored = compressed.ChunkData(index), expected = document.ChunkData(i);
            matching = matching && std::equal(restored.begin(), restored.end(), expected.begin(), expected.end());
            if (original.type == CnbContainerChunk::Metadata)
                matching = matching && entry.compression == CnbCompression::None;
            if (entry.compression != CnbCompression::None)
            {
                compressedPayload = true;
                matching = matching && entry.storedSize < entry.uncompressedSize;
            }
            ctx.Print("{}: {} ({} stored / {} logical bytes)", ChunkIdToString(entry.type),
                      CnbCompressionToString(entry.compression), entry.storedSize, entry.uncompressedSize);
        }
        ctx.Print("Compression: {} -> {} file bytes; logical chunk budget stays {}", image.size(), packed.size(), total);
        ctx.Check(matching && compressedPayload,
                  "compressed payloads restore identical data while container metadata stays uncompressed");

        auto expansionLimits = DefaultCnbReadLimits();
        expansionLimits.maxFileSize = packed.size(); // the stored image fits exactly
        expansionLimits.maxTotalUncompressedSize = total - 1;
        bool expansionReader = false, expansionWriter = false;
        try { (void)CnbDocument::Parse(packed, "compressed expansion budget", expansionLimits); }
        catch (const ContentLoadException& error) { expansionReader = true; ctx.Print("Reader: {}", error.what()); }
        writer.SetLimits(expansionLimits);
        try { (void)writer.Build(); }
        catch (const ContentLoadException& error) { expansionWriter = true; ctx.Print("Writer: {}", error.what()); }
        ctx.Check(expansionReader && expansionWriter,
                  "fitting the stored-file budget does not bypass the aggregate decoded-byte budget");
        writer.SetLimits(DefaultCnbReadLimits());
        writer.SetCompression(CnbCompression::None);
        ctx.Check(writer.Build() == image, "turning compression off restores the original uncompressed image");
    }
}

CNA_DEMOS_CONSOLE(ReadLimits, {
    .id = "content.custom.cnb_read_limits",
    .title = "CNB container budgets and schema limits (CNAEXT)",
    .description = "Inspects an aligned tile-layer container and checks every public file/table/metadata budget on "
                   "both writer and reader, plus schema limits and compressed expansion when Zstandard is available.",
    .apis = {"CNA::Content::Cnb::CnbWriter::SetLimits", "CNA::Content::Cnb::CnbWriter::Build",
             "CNA::Content::Cnb::CheckedAdd", "CNA::Content::Cnb::CheckedMultiply",
             "CNA::Content::Cnb::CnbByteWriter", "CNA::Content::Cnb::CnbByteWriter::WriteU8",
             "CNA::Content::Cnb::CnbByteWriter::WriteU16", "CNA::Content::Cnb::CnbByteWriter::WriteU32",
             "CNA::Content::Cnb::CnbByteWriter::WriteU64", "CNA::Content::Cnb::CnbByteWriter::WriteI32",
             "CNA::Content::Cnb::CnbByteWriter::WriteF32", "CNA::Content::Cnb::CnbByteWriter::WriteF64",
             "CNA::Content::Cnb::CnbByteWriter::WriteString", "CNA::Content::Cnb::CnbByteWriter::WriteBytes",
             "CNA::Content::Cnb::CnbByteWriter::WriteZeros", "CNA::Content::Cnb::CnbByteWriter::Size",
             "CNA::Content::Cnb::CnbByteWriter::View", "CNA::Content::Cnb::CnbByteWriter::Take",
             "CNA::Content::Cnb::CnbByteReader", "CNA::Content::Cnb::CnbByteReader::Remaining",
             "CNA::Content::Cnb::CnbByteReader::Position", "CNA::Content::Cnb::CnbByteReader::Size",
             "CNA::Content::Cnb::CnbByteReader::Context", "CNA::Content::Cnb::CnbByteReader::ReadU8",
             "CNA::Content::Cnb::CnbByteReader::ReadU16", "CNA::Content::Cnb::CnbByteReader::ReadU32",
             "CNA::Content::Cnb::CnbByteReader::ReadU64", "CNA::Content::Cnb::CnbByteReader::ReadI32",
             "CNA::Content::Cnb::CnbByteReader::ReadF32", "CNA::Content::Cnb::CnbByteReader::ReadF64",
             "CNA::Content::Cnb::CnbByteReader::ReadBytes", "CNA::Content::Cnb::CnbByteReader::Skip",
             "CNA::Content::Cnb::CnbByteReader::Fail", "CNA::Content::Cnb::CnbByteReader::IsWellFormedUtf8",
             "CNA::Content::Cnb::Crc32c", "CNA::Content::Cnb::Crc32cContinue",
             "CNA::Content::Cnb::Crc32cSeed", "CNA::Content::Cnb::Crc32cUsesHardwareEXT",
             "CNA::Content::Cnb::Crc32cPortableEXT",
             "CNA::Content::Cnb::CnbWriter::SetMetadata", "CNA::Content::Cnb::CnbWriter::SetExternalReferences",
             "CNA::Content::Cnb::CnbWriter::AddChunk",
             "CNA::Content::Cnb::CnbWriter::SchemaChunkCount", "CNA::Content::Cnb::CnbWriter::SetCompression",
             "CNA::Content::Cnb::CnbWriter::Limits", "CNA::Content::Cnb::CnbReadLimits",
             "CNA::Content::Cnb::IsCnbCompressionSupported", "CNA::Content::Cnb::CnbCompressionToString",
             "CNA::Content::Cnb::CompressCnbChunk", "CNA::Content::Cnb::DecompressCnbChunk",
             "CNA::Content::Cnb::CnbChunkEntry", "CNA::Content::Cnb::CnbChunkEntry::IsMandatory",
             "CNA::Content::Cnb::CnbExternalReference", "CNA::Content::Cnb::CnbMetadata",
             "CNA::Content::Cnb::CnbDocument", "CNA::Content::Cnb::CnbWriter",
             "CNA::Content::Cnb::CnbDocument::ChunkData",
             "CNA::Content::Cnb::CnbDocument::Parse", "CNA::Content::Cnb::CnbDocument::ChunkCount",
             "CNA::Content::Cnb::CnbDocument::ChunkAt", "CNA::Content::Cnb::CnbDocument::OpenChunk",
             "CNA::Content::Cnb::CnbDocument::HasMagic", "CNA::Content::Cnb::CnbDocument::Origin",
             "CNA::Content::Cnb::CnbDocument::ContainerMajor", "CNA::Content::Cnb::CnbDocument::ContainerMinor",
             "CNA::Content::Cnb::CnbDocument::AssetTypeId", "CNA::Content::Cnb::CnbDocument::AssetSchemaVersion",
             "CNA::Content::Cnb::CnbDocument::FindAll", "CNA::Content::Cnb::CnbDocument::FindSingle",
             "CNA::Content::Cnb::CnbDocument::RequireSingle", "CNA::Content::Cnb::CnbDocument::Metadata",
             "CNA::Content::Cnb::CnbDocument::ExternalReferences",
             "CNA::Content::Cnb::CnbDocument::ExternalReferenceAt",
             "CNA::Content::Cnb::CnbDocument::RequireAsset", "CNA::Content::Cnb::CnbDocument::Limits",
             "CNA::Content::Cnb::CnbByteReader::ReadCount",
             "CNA::Content::Cnb::CnbByteReader::ReadString", "CNA::Content::Cnb::DefaultCnbReadLimits"},
    .tags = {"content", "custom", "cnb", "limits", "compression", "validation", "cnaext"},
    .restrictions = "Small in-memory fixtures; compressed checks require runtime Zstandard support. Structural byte mutation is limited to checksum corruption; no timing or allocation profiling.",
})
