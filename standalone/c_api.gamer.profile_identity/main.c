/* SPDX-License-Identifier: MS-PL */
/* Gamer base identity, profile values and unsupported service lookup behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static void on_completed(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    static const char Gamertag[] = "Profile Gamer";
    static const char Display[] = "Display Alias";
    static const char Audience[] = "https://example.invalid/cna-demo";
    CnaDemo demo;
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_GamerHandle lookup = CNA_INVALID_HANDLE;
    CNA_GamerProfileHandle profile = CNA_INVALID_HANDLE;
    CNA_GamerProfileHandle async_profile = CNA_INVALID_HANDLE;
    CNA_GamerProfileInfo info;
    CNA_GamerProfileInfo malformed;
    CNA_Bool disposed = CNA_TRUE;
    CNA_Bool has_picture = CNA_TRUE;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    uint64_t tag = UINT64_C(99);
    int completions = 0;
    char text[128];
    char before[128];

    cna_demo_init(&demo, "c_api.gamer.profile_identity", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(Gamertag, sizeof(Gamertag) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create the offline signed-in gamer")) {
        goto cleanup;
    }

    memset(text, 0xA5, sizeof(text));
    memset(before, 0xA5, sizeof(before));
    cna_demo_check(
        &demo,
        cna_gamer_get_gamertag_size(gamer, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Gamertag) - 1U &&
            cna_gamer_copy_gamertag(gamer, text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(text, before, sizeof(text)) == 0 &&
            cna_gamer_copy_gamertag(gamer, text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Gamertag, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the base Gamer exposes an exact atomic unterminated gamertag");
    cna_demo_check(
        &demo,
        cna_gamer_set_display_name(
            gamer, text_view("Bad \xC3(", sizeof("Bad \xC3(") - 1U)) == CNA_RESULT_ENCODING &&
            cna_gamer_set_display_name(
                gamer, text_view(Display, sizeof(Display) - 1U)) == CNA_RESULT_SUCCESS,
        "display-name mutation rejects malformed UTF-8 before accepting a copied name");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_gamer_get_display_name_size(gamer, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Display) - 1U &&
            cna_gamer_copy_display_name(gamer, text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Display, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the changed display name round-trips exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_gamer_get_text_size(gamer, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Display) - 1U &&
            cna_gamer_copy_text(gamer, text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Display, (size_t)bytes) == 0,
        "a Gamer's text form follows its display name rather than its gamertag");
    cna_demo_check(
        &demo,
        cna_gamer_get_is_disposed(gamer, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_FALSE &&
            cna_gamer_get_tag(gamer, &tag) == CNA_RESULT_SUCCESS && tag == UINT64_C(0) &&
            cna_gamer_set_tag(gamer, UINT64_C(0x1234567890ABCDEF)) == CNA_RESULT_SUCCESS &&
            cna_gamer_get_tag(gamer, &tag) == CNA_RESULT_SUCCESS &&
            tag == UINT64_C(0x1234567890ABCDEF),
        "a live Gamer starts with no tag and preserves a full caller-defined 64-bit tag");

    if (!cna_demo_check_result(
            &demo, cna_gamer_get_profile(gamer, &profile),
            "get an owned profile synchronously")) {
        goto cleanup;
    }
    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    malformed = info;
    malformed.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_gamer_profile_get_info(profile, &info) == CNA_RESULT_SUCCESS &&
            info.gamer_zone <= CNA_GAMER_ZONE_MAXIMUM && info.reputation >= 0.0F &&
            info.reputation <= 5.0F && info.is_disposed == CNA_FALSE &&
            info.reserved[0] == 0U && info.reserved[1] == 0U && info.reserved[2] == 0U &&
            cna_gamer_profile_get_info(profile, &malformed) == CNA_RESULT_INVALID_ARGUMENT,
        "the profile publishes a canonical versioned numeric snapshot and refuses a new version");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_gamer_profile_get_motto_size(profile, &bytes) == CNA_RESULT_SUCCESS &&
            bytes < sizeof(text) &&
            cna_gamer_profile_copy_motto(profile, text, sizeof(text), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes,
        "profile motto size and copy agree for the offline profile");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_gamer_profile_get_region_name_size(profile, &bytes) == CNA_RESULT_SUCCESS &&
            bytes < sizeof(text) &&
            cna_gamer_profile_copy_region_name(profile, text, sizeof(text), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes,
        "profile region size and copy agree for the offline profile");
    cna_demo_check(
        &demo,
        cna_gamer_profile_get_picture_size(profile, &has_picture, &bytes) ==
                CNA_RESULT_SUCCESS &&
            has_picture == CNA_FALSE && bytes == UINT64_C(0) &&
            cna_gamer_profile_copy_picture(profile, NULL, UINT64_C(0), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0),
        "the offline profile reports and copies an ordinary absent gamer picture");

    if (cna_demo_check_result(
            &demo,
            cna_gamer_begin_get_profile(gamer, on_completed, &completions, &async_profile),
            "get a profile through the asynchronous adapter")) {
        cna_demo_check(
            &demo,
            completions == 1 && async_profile != CNA_INVALID_HANDLE &&
                async_profile != profile,
            "profile completion runs once before returning an independently owned snapshot");
        cna_demo_check_result(
            &demo, cna_gamer_profile_destroy(async_profile),
            "release the asynchronous profile independently");
        async_profile = CNA_INVALID_HANDLE;
    }
    cna_demo_check_result(
        &demo, cna_gamer_profile_destroy(profile),
        "release the synchronous profile");
    cna_demo_check(
        &demo,
        cna_gamer_profile_get_motto_size(profile, &bytes) == CNA_RESULT_INVALID_HANDLE,
        "a released profile handle is rejected");
    profile = CNA_INVALID_HANDLE;

    lookup = UINT64_C(99);
    completions = 0;
    cna_demo_check(
        &demo,
        cna_gamer_get_from_gamertag(
            text_view("someone", sizeof("someone") - 1U), &lookup) ==
                CNA_RESULT_NOT_SUPPORTED &&
            lookup == CNA_INVALID_HANDLE &&
            cna_gamer_begin_get_from_gamertag(
                text_view("someone", sizeof("someone") - 1U), on_completed, &completions,
                &lookup) == CNA_RESULT_NOT_SUPPORTED &&
            lookup == CNA_INVALID_HANDLE && completions == 0,
        "gamertag lookup is explicitly unsupported and never completes asynchronously");
    cna_demo_check(
        &demo,
        cna_gamer_get_partner_token_size(
            text_view(Audience, sizeof(Audience) - 1U), &bytes) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_gamer_copy_partner_token(
                text_view(Audience, sizeof(Audience) - 1U), text, sizeof(text), &bytes) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_gamer_begin_get_partner_token(
                text_view(Audience, sizeof(Audience) - 1U), on_completed, &completions,
                text, sizeof(text), &bytes) == CNA_RESULT_NOT_SUPPORTED &&
            completions == 0,
        "partner-token size, copy and asynchronous forms share one explicit refusal");

cleanup:
    if (async_profile != CNA_INVALID_HANDLE) {
        (void)cna_gamer_profile_destroy(async_profile);
    }
    if (profile != CNA_INVALID_HANDLE) {
        (void)cna_gamer_profile_destroy(profile);
    }
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(gamer),
            "release the signed-in gamer after all profile snapshots");
    }
    return cna_demo_finish(&demo);
}
