cmake_minimum_required(VERSION 2.8.3)

project(warc2text_src)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(JAVA_INCLUDE_PATH2 "$ENV{JAVA_HOME}/include")
	set(JNI_INCLUDE_DIRS "$ENV{JAVA_HOME}/include;$ENV{JAVA_HOME}/include/darwin")
	if(EXISTS "$ENV{JAVA_HOME}/jre/lib/server/libjvm.dylib")
		set(JNI_LIBRARIES "$ENV{JAVA_HOME}/jre/lib/server/libjvm.dylib")
	else()
		set(JNI_LIBRARIES "$ENV{JAVA_HOME}/lib/server/libjvm.dylib")
	endif()
else()
	find_package(JNI REQUIRED)
endif ()

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
    ${JNI_INCLUDE_DIRS}
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
    pdfextract.cc
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
    ${JNI_LIBRARIES}
)
