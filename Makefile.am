SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

EXTRA_LIB =
EXTRA_HEADERS =
EXTRA_DEF =

if ENABLE_LOGGER_PASSPORT
EXTRA_LIB += -L../logger-passport -llogger-passport -lpthread
EXTRA_HEADERS += third_party/logger-passport/logger-passport.h
EXTRA_DEF += -DENABLE_LOGGER_PASSPORT 
endif

#
# https://ask-dev.ru/info/1247790/understanding-gcc-5s-glibcxxusecxx11abi-or-the-new-abi
#-D_GLIBCXX_USE_CXX11_ABI=0
# -I../../third_party for make distcheck target be happy
#
COMMON_C_FLAGS = -Ithird_party -I../../third_party -DDEBUG_PRINT -O2
AM_CXXFLAGS= -std=c++11 $(COMMON_C_FLAGS)
AM_CPPFLAGS = $(COMMON_C_FLAGS)

HDRS = \
	platform.h logger-huffman.h util-compress.h errlist.h logger-collection.h \
	util-time-fmt.h logger-sql-clause.h logger-parse.h strptime.h dumb-logger-loader.h \
	logger-kosa-loader.h logger-builder.h \
	third_party/logger-passport/logger-passport.h third_party/argtable3/argtable3.h

#
# Binaries
#
bin_PROGRAMS = logger-huffman-print

LIBLOGGERHUFFMANSRC = \
	logger-parse.cpp logger-huffman.c logger-collection.cpp util-compress.cpp util-time-fmt.cpp \
	util-time-fmt.cpp logger-sql-clause.cpp strptime.cpp logger-builder.cpp dumb-logger-loader.cpp

#
#	Static library
#
lib_LIBRARIES = liblogger-huffman.a
liblogger_huffman_a_SOURCES = $(LIBLOGGERHUFFMANSRC)
# $(HDRS)
liblogger_huffman_a_CXXFLAGS = -std=c++11 $(COMMON_C_FLAGS) -fPIC $(EXTRA_DEF)
L_LOGGER_HUFFMAN = -L. -llogger-huffman $(EXTRA_LIB)

# Static library used for build binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

# EXTRA_DIST = $(HDRS)
include_HEADERS = $(HDRS)
# nobase_dist_include_HEADERS =

# logger_huffman_print: third_party/argtable3/argtable3.c third_party/argtable3/argtable3.h
logger_huffman_print_SOURCES = \
	logger-huffman-print.cpp \
	third_party/argtable3/argtable3.c
logger_huffman_print_LDADD = \
	$(L_LOGGER_HUFFMAN)
logger_huffman_print_CPPFLAGS = $(EXTRA_DEF)

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
test_compress_decompress_CPPFLAGS = $(EXTRA_DEF)

test_temperature_SOURCES = \
	tests/test-temperature.c
test_temperature_LDADD = $(L_LOGGER_HUFFMAN)

test_sql_SOURCES = \
	tests/test-sql.cpp
test_sql_LDADD = $(L_LOGGER_HUFFMAN)

check_PROGRAMS = \
	test-compress-decompress test-temperature test-sql

TESTS = $(check_PROGRAMS)
