cmake_minimum_required(VERSION 3.29)
project(DSAproject3)

set(CMAKE_CXX_STANDARD 17)


add_executable(DSAproject3 main.cpp
        graph.cpp
        data.cpp
        api.cpp
        api.h)

# Include directories
target_include_directories(DSAproject3 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}  # Include the current directory for json.hpp
)

# Add compiler flags
if(MSVC)
    target_compile_options(DSAproject3 PRIVATE /W4)
else()
    target_compile_options(DSAproject3 PRIVATE -Wall -Wextra)
endif()
