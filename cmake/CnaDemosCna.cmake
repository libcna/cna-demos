# =============================================================================
# CNA dependency integration for cna-demos.
# =============================================================================
# CNA does not install or export a CMake package (there is no find_package(CNA)).
# The supported way to consume it -- the one cna-template uses -- is to add a
# CNA source checkout with add_subdirectory() and link the `CNA` INTERFACE
# umbrella target, which carries every framework module, the selected
# renderer(s), Sharp Runtime and all public include directories/definitions.
#
# CNA in turn expects its own siblings next to it:
#   ../sharp-runtime            always
#   ../easy-gl and ../meta-gl   for the OPENGLES3 / OPENGL33 / WEBGL2 renderers
# and its vendored submodules (third_party/SDL, SDL_mixer, draco).
# CNA itself reports a missing sibling or submodule with a precise message, so
# this file only checks that CNA itself is where we were told it is.
#
# Renderer selection is CNA's own public interface and is passed straight
# through: CNA_GRAPHICS_RENDERER (the default renderer) and, optionally,
# CNA_GRAPHICS_RENDERERS (a list compiled into one binary and chosen at runtime
# with `cna-demos --renderer NAME` or the CNA_GRAPHICS_RENDERER environment
# variable). See docs/ARCHITECTURE.md "Renderers".
# =============================================================================

include_guard(GLOBAL)

set(_cna_demos_default_cna_root "${CMAKE_CURRENT_SOURCE_DIR}/../cna")
if(DEFINED ENV{CNA_ROOT_DIR} AND NOT "$ENV{CNA_ROOT_DIR}" STREQUAL "")
    set(_cna_demos_default_cna_root "$ENV{CNA_ROOT_DIR}")
endif()

set(CNA_ROOT_DIR "${_cna_demos_default_cna_root}" CACHE PATH
    "Path to the CNA framework source checkout (sibling ../cna by default)")
get_filename_component(CNA_ROOT_DIR "${CNA_ROOT_DIR}" ABSOLUTE)

if(NOT EXISTS "${CNA_ROOT_DIR}/CMakeLists.txt"
        OR NOT EXISTS "${CNA_ROOT_DIR}/modules/runtime/include/Microsoft/Xna/Framework/Game.hpp")
    message(FATAL_ERROR
        "cna-demos: no CNA source checkout found at '${CNA_ROOT_DIR}'.\n"
        "\n"
        "cna-demos builds against a CNA source tree (CNA exports no CMake package).\n"
        "Either clone CNA and its siblings next to this repository:\n"
        "  cd ${CMAKE_CURRENT_SOURCE_DIR}/..\n"
        "  git clone https://github.com/libcna/cna.git\n"
        "  git clone https://github.com/libcna/sharp-runtime.git\n"
        "  git clone https://github.com/libcna/easy-gl.git      # GL renderers only\n"
        "  git clone https://github.com/libcna/meta-gl.git      # GL renderers only\n"
        "  git -C cna submodule update --init third_party/SDL third_party/SDL_mixer \\\n"
        "      third_party/draco\n"
        "or point the build at an existing checkout:\n"
        "  cmake -S . -B build -DCNA_ROOT_DIR=/path/to/cna\n"
        "(the CNA_ROOT_DIR environment variable works too).\n"
        "README.md \"Requirements\" lists everything that is needed.")
endif()

# cna-demos is an application: it needs neither CNA's own test suite nor its
# example programs, and easy-gl's gates default ON. These are FORCEd for the
# same reason cna-template forces them -- they are build-size switches of the
# dependency, not knobs of this project.
set(CNA_BUILD_TESTS       OFF CACHE BOOL "Build CNA tests"                FORCE)
set(CNA_BUILD_EXAMPLES    OFF CACHE BOOL "Build CNA example applications" FORCE)
set(EASYGL_BUILD_TESTS    OFF CACHE BOOL "Build easy-gl tests"            FORCE)
set(EASYGL_BUILD_EXAMPLES OFF CACHE BOOL "Build easy-gl examples"         FORCE)

# Optional CNA modules. Their CNA defaults are kept unless cna-demos asks for
# more: CNA_CNAEXT and CNA_DEVICES are OFF in CNA, but a demo collection that
# aims to cover CNA should build them, so cna-demos turns them on by default.
# Each can still be switched off explicitly, and the demo categories that need
# a module are then simply not compiled (see cmake/CnaDemosCategories.cmake).
option(CNA_CNAEXT  "Enable CNA's standalone graphics extensions (ASCII, CRT, Depth, DebugDraw)" ON)
option(CNA_DEVICES "Enable CNA-specific device extensions (camera, clipboard, power, ...)" ON)

# CNA links its runtime into one shared libcna.so by default on Linux (CNA_SHARED_LIBRARY) and
# enables that mode for CMake >= 3.27. With CMake 3.27/3.28 the libcna.so link repeats module
# archives that form a dependency cycle inside the WHOLE_ARCHIVE group and fails with
# duplicate symbols (docs/CNA_ISSUES.md, CNA-DEMOS-ISSUE-001); CMake 4.x links it correctly.
# On an affected CMake, default to CNA's static mode instead -- an explicit
# -DCNA_SHARED_LIBRARY=... always wins.
if(NOT DEFINED CNA_SHARED_LIBRARY AND CMAKE_VERSION VERSION_LESS 3.29)
    message(STATUS "cna-demos: CMake ${CMAKE_VERSION} < 3.29 -- linking CNA statically "
                   "(CNA_SHARED_LIBRARY=OFF); see docs/CNA_ISSUES.md CNA-DEMOS-ISSUE-001")
    set(CNA_SHARED_LIBRARY OFF CACHE BOOL
        "Link the CNA runtime into one shared library (libcna.so) instead of statically into every executable")
endif()

message(STATUS "cna-demos: using CNA at ${CNA_ROOT_DIR}")
# SYSTEM (CMake >= 3.25) makes CNA's include directories system includes for our targets, so
# cna-demos' strict warning flags apply to cna-demos' own code and not to CNA's headers.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
    add_subdirectory("${CNA_ROOT_DIR}" CNA EXCLUDE_FROM_ALL SYSTEM)
else()
    add_subdirectory("${CNA_ROOT_DIR}" CNA EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET CNA)
    message(FATAL_ERROR
        "cna-demos: the checkout at '${CNA_ROOT_DIR}' did not define the `CNA` umbrella "
        "target. This CNA revision is not supported; see README.md \"Requirements\".")
endif()

# The version CNA reports about itself, for --version and the catalog.
set(_cna_demos_cna_version_header "${CNA_BINARY_DIR}/generated/include/CNA/Version.hpp")
set(CNA_DEMOS_CNA_VERSION "unknown")
if(EXISTS "${_cna_demos_cna_version_header}")
    file(STRINGS "${_cna_demos_cna_version_header}" _line
        REGEX "^#define CNA_VERSION_STRING \"[^\"]*\"$")
    string(REGEX REPLACE "^#define CNA_VERSION_STRING \"([^\"]*)\"$" "\\1"
        CNA_DEMOS_CNA_VERSION "${_line}")
endif()

# What the build compiled in, for diagnostics and the catalog.
set(CNA_DEMOS_RENDERER_LIST "${CNA_GRAPHICS_RENDERER}")
if(DEFINED CNA_GRAPHICS_RENDERERS AND NOT "${CNA_GRAPHICS_RENDERERS}" STREQUAL "")
    set(CNA_DEMOS_RENDERER_LIST "${CNA_GRAPHICS_RENDERERS}")
endif()
message(STATUS "cna-demos: CNA ${CNA_DEMOS_CNA_VERSION}, default renderer "
               "${CNA_GRAPHICS_RENDERER}, compiled-in renderers: ${CNA_DEMOS_RENDERER_LIST}")
