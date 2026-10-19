/* SPDX-License-Identifier: MS-PL */
/* Complete BoundingBox value and analytic-geometry behavior through the C ABI. */
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

static int box_is(CNA_BoundingBox value, CNA_Vector3 min, CNA_Vector3 max)
{
    return vector_is(value.min, min.x, min.y, min.z) &&
           vector_is(value.max, max.x, max.y, max.z);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] =
        "{Min:{X:-1 Y:-2 Z:-3} Max:{X:4 Y:5 Z:6}}";
    const CNA_BoundingBox outer = {{-2.0f, -2.0f, -2.0f}, {2.0f, 2.0f, 2.0f}};
    const CNA_BoundingBox inner = {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    const CNA_BoundingSphere inside_sphere = {{0.0f, 0.0f, 0.0f}, 0.5f};
    const CNA_Matrix identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const CNA_Vector3 points[] = {
        {-2.0f, 3.0f, 1.0f}, {4.0f, -5.0f, 6.0f}, {0.0f, 1.0f, -7.0f}
    };
    CnaDemo demo;
    CNA_BoundingBox zero = outer;
    CNA_BoundingBox box = zero;
    CNA_BoundingBox created = zero;
    CNA_Vector3 corners[CNA_BOUNDING_BOX_CORNER_COUNT];
    CNA_Vector3 before_corners[CNA_BOUNDING_BOX_CORNER_COUNT];
    CNA_ContainmentType containment = UINT32_C(99);
    CNA_PlaneIntersectionType plane_side = UINT32_C(99);
    CNA_Bool predicate = UINT8_C(77);
    CNA_Bool hit = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float distance = -1.0f;
    uint64_t count = 0U;
    uint64_t text_bytes = 0U;
    char text[80];
    char before_text[80];

    cna_demo_init(&demo, "c_api.math.bounding_box_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_BOUNDING_BOX_CORNER_COUNT == 8U &&
            cna_bounding_box_init(&zero) == CNA_RESULT_SUCCESS &&
            box_is(zero, (CNA_Vector3){0.0f, 0.0f, 0.0f},
                   (CNA_Vector3){0.0f, 0.0f, 0.0f}) &&
            cna_bounding_box_init_min_max(
                (CNA_Vector3){-1.0f, -2.0f, -3.0f},
                (CNA_Vector3){4.0f, 5.0f, 6.0f}, &box) == CNA_RESULT_SUCCESS &&
            box_is(box, (CNA_Vector3){-1.0f, -2.0f, -3.0f},
                   (CNA_Vector3){4.0f, 5.0f, 6.0f}),
        "zero and min-max construction expose both public corners and the fixed corner count");

    cna_demo_check(
        &demo,
        cna_bounding_box_contains_box(outer, inner, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_box_contains_sphere(outer, inside_sphere, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_box_contains_point(
                outer, (CNA_Vector3){2.0f, 0.0f, 0.0f}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_box_contains_frustum(
                outer, (CNA_BoundingFrustum){identity}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS,
        "an outer box contains an inner box, sphere, boundary point and identity frustum");
    cna_demo_check(
        &demo,
        cna_bounding_box_contains_sphere(
            outer, (CNA_BoundingSphere){{2.0f, 0.0f, 0.0f}, 1.0f}, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_INTERSECTS &&
            cna_bounding_box_contains_sphere(
                outer, (CNA_BoundingSphere){{5.0f, 0.0f, 0.0f}, 1.0f}, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_DISJOINT,
        "crossing and separated spheres distinguish Intersects from Disjoint");

    memset(corners, 0xA5, sizeof(corners));
    memcpy(before_corners, corners, sizeof(corners));
    cna_demo_check(
        &demo,
        cna_bounding_box_copy_corners(box, corners, 7U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == CNA_BOUNDING_BOX_CORNER_COUNT &&
            memcmp(corners, before_corners, sizeof(corners)) == 0 &&
            cna_bounding_box_copy_corners(
                box, corners, CNA_BOUNDING_BOX_CORNER_COUNT, &count) == CNA_RESULT_SUCCESS &&
            vector_is(corners[0], -1.0f, 5.0f, 6.0f) &&
            vector_is(corners[1], 4.0f, 5.0f, 6.0f) &&
            vector_is(corners[6], 4.0f, -2.0f, -3.0f) &&
            vector_is(corners[7], -1.0f, -2.0f, -3.0f),
        "corner copying is atomic and preserves XNA's eight-corner ordering");

    cna_demo_check(
        &demo,
        cna_bounding_box_intersects_ray(
            inner, (CNA_Ray){{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}},
            &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_TRUE && near(distance, 4.0f) &&
            cna_bounding_box_intersects_ray(
                inner, (CNA_Ray){{5.0f, 5.0f, 5.0f}, {1.0f, 0.0f, 0.0f}},
                &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_FALSE && near(distance, 0.0f),
        "ray intersection reports the nearest entry distance and canonical zero on a miss");
    cna_demo_check(
        &demo,
        cna_bounding_box_intersects_frustum(
            inner, (CNA_BoundingFrustum){identity}, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_box_intersects_sphere(inner, inside_sphere, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_TRUE &&
            cna_bounding_box_intersects_box(outer, inner, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_box_intersects_plane(
                inner, (CNA_Plane){{0.0f, 1.0f, 0.0f}, 0.0f}, &plane_side) ==
                CNA_RESULT_SUCCESS && plane_side == CNA_PLANE_INTERSECTION_INTERSECTING,
        "box intersections agree across frustum, sphere, box and plane forms");

    cna_demo_check(
        &demo,
        cna_bounding_box_equals(box, box, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_box_not_equals(box, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_box_get_hash_code(box, &hash) == CNA_RESULT_SUCCESS &&
            cna_bounding_box_get_hash_code(
                (CNA_BoundingBox){{-1.0f, -2.0f, -3.0f}, {4.0f, 5.0f, 6.0f}},
                &equal_hash) == CNA_RESULT_SUCCESS && hash == equal_hash,
        "equality and hashing observe both min and max corners");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_box_get_string_size(box, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_bounding_box_copy_string(box, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical BoundingBox text uses the legacy double-brace form without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_box_copy_string(box, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short BoundingBox text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_bounding_box_create_from_points(points, 3U, &created) == CNA_RESULT_SUCCESS &&
            box_is(created, (CNA_Vector3){-2.0f, -5.0f, -7.0f},
                   (CNA_Vector3){4.0f, 3.0f, 6.0f}) &&
            cna_bounding_box_create_from_sphere(
                (CNA_BoundingSphere){{1.0f, 1.0f, 1.0f}, 2.0f}, &created) ==
                CNA_RESULT_SUCCESS &&
            box_is(created, (CNA_Vector3){-1.0f, -1.0f, -1.0f},
                   (CNA_Vector3){3.0f, 3.0f, 3.0f}) &&
            cna_bounding_box_create_merged(inner, created, &created) == CNA_RESULT_SUCCESS &&
            box_is(created, (CNA_Vector3){-1.0f, -1.0f, -1.0f},
                   (CNA_Vector3){3.0f, 3.0f, 3.0f}),
        "point, sphere and merge factories compute exact enclosing extrema");
    created = box;
    cna_demo_check(
        &demo,
        cna_bounding_box_create_from_points(NULL, 0U, &created) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            box_is(created, box.min, box.max),
        "an empty point set is refused without changing the destination box");

    return cna_demo_finish(&demo);
}
