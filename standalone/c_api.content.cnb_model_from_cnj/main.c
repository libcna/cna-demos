/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Filesystem CNJ model compilation, dependency lists and ownership transfer through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
#include <sys/stat.h>
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
typedef CNA_Result (*ListSizeRoute)(CNA_CnbModelFromCnjHandle, uint64_t, uint64_t*);
typedef CNA_Result (*ListCopyRoute)(
    CNA_CnbModelFromCnjHandle, uint64_t, char*, uint64_t, uint64_t*);

typedef struct CnjFixture {
    char root[256];
    char cnj[288];
    char vertices[288];
    char indices[288];
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
    static const uint8_t Indices[12] = {
        0U, 0U, 1U, 0U, 2U, 0U, 0U, 0U, 2U, 0U, 3U, 0U};
    uint8_t vertices[128];
    char template_path[] = "/tmp/cna-demos-cnb-model-from-cnj-XXXXXX";
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
        snprintf(fixture->indices, sizeof(fixture->indices), "%s/quad_idx.bin", root) < 0) {
        return 0;
    }
    for (index = 0U; index < sizeof(vertices); ++index) {
        vertices[index] = (uint8_t)(index ^ 0x5AU);
    }
    return write_bytes(fixture->cnj, Document, sizeof(Document) - 1U) &&
           write_bytes(fixture->vertices, vertices, sizeof(vertices)) &&
           write_bytes(fixture->indices, Indices, sizeof(Indices));
}

static int cleanup_fixture(CnjFixture* fixture)
{
    int ok = 1;
    if (!fixture->ready) {
        return 1;
    }
    if (unlink(fixture->cnj) != 0) {
        ok = 0;
    }
    if (unlink(fixture->vertices) != 0) {
        ok = 0;
    }
    if (unlink(fixture->indices) != 0) {
        ok = 0;
    }
    if (rmdir(fixture->root) != 0) {
        ok = 0;
    }
    fixture->ready = 0;
    return ok;
}

static int list_text_is(
    CNA_CnbModelFromCnjHandle result, uint64_t index, ListSizeRoute size_route,
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

static int release_result(CNA_CnbModelFromCnjHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_model_from_cnj_destroy(*handle);
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

    cna_demo_init(&demo, "c_api.content.cnb_model_from_cnj", argc, argv, 1);
#if !defined(__linux__)
    return cna_demo_skip("the private filesystem fixture is currently implemented for Linux");
#else
    CnjFixture fixture;
    CNA_CnbModelFromCnjHandle result = CNA_INVALID_HANDLE;
    CNA_CnbModelFromCnjHandle stale_result = CNA_INVALID_HANDLE;
    CNA_CnbModelFromCnjHandle failed_result = CNA_INVALID_HANDLE;
    CNA_CnbModelDataHandle model = CNA_INVALID_HANDLE;
    CNA_CnbModelDataHandle duplicate_model = CNA_INVALID_HANDLE;
    CNA_CnbModelInfo info;
    CNA_CnbModelPartInfo part;
    CNA_CnbMeshInfo mesh;
    char text[64];
    char missing[288];
    uint64_t count = UINT64_C(0);
    uint64_t untouched = UINT64_C(0x1122334455667788);
    int releases_ok = 1;

    memset(&fixture, 0, sizeof(fixture));
    if (!cna_demo_check(&demo, prepare_fixture(&fixture),
                        "create a private CNJ document and its two binary sidecars")) {
        goto cleanup;
    }
    if (snprintf(missing, sizeof(missing), "%s/missing.cnj", fixture.root) < 0) {
        cna_demo_check(&demo, 0, "form the absent CNJ path inside the private fixture");
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_build_model_from_cnj(
                text_view(fixture.cnj), text_view(fixture.root), &result),
            "compile the filesystem CNJ model into an owned result")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_absorbed_file_count(result, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the result reports both absorbed binary dependencies");
    cna_demo_check(
        &demo,
        list_text_is(
            result, UINT64_C(0), cna_cnb_model_from_cnj_get_absorbed_file_size,
            cna_cnb_model_from_cnj_copy_absorbed_file, "quad_verts.bin") &&
            list_text_is(
                result, UINT64_C(1), cna_cnb_model_from_cnj_get_absorbed_file_size,
                cna_cnb_model_from_cnj_copy_absorbed_file, "quad_idx.bin"),
        "absorbed paths preserve authored order and copy atomically");
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_absorbed_file_size(result, count, &untouched) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            untouched == UINT64_C(0x1122334455667788),
        "an absorbed-path index at Count is refused without changing output");
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_external_reference_count(result, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the result reports both assets left for the content build graph");
    cna_demo_check(
        &demo,
        list_text_is(
            result, UINT64_C(0), cna_cnb_model_from_cnj_get_external_reference_size,
            cna_cnb_model_from_cnj_copy_external_reference, "effects/water") &&
            list_text_is(
                result, UINT64_C(1), cna_cnb_model_from_cnj_get_external_reference_size,
                cna_cnb_model_from_cnj_copy_external_reference, "textures/albedo.png"),
        "external references are normalized, sorted and copied atomically");
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_external_reference_size(result, count, &untouched) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            untouched == UINT64_C(0x1122334455667788),
        "an external-reference index at Count is refused without changing output");

    if (!cna_demo_check_result(
            &demo, cna_cnb_model_from_cnj_take_model(result, &model),
            "transfer the compiled model into an independent model handle")) {
        goto cleanup;
    }
    duplicate_model = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_take_model(result, &duplicate_model) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            duplicate_model == CNA_INVALID_HANDLE,
        "taking the result model twice is refused without publishing a handle");
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_absorbed_file_count(result, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_cnb_model_from_cnj_get_external_reference_count(result, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "taking the model leaves both dependency lists readable");

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    memset(&part, 0, sizeof(part));
    part.struct_size = (uint32_t)sizeof(part);
    part.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    memset(&mesh, 0, sizeof(mesh));
    mesh.struct_size = (uint32_t)sizeof(mesh);
    mesh.struct_version = CNA_CNB_MESH_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_info(model, &info) == CNA_RESULT_SUCCESS &&
            info.bone_count == UINT64_C(2) && info.part_count == UINT64_C(1) &&
            info.mesh_count == UINT64_C(1) && info.animation_count == UINT64_C(0) &&
            info.light_count == UINT64_C(0) && info.has_skeleton == CNA_FALSE &&
            info.applies_gltf_lighting_policy == CNA_FALSE &&
            info.has_bone_hierarchy == CNA_FALSE &&
            cna_cnb_model_get_part(model, UINT64_C(0), &part) == CNA_RESULT_SUCCESS &&
            part.vertex_stride == UINT32_C(32) && part.vertex_count == UINT32_C(4) &&
            part.index_count == UINT32_C(6) && part.index_element_size == UINT32_C(2) &&
            part.primitive_count == UINT32_C(2) && part.effect_kind == CNA_CNB_EFFECT_KIND_EXTERNAL &&
            cna_cnb_model_get_mesh(model, UINT64_C(0), &mesh) == CNA_RESULT_SUCCESS &&
            mesh.parent_bone == INT32_C(1) && mesh.part_index_count == UINT64_C(1),
        "the transferred model carries the compiled graph and derived geometry facts");
    count = UINT64_C(0);
    memset(text, 0, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_part_external_effect(
            model, UINT64_C(0), text, sizeof(text), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(13) && memcmp(text, "effects/water", 13U) == 0 &&
            cna_cnb_model_copy_material_texture(
                model, UINT64_C(0), CNA_CNB_MATERIAL_TEXTURE_BASE_COLOR,
                text, sizeof(text), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(19) && memcmp(text, "textures/albedo.png", 19U) == 0,
        "external effect and texture names survive the compile and transfer");

    stale_result = result;
    cna_demo_check_result(
        &demo, cna_cnb_model_from_cnj_destroy(result),
        "release the compiler result after transferring its model");
    result = CNA_INVALID_HANDLE;
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_cnb_model_from_cnj_get_absorbed_file_count(stale_result, &untouched) ==
                CNA_RESULT_INVALID_HANDLE &&
            untouched == UINT64_C(0x1122334455667788),
        "the released compiler result rejects stale list access");
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_info(model, &info) == CNA_RESULT_SUCCESS &&
            info.part_count == UINT64_C(1) && info.mesh_count == UINT64_C(1),
        "the transferred model remains alive after its compiler result is gone");

    failed_result = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_build_model_from_cnj(
            text_view(missing), text_view(fixture.root), &failed_result) == CNA_RESULT_IO &&
            failed_result == CNA_INVALID_HANDLE,
        "a missing CNJ is an IO failure without a partial result handle");

cleanup:
    releases_ok &= release_result(&failed_result);
    releases_ok &= release_model(&duplicate_model);
    releases_ok &= release_model(&model);
    releases_ok &= release_result(&result);
    releases_ok &= cleanup_fixture(&fixture);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining handle and remove the private filesystem fixture");
    return cna_demo_finish(&demo);
#endif
}
