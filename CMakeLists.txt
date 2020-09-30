cmake_minimum_required(VERSION 2.8.3)

project(warc2text)

set (CMAKE_CXX_STANDARD 11)

# compile executable into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# compile libraries into lib/
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# define where include files are
include_directories(
    ${PROJECT_SOURCE_DIR}
)

# process subdirectories
add_subdirectory(src)

# define executables
add_executable(decode_main decode_main.cpp)
target_link_libraries(decode_main warc)
