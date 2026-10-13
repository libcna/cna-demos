/* SPDX-License-Identifier: MS-PL */
/* Deterministic little-endian CNB primitive writing and bounded reading through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

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

static int release_writer(CNA_CnbByteWriterHandle* writer)
{
    CNA_Result result;
    if (*writer == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_byte_writer_destroy(*writer);
    *writer = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_reader(CNA_CnbReaderHandle* reader)
{
    CNA_Result result;
    if (*reader == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_reader_destroy(*reader);
    *reader = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const uint8_t Prefix[2] = {0x10U, 0x20U};
    static const uint8_t Raw[2] = {0xAAU, 0xBBU};
    static const char MalformedBytes[2] = {(char)0xC3U, (char)0x28U};
    const CNA_StringView malformed = {MalformedBytes, UINT64_C(2)};
    CnaDemo demo;
    CNA_CnbByteWriterHandle prefix_writer = CNA_INVALID_HANDLE;
    CNA_CnbByteWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle fresh = CNA_INVALID_HANDLE;
    CNA_CnbReaderHandle stale_reader = CNA_INVALID_HANDLE;
    uint8_t encoded[128];
    uint8_t expected[128];
    uint8_t short_output[128];
    uint8_t bytes[8];
    char text[64];
    char short_text[64];
    uint64_t size = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    uint64_t position = UINT64_C(0);
    uint64_t remaining = UINT64_C(0);
    uint64_t value64 = UINT64_C(0);
    uint32_t value32 = UINT32_C(0);
    uint16_t value16 = UINT16_C(0);
    uint8_t value8 = UINT8_C(0);
    int32_t signed32 = INT32_C(0);
    float single = 0.0F;
    double doubled = 0.0;
    CNA_Bool valid = CNA_FALSE;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_byte_io", argc, argv, 1);

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_byte_writer_create_from_bytes(
                Prefix, sizeof(Prefix), &prefix_writer),
            "create a writer from copied prefix bytes")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_write_u8(prefix_writer, UINT8_C(0x30)) == CNA_RESULT_SUCCESS &&
            cna_cnb_byte_writer_get_size(prefix_writer, &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(3) &&
            cna_cnb_byte_writer_copy_bytes(
                prefix_writer, bytes, sizeof(bytes), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(3) && bytes[0] == 0x10U && bytes[1] == 0x20U &&
            bytes[2] == 0x30U,
        "the prefix writer owns its initial copy and appends without replacing it");
    cna_demo_check_result(
        &demo, cna_cnb_byte_writer_destroy(prefix_writer),
        "release the independent prefix writer");
    prefix_writer = CNA_INVALID_HANDLE;

    if (!cna_demo_check_result(
            &demo, cna_cnb_byte_writer_create(&writer),
            "create an empty primitive writer")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_get_size(writer, &size) == CNA_RESULT_SUCCESS && size == UINT64_C(0),
        "a new writer starts empty");
    if (!cna_demo_check(
            &demo,
            cna_cnb_byte_writer_write_u8(writer, UINT8_C(0x7F)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u16(writer, UINT16_C(0xBEEF)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u32(writer, UINT32_C(0x11223344)) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u64(writer, UINT64_C(0x0102030405060708)) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_i32(writer, INT32_C(-2)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_f32(writer, 1.5F) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_f64(writer, -0.25) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_string(writer, text_view("chunk")) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u32(writer, UINT32_C(2)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u16(writer, UINT16_C(7)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_u16(writer, UINT16_C(9)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_bytes(writer, Raw, sizeof(Raw)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_write_zeros(writer, UINT64_C(2)) == CNA_RESULT_SUCCESS &&
                cna_cnb_byte_writer_get_size(writer, &size) == CNA_RESULT_SUCCESS &&
                size == UINT64_C(52),
            "the writer appends every primitive, a UTF-8 string, raw bytes and zeros")) {
        goto cleanup;
    }
    memset(short_output, 0xA5, sizeof(short_output));
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_copy_bytes(
            writer, short_output, size - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == size && bytes_are(short_output, sizeof(short_output), 0xA5U),
        "a short writer copy reports its requirement without a partial write");
    memset(encoded, 0, sizeof(encoded));
    if (!cna_demo_check(
            &demo,
            cna_cnb_byte_writer_copy_bytes(writer, encoded, sizeof(encoded), &required) ==
                CNA_RESULT_SUCCESS &&
                required == size && encoded[0] == 0x7FU && encoded[1] == 0xEFU &&
                encoded[2] == 0xBEU && encoded[3] == 0x44U && encoded[4] == 0x33U &&
                encoded[5] == 0x22U && encoded[6] == 0x11U,
            "writer bytes expose the exact leading little-endian representation")) {
        goto cleanup;
    }
    memcpy(expected, encoded, (size_t)size);
    memset(short_output, 0xA5, sizeof(short_output));
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_take(
            writer, short_output, size - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == size && bytes_are(short_output, sizeof(short_output), 0xA5U) &&
            cna_cnb_byte_writer_get_size(writer, &required) == CNA_RESULT_SUCCESS &&
            required == size,
        "a refused take preserves both caller bytes and writer ownership");
    cna_demo_check(
        &demo,
        cna_cnb_byte_writer_take(writer, encoded, sizeof(encoded), &required) ==
                CNA_RESULT_SUCCESS &&
            required == size && memcmp(encoded, expected, (size_t)size) == 0 &&
            cna_cnb_byte_writer_get_size(writer, &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(0),
        "a successful take transfers every byte and leaves the writer empty");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_reader_create(
                encoded, size, text_view("demo payload"), NULL, &reader),
            "create a bounded reader over a private byte copy")) {
        goto cleanup;
    }
    memset(encoded, 0, (size_t)size);
    cna_demo_check(
        &demo,
        cna_cnb_reader_get_size(reader, &required) == CNA_RESULT_SUCCESS && required == size &&
            cna_cnb_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == UINT64_C(0) &&
            cna_cnb_reader_get_remaining(reader, &remaining) == CNA_RESULT_SUCCESS &&
            remaining == size,
        "the reader owns its input copy and starts at the full bounded region");
    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    cna_demo_check(
        &demo,
        cna_cnb_reader_get_context_size(reader, &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(12) &&
            cna_cnb_reader_copy_context(
                reader, short_text, required - UINT64_C(1), &value64) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            value64 == required &&
            bytes_are((const uint8_t*)short_text, sizeof(short_text), 0xA5U) &&
            cna_cnb_reader_copy_context(reader, text, sizeof(text), &value64) ==
                CNA_RESULT_SUCCESS &&
            value64 == required && memcmp(text, "demo payload", 12U) == 0,
        "the reader context copies exactly and atomically");
    cna_demo_check(
        &demo,
        cna_cnb_reader_copy_string(reader, text, sizeof(text), &required) ==
            CNA_RESULT_INVALID_STATE,
        "copying a string before reading one reports INVALID_STATE");

    cna_demo_check(
        &demo,
        cna_cnb_reader_read_u8(reader, &value8) == CNA_RESULT_SUCCESS && value8 == 0x7FU &&
            cna_cnb_reader_read_u16(reader, &value16) == CNA_RESULT_SUCCESS &&
            value16 == UINT16_C(0xBEEF) &&
            cna_cnb_reader_read_u32(reader, &value32) == CNA_RESULT_SUCCESS &&
            value32 == UINT32_C(0x11223344) &&
            cna_cnb_reader_read_u64(reader, &value64) == CNA_RESULT_SUCCESS &&
            value64 == UINT64_C(0x0102030405060708) &&
            cna_cnb_reader_read_i32(reader, &signed32) == CNA_RESULT_SUCCESS &&
            signed32 == INT32_C(-2) &&
            cna_cnb_reader_read_f32(reader, &single) == CNA_RESULT_SUCCESS && single == 1.5F &&
            cna_cnb_reader_read_f64(reader, &doubled) == CNA_RESULT_SUCCESS && doubled == -0.25,
        "the bounded reader restores every integer and IEEE-754 primitive exactly");
    if (!cna_demo_check(
            &demo,
            cna_cnb_reader_read_string(reader, &required) == CNA_RESULT_SUCCESS &&
                required == UINT64_C(5),
            "the reader consumes one length-prefixed UTF-8 string")) {
        goto cleanup;
    }
    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    cna_demo_check(
        &demo,
        cna_cnb_reader_copy_string(reader, short_text, UINT64_C(4), &value64) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            value64 == UINT64_C(5) &&
            bytes_are((const uint8_t*)short_text, sizeof(short_text), 0xA5U) &&
            cna_cnb_reader_copy_string(reader, text, sizeof(text), &value64) ==
                CNA_RESULT_SUCCESS &&
            value64 == UINT64_C(5) && memcmp(text, "chunk", 5U) == 0,
        "the retained decoded string supports repeated atomic copies");
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_count(
            reader, UINT64_C(2), text_view("samples"), &value32) == CNA_RESULT_SUCCESS &&
            value32 == UINT32_C(2),
        "a serialized element count is accepted only when its payload fits the region");
    cna_cnb_reader_get_position(reader, &position);
    memset(bytes, 0xA5, sizeof(bytes));
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_bytes(reader, UINT64_C(4), bytes, UINT64_C(2), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == UINT64_C(4) && bytes_are(bytes, sizeof(bytes), 0xA5U) &&
            cna_cnb_reader_get_position(reader, &value64) == CNA_RESULT_SUCCESS &&
            value64 == position,
        "a short raw-byte destination consumes nothing and writes nothing");
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_bytes(
            reader, UINT64_C(4), bytes, sizeof(bytes), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(4) && bytes[0] == 0x07U && bytes[1] == 0x00U &&
            bytes[2] == 0x09U && bytes[3] == 0x00U &&
            cna_cnb_reader_skip(reader, UINT64_C(2)) == CNA_RESULT_SUCCESS &&
            cna_cnb_reader_get_remaining(reader, &remaining) == CNA_RESULT_SUCCESS &&
            remaining == UINT64_C(2),
        "raw reads and skip advance through exact authored bytes");
    cna_demo_check(
        &demo,
        cna_cnb_reader_require_exhausted(reader) == CNA_RESULT_IO &&
            cna_cnb_reader_read_bytes(
                reader, UINT64_C(2), bytes, sizeof(bytes), &required) == CNA_RESULT_SUCCESS &&
            required == UINT64_C(2) && bytes[0] == 0U && bytes[1] == 0U &&
            cna_cnb_reader_require_exhausted(reader) == CNA_RESULT_SUCCESS,
        "exact exhaustion rejects trailing bytes until the final zero padding is consumed");
    value32 = UINT32_C(0xA5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_reader_read_u32(reader, &value32) == CNA_RESULT_IO &&
            value32 == UINT32_C(0xA5A5A5A5) &&
            cna_cnb_reader_skip(reader, UINT64_C(1)) == CNA_RESULT_IO &&
            cna_cnb_reader_fail(reader, text_view("authored schema refusal")) == CNA_RESULT_IO,
        "EOF reads are atomic and explicit schema failures share the reader's IO result");

    cna_demo_check(
        &demo,
        cna_cnb_is_well_formed_utf8(text_view("currency \xE2\x82\xAC"), &valid) ==
                CNA_RESULT_SUCCESS &&
            valid == CNA_TRUE &&
            cna_cnb_is_well_formed_utf8(malformed, &valid) == CNA_RESULT_SUCCESS &&
            valid == CNA_FALSE,
        "the writer-side UTF-8 predicate distinguishes valid multibyte text from malformed bytes");

    if (cna_cnb_reader_create(
            expected, UINT64_C(1), text_view("fresh"), NULL, &fresh) == CNA_RESULT_SUCCESS) {
        cna_demo_check_result(&demo, cna_cnb_reader_destroy(fresh), "release a second copied reader");
        fresh = CNA_INVALID_HANDLE;
    } else {
        cna_demo_check(&demo, 0, "release a second copied reader");
    }
    stale_reader = reader;
    cna_demo_check_result(&demo, cna_cnb_reader_destroy(reader), "release the bounded reader");
    reader = CNA_INVALID_HANDLE;
    required = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_reader_get_size(stale_reader, &required) == CNA_RESULT_INVALID_HANDLE &&
            required == UINT64_C(0x1122334455667788),
        "a released reader rejects stale access without changing output");

cleanup:
    releases_ok &= release_reader(&fresh);
    releases_ok &= release_reader(&reader);
    releases_ok &= release_writer(&writer);
    releases_ok &= release_writer(&prefix_writer);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining reader and writer handle exactly once");
    return cna_demo_finish(&demo);
}
