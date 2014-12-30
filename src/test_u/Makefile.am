#METASOURCES = AUTO

ACLOCAL_AMFLAGS = -I m4
#AM_CPPFLAGS = \
#  -I$(builddir)/../.. \
#  -I$(srcdir)/../../common \
#  -I$(srcdir)/../../sound \
#  -I$(srcdir)/../../chance/dice

bin_PROGRAMS = olinuxino_mod_mpu6050

noinst_HEADERS = stdafx.h

olinuxino_mod_mpu6050_SOURCES = \
  olinuxino_mod_mpu6050.cpp \
  stdafx.cpp \
  stdafx.h
#olinuxino_mod_mpu6050_CPPFLAGS = \
#  $(AM_CPPFLAGS) \
#  $(pkg_libs_CFLAGS) \
#  -DBASEDIR=\"$(datadir)/@META_PACKAGE@\"
olinuxino_mod_mpu6050_CPPFLAGS = \
  $(pkg_libs_CFLAGS)
#olinuxino_mod_mpu6050_LDFLAGS = $(pkg_libs_LIBS)
#olinuxino_mod_mpu6050_LDADD = \
#  $(builddir)/../../sound/libRPG_Sound.la \
#  $(builddir)/../../common/libRPG_Common.la \
#  $(builddir)/../../chance/dice/libRPG_Dice.la \
#  $(pkg_libs_LIBS) \
#  -lSDL_mixer
olinuxino_mod_mpu6050_LDADD = \
  $(pkg_libs_LIBS)

