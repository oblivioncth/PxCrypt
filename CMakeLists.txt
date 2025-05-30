#================= Project Setup ==========================

# CMake
cmake_minimum_required(VERSION 3.23.0...3.26.0)

# Project
# NOTE: DON'T USE TRAILING ZEROS IN VERSIONS
project(PxCrypt
    VERSION 0.2.2
    LANGUAGES CXX
    DESCRIPTION "Encrypt data within an image"
)
set(PROJECT_NAME_FORMAL ${PROJECT_NAME})

# Get helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchOBCMake.cmake)
fetch_ob_cmake("v0.3.9")

# Initialize project according to standard rules
include(OB/Project)
ob_standard_project_setup()

# Configuration options
option(${PROJECT_NAME_UC}_TESTS "Build ${PROJECT_NAME} tests" OFF)
option(${PROJECT_NAME_UC}_DOCS "Build ${PROJECT_NAME} documentation" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries." OFF) # Redundant due to OB, but explicit

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
    Gui
    Concurrent
)

if(${PROJECT_NAME_UC}_TESTS)
    list(APPEND PXCRYPT_QT_COMPONENTS Test)
endif()

# Find Qt package
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)
include(OB/Qt)
ob_find_package_qt(REQUIRED COMPONENTS ${PXCRYPT_QT_COMPONENTS})

# Fetch Qx (build and import from source)
set(PXCRYPT_QX_COMPONENTS
    Core
    Io
)

include(OB/FetchQx)
ob_fetch_qx(
    REF "226b2d53da729b81e9dfaacfa37d0a9ab3328b67"
    COMPONENTS
        ${PXCRYPT_QX_COMPONENTS}
)

# Fetch Neargye's Magic Enum
include(OB/FetchMagicEnum)
ob_fetch_magicenum("v0.9.3")

# Process Targets
set(LIB_TARGET_NAME ${PROJECT_NAMESPACE_LC}_codec)
set(LIB_ALIAS_NAME Codec)
string(TOLOWER "${LIB_ALIAS_NAME}" LIB_ALIAS_NAME_LC)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_subdirectory("${LIB_PATH}")

set(APP_TARGET_NAME ${PROJECT_NAMESPACE_LC}_utility)
set(APP_ALIAS_NAME Utility)
add_subdirectory(utility)

if(${PROJECT_NAME_UC}_DOCS)
    set(DOC_TARGET_NAME ${PROJECT_NAMESPACE_LC}_docs)
    add_subdirectory(doc)
endif()

if(${PROJECT_NAME_UC}_TESTS)
    enable_testing()
    set(TESTS_TARGET_PREFIX ${PROJECT_NAMESPACE_LC})
    set(TESTS_COMMON_TARGET "${TESTS_TARGET_PREFIX}_tst_common")
    add_subdirectory(test)
endif()

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

