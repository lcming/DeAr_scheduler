#!/bin/bash
dear_path="/home/chiming/Desktop/siu";
dear_exe="main";
legacy_path="/home/chiming/Desktop/siu_legacy";
legacy_exe="scheduler";
case_path="$dear_path/case";
msa_config="$legacy_path/msa.config"
ams_config="$legacy_path/ams.config"
printf "%15s     %5s     %5s     %5s     %5s     %5s     %5s     %5s     %5s\n" "benchmark" "#add" "#mul" "#shi" "total" "DeAr" "vliw" "msa" "ams";
for case in `ls $case_path` 
do
    add=`grep -c "ADD\|add\|SUB\|sub" $case_path/$case`
    mul=`grep -c "MUL\|mul" $case_path/$case`
    shi=`grep -c "SHI\|shi" $case_path/$case`
    total=`sed -n '1p' $case_path/$case`
    dear=`$dear_path/$dear_exe $case_path/$case | grep "opc"`
    dear=${dear/"opc = "/};
    vliw=`$legacy_path/$legacy_exe $case_path/$case 1 1 1 1 | grep "opc"`
    vliw=${vliw/"opc = "/};
    msa=`$legacy_path/$legacy_exe $case_path/$case 2 $msa_config 1 | grep "opc"`
    msa=${msa/"opc = "/};
    ams=`$legacy_path/$legacy_exe $case_path/$case 2 $ams_config 1 | grep "opc"`
    ams=${ams/"opc = "/};
    printf "%15s     %5s     %5s     %5s     %5s     %5s     %5s     %5s     %5s\n" $case $add $mul $shi $total $dear $vliw $msa $ams;
done



