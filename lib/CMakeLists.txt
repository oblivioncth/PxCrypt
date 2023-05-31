# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER
        PATH "${PROJECT_NAMESPACE_LC}/${PROJECT_NAMESPACE_LC}_${LIB_ALIAS_NAME_LC}_export.h"
    HEADERS_PRIVATE
        encdec_p.h
        tools/bit_chunker.h
        tools/byte_compositer.h
        tools/px_sequence_generator.h
        tools/px_skimmer.h
        tools/px_weaver.h
    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            decode.h
            encdec.h
            encode.h
    IMPLEMENTATION
        decode.cpp
        encode.cpp
        encdec_p.cpp
        tools/bit_chunker.cpp
        tools/byte_compositer.cpp
        tools/px_sequence_generator.cpp
        tools/px_skimmer.cpp
        tools/px_weaver.cpp
    LINKS
        PRIVATE
            Qx::Core
        PUBLIC
            Qt6::Core
            Qt6::Gui
    CONFIG STANDARD
)
