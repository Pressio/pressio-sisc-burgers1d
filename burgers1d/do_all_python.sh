#!/bin/bash

# -----------------------------------------------
# handle all steps for the Python Burgers1d
# -----------------------------------------------

# load global variables
source global_vars.sh

# parse cline arguments
source cmd_line_options.sh

# check that all basic variables are set, otherwise leave
check_minimum_vars_set

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
PYWORKINGDIR=${WORKINGDIR}/python
[[ ! -d ${PYWORKINGDIR} ]] && mkdir ${PYWORKINGDIR}

# wipe everything if set to 1
[[ $WIPEEXISTING = 1 ]] && rm -rf ${PYWORKINGDIR}/*

#---------------------------
# only build
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${topDir}/python/build_scripts/build.sh
fi

#---------------------------
# rom
#---------------------------
if [ $WHICHTASK = "rom" ]; then
    # check if the build was already done
    if [ ! -d ${PYWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi
    # check if the basis are present
    if [ ! -d ${WORKINGDIR}/cpp/fom_basis ]; then
	echo "there is not basis dir in the target folder"
	echo "you need to create basis using cpp code"
	exit 0
    fi

    # create folder inside workindir
    destDir=${PYWORKINGDIR}/rom_timings
    mkdir ${destDir}
    # copy all pything scripts there
    cp ${topDir}/common/*.py ${destDir}/
    cp ${topDir}/python/run_scripts/run_rom_timing.py ${destDir}/
    cp ${topDir}/python/src/main_rom.py ${destDir}/
    cp ${topDir}/python/src/burgers1d.py ${destDir}/

    # link the bindings library
    ln -s ${PYWORKINGDIR}/build/pressio4py.so ${destDir}
    # link the ops
    ln -s ${PYWORKINGDIR}/build/pressio4pyOps.py ${destDir}

    # enter there and run
    cd ${destDir}
    python run_rom_timing.py
    cd ${topDir}
fi
