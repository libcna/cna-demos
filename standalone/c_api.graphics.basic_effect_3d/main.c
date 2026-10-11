/* SPDX-License-Identifier: MS-PL */
/*
 * 3D from C: a cube in a vertex buffer and an index buffer, lit by a BasicEffect. The vertex
 * layout comes from the stock VertexPositionNormalTexture declaration; the effect's matrices go
 * through the IEffectMatrices routes and its lights through IEffectLights (EnableDefaultLighting).
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
} Vertex; /* matches VertexPositionNormalTexture: 32 bytes */

typedef struct CubeState {
    CnaDemo* demo;
    CNA_VertexDeclarationHandle declaration; /* owned */
    CNA_VertexBufferHandle vertices;         /* owned */
    CNA_IndexBufferHandle indices;           /* owned */
    CNA_EffectHandle effect;                 /* owned */
    int frames;
} CubeState;

static const CNA_Color background = {28, 30, 40, 255};

/* Six faces, four corners each, with the face's normal. */
static void make_cube(Vertex* vertices, uint16_t* indices)
{
    static const float normals[6][3] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
    int face;
    for (face = 0; face < 6; ++face) {
        const float* n = normals[face];
        /* Two axes perpendicular to the normal span the face. */
        const float side[3] = {n[1], n[2], n[0]};
        const float up[3] = {n[1] * side[2] - n[2] * side[1], n[2] * side[0] - n[0] * side[2],
                             n[0] * side[1] - n[1] * side[0]};
        int corner;
        for (corner = 0; corner < 4; ++corner) {
            const float s = (corner == 0 || corner == 3) ? -1.0f : 1.0f;
            const float u = (corner < 2) ? -1.0f : 1.0f;
            Vertex* v = &vertices[face * 4 + corner];
            int axis;
            for (axis = 0; axis < 3; ++axis) {
                v->position[axis] = n[axis] + s * side[axis] + u * up[axis];
                v->normal[axis] = n[axis];
            }
            v->uv[0] = (s + 1.0f) / 2.0f;
            v->uv[1] = (u + 1.0f) / 2.0f;
        }
        /* The corners run counter-clockwise seen from outside; XNA's front faces are clockwise
           (RasterizerState.CullCounterClockwise is the default), so each triangle lists them backwards. */
        indices[face * 6 + 0] = (uint16_t)(face * 4 + 0);
        indices[face * 6 + 1] = (uint16_t)(face * 4 + 2);
        indices[face * 6 + 2] = (uint16_t)(face * 4 + 1);
        indices[face * 6 + 3] = (uint16_t)(face * 4 + 0);
        indices[face * 6 + 4] = (uint16_t)(face * 4 + 3);
        indices[face * 6 + 5] = (uint16_t)(face * 4 + 2);
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    CubeState* state = (CubeState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_VertexElement elements[8];
    uint64_t element_count = 0U;
    CNA_VertexBufferCreateInfo vertex_info;
    CNA_IndexBufferCreateInfo index_info;
    CNA_IndexBufferTransfer transfer;
    Vertex vertices[24];
    uint16_t indices[36];
    CNA_Matrix view;
    CNA_Matrix projection;
    const CNA_Vector3 eye = {0.0f, 2.0f, 6.0f};
    const CNA_Vector3 target = {0.0f, 0.0f, 0.0f};
    const CNA_Vector3 up = {0.0f, 1.0f, 0.0f};
    const CNA_Vector3 orange = {0.95f, 0.55f, 0.2f};
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    /* The stock VertexPositionNormalTexture layout, as a declaration this program owns. */
    result = cna_vertex_type_copy_elements(CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE, elements, 8U, &element_count);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_vertex_declaration_create(elements, element_count, &state->declaration);
    }
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    make_cube(vertices, indices);
    memset(&vertex_info, 0, sizeof(vertex_info));
    vertex_info.struct_size = (uint32_t)sizeof(vertex_info);
    vertex_info.struct_version = UINT32_C(1);
    vertex_info.vertex_declaration = state->declaration;
    vertex_info.vertex_count = 24;
    vertex_info.buffer_usage = CNA_BUFFER_USAGE_WRITE_ONLY;
    vertex_info.dynamic = CNA_FALSE;
    result = cna_vertex_buffer_create(graphics_device, &vertex_info, &state->vertices);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_vertex_buffer_set_data_raw(state->vertices, vertices, sizeof(vertices), 24U,
                                                (uint32_t)sizeof(Vertex));
    }
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    memset(&index_info, 0, sizeof(index_info));
    index_info.struct_size = (uint32_t)sizeof(index_info);
    index_info.struct_version = UINT32_C(1);
    index_info.index_count = 36;
    index_info.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    index_info.buffer_usage = CNA_BUFFER_USAGE_WRITE_ONLY;
    result = cna_index_buffer_create(graphics_device, &index_info, &state->indices);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&transfer, 0, sizeof(transfer));
    transfer.struct_size = (uint32_t)sizeof(transfer);
    transfer.struct_version = UINT32_C(1);
    transfer.index_element_size = CNA_INDEX_ELEMENT_SIZE_SIXTEEN_BITS;
    transfer.options = CNA_SET_DATA_NONE;
    transfer.start_index = 0U;
    transfer.element_count = 36U;
    result = cna_index_buffer_set_data(state->indices, &transfer, indices, 36U);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    /* BasicEffect: an orange material, XNA's three-light default rig, a camera above the front. */
    result = cna_basic_effect_create(graphics_device, &state->effect);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_basic_effect_set_diffuse_color(state->effect, orange);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_lights_enable_default(state->effect);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_matrix_create_look_at(eye, target, up, &view);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_matrix_create_perspective_field_of_view(0.7853982f, 800.0f / 480.0f, 0.1f, 100.0f, &projection);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_matrices_set_view(state->effect, view);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_matrices_set_projection(state->effect, projection);
    }
    return result;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    CubeState* state = (CubeState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

/* Distinct shades of the cube in the middle of the screen: the lit faces differ. */
static int count_shades(const CNA_Color* pixels)
{
    int seen[64];
    int count = 0;
    int x;
    int y;
    for (y = 140; y < 340; y += 4) {
        for (x = 300; x < 500; x += 4) {
            const CNA_Color p = pixels[y * 800 + x];
            int shade;
            int i;
            int known = 0;
            if (p.r == background.r && p.g == background.g && p.b == background.b) {
                continue;
            }
            shade = p.r / 16; /* coarse: shading inside a face does not count */
            for (i = 0; i < count; ++i) {
                known = known || seen[i] == shade;
            }
            if (!known && count < 64) {
                seen[count++] = shade;
            }
        }
    }
    return count;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    CubeState* state = (CubeState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_DepthStencilState depth;
    CNA_Matrix spin;
    CNA_Matrix tilt;
    CNA_Matrix world;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_game_clear(game, background);
    }
    /* Depth testing on: the near faces hide the far ones. */
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_depth_stencil_state_init(CNA_DEPTH_STENCIL_STATE_PRESET_DEFAULT, &depth);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_set_depth_stencil_state(graphics_device, &depth);
    }
    if (result == CNA_RESULT_SUCCESS) {
        /* A turn per frame drawn: the same pose on every renderer, however fast it draws. */
        result = cna_matrix_create_rotation_y(0.6f + 0.02f * (float)state->frames, &spin);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_matrix_create_rotation_x(0.35f, &tilt);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_matrix_multiply(spin, tilt, &world);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_matrices_set_world(state->effect, world);
    }
    /* Bind the buffers, apply the effect, draw 12 triangles from 24 vertices. */
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_set_vertex_buffer(graphics_device, state->vertices);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_set_index_buffer(graphics_device, state->indices);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_apply(state->effect);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_draw_indexed_primitives(graphics_device, CNA_PRIMITIVE_TRIANGLE_LIST, 0, 0, 24,
                                                             0, 12);
    }

    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames == state->demo->frames) {
        static CNA_Color pixels[800 * 480];
        uint64_t count = 0U;
        const CNA_Result readback =
            cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, pixels, 800U * 480U, &count);
        if (readback == CNA_RESULT_NOT_SUPPORTED) {
            printf("  this renderer cannot read its back buffer back; pixel checks skipped\n");
        } else if (cna_demo_check_result(state->demo, readback, "cna_graphics_device_get_backbuffer_data_rgba8")) {
            const CNA_Color centre = pixels[240 * 800 + 400];
            const CNA_Color corner = pixels[10 * 800 + 10];
            const int shades = count_shades(pixels);
            printf("  centre %u,%u,%u  corner %u,%u,%u  %d shades on the cube\n", centre.r, centre.g, centre.b,
                   corner.r, corner.g, corner.b, shades);
            cna_demo_check(state->demo, centre.r != background.r && centre.r > centre.b + 20,
                           "the orange cube covers the centre");
            cna_demo_check(state->demo, corner.r == background.r && corner.b == background.b,
                           "the clear colour remains around it");
            cna_demo_check(state->demo, shades >= 3, "the lit faces differ in brightness (default lighting)");
        }
        cna_demo_capture(state->demo, graphics_device);
    }
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CubeState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.basic_effect_3d", argc, argv, 10);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.declaration = CNA_INVALID_HANDLE;
    state.vertices = CNA_INVALID_HANDLE;
    state.indices = CNA_INVALID_HANDLE;
    state.effect = CNA_INVALID_HANDLE;
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
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");

    /* Owned resources go before the game (which refuses to be destroyed while they live). */
    if (state.effect != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_effect_destroy(state.effect), "cna_effect_destroy");
    }
    if (state.indices != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_index_buffer_destroy(state.indices), "cna_index_buffer_destroy");
    }
    if (state.vertices != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_vertex_buffer_destroy(state.vertices), "cna_vertex_buffer_destroy");
    }
    if (state.declaration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_vertex_declaration_destroy(state.declaration),
                              "cna_vertex_declaration_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
