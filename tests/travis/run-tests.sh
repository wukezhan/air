#!/bin/bash
TEST_DIR="`pwd`/tests/"

cat Makefile

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
        echo -e "\n--------$fail.php --------"
        cat -b $fail.php
        echo -e "\n--------$fail.diff--------"
        cat -b $fail.diff
        echo -e "\n--------$fail.out --------"
        cat -b $fail.out
        echo -e "\n########$fail.phpt########"
    done
    exit 1
else
    exit 0
fi
