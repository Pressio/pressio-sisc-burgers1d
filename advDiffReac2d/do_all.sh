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
[[ $WIPEEXISTING = 1 ]] && rm -rf ${CPPWORKINGDIR}/*

#---------------------------
# build exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${topDir}/build_scripts/build.sh
fi

#---------------------------------------------------------
# the manufactured solution test with eigen or kokkos
#---------------------------------------------------------
if [[ $WHICHTASK = *"_ms_rk4"* ]] ||\
       [[ $WHICHTASK = "_ms_bdf1" ]];
then
    # check if the build was already done
    if [ ! -d ${CPPWORKINGDIR}/build ]; then
	echo "there is no build in the target folder, do that first"
	exit 0
    fi

    # create folder inside workindir
    destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # set name of target executable to use
    EXENAME=adr2d_eigen_fom
    [[ $WHICHTASK = *"eigen_"* ]] && EXENAME=adr2d_eigen_fom
    [[ $WHICHTASK = *"kokkos_"* ]] && EXENAME=adr2d_kokkos_fom
    # link the executable from build directory
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${topDir}/src/input.template ${destDir}

    # link full meshes
    meshDir=${destDir}/full_meshes
    if [ ! -d ${destDir}/full_meshes ]; then
	ln -s ${topDir}/full_meshes ${destDir}/full_meshes
    fi

    # copy needed python scripts
    cp ${topDir}/constants_ms.py ${destDir}
    cp ${topDir}/myutils_common.py ${destDir}
    cp ${topDir}/myutils_ms.py ${destDir}
    cp ${topDir}/run_scripts/run_fom_ms.py ${destDir}
    cp ${topDir}/plot_scripts/plot_common.py ${destDir}
    cp ${topDir}/plot_scripts/plot_ms.py ${destDir}

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
    cd ${topDir}
fi


#--------------------------------------
# chem problem, FOM timing or basis
#-------------------------------------
if [[ $WHICHTASK == *"_chem_fom_"* ]];
then
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

    # copy the template input
    cp ${topDir}/src/input.template ${destDir}

    # link full meshes
    meshDir=${destDir}/full_meshes
    if [ ! -d ${destDir}/full_meshes ]; then
	ln -s ${topDir}/full_meshes ${destDir}/full_meshes
    fi

    # copy python scripts
    cp ${topDir}/constants_chem.py ${destDir}/
    cp ${topDir}/myutils_common.py ${destDir}
    cp ${topDir}/myutils_chem.py ${destDir}/

    if [[ $WHICHTASK == *"_timing"* ]]; then
	cp ${topDir}/run_scripts/run_fom_timing.py ${destDir}/
    fi
    if [[ $WHICHTASK == *"_basis"* ]]; then
	# the plotting scripts are only needed when we compute basis
	# because when we do timing, we do not dump snapshots
	cp ${topDir}/plot_scripts/plot_common.py ${destDir}/
	cp ${topDir}/plot_scripts/plot_chem.py ${destDir}/
	cp ${topDir}/run_scripts/run_fom_basis.py ${destDir}/
    fi

    # enter and run
    cd ${destDir}

    # choose stepper name
    STEPPERNAME=
    [[ $WHICHTASK = *"_rk4"* ]] && STEPPERNAME="RungeKutta4"
    [[ $WHICHTASK = *"_bdf1"* ]] && STEPPERNAME="bdf1"

    # set python exe
    PYTHONEXE=
    [[ $WHICHTASK == *"_timing"* ]] && PYTHONEXE=run_fom_timing.py
    [[ $WHICHTASK == *"_basis"* ]] && PYTHONEXE=run_fom_basis.py

    python ${PYTHONEXE} \
	   --exe ${EXENAME} \
	   --mesh-dir ${meshDir}\
	   --stepper-name ${STEPPERNAME}
    cd ${topDir}
fi



# #--------------------------------------
# # chem problem, LSPG ROM with FULL MESH
# #-------------------------------------
# if [[ $WHICHTASK == *"_chem_lspg_full_mesh_"* ]];
# then
#     # check if the build was already done
#     if [ ! -d ${CPPWORKINGDIR}/build ]; then
# 	echo "there is no build in the target folder, do that first"
# 	exit 0
#     fi

#     # check if the basis are present (basis are computed with eigen only)
#     BASISDIRNAME=
#     [[ $WHICHTASK = *"_bdf1_"* ]] && BASISDIRNAME=data_eigen_chem_fom_bdf1_basis
#     if [ ! -d ${CPPWORKINGDIR}/${BASISDIRNAME} ]; then
# 	echo "there is not basis dir in the target folder, do that first"
# 	exit 0
#     fi

#     # create folder for current task inside workindir
#     destDir=${CPPWORKINGDIR}/"data_"${WHICHTASK}
#     mkdir ${destDir}

#     # link the executable from build directory
#     EXENAME=
#     if [[ $WHICHTASK == *"eigen_chem_lspg_full_mesh_bdf1_"* ]]; then
# 	EXENAME=adr2d_eigen_chem_lspg_full_mesh_bdf1
#     fi
#     if [[ $WHICHTASK == *"kokkos_chem_lspg_full_mesh_bdf1_"* ]]; then
# 	EXENAME=adr2d_kokkos_chem_lspg_full_mesh_bdf1
#     fi
#     [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
#     ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

#     # copy the template input
#     cp ${topDir}/src/input.template ${destDir}

#     # link meshes directory
#     meshDir=${destDir}/full_meshes
#     if [ ! -d ${destDir}/full_meshes ]; then
#     	ln -s ${topDir}/full_meshes ${destDir}/full_meshes
#     fi

#     # copy python scripts
#     cp ${topDir}/run_scripts/myutils_chem.py ${destDir}/
#     cp ${topDir}/constants_chem.py ${destDir}/
#     cp ${topDir}/plot_scripts/plot_common.py ${destDir}/
#     cp ${topDir}/run_scripts/run_rom_full_mesh_timing.py ${destDir}

#     # enter and run
#     cd ${destDir}
#     python run_rom_full_mesh_timing.py \
# 	   --exe ${EXENAME} \
# 	   --mesh-dir ${meshDir}\
# 	   --basis-dir ${BASISDIRNAME}
#     cd ${topDir}
# fi
