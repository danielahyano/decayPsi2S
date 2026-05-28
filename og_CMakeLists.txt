cmake_minimum_required(VERSION 3.12)
project(jgen)

# Generator sources
set(SRCS
  src/TDecayPolarized.cxx
  src/TGenUPCJpsiFlat.cxx
  src/TGenPsi2S.cxx
)

# Binary generator executables
set(BIN boxgen fdgen)

# Generator library
set(LIB libgen)

# C++ flags
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-Wall -g")

# Include local headers
include_directories(include)

# --- ROOT ---
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_FLAGS_CMD)
string(REPLACE "\n" "" ROOT_FLAGS "${ROOT_FLAGS_CMD}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_FLAGS}")

execute_process(COMMAND root-config --libdir OUTPUT_VARIABLE ROOT_LIBDIR_CMD)
string(REPLACE "\n" "" ROOT_LIBDIR "${ROOT_LIBDIR_CMD}")
link_directories(${ROOT_LIBDIR})
set(ROOT_DEPS Core EG Hist Physics RIO Tree MathCore EGPythia8)

# --- EvtGen, HepMC3, Pythia8 ---
set(EVTGEN_DIR $ENV{EVTGEN})
set(HEPMC_DIR $ENV{HEPMC})
set(PYTHIA8_DIR $ENV{PYTHIA8})

# Library paths
link_directories(${EVTGEN_DIR}/lib ${HEPMC_DIR}/lib64 ${PYTHIA8_DIR}/lib)

# --- Compile and link generator library ---
add_library(${LIB} SHARED ${SRCS})
target_include_directories(${LIB} PUBLIC
    /afs/cern.ch/user/d/dyano/Tools/evtgen-install/include
    /afs/cern.ch/user/d/dyano/Tools/hepmc3-install/include
    /afs/cern.ch/user/d/dyano/Tools/pythia8303/include
)
target_link_libraries(${LIB} 
    /afs/cern.ch/user/d/dyano/Tools/evtgen-install/lib64/libEvtGen.so
    /afs/cern.ch/user/d/dyano/Tools/hepmc3-install/lib64/libHepMC3.so
    /afs/cern.ch/user/d/dyano/Tools/pythia8303/lib/libpythia8.so
    ${ROOT_DEPS}
)

# --- Build executables ---
foreach(IBIN ${BIN})
  add_executable(${IBIN} src/${IBIN}.cxx)
  target_link_libraries(${IBIN} ${LIB})
endforeach()

