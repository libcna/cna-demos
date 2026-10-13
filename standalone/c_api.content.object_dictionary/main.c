/* SPDX-License-Identifier: MS-PL */
/* XNB object-dictionary inspection, fixed-layout values, arrays and foreign objects in C. */
#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define CUSTOM_TYPE_NAME "CNA.Demos.DictionaryEntry"

typedef struct CustomValue {
    int32_t magic;
    float weight;
} CustomValue;

typedef struct State {
    CnaDemo* demo;
    CNA_Handle manager;
    int validated;
    int custom_create_calls;
    CustomValue custom;
} State;

typedef struct ExpectedEntry {
    const char* key;
    CNA_ObjectDictionaryValueKind kind;
    CNA_Bool is_array;
    uint64_t count;
} ExpectedEntry;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static CNA_Result create_custom(void* context, void** out_object)
{
    State* state = (State*)context;
    ++state->custom_create_calls;
    memset(&state->custom, 0, sizeof(state->custom));
    *out_object = &state->custom;
    return CNA_RESULT_SUCCESS;
}

static int register_custom_reader(State* state)
{
    CNA_ReflectiveTypeReaderBuilderHandle builder = CNA_INVALID_HANDLE;
    int ok;
    if (cna_reflective_type_reader_builder_create(
            view(CUSTOM_TYPE_NAME), create_custom, state, &builder) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    ok = cna_reflective_type_reader_builder_add_field(
             builder, CNA_CONTENT_FIELD_INT32, offsetof(CustomValue, magic)) == CNA_RESULT_SUCCESS &&
        cna_reflective_type_reader_builder_add_field(
            builder, CNA_CONTENT_FIELD_SINGLE, offsetof(CustomValue, weight)) == CNA_RESULT_SUCCESS &&
        cna_reflective_type_reader_builder_register_shared(builder) == CNA_RESULT_SUCCESS;
    return cna_reflective_type_reader_builder_destroy(builder) == CNA_RESULT_SUCCESS && ok;
}

static int check_entry(
    CNA_ObjectDictionaryHandle dictionary,
    const ExpectedEntry* expected)
{
    CNA_ObjectDictionaryEntry entry;
    memset(&entry, 0, sizeof(entry));
    entry.struct_size = (uint32_t)sizeof(entry);
    return cna_object_dictionary_ext_get_entry(dictionary, view(expected->key), &entry) ==
            CNA_RESULT_SUCCESS &&
        entry.struct_size == sizeof(entry) && entry.struct_version == UINT32_C(1) &&
        entry.kind == expected->kind && entry.is_array == expected->is_array &&
        entry.element_count == expected->count;
}

static CNA_Result inspect_dictionary(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    static const char RuntimeType[] =
        "System.Collections.Generic.Dictionary`2[System.String,System.Object]";
    static const ExpectedEntry Entries[] = {
        {"Array", CNA_OBJECT_DICTIONARY_VALUE_VECTOR3, CNA_TRUE, UINT64_C(3)},
        {"Boolean", CNA_OBJECT_DICTIONARY_VALUE_BOOLEAN, CNA_FALSE, UINT64_C(1)},
        {"Box", CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_BOX, CNA_FALSE, UINT64_C(1)},
        {"Color", CNA_OBJECT_DICTIONARY_VALUE_COLOR, CNA_FALSE, UINT64_C(1)},
        {"Custom", CNA_OBJECT_DICTIONARY_VALUE_FOREIGN_OBJECT, CNA_FALSE, UINT64_C(1)},
        {"Double", CNA_OBJECT_DICTIONARY_VALUE_DOUBLE, CNA_FALSE, UINT64_C(1)},
        {"Integer", CNA_OBJECT_DICTIONARY_VALUE_INT32, CNA_FALSE, UINT64_C(1)},
        {"Matrix", CNA_OBJECT_DICTIONARY_VALUE_MATRIX, CNA_FALSE, UINT64_C(1)},
        {"Name", CNA_OBJECT_DICTIONARY_VALUE_STRING, CNA_FALSE, UINT64_C(1)},
        {"Quaternion", CNA_OBJECT_DICTIONARY_VALUE_QUATERNION, CNA_FALSE, UINT64_C(1)},
        {"Single", CNA_OBJECT_DICTIONARY_VALUE_SINGLE, CNA_FALSE, UINT64_C(1)},
        {"Sphere", CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_SPHERE, CNA_FALSE, UINT64_C(1)},
        {"UnknownUInt32", CNA_OBJECT_DICTIONARY_VALUE_UNKNOWN, CNA_FALSE, UINT64_C(1)},
        {"Vector2", CNA_OBJECT_DICTIONARY_VALUE_VECTOR2, CNA_FALSE, UINT64_C(1)},
        {"Vector3", CNA_OBJECT_DICTIONARY_VALUE_VECTOR3, CNA_FALSE, UINT64_C(1)},
        {"Vector4", CNA_OBJECT_DICTIONARY_VALUE_VECTOR4, CNA_FALSE, UINT64_C(1)},
    };
    State* state = (State*)context;
    CNA_ContentManagerCreateInfo create_info;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ObjectDictionaryHandle dictionary = CNA_INVALID_HANDLE;
    CNA_ObjectDictionaryHandle second = CNA_INVALID_HANDLE;
    CNA_ObjectDictionaryHandle rejected = UINT64_C(99);
    CNA_Bool present = CNA_FALSE;
    CNA_Bool boolean = CNA_FALSE;
    CNA_Vector2 vector2;
    CNA_Vector3 vector3;
    CNA_Vector4 vector4;
    CNA_Vector3 array[3];
    CNA_Matrix matrix;
    CNA_Quaternion quaternion;
    CNA_Color color;
    CNA_BoundingSphere sphere;
    CNA_BoundingBox box;
    CNA_ObjectDictionaryEntry malformed;
    int32_t integer = 0;
    float single = 0.0F;
    double real = 0.0;
    uint64_t count = 0U;
    uint64_t bytes = 0U;
    uint64_t i;
    char text[192];
    char before[192];
    void* object = NULL;
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "borrow the game graphics device")) {
        return CNA_RESULT_SUCCESS;
    }
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.root_directory = view("assets");
    if (!cna_demo_check_result(
            state->demo, cna_content_manager_create(device, &create_info, &state->manager),
            "create the dictionary content manager")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_content_manager_register_builtin_loaders(state->manager) == CNA_RESULT_SUCCESS &&
            register_custom_reader(state),
        "register built-in XNB readers and the shared caller-created entry reader");

    cna_demo_check(
        state->demo,
        cna_content_manager_load_object_dictionary_ext(
            state->manager, view("dictionaries/absent"), &rejected) == CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE &&
            cna_content_manager_load_object_dictionary_ext(
                state->manager, view(""), &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "dictionary loading rejects missing and empty asset names without returning a handle");
    if (!cna_demo_check_result(
            state->demo,
            cna_content_manager_load_object_dictionary_ext(
                state->manager, view("dictionaries/member_audit"), &dictionary),
            "load the deterministic object-dictionary XNB")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_content_manager_load_object_dictionary_ext(
            state->manager, view("dictionaries/member_audit"), &second) == CNA_RESULT_SUCCESS &&
            second != CNA_INVALID_HANDLE && second != dictionary && state->custom_create_calls == 1,
        "a cached asset produces an independent dictionary handle without rerunning entry readers");
    if (second != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_object_dictionary_ext_destroy(second),
                              "destroy the second independent dictionary handle");
        second = CNA_INVALID_HANDLE;
    }

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_runtime_type_name_size(dictionary, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen(RuntimeType) &&
            cna_object_dictionary_ext_copy_runtime_type_name(dictionary, text, bytes, &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, RuntimeType, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the logical managed dictionary type uses exact unterminated size and copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_runtime_type_name(dictionary, text, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short runtime-type destination is rejected without a partial write");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == sizeof(Entries) / sizeof(Entries[0]) &&
            cna_object_dictionary_ext_contains_key(dictionary, view("Sphere"), &present) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_TRUE &&
            cna_object_dictionary_ext_contains_key(dictionary, view("Missing"), &present) ==
                CNA_RESULT_SUCCESS &&
            present == CNA_FALSE,
        "count and membership distinguish the sixteen authored entries from an absent key");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_key_size_at(dictionary, 0U, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen("Array") &&
            cna_object_dictionary_ext_copy_key_at(dictionary, 0U, text, bytes, &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, "Array", (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U &&
            cna_object_dictionary_ext_get_key_size_at(dictionary, count, &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "indexed enumeration exposes the map's first sorted key and refuses its end index");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_key_at(dictionary, count - 1U, text, 2U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == strlen("Vector4") && memcmp(text, before, sizeof(text)) == 0,
        "a short indexed-key destination reports the last key size without a partial write");

    for (i = 0U; i < sizeof(Entries) / sizeof(Entries[0]); ++i) {
        cna_demo_check(state->demo, check_entry(dictionary, &Entries[i]), Entries[i].key);
    }
    memset(&malformed, 0, sizeof(malformed));
    malformed.struct_size = 1U;
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_entry(dictionary, view("Name"), &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_object_dictionary_ext_get_entry(dictionary, view("Missing"), &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "entry inspection refuses an undersized output and an absent key");

    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_value(
            dictionary, view("Boolean"), CNA_OBJECT_DICTIONARY_VALUE_BOOLEAN,
            &boolean, sizeof(boolean)) == CNA_RESULT_SUCCESS && boolean == CNA_TRUE &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Integer"), CNA_OBJECT_DICTIONARY_VALUE_INT32,
                &integer, sizeof(integer)) == CNA_RESULT_SUCCESS && integer == -7 &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Single"), CNA_OBJECT_DICTIONARY_VALUE_SINGLE,
                &single, sizeof(single)) == CNA_RESULT_SUCCESS && single == 1.25F &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Double"), CNA_OBJECT_DICTIONARY_VALUE_DOUBLE,
                &real, sizeof(real)) == CNA_RESULT_SUCCESS && real == 2.5,
        "scalar value copying preserves bool, signed integer, single and double values");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_value(
            dictionary, view("Vector2"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR2,
            &vector2, sizeof(vector2)) == CNA_RESULT_SUCCESS &&
            vector2.x == 2.0F && vector2.y == 3.0F &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Vector3"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR3,
                &vector3, sizeof(vector3)) == CNA_RESULT_SUCCESS &&
            vector3.x == 4.0F && vector3.y == 5.0F && vector3.z == 6.0F &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Vector4"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR4,
                &vector4, sizeof(vector4)) == CNA_RESULT_SUCCESS &&
            vector4.x == 7.0F && vector4.y == 8.0F && vector4.z == 9.0F && vector4.w == 10.0F,
        "scalar value copying preserves every vector layout");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_value(
            dictionary, view("Matrix"), CNA_OBJECT_DICTIONARY_VALUE_MATRIX,
            &matrix, sizeof(matrix)) == CNA_RESULT_SUCCESS &&
            matrix.m11 == 1.0F && matrix.m14 == 4.0F && matrix.m22 == 6.0F &&
            matrix.m33 == 11.0F && matrix.m41 == 13.0F && matrix.m44 == 16.0F &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Quaternion"), CNA_OBJECT_DICTIONARY_VALUE_QUATERNION,
                &quaternion, sizeof(quaternion)) == CNA_RESULT_SUCCESS &&
            quaternion.x == 0.25F && quaternion.y == 0.5F && quaternion.z == 0.75F &&
            quaternion.w == 1.0F,
        "scalar value copying preserves matrix row order and quaternion component order");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_value(
            dictionary, view("Color"), CNA_OBJECT_DICTIONARY_VALUE_COLOR,
            &color, sizeof(color)) == CNA_RESULT_SUCCESS &&
            color.r == 10U && color.g == 20U && color.b == 30U && color.a == 40U &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Sphere"), CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_SPHERE,
                &sphere, sizeof(sphere)) == CNA_RESULT_SUCCESS &&
            sphere.center.x == 1.0F && sphere.center.y == 2.0F && sphere.center.z == 3.0F &&
            sphere.radius == 4.0F &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Box"), CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_BOX,
                &box, sizeof(box)) == CNA_RESULT_SUCCESS &&
            box.min.x == -1.0F && box.min.y == -2.0F && box.min.z == -3.0F &&
            box.max.x == 4.0F && box.max.y == 5.0F && box.max.z == 6.0F,
        "scalar value copying preserves color, sphere and bounding-box layouts");
    memset(&sphere, 0xA5, sizeof(sphere));
    memcpy(before, &sphere, sizeof(sphere));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_value(
            dictionary, view("Sphere"), CNA_OBJECT_DICTIONARY_VALUE_MATRIX,
            &sphere, sizeof(sphere)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Sphere"), CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_SPHERE,
                &sphere, sizeof(sphere) - 1U) == CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(&sphere, before, sizeof(sphere)) == 0 &&
            cna_object_dictionary_ext_copy_value(
                dictionary, view("Array"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR3,
                array, sizeof(array)) == CNA_RESULT_INVALID_ARGUMENT,
        "scalar copying refuses a wrong kind, an undersized destination and an array");

    memset(array, 0xA5, sizeof(array));
    memcpy(before, array, sizeof(array));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_array(
            dictionary, view("Array"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR3,
            array, sizeof(array) - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == sizeof(array) && memcmp(array, before, sizeof(array)) == 0,
        "an undersized array destination reports the exact size without a partial write");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_array(
            dictionary, view("Array"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR3,
            array, sizeof(array), &bytes) == CNA_RESULT_SUCCESS && bytes == sizeof(array) &&
            array[0].x == 0.0F && array[0].y == 0.0F && array[0].z == 0.0F &&
            array[1].x == 1.0F && array[1].y == 0.0F && array[1].z == 0.0F &&
            array[2].x == 0.0F && array[2].y == 1.0F && array[2].z == 0.0F &&
            cna_object_dictionary_ext_copy_array(
                dictionary, view("Array"), CNA_OBJECT_DICTIONARY_VALUE_VECTOR4,
                array, sizeof(array), &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_object_dictionary_ext_copy_array(
                dictionary, view("Sphere"), CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_SPHERE,
                array, sizeof(array), &bytes) == CNA_RESULT_INVALID_ARGUMENT,
        "array copying preserves every Vector3 and rejects a wrong element kind and scalar entry");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_string_size(dictionary, view("Name"), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen("triangles") &&
            cna_object_dictionary_ext_copy_string(dictionary, view("Name"), text, bytes, &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, "triangles", (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "string entries use exact unterminated size and copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_string(dictionary, view("Name"), text, 2U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(text, before, sizeof(text)) == 0 &&
            cna_object_dictionary_ext_get_string_size(dictionary, view("Array"), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "a short string destination is atomic and a non-string entry is refused");
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_foreign_object(dictionary, view("Custom"), &object) ==
                CNA_RESULT_SUCCESS &&
            object == &state->custom && state->custom.magic == 1234 && state->custom.weight == 0.5F &&
            cna_object_dictionary_ext_get_foreign_object(dictionary, view("Name"), &object) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the foreign entry returns the caller's object while a string is refused");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_type_name_size(
            dictionary, view("UnknownUInt32"), &bytes) == CNA_RESULT_SUCCESS &&
            bytes > 0U && bytes < sizeof(text) &&
            cna_object_dictionary_ext_copy_type_name(
                dictionary, view("UnknownUInt32"), text, bytes, &bytes) == CNA_RESULT_SUCCESS &&
            (unsigned char)text[bytes] == 0xA5U,
        "an unknown entry still exposes a nonempty exact diagnostic C++ type name");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_copy_type_name(
            dictionary, view("UnknownUInt32"), text, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short diagnostic-type destination is rejected without a partial write");

    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_destroy(dictionary) == CNA_RESULT_SUCCESS &&
            cna_object_dictionary_ext_destroy(dictionary) == CNA_RESULT_INVALID_HANDLE &&
            cna_object_dictionary_ext_get_count(dictionary, &count) == CNA_RESULT_INVALID_HANDLE,
        "the dictionary handle releases exactly once and then rejects member access");
    dictionary = CNA_INVALID_HANDLE;
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.content.object_dictionary", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.manager = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = inspect_dictionary;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game),
                              "create the object-dictionary game")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run its dictionary inspection callback");
        cna_demo_check(&demo, state.validated == 1,
                       "the object-dictionary audit ran exactly once");
    }
    if (state.manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_manager_destroy(state.manager),
                              "destroy the dictionary content manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game),
                              "destroy the object-dictionary game");
    }
    return cna_demo_finish(&demo);
}
