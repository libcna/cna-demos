/* SPDX-License-Identifier: MS-PL */
/* ModelMeshPart resources, ModelMesh views and pixel-proven drawing through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ModelMeshDemo {
    CnaDemo* demo;
    CNA_VertexDeclarationHandle declaration;
    CNA_VertexBufferHandle vertices;
    CNA_IndexBufferHandle indices;
    CNA_EffectHandle effect;
    CNA_EffectHandle spare_effect;
    CNA_ModelMeshPartHandle part;
    CNA_ModelMeshPartHandle part_view;
    CNA_ModelMeshPartCollectionHandle parts;
    CNA_ModelEffectCollectionHandle effects;
    CNA_ModelMeshHandle empty_mesh;
    CNA_ModelMeshHandle mesh;
    CNA_ModelMeshHandle found_mesh;
    CNA_ModelMeshCollectionHandle meshes;
    CNA_ModelBoneHandle parent;
    CNA_ModelBoneHandle parent_view;
    int loads;
    int draws;
} ModelMeshDemo;

static const CNA_Color Black = {0U, 0U, 0U, 255U};
static const CNA_Color Green = {0U, 255U, 0U, 255U};

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int mesh_name_is(CNA_ModelMeshHandle mesh, const char* expected)
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
    if (cna_model_mesh_get_name_byte_count(mesh, &size) != CNA_RESULT_SUCCESS ||
        size != expected_size) {
        return 0;
    }
    if (expected_size == UINT64_C(0)) {
        return cna_model_mesh_copy_name(mesh, NULL, UINT64_C(0), &copied) ==
                   CNA_RESULT_SUCCESS &&
               copied == UINT64_C(0);
    }
    return cna_model_mesh_copy_name(
               mesh, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           cna_model_mesh_copy_name(mesh, text, expected_size, &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int create_geometry(ModelMeshDemo* state, CNA_Handle device)
{
    static const CNA_VertexElement Elements[2] = {
        {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION, 0U},
        {12, CNA_VERTEX_ELEMENT_FORMAT_COLOR, CNA_VERTEX_ELEMENT_USAGE_COLOR, 0U}};
    static const CNA_VertexPositionColor Vertices[3] = {
        {{-0.8F, -0.8F, 0.5F}, {0U, 255U, 0U, 255U}},
        {{0.8F, -0.8F, 0.5F}, {0U, 255U, 0U, 255U}},
        {{0.0F, 0.8F, 0.5F}, {0U, 255U, 0U, 255U}}};
    static const uint16_t Indices[3] = {0U, 1U, 2U};
    CNA_VertexBufferCreateInfo vertex_info;
    CNA_IndexBufferCreateInfo index_info;
    CNA_IndexBufferTransfer transfer;

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
    vertex_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
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
    index_info.buffer_usage = CNA_BUFFER_USAGE_NONE;
    if (cna_index_buffer_create(device, &index_info, &state->indices) != CNA_RESULT_SUCCESS) {
        return 0;
    }
    memset(&transfer, 0, sizeof(transfer));
    transfer.struct_size = (uint32_t)sizeof(transfer);
    transfer.struct_version = UINT32_C(1);
    transfer.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    transfer.options = CNA_SET_DATA_NONE;
    transfer.element_count = UINT64_C(3);
    return cna_index_buffer_set_data(
               state->indices, &transfer, Indices, UINT64_C(3)) == CNA_RESULT_SUCCESS;
}

static int create_effect(CNA_Handle device, CNA_EffectHandle* out_effect)
{
    CNA_Matrix identity;
    return cna_basic_effect_create(device, out_effect) == CNA_RESULT_SUCCESS &&
           cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
           cna_effect_matrices_set_world(*out_effect, identity) == CNA_RESULT_SUCCESS &&
           cna_effect_matrices_set_view(*out_effect, identity) == CNA_RESULT_SUCCESS &&
           cna_effect_matrices_set_projection(*out_effect, identity) == CNA_RESULT_SUCCESS &&
           cna_basic_effect_set_vertex_color_enabled(*out_effect, CNA_TRUE) ==
               CNA_RESULT_SUCCESS;
}

static CNA_Result read_probe(CNA_Handle device, CNA_Color* out_color)
{
    CNA_BackBufferReadback readback;
    memset(&readback, 0, sizeof(readback));
    readback.struct_size = (uint32_t)sizeof(readback);
    readback.struct_version = UINT32_C(1);
    readback.has_source_rectangle = CNA_TRUE;
    readback.source_rectangle = (CNA_Rectangle){400, 240, 1, 1};
    readback.start_index = UINT64_C(0);
    readback.element_count = UINT64_C(1);
    memset(out_color, 0xA5, sizeof(*out_color));
    return cna_graphics_device_get_backbuffer_data_window(
        device, &readback, out_color, UINT64_C(1));
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ModelMeshDemo* state = (ModelMeshDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle returned = UINT64_MAX;
    CNA_ModelMeshHandle original_mesh = CNA_INVALID_HANDLE;
    CNA_ModelMeshHandle mesh_view = CNA_INVALID_HANDLE;
    CNA_ModelMeshHandle rejected_mesh = CNA_INVALID_HANDLE;
    CNA_ModelMeshHandle mesh_values[2];
    CNA_ModelMeshPartHandle rejected_part = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle part_values[1];
    CNA_EffectHandle returned_effect = CNA_INVALID_HANDLE;
    CNA_BoundingSphere sphere;
    CNA_BoundingSphere observed_sphere;
    CNA_ModelMeshTag tag = UINT64_C(0);
    CNA_Bool flag = CNA_FALSE;
    uint64_t count = UINT64_C(0);
    int32_t parent_index = INT32_C(-1);
    CNA_RasterizerState rasterizer;
    CNA_BlendState blend;
    CNA_DepthStencilState depth;
    static const char EmbeddedNul[3] = {'x', '\0', 'y'};
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the live graphics device for model-mesh resources")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        create_geometry(state, device) && create_effect(device, &state->effect) &&
            create_effect(device, &state->spare_effect),
        "create a filled triangle buffer pair and two vertex-color BasicEffects");
    if (state->vertices == CNA_INVALID_HANDLE || state->indices == CNA_INVALID_HANDLE ||
        state->effect == CNA_INVALID_HANDLE || state->spare_effect == CNA_INVALID_HANDLE) {
        return CNA_RESULT_SUCCESS;
    }

    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_part_create(
                state->vertices, state->indices, INT32_C(3), INT32_C(1), INT32_C(0),
                INT32_C(0), &state->part),
            "construct a drawable part that retains both filled buffers")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_mesh_part_set_effect(state->part, state->effect) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_get_effect(state->part, &flag, &returned) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && returned == state->effect &&
            cna_model_mesh_part_get_vertex_buffer(state->part, &flag, &returned) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && returned == state->vertices &&
            cna_model_mesh_part_get_index_buffer(state->part, &flag, &returned) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && returned == state->indices,
        "the part returns the exact effect and buffer identities it retains");
    cna_demo_check(
        state->demo,
        cna_effect_destroy(state->effect) == CNA_RESULT_INVALID_STATE &&
            cna_vertex_buffer_destroy(state->vertices) == CNA_RESULT_INVALID_STATE &&
            cna_index_buffer_destroy(state->indices) == CNA_RESULT_INVALID_STATE,
        "retained effect and buffers reject premature owner destruction");

    cna_demo_check(
        state->demo,
        cna_model_mesh_create(device, NULL, UINT64_C(0), &state->empty_mesh) ==
                CNA_RESULT_SUCCESS &&
            mesh_name_is(state->empty_mesh, "") &&
            cna_model_mesh_get_tag(state->empty_mesh, &tag) == CNA_RESULT_SUCCESS &&
            tag == UINT64_C(0) &&
            cna_model_mesh_get_parent_bone(state->empty_mesh, &flag, &state->parent_view) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && state->parent_view == CNA_INVALID_HANDLE,
        "an unnamed empty mesh has empty text, tag zero and no parent bone");

    part_values[0] = state->part;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_create_named(
                device, text_view("AuditMesh"), part_values, UINT64_C(1), &original_mesh),
            "create a named mesh retaining the drawable part")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        mesh_name_is(original_mesh, "AuditMesh") &&
            cna_model_mesh_set_tag(original_mesh, UINT64_MAX) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_get_tag(original_mesh, &tag) == CNA_RESULT_SUCCESS &&
            tag == UINT64_MAX,
        "mesh text copies atomically and its opaque tag keeps all 64 bits");
    sphere = (CNA_BoundingSphere){{1.0F, -2.0F, 3.0F}, -4.0F};
    memset(&observed_sphere, 0, sizeof(observed_sphere));
    cna_demo_check(
        state->demo,
        cna_model_mesh_set_bounding_sphere(original_mesh, sphere) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_get_bounding_sphere(original_mesh, &observed_sphere) ==
                CNA_RESULT_SUCCESS &&
            memcmp(&sphere, &observed_sphere, sizeof(sphere)) == 0,
        "the mesh bounding sphere preserves all authored components verbatim");

    cna_demo_check(
        state->demo,
        cna_model_mesh_get_mesh_parts(original_mesh, &state->parts) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_collection_get_count(state->parts, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_mesh_part_collection_get_at(
                state->parts, UINT64_C(0), &state->part_view) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_vertex_offset(state->part_view, INT32_C(0)) ==
                CNA_RESULT_SUCCESS,
        "the live mesh-part view returns the same mutable retained part");
    cna_demo_check(
        state->demo,
        cna_model_mesh_get_effects(original_mesh, &state->effects) == CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_get_count(state->effects, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "CNA-DEMOS-ISSUE-081: construction omits the part's pre-attached effect from Effects");
    cna_demo_check(
        state->demo,
        cna_model_mesh_part_set_effect(state->part_view, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_effect(state->part_view, state->effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_get_count(state->effects, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_effect_collection_get_at(
                state->effects, UINT64_C(0), &returned_effect) == CNA_RESULT_SUCCESS &&
            returned_effect == state->effect &&
            cna_model_effect_collection_contains(state->effects, state->effect, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "the live effect view tracks a part effect detached and reattached after construction");
    cna_demo_check(
        state->demo,
        cna_model_effect_collection_add(state->effects, state->spare_effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_add(state->effects, state->spare_effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_get_count(state->effects, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(3) &&
            cna_model_effect_collection_get_at(
                state->effects, UINT64_C(2), &returned_effect) == CNA_RESULT_SUCCESS &&
            returned_effect == state->spare_effect,
        "effect collection mutation preserves duplicate entries and their order");
    cna_demo_check(
        state->demo,
        cna_model_effect_collection_remove(state->effects, state->spare_effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_remove(state->effects, state->spare_effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_remove(state->effects, state->spare_effect) ==
                CNA_RESULT_SUCCESS &&
            cna_model_effect_collection_get_count(state->effects, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_effect_collection_contains(
                state->effects, state->spare_effect, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "effect removal erases one match at a time and absence is idempotent");

    cna_demo_check(
        state->demo,
        cna_model_bone_create(INT32_C(7), text_view("MeshParent"), &state->parent) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_set_parent_bone(original_mesh, state->parent) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_get_parent_bone(
                original_mesh, &flag, &state->parent_view) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_model_bone_get_index(state->parent_view, &parent_index) ==
                CNA_RESULT_SUCCESS &&
            parent_index == INT32_C(7),
        "a mesh retains and returns an owned view of its exact parent bone");

    part_values[0] = state->part;
    rejected_mesh = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_model_mesh_create_named(
            device, text_view("DuplicateOwner"), part_values, UINT64_C(1),
            &rejected_mesh) == CNA_RESULT_INVALID_STATE &&
            rejected_mesh == CNA_INVALID_HANDLE &&
            cna_model_mesh_create_named(
                device, (CNA_StringView){EmbeddedNul, UINT64_C(3)}, NULL, UINT64_C(0),
                &rejected_mesh) == CNA_RESULT_ENCODING &&
            rejected_mesh == CNA_INVALID_HANDLE &&
            cna_model_mesh_create(device, NULL, UINT64_C(1), &rejected_mesh) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected_mesh == CNA_INVALID_HANDLE,
        "duplicate part ownership, embedded NUL text and missing part arrays fail atomically");

    mesh_values[0] = state->empty_mesh;
    mesh_values[1] = original_mesh;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_mesh_collection_create(mesh_values, UINT64_C(2), &state->meshes),
            "create an ordered retained mesh collection")) {
        return CNA_RESULT_SUCCESS;
    }
    mesh_values[0] = CNA_INVALID_HANDLE;
    mesh_values[1] = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_model_mesh_collection_get_count(state->meshes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_model_mesh_collection_get_at(
                state->meshes, UINT64_C(1), &mesh_view) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_collection_find(
                state->meshes, text_view("AuditMesh"), &flag, &state->found_mesh) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_model_mesh_collection_contains(
                state->meshes, state->found_mesh, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && mesh_name_is(mesh_view, "AuditMesh"),
        "mesh collection index, name and identity lookup return stable exact views");
    rejected_mesh = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_model_mesh_collection_find(
            state->meshes, text_view("Missing"), &flag, &rejected_mesh) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && rejected_mesh == CNA_INVALID_HANDLE &&
            ((rejected_mesh = UINT64_MAX),
             cna_model_mesh_collection_get_at(state->meshes, count, &rejected_mesh)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected_mesh == CNA_INVALID_HANDLE,
        "mesh lookup absence and the Count boundary publish no view");

    cna_demo_check_result(
        state->demo, cna_model_mesh_destroy(original_mesh),
        "release the original mesh while collection views retain it");
    original_mesh = CNA_INVALID_HANDLE;
    cna_demo_check_result(
        state->demo, cna_model_mesh_collection_destroy(state->meshes),
        "release the mesh collection snapshot");
    state->meshes = CNA_INVALID_HANDLE;
    state->mesh = mesh_view;
    mesh_view = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        mesh_name_is(state->mesh, "AuditMesh") &&
            mesh_name_is(state->found_mesh, "AuditMesh"),
        "both stable mesh views survive their original and collection handles");

    rejected_part = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_model_mesh_part_collection_get_at(state->parts, UINT64_C(1), &rejected_part) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected_part == CNA_INVALID_HANDLE &&
            cna_model_effect_collection_get_at(
                state->effects, UINT64_C(1), &returned_effect) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "live part and effect collections enforce their current Count boundaries");

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
            cna_graphics_device_set_depth_stencil_state(device, &depth) ==
                CNA_RESULT_SUCCESS,
        "install cull-none, opaque and depth-free state for deterministic mesh drawing");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    ModelMeshDemo* state = (ModelMeshDemo*)context;
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
    ModelMeshDemo* state = (ModelMeshDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Color pixel;
    int rendered;
    (void)game_time;
    (void)out_error;
    ++state->draws;

    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS ||
        state->mesh == CNA_INVALID_HANDLE || state->empty_mesh == CNA_INVALID_HANDLE) {
        return CNA_RESULT_INVALID_STATE;
    }
    rendered = cna_graphics_device_clear_color_depth(device, Black, 1.0F) ==
                   CNA_RESULT_SUCCESS &&
               cna_model_mesh_draw(state->mesh) == CNA_RESULT_SUCCESS &&
               read_probe(device, &pixel) == CNA_RESULT_SUCCESS && pixel.r == Green.r &&
               pixel.g == Green.g && pixel.b == Green.b && pixel.a == Green.a;
    cna_demo_check(
        state->demo, rendered,
        "ModelMesh.Draw renders the retained indexed BasicEffect part as an exact green pixel");
    rendered = cna_graphics_device_clear_color_depth(device, Black, 1.0F) ==
                   CNA_RESULT_SUCCESS &&
               cna_model_mesh_draw(state->empty_mesh) == CNA_RESULT_SUCCESS &&
               read_probe(device, &pixel) == CNA_RESULT_SUCCESS && pixel.r == Black.r &&
               pixel.g == Black.g && pixel.b == Black.b && pixel.a == Black.a;
    cna_demo_check(
        state->demo, rendered,
        "drawing an empty mesh succeeds while leaving the clear pixel unchanged");
    return CNA_RESULT_SUCCESS;
}

static int cleanup_state(ModelMeshDemo* state)
{
    int ok = 1;
    if (state->part_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_set_effect(state->part_view, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
        ok &= cna_model_mesh_part_set_vertex_buffer(state->part_view, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
        ok &= cna_model_mesh_part_set_index_buffer(state->part_view, CNA_INVALID_HANDLE) ==
              CNA_RESULT_SUCCESS;
    }
    if (state->effects != CNA_INVALID_HANDLE) {
        ok &= cna_model_effect_collection_destroy(state->effects) == CNA_RESULT_SUCCESS;
        state->effects = CNA_INVALID_HANDLE;
    }
    if (state->parts != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_collection_destroy(state->parts) == CNA_RESULT_SUCCESS;
        state->parts = CNA_INVALID_HANDLE;
    }
    if (state->found_mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->found_mesh) == CNA_RESULT_SUCCESS;
        state->found_mesh = CNA_INVALID_HANDLE;
    }
    if (state->mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->mesh) == CNA_RESULT_SUCCESS;
        state->mesh = CNA_INVALID_HANDLE;
    }
    if (state->empty_mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->empty_mesh) == CNA_RESULT_SUCCESS;
        state->empty_mesh = CNA_INVALID_HANDLE;
    }
    if (state->meshes != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_collection_destroy(state->meshes) == CNA_RESULT_SUCCESS;
        state->meshes = CNA_INVALID_HANDLE;
    }
    if (state->parent_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->parent_view) == CNA_RESULT_SUCCESS;
        state->parent_view = CNA_INVALID_HANDLE;
    }
    if (state->parent != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->parent) == CNA_RESULT_SUCCESS;
        state->parent = CNA_INVALID_HANDLE;
    }
    if (state->part_view != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_destroy(state->part_view) == CNA_RESULT_SUCCESS;
        state->part_view = CNA_INVALID_HANDLE;
    }
    if (state->part != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_part_destroy(state->part) == CNA_RESULT_SUCCESS;
        state->part = CNA_INVALID_HANDLE;
    }
    if (state->spare_effect != CNA_INVALID_HANDLE) {
        ok &= cna_effect_destroy(state->spare_effect) == CNA_RESULT_SUCCESS;
        state->spare_effect = CNA_INVALID_HANDLE;
    }
    if (state->effect != CNA_INVALID_HANDLE) {
        ok &= cna_effect_destroy(state->effect) == CNA_RESULT_SUCCESS;
        state->effect = CNA_INVALID_HANDLE;
    }
    if (state->indices != CNA_INVALID_HANDLE) {
        ok &= cna_index_buffer_destroy(state->indices) == CNA_RESULT_SUCCESS;
        state->indices = CNA_INVALID_HANDLE;
    }
    if (state->vertices != CNA_INVALID_HANDLE) {
        ok &= cna_vertex_buffer_destroy(state->vertices) == CNA_RESULT_SUCCESS;
        state->vertices = CNA_INVALID_HANDLE;
    }
    if (state->declaration != CNA_INVALID_HANDLE) {
        ok &= cna_vertex_declaration_destroy(state->declaration) == CNA_RESULT_SUCCESS;
        state->declaration = CNA_INVALID_HANDLE;
    }
    return ok;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ModelMeshDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int device_ready;

    cna_demo_init(&demo, "c_api.graphics.model_meshes", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.declaration = CNA_INVALID_HANDLE;
    state.vertices = CNA_INVALID_HANDLE;
    state.indices = CNA_INVALID_HANDLE;
    state.effect = CNA_INVALID_HANDLE;
    state.spare_effect = CNA_INVALID_HANDLE;
    state.part = CNA_INVALID_HANDLE;
    state.part_view = CNA_INVALID_HANDLE;
    state.parts = CNA_INVALID_HANDLE;
    state.effects = CNA_INVALID_HANDLE;
    state.empty_mesh = CNA_INVALID_HANDLE;
    state.mesh = CNA_INVALID_HANDLE;
    state.found_mesh = CNA_INVALID_HANDLE;
    state.meshes = CNA_INVALID_HANDLE;
    state.parent = CNA_INVALID_HANDLE;
    state.parent_view = CNA_INVALID_HANDLE;
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
            "run the complete ModelMesh workflow and immediate exit request");
        cna_demo_check(
            &demo, state.loads == 1 && state.draws == 1,
            "the audit used exactly one load and one draw callback");
    }
    cna_demo_check(
        &demo, cleanup_state(&state),
        "clear retained resource links and release every model-mesh object");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
