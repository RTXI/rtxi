#!/bin/bash

set -e

conan profile detect -f

std=17
if [ "$RUNNER_OS" = Windows ]; then
  std=17
fi

profile="$(conan profile path default)"

mv "$profile" "${profile}.bak"
sed 's/^\(compiler\.cppstd=\).\{1,\}$/\1'"$std/" "${profile}.bak" > "$profile"
rm "${profile}.bak"
