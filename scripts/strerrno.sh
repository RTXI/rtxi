#!/bin/sh

ERRNO_HEADERS="/usr/include/asm-generic/errno-base.h /usr/include/asm-generic/errno.h"

if [ "$1" = "" ]; then
    ERR="[[:digit:]]+"
else
    ERR=$1
fi

cat $ERRNO_HEADERS | grep -E "[[:space:]]$ERR[[:space:]]/"

