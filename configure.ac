#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([VERSION_NUMBER], [esyscmd([. ./VERSION; echo -n $VERSION])])
m4_syscmd([. ./VERSION; tail -n +2 README > README.tmp; echo ircss $VERSION > README; cat README.tmp >> README; rm README.tmp])
m4_syscmd([. ./VERSION; cat ChangeLog | grep "ircss $VERSION" >/dev/null || /bin/echo -e "\nircss $VERSION\n---------\n" >> ChangeLog])
AC_PREREQ(2.61)
AC_INIT([ircss],[VERSION_NUMBER],[bugs@ircss.org])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([src/core.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
