#!/bin/bash
TEST_DIR="`pwd`/tests/"

make test

for file in `find $TEST_DIR -name "*.diff" 2>/dev/null`
do
        FAILS[${#FAILS[@]}]=${file%%.diff*}
done

if [ ${#FAILS[@]} -gt 0 ]
then
    for fail in "${FAILS[@]}"
    do
        echo "========$fail.phpt========"
        echo "--------$fail.diff--------"
        cat $fail.diff
        echo "--------$fail.out --------"
        cat $fail.out
        echo "########$fail.phpt########"
    done
    exit 1
else
    exit 0
fi
