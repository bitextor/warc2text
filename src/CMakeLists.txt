cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

find_package(ZLIB REQUIRED)
include_directories(
    ${ZLIB_INCLUDE_DIR}
)

add_library(warc2text_lib warc.cc)
# target_link_libraries(warc2text-lib preprocess_util ${ZLIB_LIBRARIES})
target_link_libraries(warc2text_lib ${ZLIB_LIBRARIES})

FIND_PACKAGE( Boost 1.61 COMPONENTS filesystem program_options iostreams timer REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

TARGET_LINK_LIBRARIES( warc2text_lib ${Boost_LIBRARIES} )