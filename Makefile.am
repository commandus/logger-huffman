SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

COMMON_CPP_FLAGS = -std=c++11 -Ithird_party -D_GLIBCXX_USE_CXX11_ABI=0 -DDEBUG_PRINT
AM_CPPFLAGS= $(COMMON_CPP_FLAGS)
AM_CXXFLAGS= $(COMMON_CPP_FLAGS)

#
# Binaries
#
bin_PROGRAMS = logger-huffman-print

LIBLOGGERHUFFMANSRC = \
	logger-huffman.c logger-huffman-impl.cpp utilcompress.cpp

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
	platform.h logger-huffman.h utilcompress.h errlist.h logger-huffman-impl.h \
	util-time-fmt.h \
	third_party/argtable3/argtable3.h

logger_huffman_print_SOURCES = \
	logger-huffman-print.cpp \
	util-time-fmt.cpp \
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

test_temperature_SOURCES = \
	tests/test-temperature.c
test_temperature_LDADD = $(L_LOGGER_HUFFMAN)

check_PROGRAMS = \
	test-compress-decompress \
	test-temperature

TESTS = $(check_PROGRAMS)
