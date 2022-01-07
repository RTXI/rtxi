#! /bin/bash

DIR=$PWD
cd ..

autoreconf -ivf
./configure --enable-posix --disable-analogy --enable-debug
make -j`nproc`
lcov --no-external --capture --initial --directory . --output-file ./coverage/coverage_baseline.info
make -k check
lcov --no-external --capture --directory . --output-file ./coverage/coverage_test.info
lcov --add-tracefile ./coverage/coverage_baseline.info --add-tracefile ./coverage/coverage_test.info \
     --output-file ./coverage/coverage_total.info
genhtml ./coverage/coverage_test.info --output-directory=./coverage

cd $DIR
echo "REPORT COMPLETE"
