#! /bin/bash

DIR=$PWD
cd ..

echo "======================BUILDING APPLICATION========================="
make clean
autoreconf -ivf
./configure --enable-posix --disable-analogy --enable-debug
make -j`nproc`
lcov --no-external --capture --initial --directory . --output-file ./coverage/coverage_baseline.info
make -k check

echo "=====================CREATING COVERAGE REPORT======================"
ROOTDIR=$PWD
lcov --no-external --capture --directory . --output-file ./coverage/coverage_test.info
lcov --add-tracefile ./coverage/coverage_baseline.info --add-tracefile ./coverage/coverage_test.info \
     --output-file ./coverage/coverage_total.info
lcov --remove ./coverage/coverage_total.info ${ROOTDIR}'/src/moc_*' ${ROOTDIR}'/plugins/*/moc_*' \
     ${ROOTDIR}'/libs/*/moc_*' -o ./coverage/coverage.info
genhtml ./coverage/coverage.info --output-directory=./coverage

echo "=========================REPORT COMPLETE==========================="

cd $DIR
echo "OPENING COVERAGE REPORT IN BROWSER"
firefox ../coverage/index.html
