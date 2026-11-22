cmake_minimum_required(VERSION 3.29)
project(demo VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

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
