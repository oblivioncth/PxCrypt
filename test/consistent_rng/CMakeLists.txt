include(OB/Test)

ob_add_basic_standard_test(
    TARGET_PREFIX "${TESTS_TARGET_PREFIX}"
    TARGET_VAR test_target
    LINKS
        PRIVATE
            ${TESTS_COMMON_TARGET}
)

# Bundle test data
file(GLOB encoded_images
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "data/encoded_*.png"
)

qt_add_resources(${test_target} "tst_consistent_rng_data"
    PREFIX "/"
    FILES
        ${encoded_images}
        "data/payload.bin"
)
