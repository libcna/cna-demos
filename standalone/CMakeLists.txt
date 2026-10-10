# =============================================================================
# Standalone demos: the documented exception to "every demo runs inside
# cna-demos". Each lives in standalone/<demo-id>/ with
#   metadata.cpp    CNA_DEMOS_STANDALONE({...}) -- compiled into cna-demos so the
#                   browser and the catalog know the demo exists
#   main.cpp        the program itself (plus any further sources it needs)
#   CMakeLists.txt  cna_demos_add_standalone(SOURCES main.cpp ...)
# The executable is called cna-demo-<id with '.' replaced by '-'>.
# See docs/ARCHITECTURE.md "Standalone demos".
# =============================================================================

# Metadata is always compiled in, even with CNA_DEMOS_BUILD_STANDALONE=OFF: the
# browser then still lists the demo and explains that its executable is absent.
file(GLOB _cna_demos_standalone_metadata CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*/metadata.cpp")
list(SORT _cna_demos_standalone_metadata)
if(_cna_demos_standalone_metadata)
    add_library(cna_demos_standalone_metadata OBJECT ${_cna_demos_standalone_metadata})
    target_link_libraries(cna_demos_standalone_metadata PUBLIC cna_demos_framework)
    cna_demos_apply_warnings(cna_demos_standalone_metadata)
    target_link_libraries(cna-demos PRIVATE cna_demos_standalone_metadata)
endif()

if(CNA_DEMOS_BUILD_STANDALONE)
    file(GLOB _cna_demos_standalone_dirs LIST_DIRECTORIES true CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*")
    list(SORT _cna_demos_standalone_dirs)
    foreach(_dir IN LISTS _cna_demos_standalone_dirs)
        if(IS_DIRECTORY "${_dir}" AND EXISTS "${_dir}/CMakeLists.txt")
            add_subdirectory("${_dir}")
        endif()
    endforeach()
endif()
