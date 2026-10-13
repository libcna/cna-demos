/* SPDX-License-Identifier: MS-PL */
/* Integer Rectangle value semantics and geometry through the C ABI. */
#include <CNA/C/cna.h>

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int rectangle_is(
    CNA_Rectangle value,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height)
{
    return value.x == x && value.y == y && value.width == width && value.height == height;
}

static int point_is(CNA_Point value, int32_t x, int32_t y)
{
    return value.x == x && value.y == y;
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:1 Y:2 Width:10 Height:8}";
    const CNA_Rectangle rectangle = {1, 2, 10, 8};
    const CNA_Rectangle overlap = {5, 5, 10, 10};
    const CNA_Rectangle touching = {11, 2, 2, 2};
    CnaDemo demo;
    CNA_Rectangle empty = {9, 8, 7, 6};
    CNA_Rectangle result = empty;
    CNA_Point point = {99, 98};
    CNA_Bool predicate = UINT8_C(77);
    int32_t scalar = 0;
    int32_t hash = 0;
    uint64_t text_bytes = 0U;
    char text[48];
    char before[48];

    cna_demo_init(&demo, "c_api.math.rectangle_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_rectangle_init(&empty) == CNA_RESULT_SUCCESS && rectangle_is(empty, 0, 0, 0, 0) &&
            cna_rectangle_get_empty(&result) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 0, 0, 0, 0) &&
            cna_rectangle_get_is_empty(empty, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "default initialization, Empty and IsEmpty agree on the all-zero rectangle");
    cna_demo_check(
        &demo,
        cna_rectangle_init_xywh(1, 2, 10, 8, &result) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 1, 2, 10, 8),
        "the authored constructor preserves position and size fields");
    cna_demo_check(
        &demo,
        cna_rectangle_get_left(rectangle, &scalar) == CNA_RESULT_SUCCESS && scalar == 1 &&
            cna_rectangle_get_right(rectangle, &scalar) == CNA_RESULT_SUCCESS && scalar == 11 &&
            cna_rectangle_get_top(rectangle, &scalar) == CNA_RESULT_SUCCESS && scalar == 2 &&
            cna_rectangle_get_bottom(rectangle, &scalar) == CNA_RESULT_SUCCESS && scalar == 10 &&
            cna_rectangle_get_location(rectangle, &point) == CNA_RESULT_SUCCESS &&
            point_is(point, 1, 2) &&
            cna_rectangle_get_center(rectangle, &point) == CNA_RESULT_SUCCESS &&
            point_is(point, 6, 6),
        "edges, location and integer center derive from the authored rectangle");
    cna_demo_check(
        &demo,
        cna_rectangle_get_right((CNA_Rectangle){INT32_MAX, 0, 1, 0}, &scalar) ==
                CNA_RESULT_SUCCESS &&
            scalar == INT32_MIN,
        "derived edges retain CNA's defined two's-complement wraparound");

    cna_demo_check(
        &demo,
        cna_rectangle_contains_xy(rectangle, 1, 2, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_rectangle_contains_xy(rectangle, 11, 9, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE &&
            cna_rectangle_contains_point(rectangle, (CNA_Point){10, 9}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "coordinate and Point containment use inclusive near and exclusive far edges");
    cna_demo_check(
        &demo,
        cna_rectangle_contains_rectangle(
            rectangle, (CNA_Rectangle){2, 3, 2, 2}, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_rectangle_contains_rectangle(
                rectangle, (CNA_Rectangle){0, 3, 2, 2}, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE,
        "rectangle containment requires every candidate edge inside the outer bounds");

    result = rectangle;
    cna_demo_check(
        &demo,
        cna_rectangle_set_location(&result, (CNA_Point){3, 4}) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 3, 4, 10, 8) &&
            cna_rectangle_offset_point(&result, (CNA_Point){2, -1}) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 5, 3, 10, 8) &&
            cna_rectangle_offset_xy(&result, -5, -3) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 0, 0, 10, 8) &&
            cna_rectangle_inflate(&result, 2, 3) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, -2, -3, 14, 14),
        "location, both offset forms and inflation mutate the same rectangle in sequence");
    cna_demo_check(
        &demo,
        cna_rectangle_equals(rectangle, rectangle, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_rectangle_not_equals(rectangle, result, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_rectangle_get_hash_code(rectangle, &hash) == CNA_RESULT_SUCCESS &&
            hash == (1 ^ 2 ^ 10 ^ 8),
        "field equality, inequality and hashing observe the complete rectangle");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_rectangle_get_string_size(rectangle, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_rectangle_copy_string(rectangle, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical rectangle text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    text_bytes = 0U;
    cna_demo_check(
        &demo,
        cna_rectangle_copy_string(
            rectangle, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_rectangle_copy_string(rectangle, NULL, 0U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U,
        "short and zero-capacity rectangle text requests report size without partial writes");

    cna_demo_check(
        &demo,
        cna_rectangle_intersects(rectangle, overlap, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_rectangle_intersects(rectangle, touching, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE,
        "intersection requires positive area, so an exactly touching edge does not overlap");
    cna_demo_check(
        &demo,
        cna_rectangle_intersect(rectangle, overlap, &result) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 5, 5, 6, 5) &&
            cna_rectangle_intersect(rectangle, touching, &result) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 0, 0, 0, 0) &&
            cna_rectangle_union(rectangle, overlap, &result) == CNA_RESULT_SUCCESS &&
            rectangle_is(result, 1, 2, 14, 13),
        "overlap, disjoint intersection and union return their exact canonical bounds");

    return cna_demo_finish(&demo);
}
