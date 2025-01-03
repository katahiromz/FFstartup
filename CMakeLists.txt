# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(FFstartup CXX RC)

# FFstartup.exe
add_executable(FFstartup FFstartup.cpp FFstartup_res.rc)
target_compile_definitions(FFstartup PRIVATE UNICODE _UNICODE)

##############################################################################
