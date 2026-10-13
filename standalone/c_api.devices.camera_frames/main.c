/* SPDX-License-Identifier: MS-PL */
/*
 * Camera frames from C: enumerate without opening hardware, then publish exact RGBA data through
 * CNA's synthetic camera and copy it into caller-owned textures.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct CameraDemoState {
    CnaDemo* demo;
    int validated;
} CameraDemoState;

static void descriptor_contract(CnaDemo* demo)
{
    CNA_CameraDeviceInfo info;
    memset(&info, 0x7f, sizeof(info));
    cna_demo_check(
        demo,
        cna_camera_device_info_init(&info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) &&
            info.struct_version == UINT32_C(1) &&
            info.position == CNA_CAMERA_POSITION_UNKNOWN,
        "the pure camera descriptor initializer writes its version and unknown facing");
}

static void enumeration_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_Bool supported = UINT8_C(9);
    uint64_t count = UINT64_C(0);
    uint64_t index;
    int entries_ok = 1;

    if (!cna_demo_check(
            demo,
            cna_camera_get_is_supported_ext(game, &supported) == CNA_RESULT_SUCCESS &&
                (supported == CNA_FALSE || supported == CNA_TRUE) &&
                cna_camera_get_count_ext(game, &count) == CNA_RESULT_SUCCESS,
            "driver support and the point-in-time camera count are independent readable facts")) {
        return;
    }
    printf("  platform camera driver: %u, enumerated cameras: %llu\n",
           (unsigned)supported, (unsigned long long)count);

    for (index = UINT64_C(0); index < count; ++index) {
        CNA_CameraDeviceInfo info;
        uint64_t bytes = UINT64_C(0);
        uint64_t copied = UINT64_C(0);
        char* name = NULL;
        entries_ok = entries_ok &&
            cna_camera_get_info_at_ext(game, index, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) &&
            info.struct_version == UINT32_C(1) &&
            info.position <= CNA_CAMERA_POSITION_MAXIMUM &&
            cna_camera_get_name_size_at_ext(game, index, &bytes) == CNA_RESULT_SUCCESS;
        if (entries_ok) {
            name = (char*)malloc((size_t)bytes + 1U);
            if (name != NULL) {
                name[bytes] = '\0';
            }
            entries_ok = name != NULL &&
                cna_camera_copy_name_at_ext(game, index, name, bytes + 1U, &copied) ==
                    CNA_RESULT_SUCCESS &&
                copied == bytes && strlen(name) == (size_t)bytes;
        }
        if (entries_ok) {
            printf("    camera #%llu: position %u, name '%s'\n",
                   (unsigned long long)index, (unsigned)info.position, name);
        }
        free(name);
    }
    cna_demo_check(
        demo, entries_ok,
        "every enumerated camera has a versioned facing descriptor and exact UTF-8 name bytes");

    {
        CNA_CameraDeviceInfo info;
        uint64_t bytes = UINT64_C(77);
        unsigned char guard[8];
        memset(&info, 0x5a, sizeof(info));
        memset(guard, 0x5a, sizeof(guard));
        cna_demo_check(
            demo,
            cna_camera_get_info_at_ext(game, count, &info) == CNA_RESULT_INVALID_ARGUMENT &&
                cna_camera_get_name_size_at_ext(game, count, &bytes) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_camera_copy_name_at_ext(
                    game, count, (char*)guard, sizeof(guard), &bytes) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                bytes == UINT64_C(77) && guard[0] == 0x5aU &&
                guard[sizeof(guard) - 1U] == 0x5aU &&
                ((const unsigned char*)&info)[0] == 0x5aU,
            "the index equal to the camera count is refused without changing descriptor or text outputs");
    }
}

static int same_color(CNA_Color left, CNA_Color right)
{
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}

static void synthetic_camera_contract(
    CNA_Handle game, CNA_Handle graphics_device, CnaDemo* demo)
{
    const CNA_Color pixels[4] = {
        {10U, 20U, 30U, 255U}, {40U, 50U, 60U, 200U},
        {70U, 80U, 90U, 150U}, {100U, 110U, 120U, 100U}};
    const CNA_Texture2DCreateInfo matching_info = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 2U, 2U, CNA_FALSE,
        {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR};
    const CNA_Texture2DCreateInfo other_info = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 4U, 4U, CNA_FALSE,
        {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR};
    CNA_CameraHandle camera = CNA_INVALID_HANDLE;
    CNA_Handle matching = CNA_INVALID_HANDLE;
    CNA_Handle other = CNA_INVALID_HANDLE;
    CNA_CameraState state = UINT32_MAX;
    CNA_Bool acquired = UINT8_C(9);
    CNA_Color read_back[4];
    uint64_t copied = UINT64_C(0);
    int32_t width = -1;
    int32_t height = -1;

    if (!cna_demo_check(
            demo,
            cna_camera_create_with_test_backend_ext(game, &camera) == CNA_RESULT_SUCCESS &&
                camera != CNA_INVALID_HANDLE &&
                cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
                state == CNA_CAMERA_STATE_OPENING &&
                cna_camera_get_frame_width_ext(camera, &width) == CNA_RESULT_SUCCESS && width == 0 &&
                cna_camera_get_frame_height_ext(camera, &height) == CNA_RESULT_SUCCESS && height == 0,
            "a synthetic camera starts permission-pending with no known frame size")) {
        if (camera != CNA_INVALID_HANDLE) {
            (void)cna_camera_destroy(camera);
        }
        return;
    }

    cna_demo_check(
        demo,
        cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_DENIED) == CNA_RESULT_SUCCESS &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_DENIED &&
            cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_LOST) == CNA_RESULT_SUCCESS &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_LOST &&
            cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_OPENING) == CNA_RESULT_SUCCESS,
        "the synthetic backend round-trips permission denied, lost and opening states");

    cna_demo_check(
        demo,
        cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_CLOSED) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_camera_set_test_state_ext(camera, CNA_CAMERA_STATE_NOT_SUPPORTED) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_camera_set_test_state_ext(
                camera, CNA_CAMERA_STATE_MAXIMUM + UINT32_C(1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_OPENING,
        "unreachable closed/not-supported and undefined synthetic states are refused without mutation");

    cna_demo_check(
        demo,
        cna_camera_set_test_frame_ext(camera, 2, 2, pixels, UINT64_C(3)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_camera_set_test_frame_ext(camera, -1, 2, pixels, UINT64_C(4)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_OPENING,
        "mismatched pixel count and negative dimensions are refused before changing camera state");

    cna_demo_check(
        demo,
        cna_camera_set_test_frame_ext(camera, 2, 2, pixels, UINT64_C(4)) ==
                CNA_RESULT_SUCCESS &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_READY &&
            cna_camera_get_frame_width_ext(camera, &width) == CNA_RESULT_SUCCESS && width == 2 &&
            cna_camera_get_frame_height_ext(camera, &height) == CNA_RESULT_SUCCESS && height == 2,
        "publishing four pixels makes the camera ready with an exact two-by-two format");

    if (!cna_demo_check(
            demo,
            cna_texture2d_create(graphics_device, &matching_info, &matching) == CNA_RESULT_SUCCESS &&
                cna_texture2d_create(graphics_device, &other_info, &other) == CNA_RESULT_SUCCESS,
            "caller-owned matching and mismatched Color textures are available")) {
        if (other != CNA_INVALID_HANDLE) {
            (void)cna_texture2d_destroy(other);
        }
        if (matching != CNA_INVALID_HANDLE) {
            (void)cna_texture2d_destroy(matching);
        }
        (void)cna_camera_destroy(camera);
        return;
    }

    memset(read_back, 0, sizeof(read_back));
    cna_demo_check(
        demo,
        cna_camera_try_acquire_frame_ext(camera, matching, &acquired) == CNA_RESULT_SUCCESS &&
            acquired == CNA_TRUE &&
            cna_texture2d_get_data_rgba8(
                matching, read_back, UINT64_C(4), &copied) == CNA_RESULT_SUCCESS &&
            copied == UINT64_C(4) && same_color(read_back[0], pixels[0]) &&
            same_color(read_back[1], pixels[1]) && same_color(read_back[2], pixels[2]) &&
            same_color(read_back[3], pixels[3]),
        "frame acquisition copies every exact RGBA pixel into the caller-owned matching texture");

    cna_demo_check(
        demo,
        cna_camera_try_acquire_frame_ext(camera, other, &acquired) == CNA_RESULT_SUCCESS &&
            acquired == CNA_FALSE,
        "a texture with the wrong dimensions is an ordinary not-acquired answer");

    cna_demo_check(
        demo,
        cna_camera_set_test_frame_ext(camera, 0, 0, NULL, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_CAMERA_STATE_OPENING &&
            cna_camera_get_frame_width_ext(camera, &width) == CNA_RESULT_SUCCESS && width == 0 &&
            cna_camera_get_frame_height_ext(camera, &height) == CNA_RESULT_SUCCESS && height == 0 &&
            cna_camera_try_acquire_frame_ext(camera, matching, &acquired) == CNA_RESULT_SUCCESS &&
            acquired == CNA_FALSE,
        "clearing the frame restores permission-pending zero-size state and ordinary no-frame false");

    cna_demo_check_result(demo, cna_camera_destroy(camera),
                          "destroy closes the synthetic camera and releases its handle");
    memset(read_back, 0, sizeof(read_back));
    cna_demo_check(
        demo,
        cna_camera_destroy(camera) == CNA_RESULT_INVALID_HANDLE &&
            cna_camera_get_state_ext(camera, &state) == CNA_RESULT_INVALID_HANDLE &&
            cna_texture2d_get_data_rgba8(
                matching, read_back, UINT64_C(4), &copied) == CNA_RESULT_SUCCESS &&
            copied == UINT64_C(4) && same_color(read_back[0], pixels[0]) &&
            same_color(read_back[3], pixels[3]),
        "camera release refuses stale access while the acquired caller texture remains readable");

    cna_demo_check(
        demo,
        cna_texture2d_destroy(other) == CNA_RESULT_SUCCESS &&
            cna_texture2d_destroy(matching) == CNA_RESULT_SUCCESS,
        "both caller-owned textures release independently after the camera");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    CameraDemoState* state = (CameraDemoState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    enumeration_contract(game, state->demo);
    if (cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &graphics_device),
            "obtain the game's graphics device for caller-owned camera textures")) {
        synthetic_camera_contract(game, graphics_device, state->demo);
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CameraDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.camera_frames", argc, argv, 0);
    descriptor_contract(&demo);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
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
    if (!cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                               "create a HiDef graphics device for texture read-back")) {
        (void)cna_game_destroy(game);
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "run one frame for owner-thread camera operations");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the camera audit once");
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                          "release the camera demo's graphics manager");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
