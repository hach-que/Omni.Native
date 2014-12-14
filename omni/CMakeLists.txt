cmake_minimum_required(VERSION 2.6)
project(php_omni)

include_directories(
  ${simclist_SOURCE_DIR}
  ${bstring_SOURCE_DIR}
  ${common_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(php_omni SHARED omni.c)
