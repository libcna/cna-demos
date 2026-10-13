/* SPDX-License-Identifier: MS-PL */
/* All public packed-vector formats and binary16 helpers through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

typedef struct PackedCase {
    CNA_PackedVectorFormat format;
    CNA_Vector4 source;
    uint64_t packed;
    CNA_Vector4 expanded;
} PackedCase;

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.00002f;
}

static int vector_is(CNA_Vector4 value, CNA_Vector4 expected)
{
    return near(value.x, expected.x) && near(value.y, expected.y) &&
        near(value.z, expected.z) && near(value.w, expected.w);
}

static int all_formats_round_trip(void)
{
    static const PackedCase Cases[] = {
        {CNA_PACKED_VECTOR_FORMAT_ALPHA8,
         {9.0f, 8.0f, 7.0f, 1.0f}, UINT64_C(0xff), {0.0f, 0.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_BGR565,
         {1.0f, 1.0f, 1.0f, 9.0f}, UINT64_C(0xffff), {1.0f, 1.0f, 1.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_BGRA4444,
         {1.0f, 1.0f, 1.0f, 1.0f}, UINT64_C(0xffff), {1.0f, 1.0f, 1.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_BGRA5551,
         {1.0f, 1.0f, 1.0f, 1.0f}, UINT64_C(0xffff), {1.0f, 1.0f, 1.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_BYTE4,
         {1.0f, 2.0f, 3.0f, 4.0f}, UINT64_C(0x04030201), {1.0f, 2.0f, 3.0f, 4.0f}},
        {CNA_PACKED_VECTOR_FORMAT_HALF_SINGLE,
         {1.0f, 9.0f, 8.0f, 7.0f}, UINT64_C(0x3c00), {1.0f, 0.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_HALF_VECTOR2,
         {1.0f, -2.0f, 8.0f, 7.0f}, UINT64_C(0xc0003c00), {1.0f, -2.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_HALF_VECTOR4,
         {1.0f, -2.0f, 0.5f, 4.0f}, UINT64_C(0x44003800c0003c00),
         {1.0f, -2.0f, 0.5f, 4.0f}},
        {CNA_PACKED_VECTOR_FORMAT_NORMALIZED_BYTE2,
         {-1.0f, 1.0f, 8.0f, 7.0f}, UINT64_C(0x7f81), {-1.0f, 1.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_NORMALIZED_BYTE4,
         {-1.0f, -0.5f, 0.5f, 1.0f}, UINT64_C(0x7f40c081),
         {-1.0f, -64.0f / 127.0f, 64.0f / 127.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_NORMALIZED_SHORT2,
         {-1.0f, 1.0f, 8.0f, 7.0f}, UINT64_C(0x7fff8001), {-1.0f, 1.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_NORMALIZED_SHORT4,
         {-1.0f, 0.0f, 1.0f, 0.5f}, UINT64_C(0x40007fff00008001),
         {-1.0f, 0.0f, 1.0f, 16384.0f / 32767.0f}},
        {CNA_PACKED_VECTOR_FORMAT_RG32,
         {1.0f, 0.5f, 8.0f, 7.0f}, UINT64_C(0x8000ffff),
         {1.0f, 32768.0f / 65535.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_RGBA1010102,
         {1.0f, 0.0f, 0.5f, 1.0f}, UINT64_C(0xe00003ff),
         {1.0f, 0.0f, 512.0f / 1023.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_RGBA64,
         {1.0f, 0.0f, 0.5f, 1.0f}, UINT64_C(0xffff80000000ffff),
         {1.0f, 0.0f, 32768.0f / 65535.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_SHORT2,
         {-1.0f, 32767.0f, 8.0f, 7.0f}, UINT64_C(0x7fffffff),
         {-1.0f, 32767.0f, 0.0f, 1.0f}},
        {CNA_PACKED_VECTOR_FORMAT_SHORT4,
         {-32768.0f, -1.0f, 1.0f, 32767.0f}, UINT64_C(0x7fff0001ffff8000),
         {-32768.0f, -1.0f, 1.0f, 32767.0f}}
    };
    uint32_t index;

    for (index = 0U; index < (uint32_t)(sizeof(Cases) / sizeof(Cases[0])); ++index) {
        uint64_t packed = UINT64_MAX;
        CNA_Vector4 expanded = {99.0f, 98.0f, 97.0f, 96.0f};
        if (cna_packed_vector_pack(Cases[index].format, Cases[index].source, &packed) !=
                CNA_RESULT_SUCCESS ||
            packed != Cases[index].packed ||
            cna_packed_vector_unpack(Cases[index].format, packed, &expanded) !=
                CNA_RESULT_SUCCESS ||
            !vector_is(expanded, Cases[index].expanded)) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_Vector4 expanded = {11.0f, 22.0f, 33.0f, 44.0f};
    CNA_Bool equal = UINT8_C(77);
    CNA_Bool not_equal = UINT8_C(77);
    uint64_t packed = UINT64_C(0x1122334455667788);
    uint16_t half = UINT16_MAX;
    float single = -9.0f;

    cna_demo_init(&demo, "c_api.graphics.packed_vectors", argc, argv, 1);
    cna_demo_check(
        &demo,
        sizeof(CNA_PackedVectorFormat) == sizeof(uint32_t) &&
            CNA_PACKED_VECTOR_FORMAT_ALPHA8 == UINT32_C(0) &&
            CNA_PACKED_VECTOR_FORMAT_SHORT4 == UINT32_C(16),
        "the packed-format identity range is fixed-width and contiguous");
    cna_demo_check(
        &demo, all_formats_round_trip(),
        "all 17 concrete formats produce their exact bits and canonical expanded values");
    cna_demo_check(
        &demo,
        cna_packed_vector_equals(
            CNA_PACKED_VECTOR_FORMAT_RGBA64, UINT64_C(0x123456789abcdef0),
            UINT64_C(0x123456789abcdef0), &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE &&
            cna_packed_vector_not_equals(
                CNA_PACKED_VECTOR_FORMAT_RGBA64, UINT64_C(0x123456789abcdef0),
                UINT64_C(0x123456789abcdef1), &not_equal) == CNA_RESULT_SUCCESS &&
            not_equal == CNA_TRUE,
        "equality and inequality compare the complete selected storage width");
    equal = UINT8_C(77);
    cna_demo_check(
        &demo,
        cna_packed_vector_unpack(
            CNA_PACKED_VECTOR_FORMAT_ALPHA8, UINT64_C(0x100), &expanded) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            vector_is(expanded, (CNA_Vector4){11.0f, 22.0f, 33.0f, 44.0f}) &&
            cna_packed_vector_equals(
                CNA_PACKED_VECTOR_FORMAT_BGR565, UINT64_C(0x10000), 0U, &equal) ==
                CNA_RESULT_INVALID_ARGUMENT && equal == UINT8_C(77),
        "bits above a format's storage width are refused without changing outputs");
    cna_demo_check(
        &demo,
        cna_half_from_single(1.0f, &half) == CNA_RESULT_SUCCESS &&
            half == UINT16_C(0x3c00) &&
            cna_half_from_single_bits(INT32_C(0xc0000000), &half) == CNA_RESULT_SUCCESS &&
            half == UINT16_C(0xc000) &&
            cna_half_to_single(half, &single) == CNA_RESULT_SUCCESS && single == -2.0f,
        "the scalar helpers expose exact binary32 and bit-pattern conversion to binary16");
    cna_demo_check(
        &demo,
        cna_half_from_single(1.0f / 3.0f, &half) == CNA_RESULT_SUCCESS &&
            cna_half_to_single(half, &single) == CNA_RESULT_SUCCESS &&
            fabsf(single - 1.0f / 3.0f) < 0.0002f && single != 1.0f / 3.0f,
        "binary16 keeps the expected reduced precision for one third");
    cna_demo_check(
        &demo,
        cna_packed_vector_pack(
            CNA_PACKED_VECTOR_FORMAT_HALF_VECTOR4,
            (CNA_Vector4){NAN, INFINITY, -INFINITY, -0.0f}, &packed) ==
                CNA_RESULT_SUCCESS &&
            cna_packed_vector_unpack(
                CNA_PACKED_VECTOR_FORMAT_HALF_VECTOR4, packed, &expanded) == CNA_RESULT_SUCCESS &&
            isnan(expanded.x) && isinf(expanded.y) && expanded.y > 0.0f &&
            isinf(expanded.z) && expanded.z < 0.0f && signbit(expanded.w),
        "half formats preserve NaN, both infinity signs and negative zero");
    packed = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_packed_vector_pack(
            CNA_PACKED_VECTOR_FORMAT_RGBA64,
            (CNA_Vector4){0.0f, 0.0f, 0.0f, -INFINITY}, &packed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            packed == UINT64_C(0x1122334455667788),
        "an integer format rejects a nonfinite consumed component atomically");
    cna_demo_check(
        &demo,
        cna_packed_vector_pack(
            CNA_PACKED_VECTOR_FORMAT_ALPHA8,
            (CNA_Vector4){NAN, INFINITY, -INFINITY, 1.0f}, &packed) ==
                CNA_RESULT_SUCCESS &&
            packed == UINT64_C(0xff) &&
            cna_packed_vector_pack(
                CNA_PACKED_VECTOR_FORMAT_SHORT2,
                (CNA_Vector4){1.0f, 2.0f, NAN, INFINITY}, &packed) == CNA_RESULT_SUCCESS &&
            packed == UINT64_C(0x00020001),
        "integer formats ignore nonfinite components outside their selected channel count");
    return cna_demo_finish(&demo);
}
