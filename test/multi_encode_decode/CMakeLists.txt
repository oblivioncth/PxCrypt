include(OB/Test)

ob_add_basic_standard_test(
    TARGET_PREFIX "${TESTS_TARGET_PREFIX}"
    LINKS
        PRIVATE
            ${TESTS_COMMON_TARGET}
)