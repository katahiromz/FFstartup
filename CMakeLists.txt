# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(FFstartup CXX RC)

# UTF-8 source code support
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

# JAPAN or not?
option(JAPAN "Enable Japanese support" ON)
if(JAPAN)
    add_definitions(-DJAPAN)
endif()

# FFstartup.exe
add_executable(FFstartup FFstartup.cpp FFstartup_res.rc)
target_compile_definitions(FFstartup PRIVATE UNICODE _UNICODE)
target_link_libraries(FFstartup comctl32 shlwapi)

# Don't add default manifest
if(MSVC)
    target_link_options(FFstartup PRIVATE /MANIFEST:NO)
endif()

##############################################################################
