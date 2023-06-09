include(OB/Test)

ob_add_basic_standard_test(
    TARGET_PREFIX "${TESTS_TARGET_PREFIX}"
    TARGET_VAR test_target
    LINKS
        ${TESTS_COMMON_TARGET}
)

# Bundle test data
file(GLOB test_data
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "data/*.*"
)

qt_add_resources(${test_target} "tst_consistent_rng_data"
    PREFIX "/"
    FILES
        ${test_data}
)
