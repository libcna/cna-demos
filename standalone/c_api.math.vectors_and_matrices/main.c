/* SPDX-License-Identifier: MS-PL */
/*
 * Math from C: CNA_Vector3 and CNA_Matrix are plain structs passed by value, and every math route
 * returns its answer through an out parameter (the CNA_Result reports invalid arguments). No game
 * is needed. XNA's convention holds: row vectors, so a world matrix is scale * rotation * translation.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int nearly_equal(float a, float b)
{
    return fabsf(a - b) < 1e-4f;
}

static void print_vector(const char* label, CNA_Vector3 v)
{
    char text[128];
    uint64_t length = 0U;
    /* Vector3.ToString's text, by the count/copy idiom every string in the ABI uses. */
    if (cna_vector3_copy_string(v, text, sizeof(text) - 1U, &length) == CNA_RESULT_SUCCESS) {
        text[length] = '\0';
        printf("  %-22s %s\n", label, text);
    }
}

int main(int argc, char** argv)
{
    const float half_pi = 1.57079632679f;
    CnaDemo demo;
    CNA_Vector3 point;
    CNA_Vector3 x_axis;
    CNA_Vector3 y_axis;
    CNA_Vector3 z_axis;
    CNA_Vector3 moved;
    CNA_Vector3 back;
    CNA_Vector3 diagonal;
    CNA_Matrix scale;
    CNA_Matrix rotation;
    CNA_Matrix translation;
    CNA_Matrix scale_rotation;
    CNA_Matrix world;
    CNA_Matrix inverse;
    float length = 0.0f;

    cna_demo_init(&demo, "c_api.math.vectors_and_matrices", argc, argv, 0);

    /* Values are built by routes too, so the same code works for any binding of the ABI. */
    cna_vector3_init_xyz(1.0f, 0.0f, 0.0f, &x_axis);
    cna_vector3_init_xyz(0.0f, 1.0f, 0.0f, &y_axis);
    cna_vector3_init_xyz(1.0f, 2.0f, 3.0f, &point);

    /* Cross product: X x Y = Z (right-handed, as in XNA). */
    cna_demo_check_result(&demo, cna_vector3_cross(x_axis, y_axis, &z_axis), "cna_vector3_cross");
    print_vector("Cross(UnitX, UnitY):", z_axis);
    cna_demo_check(&demo, nearly_equal(z_axis.x, 0.0f) && nearly_equal(z_axis.y, 0.0f) && nearly_equal(z_axis.z, 1.0f), "X x Y = Z");

    /* Normalize and length. */
    cna_vector3_init_xyz(3.0f, 0.0f, 4.0f, &diagonal);
    cna_demo_check_result(&demo, cna_vector3_length(diagonal, &length), "cna_vector3_length");
    cna_demo_check(&demo, nearly_equal(length, 5.0f), "(3, 0, 4) is 5 long");
    cna_demo_check_result(&demo, cna_vector3_normalize(diagonal, &diagonal), "cna_vector3_normalize");
    print_vector("Normalize((3, 0, 4)):", diagonal);
    cna_demo_check(&demo, nearly_equal(diagonal.x, 0.6f) && nearly_equal(diagonal.z, 0.8f), "normalized to (0.6, 0, 0.8)");

    /* A world matrix: scale by 2, turn a quarter about Y, move by (10, 0, 0) -- in that order. */
    cna_matrix_create_scale_scalar(2.0f, &scale);
    cna_matrix_create_rotation_y(half_pi, &rotation);
    cna_matrix_create_translation_xyz(10.0f, 0.0f, 0.0f, &translation);
    cna_matrix_multiply(scale, rotation, &scale_rotation);
    cna_demo_check_result(&demo, cna_matrix_multiply(scale_rotation, translation, &world),
                          "cna_matrix_multiply: world = scale * rotation * translation");
    cna_demo_check_result(&demo, cna_vector3_transform_matrix(point, world, &moved), "cna_vector3_transform_matrix");
    print_vector("(1, 2, 3) * world:", moved);
    /* Scaled: (2, 4, 6). A quarter turn about Y takes (x, z) to (z, -x): (6, 4, -2). Moved: (16, 4, -2). */
    cna_demo_check(&demo, nearly_equal(moved.x, 16.0f) && nearly_equal(moved.y, 4.0f) && nearly_equal(moved.z, -2.0f),
                   "scaled, then rotated, then translated: (16, 4, -2)");

    /* TransformNormal applies the supplied linear part without translation. It does not
       normalize: our scale of 2 leaves this direction two units long. */
    CNA_Vector3 direction = {0.0f, 0.0f, 0.0f};
    cna_demo_check_result(&demo, cna_vector3_transform_normal(x_axis, world, &direction),
                          "cna_vector3_transform_normal");
    cna_demo_check(&demo, nearly_equal(direction.x, 0.0f) && nearly_equal(direction.y, 0.0f) &&
                   nearly_equal(direction.z, -2.0f), "a direction is scaled and rotated but not translated");

    /* Transform a subrange of a vertex array into a different output subrange. Counts are
       whole-array capacities; indices select the first element, and length selects the range. */
    /* Each backing array has one extra guard beyond its declared count (3 and 4). Even an
       incorrectly accepted negative case stays inside allocated storage and leaves evidence. */
    const CNA_Vector3 source[4] = {{-99.0f, -99.0f, -99.0f}, {1.0f, 2.0f, 3.0f},
                                  {0.0f, 0.0f, 0.0f}, {-88.0f, -88.0f, -88.0f}};
    CNA_Vector3 destination[5] = {{-7.0f, -7.0f, -7.0f}, {-7.0f, -7.0f, -7.0f},
                                  {-7.0f, -7.0f, -7.0f}, {-7.0f, -7.0f, -7.0f}, {-8.0f, -8.0f, -8.0f}};
    cna_demo_check_result(&demo, cna_vector3_transform_matrix_array(source, 3, 1, world, destination, 4, 1, 2),
                          "transform two selected vertices into the output range");
    cna_demo_check(&demo, nearly_equal(destination[1].x, 16.0f) && nearly_equal(destination[1].y, 4.0f) &&
                   nearly_equal(destination[1].z, -2.0f) && nearly_equal(destination[2].x, 10.0f) &&
                   nearly_equal(destination[2].y, 0.0f) && nearly_equal(destination[2].z, 0.0f) &&
                   destination[0].x == -7.0f && destination[0].y == -7.0f && destination[0].z == -7.0f &&
                   destination[3].x == -7.0f && destination[3].y == -7.0f && destination[3].z == -7.0f &&
                   destination[4].x == -8.0f && destination[4].y == -8.0f && destination[4].z == -8.0f,
                   "only the requested output range changes to the expected world positions");
    CNA_Vector3 before[5];
    memcpy(before, destination, sizeof(destination));
    cna_demo_check(&demo,
                   cna_vector3_transform_matrix_array(source, 3, 1, world, destination, 4, 1, 3) == CNA_RESULT_INVALID_ARGUMENT &&
                   memcmp(before, destination, sizeof(destination)) == 0,
                   "an out-of-range source is refused before any destination byte changes");
    cna_demo_check(&demo,
                   cna_vector3_transform_matrix_array(source, 3, 1, world, destination, 4, 3, 2) == CNA_RESULT_INVALID_ARGUMENT &&
                   memcmp(before, destination, sizeof(destination)) == 0,
                   "an out-of-range destination is refused before any destination byte changes");

    /* The inverse undoes it. */
    cna_demo_check_result(&demo, cna_matrix_invert(world, &inverse), "cna_matrix_invert");
    cna_vector3_transform_matrix(moved, inverse, &back);
    print_vector("... * Invert(world):", back);
    cna_demo_check(&demo, nearly_equal(back.x, 1.0f) && nearly_equal(back.y, 2.0f) && nearly_equal(back.z, 3.0f),
                   "the inverse brings the point back");

    /* Results come back through out parameters; a null one is refused, never written through. */
    cna_demo_check(&demo, cna_vector3_length(point, NULL) == CNA_RESULT_INVALID_ARGUMENT,
                   "a null out parameter is INVALID_ARGUMENT");
    return cna_demo_finish(&demo);
}
