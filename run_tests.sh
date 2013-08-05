#!/bin/bash

echo "Build and run all the tests"
echo

result=0

for testcode in `find test -name *.pro`
do
    dir=$(dirname $testcode)
    file=$(basename $testcode .pro)

    echo "#############################################################"
    echo "### "$testcode $dir $file

    pushd $dir || exit 10
    if [ -f Makefile ]
    then
        make distclean
    fi

    qmake || exit 11
    make -j`getconf _NPROCESSORS_ONLN` || exit 12
    ./$file || result=1 ; echo "\n\n FAIL \n\n"
    make distclean
    popd
    echo
done

exit $result
