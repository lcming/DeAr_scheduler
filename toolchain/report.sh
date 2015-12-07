#!/bin/bash
list="add sub mul ashr shl fadd fsub fmul";
for op in $list
do
    cnt=`grep --count "= $op" $1`;
    echo "$op = $cnt";
done
