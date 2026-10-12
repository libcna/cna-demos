# SPDX-License-Identifier: MS-PL
if(TARGET CNA::ContentCompiler AND TARGET CNA::ContentPipelineBuild)
    cna_demos_add_standalone(SOURCES main.cpp LIBRARIES CNA::ContentCompiler CNA::ContentPipelineBuild cna_demos_framework)
else()
    message(STATUS "cna-demos: XML-to-XNB compiler tool omitted (content compiler/build modules unavailable)")
endif()
