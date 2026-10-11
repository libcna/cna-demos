/* SPDX-License-Identifier: MS-PL */
/*
 * Save games from C: select a storage device, open a container, write a save file as bytes, read
 * it back, and delete it. XNA's BeginShowSelector/BeginOpenContainer pairs are one synchronous call
 * each in the C ABI, with the completion callback invoked before the call returns. No game object
 * is needed.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* s)
{
    CNA_StringView v;
    v.data = s;
    v.byte_length = (uint64_t)strlen(s);
    return v;
}

/* The save: level and score as little-endian 32-bit integers, then a 16-byte name. */
#define SAVE_SIZE 24

static void put_u32(uint8_t* out, uint32_t value)
{
    out[0] = (uint8_t)(value & 0xFFU);
    out[1] = (uint8_t)((value >> 8) & 0xFFU);
    out[2] = (uint8_t)((value >> 16) & 0xFFU);
    out[3] = (uint8_t)((value >> 24) & 0xFFU);
}

static uint32_t get_u32(const uint8_t* in)
{
    return (uint32_t)in[0] | ((uint32_t)in[1] << 8) | ((uint32_t)in[2] << 16) | ((uint32_t)in[3] << 24);
}

static void completed(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    uint8_t save[SAVE_SIZE];
    uint8_t loaded[SAVE_SIZE];
    uint64_t bytes_read = 0U;
    int64_t length = 0;
    uint64_t files = 0U;
    CNA_Bool connected = CNA_FALSE;
    CNA_Bool exists = CNA_FALSE;
    int completions = 0;
    char root[512];
    uint64_t root_bytes = 0U;

    cna_demo_init(&demo, "c_api.storage.save_game", argc, argv, 0);
    /* Keep the demo's files apart from other programs' (CNA extension: the storage root is
       derived from the application name). */
    cna_demo_check_result(&demo, cna_storage_set_app_name_ext(view("cna-demos")), "cna_storage_set_app_name_ext");
    if (cna_storage_copy_root_ext(root, sizeof(root) - 1U, &root_bytes) == CNA_RESULT_SUCCESS) {
        root[root_bytes] = '\0';
        printf("  storage root: %s\n", root);
    }

    /* XNA's device selector: on a PC there is one device and no user interface. */
    if (!cna_demo_check_result(&demo, cna_storage_device_show_selector(completed, &completions, &device),
                               "cna_storage_device_show_selector")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check(&demo, completions == 1, "the completion callback ran before the call returned");
    cna_demo_check_result(&demo, cna_storage_device_get_is_connected(device, &connected),
                          "cna_storage_device_get_is_connected");
    cna_demo_check(&demo, connected == CNA_TRUE, "the device is connected");

    /* A container per game (its display name is the folder); files inside it. */
    if (cna_demo_check_result(&demo, cna_storage_container_open(device, view("C API Saves"), completed, &completions,
                                                                &container),
                              "cna_storage_container_open")) {
        memset(save, 0, sizeof(save));
        put_u32(save, 7U);         /* level */
        put_u32(save + 4, 12345U); /* score */
        memcpy(save + 8, "Demo Player", 11);

        if (cna_demo_check_result(&demo, cna_storage_container_create_file(container, view("slot1.sav"), &stream),
                                  "cna_storage_container_create_file")) {
            cna_demo_check_result(&demo, cna_storage_stream_write(stream, save, sizeof(save)),
                                  "cna_storage_stream_write");
            cna_demo_check_result(&demo, cna_storage_stream_close(stream), "cna_storage_stream_close");
        }

        cna_demo_check_result(&demo, cna_storage_container_file_exists(container, view("slot1.sav"), &exists),
                              "cna_storage_container_file_exists");
        cna_demo_check(&demo, exists == CNA_TRUE, "slot1.sav exists");
        cna_demo_check_result(&demo, cna_storage_container_get_file_name_count(container, view("*.sav"), &files),
                              "cna_storage_container_get_file_name_count");
        cna_demo_check(&demo, files == 1U, "one *.sav file in the container");

        /* Load it again, as the next session would. */
        if (cna_demo_check_result(&demo, cna_storage_container_open_file(container, view("slot1.sav"),
                                                                         CNA_FILE_MODE_OPEN, &stream),
                                  "cna_storage_container_open_file")) {
            cna_demo_check_result(&demo, cna_storage_stream_get_length(stream, &length),
                                  "cna_storage_stream_get_length");
            memset(loaded, 0, sizeof(loaded));
            cna_demo_check_result(&demo, cna_storage_stream_read(stream, loaded, sizeof(loaded), &bytes_read),
                                  "cna_storage_stream_read");
            cna_demo_check_result(&demo, cna_storage_stream_close(stream), "cna_storage_stream_close");
            printf("  loaded %llu of %lld bytes: level %u, score %u, name %.16s\n",
                   (unsigned long long)bytes_read, (long long)length, get_u32(loaded), get_u32(loaded + 4),
                   (const char*)(loaded + 8));
            cna_demo_check(&demo,
                           length == SAVE_SIZE && bytes_read == SAVE_SIZE && memcmp(save, loaded, SAVE_SIZE) == 0,
                           "the save reads back byte for byte");
        }

        /* "Delete save". */
        cna_demo_check_result(&demo, cna_storage_container_delete_file(container, view("slot1.sav")),
                              "cna_storage_container_delete_file");
        cna_demo_check_result(&demo, cna_storage_container_file_exists(container, view("slot1.sav"), &exists),
                              "cna_storage_container_file_exists");
        cna_demo_check(&demo, exists == CNA_FALSE, "after DeleteFile it is gone");
        cna_demo_check_result(&demo, cna_storage_container_destroy(container), "cna_storage_container_destroy");
    }
    cna_demo_check_result(&demo, cna_storage_device_destroy(device), "cna_storage_device_destroy");
    return cna_demo_finish(&demo);
}
