/* SPDX-License-Identifier: MS-PL */
/* Headless Guide keyboard and message-box request state through the C ABI. */
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
    static const char Title[] = "Account password";
    static const char Description[] = "Enter the local secret";
    static const char Password[] = "secret";
    static const char Masked[] = "******";
    static const char BoxTitle[] = "Choose a slot";
    static const char BoxText[] = "Which save slot should be used?";
    static const char FirstButton[] = "One";
    static const char SecondButton[] = "Two";
    static const char ThirdButton[] = "Three";
    CnaDemo demo;
    CNA_StringView buttons[3];
    CNA_Bool flag = CNA_TRUE;
    CNA_Bool has_choice = CNA_FALSE;
    int32_t button = -1;
    int32_t focus = -1;
    int completions = 0;
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    char text[64];
    char before[64];

    cna_demo_init(&demo, "c_api.gamer.guide_dialogs", argc, argv, 1);
    cna_demo_check(
        &demo,
        cna_guide_reset_pending_keyboard_input_ext() == CNA_RESULT_SUCCESS &&
            cna_guide_reset_pending_message_box_ext() == CNA_RESULT_SUCCESS &&
            cna_guide_get_has_pending_keyboard_input_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "both dialog slots reset idempotently and begin empty");

    cna_demo_check(
        &demo,
        cna_guide_begin_show_keyboard_input(
            CNA_PLAYER_INDEX_ONE,
            text_view(Title, sizeof(Title) - 1U),
            text_view(Description, sizeof(Description) - 1U),
            text_view(Password, sizeof(Password) - 1U), UINT8_C(9), on_completed,
            &completions) == CNA_RESULT_INVALID_ARGUMENT &&
            completions == 0 &&
            cna_guide_get_has_pending_keyboard_input_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "a noncanonical password flag is refused without starting input");
    if (!cna_demo_check_result(
            &demo,
            cna_guide_begin_show_keyboard_input(
                CNA_PLAYER_INDEX_TWO,
                text_view(Title, sizeof(Title) - 1U),
                text_view(Description, sizeof(Description) - 1U),
                text_view(Password, sizeof(Password) - 1U), CNA_TRUE, on_completed,
                &completions),
            "begin a password input for player two")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        completions == 0 &&
            cna_guide_get_has_pending_keyboard_input_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "keyboard input remains genuinely pending without invoking completion");
    cna_demo_check(
        &demo,
        cna_guide_begin_show_keyboard_input(
            CNA_PLAYER_INDEX_ONE,
            text_view("second", sizeof("second") - 1U),
            text_view("second", sizeof("second") - 1U),
            text_view("", UINT64_C(0)), CNA_FALSE, NULL, NULL) ==
                CNA_RESULT_INVALID_STATE,
        "a second keyboard request is refused while the first remains pending");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_guide_get_pending_keyboard_input_title_size_ext(&bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(Title) - 1U &&
            cna_guide_copy_pending_keyboard_input_title_ext(
                text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Title, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "pending title size and exact unterminated copy agree");
    memset(text, 0xA5, sizeof(text));
    memset(before, 0xA5, sizeof(before));
    cna_demo_check(
        &demo,
        cna_guide_copy_pending_keyboard_input_title_ext(
            text, sizeof(Title) - 2U, &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == sizeof(Title) - 1U && memcmp(text, before, sizeof(text)) == 0,
        "a short pending-title copy is atomic and still reports its size");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_guide_get_pending_keyboard_input_description_size_ext(&bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(Description) - 1U &&
            cna_guide_copy_pending_keyboard_input_description_ext(
                text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Description, (size_t)bytes) == 0,
        "pending description size and copy preserve the authored UTF-8 text");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_guide_get_pending_keyboard_input_display_text_size_ext(&bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(Masked) - 1U &&
            cna_guide_copy_pending_keyboard_input_display_text_ext(
                text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, Masked, (size_t)bytes) == 0 &&
            memcmp(text, Password, (size_t)bytes) != 0,
        "password-mode display text masks every default-text code unit");
    cna_demo_check(
        &demo,
        cna_guide_was_keyboard_input_canceled_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "a pending keyboard input is not yet canceled");
    cna_demo_check(
        &demo,
        cna_guide_simulate_keyboard_input_cancel_ext() == CNA_RESULT_SUCCESS &&
            completions == 1 &&
            cna_guide_get_has_pending_keyboard_input_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_guide_was_keyboard_input_canceled_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "simulated cancellation completes once, clears pending state and records cancellation");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_guide_end_show_keyboard_input_size(&bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_guide_end_show_keyboard_input(NULL, UINT64_C(0), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(0),
        "a canceled keyboard input repeatedly yields an exact empty answer");
    cna_demo_check(
        &demo,
        cna_guide_reset_pending_keyboard_input_ext() == CNA_RESULT_SUCCESS &&
            cna_guide_end_show_keyboard_input_size(&bytes) == CNA_RESULT_INVALID_STATE,
        "reset releases the completed C operation and removes its cached answer");

    completions = 0;
    cna_demo_check(
        &demo,
        cna_guide_begin_show_keyboard_input(
            CNA_PLAYER_INDEX_ONE,
            text_view("Discard", sizeof("Discard") - 1U),
            text_view("No completion", sizeof("No completion") - 1U),
            text_view("draft", sizeof("draft") - 1U), CNA_FALSE, on_completed,
            &completions) == CNA_RESULT_SUCCESS &&
            cna_guide_reset_pending_keyboard_input_ext() == CNA_RESULT_SUCCESS &&
            completions == 0 &&
            cna_guide_get_has_pending_keyboard_input_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "reset discards active keyboard input without reporting a completion");

    buttons[0] = text_view(FirstButton, sizeof(FirstButton) - 1U);
    buttons[1] = text_view(SecondButton, sizeof(SecondButton) - 1U);
    buttons[2] = text_view(ThirdButton, sizeof(ThirdButton) - 1U);
    completions = 0;
    cna_demo_check(
        &demo,
        cna_guide_begin_show_message_box(
            CNA_PLAYER_INDEX_ONE,
            text_view(BoxTitle, sizeof(BoxTitle) - 1U),
            text_view(BoxText, sizeof(BoxText) - 1U), buttons, UINT64_C(3), 3,
            CNA_MESSAGE_BOX_ICON_ALERT, on_completed, &completions) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            completions == 0 &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "an out-of-range initial button is refused without opening a message box");
    if (!cna_demo_check_result(
            &demo,
            cna_guide_begin_show_message_box(
                CNA_PLAYER_INDEX_THREE,
                text_view(BoxTitle, sizeof(BoxTitle) - 1U),
                text_view(BoxText, sizeof(BoxText) - 1U), buttons, UINT64_C(3), 1,
                CNA_MESSAGE_BOX_ICON_ALERT, on_completed, &completions),
            "begin a three-button message box for player three")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        completions == 0 &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_guide_get_pending_message_box_focus_button_ext(&focus) ==
                CNA_RESULT_SUCCESS &&
            focus == 1,
        "the message box remains pending with its requested middle button focused");
    cna_demo_check(
        &demo,
        cna_guide_simulate_message_box_click_ext(3) == CNA_RESULT_INVALID_ARGUMENT &&
            completions == 0 &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "an invalid simulated click preserves the pending box without completion");
    cna_demo_check(
        &demo,
        cna_guide_simulate_message_box_click_ext(2) == CNA_RESULT_SUCCESS &&
            completions == 1 &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "a valid simulated click completes once and closes the pending box");
    button = -7;
    cna_demo_check(
        &demo,
        cna_guide_end_show_message_box(&has_choice, &button) == CNA_RESULT_SUCCESS &&
            has_choice == CNA_TRUE && button == 2,
        "End reads the exact chosen button");
    button = -7;
    has_choice = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_guide_end_show_message_box(&has_choice, &button) == CNA_RESULT_SUCCESS &&
            has_choice == CNA_TRUE && button == 2,
        "the completed message-box answer remains repeatable until reset");
    cna_demo_check(
        &demo,
        cna_guide_reset_pending_message_box_ext() == CNA_RESULT_SUCCESS &&
            cna_guide_end_show_message_box(&has_choice, &button) == CNA_RESULT_INVALID_STATE,
        "message-box reset releases the completed C operation and its cached choice");

    completions = 0;
    cna_demo_check(
        &demo,
        cna_guide_begin_show_message_box(
            CNA_PLAYER_INDEX_ONE,
            text_view("Discard", sizeof("Discard") - 1U),
            text_view("No callback", sizeof("No callback") - 1U), buttons,
            UINT64_C(1), 0, CNA_MESSAGE_BOX_ICON_NONE, on_completed, &completions) ==
                CNA_RESULT_SUCCESS &&
            cna_guide_reset_pending_message_box_ext() == CNA_RESULT_SUCCESS &&
            completions == 0 &&
            cna_guide_get_has_pending_message_box_ext(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "reset discards an active message box without choosing or completing it");

cleanup:
    (void)cna_guide_reset_pending_keyboard_input_ext();
    (void)cna_guide_reset_pending_message_box_ext();
    return cna_demo_finish(&demo);
}
