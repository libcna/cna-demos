# SPDX-License-Identifier: MS-PL
if(TARGET CNA::ContentCompiler)
    cna_demos_add_standalone(SOURCES main.cpp LIBRARIES CNA::ContentCompiler cna_demos_framework)
else()
    message(STATUS "cna-demos: multi-artifact publication tool omitted (CNA::ContentCompiler unavailable)")
endif()
