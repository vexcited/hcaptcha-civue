cmake_minimum_required(VERSION 3.29)

# c2y and the defer TS take clang 22, newer than the one Xcode ships.
if(APPLE AND NOT CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
  find_program(CMAKE_C_COMPILER clang
    PATHS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin NO_DEFAULT_PATH)
  set(CMAKE_OBJC_COMPILER ${CMAKE_C_COMPILER} CACHE FILEPATH "objc compiler")
endif()

project(demo VERSION 0.0.0 LANGUAGES C)

# this checkout when the example is in it, otherwise it is fetched.
include(FetchContent)
get_filename_component(local ${CMAKE_CURRENT_SOURCE_DIR}/../.. ABSOLUTE)
if(NOT FETCHCONTENT_SOURCE_DIR_HCAPTCHA-CIVUE AND EXISTS ${local}/CMakeLists.txt)
  set(FETCHCONTENT_SOURCE_DIR_HCAPTCHA-CIVUE ${local})
endif()
if(NOT TARGET hcaptcha::civue)
  FetchContent_Declare(hcaptcha-civue
    GIT_REPOSITORY https://code.vexcited.com/hcaptcha/civue.git
    GIT_TAG main)
  FetchContent_MakeAvailable(hcaptcha-civue)
endif()

add_executable(demo src/main.c)
target_link_libraries(demo PRIVATE hcaptcha::civue)
target_compile_options(demo PRIVATE -Wall -Wextra)
