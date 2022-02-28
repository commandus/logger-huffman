cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "logger-huffman")
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

set(LIB_LOGGER_HUFFMAN_SRC
	utilcompress.cpp logger-huffman.c
)
set(SRC_LOGGER_HUFFMAN_PRINT
	logger-huffman-print.cpp
	third_party/argtable3/argtable3.c
)

include_directories("third_party")

add_library(logger-huffman STATIC
	${LIB_LOGGER_HUFFMAN_SRC}
)

add_executable(logger-huffman-print ${SRC_LOGGER_HUFFMAN_PRINT})
target_link_libraries(logger-huffman-print PRIVATE
	logger-huffman
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/loger-huffman-print DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
include(CPack)
