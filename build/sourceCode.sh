#!/bin/bash

#source $HOME/Tools/root_6.22/bin/thisroot.sh
source /afs/cern.ch/user/s/shuaiy/setup_root6.sh
source ./cleanDir.sh

cmake3 ../
make
