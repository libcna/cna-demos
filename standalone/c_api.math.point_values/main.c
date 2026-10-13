/* SPDX-License-Identifier: MS-PL */
/* Integer Point value semantics and arithmetic through the C ABI. */
#include <CNA/C/cna.h>

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int point_is(CNA_Point value, int32_t x, int32_t y)
{
    return value.x == x && value.y == y;
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:3 Y:-2}";
    CnaDemo demo;
    CNA_Point zero = {99, 98};
    CNA_Point point = {99, 98};
    CNA_Point result = {77, 88};
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = 0;
    uint64_t text_bytes = 0U;
    char text[32];
    char before[32];

    cna_demo_init(&demo, "c_api.math.point_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_point_init(&zero) == CNA_RESULT_SUCCESS && point_is(zero, 0, 0) &&
            cna_point_get_zero(&point) == CNA_RESULT_SUCCESS && point_is(point, 0, 0) &&
            cna_point_equals(zero, point, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "default initialization and the Zero property produce the same integer point");
    cna_demo_check(
        &demo,
        cna_point_init_xy(3, -2, &point) == CNA_RESULT_SUCCESS && point_is(point, 3, -2) &&
            cna_point_not_equals(point, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_point_get_hash_code(point, &hash) == CNA_RESULT_SUCCESS && hash == (3 ^ -2),
        "authored coordinates participate in inequality and the canonical component hash");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_point_get_string_size(point, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_point_copy_string(point, text, text_bytes, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical point text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    text_bytes = 0U;
    cna_demo_check(
        &demo,
        cna_point_copy_string(point, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_point_copy_string(point, NULL, 0U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U,
        "short and zero-capacity point text requests report size without partial writes");

    cna_demo_check(
        &demo,
        cna_point_add((CNA_Point){10, -3}, (CNA_Point){4, 5}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, 14, 2) &&
            cna_point_subtract((CNA_Point){10, -3}, (CNA_Point){4, 5}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, 6, -8) &&
            cna_point_multiply((CNA_Point){10, -3}, (CNA_Point){4, 5}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, 40, -15) &&
            cna_point_divide((CNA_Point){12, -9}, (CNA_Point){3, 2}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, 4, -4),
        "component arithmetic includes signed division truncated toward zero");
    cna_demo_check(
        &demo,
        cna_point_add((CNA_Point){INT32_MAX, 2}, (CNA_Point){1, 3}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, INT32_MIN, 5) &&
            cna_point_subtract((CNA_Point){INT32_MIN, 3}, (CNA_Point){1, 5}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, INT32_MAX, -2) &&
            cna_point_multiply((CNA_Point){INT32_MAX, -3}, (CNA_Point){2, 4}, &result) ==
                CNA_RESULT_SUCCESS &&
            point_is(result, -2, -12),
        "add, subtract and multiply expose CNA's defined two's-complement wraparound");
    result = (CNA_Point){77, 88};
    cna_demo_check(
        &demo,
        cna_point_divide(point, (CNA_Point){1, 0}, &result) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            point_is(result, 77, 88) &&
            cna_point_divide((CNA_Point){INT32_MIN, 1}, (CNA_Point){-1, 1}, &result) ==
                CNA_RESULT_OVERFLOW &&
            point_is(result, 77, 88),
        "zero-divisor and signed-overflow division failures preserve the caller's point");

    return cna_demo_finish(&demo);
}
