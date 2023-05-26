#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.24.0...3.25.0)

# Project
# NOTE: DON'T USE TRAILING ZEROS IN VERSIONS
project(pxcrypt
    VERSION 0.1
    LANGUAGES CXX
    DESCRIPTION "Weave arbitrary data into an image in an encrypted manner"
)

# Get helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchOBCMake.cmake)
fetch_ob_cmake("v0.3")

# Initialize project according to standard rules
include(OB/Project)
ob_standard_project_setup()

# Configuration options
# Handled by fetched libs, but set this here formally since they aren't part of the main project
option(BUILD_SHARED_LIBS "Build pxcrypt with shared libraries" OFF)

# C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build augmentation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    enable_language("RC")
endif()

#================= Build =========================

# Import Qt
set(PXCRYPT_QT_COMPONENTS
    Core
)

# Find Qt package
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
find_package(Qt6 REQUIRED COMPONENTS ${PXCRYPT_QT_COMPONENTS})

# Fetch Qx (build and import from source)
set(PXCRYPT_QX_COMPONENTS
    Core
)

include(OB/FetchQx)
ob_fetch_qx(
    REF "v0.5.0.1"
    COMPONENTS
        ${PXCRYPT_QX_COMPONENTS}
)

# Process Targets
set(LIB_TARGET_NAME ${PROJECT_NAMESPACE_LC}_codec)
set(LIB_ALIAS_NAME Codec)
string(TOLOWER "${LIB_ALIAS_NAME}" LIB_ALIAS_NAME_LC)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_subdirectory("${LIB_PATH}")

set(APP_TARGET_NAME ${PROJECT_NAMESPACE_LC}_utility)
set(APP_ALIAS_NAME Utility)
add_subdirectory(utility)

#--------------------Package Config-----------------------

ob_standard_project_package_config(
    COMPATIBILITY "SameMinorVersion"
    CONFIG STANDARD
        TARGET_CONFIGS
            TARGET "${PROJECT_NAMESPACE}::${LIB_ALIAS_NAME}" COMPONENT "${LIB_ALIAS_NAME}" DEFAULT
            TARGET "${PROJECT_NAMESPACE}::${APP_ALIAS_NAME}" COMPONENT "${APP_ALIAS_NAME}"
        DEPENDS
            PACKAGE "Qt6" COMPONENTS ${PXCRYPT_QT_COMPONENTS}
            PACKAGE "Qx" VERSION ${Qx_VERSION} COMPONENTS ${PXCRYPT_QX_COMPONENTS}
)

#================= Install ==========================

ob_standard_project_misc_install()

#====================== CPack ==============================

include(OB/Packaging)
ob_standard_project_package(VENDOR "oblivioncth")

