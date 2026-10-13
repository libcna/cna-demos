/* SPDX-License-Identifier: MS-PL */
/*
 * Effect parameter values from C: one ownerless parameter exposes CNA's tagged scalar and array
 * families, string storage and typed texture slots. Non-symmetric matrices distinguish ordinary
 * from transposed storage; guarded destinations prove array/text copies are atomic.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView view;
    view.data = text;
    view.byte_length = (uint64_t)strlen(text);
    return view;
}

static CNA_Matrix sequence_matrix(float offset)
{
    CNA_Matrix value = {
        offset + 1.0f, offset + 2.0f, offset + 3.0f, offset + 4.0f,
        offset + 5.0f, offset + 6.0f, offset + 7.0f, offset + 8.0f,
        offset + 9.0f, offset + 10.0f, offset + 11.0f, offset + 12.0f,
        offset + 13.0f, offset + 14.0f, offset + 15.0f, offset + 16.0f};
    return value;
}

static int same_matrix(const CNA_Matrix* left, const CNA_Matrix* right)
{
    return memcmp(left, right, sizeof(*left)) == 0;
}

static int bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    char name[] = "MaterialValue";
    char semantic[] = "DEMO_VALUE";
    CNA_EffectParameterCreateInfo create_info;
    CNA_EffectParameterInfo info;
    CNA_EffectParameterHandle parameter = CNA_INVALID_HANDLE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle returned_texture = CNA_INVALID_HANDLE;
    CNA_Result result;
    uint64_t count = UINT64_C(0);
    uint64_t reported = UINT64_C(0);
    char text[20];
    CNA_Bool boolean_value = CNA_TRUE;
    int32_t integer_value = -42;
    float single_value = 3.5f;
    CNA_Matrix matrix = sequence_matrix(0.0f);
    CNA_Matrix matrix_output;
    CNA_Quaternion quaternion = {1.0f, 2.0f, 3.0f, 4.0f};
    CNA_Quaternion quaternion_output = {0.0f, 0.0f, 0.0f, 0.0f};
    CNA_Vector2 vector2 = {5.0f, 6.0f};
    CNA_Vector2 vector2_output = {0.0f, 0.0f};
    CNA_Vector3 vector3 = {7.0f, 8.0f, 9.0f};
    CNA_Vector3 vector3_output = {0.0f, 0.0f, 0.0f};
    CNA_Vector4 vector4 = {10.0f, 11.0f, 12.0f, 13.0f};
    CNA_Vector4 vector4_output = {0.0f, 0.0f, 0.0f, 0.0f};
    const CNA_Bool booleans[3] = {CNA_TRUE, CNA_FALSE, CNA_TRUE};
    CNA_Bool boolean_output[3] = {CNA_FALSE, CNA_FALSE, CNA_FALSE};
    const int32_t integers[3] = {-7, 8, 99};
    int32_t integer_output[3] = {111, 222, 333};
    const float singles[3] = {1.25f, 2.5f, 5.0f};
    float single_output[3] = {0.0f, 0.0f, 0.0f};
    const CNA_Matrix matrices[2] = {sequence_matrix(0.0f), sequence_matrix(20.0f)};
    CNA_Matrix matrix_array_output[2];
    const CNA_Quaternion quaternions[2] = {
        {1.0f, 2.0f, 3.0f, 4.0f}, {5.0f, 6.0f, 7.0f, 8.0f}};
    CNA_Quaternion quaternion_array_output[2];
    const CNA_Vector2 vector2_values[2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};
    CNA_Vector2 vector2_array_output[2];
    const CNA_Vector3 vector3_values[2] = {
        {1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}};
    CNA_Vector3 vector3_array_output[2];
    const CNA_Vector4 vector4_values[2] = {
        {1.0f, 2.0f, 3.0f, 4.0f}, {5.0f, 6.0f, 7.0f, 8.0f}};
    CNA_Vector4 vector4_array_output[2];
    const CNA_Color pixel = {10U, 20U, 30U, 255U};

    cna_demo_init(&demo, "c_api.graphics.effect_parameter_values", argc, argv, 0);

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.name = text_view(name);
    create_info.semantic = text_view(semantic);
    create_info.row_count = 1;
    create_info.column_count = 4;
    create_info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_VECTOR;
    create_info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_SINGLE;
    if (!cna_demo_check_result(
            &demo, cna_effect_parameter_create(&create_info, &parameter),
            "create an ownerless mutable effect parameter from copied metadata")) {
        goto cleanup;
    }
    memset(name, '#', sizeof(name) - 1U);
    memset(semantic, '#', sizeof(semantic) - 1U);

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_effect_parameter_get_info(parameter, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.row_count == 1 && info.column_count == 4 &&
            info.parameter_class == CNA_EFFECT_PARAMETER_CLASS_VECTOR &&
            info.parameter_type == CNA_EFFECT_PARAMETER_TYPE_SINGLE,
        "the parameter reports its complete copied shape and storage metadata");

    memset(text, 0x5A, sizeof(text));
    cna_demo_check(
        &demo,
        cna_effect_parameter_get_name_byte_count(parameter, &reported) == CNA_RESULT_SUCCESS &&
            reported == UINT64_C(13) &&
            cna_effect_parameter_copy_name(parameter, text, UINT64_C(12), &reported) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == UINT64_C(13) && bytes_are(text, sizeof(text), 0x5A),
        "a short parameter-name copy reports its byte count without a partial write");
    cna_demo_check(
        &demo,
        cna_effect_parameter_copy_name(parameter, text, UINT64_C(13), &reported) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, "MaterialValue", 13U) == 0 && (unsigned char)text[13] == 0x5A &&
            cna_effect_parameter_get_semantic_byte_count(parameter, &reported) ==
                CNA_RESULT_SUCCESS &&
            reported == UINT64_C(10) &&
            cna_effect_parameter_copy_semantic(parameter, text, UINT64_C(10), &reported) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, "DEMO_VALUE", 10U) == 0,
        "exact name and semantic copies retain the original unterminated UTF-8 bytes");

    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_BOOLEAN, &boolean_value);
    boolean_value = CNA_FALSE;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_BOOLEAN, &boolean_value) == CNA_RESULT_SUCCESS &&
            boolean_value == CNA_TRUE,
        "the Boolean scalar setter copies and round-trips a canonical value");
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_INT32, &integer_value);
    integer_value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_INT32, &integer_value) == CNA_RESULT_SUCCESS &&
            integer_value == -42,
        "the signed-integer scalar setter round-trips -42");
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_SINGLE, &single_value);
    single_value = 0.0f;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_SINGLE, &single_value) == CNA_RESULT_SUCCESS &&
            single_value == 3.5f,
        "the single-precision scalar setter round-trips 3.5");

    memset(&matrix_output, 0, sizeof(matrix_output));
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_MATRIX, &matrix);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_MATRIX, &matrix_output) == CNA_RESULT_SUCCESS &&
            same_matrix(&matrix, &matrix_output),
        "ordinary non-symmetric matrix storage round-trips without transposition");
    memset(&matrix_output, 0, sizeof(matrix_output));
    result = cna_effect_parameter_set_value(
        parameter, CNA_EFFECT_VALUE_MATRIX_TRANSPOSE, &matrix);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_MATRIX_TRANSPOSE, &matrix_output) ==
                CNA_RESULT_SUCCESS &&
            same_matrix(&matrix, &matrix_output) &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_MATRIX, &matrix_output) == CNA_RESULT_SUCCESS &&
            matrix_output.m12 == matrix.m21 && matrix_output.m21 == matrix.m12,
        "transposed matrix storage preserves its matching view and changes the ordinary view");

    result = cna_effect_parameter_set_value(
        parameter, CNA_EFFECT_VALUE_QUATERNION, &quaternion);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_QUATERNION, &quaternion_output) ==
                CNA_RESULT_SUCCESS &&
            memcmp(&quaternion, &quaternion_output, sizeof(quaternion)) == 0,
        "the Quaternion scalar family preserves all four components");
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_VECTOR2, &vector2);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_VECTOR2, &vector2_output) == CNA_RESULT_SUCCESS &&
            memcmp(&vector2, &vector2_output, sizeof(vector2)) == 0,
        "the Vector2 scalar family preserves both components");
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_VECTOR3, &vector3);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_VECTOR3, &vector3_output) == CNA_RESULT_SUCCESS &&
            memcmp(&vector3, &vector3_output, sizeof(vector3)) == 0,
        "the Vector3 scalar family preserves all three components");
    result = cna_effect_parameter_set_value(parameter, CNA_EFFECT_VALUE_VECTOR4, &vector4);
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(
                parameter, CNA_EFFECT_VALUE_VECTOR4, &vector4_output) == CNA_RESULT_SUCCESS &&
            memcmp(&vector4, &vector4_output, sizeof(vector4)) == 0,
        "the Vector4 scalar family preserves all four components");

    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_BOOLEAN, booleans, UINT64_C(3));
    memset(boolean_output, 0, sizeof(boolean_output));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_BOOLEAN, UINT64_C(3), boolean_output,
                UINT64_C(3), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(3) && memcmp(booleans, boolean_output, sizeof(booleans)) == 0,
        "the Boolean array family copies and returns all requested elements");

    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_INT32, integers, UINT64_C(3));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_INT32, UINT64_C(3), integer_output,
                UINT64_C(2), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(3) && integer_output[0] == 111 &&
            integer_output[1] == 222 && integer_output[2] == 333,
        "a short integer-array destination reports the count without changing any element");
    cna_demo_check(
        &demo,
        cna_effect_parameter_get_values(
            parameter, CNA_EFFECT_VALUE_INT32, UINT64_C(2), integer_output,
            UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && integer_output[0] == -7 && integer_output[1] == 8 &&
            integer_output[2] == 333,
        "a bounded integer-array request copies only its requested prefix");

    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_SINGLE, singles, UINT64_C(3));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_SINGLE, UINT64_C(3), single_output,
                UINT64_C(3), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(3) && memcmp(singles, single_output, sizeof(singles)) == 0,
        "the float array family round-trips three copied values");

    memset(matrix_array_output, 0, sizeof(matrix_array_output));
    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_MATRIX, matrices, UINT64_C(2));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_MATRIX, UINT64_C(2), matrix_array_output,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && memcmp(matrices, matrix_array_output, sizeof(matrices)) == 0,
        "ordinary matrix arrays preserve two distinct non-symmetric matrices");
    memset(matrix_array_output, 0, sizeof(matrix_array_output));
    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_MATRIX_TRANSPOSE, matrices, UINT64_C(2));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_MATRIX_TRANSPOSE, UINT64_C(2),
                matrix_array_output, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && memcmp(matrices, matrix_array_output, sizeof(matrices)) == 0,
        "transposed matrix arrays retain the caller's matching public values");

    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_QUATERNION, quaternions, UINT64_C(2));
    memset(quaternion_array_output, 0, sizeof(quaternion_array_output));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_QUATERNION, UINT64_C(2),
                quaternion_array_output, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(quaternions, quaternion_array_output, sizeof(quaternions)) == 0,
        "Quaternion arrays preserve both authored values");
    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_VECTOR2, vector2_values, UINT64_C(2));
    memset(vector2_array_output, 0, sizeof(vector2_array_output));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_VECTOR2, UINT64_C(2), vector2_array_output,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(vector2_values, vector2_array_output, sizeof(vector2_values)) == 0,
        "Vector2 arrays preserve both authored values");
    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_VECTOR3, vector3_values, UINT64_C(2));
    memset(vector3_array_output, 0, sizeof(vector3_array_output));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_VECTOR3, UINT64_C(2), vector3_array_output,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(vector3_values, vector3_array_output, sizeof(vector3_values)) == 0,
        "Vector3 arrays preserve both authored values");
    result = cna_effect_parameter_set_values(
        parameter, CNA_EFFECT_VALUE_VECTOR4, vector4_values, UINT64_C(2));
    memset(vector4_array_output, 0, sizeof(vector4_array_output));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_values(
                parameter, CNA_EFFECT_VALUE_VECTOR4, UINT64_C(2), vector4_array_output,
                UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(vector4_values, vector4_array_output, sizeof(vector4_values)) == 0,
        "Vector4 arrays preserve both authored values");

    strcpy(text, "copied-string");
    result = cna_effect_parameter_set_value_string(parameter, text_view(text));
    memset(text, 0x5A, sizeof(text));
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value_string_byte_count(parameter, &reported) ==
                CNA_RESULT_SUCCESS &&
            reported == UINT64_C(13) &&
            cna_effect_parameter_copy_value_string(parameter, text, UINT64_C(12), &reported) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == UINT64_C(13) && bytes_are(text, sizeof(text), 0x5A),
        "the copied string value reports a short destination without a partial write");
    cna_demo_check(
        &demo,
        cna_effect_parameter_copy_value_string(parameter, text, UINT64_C(13), &reported) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, "copied-string", 13U) == 0 && (unsigned char)text[13] == 0x5A,
        "the exact string copy returns the original bytes after caller storage changes");

    if (!cna_demo_check_result(
            &demo,
            cna_texture2d_create_cpu_only_rgba8(
                UINT32_C(1), UINT32_C(1), CNA_SURFACE_FORMAT_COLOR, &pixel,
                UINT64_C(1), &texture),
            "create a standalone CPU-only Texture2D for parameter retention")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_effect_parameter_set_value_texture(
            parameter, CNA_EFFECT_TEXTURE_2D, texture) == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value_texture(
                parameter, CNA_EFFECT_TEXTURE_2D, &returned_texture) == CNA_RESULT_SUCCESS &&
            returned_texture == texture && cna_texture2d_destroy(texture) == CNA_RESULT_INVALID_STATE,
        "the typed Texture2D slot returns and retains the exact owned handle");
    cna_demo_check(
        &demo,
        cna_effect_parameter_set_value_texture(
            parameter, CNA_EFFECT_TEXTURE_BASE, texture) == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value_texture(
                parameter, CNA_EFFECT_TEXTURE_BASE, &returned_texture) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_effect_parameter_set_value_texture(
                parameter, CNA_EFFECT_TEXTURE_2D, CNA_INVALID_HANDLE) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(texture) == CNA_RESULT_INVALID_STATE,
        "the separate base slot retains the texture and deliberately has no getter");
    result = cna_effect_parameter_set_value_texture(
        parameter, CNA_EFFECT_TEXTURE_BASE, CNA_INVALID_HANDLE);
    if (result == CNA_RESULT_SUCCESS && cna_texture2d_destroy(texture) == CNA_RESULT_SUCCESS) {
        texture = CNA_INVALID_HANDLE;
        cna_demo_check(
            &demo, 1,
            "clearing the last parameter slot releases the Texture2D for destruction");
    } else {
        cna_demo_check(
            &demo, 0,
            "clearing the last parameter slot releases the Texture2D for destruction");
    }

    result = cna_effect_parameter_destroy(parameter);
    if (result == CNA_RESULT_SUCCESS) {
        parameter = CNA_INVALID_HANDLE;
    }
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS, "the mutable parameter releases cleanly");

cleanup:
    if (texture != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_set_value_texture(
            parameter, CNA_EFFECT_TEXTURE_BASE, CNA_INVALID_HANDLE);
        (void)cna_effect_parameter_set_value_texture(
            parameter, CNA_EFFECT_TEXTURE_2D, CNA_INVALID_HANDLE);
        (void)cna_texture2d_destroy(texture);
    }
    if (parameter != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(parameter);
    }
    return cna_demo_finish(&demo);
}
