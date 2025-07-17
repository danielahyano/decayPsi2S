#!/bin/bash

if [ ! -d lheFiles ]; then
    mkdir -p lheFiles
fi

if [ "`ls -A lheFiles`" != "" ]; then
    rm -rf lheFiles/*
fi

cmsEnergyDiv2=2680

#for inputFile in `ls ./splitFiles/slight_CohPsi2S_*`
for inputFile in `ls /afs/cern.ch/user/d/dyano/private/mcPsi2s/decayPsi2S/build/splitFiles/*`
do
    echo $inputFile

    baseFileName=`basename $inputFile`

    outputFile=${baseFileName%.tx}  # remove `.tx`

    echo $outputFile

    root -l -b << EOF
    .x convert_SL2LHE.C+("$inputFile", "/afs/cern.ch/user/d/dyano/private/mcPsi2s/decayPsi2S/build/lheFiles/$outputFile",$cmsEnergyDiv2,$cmsEnergyDiv2)
    .q
EOF

done
