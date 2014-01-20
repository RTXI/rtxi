AC_DEFUN([AC_CHECK_RTAI],
[
  AC_MSG_CHECKING(rtai-config)

  AC_ARG_WITH(rtai-config,
  [  --with-rtai-config=FILE location of the rtai-config program],
  [
    if test "$withval" == "no"; then
      ac_rtai_config=""
    else
      if test "$withval" == "yes"; then
        withval=""
      fi
      if test "$withval" != ""; then
        if test `echo $withval | head -c 1` == "/"; then
          ac_rtai_config=$withval
        else
          ac_rtai_config=`pwd`/$withval
        fi
      else
        AC_LOCATE_RTAI_CONFIG
      fi
    fi
  ],
  [ AC_LOCATE_RTAI_CONFIG ])

  if test "$ac_rtai_config" != ""; then
    AC_MSG_RESULT($ac_rtai_config)

    RTAI_CPPFLAGS=`$ac_rtai_config --lxrt-cflags | sed -r -e 's/ -W@<:@-A-Za-z@:>@*| -O@<:@0-9@:>@?| -pipe|-I..? //g'`
    if test -n `$ac_rtai_config --comedi-dir`; then
      RTAI_CPPFLAGS="$RTAI_CPPFLAGS -I`$ac_rtai_config --comedi-dir`/include"
    fi
    RTAI_LDFLAGS=`$ac_rtai_config --lxrt-ldflags`

    rtos=rtai3
    AC_MSG_CHECKING(RTAI_CPPFLAGS)
    AC_MSG_RESULT($RTAI_CPPFLAGS)
    AC_MSG_CHECKING(RTAI_LDFLAGS)
    AC_MSG_RESULT($RTAI_LDFLAGS)
  else
    AC_MSG_RESULT([rtai-config not found])
  fi
])

AC_DEFUN([AC_LOCATE_RTAI_CONFIG],
[
  path_list="
    `ls -dr /usr/realtim*/bin 2>/dev/null`
    `ls -dr /usr/src/rta*/bin 2>/dev/null`
    `ls -dr /opt/realtim*/bin 2>/dev/null`
    `ls -dr /usr/src/realtim*/bin 2>/dev/null`
    /usr/bin"
  progs=
  for dir in $path_list; do
    if test -x "$dir/rtai-config"; then
      progs="$progs $dir/rtai-config"
    fi
  done
  prev_ver=0
  for prog in $progs; do
    this_ver=`$prog --version`
    if test x"$this_ver" != x"" && expr $this_ver '>' $prev_ver >/dev/null; then
      ac_rtai_config=$prog
      prev_ver=$this_ver
    fi
  done
  if test x"$prev_ver" == x"0"; then
    ac_rtai_config=""
  fi
])
