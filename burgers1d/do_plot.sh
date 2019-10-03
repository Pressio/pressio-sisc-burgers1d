#!/bin/bash

set -e

# load global variables
source ${PWD}/common/global_vars.sh

# parse cline arguments
source ${PWD}/common/cmd_line_options_plot.sh

# check that all basic variables are set, otherwise leave
check_minimum_vars_set_plot

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

# check that working exists
if [ ! -d ${WORKINGDIR} ]; then
    echo "non existing ${WORKINGDIR}"
    exit 11
fi
# check that both cpp and python subdirs are there
if [ ! -d ${WORKINGDIR}/cpp ]; then
    echo "non existing ${WORKINGDIR}/cpp"
    exit 11
fi
if [ ! -d ${WORKINGDIR}/python ]; then
    echo "non existing ${WORKINGDIR}/python"
    exit 11
fi

#---------------------------
# plot either lspg or galerkin
#---------------------------
CPPTimingFile=${WORKINGDIR}/cpp/data_${WHICHTASK}/burgers1d_rom_${WHICHTASK}_timings.txt
PYTTimingFile=${WORKINGDIR}/python/data_${WHICHTASK}/burgers1d_rom_${WHICHTASK}_timings.txt

# check the timings files are present
if [ ! -f ${CPPTimingFile} ]; then
    echo "there is not timing C++ data file: ${CPPTimingFile}"
    exit 10
fi
if [ ! -f ${PYTTimingFile} ]; then
    echo "there is no timing Python data file: ${PYTTimingFile}"
    exit 10
fi

# enter there and run
#cd ${destDir}
#python run_rom_timing.py --exe ${EXENAME} --basis-dir-name=${BASISDIRNAME}
#cd ${TOPDIR}
