file(GLOB_RECURSE TEST_FILES "*.cpp")
add_executable(${TEST_TARGET} ${TEST_FILES})
if (BUILD_TESTING)
    target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest_main)
    target_include_directories(${TEST_TARGET} PRIVATE )
    include(GoogleTest)
    gtest_discover_tests(${TEST_TARGET})
endif()
