#!/bin/bash

# -----------------------------------------------
# purpose: handle all steps for the C++ Burgers1d
# -----------------------------------------------

# set environment in some way, as long as CC,CXX are set
envScript=/Users/fnrizzi/Desktop/work/ROM/setenv_ompi400_clang700.sh
source ${envScript}

# top dir where this lives
topDir=${PWD}

# burgers1d cpp source
CPPSRC=${topDir}/cpp/src

# the working directory where everything will be put
WORKINGDIR=

# bool to wipe existing content of target directory
WIPEEXISTING=0

WHICHTASK=

# parse cline arguments
source cmd_line_options.sh

if [ -z $WORKINGDIR ]; then
    echo "--working-dir is empty, must be set: exiting"
    exit 0
fi

# create dir if not existing
[[ ! -d ${WORKINGDIR} ]] && mkdir ${WORKINGDIR}

# wipe everything if set to 1
[[ $WIPEEXISTING = 1 ]] && rm -rf ${WORKINGDIR}/*

#---------------------
# only build all exes
if [ $WHICHTASK = "buildOnly" ]; then
    source ${topDir}/cpp/build_scripts/build_cpp.sh
fi

#---------------------
# only run fom timing
if [ $WHICHTASK = "fomTiming" ]; then
    # create folder inside workindir
    destDir=${WORKINGDIR}/fom_timings
    mkdir ${destDir}
    # copy all pything scripts there
    cp ${topDir}/cpp/run_scripts/*.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # copy the executable
    cp ${WORKINGDIR}/build/burgers1d_fom ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # enter there and run
    cd ${destDir}
    python run_fom_timing.py
    cd ${topDir}
fi
