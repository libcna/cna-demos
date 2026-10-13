/* SPDX-License-Identifier: MS-PL */
/* Complete Ray value and analytic-intersection behavior through the C ABI. */
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

int main(int argc, char** argv)
{
    static const char ExpectedText[] =
        "{{Position:{X:0 Y:0 Z:5} Direction:{X:0 Y:0 Z:-1}}}";
    const CNA_BoundingBox unit_box = {{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}};
    const CNA_BoundingSphere unit_sphere = {{0.0f, 0.0f, 0.0f}, 1.0f};
    const CNA_Plane origin_plane = {{0.0f, 0.0f, 1.0f}, 0.0f};
    const CNA_Matrix identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    CnaDemo demo;
    CNA_Ray zero = {{9.0f, 8.0f, 7.0f}, {6.0f, 5.0f, 4.0f}};
    CNA_Ray ray = zero;
    CNA_Bool predicate = UINT8_C(77);
    CNA_Bool hit = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float distance = -1.0f;
    uint64_t text_bytes = 0U;
    char text[80];
    char before_text[80];

    cna_demo_init(&demo, "c_api.math.ray_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_ray_init(&zero) == CNA_RESULT_SUCCESS &&
            vector_is(zero.position, 0.0f, 0.0f, 0.0f) &&
            vector_is(zero.direction, 0.0f, 0.0f, 0.0f) &&
            cna_ray_init_position_direction(
                (CNA_Vector3){0.0f, 0.0f, 5.0f},
                (CNA_Vector3){0.0f, 0.0f, -1.0f}, &ray) == CNA_RESULT_SUCCESS &&
            vector_is(ray.position, 0.0f, 0.0f, 5.0f) &&
            vector_is(ray.direction, 0.0f, 0.0f, -1.0f),
        "zero and authored constructors expose origin and direction fields directly");
    cna_demo_check(
        &demo,
        cna_ray_equals(ray, ray, &predicate) == CNA_RESULT_SUCCESS && predicate == CNA_TRUE &&
            cna_ray_not_equals(ray, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_ray_get_hash_code(ray, &hash) == CNA_RESULT_SUCCESS &&
            cna_ray_get_hash_code(
                (CNA_Ray){{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}},
                &equal_hash) == CNA_RESULT_SUCCESS && hash == equal_hash,
        "equality and hashing observe both ray vectors");

    cna_demo_check(
        &demo,
        cna_ray_intersects_box(ray, unit_box, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_TRUE && near(distance, 4.0f) &&
            cna_ray_intersects_sphere(ray, unit_sphere, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_TRUE && near(distance, 4.0f),
        "the same nearest distance reaches the front of a unit box and sphere");
    cna_demo_check(
        &demo,
        cna_ray_intersects_plane(ray, origin_plane, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_TRUE && near(distance, 5.0f) &&
            cna_ray_intersects_plane(
                (CNA_Ray){{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 1.0f}},
                origin_plane, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_FALSE && near(distance, 0.0f),
        "plane intersection reports a forward hit and canonical zero for a ray pointing away");
    cna_demo_check(
        &demo,
        cna_ray_intersects_frustum(
            (CNA_Ray){{0.0f, 0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            (CNA_BoundingFrustum){identity}, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_TRUE && near(distance, 0.0f) &&
            cna_ray_intersects_frustum(
                (CNA_Ray){{0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 1.0f}},
                (CNA_BoundingFrustum){identity}, &hit, &distance) == CNA_RESULT_SUCCESS &&
            hit == CNA_FALSE && near(distance, 0.0f),
        "frustum intersection returns zero for an inside origin and for a forward miss");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_ray_get_string_size(ray, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_ray_copy_string(ray, text, text_bytes, &text_bytes) == CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Ray text uses the legacy double-brace form and writes no terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_ray_copy_string(ray, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Ray text destination is refused without a partial write");

    return cna_demo_finish(&demo);
}
