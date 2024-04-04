cmake_minimum_required(VERSION 3.10)

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
find_package( Boost 1.71 COMPONENTS locale iostreams filesystem log regex REQUIRED )

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
    lang_cld2.cc
    lang_fasttext.cc
    util.cc
    bilangwriter.cc
    xh_scanner.cc
    entities.cc
    zipreader.cc
)


if (APPLE)
	target_link_libraries(warc2text_lib
            PRIVATE /usr/local/opt/libzip/lib/libzip.dylib)
elseif(LIBZIP_PATH)
	target_link_libraries(warc2text_lib
            PRIVATE ${LIBZIP_PATH})
else()
	target_link_libraries(warc2text_lib
            PRIVATE zip)

endif()

target_link_libraries(warc2text_lib
    PRIVATE base64
    PRIVATE preprocess_util
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE ${ZLIB_LIBRARIES}
    PRIVATE ${uchardet_LIBRARIES}
    PRIVATE nlohmann_json::nlohmann_json
)
