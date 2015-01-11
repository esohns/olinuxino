SUBDIRS = \
  3rd_party

AM_CPPFLAGS = \
  -I@kerneldir@ \
  -I@kerneldir@/arch/arm/plat-sunxi/include

EXTRA_DIST = Makefile.kernel Makefile.common
EXTRA_PROGRAMS = automake_dummy
automake_dummy_SOURCES = \
  olinuxino_mod_mpu6050.c \
  olinuxino_mod_mpu6050.h
automake_dummy_CFLAGS = \
  $(AM_CPPFLAGS) \
  -I$(abs_srcdir)/3rd_party/i2cdevlib \
  -I$(abs_srcdir)/3rd_party/msp430

module_DATA = olinuxino_mod_mpu6050.o

include ./Makefile.common
