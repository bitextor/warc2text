cmake_minimum_required(VERSION 2.8.3)

project(warc2text)

set(CMAKE_CXX_STANDARD 11)


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/local/opt/icu4c/include")
endif ()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Ofast")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(Boost 1.71 COMPONENTS program_options log log_setup REQUIRED)

# compile executable into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# compile libraries into lib/
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# define where include files are
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${Boost_INCLUDE_DIR}
)

# process subdirectories
add_subdirectory(src)

# if (PREPROCESS_PATH)
#     include_directories(${PREPROCESS_PATH})
#     add_subdirectory(${PREPROCESS_PATH})
# else ()
#     include_directories(${PROJECT_SOURCE_DIR}/preprocess)
#     add_subdirectory(${PROJECT_SOURCE_DIR}/preprocess)
# endif ()

# build libcld2.so
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/cld2/internal/libcld2.so")
    execute_process(
            COMMAND bash -c "export CFLAGS='-Wno-narrowing -O3' && bash compile_libs.sh"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/cld2/internal
    )
endif ()

# define executables
add_executable(warc2text warc2text_main.cc)
target_link_libraries(warc2text
    warc2text_lib
    ${Boost_LIBRARIES}
    ${PROJECT_SOURCE_DIR}/cld2/internal/libcld2.so
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_custom_command(TARGET warc2text
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -change libcld2.so ${PROJECT_SOURCE_DIR}/cld2/internal/libcld2.so
            $<TARGET_FILE:warc2text>)
endif()
