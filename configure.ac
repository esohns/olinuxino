#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# define version information
m4_define([OLINUXINO_VERSION],[0.1])
AC_SUBST([OLINUXINO_VERSION])

AC_PREREQ([2.69])
AC_INIT([olinuxino], [OLINUXINO_VERSION], [eriksohns@123mail.org], [OLinuxIno], [http://www.github.com/esohns/olinuxino])
AC_CONFIG_SRCDIR([src/ko/olimex_mod_mpu6050.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([aux_config])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O

# init other tools
AM_INIT_AUTOMAKE
# build time sanity check...
AM_SANITY_CHECK

# prefix config.h
AX_PREFIX_CONFIG_H([olinuxino_config.h], [OLinuxIno], [config.h])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.

# set up kernel module support
AC_PATH_KERNEL_SOURCE
default_moduledir=/lib/modules/`uname -r`/misc
AC_ARG_WITH(moduledir,
            [  --with-moduledir=DIR    kernel modules in DIR (/lib/modules/`uname -r`/misc)],
            moduledir=${withval},
            moduledir=${default_moduledir})
AC_SUBST(moduledir)

## (re)set the cpp compiler flags
#CXXFLAGS="-fno-implicit-templates"
CXXFLAGS=
# --> enable debug support ?
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug,
              [--enable-debug=[no/yes] turn on debugging [default=$debug_default]],
              ,
              enable_debug=$debug_default)
if test "x$enable_debug" = "xyes"; then
  CXXFLAGS="${CXXFLAGS} -g -O0 -Wall -D_DEBUG -DDEBUG_DEBUGGER"
  AC_MSG_RESULT(yes)
else
#  CXXFLAGS="-O3 -Wall -Werror -Wno-unused-variable -Wno-sign-compare"
  CXXFLAGS="${CXXFLAGS} -O3 -DNDEBUG -Wall -Werror"
  AC_MSG_RESULT(no)
fi

# --> enable tracing support ?
AC_MSG_CHECKING(whether to enable tracing)
tracing_default="no"
AC_ARG_ENABLE(tracing,
              [--enable-tracing=[no/yes] turn on tracing [default=$tracing_default]],
              ,
              enable_tracing=$tracing_default)
if test "x$enable_tracing" = "xyes"; then
  CXXFLAGS="${CXXFLAGS} -DOLINUXINO_NTRACE=0"
  AC_MSG_RESULT(yes)
else
  CXXFLAGS="${CXXFLAGS} -DOLINUXINO_NTRACE=1"
  AC_MSG_RESULT(no)
fi

# --> enable valgrind support ?
AC_MSG_CHECKING(whether to enable valgrind support)
valgrind_support_default="no"
AC_ARG_ENABLE(valgrind,
              [--enable-valgrind=[no/yes] turn on valgrind support [default=$valgrind_support_default]],
              ,
              enable_valgrind=$valgrind_support_default)
if test "x$enable_valgrind" = "xyes"; then
  CXXFLAGS="${CXXFLAGS} -DOLINUXINO_ENABLE_VALGRIND_SUPPORT"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

# export these variable (so Makefile substitutions can be made
AC_SUBST(CXXFLAGS)

# checks for libraries
#PKG_CHECK_MODULES(pkg_libs, xerces-c libpng sdl SDL_ttf libglade-2.0 libgnomeui-2.0 >= 1.110.0 gmodule-export-2.0 gthread-2.0 gtk+-2.0)
#AC_SEARCH_LIBS([Mix_Init], [SDL_mixer])

# checks for other libraries (non-pkg-config)
# FIXME: Replace `main' with a function in `-lACE':
AC_CHECK_LIB([ACE], [main])
# FIXME: Replace `main' with a function in `-lpthread':
#AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lxerces-c':
#AC_CHECK_LIB([xerces-c], [main])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/ko/Makefile
                 src/ko/3rd_party/Makefile
                 src/ko/3rd_party/i2cdevlib/Makefile
                 src/ko/3rd_party/msp430/Makefile
                 src/test_u/Makefile])
AC_CONFIG_FILES([OLinuxIno-${OLINUXINO_VERSION}.pc:scripts/OLinuxIno.pc.in])

AC_OUTPUT
