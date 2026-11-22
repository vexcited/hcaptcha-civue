cmake_minimum_required(VERSION 3.29)
project(hcaptcha-civue VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ${PROJECT_IS_TOP_LEVEL})

if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_CONFIGURATION_TYPES
  AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif()

option(HCAPTCHA_BUILD_EXAMPLES "build the hcaptcha-civue examples"
  ${PROJECT_IS_TOP_LEVEL})

#   FetchContent_Declare(civue
#     GIT_REPOSITORY https://code.vexcited.com/ciko/civue.git
#     GIT_TAG main)
include(FetchContent)
if(NOT TARGET civue::civue)
  if(NOT FETCHCONTENT_SOURCE_DIR_CILO
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-cilo/CMakeLists.txt)
    set(FETCHCONTENT_SOURCE_DIR_CILO ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-cilo
      CACHE PATH "the cilo checkout")
  endif()

  FetchContent_Declare(civue
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ciko-civue)
  FetchContent_MakeAvailable(civue)
endif()

add_library(hcaptcha-civue STATIC src/hcaptcha.c src/page.c)
add_library(hcaptcha::civue ALIAS hcaptcha-civue)

target_include_directories(hcaptcha-civue PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(hcaptcha-civue PUBLIC c_std_23)
target_link_libraries(hcaptcha-civue PUBLIC civue::civue)
target_compile_options(hcaptcha-civue PRIVATE -Wall -Wextra)
set_target_properties(hcaptcha-civue PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(HCAPTCHA_BUILD_EXAMPLES)
  add_subdirectory(examples/demo)
endif()
