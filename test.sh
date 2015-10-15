#!/bin/bash
function usage 
{
    echo "usage: -g or -t";
}

while getopts "gt" OPTION
do
    case $OPTION in
        g)
            echo "generate golden result";
            for file in *.ir; do
                ./main $file | grep "regress" > regress_test/$file.golden;
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
        ?) 
            usage;
            exit 0;
            ;;
    esac
done

