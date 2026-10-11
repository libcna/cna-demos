# Script mode (cmake -P): checks that `cna-demos --catalog-json` is valid JSON
# and that its demo count matches the array it carries. Uses CMake's own JSON
# parser so the check needs no Python.
#   cmake -DCNA_DEMOS_EXE=<path> -P CnaDemosCheckCatalog.cmake

execute_process(COMMAND "${CNA_DEMOS_EXE}" --catalog-json
    OUTPUT_VARIABLE _json RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "cna-demos --catalog-json exited with ${_result}")
endif()

string(JSON _schema ERROR_VARIABLE _error GET "${_json}" schema)
if(_error)
    message(FATAL_ERROR "catalog is not valid JSON: ${_error}")
endif()
if(NOT _schema STREQUAL "cna-demos-catalog/1")
    message(FATAL_ERROR "unexpected catalog schema '${_schema}'")
endif()

string(JSON _count GET "${_json}" demoCount)
string(JSON _length LENGTH "${_json}" demos)
if(NOT _count EQUAL _length)
    message(FATAL_ERROR "demoCount (${_count}) differs from the number of demos (${_length})")
endif()

# Every demo must carry the fields the documentation generator relies on.
if(DEFINED CNA_DEMOS_ISSUES_FILE)
    file(READ "${CNA_DEMOS_ISSUES_FILE}" _issues)
endif()
if(_length GREATER 0)
    math(EXPR _last "${_length} - 1")
    foreach(_i RANGE 0 ${_last})
        # string(JSON) reparses its input each time. Extract this small object once;
        # parsing the entire catalog for every field made 401 demos exceed 30 seconds.
        string(JSON _demo GET "${_json}" demos ${_i})
        foreach(_field id title description category kind automation apis source)
            string(JSON _value ERROR_VARIABLE _error GET "${_demo}" ${_field})
            if(_error)
                message(FATAL_ERROR "demo #${_i} lacks '${_field}': ${_error}")
            endif()
        endforeach()
        # Every knownIssue must name an entry that exists in docs/CNA_ISSUES.md.
        if(DEFINED CNA_DEMOS_ISSUES_FILE)
            string(JSON _issue GET "${_demo}" knownIssue)
            if(NOT _issue STREQUAL "")
                string(FIND "${_issues}" "### ${_issue} " _found)
                if(_found EQUAL -1)
                    string(JSON _id GET "${_demo}" id)
                    message(FATAL_ERROR "${_id}: knownIssue ${_issue} has no entry in ${CNA_DEMOS_ISSUES_FILE}")
                endif()
            endif()
        endif()
    endforeach()
endif()

message(STATUS "catalog OK: ${_length} demos")
