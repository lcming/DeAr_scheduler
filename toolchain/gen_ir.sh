#!/bin/bash
fullfile=$1;
filename="${fullfile%.*}";
clang -S -emit-llvm -target mips-none-none "$1" -o tmp.txt; 
opt -O3 -instcombine -unroll-threshold=999999 tmp.txt -S -o tmp1.txt; 
sed 's/nocapture readonly //g' tmp1.txt > tmp2.txt;
graph-llvm-ir tmp2.txt;
echo "digraph G {" > $filename.dot;
echo "compound=true" >> $filename.dot;
grep -v "red\|\"_\|^\"arrayidx\|^\"i\|^\"output" tmp.dot | grep "^\"sum\|^\"sub\|^\"add\|^\"mul\|^\"sh\|^\"t" >> $filename.dot;
echo "}" >> $filename.dot;
./report.sh $filename.dot;
dot $filename.dot -Tpdf -o $filename.pdf;
rm tmp*;
./parser.pl $filename.dot







