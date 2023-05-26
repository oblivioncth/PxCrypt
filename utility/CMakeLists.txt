#================= Common Build =========================

# Set custom output name
if(WIN32)
    set(app_output_name "${PROJECT_NAMESPACE}")
else()
    set(app_output_name "${PROJECT_NAMESPACE_LC}")
endif()

# Add via ob standard executable
include(OB/Executable)
ob_add_standard_executable(${APP_TARGET_NAME}
    NAMESPACE "${PROJECT_NAMESPACE}"
    ALIAS "${APP_ALIAS_NAME}"
    OUTPUT_NAME "${app_output_name}"
    SOURCE
        main.cpp
    LINKS
        PRIVATE
            pxcrypt::Codec
            Qx::Core
    CONFIG STANDARD
)
