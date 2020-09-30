cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

find_package(ZLIB REQUIRED)
include_directories(
    ${ZLIB_INCLUDE_DIR}
)

add_library(warc2text_lib warc.cc)
# target_link_libraries(warc2text-lib preprocess_util ${ZLIB_LIBRARIES})
target_link_libraries(warc2text_lib ${ZLIB_LIBRARIES})
