# This threading example uses C11; the other C ABI examples retain their C99 baseline.
if(TARGET CNA::CApi)
    find_package(Threads QUIET)
    if(Threads_FOUND)
        include(CheckCSourceCompiles)
        include(CMakePushCheckState)
        cmake_push_check_state(RESET)
        set(CMAKE_C_STANDARD 11)
        set(CMAKE_C_STANDARD_REQUIRED ON)
        set(CMAKE_C_EXTENSIONS OFF)
        set(CMAKE_REQUIRED_LIBRARIES Threads::Threads)
        check_c_source_compiles("
            #include <stdatomic.h>
            #include <threads.h>
            static int worker(void* state) { return atomic_load((atomic_int*)state); }
            int main(void) {
                atomic_int phase; atomic_init(&phase, 0);
                thrd_t thread; int result;
                if (thrd_create(&thread, worker, &phase) != thrd_success) return 1;
                const struct timespec pause = {0, 1};
                (void)thrd_sleep(&pause, 0);
                return thrd_join(thread, &result);
            }" CNA_DEMOS_HAVE_C11_THREADS)
        cmake_pop_check_state()
    endif()
    if(Threads_FOUND AND CNA_DEMOS_HAVE_C11_THREADS)
        cna_demos_add_standalone(SOURCES main.c C_API LIBRARIES Threads::Threads)
        set_target_properties(cna-demo-c_api-core-thread_local_errors PROPERTIES C_STANDARD 11)
    else()
        message(STATUS "cna-demos: thread-local diagnostics tool omitted (C11 threads/atomics unavailable)")
    endif()
endif()
