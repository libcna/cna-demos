# =============================================================================
# Runs inside ctest (included through TEST_INCLUDE_FILES, see
# cmake/CnaDemosTests.cmake). Asks cna-demos for its demo list and registers
# one test per demo and renderer.
#
# Input variables (set by the generated wrapper):
#   CNA_DEMOS_EXE, CNA_DEMOS_TEST_RENDERERS, CNA_DEMOS_WINDOWLESS_RENDERERS,
#   CNA_DEMOS_TEST_TIMEOUT, CNA_DEMOS_TEST_AUDIO_DRIVER
# =============================================================================

if(NOT EXISTS "${CNA_DEMOS_EXE}")
    # Not built yet: register one test that says so instead of silently running nothing.
    add_test(cna_demos.discovery "${CMAKE_COMMAND}" -E echo
             "cna-demos executable not built: ${CNA_DEMOS_EXE}")
    set_tests_properties(cna_demos.discovery PROPERTIES WILL_FAIL TRUE)
    return()
endif()

execute_process(
    COMMAND "${CNA_DEMOS_EXE}" --list-ids
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE _error
    RESULT_VARIABLE _result
    TIMEOUT 60)

if(NOT _result EQUAL 0)
    add_test(cna_demos.discovery "${CMAKE_COMMAND}" -E echo
             "cna-demos --list-ids failed (${_result}): ${_error}")
    set_tests_properties(cna_demos.discovery PROPERTIES WILL_FAIL TRUE)
    return()
endif()

string(REPLACE "\n" ";" _lines "${_output}")
foreach(_renderer IN LISTS CNA_DEMOS_TEST_RENDERERS)
    # list(FIND), not if(IN_LIST): this script runs inside ctest, where CMake 3.x leaves policy
    # CMP0057 (IN_LIST) unset.
    list(FIND CNA_DEMOS_WINDOWLESS_RENDERERS "${_renderer}" _windowless_index)
    if(NOT _windowless_index EQUAL -1)
        set(_display_label headless)
    else()
        set(_display_label display)
    endif()

    foreach(_line IN LISTS _lines)
        if(_line STREQUAL "")
            continue()
        endif()
        # "<id> <kind> <automation> <needs> <known-issue>"
        string(REPLACE " " ";" _fields "${_line}")
        list(LENGTH _fields _count)
        if(NOT _count EQUAL 5)
            continue()
        endif()
        list(GET _fields 0 _id)
        list(GET _fields 1 _kind)
        list(GET _fields 2 _automation)
        list(GET _fields 3 _needs)
        list(GET _fields 4 _known_issue)
        # "<issue>@R1,R2": the issue affects those renderers only; elsewhere the demo must pass.
        if(_known_issue MATCHES "^([^@]+)@(.+)$")
            set(_known_issue "${CMAKE_MATCH_1}")
            string(REPLACE "," ";" _affected "${CMAKE_MATCH_2}")
            list(FIND _affected "${_renderer}" _affected_index)
            if(_affected_index EQUAL -1)
                set(_known_issue "-")
            endif()
        endif()
        string(REGEX MATCH "^[^.]+" _category "${_id}")

        set(_labels "demo;${_category};${_automation};${_kind}-demo;${_renderer}")
        if(_kind STREQUAL "console")
            list(APPEND _labels headless)
        else()
            list(APPEND _labels ${_display_label})
        endif()

        if(NOT _known_issue STREQUAL "-")
            list(APPEND _labels known-issue)
        endif()

        set(_name "cna_demos.demo.${_renderer}.${_id}")
        add_test(${_name} "${CNA_DEMOS_EXE}" "${_id}" --automated --quiet --renderer "${_renderer}")
        set(_environment "")
        if(NOT CNA_DEMOS_TEST_AUDIO_DRIVER STREQUAL "")
            list(APPEND _environment "SDL_AUDIO_DRIVER=${CNA_DEMOS_TEST_AUDIO_DRIVER}")
        endif()
        set_tests_properties(${_name} PROPERTIES
            LABELS "${_labels}"
            SKIP_RETURN_CODE 77
            TIMEOUT "${CNA_DEMOS_TEST_TIMEOUT}"
            ENVIRONMENT "${_environment}")
        if(_automation STREQUAL "manual")
            set_tests_properties(${_name} PROPERTIES DISABLED TRUE)
        endif()
        # System Link demos find every session on the network, each other's included: one at a time.
        if(_needs MATCHES "(^|,)network(,|$)")
            set_tests_properties(${_name} PROPERTIES RESOURCE_LOCK cna_demos_system_link)
        endif()
        # Dummy-audio transport still advances on a backend thread. Concurrent audio-demo
        # processes can starve that thread long enough for wall-clock playback assertions to test
        # scheduler contention instead of Play/Pause/Stop behavior.
        if(_needs MATCHES "(^|,)audio(,|$)")
            set_tests_properties(${_name} PROPERTIES RESOURCE_LOCK cna_demos_audio_timing)
        endif()
        # These two assertions distinguish transport behavior solely through elapsed playback
        # time. Run them without unrelated CPU-heavy demos that can delay the backend clock.
        if(_id STREQUAL "audio.sound_effect_instance.playback_state" OR
           _id STREQUAL "media.media_player.play_song")
            set_tests_properties(${_name} PROPERTIES RUN_SERIAL TRUE)
        endif()
        # These demos temporarily mutate process-external desktop state. Concurrent
        # SDL processes on the same display can move the pointer, replace clipboard
        # ownership or toggle the X11 screen saver between mutation and observation.
        if(_id STREQUAL "c_api.input.keyboard_mouse" OR
           _id STREQUAL "c_api.input.clipboard" OR
           _id STREQUAL "c_api.gamer.guide_dispatcher" OR
           _id STREQUAL "gamer_services.guide.notification_position")
            set_tests_properties(${_name} PROPERTIES RUN_SERIAL TRUE)
        endif()
        # A demo that documents a recorded CNA defect is expected to fail; if it starts passing,
        # the test fails so the stale known-issue marker gets noticed.
        if(NOT _known_issue STREQUAL "-")
            set_tests_properties(${_name} PROPERTIES WILL_FAIL TRUE)
        endif()
    endforeach()
endforeach()
