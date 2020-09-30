cmake_minimum_required(VERSION 2.8.3)

project(warc2text)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# compile executable into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# compile libraries into lib/
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# define where include files are
include_directories(
    ${PROJECT_SOURCE_DIR}
    # ${PROJECT_SOURCE_DIR}/preprocess
)

# process subdirectories
add_subdirectory(src)
# add_subdirectory(preprocess)

# define executables
add_executable(decode_main decode_main.cpp)
# target_link_libraries(decode_main preprocess_util warc2text-lib)
target_link_libraries(decode_main warc2text_lib)
