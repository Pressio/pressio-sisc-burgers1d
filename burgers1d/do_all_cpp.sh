#!/bin/bash

# -----------------------------------------------
# handle all steps for the C++ Burgers1d
# -----------------------------------------------

export OPENBLAS_NUM_THREADS=1

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

#---------------------------
# fom timing
#---------------------------
if [ $WHICHTASK = "fomBdf1Timing" ] || [ $WHICHTASK = "fomRk4Timing" ];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    mkdir ${destDir}

    # link the executable from build directory
    EXENAME=
    [[ $WHICHTASK = "fomBdf1Timing" ]] && EXENAME=burgers1d_fom_bdf1
    [[ $WHICHTASK = "fomRk4Timing" ]] && EXENAME=burgers1d_fom_rk4
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # copy python scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_fom_timing.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # enter and run
    cd ${destDir}
    python run_fom_timing.py --exe ${EXENAME}
    cd ${topDir}
fi

#---------------------------
# fom to generate basis
#---------------------------
if [ $WHICHTASK = "fomBdf1Basis" ] || [ $WHICHTASK = "fomRk4Basis" ];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    mkdir ${destDir}

    # link the executable from build directory
    EXENAME=
    [[ $WHICHTASK = "fomBdf1Basis" ]] && EXENAME=burgers1d_fom_bdf1
    [[ $WHICHTASK = "fomRk4Basis" ]] && EXENAME=burgers1d_fom_rk4
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # copy all pything scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_fom_basis.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # enter there and run
    cd ${destDir}
    python run_fom_basis.py --exe ${EXENAME}
    cd ${topDir}
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
    [[ $WHICHTASK = "lspg" ]] && BASISDIRNAME=fomBdf1Basis
    [[ $WHICHTASK = "galerkin" ]] && BASISDIRNAME=fomRk4Basis
    if [ ! -d ${CPPWORKINGDIR}/"data_"${BASISDIRNAME} ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    mkdir ${destDir}

    # link the executable
    EXENAME=
    [[ $WHICHTASK = "lspg" ]] && EXENAME=burgers1d_rom_lspg
    [[ $WHICHTASK = "galerkin" ]] && EXENAME=burgers1d_rom_galerkin
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/cpp/src/input.template ${destDir}

    # copy all python scripts there
    cp ${topDir}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${topDir}/cpp/run_scripts/run_rom_timing.py ${destDir}/
    cp ${topDir}/common/*.py ${destDir}/

    # enter there and run
    cd ${destDir}
    python run_rom_timing.py --exe ${EXENAME} --basis-dir-name=${BASISDIRNAME}
    cd ${topDir}
fi
