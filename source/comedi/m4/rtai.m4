
AC_DEFUN([DS_RTAI],
[
	AC_ARG_WITH([rtaidir],
		[AC_HELP_STRING([--with-rtaidir=DIR],
			[specify path to RTAI installation or build directory])],
		[RTAI_DIR="${withval}"],
		[RTAI_DIR=/usr/realtime])

	AS_LINUX_CONFIG_OPTION_MODULE([CONFIG_RTHAL])
	AS_LINUX_CONFIG_OPTION_MODULE([CONFIG_ADEOS])
	AS_LINUX_CONFIG_OPTION_MODULE([CONFIG_IPIPE])
	
	if test "${ENABLE_RTAI}" = "yes" -a \( "${CONFIG_RTHAL}" != "no" -o "${CONFIG_ADEOS}" != "no" -o "${CONFIG_IPIPE}" != "no" \); then
		AC_MSG_CHECKING([RTAI directory ${RTAI_DIR}])
		if [[ -d ${RTAI_DIR}/include ]] ; then
			RTAI_CFLAGS="-I${RTAI_DIR}/include"
		else
			if [[ -d ${RTAI_DIR}/rtai-core/include ]] ; then
				RTAI_CFLAGS=" -I${RTAI_DIR} -I${RTAI_DIR}/rtai-core/include"
			else
				AC_MSG_ERROR([incorrect RTAI directory?])
			fi
		fi
		$1
		AC_MSG_RESULT([found])
		FUSION_TEST=`${RTAI_DIR}/bin/rtai-config --version | cut -d"-" -f2 `
		if test "${FUSION_TEST}" = "fusion"
		then
			AC_DEFINE([CONFIG_COMEDI_FUSION],[true],[Define if kernel is RTAI patched])
		else
			AC_DEFINE([CONFIG_COMEDI_RTAI],[true],[Define if kernel is RTAI patched])
		fi

	else
		$2
	fi
	AC_SUBST(RTAI_CFLAGS)

])

