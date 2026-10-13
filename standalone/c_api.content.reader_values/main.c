/* SPDX-License-Identifier: MS-PL */
/* ContentReader identity, canonical values, limits and stream ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static const char ContainerName[] = "C API Content Reader Values";
static const char AssetFile[] = "values.bin";

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void push_byte(uint8_t* data, size_t* offset, uint8_t value)
{
    data[(*offset)++] = value;
}

static void push_float(uint8_t* data, size_t* offset, float value)
{
    uint32_t bits = 0U;
    memcpy(&bits, &value, sizeof(bits));
    push_byte(data, offset, (uint8_t)(bits & UINT32_C(0xff)));
    push_byte(data, offset, (uint8_t)((bits >> 8U) & UINT32_C(0xff)));
    push_byte(data, offset, (uint8_t)((bits >> 16U) & UINT32_C(0xff)));
    push_byte(data, offset, (uint8_t)((bits >> 24U) & UINT32_C(0xff)));
}

/* Protocol bytes, 29 floats, one color and four trailing raw bytes. */
#define ASSET_BYTE_COUNT 143U

static size_t build_asset(uint8_t data[ASSET_BYTE_COUNT])
{
    size_t offset = 0U;
    int value;
    push_byte(data, &offset, 0U); /* type-reader count */
    push_byte(data, &offset, 0U); /* shared-resource count */
    push_byte(data, &offset, 0U); /* null object reference */
    push_float(data, &offset, 1.0F);
    push_float(data, &offset, 2.0F);
    push_float(data, &offset, 3.0F);
    push_float(data, &offset, 4.0F);
    push_float(data, &offset, 5.0F);
    push_float(data, &offset, 6.0F);
    push_float(data, &offset, 7.0F);
    push_float(data, &offset, 8.0F);
    push_float(data, &offset, 9.0F);
    push_float(data, &offset, 0.25F);
    push_float(data, &offset, 0.5F);
    push_float(data, &offset, 0.75F);
    push_float(data, &offset, 1.0F);
    for (value = 1; value <= 16; ++value) {
        push_float(data, &offset, (float)value);
    }
    push_byte(data, &offset, 10U);
    push_byte(data, &offset, 20U);
    push_byte(data, &offset, 30U);
    push_byte(data, &offset, 40U);
    push_float(data, &offset, 11.0F);
    push_float(data, &offset, 12.0F);
    push_float(data, &offset, 13.0F);
    push_float(data, &offset, 14.0F);
    push_byte(data, &offset, 0xAAU);
    push_byte(data, &offset, 0xBBU);
    push_byte(data, &offset, 0xCCU);
    push_byte(data, &offset, 0xDDU);
    return offset;
}

int main(int argc, char** argv)
{
    uint8_t asset[ASSET_BYTE_COUNT];
    uint8_t raw[8];
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    CNA_ContentReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_ContentReaderHandle rejected = UINT64_C(99);
    CNA_ContentReaderCreateInfo create_info;
    CNA_Handle manager = UINT64_C(99);
    CNA_Vector2 vector2;
    CNA_Vector3 vector3;
    CNA_Vector4 vector4;
    CNA_Quaternion quaternion;
    CNA_Matrix matrix;
    CNA_Color color;
    CNA_BoundingSphere sphere;
    CNA_Bool has_value = CNA_TRUE;
    CnaDemo demo;
    uint64_t bytes = 0U;
    int32_t version = 0;
    uint8_t platform = 0U;
    char name[16];
    char before[16];

    cna_demo_init(&demo, "c_api.content.reader_values", argc, argv, 0);
    cna_demo_check(&demo, build_asset(asset) == sizeof(asset),
                   "the synthetic reader payload has its documented byte layout");
    if (!cna_demo_check_result(
            &demo, cna_storage_set_app_name_ext(view("cna-demos-content-reader-values")),
            "select an isolated application storage root") ||
        !cna_demo_check_result(
            &demo, cna_storage_device_show_selector(NULL, NULL, &device),
            "select the storage device used by the reader fixture")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(
        &demo, cna_storage_device_delete_container(device, view(ContainerName)),
        "remove any previous reader fixture");
    if (!cna_demo_check_result(
            &demo, cna_storage_container_open(device, view(ContainerName), NULL, NULL, &container),
            "open a fresh reader fixture container") ||
        !cna_demo_check_result(
            &demo, cna_storage_container_create_file(container, view(AssetFile), &stream),
            "create the synthetic reader stream")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_storage_stream_write(stream, asset, sizeof(asset)) == CNA_RESULT_SUCCESS &&
            cna_storage_stream_close(stream) == CNA_RESULT_SUCCESS,
        "write and close the exact synthetic reader payload");
    stream = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(
            &demo,
            cna_storage_container_open_file(container, view(AssetFile), CNA_FILE_MODE_OPEN, &stream),
            "reopen the payload for content reading")) {
        goto cleanup;
    }

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.stream = stream;
    create_info.asset_name = view("values");
    create_info.version = INT32_C(5);
    create_info.platform = (uint8_t)'w';
    create_info.reserved[0] = 1U;
    cna_demo_check(
        &demo,
        cna_content_reader_create(&create_info, &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "reader creation rejects a nonzero reserved byte without returning a handle");
    create_info.reserved[0] = 0U;
    if (!cna_demo_check_result(
            &demo, cna_content_reader_create(&create_info, &reader),
            "create a standalone content reader over the borrowed stream")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_content_reader_get_content_manager(reader, &manager) == CNA_RESULT_SUCCESS &&
            manager == CNA_INVALID_HANDLE,
        "a standalone reader reports no content manager");
    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_content_reader_get_asset_name_size(reader, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen("values") &&
            cna_content_reader_copy_asset_name(reader, name, bytes, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen("values") && memcmp(name, "values", (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "the logical asset name uses exact unterminated size and copy semantics");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_content_reader_copy_asset_name(reader, name, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == strlen("values") && memcmp(name, before, sizeof(name)) == 0,
        "a short asset-name destination is rejected without a partial write");
    cna_demo_check(
        &demo,
        cna_content_reader_get_version(reader, &version) == CNA_RESULT_SUCCESS && version == 5 &&
            cna_content_reader_get_platform(reader, &platform) == CNA_RESULT_SUCCESS &&
            platform == (uint8_t)'w',
        "the reader preserves its container version and platform identifier");

    cna_demo_check(
        &demo,
        cna_content_reader_initialize_type_readers(reader) == CNA_RESULT_SUCCESS &&
            cna_content_reader_read_shared_resources(reader) == CNA_RESULT_SUCCESS,
        "an empty type-reader table and shared-resource table advance successfully");
    cna_demo_check(
        &demo,
        cna_content_reader_read_object_tag(reader, &has_value) == CNA_RESULT_SUCCESS &&
            has_value == CNA_FALSE,
        "a zero object reference advances the graph and reports no value");
    cna_demo_check(
        &demo,
        cna_content_reader_read_vector2(reader, &vector2) == CNA_RESULT_SUCCESS &&
            vector2.x == 1.0F && vector2.y == 2.0F,
        "Vector2 reads two little-endian single-precision components");
    cna_demo_check(
        &demo,
        cna_content_reader_read_vector3(reader, &vector3) == CNA_RESULT_SUCCESS &&
            vector3.x == 3.0F && vector3.y == 4.0F && vector3.z == 5.0F,
        "Vector3 reads three components and advances the stream");
    cna_demo_check(
        &demo,
        cna_content_reader_read_vector4(reader, &vector4) == CNA_RESULT_SUCCESS &&
            vector4.x == 6.0F && vector4.y == 7.0F && vector4.z == 8.0F && vector4.w == 9.0F,
        "Vector4 reads four components in canonical order");
    cna_demo_check(
        &demo,
        cna_content_reader_read_quaternion(reader, &quaternion) == CNA_RESULT_SUCCESS &&
            quaternion.x == 0.25F && quaternion.y == 0.5F && quaternion.z == 0.75F &&
            quaternion.w == 1.0F,
        "Quaternion reads X, Y, Z and W in order");
    memset(&matrix, 0, sizeof(matrix));
    cna_demo_check(
        &demo,
        cna_content_reader_read_matrix(reader, &matrix) == CNA_RESULT_SUCCESS &&
            matrix.m11 == 1.0F && matrix.m14 == 4.0F && matrix.m22 == 6.0F &&
            matrix.m33 == 11.0F && matrix.m41 == 13.0F && matrix.m44 == 16.0F,
        "Matrix reads all sixteen values in canonical row order");
    cna_demo_check(
        &demo,
        cna_content_reader_read_color(reader, &color) == CNA_RESULT_SUCCESS && color.r == 10U &&
            color.g == 20U && color.b == 30U && color.a == 40U,
        "Color reads four bytes in RGBA order");
    cna_demo_check(
        &demo,
        cna_content_reader_read_bounding_sphere(reader, &sphere) == CNA_RESULT_SUCCESS &&
            sphere.center.x == 11.0F && sphere.center.y == 12.0F && sphere.center.z == 13.0F &&
            sphere.radius == 14.0F,
        "BoundingSphere reads its center followed by its radius");
    cna_demo_check(
        &demo,
        cna_content_reader_check_collection_element_count(reader, 5, view("Reader")) ==
                CNA_RESULT_SUCCESS &&
            cna_content_reader_check_collection_element_count(reader, -1, view("Reader")) ==
                CNA_RESULT_IO,
        "collection limits accept a small count and reject a negative declaration");
    cna_demo_check(
        &demo,
        cna_content_reader_check_decoded_byte_size(reader, 1024, view("Reader")) ==
                CNA_RESULT_SUCCESS &&
            cna_content_reader_check_decoded_byte_size(reader, -1, view("Reader")) == CNA_RESULT_IO,
        "decoded-size limits accept a small size and reject a negative declaration");
    memset(raw, 0xA5, sizeof(raw));
    cna_demo_check(
        &demo,
        cna_content_reader_read_bytes_exact(reader, 4, view("Reader"), raw, 2U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == 4U && raw[0] == 0xA5U,
        "an undersized exact-byte destination is reported before consuming the stream");
    memset(raw, 0, sizeof(raw));
    cna_demo_check(
        &demo,
        cna_content_reader_read_bytes_exact(reader, 4, view("Reader"), raw, sizeof(raw), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == 4U && raw[0] == 0xAAU && raw[1] == 0xBBU && raw[2] == 0xCCU &&
            raw[3] == 0xDDU,
        "an exact-byte read returns all authored bytes after the capacity refusal");
    cna_demo_check(
        &demo,
        cna_content_reader_read_bytes_exact(reader, -1, view("Reader"), raw, sizeof(raw), &bytes) ==
                CNA_RESULT_IO &&
            cna_content_reader_read_bytes_exact(reader, 1, view("Reader"), raw, sizeof(raw), &bytes) ==
                CNA_RESULT_IO,
        "exact-byte reads reject a negative count and a truncated stream");
    cna_demo_check(
        &demo, cna_storage_stream_close(stream) == CNA_RESULT_INVALID_STATE,
        "the reader keeps an exclusive borrow on its underlying stream");
    cna_demo_check(
        &demo,
        cna_content_reader_destroy(reader) == CNA_RESULT_SUCCESS &&
            cna_content_reader_destroy(reader) == CNA_RESULT_INVALID_HANDLE &&
            cna_storage_stream_close(stream) == CNA_RESULT_SUCCESS &&
            cna_storage_stream_close(stream) == CNA_RESULT_INVALID_HANDLE,
        "reader destruction closes its borrow and both owned handles release exactly once");
    reader = CNA_INVALID_HANDLE;
    stream = CNA_INVALID_HANDLE;

cleanup:
    if (reader != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_reader_destroy(reader),
                              "destroy a remaining content reader");
    }
    if (stream != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_stream_close(stream),
                              "close a remaining storage stream handle");
    }
    if (container != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_container_destroy(container),
                              "destroy the reader fixture container handle");
    }
    if (device != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_storage_device_delete_container(device, view(ContainerName)),
            "delete the reader fixture container tree");
        cna_demo_check_result(&demo, cna_storage_device_destroy(device),
                              "destroy the selected storage device");
    }
    return cna_demo_finish(&demo);
}
