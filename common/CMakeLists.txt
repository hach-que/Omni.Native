cmake_minimum_required(VERSION 2.6)
project(common)

include_directories(
  ${bstring_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(common STATIC
  bstring.h
  bstring.c
)