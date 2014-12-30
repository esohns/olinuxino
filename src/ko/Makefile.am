AM_CPPFLAGS = \
  -I/usr/src/linux/include

EXTRA_DIST = Makefile.kernel Makefile.common
EXTRA_PROGRAMS = automake_dummy
automake_dummy_SOURCES = \
  olinuxino_mod_mpu6050.c
automake_dummy_CFLAGS = \
  $(AM_CPPFLAGS)

module_DATA = olinuxino_mod_mpu6050.o

include ./Makefile.common
