#!/bin/sh
#
#  Compile and run unit tests
#
#  ./run_tests.sh               -- compiles and runs all tests
#  ./run_tests.sh some_file.cpp -- compiles and runs only one test
#  

CXX="g++"
CXXFLAGS="-g -Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo"

if [ "x$1" = "x" ]; then
    FILES="test_main.cpp */test_*.cpp utils.cpp"
else
    FILES="-DSTAND_ALONE $1 utils.cpp"
fi

set -e
set -x

$CXX $FILES -I../include $CXXFLAGS -lboost_unit_test_framework -o tests

./tests

