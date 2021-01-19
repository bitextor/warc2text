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

find_package(ZLIB 1.2.11 REQUIRED)
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
    lang.cc
    util.cc
    bilangwriter.cc
    xh_scanner.cc
    entities.cc
    zipreader.cc
)


if (APPLE)
	target_link_libraries(warc2text_lib
		/usr/local/opt/libzip/lib/libzip.dylib)
elseif(LIBZIP_PATH)
	target_link_libraries(warc2text_lib
		${LIBZIP_PATH})
else()
	target_link_libraries(warc2text_lib
		zip)

endif()

target_link_libraries(warc2text_lib
    base64
    preprocess_util
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${uchardet_LIBRARIES}
)

