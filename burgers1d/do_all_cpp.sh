#!/bin/bash

set -e

# load global variables
source ${PWD}/common/global_vars_cpp.sh

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
[[ $WIPEEXISTING == yes  ]] && wipe_existing_data_in_target_dir ${CPPWORKINGDIR}

#---------------------------
# build c++ exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${TOPDIR}/cpp/build_scripts/build.sh
fi

#---------------------------
# fom timing or basis
#---------------------------
if [[ $WHICHTASK == *"fom"* ]];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # link the executable from build directory
    EXENAME=
    [[ $WHICHTASK == *"fom_bdf1"* ]] && EXENAME=burgers1d_fom_bdf1
    [[ $WHICHTASK == *"fom_rk4"* ]] && EXENAME=burgers1d_fom_rk4
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${TOPDIR}/cpp/src/input.template ${destDir}

    # copy python scripts there
    cp ${TOPDIR}/cpp/run_scripts/myutils.py ${destDir}/

    cp ${TOPDIR}/common/*.py ${destDir}/

    if [[ $WHICHTASK == *"timing"* ]]; then
	cp ${TOPDIR}/cpp/run_scripts/run_fom_timing.py ${destDir}/
    elif [[ $WHICHTASK == *"basis"* ]];
    then
	cp ${TOPDIR}/cpp/run_scripts/run_fom_basis.py ${destDir}/
    else
	echo "error: unrecognized task for WHICHTASK=${WHICHTASK}. Terminating."
	exit 11
    fi

    # set python driver which depends on the task
    PYTHONEXE=
    [[ $WHICHTASK == *"timing"* ]] && PYTHONEXE=run_fom_timing.py
    [[ $WHICHTASK == *"basis"* ]] && PYTHONEXE=run_fom_basis.py

    # enter and run
    cd ${destDir}
    python ${PYTHONEXE} --exe ${EXENAME}
    cd ${TOPDIR}
fi


#---------------------------
# rom: lspg or galerkin
#---------------------------
if [ $WHICHTASK = "lspg" ] || [ $WHICHTASK = "galerkin" ];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # check if the basis are present
    BASISDIRNAME=
    [[ $WHICHTASK = "lspg" ]] && BASISDIRNAME=fom_bdf1_basis
    [[ $WHICHTASK = "galerkin" ]] && BASISDIRNAME=fom_rk4_basis
    if [ ! -d ${CPPWORKINGDIR}/"data_"${BASISDIRNAME} ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # link the executable
    EXENAME=
    [[ $WHICHTASK = "lspg" ]] && EXENAME=burgers1d_rom_lspg
    [[ $WHICHTASK = "galerkin" ]] && EXENAME=burgers1d_rom_galerkin
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${TOPDIR}/cpp/src/input.template ${destDir}

    # copy all python scripts there
    cp ${TOPDIR}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${TOPDIR}/cpp/run_scripts/run_rom_timing.py ${destDir}/
    cp ${TOPDIR}/common/*.py ${destDir}/

    # enter there and run
    cd ${destDir}
    python run_rom_timing.py --exe ${EXENAME} --basis-dir-name=${BASISDIRNAME}
    cd ${TOPDIR}
fi
