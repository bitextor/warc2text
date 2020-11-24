cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

find_library(uchardet_LIBRARIES uchardet
    REQUIRED
    PATHS ${UCHARDET_PATH}/lib
)
find_path(uchardet_INCLUDE_DIR uchardet/uchardet.h
    REQUIRED
    PATHS ${UCHARDET_PATH}/include
)

find_package(ZLIB REQUIRED)
find_package( Boost 1.71 COMPONENTS locale iostreams filesystem log REQUIRED )

include_directories(
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${uchardet_INCLUDE_DIR}
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
    entities.cc
)

target_link_libraries(warc2text_lib
    base64
    preprocess_util
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${uchardet_LIBRARIES}
)
