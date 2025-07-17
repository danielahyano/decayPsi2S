#!/bin/bash

#source $HOME/Tools/root_6.22/bin/thisroot.sh
#source /afs/cern.ch/user/s/shuaiy/setup_root6.sh
echo $PWD

# Extract libraries
#tar -xzvf libs.tar.gz -C $PWD

# Set LD_LIBRARY_PATH
#export LD_LIBRARY_PATH="$PWD/usr/lib64:$LD_LIBRARY_PATH"

#workDir=$1
#inFile=$2
#outFile=$3

#cd $workDir

inFile=$1
outFile=$2

./fdgen $inFile $outFile
