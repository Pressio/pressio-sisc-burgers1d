#!/bin/bash

set -e

# -----------------------------------------------
# manage all steps/cases for the C++ AdvDiffReac2d
# -----------------------------------------------

# load global variables
source ${PWD}/global_vars.sh

# parse cline arguments
source ${PWD}/cmd_line_options.sh

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
# build exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${topDir}/build_scripts/build.sh
fi

#---------------------------------------------------------
# eigen_ms: run the manufactured solution test with eigen
#---------------------------------------------------------
if [ $WHICHTASK = "eigen_ms_rk4" ] || [ $WHICHTASK = "eigen_ms_bdf1" ];
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
    [[ $WHICHTASK = "eigen_ms_rk4" ]] && EXENAME=adr2d_eigen_ms_fom_rk4
    [[ $WHICHTASK = "eigen_ms_bdf1" ]] && EXENAME=adr2d_eigen_ms_fom_bdf1
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/src/input.template ${destDir}

    # link meshes directory
    meshDir=${destDir}/full_meshes
    if [ ! -d ${destDir}/full_meshes ]; then
	ln -s ${topDir}/full_meshes ${destDir}/full_meshes
    fi

    # copy python scripts
    cp ${topDir}/run_scripts/run_fom_ms.py ${destDir}/
    cp ${topDir}/run_scripts/myutils_ms.py ${destDir}/
    cp ${topDir}/constants_ms.py ${destDir}/
    cp ${topDir}/plot_scripts/plot_ms.py ${destDir}/
    cp ${topDir}/plot_scripts/plot_common.py ${destDir}/

    # enter and run
    cd ${destDir}
    python run_fom_ms.py --exe ${EXENAME} --mesh-dir ${meshDir}
    cd ${topDir}
fi


#--------------------------------------
# chem problem, FOM timing or basis
#-------------------------------------
if [[ $WHICHTASK == *"eigen_chem_fom_"* ]];
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
    if [[ $WHICHTASK == *"eigen_chem_fom_rk4_"* ]]; then
	EXENAME=adr2d_eigen_chem_fom_rk4
    fi
    if [[ $WHICHTASK == *"eigen_chem_fom_bdf1_"* ]]; then
	EXENAME=adr2d_eigen_chem_fom_bdf1
    fi
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/src/input.template ${destDir}

    # link meshes directory
    meshDir=${destDir}/full_meshes
    if [ ! -d ${destDir}/full_meshes ]; then
	ln -s ${topDir}/full_meshes ${destDir}/full_meshes
    fi

    # copy python scripts
    cp ${topDir}/run_scripts/myutils_chem.py ${destDir}/
    cp ${topDir}/constants_chem.py ${destDir}/
    cp ${topDir}/plot_scripts/plot_common.py ${destDir}/
    if [[ $WHICHTASK == *"_timing"* ]]; then
	cp ${topDir}/run_scripts/run_fom_timing.py ${destDir}/
    fi
    if [[ $WHICHTASK == *"_basis"* ]]; then
	cp ${topDir}/plot_scripts/plot_chem.py ${destDir}/
	cp ${topDir}/run_scripts/run_fom_basis.py ${destDir}/
    fi

    # enter and run
    cd ${destDir}

    PYTHONEXE=
    if [[ $WHICHTASK == *"_timing"* ]]; then
	PYTHONEXE=run_fom_timing.py
    fi
    if [[ $WHICHTASK == *"_basis"* ]]; then
	PYTHONEXE=run_fom_basis.py
    fi
    python ${PYTHONEXE} --exe ${EXENAME} --mesh-dir ${meshDir}
    cd ${topDir}
fi
