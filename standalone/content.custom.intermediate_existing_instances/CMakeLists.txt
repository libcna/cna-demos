# SPDX-License-Identifier: MS-PL
if(TARGET CNA::ContentPipelineBuild)
    cna_demos_add_standalone(SOURCES main.cpp LIBRARIES CNA::ContentPipelineBuild cna_demos_framework)
else()
    message(STATUS "cna-demos: existing-instance XML tool omitted (CNA::ContentPipelineBuild unavailable)")
endif()
