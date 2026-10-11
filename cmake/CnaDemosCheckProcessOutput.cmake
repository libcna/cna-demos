# SPDX-License-Identifier: MS-PL
# Capture through a pipe: terminal line buffering would hide the parent/child ordering defect.
execute_process(COMMAND "${CNA_DEMOS_PROCESS_TEST_EXE}"
    RESULT_VARIABLE _result OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 10)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Process fixture failed: ${_result}\n${_stderr}")
endif()
string(REPLACE "\r\n" "\n" _stdout "${_stdout}")
if(NOT _stdout STREQUAL "PARENT_CXX\nPARENT_C\nCHILD\n")
    message(FATAL_ERROR "Buffered parent output was overtaken by its child:\n${_stdout}")
endif()
