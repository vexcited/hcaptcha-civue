cmake_minimum_required(VERSION 3.29)

# c2y and the defer TS take clang 22, newer than the one Xcode ships.
if(APPLE AND NOT CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
  find_program(CMAKE_C_COMPILER clang
    PATHS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin NO_DEFAULT_PATH)
  set(CMAKE_OBJC_COMPILER ${CMAKE_C_COMPILER} CACHE FILEPATH "objc compiler")
endif()

project(demo VERSION 0.1.0 LANGUAGES C)

# it brings civue with it.
include(FetchContent)
if(NOT TARGET hcaptcha::civue)
  FetchContent_Declare(hcaptcha-civue
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
  FetchContent_MakeAvailable(hcaptcha-civue)
endif()

add_executable(demo src/main.c)
target_link_libraries(demo PRIVATE hcaptcha::civue)
target_compile_options(demo PRIVATE -Wall -Wextra)
