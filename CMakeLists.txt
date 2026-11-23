cmake_minimum_required(VERSION 3.29)

# c2y and the defer TS take clang 22, newer than the one Xcode ships.
if(APPLE AND NOT CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
  find_program(CMAKE_C_COMPILER clang
    PATHS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin NO_DEFAULT_PATH)
  set(CMAKE_OBJC_COMPILER ${CMAKE_C_COMPILER} CACHE FILEPATH "objc compiler")
endif()

project(hcaptcha-civue VERSION 0.1.0 LANGUAGES C)

# cmake knows no standard past C23, so the flags are given by hand.
set(HCAPTCHA_C2Y -std=gnu2y -fdefer-ts)
include(CheckCCompilerFlag)
check_c_compiler_flag(-fdefer-ts HCAPTCHA_HAS_DEFER)
if(NOT HCAPTCHA_HAS_DEFER)
  message(FATAL_ERROR
    "hcaptcha-civue is C2y with the defer TS, set CC to clang 22 or later")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ${PROJECT_IS_TOP_LEVEL})

if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_CONFIGURATION_TYPES
  AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif()

option(HCAPTCHA_BUILD_EXAMPLES "build the examples"
  ${PROJECT_IS_TOP_LEVEL})

include(FetchContent)
if(NOT TARGET civue::civue)
  if(NOT FETCHCONTENT_SOURCE_DIR_CIVUE
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-civue/CMakeLists.txt)
    set(FETCHCONTENT_SOURCE_DIR_CIVUE ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-civue
      CACHE PATH "civue checkout")
  endif()

  if(NOT FETCHCONTENT_SOURCE_DIR_CILO
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-cilo/CMakeLists.txt)
    set(FETCHCONTENT_SOURCE_DIR_CILO ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-cilo
      CACHE PATH "cilo checkout")
  endif()

  FetchContent_Declare(civue
    GIT_REPOSITORY https://code.vexcited.com/ciko/civue.git
    GIT_TAG main)
  FetchContent_MakeAvailable(civue)
endif()

add_library(hcaptcha-civue STATIC src/hcaptcha.c src/page.c)
add_library(hcaptcha::civue ALIAS hcaptcha-civue)

target_include_directories(hcaptcha-civue PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_options(hcaptcha-civue PUBLIC
  "$<$<COMPILE_LANGUAGE:C,OBJC>:${HCAPTCHA_C2Y}>")
target_link_libraries(hcaptcha-civue PUBLIC civue::civue)
target_compile_options(hcaptcha-civue PRIVATE -Wall -Wextra)
set_target_properties(hcaptcha-civue PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(HCAPTCHA_BUILD_EXAMPLES)
  add_subdirectory(examples/demo)
endif()
