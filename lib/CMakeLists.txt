# Add via ob standard library
include(OB/Library)
ob_add_standard_library(${LIB_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${LIB_ALIAS_NAME}"
    EXPORT_HEADER
        PATH "${PROJECT_NAMESPACE_LC}/${PROJECT_NAMESPACE_LC}_${LIB_ALIAS_NAME_LC}_export.h"
    HEADERS_API
        COMMON "${PROJECT_NAMESPACE_LC}"
        FILES
            stat.h
            codec/decoder.h
            codec/encoder.h
            codec/multi_decoder.h
            codec/multi_encoder.h
            codec/standard_decoder.h
            codec/standard_encoder.h
    IMPLEMENTATION
        stat.cpp
        art_io/artwork.h
        art_io/artwork.cpp
        art_io/artwork_error.h
        art_io/artwork_error.cpp
        art_io/measure.h
        art_io/measure.cpp
        art_io/works/standard.h
        art_io/works/standard.cpp
        art_io/works/multipart.h
        art_io/works/multipart.cpp
        codec/decoder.cpp
        codec/decoder_p.h
        codec/encdec.h
        codec/encdec.cpp
        codec/encoder.cpp
        codec/encoder_p.h
        codec/multi_decoder.cpp
        codec/multi_encoder.cpp
        codec/standard_decoder.cpp
        codec/standard_encoder.cpp
        medium_io/canvas.h
        medium_io/canvas.cpp
        medium_io/operate/data_translator.h
        medium_io/operate/data_translator.cpp
        medium_io/operate/meta_access.h
        medium_io/operate/meta_access.cpp
        medium_io/operate/px_access.h
        medium_io/operate/px_access.cpp
        medium_io/sequence/ch_sequence_generator.h
        medium_io/sequence/ch_sequence_generator.cpp
        medium_io/sequence/px_sequence_generator.h
        medium_io/sequence/px_sequence_generator.cpp
        medium_io/traverse/canvas_traverser.h
        medium_io/traverse/canvas_traverser.cpp
        medium_io/traverse/canvas_traverser_prime.h
        medium_io/traverse/canvas_traverser_prime.cpp
    LINKS
        PRIVATE
            $<BUILD_INTERFACE:magic_enum::magic_enum>
        PUBLIC
            ${Qt}::Core
            ${Qt}::Gui
            ${Qt}::Concurrent
            Qx::Core
    CONFIG STANDARD
)

# Build interface wrapper for magic_enum is a workaround for a CMake limitation.
# See https://github.com/oblivioncth/STARpp/blob/500d19354fe394ba5a1c84e56208d2186866c5e8/lib/CMakeLists.txt
