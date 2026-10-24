/* SPDX-License-Identifier: MS-PL */
/* CNB model mesh, skeleton, animation, light, camera and document codec through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t expected)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static void identity(float* matrix, float diagonal)
{
    memset(matrix, 0, 16U * sizeof(float));
    matrix[0] = diagonal;
    matrix[5] = diagonal;
    matrix[10] = diagonal;
    matrix[15] = 1.0F;
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

static int inspect_scene(CNA_CnbModelDataHandle model)
{
    CNA_CnbModelInfo model_info;
    CNA_CnbMeshInfo mesh;
    CNA_CnbSkeletonInfo skeleton;
    CNA_CnbModelLight light;
    CNA_CnbModelCamera camera;
    CNA_ClipTargetSpaceEXT target_space = CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT;
    CNA_KeyframeEXT keys[2];
    int32_t hierarchy[2] = {INT32_C(9), INT32_C(9)};
    uint32_t part_indices[2] = {UINT32_MAX, UINT32_MAX};
    float matrices[32];
    char name[16];
    double duration = 0.0;
    uint64_t count = UINT64_C(0);
    int32_t bone_index = INT32_C(-1);

    memset(&model_info, 0, sizeof(model_info));
    model_info.struct_size = (uint32_t)sizeof(model_info);
    model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    if (cna_cnb_model_get_info(model, &model_info) != CNA_RESULT_SUCCESS ||
        model_info.bone_count != UINT64_C(2) || model_info.part_count != UINT64_C(1) ||
        model_info.mesh_count != UINT64_C(1) || model_info.animation_count != UINT64_C(1) ||
        model_info.light_count != UINT64_C(1) || model_info.has_skeleton != CNA_TRUE) {
        return 0;
    }
    memset(&mesh, 0, sizeof(mesh));
    mesh.struct_size = (uint32_t)sizeof(mesh);
    mesh.struct_version = CNA_CNB_MESH_INFO_STRUCT_VERSION;
    if (cna_cnb_model_get_mesh(model, UINT64_C(0), &mesh) != CNA_RESULT_SUCCESS ||
        mesh.parent_bone != INT32_C(1) || mesh.part_index_count != UINT64_C(1) ||
        cna_cnb_model_copy_mesh_name(
            model, UINT64_C(0), name, sizeof(name), &count) != CNA_RESULT_SUCCESS ||
        count != UINT64_C(4) || memcmp(name, "body", 4U) != 0 ||
        cna_cnb_model_copy_mesh_part_indices(
            model, UINT64_C(0), part_indices, UINT64_C(2), &count) != CNA_RESULT_SUCCESS ||
        count != UINT64_C(1) || part_indices[0] != UINT32_C(0)) {
        return 0;
    }
    memset(&skeleton, 0, sizeof(skeleton));
    skeleton.struct_size = (uint32_t)sizeof(skeleton);
    skeleton.struct_version = CNA_CNB_SKELETON_INFO_STRUCT_VERSION;
    if (cna_cnb_model_get_skeleton(model, &skeleton) != CNA_RESULT_SUCCESS ||
        skeleton.joint_count != UINT64_C(2) || skeleton.has_root_prefix != CNA_TRUE ||
        cna_cnb_model_copy_skeleton_hierarchy(
            model, hierarchy, UINT64_C(2), &count) != CNA_RESULT_SUCCESS ||
        count != UINT64_C(2) || hierarchy[0] != INT32_C(-1) || hierarchy[1] != INT32_C(0) ||
        cna_cnb_model_copy_skeleton_matrices(
            model, CNA_CNB_SKELETON_MATRIX_BIND_POSE, matrices, UINT64_C(32), &count) !=
            CNA_RESULT_SUCCESS ||
        count != UINT64_C(32) || matrices[0] != 1.0F || matrices[16] != 2.0F ||
        cna_cnb_model_copy_skeleton_matrices(
            model, CNA_CNB_SKELETON_MATRIX_INVERSE_BIND_POSE,
            matrices, UINT64_C(32), &count) != CNA_RESULT_SUCCESS ||
        matrices[0] != 3.0F || matrices[16] != 4.0F ||
        cna_cnb_model_copy_skeleton_matrices(
            model, CNA_CNB_SKELETON_MATRIX_ROOT_PREFIX,
            matrices, UINT64_C(32), &count) != CNA_RESULT_SUCCESS ||
        matrices[0] != 5.0F || matrices[16] != 6.0F) {
        return 0;
    }
    if (cna_cnb_model_copy_animation_name(
            model, UINT64_C(0), name, sizeof(name), &count) != CNA_RESULT_SUCCESS ||
        count != UINT64_C(4) || memcmp(name, "walk", 4U) != 0 ||
        cna_cnb_model_get_animation(
            model, UINT64_C(0), &duration, &count, &target_space) != CNA_RESULT_SUCCESS ||
        duration != 2.0 || count != UINT64_C(1) ||
        target_space != CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT ||
        cna_cnb_model_get_animation_track(
            model, UINT64_C(0), UINT64_C(0), &bone_index, &count) != CNA_RESULT_SUCCESS ||
        bone_index != INT32_C(1) || count != UINT64_C(2) ||
        cna_cnb_model_copy_animation_keyframes(
            model, UINT64_C(0), UINT64_C(0), keys, UINT64_C(2), &count) !=
            CNA_RESULT_SUCCESS ||
        count != UINT64_C(2) || keys[0].time_seconds != 0.0 ||
        keys[1].time_seconds != 2.0 || keys[1].translation.x != 4.0F ||
        keys[1].rotation.w != 1.0F || keys[1].scale.y != 1.0F) {
        return 0;
    }
    memset(&light, 0, sizeof(light));
    if (cna_cnb_model_get_light(model, UINT64_C(0), &light) != CNA_RESULT_SUCCESS ||
        light.direction[1] != -1.0F || light.diffuse_color[0] != 1.0F ||
        light.diffuse_color[1] != 0.5F || light.diffuse_color[2] != 0.25F) {
        return 0;
    }
    memset(&camera, 0xA5, sizeof(camera));
    camera.struct_size = (uint32_t)sizeof(camera);
    camera.struct_version = CNA_CNB_MODEL_CAMERA_STRUCT_VERSION;
    memset(name, 0xA5, sizeof(name));
    return cna_cnb_model_get_camera_count(model, &count) == CNA_RESULT_SUCCESS &&
           count == UINT64_C(1) &&
           cna_cnb_model_get_camera(model, UINT64_C(0), &camera) == CNA_RESULT_SUCCESS &&
           camera.scene_node_index == INT32_C(1) && camera.perspective == CNA_TRUE &&
           bytes_are(camera.reserved, sizeof(camera.reserved), 0U) &&
           camera.yfov == 0.7F && camera.aspect_ratio == 1.25F &&
           camera.znear == 0.1F && camera.zfar == 50.0F &&
           camera.world_transform[0] == 1.0F && camera.world_transform[5] == 1.0F &&
           camera.world_transform[10] == 1.0F && camera.world_transform[13] == 1.8F &&
           camera.world_transform[15] == 1.0F &&
           cna_cnb_model_get_camera_name_size(model, UINT64_C(0), &count) ==
               CNA_RESULT_SUCCESS &&
           count == UINT64_C(5) &&
           cna_cnb_model_copy_camera_name(
               model, UINT64_C(0), name, UINT64_C(4), &count) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           count == UINT64_C(5) && bytes_are((const uint8_t*)name, sizeof(name), 0xA5U) &&
           cna_cnb_model_copy_camera_name(
               model, UINT64_C(0), name, sizeof(name), &count) == CNA_RESULT_SUCCESS &&
           count == UINT64_C(5) && memcmp(name, "Front", 5U) == 0 &&
           cna_cnb_model_get_camera(model, UINT64_C(1), &camera) ==
               CNA_RESULT_INVALID_ARGUMENT;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CnbModelDataHandle model = CNA_INVALID_HANDLE;
    CNA_CnbModelDataHandle decoded = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbModelPartInfo part;
    CNA_CnbMeshInfo mesh;
    CNA_CnbSkeletonInfo skeleton;
    CNA_CnbModelInfo model_info;
    CNA_CnbModelLight light;
    CNA_CnbModelCamera camera;
    CNA_KeyframeEXT keys[2];
    CNA_BoneTrackEXTDescriptor track;
    CNA_AnimationClipEXTDescriptor clip;
    CNA_ClipTargetSpaceEXT target_space = CNA_CLIP_TARGET_SPACE_JOINT_PALETTE_EXT;
    uint8_t vertex_bytes[36];
    uint8_t index_bytes[6] = {0U, 0U, 1U, 0U, 2U, 0U};
    uint8_t image[65536];
    uint8_t short_image[65536];
    int32_t hierarchy[2] = {INT32_C(-1), INT32_C(0)};
    int32_t copied_hierarchy[2] = {INT32_C(9), INT32_C(9)};
    uint32_t mesh_parts[1] = {UINT32_C(0)};
    uint32_t copied_parts[2] = {UINT32_MAX, UINT32_MAX};
    float bone_transform[16];
    float bind[32];
    float inverse[32];
    float prefix[32];
    float copied_matrices[32];
    char text[32];
    uint64_t index = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint64_t image_size = UINT64_C(0);
    uint32_t asset_type = UINT32_C(0);
    uint32_t byte_index;
    double duration = 0.0;
    int32_t track_bone = INT32_C(-1);
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_model_scene", argc, argv, 1);

    if (!cna_demo_check_result(
            &demo, cna_cnb_model_create(&model),
            "create the neutral model that will own the complete scene data")) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_cnb_model_set_flags(model, CNA_TRUE, CNA_TRUE),
        "mark the model as glTF-lit with a real bone hierarchy");
    identity(bone_transform, 1.0F);
    cna_demo_check(
        &demo,
        cna_cnb_model_add_bone(
            model, text_view("root"), INT32_C(-1), bone_transform, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append the root scene bone");
    identity(bone_transform, 2.0F);
    cna_demo_check(
        &demo,
        cna_cnb_model_add_bone(
            model, text_view("child"), INT32_C(0), bone_transform, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(1),
        "append the child scene bone");
    memset(&part, 0, sizeof(part));
    part.struct_size = (uint32_t)sizeof(part);
    part.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    part.vertex_stride = UINT32_C(12);
    part.vertex_count = UINT32_C(3);
    part.index_count = UINT32_C(3);
    part.index_element_size = UINT32_C(2);
    part.primitive_topology = UINT32_C(4);
    part.primitive_count = UINT32_C(1);
    part.effect_kind = CNA_CNB_EFFECT_KIND_PBR;
    cna_demo_check(
        &demo,
        cna_cnb_model_add_part(
            model, &part, text_view("triangle"), text_view(""), &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append the scene's one valid triangle part");
    for (byte_index = UINT32_C(0); byte_index < sizeof(vertex_bytes); ++byte_index) {
        vertex_bytes[byte_index] = (uint8_t)(byte_index + UINT32_C(1));
    }
    cna_demo_check(
        &demo,
        cna_cnb_model_set_part_vertex_bytes(
            model, UINT64_C(0), vertex_bytes, sizeof(vertex_bytes)) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_set_part_index_bytes(
                model, UINT64_C(0), index_bytes, sizeof(index_bytes)) == CNA_RESULT_SUCCESS,
        "attach consistent copied vertex and index bytes before encoding");

    cna_demo_check(
        &demo,
        cna_cnb_model_add_mesh(
            model, text_view("body"), INT32_C(1), mesh_parts, UINT64_C(1), &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append a named mesh attached to the child bone and triangle part");
    mesh_parts[0] = UINT32_C(99);
    memset(&mesh, 0, sizeof(mesh));
    mesh.struct_size = (uint32_t)sizeof(mesh);
    mesh.struct_version = CNA_CNB_MESH_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_mesh(model, UINT64_C(0), &mesh) == CNA_RESULT_SUCCESS &&
            mesh.parent_bone == INT32_C(1) && mesh.part_index_count == UINT64_C(1) &&
            mesh.reserved == UINT32_C(0),
        "the mesh publishes its exact parent and copied draw-list length");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_model_get_mesh_name_size(model, UINT64_C(0), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(4) &&
            cna_cnb_model_copy_mesh_name(
                model, UINT64_C(0), text, UINT64_C(3), &image_size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            image_size == UINT64_C(4) && bytes_are((const uint8_t*)text, sizeof(text), 0xA5U) &&
            cna_cnb_model_copy_mesh_name(
                model, UINT64_C(0), text, sizeof(text), &image_size) == CNA_RESULT_SUCCESS &&
            image_size == UINT64_C(4) && memcmp(text, "body", 4U) == 0,
        "the mesh name copies exactly and atomically");
    memset(copied_parts, 0xA5, sizeof(copied_parts));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_mesh_part_indices(
            model, UINT64_C(0), copied_parts, UINT64_C(0), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(1) &&
            cna_cnb_model_copy_mesh_part_indices(
                model, UINT64_C(0), copied_parts, UINT64_C(2), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && copied_parts[0] == UINT32_C(0),
        "the mesh owns its exact ordered part-index list");

    identity(bind, 1.0F);
    identity(bind + 16, 2.0F);
    identity(inverse, 3.0F);
    identity(inverse + 16, 4.0F);
    identity(prefix, 5.0F);
    identity(prefix + 16, 6.0F);
    cna_demo_check_result(
        &demo,
        cna_cnb_model_set_skeleton(
            model, hierarchy, UINT64_C(2), bind, inverse, prefix),
        "set a two-joint skeleton with bind, inverse and root-prefix matrices");
    memset(hierarchy, 0, sizeof(hierarchy));
    memset(bind, 0, sizeof(bind));
    memset(inverse, 0, sizeof(inverse));
    memset(prefix, 0, sizeof(prefix));
    memset(&skeleton, 0, sizeof(skeleton));
    skeleton.struct_size = (uint32_t)sizeof(skeleton);
    skeleton.struct_version = CNA_CNB_SKELETON_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_skeleton(model, &skeleton) == CNA_RESULT_SUCCESS &&
            skeleton.joint_count == UINT64_C(2) && skeleton.has_root_prefix == CNA_TRUE &&
            bytes_are(skeleton.reserved, sizeof(skeleton.reserved), 0U),
        "the skeleton reports its exact versioned shape");
    memset(copied_hierarchy, 0xA5, sizeof(copied_hierarchy));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_skeleton_hierarchy(
            model, copied_hierarchy, UINT64_C(1), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) &&
            cna_cnb_model_copy_skeleton_hierarchy(
                model, copied_hierarchy, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied_hierarchy[0] == INT32_C(-1) && copied_hierarchy[1] == INT32_C(0),
        "the copied joint hierarchy is atomic and independent of caller storage");
    memset(copied_matrices, 0, sizeof(copied_matrices));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_skeleton_matrices(
            model, CNA_CNB_SKELETON_MATRIX_BIND_POSE,
            copied_matrices, UINT64_C(32), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(32) && copied_matrices[0] == 1.0F &&
            copied_matrices[16] == 2.0F &&
            cna_cnb_model_copy_skeleton_matrices(
                model, CNA_CNB_SKELETON_MATRIX_INVERSE_BIND_POSE,
                copied_matrices, UINT64_C(32), &count) == CNA_RESULT_SUCCESS &&
            copied_matrices[0] == 3.0F && copied_matrices[16] == 4.0F &&
            cna_cnb_model_copy_skeleton_matrices(
                model, CNA_CNB_SKELETON_MATRIX_ROOT_PREFIX,
                copied_matrices, UINT64_C(32), &count) == CNA_RESULT_SUCCESS &&
            copied_matrices[0] == 5.0F && copied_matrices[16] == 6.0F,
        "all three skeleton matrix sets retain distinct copied values");

    memset(keys, 0, sizeof(keys));
    keys[0].time_seconds = 0.0;
    keys[0].rotation.w = 1.0F;
    keys[0].scale.x = 1.0F;
    keys[0].scale.y = 1.0F;
    keys[0].scale.z = 1.0F;
    keys[1] = keys[0];
    keys[1].time_seconds = 2.0;
    keys[1].translation.x = 4.0F;
    memset(&track, 0, sizeof(track));
    track.bone_index = INT32_C(1);
    track.keyframes = keys;
    track.keyframe_count = UINT64_C(2);
    memset(&clip, 0, sizeof(clip));
    clip.duration_seconds = 2.0;
    clip.tracks = &track;
    clip.track_count = UINT64_C(1);
    cna_demo_check(
        &demo,
        cna_cnb_model_add_animation(
            model, text_view("walk"), &clip, CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append a copied two-key scene-node animation track");
    memset(keys, 0, sizeof(keys));
    memset(&track, 0, sizeof(track));
    memset(&clip, 0, sizeof(clip));
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_model_get_animation_name_size(model, UINT64_C(0), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(4) &&
            cna_cnb_model_copy_animation_name(
                model, UINT64_C(0), text, sizeof(text), &image_size) == CNA_RESULT_SUCCESS &&
            image_size == UINT64_C(4) && memcmp(text, "walk", 4U) == 0,
        "the animation owns and returns its exact name");
    cna_demo_check(
        &demo,
        cna_cnb_model_get_animation(
            model, UINT64_C(0), &duration, &count, &target_space) == CNA_RESULT_SUCCESS &&
            duration == 2.0 && count == UINT64_C(1) &&
            target_space == CNA_CLIP_TARGET_SPACE_SCENE_NODE_EXT &&
            cna_cnb_model_get_animation_track(
                model, UINT64_C(0), UINT64_C(0), &track_bone, &count) == CNA_RESULT_SUCCESS &&
            track_bone == INT32_C(1) && count == UINT64_C(2),
        "the clip preserves duration, target index space and track shape");
    memset(keys, 0xA5, sizeof(keys));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_animation_keyframes(
            model, UINT64_C(0), UINT64_C(0), keys, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) &&
            cna_cnb_model_copy_animation_keyframes(
                model, UINT64_C(0), UINT64_C(0), keys, UINT64_C(2), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && keys[0].time_seconds == 0.0 &&
            keys[1].time_seconds == 2.0 && keys[1].translation.x == 4.0F &&
            keys[1].rotation.w == 1.0F && keys[1].scale.y == 1.0F,
        "keyframe copies are atomic and retain the caller-independent timeline");

    memset(&light, 0, sizeof(light));
    light.direction[1] = -1.0F;
    light.diffuse_color[0] = 1.0F;
    light.diffuse_color[1] = 0.5F;
    light.diffuse_color[2] = 0.25F;
    cna_demo_check(
        &demo,
        cna_cnb_model_add_light(model, &light, &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append one indexed directional light value");
    memset(&light, 0, sizeof(light));
    cna_demo_check(
        &demo,
        cna_cnb_model_get_light(model, UINT64_C(0), &light) == CNA_RESULT_SUCCESS &&
            light.direction[1] == -1.0F && light.diffuse_color[0] == 1.0F &&
            light.diffuse_color[1] == 0.5F && light.diffuse_color[2] == 0.25F,
        "the model returns the exact copied light direction and colour");
    memset(&camera, 0, sizeof(camera));
    camera.struct_size = (uint32_t)sizeof(camera);
    camera.struct_version = CNA_CNB_MODEL_CAMERA_STRUCT_VERSION;
    camera.scene_node_index = INT32_C(1);
    camera.perspective = CNA_TRUE;
    camera.yfov = 0.7F;
    camera.aspect_ratio = 1.25F;
    camera.znear = 0.1F;
    camera.zfar = 50.0F;
    identity(camera.world_transform, 1.0F);
    camera.world_transform[13] = 1.8F;
    cna_demo_check(
        &demo,
        cna_cnb_model_add_camera(model, text_view("Front"), &camera, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append a named perspective camera attached to the child scene node");
    memset(&camera, 0, sizeof(camera));
    cna_demo_check(
        &demo, inspect_scene(model),
        "the authored graph, including its owned camera, is coherent before serialization");

    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_model(
                model, text_view("models/hero"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > CNA_CNB_FORMAT_HEADER_SIZE && image_size < sizeof(image),
            "the model encoder reports its exact complete document size")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_model(
            model, text_view("models/hero"), short_image, image_size - UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short model destination is reported without a partial document");
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_model(
                model, text_view("models/hero"), image, sizeof(image), &count) ==
                    CNA_RESULT_SUCCESS &&
                count == image_size &&
                cna_cnb_document_parse(
                    image, image_size, text_view("hero.cnb"), NULL, &document) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_document_get_asset_type_id(document, &asset_type) == CNA_RESULT_SUCCESS &&
                asset_type == CNA_CNB_ASSET_TYPE_MODEL,
            "encode and parse a complete document with the Model asset identity")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_decode_model(document, &decoded) == CNA_RESULT_SUCCESS &&
            inspect_scene(decoded),
        "the decoder restores every mesh, skeleton, clip, light and camera relationship exactly");
    cna_demo_check(
        &demo,
        cna_cnb_model_clear_skeleton(decoded) == CNA_RESULT_SUCCESS,
        "clear removes the decoded skeleton as one owned nested value");
    memset(&skeleton, 0, sizeof(skeleton));
    skeleton.struct_size = (uint32_t)sizeof(skeleton);
    skeleton.struct_version = CNA_CNB_SKELETON_INFO_STRUCT_VERSION;
    memset(&model_info, 0, sizeof(model_info));
    model_info.struct_size = (uint32_t)sizeof(model_info);
    model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_skeleton(decoded, &skeleton) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_clear_skeleton(decoded) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_get_info(decoded, &model_info) == CNA_RESULT_SUCCESS &&
            model_info.has_skeleton == CNA_FALSE,
        "skeleton clear is idempotent and absence remains an explicit state");

cleanup:
    releases_ok &= release_model(&decoded);
    releases_ok &= release_document(&document);
    releases_ok &= release_model(&model);
    cna_demo_check(
        &demo, releases_ok,
        "release both model graphs and the parsed document exactly once");
    return cna_demo_finish(&demo);
}
