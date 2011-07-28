
AC_DEFUN([DS_RTLINUX],
[
	AC_ARG_WITH([rtlinuxdir],
		[AC_HELP_STRING([--with-rtlinuxdir=DIR],
			[specify path to RTLinux source directory])],
		[RTLINUX_DIR="${withval}"],
		[RTLINUX_DIR=/usr/src/rtlinux])

	AS_LINUX_CONFIG_OPTION_MODULE([CONFIG_RTLINUX])

	if test "${CONFIG_RTLINUX}" != "no" ; then
		AC_MSG_CHECKING([RTLinux directory ${RTLINUX_DIR}])
		if [[ -d ${RTLINUX_DIR}/include ]] ; then
			RTLINUX_CFLAGS="-I${RTLINUX_DIR}/include -I${RTLINUX_DIR}/include/compat -I${RTLINUX_DIR}/include/posix -D__RT__"
		else
			AC_MSG_ERROR([incorrect RTLinux directory?])
		fi
		AC_MSG_RESULT([found])
		AC_DEFINE([CONFIG_COMEDI_RTL],[true],[Define if kernel is RTLinux patched])
		$1
	else
		$2
	fi
	AC_SUBST(RTLINUX_CFLAGS)
])

