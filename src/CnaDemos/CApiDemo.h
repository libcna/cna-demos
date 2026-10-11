/* SPDX-License-Identifier: MS-PL */
/*
 * Scaffolding shared by the c_api standalone demos: C programs written against CNA's C ABI
 * (CNA::CApi). It is deliberately small and contains no CNA wrappers -- each demo makes its own
 * CNA calls where the reader can see them. What lives here is what cna-demos needs from every
 * standalone program:
 *
 *   - the command line the runner passes: --automated, --frames N, --screenshot PATH, --log-level L
 *     (CNA's log output, warn by default as in cna-demos);
 *   - checks, printed as "  check: ..." / "  CHECK FAILED: ...", and the exit code the runner
 *     reads (0 passed, 1 failed, 77 skipped);
 *   - printing the C ABI's diagnostic for a failed call (cna_error_get_last_info and
 *     cna_error_copy_last_message);
 *   - saving the back buffer as a PNG (cna_graphics_device_get_backbuffer_data_rgba8 plus a
 *     minimal uncompressed PNG writer), for --screenshot, and selecting the HiDef profile that
 *     read-back needs (cna_demo_use_hidef), as DemoGame does for the in-process demos.
 *
 * Header-only (static inline functions), C99, so each demo stays one translation unit.
 */
#ifndef CNA_DEMOS_C_API_DEMO_H
#define CNA_DEMOS_C_API_DEMO_H

#include <CNA/C/cna.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CNA_DEMO_EXIT_PASSED 0
#define CNA_DEMO_EXIT_FAILED 1
#define CNA_DEMO_EXIT_SKIPPED 77

typedef struct CnaDemo {
    const char* id;
    int automated;          /* unattended run: no person at the keyboard */
    int frames;             /* frames to run; 0 = until the window is closed */
    const char* screenshot; /* PNG to write on the last frame, or NULL */
    int checks;
    int failures;
} CnaDemo;

/* Parses the runner's arguments. default_frames applies to an automated run without --frames. */
static inline void cna_demo_init(CnaDemo* demo, const char* id, int argc, char** argv, int default_frames)
{
    const char* log_level = "warn";
    int i;
    memset(demo, 0, sizeof(*demo));
    demo->id = id;
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--automated") == 0) {
            demo->automated = 1;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            demo->frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            demo->screenshot = argv[++i];
        } else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            log_level = argv[++i];
        }
    }
    /* CNA's own log output, at the level cna-demos runs with (warn unless asked otherwise). */
    {
        static const char* const names[] = {"fatal", "error", "warn", "info", "debug", "trace"};
        static const CNA_LogLevel levels[] = {CNA_LOG_LEVEL_FATAL, CNA_LOG_LEVEL_ERROR, CNA_LOG_LEVEL_WARN,
                                              CNA_LOG_LEVEL_INFO,  CNA_LOG_LEVEL_DEBUG, CNA_LOG_LEVEL_TRACE};
        CNA_LogLevel level = CNA_LOG_LEVEL_WARN;
        size_t n;
        for (n = 0; n < sizeof(names) / sizeof(names[0]); ++n) {
            if (strcmp(log_level, names[n]) == 0) {
                level = levels[n];
            }
        }
        (void)cna_logger_set_minimum_level(level);
    }
    if (demo->automated && demo->frames <= 0) {
        demo->frames = default_frames;
    }
    printf("== %s\n", id);
}

static inline void cna_demo_print_result_diagnostic(const char* what, CNA_Result result)
{
    CNA_ErrorInfo info;
    char message[512];
    uint64_t length = 0U;
    printf("  %s returned %u", what, (unsigned)result);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (cna_error_get_last_info(&info) == CNA_RESULT_SUCCESS) {
        printf(" (error category %u)", (unsigned)info.category);
    }
    if (cna_error_copy_last_message(message, sizeof(message) - 1U, &length) == CNA_RESULT_SUCCESS) {
        message[length] = '\0';
        printf(": %s", message);
    }
    printf("\n");
}

/* Records one check. Returns the condition, so a demo can stop when a precondition failed. */
static inline int cna_demo_check(CnaDemo* demo, int condition, const char* what)
{
    ++demo->checks;
    if (condition) {
        printf("  check: %s\n", what);
    } else {
        ++demo->failures;
        printf("  CHECK FAILED: %s\n", what);
    }
    return condition;
}

/* Checks that a C ABI call succeeded; on failure prints the ABI's diagnostic as well. */
static inline int cna_demo_check_result(CnaDemo* demo, CNA_Result result, const char* what)
{
    if (result != CNA_RESULT_SUCCESS) {
        cna_demo_print_result_diagnostic(what, result);
    }
    return cna_demo_check(demo, result == CNA_RESULT_SUCCESS, what);
}

/* Prints the summary and returns the process exit code. */
static inline int cna_demo_finish(const CnaDemo* demo)
{
    if (demo->failures == 0) {
        printf("== PASSED (%d checks)\n", demo->checks);
        return CNA_DEMO_EXIT_PASSED;
    }
    printf("== FAILED (%d of %d checks)\n", demo->failures, demo->checks);
    return CNA_DEMO_EXIT_FAILED;
}

/* Reports a missing requirement (exit code 77, SKIPPED in cna-demos). */
static inline int cna_demo_skip(const char* reason)
{
    printf("== SKIPPED: %s\n", reason);
    return CNA_DEMO_EXIT_SKIPPED;
}

/*
 * Selects XNA's HiDef profile for the game, as DemoGame does for the in-process demos: the C game's
 * device starts in Reach, where GraphicsDevice.GetBackBufferData -- the screenshot read-back -- is
 * refused. This creates the game's graphics device manager (the game accepts one), requests HiDef
 * and applies it. Call it after cna_game_create and before the run; destroy *out_manager with
 * cna_graphics_device_manager_destroy before cna_game_destroy.
 */
static inline CNA_Result cna_demo_use_hidef(CNA_Handle game, CNA_Handle* out_manager)
{
    CNA_Result result = cna_graphics_device_manager_create(game, out_manager);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_manager_set_graphics_profile(*out_manager, CNA_GRAPHICS_PROFILE_HI_DEF);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_manager_apply_changes(*out_manager);
    }
    return result;
}

/* ---- PNG writer: 8-bit RGBA, stored (uncompressed) deflate blocks ---------------------------- */

static inline uint32_t cna_demo_crc32(uint32_t crc, const unsigned char* data, size_t length)
{
    static uint32_t table[256];
    static int ready = 0;
    size_t i;
    if (!ready) {
        uint32_t n;
        for (n = 0; n < 256U; ++n) {
            uint32_t c = n;
            int k;
            for (k = 0; k < 8; ++k) {
                c = (c & 1U) ? (UINT32_C(0xEDB88320) ^ (c >> 1)) : (c >> 1);
            }
            table[n] = c;
        }
        ready = 1;
    }
    crc = ~crc;
    for (i = 0; i < length; ++i) {
        crc = table[(crc ^ data[i]) & 0xFFU] ^ (crc >> 8);
    }
    return ~crc;
}

static inline void cna_demo_put32(unsigned char* out, uint32_t value)
{
    out[0] = (unsigned char)(value >> 24);
    out[1] = (unsigned char)(value >> 16);
    out[2] = (unsigned char)(value >> 8);
    out[3] = (unsigned char)value;
}

static inline int cna_demo_write_chunk(FILE* file, const char* type, const unsigned char* data, uint32_t length)
{
    unsigned char header[8];
    unsigned char trailer[4];
    uint32_t crc;
    cna_demo_put32(header, length);
    memcpy(header + 4, type, 4);
    crc = cna_demo_crc32(0U, header + 4, 4);
    crc = cna_demo_crc32(crc, data, length);
    cna_demo_put32(trailer, crc);
    return fwrite(header, 1, 8, file) == 8 && (length == 0U || fwrite(data, 1, length, file) == length) &&
           fwrite(trailer, 1, 4, file) == 4;
}

static inline int cna_demo_write_png(const char* path, const CNA_Color* pixels, uint32_t width, uint32_t height)
{
    static const unsigned char signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    const size_t row = (size_t)width * 4U + 1U;
    const size_t raw_size = row * height;
    const size_t blocks = raw_size / 65535U + 1U;
    const size_t zlib_size = 2U + raw_size + blocks * 5U + 4U;
    unsigned char* raw = (unsigned char*)malloc(raw_size);
    unsigned char* zlib = (unsigned char*)malloc(zlib_size);
    unsigned char ihdr[13];
    size_t y;
    size_t at = 0;
    size_t done = 0;
    uint32_t a = 1U;
    uint32_t b = 0U;
    FILE* file = NULL;
    int ok = 0;
    if (raw == NULL || zlib == NULL) {
        goto cleanup;
    }
    for (y = 0; y < height; ++y) {
        size_t x;
        raw[y * row] = 0; /* filter: none */
        for (x = 0; x < width; ++x) {
            const CNA_Color* c = &pixels[y * width + x];
            unsigned char* p = &raw[y * row + 1U + x * 4U];
            p[0] = c->r;
            p[1] = c->g;
            p[2] = c->b;
            p[3] = c->a;
        }
    }
    zlib[at++] = 0x78;
    zlib[at++] = 0x01;
    do {
        const size_t size = raw_size - done > 65535U ? 65535U : raw_size - done;
        zlib[at++] = (unsigned char)(done + size == raw_size ? 1 : 0);
        zlib[at++] = (unsigned char)(size & 0xFFU);
        zlib[at++] = (unsigned char)(size >> 8);
        zlib[at++] = (unsigned char)(~size & 0xFFU);
        zlib[at++] = (unsigned char)((~size >> 8) & 0xFFU);
        memcpy(zlib + at, raw + done, size);
        at += size;
        done += size;
    } while (done < raw_size);
    for (y = 0; y < raw_size; ++y) {
        a = (a + raw[y]) % 65521U;
        b = (b + a) % 65521U;
    }
    cna_demo_put32(zlib + at, (b << 16) | a);
    at += 4;

    cna_demo_put32(ihdr, width);
    cna_demo_put32(ihdr + 4, height);
    ihdr[8] = 8;  /* bit depth */
    ihdr[9] = 6;  /* colour type: RGBA */
    ihdr[10] = 0; /* compression */
    ihdr[11] = 0; /* filter */
    ihdr[12] = 0; /* interlace */
    file = fopen(path, "wb");
    if (file == NULL) {
        goto cleanup;
    }
    ok = fwrite(signature, 1, 8, file) == 8 && cna_demo_write_chunk(file, "IHDR", ihdr, 13U) &&
         cna_demo_write_chunk(file, "IDAT", zlib, (uint32_t)at) && cna_demo_write_chunk(file, "IEND", NULL, 0U);
    ok = fclose(file) == 0 && ok;
cleanup:
    free(raw);
    free(zlib);
    return ok;
}

/*
 * Saves the back buffer to demo->screenshot when one was requested. Call it from the draw callback
 * of the last frame, after drawing: the back buffer is read before the frame is presented.
 */
static inline void cna_demo_capture(CnaDemo* demo, CNA_Handle graphics_device)
{
    CNA_BackBufferInfo info;
    CNA_Color* pixels;
    uint64_t count = 0U;
    CNA_Result result;
    if (demo->screenshot == NULL) {
        return;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (!cna_demo_check_result(demo, cna_graphics_device_get_backbuffer_info(graphics_device, &info),
                               "screenshot: cna_graphics_device_get_backbuffer_info")) {
        return;
    }
    pixels = (CNA_Color*)malloc((size_t)info.width * info.height * sizeof(CNA_Color));
    if (pixels == NULL) {
        return;
    }
    result = cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, pixels,
                                                           (uint64_t)info.width * info.height, &count);
    if (result == CNA_RESULT_NOT_SUPPORTED) {
        cna_demo_print_result_diagnostic("screenshot: cna_graphics_device_get_backbuffer_data_rgba8", result);
    } else if (cna_demo_check_result(demo, result, "screenshot: cna_graphics_device_get_backbuffer_data_rgba8")) {
        if (cna_demo_write_png(demo->screenshot, pixels, info.width, info.height)) {
            printf("  screenshot: %s (%ux%u)\n", demo->screenshot, (unsigned)info.width, (unsigned)info.height);
        } else {
            cna_demo_check(demo, 0, "screenshot: the PNG file was written");
        }
    }
    free(pixels);
}

#endif
