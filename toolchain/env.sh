function opt_ll { 
    opt -O3 -unroll-threshold=999999 -debug  "$1" -S -o "$2"; 
    sed 's/nocapture readonly //g' $2 > tmp.txt
    mv tmp.txt $2
}
export -f opt_ll

function clang_mips { clang -S -emit-llvm -target mips-none-none "$1" -o "$2"; }
export -f clang_mips

function ll2pdf { 
    if [ $# -gt 2 ]
    then
        graph-llvm-ir "$3" "$1";
    else
        graph-llvm-ir "$1";
    fi
    dot tmp.dot -Tpdf -o "$2";
    rm tmp.dot; }
export -f ll2pdf


