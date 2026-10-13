/* SPDX-License-Identifier: MS-PL */
/* Storage selectors, capacity, directory/file listings, resizing and event ownership in C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static const char ContainerName[] = "C API Member Audit";

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static CNA_StringView all_names(void)
{
    CNA_StringView result;
    result.data = NULL;
    result.byte_length = 0U;
    return result;
}

static void count_completion(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    static const uint8_t Payload[6] = {10U, 20U, 30U, 40U, 50U, 60U};
    CnaDemo demo;
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageDeviceHandle temporary = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    CNA_Handle device_changed = CNA_INVALID_HANDLE;
    CNA_Handle disposing = CNA_INVALID_HANDLE;
    CNA_Bool flag = UINT8_C(99);
    CNA_StorageDeviceHandle owner = CNA_INVALID_HANDLE;
    int completions = 0;
    int changes = 0;
    int disposals = 0;
    int64_t free_space = -1;
    int64_t total_space = -1;
    int64_t length = -1;
    uint64_t bytes = 0U;
    uint64_t copied = 0U;
    uint64_t count = 0U;
    char text[1024];
    char before[1024];
    uint8_t readback[8];
    int saw_a = 0;
    int saw_b = 0;
    uint64_t i;

    cna_demo_init(&demo, "c_api.storage.members", argc, argv, 0);
    cna_demo_check_result(
        &demo, cna_storage_set_app_name_ext(view("cna-demos-storage-members")),
        "select an isolated application storage root");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_storage_get_root_size_ext(&bytes) == CNA_RESULT_SUCCESS && bytes > 0U &&
            bytes < sizeof(text) &&
            cna_storage_copy_root_ext(text, bytes, &copied) == CNA_RESULT_SUCCESS &&
            copied == bytes && (unsigned char)text[bytes] == 0xA5U,
        "storage root size and copy expose exact unterminated bytes");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_storage_copy_root_ext(text, bytes - 1U, &copied) == CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(text, before, sizeof(text)) == 0,
        "a short storage-root destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_storage_device_show_selector_for_player(
            CNA_PLAYER_INDEX_TWO, count_completion, &completions, &temporary) == CNA_RESULT_SUCCESS &&
            completions == 1 && cna_storage_device_destroy(temporary) == CNA_RESULT_SUCCESS,
        "the player selector completes synchronously and returns an owned device");
    temporary = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_storage_device_show_selector_with_space(
            1, 1, count_completion, &completions, &temporary) == CNA_RESULT_SUCCESS &&
            completions == 2 && cna_storage_device_destroy(temporary) == CNA_RESULT_SUCCESS,
        "the capacity selector accepts nonnegative byte and directory requirements");
    temporary = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_storage_device_show_selector_for_player_with_space(
            CNA_PLAYER_INDEX_ONE, 0, 0, count_completion, &completions, &temporary) ==
                CNA_RESULT_SUCCESS && completions == 3 &&
            cna_storage_device_destroy(temporary) == CNA_RESULT_SUCCESS &&
            cna_storage_device_show_selector_with_space(
                -1, 0, NULL, NULL, &temporary) == CNA_RESULT_INVALID_ARGUMENT &&
            temporary == CNA_INVALID_HANDLE,
        "the combined selector completes synchronously and negative capacity is atomic");

    if (!cna_demo_check_result(
            &demo, cna_storage_device_show_selector(NULL, NULL, &device),
            "select the device used by the member workflow")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check(
        &demo,
        cna_storage_device_get_free_space(device, &free_space) == CNA_RESULT_SUCCESS &&
            cna_storage_device_get_total_space(device, &total_space) == CNA_RESULT_SUCCESS &&
            free_space > 0 && total_space >= free_space,
        "device capacity reports positive free space within total space");
    cna_demo_check(
        &demo,
        cna_storage_device_subscribe_device_changed(
            count_completion, &changes, &device_changed) == CNA_RESULT_SUCCESS &&
            device_changed != CNA_INVALID_HANDLE && changes == 0 &&
            cna_storage_device_unsubscribe_device_changed(device_changed) == CNA_RESULT_SUCCESS &&
            cna_storage_device_unsubscribe_device_changed(device_changed) == CNA_RESULT_INVALID_HANDLE,
        "the process device-change registration is owned and released exactly once");
    device_changed = CNA_INVALID_HANDLE;
    cna_demo_check_result(
        &demo, cna_storage_device_delete_container(device, view(ContainerName)),
        "remove any prior member-audit container");
    if (!cna_demo_check_result(
            &demo,
            cna_storage_container_open(
                device, view(ContainerName), count_completion, &completions, &container),
            "open a fresh member-audit container")) {
        goto cleanup;
    }
    cna_demo_check(&demo, completions == 4, "container opening invokes its completion before return");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_storage_container_get_display_name_size(container, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen(ContainerName) &&
            cna_storage_container_copy_display_name(container, text, bytes, &copied) ==
                CNA_RESULT_SUCCESS && copied == bytes &&
            memcmp(text, ContainerName, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the container display name uses exact unterminated size/copy semantics");
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_storage_container_copy_display_name(container, text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL && copied == bytes &&
            memcmp(text, before, sizeof(text)) == 0,
        "a short display-name copy is atomic");
    cna_demo_check(
        &demo,
        cna_storage_container_get_is_disposed(container, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_storage_container_get_storage_device(container, &owner) == CNA_RESULT_SUCCESS &&
            owner == device,
        "a live container reports its undisposed state and owning device handle");

    cna_demo_check(
        &demo,
        cna_storage_container_directory_exists(container, view("slot_a"), &flag) ==
                CNA_RESULT_SUCCESS && flag == CNA_FALSE &&
            cna_storage_container_create_directory(container, view("slot_a")) == CNA_RESULT_SUCCESS &&
            cna_storage_container_create_directory(container, view("slot_b")) == CNA_RESULT_SUCCESS &&
            cna_storage_container_directory_exists(container, view("slot_a"), &flag) ==
                CNA_RESULT_SUCCESS && flag == CNA_TRUE,
        "directory creation changes the corresponding existence query");
    cna_demo_check(
        &demo,
        cna_storage_container_get_directory_name_count(container, view("slot_*"), &count) ==
                CNA_RESULT_SUCCESS && count == 2U,
        "the directory glob counts both authored slot directories");
    for (i = 0U; i < count; ++i) {
        memset(text, 0, sizeof(text));
        if (cna_storage_container_copy_directory_name(
                container, view("slot_*"), i, text, sizeof(text) - 1U, &bytes) == CNA_RESULT_SUCCESS) {
            saw_a |= bytes == 6U && memcmp(text, "slot_a", 6U) == 0;
            saw_b |= bytes == 6U && memcmp(text, "slot_b", 6U) == 0;
        }
    }
    cna_demo_check(&demo, saw_a && saw_b, "directory copies expose both names without assuming native order");
    cna_demo_check(
        &demo,
        cna_storage_container_delete_directory(container, view("slot_a")) == CNA_RESULT_SUCCESS &&
            cna_storage_container_delete_directory(container, view("slot_b")) == CNA_RESULT_SUCCESS &&
            cna_storage_container_get_directory_name_count(container, all_names(), &count) ==
                CNA_RESULT_SUCCESS && count == 0U,
        "empty directories delete cleanly and disappear from the root listing");

    if (cna_demo_check_result(
            &demo, cna_storage_container_create_file(container, view("alpha.dat"), &stream),
            "create the resize fixture file")) {
        cna_demo_check(
            &demo,
            cna_storage_stream_write(stream, Payload, sizeof(Payload)) == CNA_RESULT_SUCCESS &&
                cna_storage_stream_flush(stream) == CNA_RESULT_SUCCESS &&
                cna_storage_stream_close(stream) == CNA_RESULT_SUCCESS,
            "write and explicitly flush the six-byte fixture");
        stream = CNA_INVALID_HANDLE;
    }
    if (cna_demo_check_result(
            &demo, cna_storage_container_create_file(container, view("beta.bin"), &stream),
            "create a second file for filtered listing")) {
        cna_demo_check_result(&demo, cna_storage_stream_close(stream), "close the second file");
        stream = CNA_INVALID_HANDLE;
    }
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_storage_container_get_file_name_count(container, view("*.dat"), &count) ==
                CNA_RESULT_SUCCESS && count == 1U &&
            cna_storage_container_copy_file_name(
                container, view("*.dat"), 0U, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == strlen("alpha.dat") && memcmp(text, "alpha.dat", (size_t)bytes) == 0,
        "filtered file-name copying returns only the matching fixture");

    if (cna_demo_check_result(
            &demo,
            cna_storage_container_open_file_share(
                container, view("alpha.dat"), CNA_FILE_MODE_OPEN, CNA_FILE_ACCESS_READ_WRITE,
                CNA_FILE_SHARE_READ | CNA_FILE_SHARE_DELETE, &stream),
            "open the fixture with explicit access and share bits")) {
        cna_demo_check(
            &demo,
            cna_storage_stream_set_length(stream, 4) == CNA_RESULT_SUCCESS &&
                cna_storage_stream_get_length(stream, &length) == CNA_RESULT_SUCCESS && length == 4 &&
                cna_storage_stream_set_length(stream, 8) == CNA_RESULT_SUCCESS &&
                cna_storage_stream_get_length(stream, &length) == CNA_RESULT_SUCCESS && length == 8 &&
                cna_storage_stream_set_length(stream, -1) == CNA_RESULT_INVALID_ARGUMENT,
            "stream length truncates, extends and atomically rejects a negative value");
        memset(readback, 0xA5, sizeof(readback));
        cna_demo_check(
            &demo,
            cna_storage_stream_read(stream, readback, sizeof(readback), &copied) == CNA_RESULT_SUCCESS &&
                copied == sizeof(readback) && memcmp(readback, Payload, 4U) == 0 &&
                readback[4] == 0U && readback[5] == 0U && readback[6] == 0U && readback[7] == 0U,
            "extension preserves the truncated prefix and zero-fills the new bytes");
        cna_demo_check_result(&demo, cna_storage_stream_close(stream), "close the resized stream");
        stream = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        cna_storage_container_delete_file(container, view("alpha.dat")) == CNA_RESULT_SUCCESS &&
            cna_storage_container_delete_file(container, view("beta.bin")) == CNA_RESULT_SUCCESS,
        "delete both member-audit files");

    cna_demo_check_result(
        &demo,
        cna_storage_container_subscribe_disposing(
            container, count_completion, &disposals, &disposing),
        "subscribe to the container disposing event");
    cna_demo_check(
        &demo,
        cna_storage_container_dispose(container) == CNA_RESULT_SUCCESS && disposals == 1 &&
            cna_storage_container_get_is_disposed(container, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && cna_storage_container_dispose(container) == CNA_RESULT_SUCCESS &&
            disposals == 1,
        "container disposal is idempotent and raises its event exactly once");
    cna_demo_check(
        &demo,
        cna_storage_container_unsubscribe_disposing(disposing) == CNA_RESULT_SUCCESS &&
            cna_storage_container_unsubscribe_disposing(disposing) == CNA_RESULT_INVALID_HANDLE,
        "the disposing registration is released exactly once");
    disposing = CNA_INVALID_HANDLE;

cleanup:
    if (stream != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_stream_close(stream), "close a remaining stream");
    }
    if (disposing != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_container_unsubscribe_disposing(disposing),
                              "release a remaining disposing registration");
    }
    if (container != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_container_destroy(container),
                              "destroy the disposed container handle");
        container = CNA_INVALID_HANDLE;
    }
    if (device != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_storage_device_delete_container(device, view(ContainerName)),
            "delete the member-audit container tree");
        cna_demo_check_result(&demo, cna_storage_device_destroy(device),
                              "destroy the selected storage device");
    }
    return cna_demo_finish(&demo);
}
