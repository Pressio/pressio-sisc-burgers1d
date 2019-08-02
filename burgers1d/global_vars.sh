#!/bin/bash

# top dir where this lives
topDir=${PWD}

# burgers1d cpp source
CPPSRC=${topDir}/cpp/src

# burgers1d python sources
PYSRC=${topDir}/python/src

# store the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/cpp
CPPWORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/python
PYWORKINGDIR=

# bool to wipe existing content of target directory
WIPEEXISTING=0

WHICHTASK=

# env script
SETENVscript=

print_global_vars(){
    echo "TOPDIR	 = $TOPDIR"
    echo "CPPSRC	 = $CPPSRC"
    echo "PYSRC		 = $PYSRC"
    echo "WORKINGDIR     = $WORKINGDIR"
    echo "WIPEEXISTING   = ${WIPEEXISTING}"
    echo "SETENVscript   = $SETENVscript"
}

check_minimum_vars_set(){
    if [ -z $WORKINGDIR ]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 0
    fi
    if [ -z $WHICHTASK ]; then
	echo "--do is empty, must be one of: build, fomTiming, fomBasis, rom"
	exit 0
    fi

}
