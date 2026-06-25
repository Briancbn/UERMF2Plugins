cmake_minimum_required(VERSION 3.15)

project(
    rmf2-unreal-extern
    VERSION 0.0.0
    LANGUAGES CXX C
)

# if (DEFINED ENV{UNREAL_ENGINE_ROOT} AND NOT UNREAL_ENGINE_ROOT)
#   set(UNREAL_ENGINE_ROOT "$ENV{UNREAL_ENGINE_ROOT}")
# endif()
#
# if (NOT UNREAL_ENGINE_ROOT)
#   # On Windows and Mac, try to use the default UE 5.3 location
#   if (WIN32)
#      set(DEFAULT_UNREAL_INSTALLATION "C:/Program Files/Epic Games/UE_5.3")
#   elseif(APPLE)
#     set(DEFAULT_UNREAL_INSTALLATION "/Users/Shared/Epic Games/UE_5.3")
#   endif()
#
#   if (NOT EXISTS "${DEFAULT_UNREAL_INSTALLATION}")
#      message(FATAL_ERROR "Please specify the root of your Unreal Engine installation, either by setting the UNREAL_ENGINE_ROOT environment variable or with -DUNREAL_ENGINE_ROOT=path on the cmake command-line.")
#   else()
#     set(UNREAL_ENGINE_ROOT "${DEFAULT_UNREAL_INSTALLATION}")
#   endif()
# endif()
#
# message(STATUS "Using Unreal Engine installation at: ${UNREAL_ENGINE_ROOT}")
#
# # Set the UE environment variable so it's inherited by vcpkg builds.
# set(ENV{UNREAL_ENGINE_ROOT} "${UNREAL_ENGINE_ROOT}")

## TODO(anyone): Android, IOS build

# Set default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/../ThirdParty)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})

# download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 8.1.1
  OPTIONS
    "BUILD_SHARED_LIBS ON"
    "FMT_INSTALL ON"
)

CPMAddPackage(
  NAME paho_mqtt_c
  GITHUB_REPOSITORY eclipse-paho/paho.mqtt.c
  GIT_TAG v1.3.9
  OPTIONS
    "PAHO_BUILD_SHARED ON"
    "PAHO_WITH_SSL ON"
    "PAHO_ENABLE_TESTING OFF"
    "PAHO_ENABLE_CPACK OFF"
)

CPMAddPackage(
  NAME paho_mqtt_cpp
  GITHUB_REPOSITORY eclipse-paho/paho.mqtt.cpp
  GIT_TAG v1.5.1
  OPTIONS
    "PAHO_BUILD_SHARED ON"
    "PAHO_WITH_SSL ON"
)

CPMAddPackage(
  NAME json
  GITHUB_REPOSITORY nlohmann/json
  GIT_TAG v3.10.5
  OPTIONS
    "JSON_Install ON"
)

CPMAddPackage(
  NAME vda5050_core
  GITHUB_REPOSITORY Briancbn/vda5050_client
  SOURCE_SUBDIR vda5050_core
  GIT_TAG d321896afe294e81b045a381aafcd3b4fa7e1fc0
  OPTIONS
    "BUILD_TESTING OFF"
    "BUILD_EXAMPLES OFF"
    "EXCLUDE_FROM_ALL ON"
)
