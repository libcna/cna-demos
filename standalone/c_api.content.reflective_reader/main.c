/* SPDX-License-Identifier: MS-PL */
/* Reflective content-reader builders, every inline field kind and both result shapes in C. */
#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define VALUE_TYPE_NAME "CNA.Demos.AllFieldKinds"
#define ENUM_TYPE_NAME "CNA.Demos.FieldMode"
#define SHARED_TYPE_NAME "CNA.Demos.SharedValue"

static const char ContainerName[] = "C API Reflective Reader";

typedef struct AllFields {
    uint8_t boolean;
    float single;
    double real;
    int32_t signed32;
    uint32_t unsigned32;
    int64_t signed64;
    uint8_t byte;
    CNA_Vector2 vector2;
    CNA_Vector3 vector3;
    CNA_Vector4 vector4;
    CNA_Matrix matrix;
    CNA_Quaternion quaternion;
    CNA_Color color;
    int64_t timespan_ticks;
    int32_t mode;
    int32_t custom;
    int create_calls;
    int custom_calls;
    int borrowed_usable;
    CNA_ContentReaderHandle borrowed;
} AllFields;

typedef struct SharedValue {
    int32_t value;
    int create_calls;
} SharedValue;

static AllFields g_fields;
static SharedValue g_shared;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void push_u8(uint8_t* data, size_t* offset, uint8_t value)
{
    data[(*offset)++] = value;
}

static void push_u32(uint8_t* data, size_t* offset, uint32_t value)
{
    push_u8(data, offset, (uint8_t)(value & UINT32_C(0xff)));
    push_u8(data, offset, (uint8_t)((value >> 8U) & UINT32_C(0xff)));
    push_u8(data, offset, (uint8_t)((value >> 16U) & UINT32_C(0xff)));
    push_u8(data, offset, (uint8_t)((value >> 24U) & UINT32_C(0xff)));
}

static void push_u64(uint8_t* data, size_t* offset, uint64_t value)
{
    int shift;
    for (shift = 0; shift < 64; shift += 8) {
        push_u8(data, offset, (uint8_t)((value >> (unsigned)shift) & UINT64_C(0xff)));
    }
}

static void push_float(uint8_t* data, size_t* offset, float value)
{
    uint32_t bits = 0U;
    memcpy(&bits, &value, sizeof(bits));
    push_u32(data, offset, bits);
}

static void push_double(uint8_t* data, size_t* offset, double value)
{
    uint64_t bits = 0U;
    memcpy(&bits, &value, sizeof(bits));
    push_u64(data, offset, bits);
}

static size_t build_all_fields(uint8_t data[166])
{
    size_t offset = 0U;
    int value;
    push_u8(data, &offset, 1U);
    push_float(data, &offset, 1.25F);
    push_double(data, &offset, 2.5);
    push_u32(data, &offset, (uint32_t)INT32_C(-3));
    push_u32(data, &offset, UINT32_C(4));
    push_u64(data, &offset, (uint64_t)INT64_C(-5));
    push_u8(data, &offset, 6U);
    for (value = 7; value <= 31; ++value) {
        push_float(data, &offset, (float)value);
    }
    push_float(data, &offset, 0.25F);
    push_float(data, &offset, 0.5F);
    push_float(data, &offset, 0.75F);
    push_float(data, &offset, 1.0F);
    push_u8(data, &offset, 10U);
    push_u8(data, &offset, 20U);
    push_u8(data, &offset, 30U);
    push_u8(data, &offset, 40U);
    push_u64(data, &offset, UINT64_C(123456789));
    push_u32(data, &offset, UINT32_C(2));
    push_u32(data, &offset, UINT32_C(21));
    return offset;
}

static CNA_Result create_fields(void* context, void** out_object)
{
    AllFields* fields = (AllFields*)context;
    memset(fields, 0, sizeof(*fields));
    fields->create_calls = 1;
    *out_object = fields;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result read_custom(
    void* context,
    void* object,
    CNA_ContentReaderHandle input)
{
    AllFields* fields = (AllFields*)object;
    uint8_t raw[4];
    uint64_t bytes = 0U;
    int32_t version = -1;
    (void)context;
    fields->borrowed = input;
    fields->borrowed_usable =
        cna_content_reader_get_version(input, &version) == CNA_RESULT_SUCCESS && version == 5 &&
        cna_content_reader_destroy(input) == CNA_RESULT_INVALID_STATE;
    if (cna_content_reader_read_bytes_exact(
            input, 4, view("reflective custom field"), raw, sizeof(raw), &bytes) !=
            CNA_RESULT_SUCCESS ||
        bytes != 4U) {
        return CNA_RESULT_IO;
    }
    fields->custom = 2 * (int32_t)((uint32_t)raw[0] | ((uint32_t)raw[1] << 8U) |
                                    ((uint32_t)raw[2] << 16U) | ((uint32_t)raw[3] << 24U));
    ++fields->custom_calls;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result create_shared(void* context, void** out_object)
{
    SharedValue* shared = (SharedValue*)context;
    memset(shared, 0, sizeof(*shared));
    shared->create_calls = 1;
    *out_object = shared;
    return CNA_RESULT_SUCCESS;
}

static int create_payload_reader(
    CNA_StorageContainerHandle container,
    const char* file_name,
    const uint8_t* payload,
    uint64_t size,
    CNA_StorageStreamHandle* out_stream,
    CNA_ContentReaderHandle* out_reader)
{
    CNA_ContentReaderCreateInfo create_info;
    if (cna_storage_container_create_file(container, view(file_name), out_stream) !=
            CNA_RESULT_SUCCESS ||
        cna_storage_stream_write(*out_stream, payload, size) != CNA_RESULT_SUCCESS ||
        cna_storage_stream_close(*out_stream) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    *out_stream = CNA_INVALID_HANDLE;
    if (cna_storage_container_open_file(
            container, view(file_name), CNA_FILE_MODE_OPEN, out_stream) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.stream = *out_stream;
    create_info.asset_name = view(file_name);
    create_info.version = 5;
    create_info.platform = (uint8_t)'w';
    return cna_content_reader_create(&create_info, out_reader) == CNA_RESULT_SUCCESS;
}

static int release_payload_reader(
    CNA_ContentReaderHandle* reader,
    CNA_StorageStreamHandle* stream)
{
    int ok = 1;
    if (*reader != CNA_INVALID_HANDLE) {
        ok = cna_content_reader_destroy(*reader) == CNA_RESULT_SUCCESS;
        *reader = CNA_INVALID_HANDLE;
    }
    if (*stream != CNA_INVALID_HANDLE) {
        ok = cna_storage_stream_close(*stream) == CNA_RESULT_SUCCESS && ok;
        *stream = CNA_INVALID_HANDLE;
    }
    return ok;
}

int main(int argc, char** argv)
{
    static const char ReflectiveName[] =
        "Microsoft.Xna.Framework.Content.ReflectiveReader`1[[CNA.Demos.AllFieldKinds]]";
    static const char EnumName[] =
        "Microsoft.Xna.Framework.Content.EnumReader`1[[CNA.Demos.FieldMode]]";
    static const char SharedName[] =
        "Microsoft.Xna.Framework.Content.ReflectiveReader`1[[CNA.Demos.SharedValue]]";
    uint8_t value_payload[166];
    uint8_t shared_payload[4];
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    CNA_ContentReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_ContentTypeReaderHandle type_reader = CNA_INVALID_HANDLE;
    CNA_ReflectiveTypeReaderBuilderHandle builder = CNA_INVALID_HANDLE;
    CNA_ReflectiveTypeReaderBuilderHandle shared_builder = CNA_INVALID_HANDLE;
    CNA_ReflectiveTypeReaderBuilderHandle rejected = UINT64_C(99);
    CNA_Bool flag = CNA_FALSE;
    CNA_Bool has_value = CNA_FALSE;
    CnaDemo demo;
    uint64_t bytes = 0U;
    int32_t version = -1;
    char name[192];
    char before[192];
    size_t offset = 0U;

    cna_demo_init(&demo, "c_api.content.reflective_reader", argc, argv, 0);
    cna_demo_check(&demo, build_all_fields(value_payload) == sizeof(value_payload),
                   "the all-field payload has its documented byte layout");
    push_u32(shared_payload, &offset, UINT32_C(9));
    cna_demo_check(&demo, offset == sizeof(shared_payload),
                   "the shared-reader payload has its documented byte layout");
    cna_demo_check_result(&demo, cna_content_type_reader_manager_clear_type_creators(),
                          "start with an isolated empty type-reader registry");

    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_get_canonical_name_size(view(VALUE_TYPE_NAME), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen(ReflectiveName) &&
            cna_reflective_type_reader_copy_canonical_name(
                view(VALUE_TYPE_NAME), name, bytes, &bytes) == CNA_RESULT_SUCCESS &&
            memcmp(name, ReflectiveName, (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "the reflective canonical name uses exact unterminated size and copy semantics");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_copy_canonical_name(
            view(VALUE_TYPE_NAME), name, bytes - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(name, before, sizeof(name)) == 0,
        "a short reflective-name destination is rejected without a partial write");
    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_enum_type_reader_get_canonical_name_size(view(ENUM_TYPE_NAME), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen(EnumName) &&
            cna_enum_type_reader_copy_canonical_name(view(ENUM_TYPE_NAME), name, bytes, &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(name, EnumName, (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "the enum canonical name uses exact unterminated size and copy semantics");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_enum_type_reader_copy_canonical_name(
            view(ENUM_TYPE_NAME), name, bytes - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(name, before, sizeof(name)) == 0,
        "a short enum-name destination is rejected without a partial write");

    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_create(view(VALUE_TYPE_NAME), NULL, NULL, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE &&
            cna_reflective_type_reader_builder_create(view(""), create_fields, &g_fields, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "builder creation rejects a missing factory and an empty target atomically");
    if (!cna_demo_check_result(
            &demo,
            cna_reflective_type_reader_builder_create(
                view(VALUE_TYPE_NAME), create_fields, &g_fields, &builder),
            "create the all-field reflective reader builder")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_add_field(
            builder, CNA_CONTENT_FIELD_MAXIMUM + 1U, 0U) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_reflective_type_reader_builder_add_enum_field(builder, 0U, view("")) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_reflective_type_reader_builder_add_custom(builder, NULL, NULL) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "field declarations reject an unknown kind, empty enum and null callback");
#define ADD_FIELD(kind, member) \
    cna_reflective_type_reader_builder_add_field(builder, kind, offsetof(AllFields, member))
    cna_demo_check(
        &demo,
        ADD_FIELD(CNA_CONTENT_FIELD_BOOLEAN, boolean) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_SINGLE, single) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_DOUBLE, real) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_INT32, signed32) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_UINT32, unsigned32) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_INT64, signed64) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_BYTE, byte) == CNA_RESULT_SUCCESS,
        "declare every scalar reflective field kind in wire order");
    cna_demo_check(
        &demo,
        ADD_FIELD(CNA_CONTENT_FIELD_VECTOR2, vector2) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_VECTOR3, vector3) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_VECTOR4, vector4) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_MATRIX, matrix) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_QUATERNION, quaternion) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_COLOR, color) == CNA_RESULT_SUCCESS &&
            ADD_FIELD(CNA_CONTENT_FIELD_TIMESPAN, timespan_ticks) == CNA_RESULT_SUCCESS,
        "declare every structured and time reflective field kind in wire order");
#undef ADD_FIELD
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_add_enum_field(
            builder, offsetof(AllFields, mode), view(ENUM_TYPE_NAME)) == CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_add_custom(builder, read_custom, NULL) ==
                CNA_RESULT_SUCCESS,
        "declare an inline named enum followed by a caller-read custom member");
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_register(builder) == CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_add_field(
                builder, CNA_CONTENT_FIELD_INT32, offsetof(AllFields, create_calls)) ==
                CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_register(builder) == CNA_RESULT_SUCCESS,
        "value registration is idempotent and retains the first field-list snapshot");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_get_is_registered(view(ReflectiveName), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_content_type_reader_manager_get_is_registered(view(EnumName), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "registration publishes both the reflective reader and its enum reader");
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_destroy(builder) == CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_destroy(builder) == CNA_RESULT_INVALID_HANDLE &&
            cna_reflective_type_reader_builder_add_field(
                builder, CNA_CONTENT_FIELD_INT32, 0U) == CNA_RESULT_INVALID_HANDLE,
        "builder release is one-shot and invalidates further declarations");
    builder = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_storage_set_app_name_ext(view("cna-demos-reflective-reader")) == CNA_RESULT_SUCCESS &&
            cna_storage_device_show_selector(NULL, NULL, &device) == CNA_RESULT_SUCCESS,
        "select isolated storage for the reflective payloads");
    if (device == CNA_INVALID_HANDLE) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_storage_device_delete_container(device, view(ContainerName)),
        "remove any previous reflective-reader fixture");
    if (!cna_demo_check_result(
            &demo, cna_storage_container_open(device, view(ContainerName), NULL, NULL, &container),
            "open a fresh reflective-reader fixture")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        create_payload_reader(
            container, "all-fields.bin", value_payload, sizeof(value_payload), &stream, &reader),
        "create a content reader over the all-field payload");
    if (reader == CNA_INVALID_HANDLE) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo, cna_content_type_reader_manager_create_reader(view(ReflectiveName), &type_reader),
            "create the registered value-shaped reflective reader")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_content_type_reader_read_untyped(type_reader, reader, &has_value) == CNA_RESULT_SUCCESS &&
            has_value == CNA_TRUE && g_fields.create_calls == 1 && g_fields.custom_calls == 1 &&
            g_fields.borrowed_usable == 1,
        "the value-shaped reader creates an object and runs its custom borrowed-reader callback");
    cna_demo_check(
        &demo,
        cna_content_reader_get_version(g_fields.borrowed, &version) == CNA_RESULT_INVALID_HANDLE,
        "the reflective custom callback's reader handle expires on return");
    cna_demo_check(
        &demo,
        g_fields.boolean == 1U && g_fields.single == 1.25F && g_fields.real == 2.5 &&
            g_fields.signed32 == -3 && g_fields.unsigned32 == 4U && g_fields.signed64 == -5 &&
            g_fields.byte == 6U && g_fields.timespan_ticks == INT64_C(123456789) &&
            g_fields.mode == 2 && g_fields.custom == 42,
        "all scalar, timespan, enum and custom values land at their declared offsets");
    cna_demo_check(
        &demo,
        g_fields.vector2.x == 7.0F && g_fields.vector2.y == 8.0F &&
            g_fields.vector3.x == 9.0F && g_fields.vector3.z == 11.0F &&
            g_fields.vector4.x == 12.0F && g_fields.vector4.w == 15.0F,
        "all vector field kinds preserve their canonical component order");
    cna_demo_check(
        &demo,
        g_fields.matrix.m11 == 16.0F && g_fields.matrix.m14 == 19.0F &&
            g_fields.matrix.m22 == 21.0F && g_fields.matrix.m33 == 26.0F &&
            g_fields.matrix.m41 == 28.0F && g_fields.matrix.m44 == 31.0F &&
            g_fields.quaternion.x == 0.25F && g_fields.quaternion.w == 1.0F &&
            g_fields.color.r == 10U && g_fields.color.g == 20U && g_fields.color.b == 30U &&
            g_fields.color.a == 40U,
        "matrix, quaternion and color fields preserve their canonical layouts");
    cna_demo_check_result(&demo, cna_content_type_reader_destroy(type_reader),
                          "destroy the value-shaped type-reader instance");
    type_reader = CNA_INVALID_HANDLE;
    cna_demo_check(&demo, release_payload_reader(&reader, &stream),
                   "release the value payload reader and stream handles");

    if (!cna_demo_check_result(
            &demo,
            cna_reflective_type_reader_builder_create(
                view(SHARED_TYPE_NAME), create_shared, &g_shared, &shared_builder),
            "create a distinct shared-shaped reflective builder")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_reflective_type_reader_builder_add_field(
            shared_builder, CNA_CONTENT_FIELD_INT32, offsetof(SharedValue, value)) ==
                CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_register_shared(shared_builder) ==
                CNA_RESULT_SUCCESS &&
            cna_reflective_type_reader_builder_register_shared(shared_builder) ==
                CNA_RESULT_SUCCESS,
        "shared registration is idempotent for a reference-shaped reflective reader");
    cna_demo_check_result(&demo, cna_reflective_type_reader_builder_destroy(shared_builder),
                          "destroy the shared builder without withdrawing its registration");
    shared_builder = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        create_payload_reader(
            container, "shared.bin", shared_payload, sizeof(shared_payload), &stream, &reader),
        "create a content reader over the shared-shaped payload");
    if (reader == CNA_INVALID_HANDLE) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo, cna_content_type_reader_manager_create_reader(view(SharedName), &type_reader),
            "create the registered shared-shaped reflective reader")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_content_type_reader_read_untyped(type_reader, reader, &has_value) == CNA_RESULT_SUCCESS &&
            has_value == CNA_TRUE && g_shared.create_calls == 1 && g_shared.value == 9,
        "the shared-shaped reader returns a reference carrier over the caller-created object");
    cna_demo_check_result(&demo, cna_content_type_reader_destroy(type_reader),
                          "destroy the shared-shaped type-reader instance");
    type_reader = CNA_INVALID_HANDLE;
    cna_demo_check(&demo, release_payload_reader(&reader, &stream),
                   "release the shared payload reader and stream handles");

cleanup:
    if (type_reader != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_destroy(type_reader),
                              "destroy a remaining reflective type reader");
    }
    if (reader != CNA_INVALID_HANDLE || stream != CNA_INVALID_HANDLE) {
        cna_demo_check(&demo, release_payload_reader(&reader, &stream),
                       "release a remaining payload reader and stream");
    }
    if (builder != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_reflective_type_reader_builder_destroy(builder),
                              "destroy a remaining value builder");
    }
    if (shared_builder != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_reflective_type_reader_builder_destroy(shared_builder),
                              "destroy a remaining shared builder");
    }
    if (container != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_container_destroy(container),
                              "destroy the reflective fixture container handle");
    }
    if (device != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_storage_device_delete_container(device, view(ContainerName)),
            "delete the reflective fixture container tree");
        cna_demo_check_result(&demo, cna_storage_device_destroy(device),
                              "destroy the selected storage device");
    }
    cna_demo_check_result(&demo, cna_content_type_reader_manager_clear_type_creators(),
                          "leave the process type-reader registry empty");
    return cna_demo_finish(&demo);
}
