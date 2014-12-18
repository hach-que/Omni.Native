cmake_minimum_required(VERSION 2.6)
project(omni_php_extensions)

set(CMAKE_MODULE_PATH "${toolchain_SOURCE_DIR}/CMakeScripts")

# Locate Bison.
if(NOT DEFINED FLEX_EXECUTABLE)
    find_program(FLEX_EXECUTABLE NAMES flex.exe flex)
endif()
if(NOT DEFINED BISON_EXECUTABLE)
    find_program(BISON_EXECUTABLE NAMES bison.exe bison)
endif()

# Show configuration.
message("-- Showing configuration")
message("  >> bison is at: ${BISON_EXECUTABLE}")

# Speed up build with cotire.
if(MSVC)
    message("  >> compiler is MSVC -> NOT using cotire")
    function(cotire proj)
    endfunction()
else()
    # use cotire only when the version is 2.8.6. or greater
    if (CMAKE_VERSION VERSION_GREATER 2.8.5)
        if(${USE_COTIRE})
            message("  >> cmake version is >= 2.8.6 -> using cotire")
            include(cotire)
        else()
            function(cotire proj)
            endfunction()
        endif()
    else()
        message("  >> cmake version is < 2.8.6 -> NOT using cotire")
        function(cotire proj)
        endfunction()
    endif()
endif()

include_directories(
  /usr/include/php5/main
  /usr/include/php5/Zend
  /usr/include/php5/TSRM
  /usr/include/php5/sapi
  /usr/include/php5/ext
  /usr/include/php5/include
  /usr/include/php5
  /usr/include/readline
)

# Find Bison and Flex
find_package(FLEX)
find_package(BISON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(third-party/bstring)
add_subdirectory(third-party/simclist)
add_subdirectory(third-party/libancillary)
add_subdirectory(common)
add_subdirectory(omnilang)
add_subdirectory(omni)
add_subdirectory(editline)
add_subdirectory(fd)
add_subdirectory(tc)