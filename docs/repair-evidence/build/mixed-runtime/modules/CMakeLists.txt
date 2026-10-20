# SPDX-License-Identifier: MS-PL
add_library(probe_core STATIC core.cpp)
if(PROBE_CYCLE)
    add_library(probe_peer STATIC peer.cpp)
    target_link_libraries(probe_core PUBLIC probe_peer)
    target_link_libraries(probe_peer PUBLIC probe_core)
    target_compile_definitions(probe_core PRIVATE PROBE_CYCLE)
endif()
