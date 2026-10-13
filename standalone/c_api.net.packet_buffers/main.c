/* SPDX-License-Identifier: MS-PL */
/* PacketWriter and PacketReader value round trips, seeking and buffer ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_Matrix sample_matrix(void)
{
    CNA_Matrix value;
    value.m11 = 1.0F;
    value.m12 = 2.0F;
    value.m13 = 3.0F;
    value.m14 = 4.0F;
    value.m21 = 5.0F;
    value.m22 = 6.0F;
    value.m23 = 7.0F;
    value.m24 = 8.0F;
    value.m31 = 9.0F;
    value.m32 = 10.0F;
    value.m33 = 11.0F;
    value.m34 = 12.0F;
    value.m41 = 13.0F;
    value.m42 = 14.0F;
    value.m43 = 15.0F;
    value.m44 = 16.0F;
    return value;
}

static int same_matrix(CNA_Matrix left, CNA_Matrix right)
{
    return left.m11 == right.m11 && left.m12 == right.m12 &&
        left.m13 == right.m13 && left.m14 == right.m14 &&
        left.m21 == right.m21 && left.m22 == right.m22 &&
        left.m23 == right.m23 && left.m24 == right.m24 &&
        left.m31 == right.m31 && left.m32 == right.m32 &&
        left.m33 == right.m33 && left.m34 == right.m34 &&
        left.m41 == right.m41 && left.m42 == right.m42 &&
        left.m43 == right.m43 && left.m44 == right.m44;
}

int main(int argc, char** argv)
{
    static const int32_t PacketBytes = 132;
    const CNA_Color first_color = {10U, 20U, 30U, 40U};
    const CNA_Color replacement_color = {50U, 60U, 70U, 80U};
    const CNA_Vector2 vector2 = {1.5F, 2.5F};
    const CNA_Vector3 vector3 = {3.5F, 4.5F, 5.5F};
    const CNA_Vector4 vector4 = {6.5F, 7.5F, 8.5F, 9.5F};
    const CNA_Quaternion quaternion = {0.25F, 0.5F, 0.75F, 1.0F};
    const CNA_Matrix matrix = sample_matrix();
    CnaDemo demo;
    CNA_PacketWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_PacketReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_PacketWriterHandle rejected_writer = UINT64_C(99);
    CNA_PacketReaderHandle rejected_reader = UINT64_C(99);
    CNA_Color read_color = {0U, 0U, 0U, 0U};
    CNA_Vector2 read_vector2 = {0.0F, 0.0F};
    CNA_Vector3 read_vector3 = {0.0F, 0.0F, 0.0F};
    CNA_Vector4 read_vector4 = {0.0F, 0.0F, 0.0F, 0.0F};
    CNA_Quaternion read_quaternion = {0.0F, 0.0F, 0.0F, 0.0F};
    CNA_Matrix read_matrix;
    uint8_t bytes[256];
    uint8_t copy[256];
    uint8_t before[256];
    uint64_t copied = UINT64_C(0);
    int32_t length = -1;
    int32_t position = -1;
    float single = 0.0F;
    double double_value = 0.0;

    cna_demo_init(&demo, "c_api.net.packet_buffers", argc, argv, 1);
    cna_demo_check(
        &demo,
        cna_packet_writer_create(-1, &rejected_writer) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected_writer == CNA_INVALID_HANDLE &&
            cna_packet_reader_create(-1, &rejected_reader) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected_reader == CNA_INVALID_HANDLE,
        "negative capacity is refused atomically by both packet-buffer constructors");
    if (!cna_demo_check_result(&demo, cna_packet_writer_create(16, &writer),
                               "create a growable PacketWriter") ||
        !cna_demo_check_result(&demo, cna_packet_reader_create(16, &reader),
                               "create an independently owned PacketReader")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_packet_writer_get_length(writer, &length) == CNA_RESULT_SUCCESS && length == 0 &&
            cna_packet_writer_get_position(writer, &position) == CNA_RESULT_SUCCESS &&
            position == 0,
        "a fresh writer has zero length and position despite its capacity hint");

    cna_demo_check(
        &demo,
        cna_packet_writer_write_color(writer, first_color) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_vector2(writer, vector2) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_vector3(writer, vector3) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_vector4(writer, vector4) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_quaternion(writer, quaternion) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_matrix(writer, matrix) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_single(writer, 11.25F) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_double(writer, 12.5) == CNA_RESULT_SUCCESS,
        "the writer accepts every public packet value in one ordered payload");
    cna_demo_check(
        &demo,
        cna_packet_writer_get_length(writer, &length) == CNA_RESULT_SUCCESS &&
            length == PacketBytes &&
            cna_packet_writer_get_position(writer, &position) == CNA_RESULT_SUCCESS &&
            position == PacketBytes,
        "the complete payload length and write position equal 132 encoded bytes");

    memset(bytes, 0xA5, sizeof(bytes));
    memcpy(before, bytes, sizeof(before));
    cna_demo_check(
        &demo,
        cna_packet_writer_copy_data_ext(writer, bytes, (uint64_t)PacketBytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == (uint64_t)PacketBytes && memcmp(bytes, before, sizeof(bytes)) == 0,
        "a short writer destination reports the exact length without a partial copy");
    cna_demo_check(
        &demo,
        cna_packet_writer_copy_data_ext(writer, bytes, sizeof(bytes), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == (uint64_t)PacketBytes &&
            bytes[0] == first_color.r && bytes[1] == first_color.g &&
            bytes[2] == first_color.b && bytes[3] == first_color.a,
        "an exact writer copy begins with the Color's public RGBA bytes");
    cna_demo_check(
        &demo,
        cna_packet_writer_set_position(writer, 0) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_write_color(writer, replacement_color) == CNA_RESULT_SUCCESS &&
            cna_packet_writer_get_position(writer, &position) == CNA_RESULT_SUCCESS &&
            position == 4 &&
            cna_packet_writer_get_length(writer, &length) == CNA_RESULT_SUCCESS &&
            length == PacketBytes &&
            cna_packet_writer_copy_data_ext(writer, bytes, sizeof(bytes), &copied) ==
                CNA_RESULT_SUCCESS &&
            bytes[0] == replacement_color.r && bytes[1] == replacement_color.g &&
            bytes[2] == replacement_color.b && bytes[3] == replacement_color.a,
        "seeking and rewriting replaces the first value without truncating later packet data");

    cna_demo_check(
        &demo,
        cna_packet_reader_set_data_ext(reader, bytes, copied) == CNA_RESULT_SUCCESS &&
            cna_packet_reader_get_length(reader, &length) == CNA_RESULT_SUCCESS &&
            length == PacketBytes &&
            cna_packet_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == 0,
        "SetData copies the complete packet into a reader positioned at its beginning");
    memset(&read_matrix, 0, sizeof(read_matrix));
    cna_demo_check(
        &demo,
        cna_packet_reader_read_color(reader, &read_color) == CNA_RESULT_SUCCESS &&
            memcmp(&read_color, &replacement_color, sizeof(read_color)) == 0 &&
            cna_packet_reader_read_vector2(reader, &read_vector2) == CNA_RESULT_SUCCESS &&
            read_vector2.x == vector2.x && read_vector2.y == vector2.y &&
            cna_packet_reader_read_vector3(reader, &read_vector3) == CNA_RESULT_SUCCESS &&
            read_vector3.x == vector3.x && read_vector3.y == vector3.y &&
            read_vector3.z == vector3.z &&
            cna_packet_reader_read_vector4(reader, &read_vector4) == CNA_RESULT_SUCCESS &&
            read_vector4.x == vector4.x && read_vector4.y == vector4.y &&
            read_vector4.z == vector4.z && read_vector4.w == vector4.w,
        "the reader restores Color and every vector with exact components");
    cna_demo_check(
        &demo,
        cna_packet_reader_read_quaternion(reader, &read_quaternion) == CNA_RESULT_SUCCESS &&
            read_quaternion.x == quaternion.x && read_quaternion.y == quaternion.y &&
            read_quaternion.z == quaternion.z && read_quaternion.w == quaternion.w &&
            cna_packet_reader_read_matrix(reader, &read_matrix) == CNA_RESULT_SUCCESS &&
            same_matrix(read_matrix, matrix),
        "the reader restores the Quaternion and all sixteen Matrix elements");
    cna_demo_check(
        &demo,
        cna_packet_reader_read_single(reader, &single) == CNA_RESULT_SUCCESS &&
            single == 11.25F &&
            cna_packet_reader_read_double(reader, &double_value) == CNA_RESULT_SUCCESS &&
            double_value == 12.5 &&
            cna_packet_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == PacketBytes &&
            cna_packet_reader_read_single(reader, &single) == CNA_RESULT_IO,
        "scalar reads consume the final bytes and a read at end reports I/O exhaustion");

    memset(copy, 0xA5, sizeof(copy));
    memcpy(before, copy, sizeof(before));
    cna_demo_check(
        &demo,
        cna_packet_reader_copy_data_ext(reader, copy, (uint64_t)PacketBytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == (uint64_t)PacketBytes && memcmp(copy, before, sizeof(copy)) == 0,
        "a short reader destination is atomic even when the read position is at end");
    cna_demo_check(
        &demo,
        cna_packet_reader_copy_data_ext(reader, copy, sizeof(copy), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == (uint64_t)PacketBytes && memcmp(copy, bytes, (size_t)copied) == 0 &&
            cna_packet_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == PacketBytes,
        "copying all reader bytes is independent of and preserves its current position");
    cna_demo_check(
        &demo,
        cna_packet_reader_set_position(reader, 0) == CNA_RESULT_SUCCESS &&
            cna_packet_reader_read_color(reader, &read_color) == CNA_RESULT_SUCCESS &&
            memcmp(&read_color, &replacement_color, sizeof(read_color)) == 0 &&
            cna_packet_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == 4,
        "reader seeking permits the rewritten first value to be consumed again");
    cna_demo_check(
        &demo,
        cna_packet_reader_set_data_ext(reader, bytes, UINT64_C(3)) == CNA_RESULT_SUCCESS &&
            cna_packet_reader_read_color(reader, &read_color) == CNA_RESULT_IO,
        "an undersized Color payload reports I/O exhaustion");
    cna_demo_check(
        &demo,
        cna_packet_reader_set_data_ext(reader, NULL, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_packet_reader_get_length(reader, &length) == CNA_RESULT_SUCCESS && length == 0 &&
            cna_packet_reader_get_position(reader, &position) == CNA_RESULT_SUCCESS &&
            position == 0 &&
            cna_packet_reader_set_data_ext(reader, NULL, UINT64_C(1)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "an empty null data view resets the reader while a nonempty null view is refused");

cleanup:
    if (reader != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_packet_reader_destroy(reader) == CNA_RESULT_SUCCESS &&
                cna_packet_reader_destroy(reader) == CNA_RESULT_INVALID_HANDLE,
            "the PacketReader releases exactly once");
    }
    if (writer != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_packet_writer_destroy(writer) == CNA_RESULT_SUCCESS &&
                cna_packet_writer_destroy(writer) == CNA_RESULT_INVALID_HANDLE,
            "the PacketWriter releases exactly once");
    }
    return cna_demo_finish(&demo);
}
