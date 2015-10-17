#!/bin/bash
ptest='./regress_test/';
pcase='./case/';
function usage 
{
    echo "usage: -g or -t or -d";
}

while getopts "gtd" OPTION
do
    case $OPTION in
        g)
            echo "generate golden result";
            for file in $pcase*.ir; do
                _file=`basename $file`;
                ./main $file | grep "schedule" > $ptest$_file.golden;
            done
            exit 1;
            ;;
        t)
            echo "regression test"
            for file in $pcase*.ir; do
                _file=`basename $file`;
                echo "$_file:";
                ./main $file | grep "schedule" > $ptest$_file.result;
                diff $ptest$_file.result $ptest$_file.golden
            done
            exit 1;
            ;;
        d)
            echo "delete all test files"
            rm -f $ptest*
            exit 1;
            ;;
        ?) 
            usage;
            exit 0;
            ;;
    esac
done

