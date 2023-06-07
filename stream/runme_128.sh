#!/bin/bash

ulimit -s unlimited

module purge
module load gcc/11.2.0

for THREAD in 1 2 4 8 16
do
    export OMP_NUM_THREADS=${THREAD}
    export OMP_BIND_PROC=true
    CPUAFTY=`echo 128/${THREAD}|bc`
    export GOMP_CPU_AFFINITY=0-127:${CPUAFTY}
    echo $GOMP_CPU_AFFINITY
    ./stream_c.exe > "stream_c_thread_${THREAD}.log"
    #./stream_c.exe > "stream_c_nocpua_thread_${THREAD}.log"
    sleep 5
    ./stream_c_avx2.exe > "stream_c_avx2_thread_${THREAD}.log"
    #./stream_c_avx2.exe > "stream_c_nocpua_avx2_thread_${THREAD}.log"
    sleep 5
done
