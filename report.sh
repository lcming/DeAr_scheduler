#!/bin/bash
dear_path="/home/lcming/siu";
dear_exe="main";
legacy_path="/home/lcming/siu_legacy";
legacy_exe="scheduler";
blas_path="$dear_path/blas";
general_path="$dear_path/general";
msa_config="$legacy_path/msa.config"
ams_config="$legacy_path/ams.config"
printf "%15s     %5s     %5s     %5s     %5s     %5s     %5s     %5s     %5s\n" "benchmark" "#add" "#mul" "#shi" "total" "DeAr" "vliw" "msa" "ams";
printf "======================================= BLAS =========================================\n";
for case in `ls $blas_path` 
do
    add=`grep -c "ADD\|add\|SUB\|sub" $blas_path/$case`
    mul=`grep -c "MUL\|mul" $blas_path/$case`
    shi=`grep -c "SHI\|shi" $blas_path/$case`
    total=`sed -n '1p' $blas_path/$case`
    dear=`$dear_path/$dear_exe $blas_path/$case | grep "opc"`
    dear=${dear/"opc = "/};
    vliw=`$legacy_path/$legacy_exe $blas_path/$case 1 1 1 1 | grep "opc"`
    vliw=${vliw/"opc = "/};
    msa=`$legacy_path/$legacy_exe $blas_path/$case 2 $msa_config 1 | grep "opc"`
    msa=${msa/"opc = "/};
    ams=`$legacy_path/$legacy_exe $blas_path/$case 2 $ams_config 1 | grep "opc"`
    ams=${ams/"opc = "/};
    printf "%15s     %5s     %5s     %5s     %5s     %5s     %5s     %5s     %5s\n" $case $add $mul $shi $total $dear $vliw $msa $ams;
done

printf "==================================== GENERAL  =========================================\n";
for case in `ls $general_path` 
do
    add=`grep -c "ADD\|add\|SUB\|sub" $general_path/$case`
    mul=`grep -c "MUL\|mul" $general_path/$case`
    shi=`grep -c "SHI\|shi" $general_path/$case`
    total=`sed -n '1p' $general_path/$case`
    dear=`$dear_path/$dear_exe $general_path/$case | grep "opc"`
    dear=${dear/"opc = "/};
    vliw=`$legacy_path/$legacy_exe $general_path/$case 1 1 1 1 | grep "opc"`
    vliw=${vliw/"opc = "/};
    msa=`$legacy_path/$legacy_exe $general_path/$case 2 $msa_config 1 | grep "opc"`
    msa=${msa/"opc = "/};
    ams=`$legacy_path/$legacy_exe $general_path/$case 2 $ams_config 1 | grep "opc"`
    ams=${ams/"opc = "/};
    printf "%15s     %5s     %5s     %5s     %5s     %5s     %5s     %5s     %5s\n" $case $add $mul $shi $total $dear $vliw $msa $ams;
done




