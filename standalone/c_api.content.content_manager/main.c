/* SPDX-License-Identifier: MS-PL */
/*
 * Content from C: an owned ContentManager rooted at the demos' assets loads a PNG texture and a
 * compiled SpriteFont. A loaded font comes back as two owned handles -- the font and the atlas
 * texture it draws from -- and the font has to be destroyed before its atlas.
 * Root changes preserve old cache entries and redirect new loads. Both original resources remain
 * usable after unloading and destroying the content manager.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ContentState {
    CnaDemo* demo;
    CNA_Handle content;      /* owned ContentManager */
    CNA_Handle game_content; /* borrowed from the game; never destroy or release */
    CNA_Handle checker;      /* owned Texture2D */
    CNA_Handle font;         /* owned SpriteFont */
    CNA_Handle font_atlas;   /* owned Texture2D the font draws from */
    CNA_EffectHandle effect; /* owned BasicEffect loaded from an authored CNJ */
    CNA_Handle sprite_batch; /* owned */
    CNA_Vector2 text_size;
    int frames;
} ContentState;

static const char text[] = "Loaded through the C API";

static CNA_StringView view(const char* s)
{
    CNA_StringView v;
    v.data = s;
    v.byte_length = strlen(s);
    return v;
}

static int effect_has_authored_state(CNA_EffectHandle effect)
{
    CNA_Vector3 vector = {0};
    CNA_Bool vertex_color = CNA_FALSE;
    float scalar = 0.0F;
    return cna_basic_effect_get_diffuse_color(effect, &vector) == CNA_RESULT_SUCCESS &&
           vector.x == 0.125F && vector.y == 0.25F && vector.z == 0.5F &&
           cna_basic_effect_get_emissive_color(effect, &vector) == CNA_RESULT_SUCCESS &&
           vector.x == 0.0625F && vector.y == 0.125F && vector.z == 0.25F &&
           cna_basic_effect_get_specular_power(effect, &scalar) == CNA_RESULT_SUCCESS &&
           scalar == 17.0F &&
           cna_basic_effect_get_alpha(effect, &scalar) == CNA_RESULT_SUCCESS && scalar == 0.75F &&
           cna_basic_effect_get_vertex_color_enabled(effect, &vertex_color) == CNA_RESULT_SUCCESS &&
           vertex_color == CNA_TRUE;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    ContentState* state = (ContentState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_ContentManagerCreateInfo content_info;
    CNA_Texture2DInfo info;
    CNA_Result result;
    int root_changed = 0;
    (void)game_time;
    (void)out_error;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    /* The root is prepended to every asset name, as Content.RootDirectory is in XNA. */
    memset(&content_info, 0, sizeof(content_info));
    content_info.struct_size = (uint32_t)sizeof(content_info);
    content_info.struct_version = UINT32_C(1);
    content_info.root_directory = view("assets");
    if (!cna_demo_check_result(state->demo, cna_content_manager_create(graphics_device, &content_info, &state->content),
                               "cna_content_manager_create")) {
        return CNA_RESULT_SUCCESS; /* the checks report it; the frames still run */
    }

    /* The game owns a separate manager value. Its borrowed handle remains stable across
       copying another manager into that value; it is not an independently owned child. */
    if (cna_demo_check_result(state->demo, cna_game_get_content_manager_ext(game, &state->game_content),
                              "borrow the game's own content manager")) {
        CNA_Handle same_manager = CNA_INVALID_HANDLE;
        result = cna_content_manager_destroy(state->game_content);
        cna_demo_check(state->demo, result == CNA_RESULT_INVALID_STATE,
                       "the game's borrowed content manager refuses independent destruction");
        cna_demo_check_result(state->demo,
            cna_content_manager_set_root_directory(state->game_content, view("game-only-root")),
            "give the game manager a distinguishable root before copying");
        /* Copy before loading assets: this example tests configuration, not cached-object sharing. */
        cna_demo_check_result(state->demo, cna_game_set_content_manager_ext(game, state->content),
                              "copy the caller-owned manager into the game's existing manager value");
        result = cna_game_get_content_manager_ext(game, &same_manager);
        cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS && same_manager == state->game_content,
                       "borrowing after replacement returns the same game-owned manager handle");
    }

    /* Asset names have no extension: the manager finds textures/checker.png. */
    if (cna_demo_check_result(state->demo,
                              cna_content_manager_load_texture2d(state->content, view("textures/checker"), &state->checker),
                              "cna_content_manager_load_texture2d(\"textures/checker\")")) {
        memset(&info, 0, sizeof(info));
        info.struct_size = (uint32_t)sizeof(info);
        info.struct_version = UINT32_C(1);
        if (cna_texture2d_get_info(state->checker, &info) == CNA_RESULT_SUCCESS) {
            printf("  textures/checker: %ux%u, %u level(s)\n", (unsigned)info.width, (unsigned)info.height,
                   (unsigned)info.level_count);
            cna_demo_check(state->demo, info.width == 64U && info.height == 64U, "the checker texture is 64x64");
        }
    }

    /* A compiled font: two owned handles, the font and its atlas. */
    if (cna_demo_check_result(state->demo,
                              cna_content_manager_load_sprite_font(state->content, view("fonts/pixel_font"),
                                                                   &state->font, &state->font_atlas),
                              "cna_content_manager_load_sprite_font(\"fonts/pixel_font\")") &&
        cna_demo_check_result(state->demo, cna_sprite_font_measure_utf8(state->font, view(text), &state->text_size),
                              "cna_sprite_font_measure_utf8")) {
        printf("  \"%s\" measures %.0f x %.0f\n", text, state->text_size.x, state->text_size.y);
        cna_demo_check(state->demo, state->text_size.x > 100.0f && state->text_size.y > 5.0f,
                       "the text has a size in the font");
    }
    /* A font retains its atlas: an early release must be refused, leaving both usable. */
    if (state->font != CNA_INVALID_HANDLE && state->font_atlas != CNA_INVALID_HANDLE) {
        result = cna_texture2d_destroy(state->font_atlas);
        cna_demo_check(state->demo, result == CNA_RESULT_INVALID_STATE,
                      "the live font prevents destroying its retained atlas");
        if (result == CNA_RESULT_SUCCESS) state->font_atlas = CNA_INVALID_HANDLE;
    }

    /* A stock-effect CNJ avoids compiled-shader capability differences while still exercising the
       canonical ContentManager.Load<Effect> path and authored material values. */
    if (cna_demo_check_result(
            state->demo,
            cna_content_manager_load_effect(
                state->content, view("effects/c_api_basic_effect"), &state->effect),
            "load an authored BasicEffect through ContentManager.Load<Effect>")) {
        cna_demo_check(
            state->demo, effect_has_authored_state(state->effect),
            "the loaded effect preserves every authored BasicEffect material value");
        cna_demo_check_result(
            state->demo, cna_effect_apply(state->effect),
            "the loaded stock effect applies through the ordinary Effect interface");
    }

    /* This C setter explicitly preserves cached assets while changing uncached resolution.
       Its StringView is copied during the call, so the caller can reuse the source bytes. */
    {
        char root_source[] = "assets/textures";
        char root_copy[64];
        uint64_t root_bytes = 0U, copied = 0U;
        CNA_Handle cached = CNA_INVALID_HANDLE, fresh = CNA_INVALID_HANDLE;
        if (cna_demo_check_result(state->demo,
                cna_content_manager_set_root_directory(state->content, view(root_source)),
                "change the C manager root without unloading cached assets")) {
            root_changed = 1;
            memset(root_source, '?', sizeof(root_source) - 1U);
            result = cna_content_manager_get_root_directory_size(state->content, &root_bytes);
            if (cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS &&
                               root_bytes == sizeof("assets/textures") - 1U && root_bytes < sizeof(root_copy),
                               "the root count reports the copied path's bytes, excluding a terminator")) {
                memset(root_copy, '#', sizeof(root_copy));
                result = cna_content_manager_copy_root_directory(state->content, root_copy, root_bytes, &copied);
                cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS && copied == root_bytes &&
                               memcmp(root_copy, "assets/textures", (size_t)root_bytes) == 0 && root_copy[root_bytes] == '#',
                               "the manager copied its root independently of the caller's reused buffer");
            }
            /* The setter copied the old assets root; it did not alias this caller-owned manager. */
            memset(root_copy, '#', sizeof(root_copy));
            copied = UINT64_MAX;
            result = cna_content_manager_copy_root_directory(state->game_content, root_copy, sizeof(root_copy), &copied);
            cna_demo_check(state->demo, result == CNA_RESULT_SUCCESS && copied == sizeof("assets") - 1U &&
                           memcmp(root_copy, "assets", sizeof("assets") - 1U) == 0 && root_copy[6] == '#',
                           "the game keeps the copied root when the caller-owned source changes later");
            /* textures/checker would not exist under assets/textures; this name is already cached. */
            if (cna_demo_check_result(state->demo,
                    cna_content_manager_load_texture2d(state->content, view("textures/checker"), &cached),
                    "load the previously cached logical name after changing roots")) {
                memset(&info, 0, sizeof(info));
                info.struct_size = (uint32_t)sizeof(info);
                info.struct_version = UINT32_C(1);
                cna_demo_check(state->demo, cna_texture2d_get_info(cached, &info) == CNA_RESULT_SUCCESS &&
                               info.width == 64U && info.height == 64U,
                               "the retained cache entry still describes the original checker");
                cna_demo_check_result(state->demo, cna_texture2d_destroy(cached), "release the extra cached texture handle");
            }
            if (cna_demo_check_result(state->demo,
                    cna_content_manager_load_texture2d(state->content, view("sprite_strip"), &fresh),
                    "an uncached logical name resolves under the new root")) {
                memset(&info, 0, sizeof(info));
                info.struct_size = (uint32_t)sizeof(info);
                info.struct_version = UINT32_C(1);
                cna_demo_check(state->demo, cna_texture2d_get_info(fresh, &info) == CNA_RESULT_SUCCESS &&
                               info.width == 128U && info.height == 32U,
                               "the new root resolves the four-frame sprite strip");
                cna_demo_check_result(state->demo, cna_texture2d_destroy(fresh), "release the newly loaded strip handle");
            }
        }
    }

    /* Unlike cached C++ objects, these independently owned C handles survive the manager.
       Check actual pixels and font measurement after each lifetime boundary, then draw them. */
    for (int stage = 0; stage < 2; ++stage) {
        CNA_Texture2DTransfer transfer;
        CNA_Color pixels[9];
        CNA_Vector2 measured;
        uint64_t required = 0U;
        result = stage == 0 ? cna_content_manager_unload(state->content)
                            : cna_content_manager_destroy(state->content);
        if (!cna_demo_check_result(state->demo, result,
                                  stage == 0 ? "unload the content manager" : "destroy the content manager")) {
            break;
        }
        if (stage == 1) state->content = CNA_INVALID_HANDLE;
        if (stage == 0 && root_changed) {
            CNA_Handle no_longer_cached = CNA_INVALID_HANDLE;
            CNA_ErrorInfo error_info;
            uint64_t exception_bytes = UINT64_MAX;
            uint64_t exception_copied = UINT64_MAX;
            char exception_type[128] = {0};
            int copied_exception = 0;
            result = cna_content_manager_load_texture2d(state->content, view("textures/checker"), &no_longer_cached);
            cna_demo_check(state->demo, result == CNA_RESULT_IO,
                           "Unload removes the cached name, which cannot resolve under the changed root");
            memset(&error_info, 0, sizeof(error_info));
            error_info.struct_size = (uint32_t)sizeof(error_info);
            error_info.struct_version = UINT32_C(1);
            if (cna_error_get_last_exception_type_size_ext(&exception_bytes) == CNA_RESULT_SUCCESS &&
                exception_bytes > 0U && exception_bytes < sizeof(exception_type) &&
                cna_error_copy_last_exception_type_ext(
                    exception_type, exception_bytes, &exception_copied) == CNA_RESULT_SUCCESS &&
                exception_copied == exception_bytes) {
                exception_type[exception_bytes] = '\0';
                copied_exception = 1;
            }
            printf("  missing load exception type: %s\n",
                   copied_exception ? exception_type : "<none>");
            cna_demo_check(
                state->demo,
                cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
                    error_info.result == CNA_RESULT_IO &&
                    error_info.category == CNA_ERROR_CATEGORY_IO && copied_exception &&
                    strcmp(
                        exception_type,
                        "Microsoft.Xna.Framework.Content.ContentLoadException") == 0,
                "the failed native load retains ContentLoadException for a binding to rethrow "
                "(CNA-DEMOS-ISSUE-103)");
            if (result == CNA_RESULT_SUCCESS)
                cna_demo_check_result(state->demo, cna_texture2d_destroy(no_longer_cached), "release unexpected cache reload");
        }
        printf("  checking owned resources after manager %s\n", stage == 0 ? "Unload" : "destruction");
        memset(&transfer, 0, sizeof(transfer));
        transfer.struct_size = (uint32_t)sizeof(transfer);
        transfer.struct_version = UINT32_C(1);
        transfer.has_rectangle = CNA_TRUE;
        transfer.rectangle.width = 9;
        transfer.rectangle.height = 1;
        transfer.element_count = 9U;
        if (cna_demo_check_result(state->demo,
                cna_texture2d_get_data(state->checker, CNA_TEXTURE_DATA_COLOR, &transfer, pixels, 9U, &required),
                "read owned texture after the lifetime boundary")) {
            cna_demo_check(state->demo, required == 9U && pixels[0].r == 255U && pixels[0].g == 255U &&
                          pixels[0].b == 255U && pixels[0].a == 255U && pixels[8].r == 72U &&
                          pixels[8].g == 61U && pixels[8].b == 139U && pixels[8].a == 255U,
                          "both original checker colors remain intact");
        }
        if (cna_demo_check_result(state->demo, cna_sprite_font_measure_utf8(state->font, view(text), &measured),
                                  "measure owned font after the lifetime boundary")) {
            cna_demo_check(state->demo, measured.x == state->text_size.x && measured.y == state->text_size.y,
                          "the owned font keeps its original metrics");
        }
        cna_demo_check(
            state->demo,
            state->effect != CNA_INVALID_HANDLE && effect_has_authored_state(state->effect) &&
                cna_effect_apply(state->effect) == CNA_RESULT_SUCCESS,
            stage == 0
                ? "the loaded Effect remains authored and usable after ContentManager.Unload"
                : "the loaded Effect remains authored and usable after ContentManager destruction");
    }
    return cna_sprite_batch_create(graphics_device, &state->sprite_batch);
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    ContentState* state = (ContentState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    static const CNA_Color background = {24, 28, 40, 255};
    ContentState* state = (ContentState*)context;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    result = cna_game_clear(game, background);
    if (result != CNA_RESULT_SUCCESS || state->checker == CNA_INVALID_HANDLE || state->font == CNA_INVALID_HANDLE) {
        return result;
    }
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    if (result == CNA_RESULT_SUCCESS) {
        CNA_SpriteCommand sprite;
        CNA_SpriteTextCommand label;
        memset(&sprite, 0, sizeof(sprite));
        sprite.struct_size = (uint32_t)sizeof(sprite);
        sprite.struct_version = UINT32_C(1);
        sprite.texture = state->checker;
        sprite.destination.x = 272;
        sprite.destination.y = 120;
        sprite.destination.width = 256;
        sprite.destination.height = 256;
        sprite.source.width = 64;
        sprite.source.height = 64;
        sprite.color.r = sprite.color.g = sprite.color.b = sprite.color.a = 255;
        result = cna_sprite_batch_submit_many(state->sprite_batch, &sprite, 1U);

        /* Text centred above the texture, drawn with the loaded font at twice its size. */
        memset(&label, 0, sizeof(label));
        label.struct_size = (uint32_t)sizeof(label);
        label.struct_version = UINT32_C(1);
        label.sprite_font = state->font;
        label.text = view(text);
        label.position.x = 400.0f - state->text_size.x;
        label.position.y = 60.0f;
        label.color.r = label.color.g = label.color.b = label.color.a = 255;
        label.scale.x = 2.0f;
        label.scale.y = 2.0f;
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_sprite_batch_draw_string(state->sprite_batch, &label);
        }
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_sprite_batch_end(state->sprite_batch);
        } else {
            (void)cna_sprite_batch_end(state->sprite_batch);
        }
    }
    if (result == CNA_RESULT_SUCCESS && state->frames == state->demo->frames) {
        CNA_Handle graphics_device = CNA_INVALID_HANDLE;
        if (cna_game_get_graphics_device(game, &graphics_device) == CNA_RESULT_SUCCESS) {
            static CNA_Color pixels[800 * 480];
            uint64_t count = 0U;
            if (cna_demo_check_result(state->demo,
                    cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, pixels, 800U * 480U, &count),
                    "read the combined sprite/text frame")) {
                const CNA_Color white = pixels[136 * 800 + 288];
                const CNA_Color purple = pixels[136 * 800 + 320];
                printf("  rendered checker: %u,%u,%u and %u,%u,%u\n",
                       white.r, white.g, white.b, purple.r, purple.g, purple.b);
                cna_demo_check(state->demo, count == 800U * 480U && white.r == 255U && white.g == 255U &&
                              white.b == 255U && purple.r == 72U && purple.g == 61U && purple.b == 139U,
                              "the loaded checker remains visible in the sprite/text batch");
                /* The text rectangle is above the checker and the clear color is dark. */
                unsigned bright = 0U;
                for (int y = 60; y < 100; ++y) {
                    for (int x = 112; x < 688; ++x) {
                        const CNA_Color pixel = pixels[y * 800 + x];
                        if (pixel.r > 240U && pixel.g > 240U && pixel.b > 240U) ++bright;
                    }
                }
                cna_demo_check(state->demo, bright > 100U,
                              "the retained font atlas still draws visible text after the refused release");
            }
            cna_demo_capture(state->demo, graphics_device);
        }
    }
    return result;
}

/* Releases one owned handle with the matching destroy route and reports a refusal. */
static void release(ContentState* state, CNA_Handle* handle, CNA_Result (*destroy)(CNA_Handle), const char* what)
{
    if (*handle != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, destroy(*handle), what);
        *handle = CNA_INVALID_HANDLE;
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ContentState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.content.content_manager", argc, argv, 10);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.content = state.game_content = state.checker = state.font = state.font_atlas =
        state.effect = state.sprite_batch = CNA_INVALID_HANDLE;

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
    {
        uint64_t root_bytes = UINT64_MAX;
        const CNA_Result result = cna_content_manager_get_root_directory_size(state.game_content, &root_bytes);
        cna_demo_check(&demo, result == CNA_RESULT_SUCCESS && root_bytes == sizeof("assets") - 1U,
                       "the game-owned borrow survives callbacks and destruction of the caller-owned source");
    }

    /* Loaded resources are owned by the caller, not by the manager: each is destroyed by itself --
       the font before the atlas it draws from -- and all of them before the game. */
    release(&state, &state.sprite_batch, cna_sprite_batch_destroy, "cna_sprite_batch_destroy");
    release(&state, &state.font, cna_sprite_font_destroy, "cna_sprite_font_destroy (before its atlas)");
    release(&state, &state.font_atlas, cna_texture2d_destroy, "cna_texture2d_destroy (the font's atlas)");
    release(&state, &state.checker, cna_texture2d_destroy, "cna_texture2d_destroy (the checker)");
    release(&state, &state.effect, cna_effect_destroy, "cna_effect_destroy (the loaded BasicEffect)");
    release(&state, &state.content, cna_content_manager_destroy, "cna_content_manager_destroy");
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    /* A game-owned content borrow does not prevent game destruction; its old identifier expires. */
    if (cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy")) {
        uint64_t untouched = UINT64_C(987654321);
        const CNA_Result result = cna_content_manager_get_root_directory_size(state.game_content, &untouched);
        cna_demo_check(&demo, result == CNA_RESULT_INVALID_HANDLE && untouched == UINT64_C(987654321),
                       "game destruction invalidates its manager borrow without writing a stale query's output");
    }
    return cna_demo_finish(&demo);
}
