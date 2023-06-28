# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER
        PATH "${PROJECT_NAMESPACE_LC}/${PROJECT_NAMESPACE_LC}_${LIB_ALIAS_NAME_LC}_export.h"
    HEADERS_PRIVATE
        encdec.h
        tools/bit_chunker.h
        tools/byte_compositer.h
        tools/ch_sequence_generator.h
        tools/px_access_read.h
        tools/px_access_write.h
        tools/px_sequence_generator.h
        tools/px_skimmer.h
        tools/px_weaver.h
    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            _internal.h
            decoder.h
            encoder.h
    IMPLEMENTATION
        _internal.cpp
        decoder.cpp
        encoder.cpp
        encdec.cpp
        tools/bit_chunker.cpp
        tools/byte_compositer.cpp
        tools/ch_sequence_generator.cpp
        tools/px_access_read.cpp
        tools/px_access_write.cpp
        tools/px_sequence_generator.cpp
        tools/px_skimmer.cpp
        tools/px_weaver.cpp
    LINKS
        PRIVATE
            $<BUILD_INTERFACE:magic_enum::magic_enum>
        PUBLIC
            Qt6::Core
            Qt6::Gui
            Qx::Core
    CONFIG STANDARD
)

# Build interface wrapper for magic_enum is a workaround for a CMake limitation.
# See https://github.com/oblivioncth/STARpp/blob/500d19354fe394ba5a1c84e56208d2186866c5e8/lib/CMakeLists.txt
