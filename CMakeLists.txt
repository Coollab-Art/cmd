cmake_minimum_required(VERSION 3.8)

add_library(cmd INTERFACE)
add_library(cmd::cmd ALIAS cmd)
target_compile_features(cmd INTERFACE cxx_std_20)

target_include_directories(cmd INTERFACE include)

# if (MSVC)
#     target_compile_options(cmd PRIVATE /WX /W4)
# else()
#     target_compile_options(cmd PRIVATE -Werror -Wall -Wextra -Wpedantic -pedantic-errors)
# endif()