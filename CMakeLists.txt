cmake_minimum_required (VERSION 2.8)

project(paradox_voice)
set(PROJECT_NAME paradox_voice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DALT_CLIENT)
add_definitions(-DALT_CLIENT_API)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.hpp" "src/*.cpp" "include/*.h" "include/*.hpp" "include/*.cpp")

macro(GroupSources curdir groupindex)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)

  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      GroupSources(${curdir}/${child} ${groupindex}/${child})
    else()

      string(REPLACE "/" "\\" groupname ${groupindex})

      source_group(${groupname} FILES ${curdir}/${child})
    endif()
  endforeach()
endmacro()

GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")
GroupSources(${PROJECT_SOURCE_DIR}/include "Include Files")

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wno-deprecated-declarations
    -Wno-switch-enum
    -Wno-unused-command-line-argument
    -Wno-macro-redefined
    -Wno-inconsistent-missing-override
    -Wno-deprecated-declarations
    -Wno-return-type
    -Wno-switch-enum
    -Wno-switch
    -Wno-parentheses
    -Wno-unused-private-field
    -Wno-unused-variable
    -Wno-format
    -Wno-reorder
    -Wno-delete-non-virtual-dtor
    -Wno-microsoft-cast
    -Wno-unused-function
    -Wno-unused-lambda-capture
    -Wno-missing-braces
    -Wno-unused-local-typedef
    -Wno-tautological-constant-out-of-range-compare
    -Wno-delete-incomplete
    -Wno-c++11-narrowing
    -Wno-comment
  )

    target_compile_options(${PROJECT_NAME} PRIVATE
      -fms-compatibility
      -fms-extensions
      /EHsc
	  /FORCE
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
      COTIRE_ENABLE_PRECOMPILED_HEADER FALSE
      COTIRE_ADD_UNITY_BUILD FALSE
    )
endif()

#link_directories(
#  C:/MinHook/lib
#  $ENV{DXSDK_DIR}/Lib/x64
#)

include (CMakeParseArguments)
set (GenerateProductVersionCurrentDir ${CMAKE_CURRENT_LIST_DIR})

function(generate_product_version outfiles)
    set (options)
    set (oneValueArgs
        NAME
        BUNDLE
        ICON
        VERSION_MAJOR
        VERSION_MINOR
        VERSION_PATCH
        VERSION_REVISION
        COMPANY_NAME
        COMPANY_COPYRIGHT
        COMMENTS
        ORIGINAL_FILENAME
        INTERNAL_NAME
        FILE_DESCRIPTION)
    set (multiValueArgs)
    cmake_parse_arguments(PRODUCT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT PRODUCT_BUNDLE OR "${PRODUCT_BUNDLE}" STREQUAL "")
        set(PRODUCT_BUNDLE "${PRODUCT_NAME}")
    endif()

    if (NOT PRODUCT_VERSION_MAJOR EQUAL 0 AND (NOT PRODUCT_VERSION_MAJOR OR "${PRODUCT_VERSION_MAJOR}" STREQUAL ""))
        set(PRODUCT_VERSION_MAJOR 1)
    endif()
    if (NOT PRODUCT_VERSION_MINOR EQUAL 0 AND (NOT PRODUCT_VERSION_MINOR OR "${PRODUCT_VERSION_MINOR}" STREQUAL ""))
        set(PRODUCT_VERSION_MINOR 0)
    endif()
    if (NOT PRODUCT_VERSION_PATCH EQUAL 0 AND (NOT PRODUCT_VERSION_PATCH OR "${PRODUCT_VERSION_PATCH}" STREQUAL ""))
        set(PRODUCT_VERSION_PATCH 0)
    endif()
    if (NOT PRODUCT_VERSION_REVISION EQUAL 0 AND (NOT PRODUCT_VERSION_REVISION OR "${PRODUCT_VERSION_REVISION}" STREQUAL ""))
        set(PRODUCT_VERSION_REVISION 0)
    endif()

    if (NOT PRODUCT_COMPANY_COPYRIGHT OR "${PRODUCT_COMPANY_COPYRIGHT}" STREQUAL "")
        string(TIMESTAMP PRODUCT_CURRENT_YEAR "%Y")
        set(PRODUCT_COMPANY_COPYRIGHT "${PRODUCT_COMPANY_NAME} (C) Copyright ${PRODUCT_CURRENT_YEAR}")
    endif()
    if (NOT PRODUCT_COMMENTS OR "${PRODUCT_COMMENTS}" STREQUAL "")
        set(PRODUCT_COMMENTS "${PRODUCT_NAME} v${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}")
    endif()
    if (NOT PRODUCT_ORIGINAL_FILENAME OR "${PRODUCT_ORIGINAL_FILENAME}" STREQUAL "")
        set(PRODUCT_ORIGINAL_FILENAME "${PRODUCT_NAME}")
    endif()
    if (NOT PRODUCT_INTERNAL_NAME OR "${PRODUCT_INTERNAL_NAME}" STREQUAL "")
        set(PRODUCT_INTERNAL_NAME "${PRODUCT_NAME}")
    endif()
    if (NOT PRODUCT_FILE_DESCRIPTION OR "${PRODUCT_FILE_DESCRIPTION}" STREQUAL "")
        set(PRODUCT_FILE_DESCRIPTION "${PRODUCT_NAME}")
    endif()

    set (_VersionInfoFile ${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.h)
    set (_VersionResourceFile ${CMAKE_CURRENT_BINARY_DIR}/VersionResource.rc)
    configure_file(
        ${GenerateProductVersionCurrentDir}/VersionInfo.in
        ${_VersionInfoFile}
        @ONLY)
    configure_file(
        ${GenerateProductVersionCurrentDir}/VersionResource.rc
        ${_VersionResourceFile}
        COPYONLY)
    list(APPEND ${outfiles} ${_VersionInfoFile} ${_VersionResourceFile})
    set (${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

generate_product_version(ProductVersionFiles
    NAME "PARADOX Voice"
    VERSION_MAJOR 1
    VERSION_MINOR 0
    VERSION_PATCH 0
    VERSION_REVISION 0
    COMPANY_NAME "PARADOX INTERNATIONAL"
	COMPANY_COPYRIGHT "Copyright 2021 (C) Nova & Captcha."
)

add_library(
  ${PROJECT_NAME} SHARED
  ${PROJECT_SOURCE_FILES}
  ${ProductVersionFiles}
)

if (WIN32)
  target_link_libraries(${PROJECT_NAME} wsock32.lib ws2_32.lib)
endif (WIN32)