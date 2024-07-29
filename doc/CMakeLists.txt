# Include standard documentation target creator
include(OB/Documentation)

# Set project specific doxygen overrides
set(DOXYGEN_HTML_COLORSTYLE_HUE 102)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 70)
set(DOXYGEN_EXCLUDE_SYMBOLS
    "PxCryptPrivate::*"
) # Reduces 'cond' requirement
#TODO: Improve the above to futher reduce the need of "@cond", though
# this likely requires looking at Doxygen's source as patterns that
# seem they should work don't
set(DOXYGEN_PREDEFINED
    "PXCRYPT_CODEC_EXPORT="
)

ob_standard_documentation(${DOC_TARGET_NAME}
    DOXY_VER 1.9.7
    PROJ_NAME "${PROJECT_NAME_FORMAL}"
    QT_PREFIX "${Qt6_PREFIX_PATH}"
    QT_MODULES
        qtcore
    INPUT_LIST
        "${LIB_PATH}/src"
        "${LIB_PATH}/include"
)
