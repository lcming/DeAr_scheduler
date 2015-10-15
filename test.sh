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
                ./main $file | grep "regress" > $ptest$_file.golden;
            done
            exit 1;
            ;;
        t)
            echo "regression test"
            for file in *.ir; do
                echo "$file:";
                ./main $file | grep "regress" > regress_test/$file.result;
                diff regress_test/$file.result regress_test/$file.golden
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

