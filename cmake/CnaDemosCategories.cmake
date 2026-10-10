# =============================================================================
# Build helpers for cna-demos: demo categories, standalone demos, warnings and
# executable finalisation.
# =============================================================================
#
# Demo source discovery
# ---------------------
# Every top-level directory demos/<category>/ is declared once in
# demos/CMakeLists.txt with cna_demos_add_category(). The .cpp files below it
# are discovered with file(GLOB_RECURSE ... CONFIGURE_DEPENDS), which:
#   * returns a sorted, deterministic list;
#   * is re-evaluated on every build, so adding a demo is "add one .cpp file and
#     build" -- no list to edit, no generated registry, no Python;
#   * stays fast with thousands of files (a directory walk per build).
# Each category becomes one OBJECT library. Object files are linked into the
# executable unconditionally, which is what keeps the self-registering static
# objects in each demo from being discarded by the linker (a static library
# would drop them).
# =============================================================================

include_guard(GLOBAL)

# -----------------------------------------------------------------------------
# Warnings: strict for our own sources, never applied to CNA's.
# -----------------------------------------------------------------------------
function(cna_demos_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(CNA_DEMOS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(CNA_DEMOS_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# -----------------------------------------------------------------------------
# cna_demos_add_category(<name> [REQUIRES <cache-var>...] [LIBRARIES <target>...])
#
# Compiles demos/<name>/**/*.cpp into the OBJECT library cna_demos_cat_<name>
# and records it for the cna-demos executable. REQUIRES names CMake variables
# (usually CNA options such as CNA_ENABLE_NET) that must be true for the
# category to be built; when one is false the category is skipped with a status
# message and its demos are absent from the catalog. LIBRARIES adds CNA targets
# that are not part of the `CNA` umbrella (CNA::Net, CNA::GamerServices, ...).
# -----------------------------------------------------------------------------
function(cna_demos_add_category name)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "REQUIRES;LIBRARIES")

    foreach(_requirement IN LISTS ARG_REQUIRES)
        if(NOT ${_requirement})
            message(STATUS "cna-demos: category '${name}' skipped (${_requirement} is off)")
            set_property(GLOBAL APPEND PROPERTY CNA_DEMOS_SKIPPED_CATEGORIES
                "${name}:${_requirement}")
            return()
        endif()
    endforeach()

    foreach(_library IN LISTS ARG_LIBRARIES)
        if(NOT TARGET ${_library})
            message(STATUS "cna-demos: category '${name}' skipped (CNA target ${_library} not built)")
            set_property(GLOBAL APPEND PROPERTY CNA_DEMOS_SKIPPED_CATEGORIES
                "${name}:${_library}")
            return()
        endif()
    endforeach()

    file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*.cpp")
    list(SORT _sources)
    list(LENGTH _sources _count)
    if(_count EQUAL 0)
        message(STATUS "cna-demos: category '${name}' has no sources yet")
        return()
    endif()

    set(_target cna_demos_cat_${name})
    add_library(${_target} OBJECT ${_sources})
    target_link_libraries(${_target} PUBLIC cna_demos_framework ${ARG_LIBRARIES})
    cna_demos_apply_warnings(${_target})

    set_property(GLOBAL APPEND PROPERTY CNA_DEMOS_CATEGORY_TARGETS ${_target})
    set_property(GLOBAL APPEND PROPERTY CNA_DEMOS_BUILT_CATEGORIES ${name})
    message(STATUS "cna-demos: category '${name}': ${_count} source file(s)")
endfunction()

# -----------------------------------------------------------------------------
# cna_demos_finalize_executable(<target>)
#
# Platform packaging every CNA executable needs: CNA's renderer runtime payload
# (shared libraries some renderers load), SDL3main, and the Windows/MinGW
# runtime DLL copies. Mirrors what cna-template does for its one executable.
# -----------------------------------------------------------------------------
function(cna_demos_finalize_executable target)
    if(COMMAND cna_copy_renderer_runtime)
        cna_copy_renderer_runtime(${target})
    endif()
    if(TARGET SDL3::SDL3main)
        target_link_libraries(${target} PRIVATE SDL3::SDL3main)
    endif()
    if(WIN32 AND COMMAND cna_copy_sdl_runtime)
        cna_copy_sdl_runtime(${target})
    endif()
    if(MINGW AND COMMAND cna_copy_mingw_cxx_runtime)
        cna_copy_mingw_cxx_runtime(${target})
    endif()
endfunction()

# -----------------------------------------------------------------------------
# cna_demos_add_standalone(SOURCES <file>... [LIBRARIES <target>...] [REQUIRES <var>...])
#
# Called from standalone/<demo-id>/CMakeLists.txt. The executable name is
# derived from the directory name (the demo ID) exactly as the C++ side derives
# it (CnaDemos::StandaloneExecutableName): "cna-demo-" followed by the ID with
# every '.' replaced by '-'. IDs never contain '-', so the mapping is unique.
# -----------------------------------------------------------------------------
function(cna_demos_add_standalone)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "SOURCES;LIBRARIES;REQUIRES")
    get_filename_component(_id "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

    foreach(_requirement IN LISTS ARG_REQUIRES)
        if(NOT ${_requirement})
            message(STATUS "cna-demos: standalone '${_id}' skipped (${_requirement} is off)")
            return()
        endif()
    endforeach()

    string(REPLACE "." "-" _exe "cna-demo-${_id}")
    add_executable(${_exe} ${ARG_SOURCES})
    target_link_libraries(${_exe} PRIVATE CNA ${ARG_LIBRARIES})
    set_target_properties(${_exe} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CNA_DEMOS_OUTPUT_DIR}")
    cna_demos_apply_warnings(${_exe})
    cna_demos_finalize_executable(${_exe})
    add_dependencies(${_exe} cna_demos_assets)
    add_dependencies(cna-demos ${_exe})
    set_property(GLOBAL APPEND PROPERTY CNA_DEMOS_STANDALONE_TARGETS ${_exe})
endfunction()
