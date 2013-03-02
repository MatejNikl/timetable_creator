# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([timetable_creator], [0.1.0])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([1.9 foreign subdir-objects dist-xz -Wall -Werror])
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

# Checks for programs.
AC_LANG([C])
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([isascii setlocale strerror])

AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_OUTPUT

