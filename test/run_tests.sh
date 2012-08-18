#!/bin/sh
#
#  Compile and run unit tests
#
#  ./run_tests.sh [-v]                         -- compiles and runs all tests
#  ./run_tests.sh [-v] DIR|GROUP               -- compiles and runs tests in one group
#  ./run_tests.sh [-v] DIR|GROUP SOME_FILE.CPP -- compiles and runs only one test in one group
#
#  -v  -- Run tests under valgrind
#  

set -e

CXX="g++"
CXXFLAGS="-g -Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo"
COMPILE="$CXX -I../include -I. $CXXFLAGS -o tests test_utils.cpp"

if [ "x$1" = "x-v" ]; then
    VALGRIND="valgrind --leak-check=full --show-reachable=yes"
    shift
else
    VALGRIND=""
fi

#set -x

if [ "x$1" = "x" ]; then
    for DIR in testgroup_*; do
        GROUP=${DIR##testgroup_}
        echo "\nTesting group $GROUP...\n"
        . $DIR/setup.sh
        FILES="test_main.cpp $DIR/*/test_*.cpp"
        echo $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $VALGRIND ./tests
    done
else
    GROUP=${1##testgroup_}
    DIR=testgroup_$GROUP
    . $DIR/setup.sh
    if [ "x$2" = "x" ]; then
        echo "\nTesting group $GROUP...\n"
        FILES="test_main.cpp $DIR/*/test_*.cpp"
        echo $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $VALGRIND ./tests
    else
        echo "\nTesting file $2 in group $GROUP...\n"
        FILES="-DSTAND_ALONE $DIR/$2"
        echo $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $COMPILE $FILES $FLAGS -lboost_unit_test_framework
        $VALGRIND ./tests
    fi
fi

