/* SPDX-License-Identifier: MS-PL */
/* Avatar expression defaults, description bytes, random bodies and events through the C ABI. */
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

static void init_info(CNA_AvatarDescriptionInfo* info)
{
    memset(info, 0xA5, sizeof(*info));
    info->struct_size = (uint32_t)sizeof(*info);
    info->struct_version = UINT32_C(1);
}

static int reserve_is_zero(const CNA_AvatarDescriptionInfo* info)
{
    int index;
    for (index = 0; index < 7; ++index) {
        if (info->reserved[index] != 0U) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    static const char GamerName[] = "Avatar Description Gamer";
    CnaDemo demo;
    CNA_AvatarExpression expression;
    CNA_AvatarDescriptionHandle foreign = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionHandle random = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionHandle female = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionHandle male = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionHandle round_trip = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionHandle gamer_description = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_Handle registration = CNA_INVALID_HANDLE;
    CNA_AvatarDescriptionInfo info;
    CNA_AvatarDescriptionInfo random_info;
    CNA_AvatarDescriptionInfo round_trip_info;
    CNA_AvatarDescriptionInfo malformed;
    unsigned char malformed_before[sizeof(CNA_AvatarDescriptionInfo)];
    uint8_t source[CNA_AVATAR_DESCRIPTION_BYTE_COUNT];
    uint8_t copied[CNA_AVATAR_DESCRIPTION_BYTE_COUNT];
    uint8_t short_copy[CNA_AVATAR_DESCRIPTION_BYTE_COUNT];
    uint8_t short_before[CNA_AVATAR_DESCRIPTION_BYTE_COUNT];
    uint64_t bytes = UINT64_C(0);
    int completions = 0;
    int changes = 0;

    cna_demo_init(&demo, "c_api.gamer.avatar_descriptions", argc, argv, 1);

    memset(&expression, 0xA5, sizeof(expression));
    cna_demo_check(
        &demo,
        cna_avatar_expression_init(&expression) == CNA_RESULT_SUCCESS &&
            expression.struct_size == (uint32_t)sizeof(expression) &&
            expression.struct_version == UINT32_C(1) &&
            expression.mouth == CNA_AVATAR_MOUTH_NEUTRAL &&
            expression.left_eye == CNA_AVATAR_EYE_NEUTRAL &&
            expression.right_eye == CNA_AVATAR_EYE_NEUTRAL &&
            expression.left_eyebrow == CNA_AVATAR_EYEBROW_NEUTRAL &&
            expression.right_eyebrow == CNA_AVATAR_EYEBROW_NEUTRAL,
        "AvatarExpression initializes to a complete neutral face");

    memset(source, 0, sizeof(source));
    source[0] = UINT8_C(1);
    source[1] = UINT8_C(0x42);
    foreign = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_avatar_description_create(source, sizeof(source) - 1U, &foreign) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            foreign == CNA_INVALID_HANDLE,
        "description construction rejects a noncanonical byte count without publishing a handle");
    if (!cna_demo_check_result(
            &demo,
            cna_avatar_description_create(source, sizeof(source), &foreign),
            "create a valid foreign-format description from exactly 1021 bytes")) {
        goto cleanup;
    }
    source[1] = UINT8_C(0x99);
    init_info(&info);
    cna_demo_check(
        &demo,
        cna_avatar_description_get_info(foreign, &info) == CNA_RESULT_SUCCESS &&
            info.struct_size == (uint32_t)sizeof(info) &&
            info.struct_version == UINT32_C(1) &&
            info.body_type == CNA_AVATAR_BODY_TYPE_FEMALE && info.height == 0.0F &&
            info.description_byte_count == CNA_AVATAR_DESCRIPTION_BYTE_COUNT &&
            info.is_valid == CNA_TRUE && reserve_is_zero(&info),
        "a nonzero foreign payload is valid but keeps the documented unmeasured defaults");
    memset(&malformed, 0xA5, sizeof(malformed));
    malformed.struct_size = (uint32_t)sizeof(malformed);
    malformed.struct_version = UINT32_C(2);
    memcpy(malformed_before, &malformed, sizeof(malformed));
    cna_demo_check(
        &demo,
        cna_avatar_description_get_info(foreign, &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(&malformed, malformed_before, sizeof(malformed)) == 0,
        "an unknown description-info version is rejected without changing the output");

    memset(short_copy, 0xA5, sizeof(short_copy));
    memset(short_before, 0xA5, sizeof(short_before));
    cna_demo_check(
        &demo,
        cna_avatar_description_copy_description(
            foreign, short_copy, sizeof(short_copy) - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == CNA_AVATAR_DESCRIPTION_BYTE_COUNT &&
            memcmp(short_copy, short_before, sizeof(short_copy)) == 0,
        "an undersized description copy reports 1021 bytes without a partial write");
    memset(copied, 0, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_avatar_description_copy_description(
            foreign, copied, sizeof(copied), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == CNA_AVATAR_DESCRIPTION_BYTE_COUNT && copied[0] == UINT8_C(1) &&
            copied[1] == UINT8_C(0x42) &&
            memcmp(copied + 2, source + 2, sizeof(copied) - 2U) == 0,
        "description construction copied its input and exact copy-out preserves all bytes");

    if (!cna_demo_check_result(
            &demo, cna_avatar_description_create_random(&random),
            "create an unconstrained random avatar description") ||
        !cna_demo_check_result(
            &demo,
            cna_avatar_description_create_random_for_body_type(
                CNA_AVATAR_BODY_TYPE_FEMALE, &female),
            "create a random female avatar description") ||
        !cna_demo_check_result(
            &demo,
            cna_avatar_description_create_random_for_body_type(
                CNA_AVATAR_BODY_TYPE_MALE, &male),
            "create a random male avatar description")) {
        goto cleanup;
    }
    init_info(&random_info);
    cna_demo_check(
        &demo,
        cna_avatar_description_get_info(random, &random_info) == CNA_RESULT_SUCCESS &&
            random_info.is_valid == CNA_TRUE &&
            random_info.description_byte_count == CNA_AVATAR_DESCRIPTION_BYTE_COUNT &&
            random_info.body_type <= CNA_AVATAR_BODY_TYPE_MAXIMUM &&
            random_info.height >= 1.45F && random_info.height <= 2.05F,
        "the unconstrained random description is a valid measured canonical avatar");
    init_info(&info);
    cna_demo_check(
        &demo,
        cna_avatar_description_get_info(female, &info) == CNA_RESULT_SUCCESS &&
            info.body_type == CNA_AVATAR_BODY_TYPE_FEMALE && info.is_valid == CNA_TRUE &&
            info.height >= 1.45F && info.height <= 2.05F &&
            cna_avatar_description_get_info(male, &info) == CNA_RESULT_SUCCESS &&
            info.body_type == CNA_AVATAR_BODY_TYPE_MALE && info.is_valid == CNA_TRUE &&
            info.height >= 1.45F && info.height <= 2.05F,
        "body-constrained random descriptions honor female and male requests");
    round_trip = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_avatar_description_create_random_for_body_type(
            CNA_AVATAR_BODY_TYPE_MAXIMUM + UINT32_C(1), &round_trip) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            round_trip == CNA_INVALID_HANDLE,
        "an undefined avatar body type is rejected without publishing a handle");

    cna_demo_check_result(
        &demo,
        cna_avatar_description_copy_description(
            random, copied, sizeof(copied), &bytes),
        "copy the complete encoded random avatar");
    if (cna_demo_check_result(
            &demo,
            cna_avatar_description_create(copied, bytes, &round_trip),
            "recreate the random avatar from its public bytes")) {
        init_info(&round_trip_info);
        cna_demo_check(
            &demo,
            cna_avatar_description_get_info(round_trip, &round_trip_info) ==
                    CNA_RESULT_SUCCESS &&
                round_trip_info.body_type == random_info.body_type &&
                round_trip_info.height == random_info.height &&
                round_trip_info.description_byte_count == random_info.description_byte_count &&
                round_trip_info.is_valid == random_info.is_valid,
            "encoded bytes retain the random avatar's body type, height and validity");
    }

    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(GamerName, sizeof(GamerName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &gamer),
            "create an offline gamer for avatar lookup")) {
        goto cleanup;
    }
    completions = 0;
    if (cna_demo_check_result(
            &demo,
            cna_avatar_description_get_from_gamer(
                gamer, on_completed, &completions, &gamer_description),
            "complete the offline gamer's avatar read")) {
        init_info(&info);
        cna_demo_check(
            &demo,
            completions == 1 &&
                cna_avatar_description_get_info(gamer_description, &info) ==
                    CNA_RESULT_SUCCESS &&
                info.is_valid == CNA_FALSE && info.height == 0.0F &&
                info.description_byte_count == CNA_AVATAR_DESCRIPTION_BYTE_COUNT,
            "the local read calls back once and returns an ordinary invalid default description");
    }
    if (cna_demo_check_result(
            &demo,
            cna_avatar_description_subscribe_changed_ext(
                gamer_description, on_completed, &changes, &registration),
            "own a change registration for that specific description")) {
        cna_demo_check(
            &demo, changes == 0,
            "the isolated local workflow does not invent an avatar-change event");
        cna_demo_check_result(
            &demo, cna_gamer_unsubscribe_ext(registration),
            "release the description change registration once");
        cna_demo_check(
            &demo,
            cna_gamer_unsubscribe_ext(registration) == CNA_RESULT_INVALID_HANDLE && changes == 0,
            "a released change registration rejects repeated unsubscribe and stays silent");
        registration = CNA_INVALID_HANDLE;
    }

    cna_demo_check_result(
        &demo, cna_avatar_description_destroy(foreign),
        "release the foreign description once");
    cna_demo_check(
        &demo,
        cna_avatar_description_destroy(foreign) == CNA_RESULT_INVALID_HANDLE,
        "a released description rejects repeated destruction");
    foreign = CNA_INVALID_HANDLE;

cleanup:
    if (registration != CNA_INVALID_HANDLE) {
        (void)cna_gamer_unsubscribe_ext(registration);
    }
    if (gamer_description != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(gamer_description);
    }
    if (round_trip != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(round_trip);
    }
    if (male != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(male);
    }
    if (female != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(female);
    }
    if (random != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(random);
    }
    if (foreign != CNA_INVALID_HANDLE) {
        (void)cna_avatar_description_destroy(foreign);
    }
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(gamer),
            "release the offline avatar gamer");
    }
    return cna_demo_finish(&demo);
}
