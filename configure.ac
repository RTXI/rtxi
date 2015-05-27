AC_INIT([RTXI],[2.0-rc3],[help@rtxi.org],rtxi)

AM_CONFIG_HEADER(include/rtxi_config.h)
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

LIBS=
AC_CHECK_HEADER([hdf5.h])
AC_CHECK_LIB([hdf5],[H5Fopen])
AC_CHECK_LIB([hdf5_hl],[H5PTopen])
HDF_LIBS=$LIBS
AC_SUBST(HDF_LIBS)

rtos=""

AX_BOOST_BASE([1.54],,[AC_MSG_ERROR([libboost version 1.54 or great is required])])

AC_CHECK_RTAI
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

AM_CONDITIONAL(RTAI3, test x$rtos = xrtai3)
AM_CONDITIONAL([XENOMAI],[test x$rtos = xxenomai])
AM_CONDITIONAL([POSIX],[test x$rtos = xposix])
if test x$rtos = xrtai3; then
  RTOS_CPPFLAGS=$RTAI_CPPFLAGS
  RTOS_LDFLAGS=$RTAI_LDFLAGS
elif test x$rtos = xxenomai; then
  if test -x /usr/xenomai/bin/xeno-config; then
    XENO_CONFIG='/usr/xenomai/bin/xeno-config'
  elif test -x /usr/bin/xeno-config; then
    XENO_CONFIG='/usr/bin/xeno-config'
  else
    XENO_CONFIG=`which xeno-config`
  fi
  RTOS_CPPFLAGS=`$XENO_CONFIG --skin=native --cflags`
  RTOS_LDFLAGS=`$XENO_CONFIG --skin=native --ldflags`
elif test x$rtos = xposix; then
  RTOS_CPPFLAGS='-I/usr/local/include'
  RTOS_LDFLAGS='-I/usr/local/include -L/usr/local/lib -lpthread'
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

AC_ARG_ENABLE(comedi,
  [  --enable-comedi         build the comedi driver],
  [case "${enableval}" in
    "" | y | ye | yes) comedi=true;;
    n | no) comedi=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-comedi);;
  esac],
[comedi=true])

AC_ARG_ENABLE(analogy,
  [  --enable-analogy        build the analogy driver],
  [case "${enableval}" in
    "" | y | ye | yes) analogy=true;;
    n | no) analogy=false;;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-analogy);;
  esac],
  [analogy=true])  

if test x$rtos = xrtai3; then
  AM_CONDITIONAL(BUILD_COMEDI, test x$comedi = xtrue)
  AM_CONDITIONAL(BUILD_COMEDILIB,false)
  AM_CONDITIONAL(BUILD_ANALOGY, false)
elif test x$rtos = xposix; then
  AM_CONDITIONAL(BUILD_COMEDI,false)
  AM_CONDITIONAL(BUILD_COMEDILIB, test x$comedi = xtrue)
  AM_CONDITIONAL(BUILD_ANALOGY, false)
elif test x$rtos = xxenomai; then
  AM_CONDITIONAL(BUILD_COMEDI,false)
  AM_CONDITIONAL(BUILD_COMEDILIB,false)
  AM_CONDITIONAL(BUILD_ANALOGY, test x$analogy = xtrue)
else
  AM_CONDITIONAL(BUILD_COMEDI,false)
  AM_CONDITIONAL(BUILD_COMEDILIB,false)
  AM_CONDITIONAL(BUILD_ANALOGY, false)
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
rtxi/Makefile
scripts/Makefile
scripts/init_rtxi
scripts/rtxi_comedi
scripts/rtxi_plugin_config
plugins/Makefile
plugins/analogy_device/Makefile
plugins/comedi_device/Makefile
plugins/comedilib_device/Makefile
plugins/connector/Makefile
plugins/data_recorder/Makefile
plugins/dynamo_model_loader/Makefile
plugins/model_loader/Makefile
plugins/oscilloscope/Makefile
plugins/performance_measurement/Makefile
plugins/system_control/Makefile
plugins/userprefs/Makefile
deps/hdf/Makefile
])

dnl Generate config.status and launch it
AC_OUTPUT
