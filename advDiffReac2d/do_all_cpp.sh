#!/bin/bash

set -e

# -----------------------------------------------
# handle all steps for the C++ AdvDiffReac2d
# -----------------------------------------------

# load global variables
source ${PWD}/common/global_vars.sh

# parse cline arguments
source ${PWD}/common/cmd_line_options.sh

# check that all basic variables are set, otherwise leave
check_minimum_vars_set_cpp

echo ""
echo "--------------------------------------------"
echo " current setting is: "
echo ""
print_global_vars

# set env if not already set
if [[ ! -z ${SETENVscript} ]]; then
    echo "loading environment from ${SETENVscript}"
    source ${SETENVscript}
    echo "PATH = $PATH"
else
    echo "--with-env-script NOT set, so we assume env is set already"
fi

# create working dir if not existing
[[ ! -d ${WORKINGDIR} ]] && mkdir ${WORKINGDIR}

# create subworking dir if not existing
CPPWORKINGDIR=${WORKINGDIR}/cpp
[[ ! -d ${CPPWORKINGDIR} ]] && mkdir ${CPPWORKINGDIR}

# wipe everything if set to 1
[[ $WIPEEXISTING = 1 ]] && rm -rf ${CPPWORKINGDIR}/*

#---------------------------
# do build all c++ exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${topDir}/cpp/build_scripts/build.sh
fi
