# Keep the build-time serializer and its dependencies in the tool process, outside the browser.
if(TARGET CNA::ContentPipelineBuild)
    cna_demos_add_standalone(SOURCES main.cpp LIBRARIES CNA::ContentPipelineBuild)
else()
    message(STATUS "cna-demos: intermediate scalar formats tool omitted (CNA::ContentPipelineBuild unavailable)")
endif()
