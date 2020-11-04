cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

find_library(UCHARDET uchardet REQUIRED)
find_package(ZLIB REQUIRED)
find_package( Boost 1.61 COMPONENTS locale iostreams filesystem log REQUIRED )
include_directories(
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

add_library(warc2text_lib
    warcpreprocessor.cc
    warcreader.cc
    record.cc
    html.cc
    deferred.cc
    lang.cc
    util.cc
    bilangwriter.cc
    xh_scanner.cc
    entities.c
)

target_link_libraries(warc2text_lib
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UCHARDET}
)
