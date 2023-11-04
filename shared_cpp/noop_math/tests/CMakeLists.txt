include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding parent project's compiler/linker settings
FetchContent_MakeAvailable(googletest)

enable_testing() #Enables testing for this directory and below.

set(NOOP_MATH_LIBS noop_math gtest_main)

# noop_math_test
add_executable(
        noop_math_test
        noop_math_test.cpp
)
target_link_libraries(noop_math_test ${NOOP_MATH_LIBS})
target_include_directories(noop_math_test PRIVATE ${SHARED_CPP} ${SHARED_CPP}/noop_math)

include(GoogleTest)
gtest_discover_tests(
        noop_math_test
)