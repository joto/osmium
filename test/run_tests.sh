#!/bin/sh
#
#  Compile and run unit tests
#
#  ./run_tests.sh [-v]               -- compiles and runs all tests
#  ./run_tests.sh [-v] SOME_FILE.CPP -- compiles and runs only one test
#
#  -v  -- Run tests under valgrind
#  

set -e

if [ -z "$CXX" ]; then
    CXX="c++"
fi

if [ -z "$CXXFLAGS_WARNINGS" ]; then
    CXXFLAGS_WARNINGS="-Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long"
fi

if [ -z "$CXXFLAGS" ]; then
    CXXFLAGS="-g"
fi

COMPILE="$CXX -I../include -I. $CXXFLAGS $CXXFLAGS_WARNINGS -o tests"

if [ "x$1" = "x-v" ]; then
    VALGRIND="valgrind --leak-check=full --show-reachable=yes"
    shift
else
    VALGRIND=""
fi

BOLD="\033[1m"
NORM="\033[0m"
GREEN="\033[1;32m"
DARKRED="\033[31m"
RED="\033[1;31m"

TESTS_COMPILE_ERROR=0
TESTS_FAILED=0
TESTS_OK=0

OPTS_CFLAGS="$(geos-config --cflags) $(gdal-config --cflags)"
OPTS_LIBS="$(geos-config --libs) $(gdal-config --libs) -lboost_regex -lboost_iostreams -lboost_filesystem -lboost_system"

test_file () {
    FILES="test_main.o test_utils.o $1"
    echo -n "Checking $BOLD$1$NORM..."
    if ! output=$($COMPILE $FILES $OPTS_CFLAGS $OPTS_LIBS -DBOOST_TEST_DYN_LINK $LDFLAGS -lboost_unit_test_framework 2>&1 )
    then
        echo "$DARKRED[COMPILE ERROR]$NORM"
        TESTS_COMPILE_ERROR=$(($TESTS_COMPILE_ERROR+1))
        echo "=========================="
        echo $COMPILE $FILES $OPTS_CFLAGS $OPTS_LIBS -DBOOST_TEST_DYN_LINK $LDFLAGS -lboost_unit_test_framework
        echo "--------------------------"
        echo "$output"
        echo "=========================="
        return
    fi

    if ! output=$($VALGRIND ./tests 2>&1 )
    then
        echo "$RED[TEST FAILED]$NORM"
        TESTS_FAILED=$(($TESTS_FAILED+1))
        echo "=========================="
        echo "$output"
        echo "=========================="
        return
    fi
    echo "$GREEN[SUCCESS]$NORM"
    TESTS_OK=$((TESTS_OK+1))
}

setup() {
    if [ \( ! -e test_main.o \) -o \( test_main.cpp -nt test_main.o \) ]
    then
        echo "Compiling test runner"
        $CXX -I../include -I. $CXXFLAGS -DBOOST_TEST_DYN_LINK -c test_main.cpp 
    fi
    if [ \( ! -e test_utils.o \) -o \( test_utils.cpp -nt test_utils.o \) ]
    then
        echo "Compiling test helper"
        $CXX -I../include -I. $CXXFLAGS -DBOOST_TEST_DYN_LINK -c test_utils.cpp
    fi 
}

my_path=`dirname $0`
cd $my_path
setup
if [ "x$1" = "x" ]; then
    for FILE in t/*/test_*.cpp; do
        test_file $FILE
    done
else
    test_file $1
fi

if [ $(($TESTS_COMPILE_ERROR + $TESTS_FAILED)) = 0 ]
then
    echo "all tests succeeded"
    exit 0
else
    echo "some tests failed"
    echo "$TESTS_OK ok, $TESTS_COMPILE_ERROR compile error, $TESTS_FAILED fail"
    exit 1
fi
