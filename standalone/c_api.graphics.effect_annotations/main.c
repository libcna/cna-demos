/* SPDX-License-Identifier: MS-PL */
/*
 * Effect annotations from C: immutable reflection values copy their UTF-8 metadata and raw
 * payload, expose typed views, and enter collections by value. Collection lookups return new
 * owned copies that remain valid independently of both their source handles and the collection.
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
    char name[] = "M\xc3\xa1" "trix";
    char semantic[] = "WORLD";
    char cached[] = "column-major";
    float matrix_data[16] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f};
    int32_t integer_bits = -42;
    float integer_data = 0.0f;
    CNA_EffectAnnotationCreateInfo create_info;
    CNA_EffectAnnotationInfo info;
    CNA_EffectAnnotationHandle matrix = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle integer = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle indexed = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle found_annotation = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_Result result;
    CNA_Bool boolean_value = CNA_FALSE;
    CNA_Bool found = CNA_FALSE;
    int32_t integer_value = 0;
    float single_value = 0.0f;
    CNA_Vector2 vector2 = {0.0f, 0.0f};
    CNA_Vector3 vector3 = {0.0f, 0.0f, 0.0f};
    CNA_Vector4 vector4 = {0.0f, 0.0f, 0.0f, 0.0f};
    CNA_Matrix matrix_value;
    uint64_t count = UINT64_C(0);
    uint64_t reported = UINT64_C(0);
    char copied[20];

    cna_demo_init(&demo, "c_api.graphics.effect_annotations", argc, argv, 0);

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.name = text_view(name);
    create_info.semantic = text_view(semantic);
    create_info.row_count = 4;
    create_info.column_count = 4;
    create_info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_MATRIX;
    create_info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_SINGLE;
    create_info.data = matrix_data;
    create_info.data_count = UINT64_C(16);
    create_info.cached_string = text_view(cached);
    if (!cna_demo_check_result(
            &demo, cna_effect_annotation_create(&create_info, &matrix),
            "create an immutable matrix annotation from caller-owned metadata and values")) {
        goto cleanup;
    }

    memset(name, '#', sizeof(name) - 1U);
    memset(semantic, '#', sizeof(semantic) - 1U);
    memset(cached, '#', sizeof(cached) - 1U);
    memset(matrix_data, 0, sizeof(matrix_data));
    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_info(matrix, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == sizeof(info) && info.struct_version == UINT32_C(1) &&
            info.row_count == 4 && info.column_count == 4 &&
            info.parameter_class == CNA_EFFECT_PARAMETER_CLASS_MATRIX &&
            info.parameter_type == CNA_EFFECT_PARAMETER_TYPE_SINGLE,
        "annotation metadata preserves its complete versioned matrix shape and type");

    memset(copied, 0x5A, sizeof(copied));
    reported = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_name_byte_count(matrix, &reported) == CNA_RESULT_SUCCESS &&
            reported == UINT64_C(7) &&
            cna_effect_annotation_copy_name(matrix, copied, UINT64_C(6), &reported) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            reported == UINT64_C(7) && bytes_are(copied, sizeof(copied), 0x5A),
        "the UTF-8 name reports bytes and a short copy leaves every destination byte unchanged");
    cna_demo_check(
        &demo,
        cna_effect_annotation_copy_name(matrix, copied, UINT64_C(7), &reported) ==
                CNA_RESULT_SUCCESS &&
            reported == UINT64_C(7) && memcmp(copied, "M\xc3\xa1" "trix", 7U) == 0 &&
            (unsigned char)copied[7] == 0x5A,
        "the exact-capacity name copy returns the original unterminated UTF-8 bytes");

    memset(copied, 0x5A, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_semantic_byte_count(matrix, &reported) == CNA_RESULT_SUCCESS &&
            reported == UINT64_C(5) &&
            cna_effect_annotation_copy_semantic(matrix, copied, UINT64_C(5), &reported) ==
                CNA_RESULT_SUCCESS &&
            memcmp(copied, "WORLD", 5U) == 0 && (unsigned char)copied[5] == 0x5A,
        "the semantic is an independently copied exact UTF-8 value");

    memset(copied, 0x5A, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_value_string_byte_count(matrix, &reported) ==
                CNA_RESULT_SUCCESS &&
            reported == UINT64_C(12) &&
            cna_effect_annotation_copy_value_string(matrix, copied, UINT64_C(12), &reported) ==
                CNA_RESULT_SUCCESS &&
            memcmp(copied, "column-major", 12U) == 0 && (unsigned char)copied[12] == 0x5A,
        "the cached string remains owned after the caller buffer is overwritten");

    memset(&matrix_value, 0, sizeof(matrix_value));
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_value_boolean(matrix, &boolean_value) == CNA_RESULT_SUCCESS &&
            boolean_value == CNA_TRUE &&
            cna_effect_annotation_get_value_int32(matrix, &integer_value) == CNA_RESULT_SUCCESS &&
            integer_value == INT32_C(1065353216) &&
            cna_effect_annotation_get_value_single(matrix, &single_value) == CNA_RESULT_SUCCESS &&
            single_value == 1.0f,
        "scalar views read the first copied raw value as Boolean, integer bits and float");
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_value_vector2(matrix, &vector2) == CNA_RESULT_SUCCESS &&
            vector2.x == 1.0f && vector2.y == 2.0f &&
            cna_effect_annotation_get_value_vector3(matrix, &vector3) == CNA_RESULT_SUCCESS &&
            vector3.x == 1.0f && vector3.y == 2.0f && vector3.z == 3.0f &&
            cna_effect_annotation_get_value_vector4(matrix, &vector4) == CNA_RESULT_SUCCESS &&
            vector4.x == 1.0f && vector4.y == 2.0f && vector4.z == 3.0f &&
            vector4.w == 4.0f,
        "vector views expose the first two, three and four copied float components");
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_value_matrix(matrix, &matrix_value) == CNA_RESULT_SUCCESS &&
            matrix_value.m11 == 1.0f && matrix_value.m12 == 5.0f &&
            matrix_value.m13 == 9.0f && matrix_value.m14 == 13.0f &&
            matrix_value.m21 == 2.0f && matrix_value.m32 == 7.0f &&
            matrix_value.m43 == 12.0f && matrix_value.m44 == 16.0f,
        "the matrix view maps all copied native columns into the public row-major value");

    memcpy(&integer_data, &integer_bits, sizeof(integer_data));
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.name = text_view("Answer");
    create_info.row_count = 1;
    create_info.column_count = 1;
    create_info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_SCALAR;
    create_info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_INT32;
    create_info.data = &integer_data;
    create_info.data_count = UINT64_C(1);
    if (!cna_demo_check_result(
            &demo, cna_effect_annotation_create(&create_info, &integer),
            "create a signed-integer annotation from its native four-byte storage")) {
        goto cleanup;
    }
    integer_data = 0.0f;
    cna_demo_check(
        &demo,
        cna_effect_annotation_get_value_int32(integer, &integer_value) == CNA_RESULT_SUCCESS &&
            integer_value == -42 &&
            cna_effect_annotation_get_value_boolean(integer, &boolean_value) == CNA_RESULT_SUCCESS &&
            boolean_value == CNA_TRUE,
        "the integer annotation retains -42 and exposes its nonzero Boolean view");

    cna_demo_check(
        &demo,
        cna_effect_annotation_collection_create(&collection) == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_annotation_collection_add(collection, matrix) == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_add(collection, integer) == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "an owned collection preserves insertion order and adds annotations by value");

    result = cna_effect_annotation_destroy(matrix);
    if (result == CNA_RESULT_SUCCESS) {
        matrix = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, result == CNA_RESULT_SUCCESS,
        "the source matrix handle can be destroyed after its value enters the collection");
    result = cna_effect_annotation_destroy(integer);
    if (result == CNA_RESULT_SUCCESS) {
        integer = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, result == CNA_RESULT_SUCCESS,
        "the source integer handle can be destroyed after its value enters the collection");

    cna_demo_check(
        &demo,
        cna_effect_annotation_collection_get_at(collection, UINT64_C(0), &indexed) ==
                CNA_RESULT_SUCCESS &&
            indexed != CNA_INVALID_HANDLE &&
            cna_effect_annotation_get_value_matrix(indexed, &matrix_value) == CNA_RESULT_SUCCESS &&
            matrix_value.m14 == 13.0f && matrix_value.m44 == 16.0f,
        "index zero returns a new owned copy of the first annotation");
    cna_demo_check(
        &demo,
        cna_effect_annotation_collection_find(
            collection, text_view("Answer"), &found, &found_annotation) == CNA_RESULT_SUCCESS &&
            found == CNA_TRUE && found_annotation != CNA_INVALID_HANDLE &&
            cna_effect_annotation_get_value_int32(found_annotation, &integer_value) ==
                CNA_RESULT_SUCCESS &&
            integer_value == -42,
        "exact-name lookup returns a new owned copy of the matching annotation");

    found = CNA_TRUE;
    matrix = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_effect_annotation_collection_find(
            collection, text_view("answer"), &found, &matrix) == CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && matrix == CNA_INVALID_HANDLE,
        "annotation lookup is case-sensitive and reports absence without a handle");
    matrix = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_effect_annotation_collection_get_at(collection, UINT64_C(2), &matrix) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            matrix == CNA_INVALID_HANDLE,
        "an out-of-range collection index is refused before publishing a handle");
    matrix = CNA_INVALID_HANDLE;

    result = cna_effect_annotation_collection_destroy(collection);
    if (result == CNA_RESULT_SUCCESS) {
        collection = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_get_value_matrix(indexed, &matrix_value) == CNA_RESULT_SUCCESS &&
            matrix_value.m14 == 13.0f &&
            cna_effect_annotation_get_value_int32(found_annotation, &integer_value) ==
                CNA_RESULT_SUCCESS &&
            integer_value == -42,
        "owned index and lookup copies remain usable after their collection is destroyed");

    matrix = indexed;
    result = cna_effect_annotation_destroy(indexed);
    if (result == CNA_RESULT_SUCCESS) {
        indexed = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_destroy(matrix) == CNA_RESULT_INVALID_HANDLE,
        "an owned annotation copy destroys exactly once and then becomes stale");
    matrix = CNA_INVALID_HANDLE;
    result = cna_effect_annotation_destroy(found_annotation);
    if (result == CNA_RESULT_SUCCESS) {
        found_annotation = CNA_INVALID_HANDLE;
    }
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS, "the independent lookup copy releases cleanly");

cleanup:
    if (found_annotation != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(found_annotation);
    }
    if (indexed != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(indexed);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_collection_destroy(collection);
    }
    if (integer != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(integer);
    }
    if (matrix != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(matrix);
    }
    return cna_demo_finish(&demo);
}
