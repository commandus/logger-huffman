SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS=-std=c++11
AM_CXXFLAGS=-std=c++11
COMMON_CPP_FLAGS = -Ithird_party -D_GLIBCXX_USE_CXX11_ABI=0 -DDEBUG_PRINT

#
# Binaries
#
bin_PROGRAMS = logger-huffman-print

LIBLOGGERHUFFMANSRC = \
	logger-huffman.c utilcompress.cpp

#
#	Static library
#
lib_LIBRARIES = liblogger-huffman.a
liblogger_huffman_a_SOURCES = $(LIBLOGGERHUFFMANSRC)
liblogger_huffman_a_CXXFLAGS = -fPIC

L_LOGGER_HUFFMAN = -L. -llogger-huffman

# Static library used for buil binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

nobase_dist_include_HEADERS = \
	logger-huffamn.h utilcompress.h \
	third_party/argtable3/argtable3.h

logger_huffman_print_SOURCES = \
	logger-huffman-print.cpp \
	third_party/argtable3/argtable3.c
logger_huffman_print_LDADD = \
	$(L_LOGGER_HUFFMAN)
logger_huffman_print_CPPFLAGS = 

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

#
# Tests
#
test_compress_decompress_SOURCES = \
	tests/test-compress-decompress.cpp
test_compress_decompress_LDADD = $(L_LOGGER_HUFFMAN)
test_compress_decompress_CPPFLAGS = 

check_PROGRAMS = \
	test-encrypt-join-accept-response

TESTS = $(check_PROGRAMS)
