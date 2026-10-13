/* SPDX-License-Identifier: MS-PL */
/* Deterministic CNB container assembly, parsing and ownership through C. */
#include <CNA/C/cna.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static const char TemporaryPath[] = "/tmp/cna-demos-cnb-document-roundtrip.cnb";

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t value)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static int release_reader(CNA_CnbReaderHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_reader_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_writer(CNA_CnbWriterHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_writer_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const uint8_t BodyFirst[] = {9U, 8U, 7U};
    static const uint8_t BodySecond[] = {6U, 5U};
    static const uint8_t Garbage[] = {1U, 2U, 3U, 4U, 5U, 6U};
    CnaDemo demo;
    CNA_CnbWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_CnbWriterHandle stale_writer = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle file_document = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle broken = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle stale_document = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle chunk_reader = CNA_INVALID_HANDLE;
    CNA_CnbChunkId head = UINT32_C(0);
    CNA_CnbChunkId body = UINT32_C(0);
    CNA_CnbChunkId data_id = UINT32_C(0);
    CNA_CnbChunkId absent = UINT32_C(0);
    CNA_CnbExternalReference reference;
    CNA_CnbMetadata metadata;
    CNA_CnbChunkEntry entry;
    CNA_CnbReadLimits limits;
    uint8_t head_bytes[4] = {1U, 2U, 3U, 4U};
    uint8_t repeated[1024];
    uint8_t image[8192];
    uint8_t second_image[8192];
    uint8_t output[1100];
    uint8_t short_output[8192];
    char text[128];
    uint64_t indices[4] = {UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX};
    uint64_t image_size = UINT64_C(0);
    uint64_t second_size = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint64_t head_index = UINT64_MAX;
    uint64_t data_index = UINT64_MAX;
    uint64_t untouched_index = UINT64_C(0x1122334455667788);
    uint32_t number = UINT32_C(0);
    uint16_t major = UINT16_C(0);
    uint16_t minor = UINT16_C(0);
    uint8_t first = UINT8_C(0);
    CNA_Bool found = CNA_FALSE;
    CNA_Bool zstd_supported = CNA_FALSE;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_document", argc, argv, 1);
    (void)remove(TemporaryPath);
    memset(repeated, 0x4A, sizeof(repeated));

    if (!cna_demo_check(
            &demo,
            cna_cnb_make_chunk_id('H', 'E', 'A', 'D', &head) == CNA_RESULT_SUCCESS &&
                cna_cnb_make_chunk_id('B', 'O', 'D', 'Y', &body) == CNA_RESULT_SUCCESS &&
                cna_cnb_make_chunk_id('D', 'A', 'T', 'A', &data_id) == CNA_RESULT_SUCCESS &&
                cna_cnb_make_chunk_id('N', 'O', 'P', 'E', &absent) == CNA_RESULT_SUCCESS,
            "create the schema's printable chunk identities")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo, cna_cnb_writer_create(CNA_CNB_ASSET_TYPE_CURVE, UINT32_C(1), &writer),
            "create a version-one Curve document writer")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_writer_get_schema_chunk_count(writer, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "a new writer contains no schema chunks");
    cna_demo_check_result(
        &demo,
        cna_cnb_writer_set_metadata(
            writer, text_view("Microsoft.Xna.Framework.Curve"), text_view("curves/wobble")),
        "set canonical type and logical content metadata");

    memset(&reference, 0, sizeof(reference));
    reference.struct_size = (uint32_t)sizeof(reference);
    reference.struct_version = CNA_CNB_EXTERNAL_REFERENCE_STRUCT_VERSION;
    reference.expected_asset_type_id = CNA_CNB_ASSET_TYPE_TEXTURE2D;
    cna_demo_check(
        &demo,
        cna_cnb_writer_add_external_reference(
            writer, &reference, text_view("textures/old.png")) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_clear_external_references(writer) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_add_external_reference(
                writer, &reference, text_view("textures/hero.png")) == CNA_RESULT_SUCCESS,
        "replace the ordered external-reference table through clear and append");

    if (!cna_demo_check(
            &demo,
            cna_cnb_writer_add_chunk(
                writer, head, head_bytes, sizeof(head_bytes),
                CNA_CNB_CHUNK_FLAG_MANDATORY, UINT32_C(4)) == CNA_RESULT_SUCCESS &&
                cna_cnb_writer_add_chunk(
                    writer, body, BodyFirst, sizeof(BodyFirst), CNA_CNB_CHUNK_FLAG_NONE,
                    UINT32_C(16)) == CNA_RESULT_SUCCESS &&
                cna_cnb_writer_add_chunk(
                    writer, body, BodySecond, sizeof(BodySecond), CNA_CNB_CHUNK_FLAG_NONE,
                    UINT32_C(2)) == CNA_RESULT_SUCCESS &&
                cna_cnb_writer_add_chunk(
                    writer, data_id, repeated, sizeof(repeated), CNA_CNB_CHUNK_FLAG_NONE,
                    UINT32_C(8)) == CNA_RESULT_SUCCESS,
            "append copied mandatory, repeated and compressible schema chunks")) {
        goto cleanup;
    }
    head_bytes[0] = 0xFFU;
    memset(repeated, 0, sizeof(repeated));
    cna_demo_check(
        &demo,
        cna_cnb_writer_add_chunk(
            writer, CNA_CNB_CONTAINER_CHUNK_METADATA, BodyFirst, sizeof(BodyFirst),
            CNA_CNB_CHUNK_FLAG_NONE, UINT32_C(4)) == CNA_RESULT_IO &&
            cna_cnb_writer_get_schema_chunk_count(writer, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(4),
        "container-owned chunk identifiers are refused without changing schema state");

    memset(&limits, 0, sizeof(limits));
    limits.struct_size = (uint32_t)sizeof(limits);
    limits.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    if (!cna_demo_check_result(
            &demo, cna_cnb_writer_get_limits(writer, &limits),
            "read the writer's default parse-compatible limits")) {
        goto cleanup;
    }
    limits.max_file_size = UINT64_C(64);
    cna_demo_check(
        &demo,
        cna_cnb_writer_set_limits(writer, &limits) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_build(writer, image, sizeof(image), &required) == CNA_RESULT_IO,
        "writer limits reject a document that its paired reader would refuse");
    cna_demo_check(
        &demo,
        cna_cnb_read_limits_init(&limits) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_set_limits(writer, &limits) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_get_limits(writer, &limits) == CNA_RESULT_SUCCESS &&
            limits.max_file_size == UINT64_C(512) * UINT64_C(1024) * UINT64_C(1024),
        "restoring default writer limits restores the published file ceiling");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_is_compression_supported(CNA_CNB_COMPRESSION_ZSTD, &zstd_supported),
            "query whether this CNA build can write Zstandard chunks")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_writer_set_compression(writer, CNA_CNB_COMPRESSION_ZSTD, INT32_C(3)) ==
            (zstd_supported == CNA_TRUE ? CNA_RESULT_SUCCESS : CNA_RESULT_NOT_SUPPORTED),
        "select Zstandard exactly when the linked build implements it");
    if (zstd_supported != CNA_TRUE) {
        cna_demo_check_result(
            &demo,
            cna_cnb_writer_set_compression(writer, CNA_CNB_COMPRESSION_NONE, INT32_C(3)),
            "retain stored chunks when Zstandard is unavailable");
    }

    if (!cna_demo_check(
            &demo,
            cna_cnb_writer_build(writer, NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > CNA_CNB_FORMAT_HEADER_SIZE && image_size < sizeof(image),
            "the non-destructive sizing build reports the exact required image size")) {
        goto cleanup;
    }
    memset(short_output, 0xA5, sizeof(short_output));
    cna_demo_check(
        &demo,
        cna_cnb_writer_build(writer, short_output, image_size - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == image_size && bytes_are(short_output, sizeof(short_output), 0xA5U),
        "a short document destination is reported without a partial write");
    if (!cna_demo_check(
            &demo,
            cna_cnb_writer_build(writer, image, sizeof(image), &required) == CNA_RESULT_SUCCESS &&
                required == image_size &&
                cna_cnb_writer_build(writer, second_image, sizeof(second_image), &second_size) ==
                    CNA_RESULT_SUCCESS &&
                second_size == image_size &&
                memcmp(image, second_image, (size_t)image_size) == 0,
            "repeated builds produce the same complete byte image")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_writer_write_to_file(writer, text_view(TemporaryPath)) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_write_to_file(writer, text_view("")) == CNA_RESULT_INVALID_ARGUMENT,
        "the writer persists the image and refuses an empty path");
    stale_writer = writer;
    cna_demo_check_result(&demo, cna_cnb_writer_destroy(writer), "release the document writer");
    writer = CNA_INVALID_HANDLE;
    count = UINT64_C(0xA5A5A5A5A5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_writer_get_schema_chunk_count(stale_writer, &count) == CNA_RESULT_INVALID_HANDLE,
        "the released writer rejects stale access");

    cna_demo_check(
        &demo,
        cna_cnb_document_parse_file(text_view(TemporaryPath), NULL, &file_document) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_document_get_chunk_count(file_document, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(6),
        "the filesystem path parses the same six-entry document");
    cna_demo_check_result(
        &demo, cna_cnb_document_destroy(file_document), "release the file-parsed document");
    file_document = CNA_INVALID_HANDLE;
    broken = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_document_parse_file(
            text_view("/tmp/cna-demos-cnb-document-absent.cnb"), NULL, &broken) ==
                CNA_RESULT_IO &&
            broken == CNA_INVALID_HANDLE,
        "an absent document path is an IO failure without a partial handle");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_document_parse(
                image, image_size, text_view("memory-roundtrip.cnb"), NULL, &document),
            "parse and fully validate the caller-owned byte image")) {
        goto cleanup;
    }
    memset(image, 0, (size_t)image_size);
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_document_get_origin_size(document, &required) == CNA_RESULT_SUCCESS &&
            required == strlen("memory-roundtrip.cnb") &&
            cna_cnb_document_copy_origin(document, text, required - UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == required && bytes_are((const uint8_t*)text, sizeof(text), 0xA5U) &&
            cna_cnb_document_copy_origin(document, text, sizeof(text), &count) ==
                CNA_RESULT_SUCCESS &&
            count == required && memcmp(text, "memory-roundtrip.cnb", (size_t)count) == 0,
        "the document owns its image and copies its exact origin atomically");
    cna_demo_check(
        &demo,
        cna_cnb_document_get_container_major(document, &major) == CNA_RESULT_SUCCESS &&
            major == (uint16_t)CNA_CNB_FORMAT_CONTAINER_MAJOR &&
            cna_cnb_document_get_container_minor(document, &minor) == CNA_RESULT_SUCCESS &&
            minor == (uint16_t)CNA_CNB_FORMAT_CONTAINER_MINOR &&
            cna_cnb_document_get_asset_type_id(document, &number) == CNA_RESULT_SUCCESS &&
            number == CNA_CNB_ASSET_TYPE_CURVE &&
            cna_cnb_document_get_asset_schema_version(document, &number) == CNA_RESULT_SUCCESS &&
            number == UINT32_C(1),
        "container and asset versions survive the writer-to-document round trip");
    cna_demo_check(
        &demo,
        cna_cnb_document_get_chunk_count(document, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(6),
        "metadata, references and four schema chunks form six table entries");

    if (!cna_demo_check_result(
            &demo, cna_cnb_document_require_single(document, head, &head_index),
            "locate the one mandatory header chunk by type")) {
        goto cleanup;
    }
    memset(&entry, 0, sizeof(entry));
    entry.struct_size = (uint32_t)sizeof(entry);
    entry.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_chunk(document, head_index, &entry) == CNA_RESULT_SUCCESS &&
            entry.type == head && entry.stored_size == UINT64_C(4) &&
            entry.uncompressed_size == UINT64_C(4) &&
            entry.compression == CNA_CNB_COMPRESSION_NONE &&
            entry.flags == CNA_CNB_CHUNK_FLAG_MANDATORY && entry.alignment == UINT32_C(4) &&
            entry.reserved == UINT32_C(0) && (entry.offset % UINT64_C(4)) == UINT64_C(0),
        "the header table entry exposes exact size, flags, codec and alignment");
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_document_copy_chunk_data(
            document, head_index, output, UINT64_C(3), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == UINT64_C(4) && bytes_are(output, sizeof(output), 0xA5U) &&
            cna_cnb_document_copy_chunk_data(
                document, head_index, output, sizeof(output), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(4) && output[0] == 1U && output[1] == 2U && output[2] == 3U &&
            output[3] == 4U,
        "logical chunk copying is atomic and proves the writer copied its source bytes");
    cna_demo_check_result(
        &demo, cna_cnb_document_open_chunk(document, head_index, &chunk_reader),
        "open a bounded reader that borrows the document's chunk bytes");
    cna_demo_check(
        &demo,
        cna_cnb_document_destroy(document) == CNA_RESULT_INVALID_STATE &&
            cna_cnb_reader_read_u8(chunk_reader, &first) == CNA_RESULT_SUCCESS && first == 1U,
        "a live borrowed reader prevents document destruction and remains readable");
    cna_demo_check_result(
        &demo, cna_cnb_reader_destroy(chunk_reader), "release the borrowed chunk reader");
    chunk_reader = CNA_INVALID_HANDLE;

    memset(indices, 0xA5, sizeof(indices));
    cna_demo_check(
        &demo,
        cna_cnb_document_find_all(document, body, indices, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) && bytes_are((const uint8_t*)indices, sizeof(indices), 0xA5U) &&
            cna_cnb_document_find_all(document, body, indices, UINT64_C(4), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && indices[0] < indices[1],
        "multi-chunk lookup preserves a short destination and returns file order");
    found = CNA_TRUE;
    untouched_index = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_document_find_single(document, absent, &found, &untouched_index) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && untouched_index == UINT64_C(0x1122334455667788) &&
            cna_cnb_document_find_single(document, head, &found, &head_index) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_TRUE,
        "single lookup distinguishes ordinary absence from one exact match");
    untouched_index = UINT64_C(0x8877665544332211);
    cna_demo_check(
        &demo,
        cna_cnb_document_require_single(document, absent, &untouched_index) == CNA_RESULT_IO &&
            untouched_index == UINT64_C(0x8877665544332211),
        "required lookup rejects an absent chunk without inventing an index");
    {
        const CNA_CnbChunkId known_all[] = {head, body, data_id};
        const CNA_CnbChunkId known_optional[] = {body, data_id};
        cna_demo_check(
            &demo,
            cna_cnb_document_require_mandatory_chunks_understood(
                document, known_all, UINT64_C(3)) == CNA_RESULT_SUCCESS &&
                cna_cnb_document_require_mandatory_chunks_understood(
                    document, known_optional, UINT64_C(2)) == CNA_RESULT_IO,
            "mandatory-chunk enforcement accepts known types and rejects an unknown requirement");
    }

    memset(&metadata, 0, sizeof(metadata));
    metadata.struct_size = (uint32_t)sizeof(metadata);
    metadata.struct_version = CNA_CNB_METADATA_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_metadata(document, &metadata) == CNA_RESULT_SUCCESS &&
            metadata.present == CNA_TRUE && metadata.flags == UINT32_C(0) &&
            bytes_are(metadata.reserved, sizeof(metadata.reserved), 0U),
        "the parsed metadata record is present, versioned and reserved-zero");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_document_get_metadata_asset_type_name_size(document, &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(29) &&
            cna_cnb_document_copy_metadata_asset_type_name(
                document, text, required - UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == required && bytes_are((const uint8_t*)text, sizeof(text), 0xA5U) &&
            cna_cnb_document_copy_metadata_asset_type_name(
                document, text, sizeof(text), &count) == CNA_RESULT_SUCCESS &&
            count == required && memcmp(text, "Microsoft.Xna.Framework.Curve", 29U) == 0,
        "the canonical metadata type name has exact atomic transfer semantics");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_document_get_metadata_content_name_size(document, &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(13) &&
            cna_cnb_document_copy_metadata_content_name(
                document, text, sizeof(text), &count) == CNA_RESULT_SUCCESS &&
            count == required && memcmp(text, "curves/wobble", 13U) == 0,
        "the logical content name round-trips independently of the type identity");

    cna_demo_check(
        &demo,
        cna_cnb_document_get_external_reference_count(document, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "only the post-clear external reference is serialized");
    memset(&reference, 0, sizeof(reference));
    reference.struct_size = (uint32_t)sizeof(reference);
    reference.struct_version = CNA_CNB_EXTERNAL_REFERENCE_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_external_reference(
            document, UINT64_C(0), text_view("curve texture"), &reference) ==
                CNA_RESULT_SUCCESS &&
            reference.flags == UINT32_C(0) &&
            reference.expected_asset_type_id == CNA_CNB_ASSET_TYPE_TEXTURE2D,
        "the external reference retains its expected asset type and flags");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_document_get_external_reference_name_size(
            document, UINT64_C(0), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(17) &&
            cna_cnb_document_copy_external_reference_name(
                document, UINT64_C(0), text, required - UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == required && bytes_are((const uint8_t*)text, sizeof(text), 0xA5U) &&
            cna_cnb_document_copy_external_reference_name(
                document, UINT64_C(0), text, sizeof(text), &count) == CNA_RESULT_SUCCESS &&
            count == required && memcmp(text, "textures/hero.png", 17U) == 0,
        "the exact logical reference name copies atomically");
    cna_demo_check(
        &demo,
        cna_cnb_document_require_asset(
            document, CNA_CNB_ASSET_TYPE_CURVE, UINT32_C(1)) == CNA_RESULT_SUCCESS &&
            cna_cnb_document_require_asset(
                document, CNA_CNB_ASSET_TYPE_MODEL, UINT32_C(1)) == CNA_RESULT_IO &&
            cna_cnb_document_require_asset(
                document, CNA_CNB_ASSET_TYPE_CURVE, UINT32_C(0)) == CNA_RESULT_IO,
        "asset requirements distinguish the exact type and supported schema range");
    memset(&limits, 0, sizeof(limits));
    limits.struct_size = (uint32_t)sizeof(limits);
    limits.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_limits(document, &limits) == CNA_RESULT_SUCCESS &&
            limits.max_file_size == UINT64_C(512) * UINT64_C(1024) * UINT64_C(1024) &&
            limits.max_chunk_alignment == UINT32_C(4096),
        "the document reports the default limits that admitted it");

    cna_demo_check_result(
        &demo, cna_cnb_document_require_single(document, data_id, &data_index),
        "locate the compressible data chunk by schema identity");
    memset(&entry, 0, sizeof(entry));
    entry.struct_size = (uint32_t)sizeof(entry);
    entry.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    memset(output, 0, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_document_get_chunk(document, data_index, &entry) == CNA_RESULT_SUCCESS &&
            entry.uncompressed_size == UINT64_C(1024) &&
            entry.compression == (zstd_supported == CNA_TRUE ? CNA_CNB_COMPRESSION_ZSTD
                                                             : CNA_CNB_COMPRESSION_NONE) &&
            (zstd_supported != CNA_TRUE || entry.stored_size < entry.uncompressed_size) &&
            cna_cnb_document_copy_chunk_data(
                document, data_index, output, sizeof(output), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(1024) && bytes_are(output, 1024U, 0x4AU),
        "document access restores the writer-owned logical bytes across optional compression");
    memset(&entry, 0xA5, sizeof(entry));
    entry.struct_size = (uint32_t)sizeof(entry);
    entry.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_document_get_chunk(document, UINT64_C(99), &entry) ==
            CNA_RESULT_INVALID_ARGUMENT,
        "a caller-supplied chunk index outside the table is an argument error");

    stale_document = document;
    cna_demo_check_result(&demo, cna_cnb_document_destroy(document), "release the parsed document");
    document = CNA_INVALID_HANDLE;
    count = UINT64_C(0xA5A5A5A5A5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_document_get_chunk_count(stale_document, &count) == CNA_RESULT_INVALID_HANDLE,
        "the released document rejects stale access");
    broken = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_document_parse(
            second_image, image_size - UINT64_C(1), text_view("truncated"), NULL, &broken) ==
                CNA_RESULT_IO &&
            broken == CNA_INVALID_HANDLE,
        "a truncated image is rejected without publishing a partial document");
    broken = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_document_parse(
            Garbage, sizeof(Garbage), text_view("garbage"), NULL, &broken) == CNA_RESULT_IO &&
            broken == CNA_INVALID_HANDLE,
        "non-container bytes are an IO failure without a partial document");

cleanup:
    releases_ok &= release_reader(&chunk_reader);
    releases_ok &= release_document(&file_document);
    releases_ok &= release_document(&broken);
    releases_ok &= release_document(&document);
    releases_ok &= release_writer(&writer);
    if (remove(TemporaryPath) != 0 && errno != ENOENT) {
        releases_ok = 0;
    }
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining document object and remove the temporary file");
    return cna_demo_finish(&demo);
}
