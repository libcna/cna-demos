/* SPDX-License-Identifier: MS-PL */
/* Compile-time platform identity and process assembly-title ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* text, uint64_t bytes)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = bytes;
    return result;
}

static int platform_name_matches(
    CNA_Platform platform,
    CNA_DesktopOS desktop_os,
    const char* name,
    uint64_t bytes)
{
    const char* expected = 0;
    if (platform == CNA_PLATFORM_ANDROID) {
        expected = "Android";
    } else if (platform == CNA_PLATFORM_IOS) {
        expected = "iOS";
    } else if (platform == CNA_PLATFORM_WEB) {
        expected = "Web";
    } else if (desktop_os == CNA_DESKTOP_OS_WINDOWS) {
        expected = "Windows";
    } else if (desktop_os == CNA_DESKTOP_OS_LINUX) {
        expected = "Linux";
    } else if (desktop_os == CNA_DESKTOP_OS_MACOSX) {
        expected = "macOS";
    } else {
        expected = "Desktop";
    }
    return bytes == (uint64_t)strlen(expected) && memcmp(name, expected, (size_t)bytes) == 0;
}

int main(int argc, char** argv)
{
    static const char AuthoredTitle[] = "C API host identity";
    static const char EmbeddedNul[] = {'b', 'a', 'd', '\0', 'x'};
    CnaDemo demo;
    CNA_Platform platform = UINT32_MAX;
    CNA_DesktopOS desktop_os = UINT32_MAX;
    CNA_Bool apple = UINT8_C(77);
    CNA_Bool mobile = UINT8_C(77);
    CNA_Result desktop_result;
    CNA_Result prior_size_result;
    uint64_t platform_bytes = UINT64_MAX;
    uint64_t copied_bytes = UINT64_MAX;
    uint64_t prior_bytes = UINT64_MAX;
    uint64_t title_bytes = UINT64_MAX;
    char platform_name[32];
    char short_name[32];
    char before_name[32];
    char title[64];
    char before_title[64];
    char* prior_title = 0;

    cna_demo_init(&demo, "c_api.platform.host_identity", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_PLATFORM_DESKTOP == 0U && CNA_PLATFORM_ANDROID == 1U && CNA_PLATFORM_IOS == 2U &&
            CNA_PLATFORM_WEB == 3U && CNA_DESKTOP_OS_WINDOWS == 0U &&
            CNA_DESKTOP_OS_OTHER == 3U,
        "platform and desktop operating-system identities retain fixed ordinals");

    cna_demo_check(
        &demo,
        cna_platform_get_current(&platform) == CNA_RESULT_SUCCESS &&
            platform <= CNA_PLATFORM_WEB &&
            cna_platform_get_is_apple_ext(&apple) == CNA_RESULT_SUCCESS &&
            cna_platform_get_is_mobile_ext(&mobile) == CNA_RESULT_SUCCESS &&
            (apple == CNA_FALSE || apple == CNA_TRUE) &&
            mobile == ((platform == CNA_PLATFORM_ANDROID || platform == CNA_PLATFORM_IOS)
                           ? CNA_TRUE
                           : CNA_FALSE),
        "target, Apple and mobile compile-time facts agree");

    desktop_result = cna_desktop_os_get_current(&desktop_os);
    cna_demo_check(
        &demo,
        (platform == CNA_PLATFORM_DESKTOP && desktop_result == CNA_RESULT_SUCCESS &&
         desktop_os <= CNA_DESKTOP_OS_OTHER &&
         apple == (desktop_os == CNA_DESKTOP_OS_MACOSX ? CNA_TRUE : CNA_FALSE)) ||
            (platform != CNA_PLATFORM_DESKTOP && desktop_result == CNA_RESULT_INVALID_STATE &&
             desktop_os == UINT32_MAX &&
             apple == (platform == CNA_PLATFORM_IOS ? CNA_TRUE : CNA_FALSE)),
        "the desktop operating-system query is available only on desktop targets");

    memset(platform_name, 0xA5, sizeof(platform_name));
    cna_demo_check(
        &demo,
        cna_platform_get_current_name_size_ext(&platform_bytes) == CNA_RESULT_SUCCESS &&
            platform_bytes > 0U && platform_bytes < sizeof(platform_name) &&
            cna_platform_copy_current_name_ext(
                platform_name, sizeof(platform_name), &copied_bytes) == CNA_RESULT_SUCCESS &&
            copied_bytes == platform_bytes &&
            (unsigned char)platform_name[platform_bytes] == 0xA5U &&
            platform_name_matches(platform, desktop_os, platform_name, platform_bytes),
        "the exact unterminated platform name matches its target identity");

    memset(short_name, 0xA5, sizeof(short_name));
    memcpy(before_name, short_name, sizeof(short_name));
    cna_demo_check(
        &demo,
        cna_platform_copy_current_name_ext(
            short_name, platform_bytes - 1U, &copied_bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied_bytes == platform_bytes &&
            memcmp(short_name, before_name, sizeof(short_name)) == 0,
        "a short platform-name destination is refused without a partial write");

    prior_size_result = cna_assembly_copy_title_ext(0, 0U, &prior_bytes);
    prior_title = (char*)malloc((size_t)(prior_bytes == 0U ? 1U : prior_bytes));
    cna_demo_check(
        &demo,
        ((prior_bytes == 0U && prior_size_result == CNA_RESULT_SUCCESS) ||
         (prior_bytes > 0U && prior_size_result == CNA_RESULT_BUFFER_TOO_SMALL)) &&
            prior_title != 0 &&
            cna_assembly_copy_title_ext(prior_title, prior_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS && copied_bytes == prior_bytes,
        "the prior process title is sized and copied for later restoration");

    memset(title, 0xA5, sizeof(title));
    cna_demo_check(
        &demo,
        cna_assembly_set_title_ext(
            view(AuthoredTitle, (uint64_t)sizeof(AuthoredTitle) - 1U)) == CNA_RESULT_SUCCESS &&
            cna_assembly_copy_title_ext(0, 0U, &title_bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            title_bytes == sizeof(AuthoredTitle) - 1U &&
            cna_assembly_copy_title_ext(title, title_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS && copied_bytes == title_bytes &&
            memcmp(title, AuthoredTitle, sizeof(AuthoredTitle) - 1U) == 0 &&
            (unsigned char)title[title_bytes] == 0xA5U,
        "an authored product title round-trips as exact unterminated bytes");

    memset(title, 0xA5, sizeof(title));
    memcpy(before_title, title, sizeof(title));
    cna_demo_check(
        &demo,
        cna_assembly_copy_title_ext(title, title_bytes - 1U, &copied_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied_bytes == title_bytes &&
            memcmp(title, before_title, sizeof(title)) == 0,
        "a short product-title destination is refused without a partial write");
    cna_demo_check(
        &demo,
        cna_assembly_set_title_ext(view(EmbeddedNul, sizeof(EmbeddedNul))) ==
                CNA_RESULT_ENCODING &&
            cna_assembly_copy_title_ext(title, title_bytes, &copied_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(title, AuthoredTitle, sizeof(AuthoredTitle) - 1U) == 0,
        "an embedded-NUL title is refused as invalid text without changing the title");

    cna_demo_check(
        &demo,
        cna_assembly_set_title_ext(view(prior_title, prior_bytes)) == CNA_RESULT_SUCCESS &&
            cna_assembly_copy_title_ext(0, 0U, &copied_bytes) == prior_size_result &&
            copied_bytes == prior_bytes,
        "the previous process title is restored exactly");

    free(prior_title);
    return cna_demo_finish(&demo);
}
