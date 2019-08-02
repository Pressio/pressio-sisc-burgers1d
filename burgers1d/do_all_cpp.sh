#!/bin/bash

# -----------------------------------------------
# handle all steps for the C++ Burgers1d
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
CPPWORKINGDIR=${WORKINGDIR}/cpp
[[ ! -d ${CPPWORKINGDIR} ]] && mkdir ${CPPWORKINGDIR}

# wipe everything if set to 1
[[ $WIPEEXISTING = 1 ]] && rm -rf ${CPPWORKINGDIR}/*

#---------------------------
# only build all exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${topDir}/cpp/build_scripts/build.sh
fi

#---------------------------
# only run fom timing
#---------------------------
if [ $WHICHTASK = "fomTiming" ]; then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/fom_timings
    mkdir ${destDir}
    # copy all pything scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_fom_timing.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # link the executable
    ln -s ${CPPWORKINGDIR}/build/burgers1d_fom ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # enter there and run
    cd ${destDir}
    python run_fom_timing.py
    cd ${topDir}
fi

#---------------------------
# run fom to generate basis
#---------------------------
if [ $WHICHTASK = "fomBasis" ]; then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/fom_basis
    mkdir ${destDir}
    # copy all pything scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_fom_basis.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # link the executable
    ln -s ${CPPWORKINGDIR}/build/burgers1d_fom ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # enter there and run
    cd ${destDir}
    python run_fom_basis.py
    cd ${topDir}
fi


#---------------------------
# rom
#---------------------------
if [ $WHICHTASK = "rom" ]; then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi
    # check if the basis are present
    if [ ! -d ${CPPWORKINGDIR}/fom_basis ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/rom_timings
    mkdir ${destDir}
    # copy all pything scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_rom_timing.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # link the executable
    ln -s ${CPPWORKINGDIR}/build/burgers1d_rom ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # enter there and run
    cd ${destDir}
    python run_rom_timing.py
    cd ${topDir}
fi
