/* ISSUE-145: title argument categories and output preservation through the actual C ABI. */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

static int checks, failures;
static void check(int ok, const char* name)
{
    ++checks;
    if (!ok) { ++failures; printf("FAIL %s\n", name); }
}
static CNA_StringView view(const char* text)
{
    CNA_StringView result = {text, (uint64_t)strlen(text)};
    return result;
}
int main(void)
{
    CNA_GameCallbacks callbacks = {sizeof(CNA_GameCallbacks), UINT32_C(1), 0, 0, 0, 0, 0, 0};
    CNA_GameCreateInfo info = {
        sizeof(CNA_GameCreateInfo), UINT32_C(1), CNA_TRUE, {0, 0, 0, 0, 0, 0, 0},
        INT64_C(166667), {"Title validation", UINT64_C(16)}, &callbacks
    };
    CNA_Handle game = CNA_INVALID_HANDLE;
    const char* const invalid[] = {"", "/rooted.bin", "../outside.bin", "C:drive.bin", "bad?.bin"};
    size_t index;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) return 2;
    for (index = 0; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        uint8_t buffer[4] = {0xA5, 0xA5, 0xA5, 0xA5};
        uint64_t bytes = UINT64_MAX;
        const CNA_Result result = cna_title_container_read_ext(game, view(invalid[index]), buffer, sizeof(buffer), &bytes);
        check(result == CNA_RESULT_INVALID_ARGUMENT, invalid[index]);
        check(buffer[0] == 0xA5 && buffer[1] == 0xA5 && buffer[2] == 0xA5 && buffer[3] == 0xA5,
              "invalid title read preserves all output bytes");
    }
    {
        uint8_t buffer[4] = {0xA5, 0xA5, 0xA5, 0xA5};
        uint64_t bytes = UINT64_MAX;
        check(cna_title_container_read_ext(game, view("."), buffer, sizeof(buffer), &bytes) == CNA_RESULT_IO,
              "directory remains an I/O failure");
        check(buffer[0] == 0xA5 && buffer[1] == 0xA5 && buffer[2] == 0xA5 && buffer[3] == 0xA5,
              "directory read preserves all output bytes");
    }
    check(cna_game_destroy(game) == CNA_RESULT_SUCCESS, "game cleanup");
    printf("checks=%d failed=%d\n", checks, failures);
    return failures ? 1 : 0;
}
