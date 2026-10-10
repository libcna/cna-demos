# CNA issues found by cna-demos

One purpose of cna-demos is to expose real CNA problems. A demo that reveals one does **not**
work around it silently: the problem is minimised, recorded here, and the demo either keeps
failing visibly or is marked with the issue ID in its `restrictions` metadata.

Each entry records the affected CNA area, the demo(s) involved, reproduction steps, observed
and expected behaviour, the environment, and the investigation status. IDs are permanent
(`CNA-DEMOS-ISSUE-NNN`); a fixed issue stays listed under "Resolved".

CNA revision used unless stated otherwise: `10a4ef1cb4cd832f761536d77588000104947f8d`
(CNA 0.1.0, "chore(release): prepare v0.1.0").

---

## Open

### CNA-DEMOS-ISSUE-001 -- `libcna.so` fails to link with CMake 3.28 (duplicate symbols)

- **Area:** build system -- CNA's shared runtime library (`CNA_SHARED_LIBRARY`,
  `cmake/SharedRuntimeLibrary.cmake`).
- **Demos:** none specifically; it blocks building any consumer.
- **Environment:** Ubuntu 24.04, CMake 3.28.3 (distribution package), GCC 14.2, lld (CNA's
  default linker choice), multi-renderer build
  `OPENGLES3;OPENGL33;SDL_RENDERER;VULKAN;SDL_GPU;SOFTWARE;HEADLESS;STUB`,
  `CNA_CNAEXT=ON`, `CNA_DEVICES=ON`, CNA added with `add_subdirectory()`.
- **Reproduction:**
  1. Configure cna-demos with `/usr/bin/cmake` 3.28.3 and `-DCNA_SHARED_LIBRARY=ON`
     (CNA's own default on Linux with CMake >= 3.27).
  2. `cmake --build <dir> --target cna-demos`.
- **Observed:** the `CNA/libcna.so` link fails with hundreds of
  `ld.lld: error: duplicate symbol: ...` (first ones: `CNA::Internal::Xnb::XnbAssemblyQualifier`,
  `CNA::Internal::Audio::DecodeMsAdpcm`, `CNA::Internal::Media::MediaLibraryIndex::...`,
  `Microsoft::Xna::Framework::Audio::SoundBank::...`). The generated link line lists
  `libcna_runtime.a` and `libcna_content.a` twice inside the first `--whole-archive` group, and
  `libcna_media.a`/`libcna_audio.a` and the input/graphics/renderer archives twice inside later
  groups -- the repetition CMake emits for static libraries in a dependency cycle. Inside
  `--whole-archive` every repetition re-adds every object, hence the duplicates.
- **Expected:** CNA builds on every CMake version its configuration admits. `CNA_SHARED_LIBRARY`
  defaults to ON for CMake >= 3.27, and `cmake_minimum_required` is 3.20.
- **Cross-check:** the identical configuration with CMake 4.4.4 (`pip install cmake`) generates a
  link line with every archive exactly once and links successfully.
- **Likely cause (not verified):** CMake 3.29 changed link-line de-duplication (policy CMP0156),
  so 3.27/3.28 still repeat cyclic static libraries. Which exact CMake version is the first good
  one, and whether CNA's default option set (CNAEXT/DEVICES off, single renderer) also
  reproduces on 3.28, has not been tested.
- **Workaround in cna-demos:** `cmake/CnaDemosCna.cmake` sets `CNA_SHARED_LIBRARY=OFF` (static
  CNA) when CMake is older than 3.29 and the user did not choose a value. With CMake >= 3.29 CNA's
  default is kept. This is a build-mode choice, not a change to CNA.
- **Suggested upstream fix:** raise the `CNA_SHARED_LIBRARY` support gate in CNA's
  `CMakeLists.txt` to the first CMake that de-duplicates correctly, or de-duplicate the
  WHOLE_ARCHIVE list in `cmake/SharedRuntimeLibrary.cmake` so cycle repetition cannot occur.
- **Status:** documented; not yet reported upstream.

---

## Resolved

(none yet)
