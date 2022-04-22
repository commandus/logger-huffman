AC_PREREQ([2.63])
AC_INIT([logger-huffman], [0.1], [andrey.ivanov@ikfia.ysn.ru])
AM_INIT_AUTOMAKE([1.11 foreign no-define subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([logger-huffman.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_ARG_ENABLE([logger-passport],
[  --enable-logger-passport    Turn on logger passport ],
[case "${enableval}" in
  yes) logger_passport=true ;;
  no)  logger_passport=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-logger-passport]) ;;
esac],[logger_passport=false])
AM_CONDITIONAL([ENABLE_LOGGER_PASSPORT], [test x$logger_passport = xtrue])

AC_OUTPUT
