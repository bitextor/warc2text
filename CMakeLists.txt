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
)

# process subdirectories
add_subdirectory(src)

if( PREPROCESS_PATH )
include_directories(${PREPROCESS_PATH})
add_subdirectory(${PREPROCESS_PATH} preprocess-bin)
else()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/preprocess)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/preprocess preprocess-bin)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/opt/icu4c/include")
endif()

include_directories(preprocess)
add_executable(warc2text
warc2text_main.cc
)

# target_link_libraries(warc2text preprocess_util)
target_link_libraries(warc2text warc2text_lib)

# define executables
add_executable(decode_main decode_main.cpp)
# target_link_libraries(decode_main preprocess_util)
target_link_libraries(decode_main warc2text_lib)
