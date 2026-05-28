#!/bin/bash

#source $HOME/Tools/root_6.22/bin/thisroot.sh
#source /afs/cern.ch/user/s/shuaiy/setup_root6.sh
#source ./cleanDir.sh

#cmake3 ../
#make
#!/bin/bash

# ROOT
#source $HOME/Tools/root_6.22/bin/thisroot.sh
#source /afs/cern.ch/user/s/shuaiy/setup_root6.sh

#!/bin/bash

# Clean build directory
#source ./cleanDir.sh

# EvtGen, HepMC3, Pythia8
#export EVTGEN=$HOME/Tools/evtgen-install
#export HEPMC=$HOME/Tools/hepmc3-install
#export PYTHIA8=$HOME/Tools/pythia8303

#export LD_LIBRARY_PATH=$EVTGEN/lib64:$HEPMC/lib64:$PYTHIA8/lib:$LD_LIBRARY_PATH
#export PATH=$EVTGEN/bin:$HEPMC/bin:$PYTHIA8/bin:$PATH
#export EVTGEN=$(scram tool tag evtgen EVTGEN_BASE)
#export HEPMC=$(scram tool tag hepmc3 HEPMC3_BASE)
#export PYTHIA8=$(scram tool tag pythia8 PYTHIA8_BASE)

#export LD_LIBRARY_PATH=$EVTGEN/lib:$HEPMC/lib:$PYTHIA8/lib:$LD_LIBRARY_PATH

#cmake3 ../ \
#  -DEVTGEN_ROOT_DIR=$EVTGEN \
#  -DHEPMC3_ROOT_DIR=$HEPMC \
#  -DPYTHIA8_ROOT_DIR=$PYTHIA8
# Run CMake in parent directory
#cmake3 ../

# Build
#make -j 4
#!/bin/bash

# Move to CMSSW area so scram works, then come back
SCRIPT_DIR=$(dirname $(readlink -f $BASH_SOURCE))
cd $CMSSW_BASE/src
export CC=$(which gcc)
export CXX=$(which g++)
export EVTGEN=$(scram tool tag evtgen EVTGEN_BASE)
export HEPMC=$(scram tool tag hepmc3 HEPMC3_BASE)
export PYTHIA8=$(scram tool tag pythia8 PYTHIA8_BASE)

export LD_LIBRARY_PATH=$EVTGEN/lib:$HEPMC/lib:$PYTHIA8/lib:$LD_LIBRARY_PATH

cd $SCRIPT_DIR

cmake3 ../ \
  -DEVTGEN_ROOT_DIR=$EVTGEN \
  -DHEPMC3_ROOT_DIR=$HEPMC \
  -DPYTHIA8_ROOT_DIR=$PYTHIA8

make -j 4

