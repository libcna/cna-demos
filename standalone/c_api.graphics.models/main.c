/* SPDX-License-Identifier: MS-PL */
/* Top-level Model ownership, transforms, diagnostics and pixel-proven drawing through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct OwnedProbe {
    int releases;
} OwnedProbe;

typedef struct ModelDemo {
    CnaDemo* demo;
    CNA_VertexDeclarationHandle declaration;
    CNA_VertexBufferHandle vertices;
    CNA_IndexBufferHandle indices;
    CNA_EffectHandle effect;
    CNA_ModelMeshPartHandle part;
    CNA_ModelMeshHandle mesh;
    CNA_ModelBoneHandle root;
    CNA_ModelBoneHandle child;
    CNA_ModelHandle model;
    CNA_ModelHandle simple_model;
    CNA_ModelBoneCollectionHandle bones;
    CNA_ModelMeshCollectionHandle meshes;
    CNA_ModelBoneHandle root_view;
    CNA_ModelBoneHandle bone_view;
    CNA_ModelMeshHandle mesh_view;
    int loads;
    int draws;
} ModelDemo;

static const CNA_Color Black = {0U, 0U, 0U, 255U};
static const CNA_Color Green = {0U, 255U, 0U, 255U};

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static void release_owned_probe(void* context)
{
    OwnedProbe* probe = (OwnedProbe*)context;
    ++probe->releases;
}

static int bytes_are(const char* bytes, uint64_t count, const char* expected)
{
    const uint64_t expected_count = (uint64_t)strlen(expected);
    return count == expected_count && memcmp(bytes, expected, (size_t)count) == 0;
}

static int audit_default_model(CnaDemo* demo)
{
    CNA_ModelHandle model = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle bones = CNA_INVALID_HANDLE;
    CNA_ModelMeshCollectionHandle meshes = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle root = UINT64_MAX;
    CNA_ModelTag tag = UINT64_MAX;
    CNA_Bool has = CNA_TRUE;
    CNA_Matrix identity;
    CNA_GltfImportReportEXT report;
    CNA_GltfImportReportEXT authored;
    CNA_GltfImportDiagnosticEXT diagnostic;
    CNA_GltfImportDiagnosticDescriptorEXT descriptor;
    CNA_StringView details[2];
    OwnedProbe first = {0};
    OwnedProbe second = {0};
    OwnedProbe final = {0};
    char code[] = "unsupported-extension";
    char subject[] = "KHR_materials_sheen";
    char detail0[] = "coat";
    char detail1[] = "roughness";
    char message[] = "Three authored features were not represented.";
    char buffer[96];
    char short_buffer[8];
    char short_before[8];
    uint64_t count = UINT64_MAX;
    uint64_t length = UINT64_MAX;
    int ok = 1;

    if (!cna_demo_check_result(
            demo, cna_model_create_default(&model),
            "create a graphics-device-independent empty Model")) {
        return 0;
    }
    cna_demo_check(
        demo,
        cna_model_get_bones(model, &bones) == CNA_RESULT_SUCCESS &&
            cna_model_bone_collection_get_count(bones, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_get_meshes(model, &meshes) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_collection_get_count(meshes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_get_root(model, &has, &root) == CNA_RESULT_SUCCESS &&
            has == CNA_FALSE && root == CNA_INVALID_HANDLE,
        "the default Model exposes empty retained collections and no root");
    cna_demo_check(
        demo,
        cna_model_get_tag(model, &tag) == CNA_RESULT_SUCCESS && tag == UINT64_C(0) &&
            cna_model_set_tag(model, UINT64_MAX) == CNA_RESULT_SUCCESS &&
            cna_model_get_tag(model, &tag) == CNA_RESULT_SUCCESS && tag == UINT64_MAX,
        "the Model tag preserves all 64 opaque bits");
    cna_demo_check(
        demo,
        cna_model_get_bone_transform_count(model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_copy_bone_transforms(model, NULL, UINT64_C(0), &count) ==
                CNA_RESULT_SUCCESS &&
            cna_model_copy_absolute_bone_transforms(model, NULL, UINT64_C(0), &count) ==
                CNA_RESULT_SUCCESS &&
            cna_model_set_bone_transforms(model, NULL, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            cna_model_draw(model, identity, identity, identity) == CNA_RESULT_SUCCESS,
        "zero-bone transform transfers and empty drawing are valid no-ops");

    cna_demo_check(
        demo,
        cna_model_set_owned_resources(model, &first, release_owned_probe) ==
                CNA_RESULT_SUCCESS &&
            cna_model_set_owned_resources(model, &second, release_owned_probe) ==
                CNA_RESULT_SUCCESS &&
            first.releases == 1 && second.releases == 0 &&
            cna_model_set_owned_resources(model, NULL, NULL) == CNA_RESULT_SUCCESS &&
            second.releases == 1 &&
            cna_model_set_owned_resources(model, &first, NULL) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_set_owned_resources(model, &final, release_owned_probe) ==
                CNA_RESULT_SUCCESS,
        "owned-resource replacement releases each prior context once and refuses no callback");

    memset(&report, 0, sizeof(report));
    report.struct_size = (uint32_t)sizeof(report);
    report.struct_version = UINT32_C(1);
    cna_demo_check(
        demo,
        cna_model_get_gltf_import_report_ext(model, &report) == CNA_RESULT_SUCCESS &&
            report.node_count == UINT64_C(0) && report.diagnostic_count == UINT64_C(0) &&
            report.anything_lost == CNA_FALSE,
        "a procedural Model starts with an empty versioned glTF import report");
    authored = report;
    authored.node_count = UINT64_C(5);
    authored.mesh_instance_count = UINT64_C(2);
    authored.distinct_mesh_count = UINT64_C(1);
    authored.shared_mesh_count = UINT64_C(1);
    authored.max_node_depth = UINT64_C(3);
    authored.camera_node_count = UINT64_C(1);
    authored.light_node_count = UINT64_C(2);
    authored.imported_light_count = UINT64_C(1);
    authored.primitive_count = UINT64_C(4);
    authored.skin_count = UINT64_C(1);
    authored.animation_count = UINT64_C(2);
    authored.clip_count = UINT64_C(1);
    cna_demo_check(
        demo,
        cna_model_set_gltf_import_report_ext(model, &authored) == CNA_RESULT_SUCCESS &&
            cna_model_get_gltf_import_report_ext(model, &report) == CNA_RESULT_SUCCESS &&
            report.node_count == UINT64_C(5) &&
            report.mesh_instance_count == UINT64_C(2) &&
            report.distinct_mesh_count == UINT64_C(1) &&
            report.shared_mesh_count == UINT64_C(1) &&
            report.max_node_depth == UINT64_C(3) &&
            report.camera_node_count == UINT64_C(1) &&
            report.light_node_count == UINT64_C(2) &&
            report.imported_light_count == UINT64_C(1) &&
            report.primitive_count == UINT64_C(4) && report.skin_count == UINT64_C(1) &&
            report.animation_count == UINT64_C(2) && report.clip_count == UINT64_C(1),
        "all twelve authored glTF report counts round-trip exactly");
    authored.warning_count = UINT64_C(1);
    cna_demo_check(
        demo,
        cna_model_set_gltf_import_report_ext(model, &authored) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_get_gltf_import_report_ext(model, &report) == CNA_RESULT_SUCCESS &&
            report.node_count == UINT64_C(5) && report.warning_count == UINT64_C(0),
        "derived report fields are refused without changing authored counts");
    authored.warning_count = UINT64_C(0);

    details[0] = text_view(detail0);
    details[1] = text_view(detail1);
    descriptor.code = text_view(code);
    descriptor.severity = CNA_GLTF_IMPORT_SEVERITY_WARNING_EXT;
    descriptor.kind = CNA_GLTF_IMPORT_KIND_UNSUPPORTED_FEATURE_EXT;
    descriptor.subject = text_view(subject);
    descriptor.count = UINT64_C(3);
    descriptor.worst_magnitude = 0.125;
    descriptor.details = details;
    descriptor.detail_count = UINT64_C(2);
    descriptor.message = text_view(message);
    cna_demo_check(
        demo,
        cna_model_add_gltf_import_diagnostic_ext(model, &descriptor) ==
                CNA_RESULT_SUCCESS,
        "append one warning while its four text sources are borrowed");
    memset(code, 'x', sizeof(code) - 1U);
    memset(subject, 'x', sizeof(subject) - 1U);
    memset(detail0, 'x', sizeof(detail0) - 1U);
    memset(detail1, 'x', sizeof(detail1) - 1U);
    memset(message, 'x', sizeof(message) - 1U);
    memset(&diagnostic, 0, sizeof(diagnostic));
    diagnostic.struct_size = (uint32_t)sizeof(diagnostic);
    diagnostic.struct_version = UINT32_C(1);
    cna_demo_check(
        demo,
        cna_model_get_gltf_import_report_ext(model, &report) == CNA_RESULT_SUCCESS &&
            report.diagnostic_count == UINT64_C(1) && report.warning_count == UINT64_C(1) &&
            report.dropped_feature_count == UINT64_C(3) &&
            report.approximation_count == UINT64_C(0) && report.anything_lost == CNA_TRUE &&
            cna_model_get_gltf_import_diagnostic_ext(
                model, UINT64_C(0), &diagnostic) == CNA_RESULT_SUCCESS &&
            diagnostic.severity == CNA_GLTF_IMPORT_SEVERITY_WARNING_EXT &&
            diagnostic.kind == CNA_GLTF_IMPORT_KIND_UNSUPPORTED_FEATURE_EXT &&
            diagnostic.count == UINT64_C(3) && diagnostic.worst_magnitude == 0.125 &&
            diagnostic.detail_count == UINT64_C(2),
        "the owned diagnostic drives exact derived loss totals and preserves every scalar");

    memset(buffer, 0, sizeof(buffer));
    cna_demo_check(
        demo,
        cna_model_get_gltf_import_diagnostic_code_byte_count_ext(
            model, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(21) &&
            cna_model_copy_gltf_import_diagnostic_code_ext(
                model, UINT64_C(0), buffer, sizeof(buffer), &length) == CNA_RESULT_SUCCESS &&
            bytes_are(buffer, length, "unsupported-extension") &&
            cna_model_get_gltf_import_diagnostic_subject_byte_count_ext(
                model, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(19) &&
            cna_model_copy_gltf_import_diagnostic_subject_ext(
                model, UINT64_C(0), buffer, sizeof(buffer), &length) == CNA_RESULT_SUCCESS &&
            bytes_are(buffer, length, "KHR_materials_sheen"),
        "owned diagnostic code and subject expose exact nonterminated UTF-8 copies");
    cna_demo_check(
        demo,
        cna_model_get_gltf_import_diagnostic_message_byte_count_ext(
            model, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(45) &&
            cna_model_copy_gltf_import_diagnostic_message_ext(
                model, UINT64_C(0), buffer, sizeof(buffer), &length) == CNA_RESULT_SUCCESS &&
            bytes_are(buffer, length, "Three authored features were not represented.") &&
            cna_model_get_gltf_import_diagnostic_detail_byte_count_ext(
                model, UINT64_C(0), UINT64_C(1), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(9) &&
            cna_model_copy_gltf_import_diagnostic_detail_ext(
                model, UINT64_C(0), UINT64_C(1), buffer, sizeof(buffer), &length) ==
                CNA_RESULT_SUCCESS &&
            bytes_are(buffer, length, "roughness"),
        "owned diagnostic message and indexed details survive caller-buffer overwrite");
    memset(short_buffer, 0xA5, sizeof(short_buffer));
    memcpy(short_before, short_buffer, sizeof(short_buffer));
    cna_demo_check(
        demo,
        cna_model_copy_gltf_import_diagnostic_code_ext(
            model, UINT64_C(0), short_buffer, sizeof(short_buffer), &length) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            length == UINT64_C(21) &&
            memcmp(short_buffer, short_before, sizeof(short_buffer)) == 0 &&
            cna_model_get_gltf_import_diagnostic_ext(
                model, UINT64_C(1), &diagnostic) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_get_gltf_import_diagnostic_detail_byte_count_ext(
                model, UINT64_C(0), UINT64_C(2), &length) == CNA_RESULT_INVALID_ARGUMENT,
        "short text transfers and both diagnostic Count boundaries are atomic");

    ok &= cna_model_bone_collection_destroy(bones) == CNA_RESULT_SUCCESS;
    ok &= cna_model_mesh_collection_destroy(meshes) == CNA_RESULT_SUCCESS;
    ok &= cna_model_destroy(model) == CNA_RESULT_SUCCESS;
    cna_demo_check(
        demo, ok && final.releases == 1,
        "destroying the Model releases its final owned-resource context exactly once");
    return ok && final.releases == 1;
}

static int create_geometry(ModelDemo* state, CNA_Handle device)
{
    static const CNA_VertexElement Elements[2] = {
        {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION, 0U},
        {12, CNA_VERTEX_ELEMENT_FORMAT_COLOR, CNA_VERTEX_ELEMENT_USAGE_COLOR, 0U}};
    static const CNA_VertexPositionColor Vertices[3] = {
        {{-0.25F, -0.25F, 0.5F}, {0U, 255U, 0U, 255U}},
        {{0.25F, -0.25F, 0.5F}, {0U, 255U, 0U, 255U}},
        {{0.0F, 0.25F, 0.5F}, {0U, 255U, 0U, 255U}}};
    static const uint16_t Indices[3] = {0U, 1U, 2U};
    CNA_VertexBufferCreateInfo vertex_info;
    CNA_IndexBufferCreateInfo index_info;
    CNA_IndexBufferTransfer transfer;
    CNA_Matrix identity;

    if (cna_vertex_declaration_create_with_stride(
            (int32_t)sizeof(CNA_VertexPositionColor), Elements, UINT64_C(2),
            &state->declaration) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&vertex_info, 0, sizeof(vertex_info));
    vertex_info.struct_size = (uint32_t)sizeof(vertex_info);
    vertex_info.struct_version = UINT32_C(1);
    vertex_info.vertex_declaration = state->declaration;
    vertex_info.vertex_count = 3;
    if (cna_vertex_buffer_create(device, &vertex_info, &state->vertices) != CNA_RESULT_SUCCESS ||
        cna_vertex_buffer_set_data_raw(
            state->vertices, Vertices, sizeof(Vertices), UINT64_C(3),
            (uint32_t)sizeof(CNA_VertexPositionColor)) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&index_info, 0, sizeof(index_info));
    index_info.struct_size = (uint32_t)sizeof(index_info);
    index_info.struct_version = UINT32_C(1);
    index_info.index_count = 3;
    index_info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    if (cna_index_buffer_create(device, &index_info, &state->indices) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&transfer, 0, sizeof(transfer));
    transfer.struct_size = (uint32_t)sizeof(transfer);
    transfer.struct_version = UINT32_C(1);
    transfer.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    transfer.element_count = UINT64_C(3);
    if (cna_index_buffer_set_data(state->indices, &transfer, Indices, UINT64_C(3)) !=
            CNA_RESULT_SUCCESS ||
        cna_basic_effect_create(device, &state->effect) != CNA_RESULT_SUCCESS ||
        cna_matrix_get_identity(&identity) != CNA_RESULT_SUCCESS ||
        cna_effect_matrices_set_world(state->effect, identity) != CNA_RESULT_SUCCESS ||
        cna_effect_matrices_set_view(state->effect, identity) != CNA_RESULT_SUCCESS ||
        cna_effect_matrices_set_projection(state->effect, identity) != CNA_RESULT_SUCCESS ||
        cna_basic_effect_set_vertex_color_enabled(state->effect, CNA_TRUE) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    return 1;
}

static CNA_Result read_pixel(CNA_Handle device, int32_t x, int32_t y, CNA_Color* out_color)
{
    CNA_BackBufferReadback readback;
    memset(&readback, 0, sizeof(readback));
    readback.struct_size = (uint32_t)sizeof(readback);
    readback.struct_version = UINT32_C(1);
    readback.has_source_rectangle = CNA_TRUE;
    readback.source_rectangle = (CNA_Rectangle){x, y, 1, 1};
    readback.element_count = UINT64_C(1);
    memset(out_color, 0xA5, sizeof(*out_color));
    return cna_graphics_device_get_backbuffer_data_window(
        device, &readback, out_color, UINT64_C(1));
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ModelDemo* state = (ModelDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle bone_values[2];
    CNA_ModelBoneHandle parent_values[1];
    CNA_ModelMeshHandle mesh_values[1];
    CNA_ModelHandle rejected = UINT64_MAX;
    CNA_ModelEffectCollectionHandle effects = CNA_INVALID_HANDLE;
    CNA_Matrix identity;
    CNA_Matrix local[2];
    CNA_Matrix observed[2];
    CNA_Matrix absolute[2];
    CNA_Matrix sentinel;
    CNA_Bool has = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    int32_t index = INT32_C(-1);
    CNA_RasterizerState rasterizer;
    CNA_BlendState blend;
    CNA_DepthStencilState depth;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the live device for a procedural Model")) {
        return CNA_RESULT_SUCCESS;
    }
    if (!cna_demo_check(
            state->demo, create_geometry(state, device),
            "create one small indexed green triangle and its BasicEffect")) {
        return CNA_RESULT_SUCCESS;
    }
    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_part_create(
                state->vertices, state->indices, INT32_C(3), INT32_C(1), INT32_C(0),
                INT32_C(0), &state->part),
            "create the Model's drawable mesh part")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check_result(
        state->demo, cna_model_mesh_part_set_effect(state->part, state->effect),
        "attach the material before constructing its mesh");
    mesh_values[0] = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_create_named(
                device, text_view("ModelMesh"), &state->part, UINT64_C(1), &state->mesh),
            "construct the named mesh around the retained part")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_mesh_get_effects(state->mesh, &effects) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_effect(state->part, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_effect(state->part, state->effect) == CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_get_count(effects, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_effect_collection_destroy(effects) == CNA_RESULT_SUCCESS,
        "reattach the material so Model.Draw can update the mesh's live Effects view");
    effects = CNA_INVALID_HANDLE;

    cna_demo_check(
        state->demo,
        cna_model_bone_create(INT32_C(0), text_view("Root"), &state->root) ==
                CNA_RESULT_SUCCESS &&
            cna_model_bone_create(INT32_C(1), text_view("Child"), &state->child) ==
                CNA_RESULT_SUCCESS &&
            cna_model_bone_add_child(state->root, state->child) == CNA_RESULT_SUCCESS,
        "build an ordered two-bone hierarchy for the Model");
    bone_values[0] = state->root;
    bone_values[1] = state->child;
    mesh_values[0] = state->mesh;
    parent_values[0] = state->child;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_create_with_parents(
                device, bone_values, UINT64_C(2), mesh_values, UINT64_C(1), parent_values,
                UINT64_C(1), UINT64_C(0), &state->model),
            "create a Model with explicit mesh parent and root selection")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_create(
            device, bone_values, UINT64_C(2), NULL, UINT64_C(0),
            &state->simple_model) == CNA_RESULT_SUCCESS &&
            cna_model_create_with_parents(
                device, bone_values, UINT64_C(2), mesh_values, UINT64_C(1), parent_values,
                UINT64_C(0), UINT64_C(2), &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE &&
            cna_model_create_with_parents(
                device, bone_values, UINT64_C(2), mesh_values, UINT64_C(1), parent_values,
                UINT64_C(2), UINT64_C(0), &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "the simple constructor succeeds while invalid root and parent counts fail atomically");

    cna_demo_check(
        state->demo,
        cna_model_get_bones(state->model, &state->bones) == CNA_RESULT_SUCCESS &&
            cna_model_bone_collection_get_count(state->bones, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_model_bone_collection_get_at(
                state->bones, UINT64_C(1), &state->bone_view) == CNA_RESULT_SUCCESS &&
            cna_model_bone_get_index(state->bone_view, &index) == CNA_RESULT_SUCCESS &&
            index == INT32_C(1) &&
            cna_model_get_root(state->model, &has, &state->root_view) == CNA_RESULT_SUCCESS &&
            has == CNA_TRUE &&
            cna_model_bone_get_index(state->root_view, &index) == CNA_RESULT_SUCCESS &&
            index == INT32_C(0),
        "the Model publishes its exact ordered bones and explicitly selected root");
    cna_demo_check(
        state->demo,
        cna_model_get_meshes(state->model, &state->meshes) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_collection_get_count(state->meshes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_mesh_collection_get_at(
                state->meshes, UINT64_C(0), &state->mesh_view) == CNA_RESULT_SUCCESS,
        "the Model publishes a stable view of its retained mesh");
    cna_demo_check(
        state->demo,
        cna_model_get_tag(state->model, &count) == CNA_RESULT_SUCCESS && count == UINT64_C(0) &&
            cna_model_set_tag(state->model, UINT64_MAX) == CNA_RESULT_SUCCESS &&
            cna_model_get_tag(state->model, &count) == CNA_RESULT_SUCCESS && count == UINT64_MAX,
        "the aggregate Model keeps its own full-width opaque tag");

    cna_matrix_get_identity(&identity);
    local[0] = identity;
    local[1] = identity;
    local[0].m41 = 0.25F;
    local[1].m42 = 0.5F;
    memset(&sentinel, 0xA5, sizeof(sentinel));
    observed[0] = identity;
    observed[1] = identity;
    cna_demo_check(
        state->demo,
        cna_model_set_bone_transforms(state->model, local, UINT64_C(1)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_copy_bone_transforms(
                state->model, observed, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(observed, &identity, sizeof(identity)) == 0 &&
            memcmp(&observed[1], &identity, sizeof(identity)) == 0,
        "a short local-transform source is refused without partial bone mutation");
    cna_demo_check(
        state->demo,
        cna_model_set_bone_transforms(state->model, local, UINT64_C(2)) ==
                CNA_RESULT_SUCCESS &&
            cna_model_get_bone_transform_count(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the Model reports the exact transform count after a complete update");
    memset(&sentinel, 0xA5, sizeof(sentinel));
    {
        CNA_Matrix before = sentinel;
        cna_demo_check(
            state->demo,
            cna_model_copy_bone_transforms(
                state->model, &sentinel, UINT64_C(1), &count) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                count == UINT64_C(2) && memcmp(&sentinel, &before, sizeof(sentinel)) == 0,
            "a short local-transform destination remains byte-for-byte unchanged");
    }
    cna_demo_check(
        state->demo,
        cna_model_copy_bone_transforms(
            state->model, observed, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            memcmp(observed, local, sizeof(local)) == 0 &&
            cna_model_copy_absolute_bone_transforms(
                state->model, absolute, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            absolute[0].m41 == 0.25F && absolute[0].m42 == 0.0F &&
            absolute[1].m41 == 0.25F && absolute[1].m42 == 0.5F,
        "local copies stay exact and absolute transforms compose the parent translation");

    cna_demo_check(
        state->demo,
        cna_rasterizer_state_init(
            CNA_RASTERIZER_STATE_PRESET_CULL_NONE, &rasterizer) == CNA_RESULT_SUCCESS &&
            cna_blend_state_init(CNA_BLEND_STATE_PRESET_OPAQUE, &blend) ==
                CNA_RESULT_SUCCESS &&
            cna_depth_stencil_state_init(CNA_DEPTH_STENCIL_STATE_PRESET_NONE, &depth) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_rasterizer_state(device, &rasterizer) ==
                CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_blend_state(device, &blend) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_depth_stencil_state(device, &depth) == CNA_RESULT_SUCCESS,
        "install deterministic raster, blend and depth state for Model drawing");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ModelDemo* state = (ModelDemo*)context;
    (void)game_time;
    (void)out_error;
    if (state->draws != 0) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ModelDemo* state = (ModelDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Matrix identity;
    CNA_Matrix world;
    CNA_Color shifted;
    CNA_Color vacated;
    int rendered;
    (void)game_time;
    (void)out_error;
    ++state->draws;

    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS ||
        cna_matrix_get_identity(&identity) != CNA_RESULT_SUCCESS) {
        return CNA_RESULT_INVALID_STATE;
    }
    world = identity;
    world.m41 = 0.5F;
    rendered = cna_graphics_device_clear_color_depth(device, Black, 1.0F) ==
                   CNA_RESULT_SUCCESS &&
               cna_model_draw(state->model, world, identity, identity) == CNA_RESULT_SUCCESS &&
               read_pixel(device, INT32_C(700), INT32_C(120), &shifted) ==
                   CNA_RESULT_SUCCESS &&
               read_pixel(device, INT32_C(400), INT32_C(240), &vacated) ==
                   CNA_RESULT_SUCCESS &&
               shifted.r == Green.r && shifted.g == Green.g && shifted.b == Green.b &&
               shifted.a == Green.a && vacated.r == Black.r && vacated.g == Black.g &&
               vacated.b == Black.b && vacated.a == Black.a;
    cna_demo_check(
        state->demo, rendered,
        "Model.Draw composes parent-bone and world transforms while the old center stays clear");
    return CNA_RESULT_SUCCESS;
}

static int cleanup_state(ModelDemo* state)
{
    int ok = 1;
    if (state->part != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_set_effect(state->part, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
        ok &= cna_model_mesh_part_set_vertex_buffer(state->part, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
        ok &= cna_model_mesh_part_set_index_buffer(state->part, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
    }
    if (state->model != CNA_INVALID_HANDLE) {
        ok &= cna_model_destroy(state->model) == CNA_RESULT_SUCCESS;
    }
    if (state->simple_model != CNA_INVALID_HANDLE) {
        ok &= cna_model_destroy(state->simple_model) == CNA_RESULT_SUCCESS;
    }
    if (state->root_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->root_view) == CNA_RESULT_SUCCESS;
    }
    if (state->bone_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->bone_view) == CNA_RESULT_SUCCESS;
    }
    if (state->bones != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_collection_destroy(state->bones) == CNA_RESULT_SUCCESS;
    }
    if (state->mesh_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->mesh_view) == CNA_RESULT_SUCCESS;
    }
    if (state->meshes != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_collection_destroy(state->meshes) == CNA_RESULT_SUCCESS;
    }
    if (state->mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->mesh) == CNA_RESULT_SUCCESS;
    }
    if (state->child != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->child) == CNA_RESULT_SUCCESS;
    }
    if (state->root != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->root) == CNA_RESULT_SUCCESS;
    }
    if (state->part != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_destroy(state->part) == CNA_RESULT_SUCCESS;
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        ok &= cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS;
    }
    if (state->indices != CNA_INVALID_HANDLE) {
        ok &= cna_index_buffer_destroy(state->indices) == CNA_RESULT_SUCCESS;
    }
    if (state->vertices != CNA_INVALID_HANDLE) {
        ok &= cna_vertex_buffer_destroy(state->vertices) == CNA_RESULT_SUCCESS;
    }
    if (state->declaration != CNA_INVALID_HANDLE) {
        ok &= cna_vertex_declaration_destroy(state->declaration) == CNA_RESULT_SUCCESS;
    }
    return ok;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ModelDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int device_ready;

    cna_demo_init(&demo, "c_api.graphics.models", argc, argv, 1);
    audit_default_model(&demo);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.declaration = CNA_INVALID_HANDLE;
    state.vertices = CNA_INVALID_HANDLE;
    state.indices = CNA_INVALID_HANDLE;
    state.effect = CNA_INVALID_HANDLE;
    state.part = CNA_INVALID_HANDLE;
    state.mesh = CNA_INVALID_HANDLE;
    state.root = CNA_INVALID_HANDLE;
    state.child = CNA_INVALID_HANDLE;
    state.model = CNA_INVALID_HANDLE;
    state.simple_model = CNA_INVALID_HANDLE;
    state.bones = CNA_INVALID_HANDLE;
    state.meshes = CNA_INVALID_HANDLE;
    state.root_view = CNA_INVALID_HANDLE;
    state.bone_view = CNA_INVALID_HANDLE;
    state.mesh_view = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the graphics-owner game")) {
        return cna_demo_finish(&demo);
    }
    device_ready = cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_set_graphics_profile(
                       manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_set_preferred_back_buffer_width(
                       manager, INT32_C(800)) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_set_preferred_back_buffer_height(
                       manager, INT32_C(480)) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(
            &demo, device_ready,
            "create an explicit HiDef 800x480 graphics device")) {
        cna_demo_check_result(
            &demo, cna_game_run(game),
            "run the complete Model graph and draw workflow");
        cna_demo_check(
            &demo, state.loads == 1 && state.draws == 1,
            "the audit used exactly one load and one draw callback");
    }
    cna_demo_check(
        &demo, cleanup_state(&state),
        "release every Model view, graph node and retained graphics resource");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
