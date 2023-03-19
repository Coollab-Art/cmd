cmake_minimum_required(VERSION 3.8)

set(WARNINGS_AS_ERRORS_FOR_CMD OFF CACHE BOOL "ON iff you want to treat warnings as errors")

add_library(cmd INTERFACE)
add_library(cmd::cmd ALIAS cmd)
target_compile_features(cmd INTERFACE cxx_std_20)

if(WARNINGS_AS_ERRORS_FOR_CMD)
    target_include_directories(cmd INTERFACE include)
else()
    target_include_directories(cmd SYSTEM INTERFACE include)
endif()