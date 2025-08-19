#!/bin/bash

#source $HOME/Tools/root_6.22/bin/thisroot.sh
#source /afs/cern.ch/user/s/shuaiy/setup_root6.sh
#source ./cleanDir.sh

#cmake3 ../
#make
#!/bin/bash

# ROOT
source $HOME/Tools/root_6.22/bin/thisroot.sh
#source /afs/cern.ch/user/s/shuaiy/setup_root6.sh

#!/bin/bash

# Clean build directory
source ./cleanDir.sh

# EvtGen, HepMC3, Pythia8
export EVTGEN=$HOME/Tools/evtgen-install
export HEPMC=$HOME/Tools/hepmc3-install
export PYTHIA8=$HOME/Tools/pythia8303

export LD_LIBRARY_PATH=$EVTGEN/lib:$HEPMC/lib64:$PYTHIA8/lib:$LD_LIBRARY_PATH
export PATH=$EVTGEN/bin:$HEPMC/bin:$PYTHIA8/bin:$PATH

# Run CMake in parent directory
cmake3 ../

# Build
make -j 4

