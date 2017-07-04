#
# The Real-Time eXperiment Interface (RTXI)
# 
# Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill
# Cornell Medical College
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.
#

AC_INIT([RTXI],[2.1],[help@rtxi.org],rtxi)

AM_CONFIG_HEADER(src/rtxi_config.h)
AM_INIT_AUTOMAKE([foreign])
AM_PATH_CPPUNIT
AM_SILENT_RULES([yes])

AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX

LIBS=
AC_CHECK_LIB([dl],[dlopen])
AC_CHECK_LIB([m],[cos])
AC_CHECK_LIB([gslcblas],[cblas_dgemm])
AC_CHECK_LIB([gsl],[gsl_blas_dgemm])
GSL_LIBS=$LIBS
AC_SUBST(GSL_LIBS)

# LIBS=
# AC_CHECK_HEADER([hdf5.h])
# AC_CHECK_LIB([hdf5],[H5Fopen])
# AC_CHECK_LIB([hdf5_hl],[H5PTopen])
# HDF_LIBS=$LIBS
# AC_SUBST(HDF_LIBS)

# Quiet ar warning about default modifiers
AC_SUBST(AR_FLAGS, [cr])

rtos=""

AX_BOOST_BASE([1.54],,[AC_MSG_ERROR([libboost version 1.54 or great is required])])

AC_ARG_ENABLE(xenomai,
  [  --enable-xenomai        build the Xenomai interface],
  [case "${enablecal}" in
    "" | y | ye | yes) rtos=xenomai;;
    n | no);;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-xenomai);;
  esac],
  [])
AC_ARG_ENABLE(posix,
  [  --enable-posix          build the POSIX non-RT interface],
  [case "${enableval}" in
    "" | y | ye | yes) rtos=posix;;
    n | no);;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-posix);;
  esac],
  [])

AM_CONDITIONAL([XENOMAI],[test x$rtos = xxenomai])
AM_CONDITIONAL([POSIX],[test x$rtos = xposix])
if test x$rtos = xxenomai; then
  if test -x /usr/xenomai/bin/xeno-config; then
    XENO_CONFIG='/usr/xenomai/bin/xeno-config'
  elif test -x /usr/bin/xeno-config; then
    XENO_CONFIG='/usr/bin/xeno-config'
  else
    XENO_CONFIG=`which xeno-config`
  fi
  RTOS_CPPFLAGS=`$XENO_CONFIG --alchemy --cflags`
  RTOS_LDFLAGS=`$XENO_CONFIG --alchemy --ldflags --auto-init-solib`
elif test x$rtos = xposix; then
  RTOS_CPPFLAGS='-I/usr/local/include'
  RTOS_LDFLAGS='-L/usr/local/lib -lpthread'
elif test x$rtos = x;then
  AC_MSG_ERROR([no realtime system found])
fi
AC_SUBST(RTOS_CPPFLAGS)
AC_SUBST(RTOS_LDFLAGS)

AC_ARG_ENABLE(debug,
  [  --enable-debug          turn on debugging],
  [case "${enableval}" in
    "" | y | ye | yes) debug=true ;;
    n | no) debug=false ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
  esac],
  [debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

AC_ARG_ENABLE(analogy,
  [  --enable-analogy        build the analogy driver],
  [case "${enableval}" in
    "" | y | ye | yes) analogy=true;;
    n | no) analogy=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-analogy);;
  esac],
  [analogy=true])  

if test x$rtos = xposix; then
  AM_CONDITIONAL(BUILD_ANALOGY, false)
elif test x$rtos = xxenomai; then
  AM_CONDITIONAL(BUILD_ANALOGY, test x$analogy = xtrue)
fi

dnl Clear build variables

INCLUDES=
DEFS=
CPPFLAGS=
CXXFLAGS=
CFLAGS=
LDADD=
LDFLAGS=
LIBS=

dnl Create makefiles and other configuration files
AC_CONFIG_FILES([
Makefile
src/Makefile
scripts/Makefile
scripts/rtxi_plugin_config
res/Makefile
res/icons/Makefile
libs/Makefile
libs/dsp/Makefile
libs/gen/Makefile
libs/math/Makefile
libs/plot/Makefile
plugins/Makefile
plugins/analogy_device/Makefile
plugins/connector/Makefile
plugins/data_recorder/Makefile
plugins/model_loader/Makefile
plugins/oscilloscope/Makefile
plugins/performance_measurement/Makefile
plugins/system_control/Makefile
plugins/userprefs/Makefile
plugins/module_installer/Makefile
])

dnl Generate config.status and launch it
AC_OUTPUT
