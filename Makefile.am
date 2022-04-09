SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

#
# https://ask-dev.ru/info/1247790/understanding-gcc-5s-glibcxxusecxx11abi-or-the-new-abi
#-D_GLIBCXX_USE_CXX11_ABI=0
#
COMMON_C_FLAGS = -I./third_party -DDEBUG_PRINT -O2
AM_CXXFLAGS= -std=c++11 $(COMMON_C_FLAGS)
AM_CPPFLAGS = $(COMMON_C_FLAGS)

HDRS = \
	platform.h logger-huffman.h util-compress.h errlist.h logger-collection.h \
	util-time-fmt.h logger-passport.h logger-sql-clause.h logger-parse.h \
	strptime.h \
	third_party/argtable3/argtable3.h

#
# Binaries
#
bin_PROGRAMS = logger-huffman-print

LIBLOGGERHUFFMANSRC = \
	logger-parse.cpp logger-huffman.c logger-collection.cpp util-compress.cpp util-time-fmt.cpp \
	util-time-fmt.cpp logger-passport.cpp logger-sql-clause.cpp strptime.cpp

#
#	Static library
#
lib_LIBRARIES = liblogger-huffman.a
liblogger_huffman_a_SOURCES = $(LIBLOGGERHUFFMANSRC)
# $(HDRS)
liblogger_huffman_a_CXXFLAGS = -std=c++11 $(COMMON_C_FLAGS) -fPIC

L_LOGGER_HUFFMAN = -L. -llogger-huffman

# Static library used for buil binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

# EXTRA_DIST = $(HDRS)
include_HEADERS = $(HDRS)
# nobase_dist_include_HEADERS =

# logger_huffman_print: third_party/argtable3/argtable3.c third_party/argtable3/argtable3.h
logger_huffman_print_SOURCES = \
	logger-huffman-print.cpp \
	third_party/argtable3/argtable3.c
#	$(HDRS)
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

test_sql_SOURCES = \
	tests/test-sql.cpp
test_sql_LDADD = $(L_LOGGER_HUFFMAN)

check_PROGRAMS = \
	test-compress-decompress test-temperature test-sql

TESTS = $(check_PROGRAMS)
