/* SPDX-License-Identifier: MS-PL */
/* CNB wire identities, read limits, overflow guards, CRC-32C and compression through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef CNA_Result (*NameSizeRoute)(uint32_t, uint64_t*);
typedef CNA_Result (*NameCopyRoute)(uint32_t, char*, uint64_t, uint64_t*);

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

static int name_is(
    uint32_t value, NameSizeRoute size_route, NameCopyRoute copy_route,
    const char* expected)
{
    char text[64];
    char short_text[64];
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    const uint64_t expected_bytes = (uint64_t)strlen(expected);
    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    return expected_bytes > UINT64_C(0) && expected_bytes < sizeof(text) &&
        size_route(value, &bytes) == CNA_RESULT_SUCCESS && bytes == expected_bytes &&
        copy_route(value, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
        copied == bytes && memcmp(text, expected, (size_t)bytes) == 0 &&
        copy_route(value, short_text, bytes - UINT64_C(1), &copied) ==
            CNA_RESULT_BUFFER_TOO_SMALL &&
        copied == bytes && bytes_are((const uint8_t*)short_text, sizeof(short_text), 0xA5U);
}

int main(int argc, char** argv)
{
    static const uint8_t CheckVector[] = "123456789";
    static const uint8_t Payload[] =
        "cnb chunk payload, repeated so a codec has something to find; "
        "cnb chunk payload, repeated so a codec has something to find; "
        "cnb chunk payload, repeated so a codec has something to find.";
    const uint64_t payload_bytes = (uint64_t)sizeof(Payload) - UINT64_C(1);
    CnaDemo demo;
    CNA_CnbReadLimits limits;
    CNA_CnbReadLimits invalid_limits;
    CNA_CnbChunkEntry entry;
    CNA_CnbChunkId chunk = UINT32_C(0);
    CNA_Bool value = CNA_FALSE;
    CNA_Bool zstd_supported = CNA_FALSE;
    uint8_t magic[4];
    uint8_t output[512];
    uint8_t compressed[512];
    char chunk_text[4];
    char chunk_short[4];
    char problem[128];
    char problem_short[128];
    uint64_t required = UINT64_C(0);
    uint64_t compressed_bytes = UINT64_C(0);
    uint64_t arithmetic = UINT64_C(0);
    uint32_t checksum = UINT32_C(0);
    uint32_t continued = UINT32_C(0);
    uint32_t portable = UINT32_C(0);
    uint32_t custom_type = UINT32_C(0);
    uint32_t other_type = UINT32_C(0);
    const char malformed_bytes[3] = {'a', (char)0xFFU, 'b'};
    const CNA_StringView malformed = {malformed_bytes, UINT64_C(3)};

    cna_demo_init(&demo, "c_api.content.cnb_foundations", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_CNB_FORMAT_MAGIC_SIZE == UINT32_C(4) &&
            CNA_CNB_FORMAT_HEADER_SIZE == UINT32_C(64) &&
            CNA_CNB_FORMAT_TOC_ENTRY_SIZE == UINT32_C(48) &&
            CNA_CNB_FORMAT_HEADER_CHECKSUM_COVERAGE == CNA_CNB_FORMAT_HEADER_CHECKSUM_OFFSET &&
            CNA_CNB_FORMAT_HEADER_CHECKSUM_OFFSET + CNA_CNB_FORMAT_HEADER_RESERVED_SIZE +
                    sizeof(uint32_t) ==
                CNA_CNB_FORMAT_HEADER_SIZE &&
            CNA_CNB_FORMAT_DEFAULT_TOC_OFFSET == CNA_CNB_FORMAT_HEADER_SIZE &&
            CNA_CNB_FORMAT_CONTAINER_MAJOR == UINT32_C(1) &&
            CNA_CNB_FORMAT_CONTAINER_MINOR == UINT32_C(0),
        "the public constants describe the fixed CNB v1 header and table layout");

    memset(&limits, 0, sizeof(limits));
    limits.struct_size = (uint32_t)sizeof(limits);
    limits.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_read_limits_init(&limits) == CNA_RESULT_SUCCESS &&
            limits.max_file_size == UINT64_C(512) * UINT64_C(1024 * 1024) &&
            limits.max_chunk_size == UINT64_C(384) * UINT64_C(1024 * 1024) &&
            limits.max_total_uncompressed_size == UINT64_C(1024) * UINT64_C(1024 * 1024) &&
            limits.max_chunk_count == UINT32_C(65536) &&
            limits.max_string_bytes == UINT32_C(1024 * 1024) &&
            limits.max_array_element_count == UINT32_C(16 * 1024 * 1024) &&
            limits.max_chunk_alignment == UINT32_C(4096) &&
            limits.max_chunk_size < limits.max_file_size &&
            limits.max_total_uncompressed_size > limits.max_file_size,
        "versioned default limits bound each allocation while permitting compressed expansion");
    memset(&invalid_limits, 0, sizeof(invalid_limits));
    invalid_limits.struct_size = (uint32_t)sizeof(invalid_limits) - UINT32_C(1);
    invalid_limits.struct_version = CNA_CNB_READ_LIMITS_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_read_limits_init(&invalid_limits) == CNA_RESULT_INVALID_ARGUMENT &&
            invalid_limits.max_file_size == UINT64_C(0),
        "a short read-limits version is refused before changing its payload");

    memset(magic, 0xA5, sizeof(magic));
    cna_demo_check(
        &demo,
        cna_cnb_copy_format_magic(magic, UINT64_C(3), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == UINT64_C(4) && bytes_are(magic, sizeof(magic), 0xA5U),
        "a short magic destination reports four bytes without a partial write");
    cna_demo_check(
        &demo,
        cna_cnb_copy_format_magic(magic, sizeof(magic), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(4) && magic[0] == 0x43U && magic[1] == 0x4EU &&
            magic[2] == 0x42U && magic[3] == 0x1AU &&
            cna_cnb_has_magic(magic, sizeof(magic), &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_cnb_has_magic(magic, UINT64_C(3), &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "the exact CNB magic is recognized while a short prefix is ordinary non-magic");

    cna_demo_check(
        &demo,
        cna_cnb_make_chunk_id('C', 'M', 'E', 'T', &chunk) == CNA_RESULT_SUCCESS &&
            chunk == CNA_CNB_CONTAINER_CHUNK_METADATA &&
            (chunk & UINT32_C(0xFF)) == (uint32_t)'C' &&
            ((chunk >> 24) & UINT32_C(0xFF)) == (uint32_t)'T' &&
            cna_cnb_get_chunk_id_string_size(chunk, &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(4),
        "CMET packs little-endian and publishes its exact diagnostic length");
    memset(chunk_text, 0, sizeof(chunk_text));
    memset(chunk_short, 0xA5, sizeof(chunk_short));
    cna_demo_check(
        &demo,
        cna_cnb_copy_chunk_id_string(chunk, chunk_short, UINT64_C(3), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == UINT64_C(4) &&
            bytes_are((const uint8_t*)chunk_short, sizeof(chunk_short), 0xA5U) &&
            cna_cnb_copy_chunk_id_string(chunk, chunk_text, sizeof(chunk_text), &required) ==
                CNA_RESULT_SUCCESS &&
            memcmp(chunk_text, "CMET", 4U) == 0 &&
            cna_cnb_is_well_formed_chunk_id(chunk, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE,
        "a printable chunk ID renders exactly and copies atomically");
    cna_cnb_make_chunk_id(0x01U, 'M', 'E', 0x7FU, &chunk);
    cna_demo_check(
        &demo,
        cna_cnb_is_well_formed_chunk_id(chunk, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_cnb_copy_chunk_id_string(chunk, chunk_text, sizeof(chunk_text), &required) ==
                CNA_RESULT_SUCCESS &&
            memcmp(chunk_text, "?ME?", 4U) == 0,
        "nonprintable chunk bytes are rejected as identities and sanitized for diagnostics");
    memset(&entry, 0, sizeof(entry));
    entry.struct_size = (uint32_t)sizeof(entry);
    entry.struct_version = CNA_CNB_CHUNK_ENTRY_STRUCT_VERSION;
    entry.flags = CNA_CNB_CHUNK_FLAG_MANDATORY;
    cna_demo_check(
        &demo,
        cna_cnb_chunk_entry_is_mandatory(&entry, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            (entry.flags = CNA_CNB_CHUNK_FLAG_NONE,
             cna_cnb_chunk_entry_is_mandatory(&entry, &value) == CNA_RESULT_SUCCESS) &&
            value == CNA_FALSE,
        "the versioned chunk entry observes its sole frozen mandatory flag");

    cna_demo_check(
        &demo,
        name_is(CNA_CNB_ASSET_TYPE_TEXTURE2D, cna_cnb_get_asset_type_name_size,
                cna_cnb_copy_asset_type_name, "Texture2D") &&
            name_is(CNA_CNB_ASSET_TYPE_EFFECT, cna_cnb_get_asset_type_name_size,
                    cna_cnb_copy_asset_type_name, "Effect") &&
            name_is(CNA_CNB_ASSET_TYPE_RESERVED_RANGE_FIRST,
                    cna_cnb_get_asset_type_name_size, cna_cnb_copy_asset_type_name,
                    "unknown type 0x40000000") &&
            name_is(CNA_CNB_ASSET_TYPE_CUSTOM_RANGE_FIRST,
                    cna_cnb_get_asset_type_name_size, cna_cnb_copy_asset_type_name,
                    "custom type 0x80000000"),
        "built-in, reserved and custom asset identities have exact atomic diagnostic names");
    cna_demo_check(
        &demo,
        cna_cnb_asset_type_id_from_name(text_view("MyGame.Level"), &custom_type) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_asset_type_id_from_name(text_view("MyGame.Other"), &other_type) ==
                CNA_RESULT_SUCCESS &&
            custom_type != other_type && custom_type >= CNA_CNB_ASSET_TYPE_CUSTOM_RANGE_FIRST &&
            other_type >= CNA_CNB_ASSET_TYPE_CUSTOM_RANGE_FIRST &&
            cna_cnb_is_custom_asset_type_id(custom_type, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_cnb_is_custom_asset_type_id(CNA_CNB_ASSET_TYPE_MODEL, &value) ==
                CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "distinct canonical names deterministically mint values in the custom asset range");
    other_type = UINT32_C(0xA5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_asset_type_id_from_name(text_view(""), &other_type) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            other_type == UINT32_C(0xA5A5A5A5) &&
            cna_cnb_asset_type_id_from_name(malformed, &other_type) == CNA_RESULT_ENCODING &&
            other_type == UINT32_C(0xA5A5A5A5),
        "empty and malformed custom type names are refused without changing output");

    cna_demo_check(
        &demo,
        cna_cnb_get_logical_name_problem_size(
            text_view("Textures/hero.png"), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(0) &&
            cna_cnb_copy_logical_name_problem(
                text_view("Textures/hero.png"), NULL, UINT64_C(0), &required) ==
                CNA_RESULT_SUCCESS &&
            required == UINT64_C(0),
        "a relative slash-separated UTF-8 logical name has no reported problem");
    memset(problem, 0, sizeof(problem));
    memset(problem_short, 0xA5, sizeof(problem_short));
    cna_demo_check(
        &demo,
        cna_cnb_get_logical_name_problem_size(text_view("../escape"), &required) ==
                CNA_RESULT_SUCCESS &&
            required > UINT64_C(0) && required < sizeof(problem) &&
            cna_cnb_copy_logical_name_problem(
                text_view("../escape"), problem_short, required - UINT64_C(1), &arithmetic) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            arithmetic == required &&
            bytes_are((const uint8_t*)problem_short, sizeof(problem_short), 0xA5U) &&
            cna_cnb_copy_logical_name_problem(
                text_view("../escape"), problem, sizeof(problem), &arithmetic) ==
                CNA_RESULT_SUCCESS &&
            arithmetic == required,
        "parent traversal produces an exact, atomically copied diagnostic");
    cna_demo_check(
        &demo,
        cna_cnb_get_logical_name_problem_size(malformed, &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(24) &&
            cna_cnb_copy_logical_name_problem(
                malformed, problem, sizeof(problem), &required) == CNA_RESULT_SUCCESS &&
            memcmp(problem, "is not well-formed UTF-8", 24U) == 0,
        "malformed UTF-8 is deliberately returned as a logical-name verdict");

    arithmetic = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_checked_add(UINT64_C(2), UINT64_C(3), &arithmetic) == CNA_RESULT_SUCCESS &&
            arithmetic == UINT64_C(5) &&
            cna_cnb_checked_multiply(UINT64_C(6), UINT64_C(7), &arithmetic) ==
                CNA_RESULT_SUCCESS &&
            arithmetic == UINT64_C(42),
        "checked arithmetic returns exact representable file offsets and byte counts");
    arithmetic = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_checked_add(UINT64_MAX, UINT64_C(1), &arithmetic) == CNA_RESULT_OVERFLOW &&
            arithmetic == UINT64_C(0x1122334455667788) &&
            cna_cnb_checked_multiply(UINT64_MAX, UINT64_C(2), &arithmetic) ==
                CNA_RESULT_OVERFLOW &&
            arithmetic == UINT64_C(0x1122334455667788),
        "addition and multiplication reject overflow without clobbering the prior result");

    cna_demo_check(
        &demo,
        cna_cnb_crc32c(CheckVector, UINT64_C(9), &checksum) == CNA_RESULT_SUCCESS &&
            checksum == UINT32_C(0xE3069283) &&
            cna_cnb_crc32c_continue(
                CNA_CNB_CRC32C_SEED, CheckVector, UINT64_C(5), &continued) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_crc32c_continue(
                continued, CheckVector + 5, UINT64_C(4), &continued) == CNA_RESULT_SUCCESS &&
            continued == checksum,
        "one-shot and split CRC-32C match the published iSCSI check vector");
    cna_demo_check(
        &demo,
        cna_cnb_crc32c_portable(CheckVector, UINT64_C(9), &portable) == CNA_RESULT_SUCCESS &&
            portable == checksum &&
            cna_cnb_crc32c_uses_hardware(&value) == CNA_RESULT_SUCCESS &&
            (value == CNA_TRUE || value == CNA_FALSE),
        "the selected CRC path matches the portable definition and reports its acceleration state");

    cna_demo_check(
        &demo,
        name_is(CNA_CNB_COMPRESSION_NONE, cna_cnb_get_compression_name_size,
                cna_cnb_copy_compression_name, "none") &&
            name_is(CNA_CNB_COMPRESSION_LZ4, cna_cnb_get_compression_name_size,
                    cna_cnb_copy_compression_name, "LZ4") &&
            name_is(CNA_CNB_COMPRESSION_ZSTD, cna_cnb_get_compression_name_size,
                    cna_cnb_copy_compression_name, "Zstandard") &&
            name_is(CNA_CNB_COMPRESSION_DEFLATE, cna_cnb_get_compression_name_size,
                    cna_cnb_copy_compression_name, "Deflate") &&
            name_is(UINT32_C(7), cna_cnb_get_compression_name_size,
                    cna_cnb_copy_compression_name, "unknown codec 7"),
        "every assigned codec and an unknown wire value have exact atomic names");
    cna_demo_check(
        &demo,
        cna_cnb_is_compression_supported(CNA_CNB_COMPRESSION_NONE, &value) ==
                CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_cnb_is_compression_supported(CNA_CNB_COMPRESSION_LZ4, &value) ==
                CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_cnb_is_compression_supported(UINT32_C(7), &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_cnb_is_compression_supported(
                CNA_CNB_COMPRESSION_ZSTD, &zstd_supported) == CNA_RESULT_SUCCESS,
        "stored bytes are always supported while reserved and unknown codecs are not");
    memset(output, 0, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_get_compressed_byte_count(
            Payload, payload_bytes, CNA_CNB_COMPRESSION_NONE, 3, &required) ==
                CNA_RESULT_SUCCESS &&
            required == payload_bytes &&
            cna_cnb_copy_compressed(
                Payload, payload_bytes, CNA_CNB_COMPRESSION_NONE, 3,
                output, sizeof(output), &required) == CNA_RESULT_SUCCESS &&
            required == payload_bytes && memcmp(output, Payload, (size_t)payload_bytes) == 0 &&
            cna_cnb_copy_decompressed(
                output, required, CNA_CNB_COMPRESSION_NONE,
                UINT64_C(0), UINT64_C(0), output, sizeof(output), &required) ==
                CNA_RESULT_SUCCESS &&
            required == payload_bytes && memcmp(output, Payload, (size_t)payload_bytes) == 0,
        "the stored codec round-trips exact bytes and deliberately ignores declared sizes");
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_copy_compressed(
            Payload, payload_bytes, CNA_CNB_COMPRESSION_NONE, 3,
            output, UINT64_C(4), &required) == CNA_RESULT_BUFFER_TOO_SMALL &&
            required == payload_bytes && bytes_are(output, sizeof(output), 0xA5U),
        "a short compressed destination reports its requirement without a partial write");
    cna_demo_check(
        &demo,
        cna_cnb_get_compressed_byte_count(
            Payload, payload_bytes, CNA_CNB_COMPRESSION_LZ4, 3, &required) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_cnb_copy_decompressed(
                Payload, payload_bytes, UINT32_C(7), payload_bytes, payload_bytes,
                output, sizeof(output), &required) == CNA_RESULT_NOT_SUPPORTED,
        "assigned-but-unimplemented and unknown codecs give the retryable NOT_SUPPORTED result");

    if (zstd_supported == CNA_TRUE) {
        cna_demo_check(
            &demo,
            cna_cnb_get_compressed_byte_count(
                Payload, payload_bytes, CNA_CNB_COMPRESSION_ZSTD, -1000,
                &compressed_bytes) == CNA_RESULT_SUCCESS &&
                compressed_bytes > UINT64_C(0) && compressed_bytes < payload_bytes &&
                cna_cnb_copy_compressed(
                    Payload, payload_bytes, CNA_CNB_COMPRESSION_ZSTD, 1000,
                    compressed, sizeof(compressed), &required) == CNA_RESULT_SUCCESS &&
                required > UINT64_C(0) && required < payload_bytes &&
                cna_cnb_copy_decompressed(
                    compressed, required, CNA_CNB_COMPRESSION_ZSTD,
                    payload_bytes, payload_bytes, output, sizeof(output), &compressed_bytes) ==
                    CNA_RESULT_SUCCESS &&
                compressed_bytes == payload_bytes &&
                memcmp(output, Payload, (size_t)payload_bytes) == 0,
            "available Zstandard clamps effort and round-trips a genuinely compressed payload");
    } else {
        cna_demo_check(
            &demo,
            cna_cnb_get_compressed_byte_count(
                Payload, payload_bytes, CNA_CNB_COMPRESSION_ZSTD, 3, &required) ==
                CNA_RESULT_NOT_SUPPORTED,
            "a build without Zstandard refuses that optional codec by name");
    }

    return cna_demo_finish(&demo);
}
