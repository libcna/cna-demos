# =============================================================================
# cna-demos -- the CNA Demo Browser.
# =============================================================================
# One user-facing executable (`cna-demos`) holds every in-process demo. Demos
# are small .cpp files under demos/<category>/...; each registers itself with
# the demo registry and is compiled into one OBJECT library per top-level
# category. A handful of demos that cannot run inside an already-running
# browser are separate executables under standalone/.
#
# See docs/ARCHITECTURE.md for the full picture and docs/DEMO_AUTHORING.md for
# how to add a demo (normally: add one .cpp file and re-run the build).
# =============================================================================

# 3.23: the easy-gl/meta-gl siblings used by CNA's GL renderers require it
# (FILE_SET HEADERS); CNA itself needs 3.20.
cmake_minimum_required(VERSION 3.23)

project(CnaDemos VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(CNA_DEMOS_BUILD_TESTS      "Register cna-demos CTest tests" ON)
option(CNA_DEMOS_BUILD_STANDALONE "Build the standalone (separate-executable) demos" ON)
option(CNA_DEMOS_WARNINGS_AS_ERRORS "Treat compiler warnings in cna-demos sources as errors" OFF)

message(STATUS "cna-demos: version ${PROJECT_VERSION}, ${CMAKE_SYSTEM_NAME}, "
               "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, ${CMAKE_BUILD_TYPE}")

include(CnaDemosCna)          # locates CNA and add_subdirectory()s it
include(CnaDemosCategories)   # cna_demos_add_category(), warning flags

# Everything is placed in one output directory so the main executable, the
# standalone demo executables and the copied assets sit side by side; CNA
# resolves content relative to the executable's directory.
set(CNA_DEMOS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

# -----------------------------------------------------------------------------
# Demo framework: registry, metadata, runner, the DemoGame base class.
# -----------------------------------------------------------------------------
add_library(cna_demos_framework STATIC
    src/framework/Registry.cpp
    src/framework/Metadata.cpp
    src/framework/DemoContext.cpp
    src/framework/DemoGame.cpp
    src/framework/Runner.cpp
    src/framework/Catalog.cpp
    src/framework/Validation.cpp
    src/framework/Process.cpp
)
target_include_directories(cna_demos_framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(cna_demos_framework PUBLIC CNA)
target_compile_definitions(cna_demos_framework PRIVATE
    "CNA_DEMOS_VERSION=\"${PROJECT_VERSION}\""
    "CNA_DEMOS_CNA_VERSION=\"${CNA_DEMOS_CNA_VERSION}\""
    "CNA_DEMOS_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
cna_demos_apply_warnings(cna_demos_framework)

# -----------------------------------------------------------------------------
# Demo categories (demos/<category>/**/*.cpp, one OBJECT library each).
# -----------------------------------------------------------------------------
add_subdirectory(demos)
get_property(CNA_DEMOS_CATEGORY_TARGETS GLOBAL PROPERTY CNA_DEMOS_CATEGORY_TARGETS)

# -----------------------------------------------------------------------------
# Shared assets, copied next to the executables.
# -----------------------------------------------------------------------------
add_custom_target(cna_demos_assets ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CNA_DEMOS_OUTPUT_DIR}/assets"
    COMMENT "Copying demo assets to ${CNA_DEMOS_OUTPUT_DIR}/assets"
    VERBATIM)

# -----------------------------------------------------------------------------
# The cna-demos executable: CLI + interactive terminal browser.
# -----------------------------------------------------------------------------
add_executable(cna-demos
    src/app/main.cpp
    src/app/Cli.cpp
    src/app/Browser.cpp
)
target_link_libraries(cna-demos PRIVATE cna_demos_framework ${CNA_DEMOS_CATEGORY_TARGETS})
set_target_properties(cna-demos PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CNA_DEMOS_OUTPUT_DIR}")
cna_demos_apply_warnings(cna-demos)
cna_demos_finalize_executable(cna-demos)
add_dependencies(cna-demos cna_demos_assets)

# -----------------------------------------------------------------------------
# Standalone demos (standalone/<demo-id>/), the documented exception.
# -----------------------------------------------------------------------------
add_subdirectory(standalone)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(CNA_DEMOS_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING AND NOT ANDROID AND NOT EMSCRIPTEN)
    enable_testing()
    include(CnaDemosTests)
endif()
