/* SPDX-License-Identifier: MS-PL */
/* Complete Plane value, classification and transform behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static int plane_is(CNA_Plane value, float x, float y, float z, float d)
{
    return near(value.normal.x, x) && near(value.normal.y, y) &&
           near(value.normal.z, z) && near(value.d, d);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{Normal:{X:0 Y:1 Z:0} D:-2}";
    const CNA_Plane floor = {{0.0f, 1.0f, 0.0f}, -2.0f};
    const CNA_BoundingBox front_box = {{-1.0f, 3.0f, -1.0f}, {1.0f, 4.0f, 1.0f}};
    const CNA_BoundingSphere crossing_sphere = {{0.0f, 2.5f, 0.0f}, 1.0f};
    const CNA_Matrix identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const CNA_Matrix raise_two = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 1.0f
    };
    CnaDemo demo;
    CNA_Plane zero = floor;
    CNA_Plane value = floor;
    CNA_Plane normalized = floor;
    CNA_Plane transformed = floor;
    CNA_PlaneIntersectionType classification = UINT32_C(99);
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float scalar = 0.0f;
    uint64_t text_bytes = 0U;
    char text[48];
    char before_text[48];

    cna_demo_init(&demo, "c_api.math.plane_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_plane_init(&zero) == CNA_RESULT_SUCCESS &&
            plane_is(zero, 0.0f, 0.0f, 0.0f, 0.0f) &&
            cna_plane_init_vector4((CNA_Vector4){0.0f, 1.0f, 0.0f, -2.0f}, &value) ==
                CNA_RESULT_SUCCESS && plane_is(value, 0.0f, 1.0f, 0.0f, -2.0f) &&
            cna_plane_init_normal_d((CNA_Vector3){0.0f, 1.0f, 0.0f}, -2.0f, &value) ==
                CNA_RESULT_SUCCESS && plane_is(value, 0.0f, 1.0f, 0.0f, -2.0f),
        "zero, Vector4 and normal-distance constructors map the public fields exactly");
    cna_demo_check(
        &demo,
        cna_plane_init_points(
            (CNA_Vector3){0.0f, 2.0f, 0.0f},
            (CNA_Vector3){0.0f, 2.0f, 1.0f},
            (CNA_Vector3){1.0f, 2.0f, 0.0f}, &value) == CNA_RESULT_SUCCESS &&
            plane_is(value, 0.0f, 1.0f, 0.0f, -2.0f) &&
            cna_plane_init_abcd(0.0f, 1.0f, 0.0f, -2.0f, &value) ==
                CNA_RESULT_SUCCESS && plane_is(value, 0.0f, 1.0f, 0.0f, -2.0f),
        "ordered points and four coefficients construct the same floor plane");

    cna_demo_check(
        &demo,
        cna_plane_dot(floor, (CNA_Vector4){0.0f, 3.0f, 0.0f, 2.0f}, &scalar) ==
                CNA_RESULT_SUCCESS && near(scalar, -1.0f) &&
            cna_plane_dot_coordinate(floor, (CNA_Vector3){5.0f, 7.0f, 1.0f}, &scalar) ==
                CNA_RESULT_SUCCESS && near(scalar, 5.0f) &&
            cna_plane_dot_normal(floor, (CNA_Vector3){3.0f, -1.0f, 4.0f}, &scalar) ==
                CNA_RESULT_SUCCESS && near(scalar, -1.0f),
        "dot forms distinguish homogeneous values, coordinates and directions");

    cna_demo_check(
        &demo,
        cna_plane_equals(floor, value, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_plane_not_equals(floor, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_plane_get_hash_code(floor, &hash) == CNA_RESULT_SUCCESS &&
            cna_plane_get_hash_code(value, &equal_hash) == CNA_RESULT_SUCCESS && hash == equal_hash,
        "equality and hashing observe the normal and distance together");

    normalized = (CNA_Plane){{0.0f, 2.0f, 0.0f}, -4.0f};
    cna_demo_check(
        &demo,
        cna_plane_normalize_in_place(&normalized) == CNA_RESULT_SUCCESS &&
            plane_is(normalized, 0.0f, 1.0f, 0.0f, -2.0f) &&
            cna_plane_normalize((CNA_Plane){{0.0f, 3.0f, 0.0f}, -6.0f}, &normalized) ==
                CNA_RESULT_SUCCESS && plane_is(normalized, 0.0f, 1.0f, 0.0f, -2.0f),
        "in-place and returned normalization preserve the represented plane");

    cna_demo_check(
        &demo,
        cna_plane_intersects_box(floor, front_box, &classification) == CNA_RESULT_SUCCESS &&
            classification == CNA_PLANE_INTERSECTION_FRONT &&
            cna_plane_intersects_sphere(floor, crossing_sphere, &classification) ==
                CNA_RESULT_SUCCESS && classification == CNA_PLANE_INTERSECTION_INTERSECTING &&
            cna_plane_intersects_frustum(
                floor, (CNA_BoundingFrustum){identity}, &classification) == CNA_RESULT_SUCCESS &&
            classification == CNA_PLANE_INTERSECTION_BACK,
        "plane classification distinguishes a front box, crossing sphere and back identity frustum");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_plane_get_string_size(floor, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_plane_copy_string(floor, text, text_bytes, &text_bytes) == CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Plane text exposes the nested normal and writes no terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_plane_copy_string(floor, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Plane text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_plane_transform_matrix(
            (CNA_Plane){{0.0f, 1.0f, 0.0f}, 0.0f}, raise_two, &transformed) ==
                CNA_RESULT_SUCCESS && plane_is(transformed, 0.0f, 1.0f, 0.0f, -2.0f) &&
            cna_plane_transform_quaternion(
                (CNA_Plane){{0.0f, 1.0f, 0.0f}, 0.0f},
                (CNA_Quaternion){0.0f, 0.0f, 0.70710678f, 0.70710678f},
                &transformed) == CNA_RESULT_SUCCESS &&
            plane_is(transformed, -1.0f, 0.0f, 0.0f, 0.0f),
        "matrix translation raises a plane while quaternion rotation turns its normal");

    cna_demo_check(
        &demo,
        cna_plane_transform_matrix(floor, raise_two, &transformed) == CNA_RESULT_SUCCESS &&
            transformed.normal.x == 0.0f && transformed.normal.y == 1.0f &&
            transformed.normal.z == 0.0f && transformed.d == -4.0f,
        "translating an already offset plane preserves its normal and adds the displacement");
    transformed = floor;
    cna_demo_check(
        &demo,
        cna_plane_transform_matrix(transformed, raise_two, &transformed) == CNA_RESULT_SUCCESS &&
            transformed.normal.x == 0.0f && transformed.normal.y == 1.0f &&
            transformed.normal.z == 0.0f && transformed.d == -4.0f,
        "an aliased Plane transform retains the same nonzero-offset result");

    return cna_demo_finish(&demo);
}
