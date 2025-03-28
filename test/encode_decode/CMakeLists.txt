include(OB/Test)

ob_add_basic_standard_test(
    TARGET_PREFIX "${TESTS_TARGET_PREFIX}"
    TARGET_VAR test_target
    LINKS
        PRIVATE
            ${TESTS_COMMON_TARGET}
)

qt_add_resources(${test_target} "tst_encode_decode_data"
    PREFIX "/"
    FILES
        "data/real_world_image.jpg"
)
