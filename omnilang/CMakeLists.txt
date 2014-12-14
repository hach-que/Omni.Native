cmake_minimum_required(VERSION 2.6)
project(php_omnilang)

bison_target(omnilang-parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c COMPILE_FLAGS "-d -r all --report-file=${CMAKE_CURRENT_SOURCE_DIR}/parser.output")
flex_target(omnilang-lexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c COMPILE_FLAGS \"--header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.h\")
add_flex_bison_dependency(omnilang-lexer omnilang-parser)
if(NOT MSVC)
  set_source_files_properties(${FLEX_omnilang-lexer_OUTPUTS} ${BISON_omnilang-parser_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-implicit-function-declaration)
endif()

add_library(php_omnilang SHARED
  ast.c
  omnilang.c
  ${FLEX_omnilang-lexer_OUTPUTS}
  ${BISON_omnilang-parser_OUTPUTS}
  parser.y
  lexer.l
)

include_directories(
  ${simclist_SOURCE_DIR}
  ${bstring_SOURCE_DIR}
  ${common_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(php_omnilang
  bstring
  simclist
  common
)
  