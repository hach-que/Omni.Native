cmake_minimum_required(VERSION 2.6)
project(php_omni)

include_directories(
  /usr/include/php5/main
  /usr/include/php5/Zend
  /usr/include/php5/TSRM
  /usr/include/php5/sapi
  /usr/include/php5/ext
  /usr/include/php5/include
  /usr/include/php5
)
  
add_library(php_omni SHARED terminal.c)
