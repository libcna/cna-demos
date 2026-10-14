/* SPDX-License-Identifier: MS-PL */
/* Model camera, skin, bound and infinite-projection scene metadata through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SceneDemo {
    CnaDemo* demo;
    CNA_ModelBoneHandle root;
    CNA_ModelMeshHandle mesh;
    CNA_ModelHandle model;
    CNA_SkinningDataHandle skeleton;
    CNA_SkinningDataHandle skeleton_view;
    CNA_Matrix infinite_projection;
    int loads;
} SceneDemo;

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = 1.0F;
    value.m22 = 1.0F;
    value.m33 = 1.0F;
    value.m44 = 1.0F;
    return value;
}

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int nearly_equal(float left, float right)
{
    return fabsf(left - right) < 0.0001F;
}

static int cleanup_state(SceneDemo* state)
{
    int ok = 1;
    if (state->model != CNA_INVALID_HANDLE) {
        ok &= cna_model_destroy(state->model) == CNA_RESULT_SUCCESS;
    }
    if (state->mesh != CNA_INVALID_HANDLE) {
        ok &= cna_model_mesh_destroy(state->mesh) == CNA_RESULT_SUCCESS;
    }
    if (state->root != CNA_INVALID_HANDLE) {
        ok &= cna_model_bone_destroy(state->root) == CNA_RESULT_SUCCESS;
    }
    if (state->skeleton != CNA_INVALID_HANDLE) {
        ok &= cna_skinning_data_destroy(state->skeleton) == CNA_RESULT_SUCCESS;
    }
    if (state->skeleton_view != CNA_INVALID_HANDLE) {
        ok &= cna_skinning_data_destroy(state->skeleton_view) == CNA_RESULT_SUCCESS;
    }
    return ok;
}

static int audit_camera(SceneDemo* state)
{
    char name[] = "MainCamera";
    char buffer[32];
    CNA_ModelCameraEXT authored;
    CNA_ModelCameraEXT observed;
    CNA_ModelCameraEXT malformed;
    CNA_ModelCameraDescriptorEXT descriptor;
    CNA_Matrix world = identity_matrix();
    uint64_t count = UINT64_MAX;
    uint64_t length = UINT64_MAX;

    world.m42 = 5.0F;
    memset(&authored, 0, sizeof(authored));
    authored.struct_size = (uint32_t)sizeof(authored);
    authored.struct_version = UINT32_C(1);
    authored.scene_node_index = INT32_C(7);
    authored.is_perspective = CNA_TRUE;
    authored.has_infinite_far_plane = CNA_TRUE;
    authored.has_authored_aspect_ratio = CNA_TRUE;
    authored.projection = state->infinite_projection;
    authored.world_transform = world;
    authored.aspect_ratio = 1.5F;
    authored.field_of_view = 1.0F;
    authored.near_plane_distance = 0.5F;
    authored.far_plane_distance = 0.0F;
    descriptor.name = text_view(name);
    descriptor.camera = authored;
    if (!cna_demo_check_result(
            state->demo, cna_model_add_camera_ext(state->model, &descriptor),
            "deep-copy one infinite-perspective imported camera")) {
        return 0;
    }
    name[0] = 'X';
    authored.scene_node_index = INT32_C(99);
    memset(&observed, 0, sizeof(observed));
    observed.struct_size = (uint32_t)sizeof(observed);
    observed.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_model_get_camera_count_ext(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_get_camera_ext(
                state->model, UINT64_C(0), &observed) == CNA_RESULT_SUCCESS &&
            observed.scene_node_index == INT32_C(7) &&
            observed.is_perspective == CNA_TRUE &&
            observed.has_infinite_far_plane == CNA_TRUE &&
            observed.has_authored_aspect_ratio == CNA_TRUE &&
            observed.aspect_ratio == 1.5F && observed.field_of_view == 1.0F &&
            observed.near_plane_distance == 0.5F &&
            observed.far_plane_distance == 0.0F &&
            observed.projection.m33 == -1.0F && observed.world_transform.m42 == 5.0F,
        "the camera retains every scalar, flag and matrix after caller mutation");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        state->demo,
        cna_model_get_camera_name_byte_count_ext(
            state->model, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(10) &&
            cna_model_copy_camera_name_ext(
                state->model, UINT64_C(0), buffer, UINT64_C(3), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(10) && buffer[0] == '#' &&
            cna_model_copy_camera_name_ext(
                state->model, UINT64_C(0), buffer, sizeof(buffer), &length) ==
                CNA_RESULT_SUCCESS &&
            length == UINT64_C(10) && memcmp(buffer, "MainCamera", 10U) == 0,
        "the copied camera name is exact and short output stays untouched");
    memset(&malformed, 0, sizeof(malformed));
    cna_demo_check(
        state->demo,
        cna_model_get_camera_ext(
            state->model, UINT64_C(1), &observed) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_get_camera_ext(
                state->model, UINT64_C(0), &malformed) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_add_camera_ext(state->model, NULL) == CNA_RESULT_INVALID_ARGUMENT,
        "camera Count bounds and malformed versioned structures are rejected");
    cna_demo_check(
        state->demo,
        cna_model_clear_cameras_ext(state->model) == CNA_RESULT_SUCCESS &&
            cna_model_get_camera_count_ext(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "clearing cameras restores the empty imported-camera list");
    return 1;
}

static int audit_skin_and_bounds(SceneDemo* state)
{
    const uint64_t mesh_indices[1] = {UINT64_C(0)};
    const uint64_t invalid_indices[1] = {UINT64_C(1)};
    int32_t hierarchy[1] = {-1};
    CNA_Matrix bind_pose[1] = {identity_matrix()};
    CNA_SkinningDataDescriptor descriptor;
    CNA_BoundingSphere sphere;
    CNA_Bool has_data = CNA_FALSE;
    CNA_Bool has_bound = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    uint64_t length = UINT64_MAX;
    uint64_t mesh_index = UINT64_MAX;
    char buffer[16];

    cna_demo_check(
        state->demo,
        cna_model_get_bounding_sphere_ext(
            state->model, &has_bound, &sphere) == CNA_RESULT_SUCCESS &&
            has_bound == CNA_TRUE && nearly_equal(sphere.center.x, 11.0F) &&
            nearly_equal(sphere.center.y, 2.0F) &&
            nearly_equal(sphere.center.z, 3.0F) && nearly_equal(sphere.radius, 2.0F),
        "the Model bound places its mesh sphere through the root-bone transform");
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.bone_count = 1;
    descriptor.skeleton_hierarchy = hierarchy;
    descriptor.bind_pose = bind_pose;
    descriptor.inverse_bind_pose = bind_pose;
    if (!cna_demo_check_result(
            state->demo,
            cna_skinning_data_create(&descriptor, &state->skeleton),
            "create the one-bone skeleton named by the Model skin")) {
        return 0;
    }
    if (!cna_demo_check_result(
            state->demo,
            cna_model_add_skin_ext(
                state->model, text_view("Body"), state->skeleton,
                mesh_indices, UINT64_C(1)),
            "map the retained skeleton to the Model's first mesh")) {
        return 0;
    }
    cna_demo_check(
        state->demo,
        cna_model_get_skin_count_ext(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_get_skin_ext(
                state->model, UINT64_C(0), &has_data, &count) == CNA_RESULT_SUCCESS &&
            has_data == CNA_TRUE && count == UINT64_C(1) &&
            cna_model_get_skin_mesh_index_ext(
                state->model, UINT64_C(0), UINT64_C(0), &mesh_index) ==
                CNA_RESULT_SUCCESS &&
            mesh_index == UINT64_C(0),
        "the skin reports its skeleton and exact model-mesh index");
    memset(buffer, '#', sizeof(buffer));
    cna_demo_check(
        state->demo,
        cna_model_get_skin_name_byte_count_ext(
            state->model, UINT64_C(0), &length) == CNA_RESULT_SUCCESS &&
            length == UINT64_C(4) &&
            cna_model_copy_skin_name_ext(
                state->model, UINT64_C(0), buffer, UINT64_C(2), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(4) && buffer[0] == '#' &&
            cna_model_copy_skin_name_ext(
                state->model, UINT64_C(0), buffer, sizeof(buffer), &length) ==
                CNA_RESULT_SUCCESS &&
            length == UINT64_C(4) && memcmp(buffer, "Body", 4U) == 0,
        "the skin name reports exact bytes and refuses partial output");

    cna_demo_check_result(
        state->demo, cna_skinning_data_destroy(state->skeleton),
        "release the source skeleton after the Model retains it");
    state->skeleton = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_model_create_skin_skeleton_handle_ext(
            state->model, UINT64_C(0), &state->skeleton_view) == CNA_RESULT_SUCCESS &&
            cna_skinning_data_get_bone_count(state->skeleton_view, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "a fresh owned skeleton handle remains available after source release");
    cna_demo_check(
        state->demo,
        cna_model_add_skin_ext(
            state->model, text_view("Unrigged"), CNA_INVALID_HANDLE,
            NULL, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_model_get_skin_ext(
                state->model, UINT64_C(1), &has_data, &count) == CNA_RESULT_SUCCESS &&
            has_data == CNA_FALSE && count == UINT64_C(0) &&
            cna_model_create_skin_skeleton_handle_ext(
                state->model, UINT64_C(1), &state->skeleton) ==
                CNA_RESULT_INVALID_STATE &&
            state->skeleton == CNA_INVALID_HANDLE,
        "a legal unrigged skin reports absence instead of returning a skeleton handle");
    cna_demo_check(
        state->demo,
        cna_model_add_skin_ext(
            state->model, text_view("Bad"), CNA_INVALID_HANDLE,
            invalid_indices, UINT64_C(1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_add_skin_ext(
                state->model, text_view("Bad"), CNA_INVALID_HANDLE,
                NULL, UINT64_C(1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_get_skin_count_ext(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_model_get_skin_mesh_index_ext(
                state->model, UINT64_C(0), UINT64_C(1), &mesh_index) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "invalid mesh mappings and Count bounds leave the skin list unchanged");
    cna_demo_check(
        state->demo,
        cna_model_clear_skins_ext(state->model) == CNA_RESULT_SUCCESS &&
            cna_model_get_skin_count_ext(state->model, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_skinning_data_get_bone_count(state->skeleton_view, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "clearing skins releases Model ownership while an issued skeleton handle stays valid");
    return 1;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    SceneDemo* state = (SceneDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle bones[1];
    CNA_ModelMeshHandle meshes[1];
    CNA_Matrix root_transform = identity_matrix();
    CNA_BoundingSphere mesh_bound = {{1.0F, 2.0F, 3.0F}, 2.0F};
    uint64_t count = UINT64_MAX;
    CNA_Bool has_bound = CNA_TRUE;
    CNA_BoundingSphere empty_bound;
    CNA_ModelHandle empty = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the device for a one-mesh Model")) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_create_default(&empty) == CNA_RESULT_SUCCESS &&
            cna_model_get_camera_count_ext(empty, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_get_skin_count_ext(empty, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_get_bounding_sphere_ext(empty, &has_bound, &empty_bound) ==
                CNA_RESULT_SUCCESS &&
            has_bound == CNA_FALSE &&
            cna_model_destroy(empty) == CNA_RESULT_SUCCESS,
        "an empty Model begins without cameras, skins or aggregate bounds");
    root_transform.m41 = 10.0F;
    cna_demo_check(
        state->demo,
        cna_model_bone_create(INT32_C(0), text_view("Root"), &state->root) ==
                CNA_RESULT_SUCCESS &&
            cna_model_bone_set_transform(state->root, root_transform) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_create_named(
                device, text_view("BodyMesh"), NULL, UINT64_C(0), &state->mesh) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_set_bounding_sphere(state->mesh, mesh_bound) ==
                CNA_RESULT_SUCCESS,
        "create a translated root and one authored mesh sphere");
    bones[0] = state->root;
    meshes[0] = state->mesh;
    if (!cna_demo_check_result(
            state->demo,
            cna_model_create(
                device, bones, UINT64_C(1), meshes, UINT64_C(1), &state->model),
            "construct the aggregate Model around the root and mesh")) {
        return CNA_RESULT_SUCCESS;
    }
    (void)audit_camera(state);
    (void)audit_skin_and_bounds(state);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SceneDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_Matrix sentinel;
    int device_ready;

    cna_demo_init(&demo, "c_api.graphics.model_scene_metadata", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.root = CNA_INVALID_HANDLE;
    state.mesh = CNA_INVALID_HANDLE;
    state.model = CNA_INVALID_HANDLE;
    state.skeleton = CNA_INVALID_HANDLE;
    state.skeleton_view = CNA_INVALID_HANDLE;
    memset(&sentinel, 0xA5, sizeof(sentinel));
    cna_demo_check(
        &demo,
        cna_matrix_create_infinite_perspective_field_of_view_ext(
            1.0F, 1.5F, 0.5F, &state.infinite_projection) == CNA_RESULT_SUCCESS &&
            state.infinite_projection.m33 == -1.0F &&
            state.infinite_projection.m34 == -1.0F &&
            state.infinite_projection.m44 == 0.0F,
        "create the exact limiting perspective matrix with no far plane");
    cna_demo_check(
        &demo,
        cna_matrix_create_infinite_perspective_field_of_view_ext(
            0.0F, 1.5F, 0.5F, &sentinel) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_matrix_create_infinite_perspective_field_of_view_ext(
                1.0F, 1.5F, 0.0F, &sentinel) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_matrix_create_infinite_perspective_field_of_view_ext(
                1.0F, 1.5F, 0.5F, NULL) == CNA_RESULT_INVALID_ARGUMENT,
        "invalid field of view, near plane and null output are refused");

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game),
            "create the graphics owner for Model scene metadata")) {
        return cna_demo_finish(&demo);
    }
    device_ready = cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_set_graphics_profile(
                       manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
                   cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(
            &demo, device_ready,
            "create a HiDef device for Model and ModelMesh ownership")) {
        cna_demo_check_result(
            &demo, cna_game_run_one_frame(game),
            "run one frame containing the scene-metadata audit");
        cna_demo_check(
            &demo, state.loads == 1,
            "the scene-metadata audit ran exactly once in the load callback");
    }
    cna_demo_check(
        &demo, cleanup_state(&state),
        "release the Model, mesh, bone and final skeleton view");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
