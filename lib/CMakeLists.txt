# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER
        PATH "${PROJECT_NAMESPACE_LC}/${PROJECT_NAMESPACE_LC}_${LIB_ALIAS_NAME_LC}_export.h"
#    HEADERS_PRIVATE

    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            dummy.h
    IMPLEMENTATION
        dummy.cpp
    LINKS
        PRIVATE
            Qx::Core
        PUBLIC
            Qt6::Core
    CONFIG STANDARD
)
