/* SPDX-License-Identifier: MS-PL */
/* Complete BoundingSphere value and analytic-geometry behavior through the C ABI. */
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

static int sphere_is(CNA_BoundingSphere value, float x, float y, float z, float radius)
{
    return vector_is(value.center, x, y, z) && near(value.radius, radius);
}

static int sphere_contains_point(CNA_BoundingSphere sphere, CNA_Vector3 point)
{
    const float x = point.x - sphere.center.x;
    const float y = point.y - sphere.center.y;
    const float z = point.z - sphere.center.z;
    return x * x + y * y + z * z <= sphere.radius * sphere.radius + 0.0001f;
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{Center:{X:1 Y:2 Z:3} Radius:2}";
    const CNA_BoundingBox unit_box = {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    const CNA_BoundingSphere inner = {{0.0f, 0.0f, 0.0f}, 1.0f};
    const CNA_BoundingSphere outer = {{0.0f, 0.0f, 0.0f}, 5.0f};
    const CNA_Matrix identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const CNA_Matrix transform = {
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        5.0f, -2.0f, 1.0f, 1.0f
    };
    const CNA_Vector3 points[] = {
        {-2.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
    };
    CnaDemo demo;
    CNA_BoundingSphere zero = outer;
    CNA_BoundingSphere sphere = zero;
    CNA_BoundingSphere result = zero;
    CNA_ContainmentType containment = UINT32_C(99);
    CNA_PlaneIntersectionType plane_side = UINT32_C(99);
    CNA_Bool predicate = UINT8_C(77);
    CNA_Bool hit = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float distance = -1.0f;
    uint64_t text_bytes = 0U;
    char text[64];
    char before_text[64];

    cna_demo_init(&demo, "c_api.math.bounding_sphere_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_bounding_sphere_init(&zero) == CNA_RESULT_SUCCESS &&
            sphere_is(zero, 0.0f, 0.0f, 0.0f, 0.0f) &&
            cna_bounding_sphere_init_center_radius(
                (CNA_Vector3){1.0f, 2.0f, 3.0f}, 2.0f, &sphere) == CNA_RESULT_SUCCESS &&
            sphere_is(sphere, 1.0f, 2.0f, 3.0f, 2.0f),
        "zero and center-radius construction expose both public fields");
    cna_demo_check(
        &demo,
        cna_bounding_sphere_transform(sphere, transform, &result) == CNA_RESULT_SUCCESS &&
            sphere_is(result, 7.0f, 4.0f, 13.0f, 8.0f),
        "matrix transform moves the center and scales radius by the largest axis");

    cna_demo_check(
        &demo,
        cna_bounding_sphere_contains_box(outer, unit_box, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_sphere_contains_frustum(
                (CNA_BoundingSphere){{0.0f, 0.0f, 0.0f}, 3.0f},
                (CNA_BoundingFrustum){identity}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_sphere_contains_sphere(outer, inner, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_CONTAINS &&
            cna_bounding_sphere_contains_point(
                outer, (CNA_Vector3){5.0f, 0.0f, 0.0f}, &containment) == CNA_RESULT_SUCCESS &&
            containment == CNA_CONTAINMENT_DISJOINT,
        "sphere containment covers box, frustum, nested sphere and boundary-point semantics");
    cna_demo_check(
        &demo,
        cna_bounding_sphere_contains_sphere(
            inner, (CNA_BoundingSphere){{1.5f, 0.0f, 0.0f}, 1.0f}, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_INTERSECTS &&
            cna_bounding_sphere_contains_sphere(
                inner, (CNA_BoundingSphere){{3.0f, 0.0f, 0.0f}, 1.0f}, &containment) ==
                CNA_RESULT_SUCCESS && containment == CNA_CONTAINMENT_DISJOINT,
        "partly overlapping and separated spheres distinguish Intersects from Disjoint");

    cna_demo_check(
        &demo,
        cna_bounding_sphere_equals(sphere, sphere, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_sphere_not_equals(sphere, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_sphere_get_hash_code(sphere, &hash) == CNA_RESULT_SUCCESS &&
            cna_bounding_sphere_get_hash_code(
                (CNA_BoundingSphere){{1.0f, 2.0f, 3.0f}, 2.0f}, &equal_hash) ==
                CNA_RESULT_SUCCESS && hash == equal_hash,
        "equality and hashing observe center and radius together");

    cna_demo_check(
        &demo,
        cna_bounding_sphere_create_from_box(unit_box, &result) == CNA_RESULT_SUCCESS &&
            sphere_is(result, 0.0f, 0.0f, 0.0f, sqrtf(3.0f)) &&
            cna_bounding_sphere_create_from_frustum(
                (CNA_BoundingFrustum){identity}, &result) == CNA_RESULT_SUCCESS &&
            result.radius > 0.0f &&
            sphere_contains_point(result, (CNA_Vector3){-1.0f, 1.0f, 0.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){1.0f, 1.0f, 0.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){1.0f, -1.0f, 0.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){-1.0f, -1.0f, 0.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){-1.0f, 1.0f, 1.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){1.0f, 1.0f, 1.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){1.0f, -1.0f, 1.0f}) &&
            sphere_contains_point(result, (CNA_Vector3){-1.0f, -1.0f, 1.0f}),
        "box and identity-frustum factories enclose every source corner");
    cna_demo_check(
        &demo,
        cna_bounding_sphere_create_from_points(points, 3U, &result) == CNA_RESULT_SUCCESS &&
            sphere_is(result, 0.0f, 0.0f, 0.0f, 2.0f) &&
            cna_bounding_sphere_create_merged(
                (CNA_BoundingSphere){{-2.0f, 0.0f, 0.0f}, 1.0f},
                (CNA_BoundingSphere){{2.0f, 0.0f, 0.0f}, 1.0f}, &result) ==
                CNA_RESULT_SUCCESS && sphere_is(result, 0.0f, 0.0f, 0.0f, 3.0f),
        "point and merge factories compute symmetric enclosing spheres");
    result = sphere;
    cna_demo_check(
        &demo,
        cna_bounding_sphere_create_from_points(NULL, 0U, &result) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            sphere_is(result, 1.0f, 2.0f, 3.0f, 2.0f),
        "an empty point set is refused without changing the destination sphere");

    cna_demo_check(
        &demo,
        cna_bounding_sphere_intersects_box(inner, unit_box, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_sphere_intersects_frustum(
                (CNA_BoundingSphere){{0.0f, 0.0f, 0.5f}, 0.25f},
                (CNA_BoundingFrustum){identity}, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_bounding_sphere_intersects_sphere(
                (CNA_BoundingSphere){{0.0f, 0.0f, 0.0f}, 2.0f},
                (CNA_BoundingSphere){{4.0f, 0.0f, 0.0f}, 2.0f}, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_FALSE,
        "sphere intersection includes overlapping box and frustum but excludes external sphere tangency");
    cna_demo_check(
        &demo,
        cna_bounding_sphere_intersects_ray(
            inner, (CNA_Ray){{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}},
            &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_TRUE && near(distance, 4.0f) &&
            cna_bounding_sphere_intersects_ray(
                inner, (CNA_Ray){{5.0f, 5.0f, 5.0f}, {1.0f, 0.0f, 0.0f}},
                &hit, &distance) == CNA_RESULT_SUCCESS && hit == CNA_FALSE && near(distance, 0.0f) &&
            cna_bounding_sphere_intersects_plane(
                (CNA_BoundingSphere){{0.0f, 2.0f, 0.0f}, 0.5f},
                (CNA_Plane){{0.0f, 1.0f, 0.0f}, 0.0f}, &plane_side) ==
                CNA_RESULT_SUCCESS && plane_side == CNA_PLANE_INTERSECTION_FRONT,
        "ray hit/miss distances and plane-side classification remain analytic");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_sphere_get_string_size(sphere, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_bounding_sphere_copy_string(sphere, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical BoundingSphere text exposes center and radius without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_bounding_sphere_copy_string(
            sphere, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short BoundingSphere text destination is refused without a partial write");

    return cna_demo_finish(&demo);
}
