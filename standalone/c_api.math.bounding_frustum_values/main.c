/* SPDX-License-Identifier: MS-PL */
/* Complete BoundingFrustum value and analytic-culling behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return near(value.x, x) && near(value.y, y) && near(value.z, z);
}

static int plane_is(CNA_Plane value, float x, float y, float z, float d)
{
    return vector_is(value.normal, x, y, z) && near(value.d, d);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] =
        "{Near:{Normal:{X:0 Y:0 Z:-1} D:0} Far:{Normal:{X:0 Y:0 Z:1} D:-1} "
        "Left:{Normal:{X:-1 Y:0 Z:0} D:-1} Right:{Normal:{X:1 Y:0 Z:0} D:-1} "
        "Top:{Normal:{X:0 Y:1 Z:0} D:-1} Bottom:{Normal:{X:0 Y:-1 Z:0} D:-1}}";
    const CNA_Matrix identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const CNA_Matrix perspective = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.11111111f, -1.0f,
        0.0f, 0.0f, -1.11111111f, 0.0f
    };
    const CNA_BoundingBox inner_box = {{-0.5f, -0.5f, 0.25f}, {0.5f, 0.5f, 0.75f}};
    const CNA_BoundingSphere inner_sphere = {{0.0f, 0.0f, 0.5f}, 0.1f};
    CnaDemo demo;
    CNA_BoundingFrustum frustum = {perspective};
    CNA_BoundingFrustum other = {identity};
    CNA_Plane plane = {{9.0f, 8.0f, 7.0f}, 6.0f};
    CNA_Vector3 corners[CNA_BOUNDING_FRUSTUM_CORNER_COUNT];
    CNA_Vector3 before_corners[CNA_BOUNDING_FRUSTUM_CORNER_COUNT];
    CNA_ContainmentType containment = UINT32_C(99);
    CNA_PlaneIntersectionType plane_side = UINT32_C(99);
    CNA_Bool predicate = UINT8_C(77);
    CNA_Bool hit = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float distance = -1.0f;
    uint64_t count = 0U;
    uint64_t text_bytes = 0U;
    char text[320];
    char before_text[320];

    cna_demo_init(&demo, "c_api.math.bounding_frustum_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_BOUNDING_FRUSTUM_CORNER_COUNT == 8U &&
            cna_bounding_frustum_init_matrix(identity, &frustum) == CNA_RESULT_SUCCESS &&
            memcmp(&frustum.matrix, &identity, sizeof(identity)) == 0,
        "construction retains the combined matrix and exposes the fixed corner count");

    cna_demo_check(
        &demo,
        cna_bounding_frustum_get_near(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, 0.0f, 0.0f, -1.0f, 0.0f) &&
            cna_bounding_frustum_get_far(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, 0.0f, 0.0f, 1.0f, -1.0f),
        "identity near and far planes bound Z from zero through one");
    cna_demo_check(
        &demo,
        cna_bounding_frustum_get_left(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, -1.0f, 0.0f, 0.0f, -1.0f) &&
            cna_bounding_frustum_get_right(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, 1.0f, 0.0f, 0.0f, -1.0f),
        "identity left and right planes bound X from minus one through one");
    cna_demo_check(
        &demo,
        cna_bounding_frustum_get_top(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, 0.0f, 1.0f, 0.0f, -1.0f) &&
            cna_bounding_frustum_get_bottom(frustum, &plane) == CNA_RESULT_SUCCESS &&
            plane_is(plane, 0.0f, -1.0f, 0.0f, -1.0f),
        "identity top and bottom planes bound Y from minus one through one");

    cna_demo_check(
        &demo,
        cna_bounding_frustum_contains_frustum(frustum, frustum, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_frustum_contains_box(frustum, inner_box, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_frustum_contains_sphere(frustum, inner_sphere, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_CONTAINS,
        "identity frustum contains itself and centered box and sphere volumes");
    cna_demo_check(
        &demo,
        cna_bounding_frustum_contains_point(
            frustum, (CNA_Vector3){0.0f, 0.0f, 0.5f}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_frustum_contains_point(
                frustum, (CNA_Vector3){0.0f, 0.0f, 0.0f}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_INTERSECTS &&
            cna_bounding_frustum_contains_point(
                frustum, (CNA_Vector3){0.0f, 0.0f, 2.0f}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_DISJOINT,
        "interior, boundary and exterior points produce all containment classifications");

    memset(corners, 0xA5, sizeof(corners));
    memcpy(before_corners, corners, sizeof(corners));
    cna_demo_check(
        &demo,
        cna_bounding_frustum_copy_corners(frustum, corners, 7U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == CNA_BOUNDING_FRUSTUM_CORNER_COUNT &&
            memcmp(corners, before_corners, sizeof(corners)) == 0 &&
            cna_bounding_frustum_copy_corners(
                frustum, corners, CNA_BOUNDING_FRUSTUM_CORNER_COUNT, &count) ==
                CNA_RESULT_SUCCESS &&
            vector_is(corners[0], -1.0f, 1.0f, 0.0f) &&
            vector_is(corners[3], -1.0f, -1.0f, 0.0f) &&
            vector_is(corners[4], -1.0f, 1.0f, 1.0f) &&
            vector_is(corners[7], -1.0f, -1.0f, 1.0f),
        "corner copying is atomic and preserves XNA's near-then-far ordering");

    cna_demo_check(
        &demo,
        cna_bounding_frustum_intersects_frustum(frustum, frustum, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_TRUE &&
            cna_bounding_frustum_intersects_box(frustum, inner_box, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_TRUE &&
            cna_bounding_frustum_intersects_sphere(frustum, inner_sphere, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_TRUE &&
            cna_bounding_frustum_intersects_plane(
                frustum, (CNA_Plane){{0.0f, 1.0f, 0.0f}, 0.0f}, &plane_side) ==
                CNA_RESULT_SUCCESS && plane_side == CNA_PLANE_INTERSECTION_INTERSECTING,
        "frustum intersection agrees for itself, contained volumes and a bisecting plane");

    cna_demo_check(
        &demo,
        cna_bounding_frustum_intersects_ray(
            frustum, (CNA_Ray){{0.0f, 0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_TRUE && near(distance, 0.0f) &&
            cna_bounding_frustum_intersects_ray(
                frustum, (CNA_Ray){{0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 1.0f}},
                &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_FALSE && near(distance, 0.0f),
        "ray intersection returns canonical zero for an inside origin and a forward miss");
    hit = CNA_TRUE;
    distance = 77.0f;
    cna_demo_check(
        &demo,
        cna_bounding_frustum_intersects_ray(
            frustum, (CNA_Ray){{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            &hit, &distance) == CNA_RESULT_NOT_SUPPORTED &&
            hit == CNA_TRUE && near(distance, 77.0f),
        "the documented native boundary-origin ray case is refused atomically");

    cna_demo_check(
        &demo,
        cna_bounding_frustum_init_matrix(perspective, &other) == CNA_RESULT_SUCCESS &&
            cna_bounding_frustum_equals(frustum, frustum, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_frustum_not_equals(frustum, other, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_frustum_get_hash_code(frustum, &hash) == CNA_RESULT_SUCCESS &&
            cna_bounding_frustum_get_hash_code(
                (CNA_BoundingFrustum){identity}, &equal_hash) == CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "equality and hashing observe the combined matrix");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_frustum_get_string_size(frustum, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_bounding_frustum_copy_string(frustum, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical BoundingFrustum text exposes all six planes without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_frustum_copy_string(
            frustum, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short BoundingFrustum text destination is refused without a partial write");

    return cna_demo_finish(&demo);
}
