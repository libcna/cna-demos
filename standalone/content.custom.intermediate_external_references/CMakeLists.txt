# SPDX-License-Identifier: MS-PL
if(TARGET CNA::ContentPipelineBuild)
    cna_demos_add_standalone(SOURCES main.cpp LIBRARIES CNA::ContentPipelineBuild)
else()
    message(STATUS "cna-demos: external XML references tool omitted (CNA::ContentPipelineBuild unavailable)")
endif()
