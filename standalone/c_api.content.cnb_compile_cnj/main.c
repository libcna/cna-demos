/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Generic filesystem CNJ-to-CNB compilation, bytes and dependency results through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#endif

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

#if defined(__linux__)
typedef CNA_Result (*ListSizeRoute)(CNA_CnjToCnbResultHandle, uint64_t, uint64_t*);
typedef CNA_Result (*ListCopyRoute)(
    CNA_CnjToCnbResultHandle, uint64_t, char*, uint64_t, uint64_t*);

typedef struct CnjFixture {
    char root[256];
    char cnj[288];
    char vertices[288];
    char indices[288];
    char unsupported[288];
    int ready;
} CnjFixture;

static int write_bytes(const char* path, const void* bytes, size_t byte_count)
{
    FILE* file = fopen(path, "wb");
    int ok;
    if (file == NULL) {
        return 0;
    }
    ok = fwrite(bytes, 1U, byte_count, file) == byte_count;
    return fclose(file) == 0 && ok;
}

static int prepare_fixture(CnjFixture* fixture)
{
    static const char Document[] =
        "{\n"
        "  \"cnjVersion\": 1,\n"
        "  \"type\": \"Model\",\n"
        "  \"meshes\": [{\n"
        "    \"name\": \"Quad\",\n"
        "    \"vertices\": \"quad_verts.bin\",\n"
        "    \"indices\": \"quad_idx.bin\",\n"
        "    \"vertexStride\": 32,\n"
        "    \"effect\": \"effects/water\",\n"
        "    \"texture\": \"textures/albedo.png\"\n"
        "  }]\n"
        "}\n";
    static const char Unsupported[] =
        "{\"cnjVersion\":1,\"type\":\"ContosoBlob\"}\n";
    static const uint8_t Indices[12] = {
        0U, 0U, 1U, 0U, 2U, 0U, 0U, 0U, 2U, 0U, 3U, 0U};
    char template_path[] = "/tmp/cna-demos-cnb-compile-cnj-XXXXXX";
    uint8_t vertices[128];
    char* root;
    size_t index;

    memset(fixture, 0, sizeof(*fixture));
    root = mkdtemp(template_path);
    if (root == NULL || strlen(root) >= sizeof(fixture->root)) {
        return 0;
    }
    memcpy(fixture->root, root, strlen(root) + 1U);
    fixture->ready = 1;
    if (snprintf(fixture->cnj, sizeof(fixture->cnj), "%s/quad.cnj", root) < 0 ||
        snprintf(fixture->vertices, sizeof(fixture->vertices), "%s/quad_verts.bin", root) < 0 ||
        snprintf(fixture->indices, sizeof(fixture->indices), "%s/quad_idx.bin", root) < 0 ||
        snprintf(fixture->unsupported, sizeof(fixture->unsupported), "%s/blob.cnj", root) < 0) {
        return 0;
    }
    for (index = 0U; index < sizeof(vertices); ++index) {
        vertices[index] = (uint8_t)(index ^ 0x5AU);
    }
    return write_bytes(fixture->cnj, Document, sizeof(Document) - 1U) &&
           write_bytes(fixture->vertices, vertices, sizeof(vertices)) &&
           write_bytes(fixture->indices, Indices, sizeof(Indices)) &&
           write_bytes(fixture->unsupported, Unsupported, sizeof(Unsupported) - 1U);
}

static int cleanup_fixture(CnjFixture* fixture)
{
    int ok = 1;
    if (!fixture->ready) {
        return 1;
    }
    if (unlink(fixture->cnj) != 0 || unlink(fixture->vertices) != 0 ||
        unlink(fixture->indices) != 0 || unlink(fixture->unsupported) != 0 ||
        rmdir(fixture->root) != 0) {
        ok = 0;
    }
    fixture->ready = 0;
    return ok;
}

static int text_is_atomic(
    CNA_CnjToCnbResultHandle result, uint64_t index, ListSizeRoute size_route,
    ListCopyRoute copy_route, const char* expected)
{
    char text[64];
    char short_text[64];
    char short_before[64];
    uint64_t size = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_size = (uint64_t)strlen(expected);

    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memcpy(short_before, short_text, sizeof(short_text));
    return size_route(result, index, &size) == CNA_RESULT_SUCCESS && size == expected_size &&
           copy_route(result, index, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           copy_route(result, index, text, expected_size, &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int result_type_name_is(CNA_CnjToCnbResultHandle result, const char* expected)
{
    char text[64];
    char short_text[64];
    char short_before[64];
    uint64_t size = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_size = (uint64_t)strlen(expected);

    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memcpy(short_before, short_text, sizeof(short_text));
    return cna_cnb_cnj_result_get_asset_type_name_size(result, &size) == CNA_RESULT_SUCCESS &&
           size == expected_size &&
           cna_cnb_cnj_result_copy_asset_type_name(
               result, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           cna_cnb_cnj_result_copy_asset_type_name(
               result, text, expected_size, &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int release_result(CNA_CnjToCnbResultHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_cnj_result_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_model(CNA_CnbModelDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_model_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}
#endif

int main(int argc, char** argv)
{
    CnaDemo demo;

    cna_demo_init(&demo, "c_api.content.cnb_compile_cnj", argc, argv, 1);
#if !defined(__linux__)
    return cna_demo_skip("the private filesystem fixture is currently implemented for Linux");
#else
    CnjFixture fixture;
    CNA_CnjToCnbResultHandle result = CNA_INVALID_HANDLE;
    CNA_CnjToCnbResultHandle stale_result = CNA_INVALID_HANDLE;
    CNA_CnjToCnbResultHandle rejected_result = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbModelDataHandle model = CNA_INVALID_HANDLE;
    CNA_CnbModelInfo model_info;
    uint8_t* image = NULL;
    uint8_t* repeat = NULL;
    uint8_t* short_image = NULL;
    uint32_t asset_type = UINT32_C(0);
    uint32_t parsed_type = UINT32_C(0);
    uint64_t image_size = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint64_t untouched = UINT64_C(0x1122334455667788);
    uint32_t untouched_type = UINT32_C(0x89abcdef);
    char missing[288];
    int releases_ok = 1;

    memset(&fixture, 0, sizeof(fixture));
    if (!cna_demo_check(
            &demo, prepare_fixture(&fixture),
            "create a private Model CNJ, two binary sidecars and an unsupported CNJ")) {
        goto cleanup;
    }
    if (snprintf(missing, sizeof(missing), "%s/missing.cnj", fixture.root) < 0) {
        cna_demo_check(&demo, 0, "form the missing CNJ path");
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_compile_cnj(
                text_view(fixture.cnj), text_view(fixture.root), text_view("Models/Quad"),
                &result),
            "compile the filesystem CNJ through the generic result route")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_asset_type_id(result, &asset_type) == CNA_RESULT_SUCCESS &&
            asset_type == CNA_CNB_ASSET_TYPE_MODEL,
        "the generic result reports the exact Model asset identity");
    cna_demo_check(
        &demo,
        result_type_name_is(result, "Microsoft.Xna.Framework.Graphics.Model"),
        "the result's canonical asset type name sizes and copies atomically");

    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_absorbed_file_count(result, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(3),
        "the result reports the CNJ and both embedded binary dependencies");
    cna_demo_check(
        &demo,
        text_is_atomic(
            result, UINT64_C(0), cna_cnb_cnj_result_get_absorbed_file_size,
            cna_cnb_cnj_result_copy_absorbed_file, "quad.cnj") &&
            text_is_atomic(
                result, UINT64_C(1), cna_cnb_cnj_result_get_absorbed_file_size,
                cna_cnb_cnj_result_copy_absorbed_file, "quad_verts.bin") &&
            text_is_atomic(
                result, UINT64_C(2), cna_cnb_cnj_result_get_absorbed_file_size,
                cna_cnb_cnj_result_copy_absorbed_file, "quad_idx.bin"),
        "absorbed paths keep CNJ-first and authored sidecar order with atomic copies");
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_absorbed_file_size(result, count, &untouched) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            untouched == UINT64_C(0x1122334455667788),
        "the absorbed-file Count boundary leaves its size output untouched");

    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_external_reference_count(result, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the result reports both external runtime asset dependencies");
    cna_demo_check(
        &demo,
        text_is_atomic(
            result, UINT64_C(0), cna_cnb_cnj_result_get_external_reference_size,
            cna_cnb_cnj_result_copy_external_reference, "effects/water") &&
            text_is_atomic(
                result, UINT64_C(1), cna_cnb_cnj_result_get_external_reference_size,
                cna_cnb_cnj_result_copy_external_reference, "textures/albedo.png"),
        "external references are sorted logical names with atomic copies");
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_external_reference_size(result, count, &untouched) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            untouched == UINT64_C(0x1122334455667788),
        "the external-reference Count boundary leaves its size output untouched");

    if (!cna_demo_check(
            &demo,
            cna_cnb_cnj_result_copy_bytes(result, NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= SIZE_MAX,
            "a zero-capacity byte copy reports the complete compiled CNB size")) {
        goto cleanup;
    }
    image = (uint8_t*)malloc((size_t)image_size);
    repeat = (uint8_t*)malloc((size_t)image_size);
    short_image = (uint8_t*)malloc((size_t)image_size);
    if (!cna_demo_check(
            &demo, image != NULL && repeat != NULL && short_image != NULL,
            "allocate exact caller-owned buffers for the compiled image")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, (size_t)image_size);
    copied = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_copy_bytes(
            result, short_image, image_size - UINT64_C(1), &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == image_size && short_image[0] == 0xA5U &&
            short_image[(size_t)image_size - 1U] == 0xA5U,
        "a short CNB destination reports its size without a partial endpoint write");
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_copy_bytes(result, image, image_size, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == image_size &&
            cna_cnb_cnj_result_copy_bytes(result, repeat, image_size, &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == image_size && memcmp(image, repeat, (size_t)image_size) == 0,
        "two complete copies return the same deterministic CNB image");

    stale_result = result;
    cna_demo_check_result(
        &demo, cna_cnb_cnj_result_destroy(result),
        "release the compile result after copying all build outputs");
    result = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_cnb_cnj_result_get_asset_type_id(stale_result, &untouched_type) ==
                CNA_RESULT_INVALID_HANDLE &&
            untouched_type == UINT32_C(0x89abcdef),
        "the released result rejects stale access without changing output");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_document_parse(
                image, image_size, text_view("compiled-model.cnb"), NULL, &document),
            "open the copied compiler output as a complete CNB document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(document, &parsed_type) == CNA_RESULT_SUCCESS &&
            parsed_type == asset_type,
        "the copied file header agrees with the generic result identity");
    if (cna_demo_check_result(
            &demo, cna_cnb_decode_model(document, &model),
            "decode the generic compiler output through the Model codec")) {
        memset(&model_info, 0, sizeof(model_info));
        model_info.struct_size = (uint32_t)sizeof(model_info);
        model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
        cna_demo_check(
            &demo,
            cna_cnb_model_get_info(model, &model_info) == CNA_RESULT_SUCCESS &&
                model_info.bone_count == UINT64_C(2) &&
                model_info.part_count == UINT64_C(1) &&
                model_info.mesh_count == UINT64_C(1),
            "the compiled bytes restore the derived quad model graph");
    }

    rejected_result = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_compile_cnj(
            text_view(missing), text_view(fixture.root), text_view(""),
            &rejected_result) == CNA_RESULT_IO &&
            rejected_result == CNA_INVALID_HANDLE &&
            ((rejected_result = UINT64_MAX),
             cna_cnb_compile_cnj(
                 text_view(fixture.unsupported), text_view(fixture.root), text_view("Blob"),
                 &rejected_result)) == CNA_RESULT_IO &&
            rejected_result == CNA_INVALID_HANDLE,
        "missing and unsupported CNJ documents fail without partial result handles");

cleanup:
    releases_ok &= release_model(&model);
    releases_ok &= release_document(&document);
    releases_ok &= release_result(&rejected_result);
    releases_ok &= release_result(&result);
    free(short_image);
    free(repeat);
    free(image);
    releases_ok &= cleanup_fixture(&fixture);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining handle, buffer and private filesystem fixture");
    return cna_demo_finish(&demo);
#endif
}
