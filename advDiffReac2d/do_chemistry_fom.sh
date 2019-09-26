#!/bin/bash

#--------------------------------------
#
#     Chemisty adr2d FOM only
#
# either FOM timing or basis generation
#-------------------------------------

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
if [[ ${TASKNAME} != eigen_rk4_timing &&
	  ${TASKNAME} != eigen_rk4_basis &&
	  ${TASKNAME} != eigen_bdf1_timing &&
	  ${TASKNAME} != eigen_bdf1_basis &&
	  ${TASKNAME} != kokkos_rk4_timing &&
	  ${TASKNAME} != kokkos_rk4_basis ]];
then
    echo "--do is set to non-admissible value (I am terminating), choose one of:"
    echo "   eigen_{rk4,bdf1}_{timing,basis}, kokkos_rk4_{timing,basis}"
    exit 3
fi


# check if the build was already done
if [ ! -d ${CPPWORKINGDIR}/build ]; then
    echo "there is no build in the target folder, do that first"
    exit 0
fi

# create folder inside workindir
destDir=${CPPWORKINGDIR}/"data_chem_fom_"$MESHORDERNAME"_mesh_"${TASKNAME}
[[ ! -d ${destDir} ]] && mkdir ${destDir}

# set the name of the target executable
EXENAME=
[[ $TASKNAME == *"eigen_"* ]] && EXENAME=adr2d_eigen_fom
[[ $TASKNAME == *"kokkos_"* ]] && EXENAME=adr2d_kokkos_fom
# link the executable from build directory
[[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}
echo "EXENAME = ${EXENAME}"

# copy the template input
cp ${TOPDIR}/src/input.template ${destDir}

# create links to full meshes
meshDir=${destDir}/meshes
if [ ! -d $meshDir ]; then
    ln -s ${TOPDIR}/meshing/meshes_${MESHORDERNAME} ${meshDir}
fi

# copy python scripts
cp ${TOPDIR}/help_scripts/myutils_common.py ${destDir}
cp ${TOPDIR}/help_scripts/constants_chem.py ${destDir}/
cp ${TOPDIR}/help_scripts/myutils_chem.py ${destDir}/

if [[ $TASKNAME == *"_timing"* ]]; then
    cp ${TOPDIR}/run_scripts/run_fom_timing.py ${destDir}/
elif [[ $TASKNAME == *"_basis"* ]];
then
    # the plotting scripts are only needed when we compute basis
    # because when we do timing, we do not dump snapshots
    cp ${TOPDIR}/plot_scripts/plot_common.py ${destDir}/
    cp ${TOPDIR}/plot_scripts/plot_chem.py ${destDir}/
    cp ${TOPDIR}/run_scripts/run_fom_basis.py ${destDir}/
else
    echo "error: unrecognized task for TASKNAME=${TASKNAME}. Terminating."
    exit 11
fi

# enter and run
cd ${destDir}

# choose stepper name
STEPPERNAME=
[[ $TASKNAME = *"_rk4"* ]] && STEPPERNAME="RungeKutta4"
[[ $TASKNAME = *"_bdf1"* ]] && STEPPERNAME="bdf1"

# set python driver which depends on the task
PYTHONEXE=
[[ $TASKNAME == *"_timing"* ]] && PYTHONEXE=run_fom_timing.py
[[ $TASKNAME == *"_basis"* ]] && PYTHONEXE=run_fom_basis.py

python ${PYTHONEXE} \
       --exe ${EXENAME} \
       --mesh-dir ${meshDir}\
       --stepper-name ${STEPPERNAME}
cd ${TOPDIR}
