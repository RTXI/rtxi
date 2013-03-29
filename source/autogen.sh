#!/bin/bash

aclocal -I m4
libtoolize --copy --force --automake
autoheader 
autoconf
automake -a -c
