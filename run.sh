data_path="data/"
output="output"
loop_times=1
rules_shuffle=1
lookup_round=10
print_mode=0
force_test=0

RunProgram() {
    program_name="$1"
    method_name="$2"
    data="$3"

    rules="${data_path}${data}_rules"
    traces="${data_path}${data}_traces"

    i=0
    while [ $i -lt $loop_times ]
    do
        if [ $loop_times -gt 1 ]; then
            echo "  \c" >> ${output}
        else
            echo "${program_name}\t${data}\t\c" >> ${output}
        fi
        ./main --run_mode classification --method_name ${method_name} \
               --rules_file ${rules} --traces_file ${traces} \
               --rules_shuffle ${rules_shuffle} --lookup_round ${lookup_round} \
               --force_test ${force_test} --print_mode ${print_mode} ${4} ${5} ${6} ${7} ${8} ${9} ${10} ${11} ${12} ${13} ${14} ${15} ${16} ${17} ${18} ${19} ${20} ${21} ${22} ${24} ${25} ${26} ${27} ${28} ${29} ${30} \
               # >> ${output}
        i=`expr $i + 1`
    done
    
    if [ $loop_times -gt 1 ]; then
        echo "${program_name} ${data}" >> ${output}
    fi
}

RunMethod() {
    for data_size in 10K 
    do
        for data_type in acl1 # acl2 acl3 acl4 acl5 fw1 fw2 fw3 fw4 fw5 ipc1 ipc2
        do
            data=${data_size}_${data_type}
            echo "${data}" >> ${output}

            for prefix_dims_num in 5 # 2
            do
                RunProgram    DimTSS             DimTSS             ${data} --prefix_dims_num ${prefix_dims_num}
                RunProgram    TupleMerge         TupleMerge         ${data} --prefix_dims_num ${prefix_dims_num}
                RunProgram    PartitionSort      PartitionSort      ${data} --prefix_dims_num ${prefix_dims_num}
                RunProgram    MultilayerTuple    MultilayerTuple    ${data} --prefix_dims_num ${prefix_dims_num}

            done
        done
    done
}

make
rm -rf ${output}
date >> ${output}

echo "method_name data_set \
rules_num traces_num \
data_memory_size(MB) index_memory_size(MB) \
build_time(S) \
lookup_speed(MLPS) \
insert_speed(MUIPS) \
delete_speed(MUPS) \
update_speed(MUPS) \
tuples_num tuples_sum \
access_tuples_sum access_tuples_avg access_tuples_max \
access_rules_sum access_rules_avg access_rules_max \
next_layer_num" >> ${output}

RunMethod

date >> ${output}
