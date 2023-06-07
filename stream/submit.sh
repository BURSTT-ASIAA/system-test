#!/bin/bash
###### Job name ######
#PBS -N stream
###### Output files ######
#PBS -o stream.out
#PBS -e stream.err
###### Number of nodes and cores ######
#PBS -l nodes=1:ppn=128
###### Specify how many hours do you need ######
#PBS -l walltime=01:00:00
###### Queue name ######
#PBS -q ctest
###### Sandbox ######
#PBS -W sandbox=PRIVATE
###### Specific the shell types ######
#PBS -S /bin/bash

###### Enter this job's working directory ######
cd $PBS_O_WORKDIR

###### Load modules to setup environment ######
. /etc/profile.d/modules.sh
module purge
module load gcc/11.2.0

./runme.sh
