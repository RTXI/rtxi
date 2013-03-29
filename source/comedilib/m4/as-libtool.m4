dnl as-libtool.m4 0.0.2
dnl autostars m4 macro for libtool versioning
dnl thomas@apestaart.org
dnl
dnl AS_LIBTOOL(PREFIX, CURRENT, REVISION, AGE, USE_RELEASE)
dnl example
dnl AS_VERSION(GST, 2, 0, 0)
dnl
dnl this macro
dnl - defines [$PREFIX]_CURRENT, REVISION AND AGE
dnl - defines [$PREFIX]_LIBVERSION
dnl - defines [$PREFIX]_LT_LDFLAGS to set versioning
dnl - AC_SUBST's them all
dnl
dnl if USE_RELEASE = yes, then add a -release option to the LDFLAGS
dnl with the (pre-defined) [$PREFIX]_VERSION
dnl then use [$PREFIX]_LT_LDFLAGS in the relevant Makefile.am's

AC_DEFUN([AS_LIBTOOL],
[
  [$1]_CURRENT=[$2]
  [$1]_REVISION=[$3]
  [$1]_AGE=[$4]
  [$1]_LIBVERSION=[$2]:[$3]:[$4]
  AC_SUBST([$1]_CURRENT)
  AC_SUBST([$1]_REVISION)
  AC_SUBST([$1]_AGE)
  AC_SUBST([$1]_LIBVERSION)

  [$1]_LT_LDFLAGS="$[$1]_LT_LDFLAGS -version-info $[$1]_LIBVERSION"
  if test ! -z "[$5]"
  then
    [$1]_LT_LDFLAGS="$[$1]_LT_LDFLAGS -release $[$1]_VERSION"
  fi
  AC_SUBST([$1]_LT_LDFLAGS)

  AC_LIBTOOL_DLOPEN
  AC_PROG_LIBTOOL
])
