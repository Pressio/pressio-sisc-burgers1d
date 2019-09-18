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
check_minimum_vars_set

echo ""
echo "--------------------------------------------"
echo " current setting is: "
echo ""
print_global_vars
echo ""
echo "--------------------------------------------"

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
[[ $WIPEEXISTING -eq 1 ]] && rm -rf ${CPPWORKINGDIR}/*


#---------------------------
# build all executables
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${TOPDIR}/build_scripts/build.sh
fi


#------------------------------------------------------------
#
#     Manufactured solition problem
#
# run the FOM manufactured solution problem with eigen or kokkos
# this test is used to check that things are correct
#------------------------------------------------------------
if [[ $WHICHTASK = *"_ms_rk4"* ]] || \
       [[ $WHICHTASK = *"_ms_bdf1"* ]]; then

    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # set name of target executable to use
    EXENAME=
    [[ $WHICHTASK = *"eigen_"* ]] && EXENAME=adr2d_eigen_fom
    [[ $WHICHTASK = *"kokkos_"* ]] && EXENAME=adr2d_kokkos_fom
    # create link to the executable in the build directory
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}
    echo "EXENAME = ${EXENAME}"

    # copy the template input
    cp ${TOPDIR}/src/input.template ${destDir}

    # create links to full meshes
    meshDir=${destDir}/meshes
    if [ ! -d ${destDir}/meshes ]; then
	ln -s ${TOPDIR}/meshes ${destDir}/meshes
    fi

    # copy needed python scripts
    cp ${TOPDIR}/myutils_common.py ${destDir}
    cp ${TOPDIR}/constants_ms.py ${destDir}
    cp ${TOPDIR}/myutils_ms.py ${destDir}
    cp ${TOPDIR}/run_scripts/run_fom_ms.py ${destDir}
    cp ${TOPDIR}/plot_scripts/plot_common.py ${destDir}
    cp ${TOPDIR}/plot_scripts/plot_ms.py ${destDir}

    # enter and run
    cd ${destDir}

    # choose stepper name
    STEPPERNAME=
    [[ $WHICHTASK = *"_ms_rk4"* ]] && STEPPERNAME="RungeKutta4"
    [[ $WHICHTASK = *"_ms_bdf1"* ]] && STEPPERNAME="bdf1"

    python run_fom_ms.py \
    	   --exe ${EXENAME} \
    	   --mesh-dir ${meshDir}\
    	   --stepper-name ${STEPPERNAME}
    cd ${TOPDIR}
fi


#--------------------------------------
#
#     Chemisty adr2d FOM only
#
# either FOM timing or basis generation
#-------------------------------------
if [[ $WHICHTASK == *"_chem_fom_"* ]]; then

    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # set the name of the target executable
    EXENAME=
    [[ $WHICHTASK == *"eigen_chem_"* ]] && EXENAME=adr2d_eigen_fom
    [[ $WHICHTASK == *"kokkos_chem_"* ]] && EXENAME=adr2d_kokkos_fom
    # link the executable from build directory
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}
    echo "EXENAME = ${EXENAME}"

    # copy the template input
    cp ${TOPDIR}/src/input.template ${destDir}

    # link full meshes
    meshDir=${destDir}/meshes
    if [ ! -d ${destDir}/meshes ]; then
	ln -s ${TOPDIR}/meshes ${destDir}/meshes
    fi

    # copy python scripts
    cp ${TOPDIR}/myutils_common.py ${destDir}
    cp ${TOPDIR}/constants_chem.py ${destDir}/
    cp ${TOPDIR}/myutils_chem.py ${destDir}/

    if [[ $WHICHTASK == *"_timing"* ]]; then
	cp ${TOPDIR}/run_scripts/run_fom_timing.py ${destDir}/
    elif [[ $WHICHTASK == *"_basis"* ]];
    then
	# the plotting scripts are only needed when we compute basis
	# because when we do timing, we do not dump snapshots
	cp ${TOPDIR}/plot_scripts/plot_common.py ${destDir}/
	cp ${TOPDIR}/plot_scripts/plot_chem.py ${destDir}/
	cp ${TOPDIR}/run_scripts/run_fom_basis.py ${destDir}/
    else
	echo "error: unrecognized task for WHICHTASK=${WHICHTASK}. Terminating."
	exit 11
    fi

    # enter and run
    cd ${destDir}

    # choose stepper name
    STEPPERNAME=
    [[ $WHICHTASK = *"_rk4"* ]] && STEPPERNAME="RungeKutta4"
    [[ $WHICHTASK = *"_bdf1"* ]] && STEPPERNAME="bdf1"

    # set python driver which depends on the task
    PYTHONEXE=
    [[ $WHICHTASK == *"_timing"* ]] && PYTHONEXE=run_fom_timing.py
    [[ $WHICHTASK == *"_basis"* ]] && PYTHONEXE=run_fom_basis.py

    python ${PYTHONEXE} \
	   --exe ${EXENAME} \
	   --mesh-dir ${meshDir}\
	   --stepper-name ${STEPPERNAME}
    cd ${TOPDIR}
fi


#--------------------------------------
#
#     Chemisty adr2d LSPG FULL mesh
#
#-------------------------------------
if [[ $WHICHTASK == *"_chem_lspg_full_mesh_bdf1_"* ]];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # check if the basis are present (basis are computed with eigen only
    # regardless of whether we use kokkos or eigen to run LSPG)
    BASISDIRNAME=
    [[ $WHICHTASK = *"_bdf1_"* ]] && BASISDIRNAME=data_eigen_chem_fom_bdf1_basis
    if [ ! -d ${CPPWORKINGDIR}/${BASISDIRNAME} ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder for current task inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # link the executable from build directory
    EXENAME=
    if [[ $WHICHTASK == *"eigen_chem_lspg_full_mesh_bdf1_"* ]]; then
	EXENAME=adr2d_eigen_chem_lspg_full_mesh_bdf1
    fi
    if [[ $WHICHTASK == *"kokkos_chem_lspg_full_mesh_bdf1_"* ]]; then
	EXENAME=adr2d_kokkos_chem_lspg_full_mesh_bdf1
    fi
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${TOPDIR}/src/input.template ${destDir}

    # link full meshes directory
    meshDir=${destDir}/meshes
    if [ ! -d ${destDir}/meshes ]; then
    	ln -s ${TOPDIR}/meshes ${destDir}/meshes
    fi

    # copy python scripts
    cp ${TOPDIR}/myutils_common.py ${destDir}/
    cp ${TOPDIR}/myutils_chem.py ${destDir}/
    cp ${TOPDIR}/constants_chem.py ${destDir}/
    cp ${TOPDIR}/plot_scripts/plot_common.py ${destDir}/
    cp ${TOPDIR}/plot_scripts/plot_chem_rom.py ${destDir}/
    cp ${TOPDIR}/run_scripts/run_rom_full_mesh_timing.py ${destDir}

    # enter and run
    cd ${destDir}
    python run_rom_full_mesh_timing.py \
	   --exe ${EXENAME} \
	   --mesh-dir ${meshDir}\
	   --stepper-name bdf1\
	   --basis-dir ${BASISDIRNAME}
    cd ${TOPDIR}
fi



#--------------------------------------
#
#     Chemisty adr2d LSPG SAMPLE mesh
#
#-------------------------------------
if [[ $WHICHTASK == *"_chem_lspg_sample_mesh_bdf1_"* ]];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # check if the basis are present (basis are computed with eigen only
    # regardless of whether we use kokkos or eigen to run LSPG)
    BASISDIRNAME=
    [[ $WHICHTASK = *"_bdf1_"* ]] && BASISDIRNAME=data_eigen_chem_fom_bdf1_basis
    if [ ! -d ${CPPWORKINGDIR}/${BASISDIRNAME} ]; then
	echo "there is not basis dir in the target folder, do that first"
	exit 0
    fi

    # create folder for current task inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # link the executable from build directory
    EXENAME=
    if [[ $WHICHTASK == *"eigen_chem_lspg_sample_mesh_bdf1_"* ]]; then
	EXENAME=adr2d_eigen_chem_lspg_sample_mesh_bdf1
    fi
    if [[ $WHICHTASK == *"kokkos_chem_lspg_sample_mesh_bdf1_"* ]]; then
	EXENAME=adr2d_kokkos_chem_lspg_sample_mesh_bdf1
    fi
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${TOPDIR}/src/input.template ${destDir}

    # link full meshes directory
    meshDir=${destDir}/meshes
    if [ ! -d ${destDir}/meshes ]; then
    	ln -s ${TOPDIR}/meshes ${destDir}/meshes
    fi

    # copy python scripts
    cp ${TOPDIR}/myutils_common.py ${destDir}/
    cp ${TOPDIR}/myutils_chem.py ${destDir}/
    cp ${TOPDIR}/constants_chem.py ${destDir}/
    cp ${TOPDIR}/plot_scripts/plot_common.py ${destDir}/
    cp ${TOPDIR}/plot_scripts/plot_chem_rom.py ${destDir}/
    cp ${TOPDIR}/run_scripts/run_rom_sample_mesh_timing.py ${destDir}

    # enter and run
    cd ${destDir}
    python run_rom_sample_mesh_timing.py \
    	   --exe ${EXENAME} \
    	   --mesh-dir ${meshDir}\
    	   --stepper-name bdf1\
    	   --basis-dir ${BASISDIRNAME}

    cd ${TOPDIR}
fi
