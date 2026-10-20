# SPDX-License-Identifier: MS-PL
cmake_minimum_required(VERSION 3.30)
project(CnaMixedRuntimeProbe LANGUAGES CXX)
if(PROBE_OLD_POLICIES)
    # Match the consuming demo project's policy baseline, not just its CMake binary.
    cmake_policy(VERSION 3.23)
endif()
include("${CNA_SOURCE_DIR}/cmake/SharedRuntimePolicy.cmake")
include("${CNA_SOURCE_DIR}/cmake/SharedRuntimeLibrary.cmake")
add_compile_options(-Wall -Wextra -Werror)
add_subdirectory(modules)
set(CNA_SHARED_MODULES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules")
add_library(CNA INTERFACE)
if(PROBE_AUXILIARY)
    # The real umbrella places its independent XML dependency before the
    # deferred shared-runtime target. It is not part of the shared runtime.
    add_library(auxiliary STATIC auxiliary.cpp)
    target_link_libraries(CNA INTERFACE auxiliary)
endif()
cna_define_shared_runtime(CNA probe_core)

# A CNA sidecar exports a runtime dependency as a static archive, whereas the
# application's framework uses the shared umbrella. This is the historical shape.
add_library(sidecar STATIC sidecar.cpp)
target_link_libraries(sidecar PUBLIC probe_core)
add_library(framework STATIC framework.cpp)
target_link_libraries(framework PUBLIC CNA)
add_library(objects OBJECT object.cpp)
target_link_libraries(objects PUBLIC framework sidecar)
add_executable(mixed_runtime_probe main.cpp)
if(PROBE_WORKAROUND)
    target_link_libraries(mixed_runtime_probe PRIVATE "$<TARGET_FILE:cna_shared>")
endif()
if(PROBE_REVERSE)
    target_link_libraries(mixed_runtime_probe PRIVATE objects framework)
else()
    target_link_libraries(mixed_runtime_probe PRIVATE framework objects)
endif()
