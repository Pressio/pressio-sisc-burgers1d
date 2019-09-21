#!/bin/bash

set -e

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
[[ $WIPEEXISTING == yes ]] && wipe_existing_data_in_target_dir

# make sure the task name passed in is valid
if [[ ${TASKNAME} != eigen &&
	  ${TASKNAME} != kokkos ]];
then
    echo "--do is set to non-admissible value (I am terminating), choose one of:"
    echo "   eigen or kokkos "
    exit 3
fi

#--------------------------------------
#
#     Chemisty adr2d LSPG SAMPLE mesh
#
#-------------------------------------
# check if the build was already done
if [ ! -d ${CPPWORKINGDIR}/build ]; then
    echo "there is no build in the target folder, do that first"
    exit 0
fi

# check if the basis are present (basis are computed with eigen only
# regardless of whether we use kokkos or eigen to run LSPG)
BASISDIRNAME="data_chem_fom_${MESHORDERNAME}_mesh_eigen_bdf1_basis"
if [ ! -d ${CPPWORKINGDIR}/${BASISDIRNAME} ]; then
    echo "there is not basis dir in the target folder, do that first"
    exit 0
fi

# create folder for current task inside workindir
destDir=${CPPWORKINGDIR}/"data_chem_lspg_"$MESHORDERNAME"_sample_mesh_"${TASKNAME}
[[ ! -d ${destDir} ]] && mkdir ${destDir}

# find executable name
EXENAME=
[[ $TASKNAME == *"eigen"* ]] && EXENAME=adr2d_eigen_chem_lspg_sample_mesh_bdf1
[[ $TASKNAME == *"kokkos"* ]] && EXENAME=adr2d_kokkos_chem_lspg_sample_mesh_bdf1
# link the executable from build directory
[[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

# copy the template input
cp ${TOPDIR}/src/input.template ${destDir}

# create links to full meshes
meshDir=${destDir}/meshes
if [ ! -d $meshDir ]; then
    ln -s ${TOPDIR}/meshing/meshes_${MESHORDERNAME} ${meshDir}
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
