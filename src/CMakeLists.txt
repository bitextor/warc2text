cmake_minimum_required(VERSION 2.8.3)

project(warc2test-src)

set(COMPRESS_FLAGS)
set(COMPRESS_LIBS)
find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIR})
set_source_files_properties(warc.cc PROPERTIES COMPILE_FLAGS "-DHAVE_ZLIB")
add_library(warc warc.cc)
target_link_libraries(warc ${ZLIB_LIBRARIES})
