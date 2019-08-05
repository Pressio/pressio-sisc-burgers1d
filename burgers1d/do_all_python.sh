#!/bin/bash

# -----------------------------------------------
# handle all steps for the Python Burgers1d
# -----------------------------------------------

# load global variables
source ${PWD}/common/global_vars.sh

# parse cline arguments
source ${PWD}/common/cmd_line_options.sh

# check that all basic variables are set, otherwise leave
check_minimum_vars_set_python

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
# rom: lspg or galerkin
#---------------------------
if [ $WHICHTASK = "lspg" ] || [ $WHICHTASK = "galerkin" ];
then
    # check if the build was already done
    if [ ! -d ${PYWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # check if the basis are present
    BASISDIRNAME=
    [[ $WHICHTASK = "lspg" ]] && BASISDIRNAME=fomBdf1Basis
    [[ $WHICHTASK = "galerkin" ]] && BASISDIRNAME=fomRk4Basis
    if [ ! -d ${WORKINGDIR}/cpp/data_${BASISDIRNAME} ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${PYWORKINGDIR}/"data_"${WHICHTASK}
    mkdir ${destDir}

    # alias the name of target executable
    EXENAME=
    [[ $WHICHTASK = "lspg" ]] && EXENAME=main_rom_lspg
    [[ $WHICHTASK = "galerkin" ]] && EXENAME=main_rom_galerkin

    # link the bindings library
    ln -s ${PYWORKINGDIR}/build/pressio4py.so ${destDir}
    if [ $WHICHTASK = "lspg" ]; then
	ln -s ${PYWORKINGDIR}/build/pressio4pyLspg.so ${destDir}
    fi
    if [ $WHICHTASK = "galerkin" ]; then
	ln -s ${PYWORKINGDIR}/build/pressio4pyGalerkin.so ${destDir}
    fi
    # link the ops
    ln -s ${PYWORKINGDIR}/build/pressio4pyOps.py ${destDir}

    # copy all pything scripts there
    cp ${topDir}/common/constants.py ${destDir}/
    cp ${topDir}/python/run_scripts/run_rom_timing.py ${destDir}/
    cp ${topDir}/python/src/burgers1d.py ${destDir}/
    if [ $WHICHTASK = "lspg" ]; then
	cp ${topDir}/python/src/main_rom_lspg.py ${destDir}/
    fi
    if [ $WHICHTASK = "galerkin" ]; then
	cp ${topDir}/python/src/main_rom_galerkin.py ${destDir}/
    fi

    # enter there and run
    cd ${destDir}
    python run_rom_timing.py --exe=${EXENAME} --basis-dir-name=${BASISDIRNAME}
    cd ${topDir}
fi
