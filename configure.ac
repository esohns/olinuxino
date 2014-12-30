#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# define version information
ONLINUXINO_VERSION="0.1"
AC_SUBST([ONLINUXINO_VERSION])

AC_PREREQ([2.69])
AC_INIT([olinuxino], [ONLINUXINO_VERSION], [eriksohns@123mail.org], [OLinuxIno], [http://www.github.com/esohns/olinuxino])
AC_CONFIG_SRCDIR([src/ko/olinuxino_mod_mpu6050.c])
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
	    moduledir=${default_moduledir}
)
AC_SUBST(moduledir)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/ko/Makefile
                 src/ko/3rd_party/Makefile
                 src/ko/3rd_party/msp430/Makefile
                 src/test_u/Makefile])
AC_OUTPUT
