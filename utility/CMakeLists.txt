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
        command/c-encode.h
        command/c-encode.cpp
        command/c-decode.h
        command/c-decode.cpp
        command/command.h
        command/command.cpp
        kernel/core.h
        kernel/core.cpp
        kernel/errorcode.h
        main.cpp
    LINKS
        PRIVATE
            PxCrypt::Codec
            Qx::Core
            Qx::Io
            magic_enum::magic_enum
    CONFIG STANDARD
)

## Forward select project variables to C++ code
include(OB/CppVars)
ob_add_cpp_vars(${APP_TARGET_NAME}
    NAME "project_vars"
    PREFIX "PROJECT_"
    VARS
        VERSION_STR "\"${PROJECT_VERSION}\""
        SHORT_NAME "\"${PROJECT_NAME}\""
)
