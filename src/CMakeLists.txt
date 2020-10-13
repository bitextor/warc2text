cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

find_package(ZLIB REQUIRED)
find_package( Boost 1.61 COMPONENTS filesystem program_options iostreams timer REQUIRED )
include_directories(
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

add_library(warc2text_lib warcreader.cc record.cc xh_scanner.cc entities.c)
# target_link_libraries(warc2text-lib preprocess_util ${ZLIB_LIBRARIES})
target_link_libraries(warc2text_lib ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
