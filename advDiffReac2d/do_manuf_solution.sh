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
if [[ ${TASKNAME} != eigen_rk4 &&
	  ${TASKNAME} != eigen_bdf1 &&
	  ${TASKNAME} != kokkos_rk4 ]];
then
    echo "--do is set to non-admissible value (I am terminating), choose one of:"
    echo "   eigen_rk4, eigen_bdf1, kokkos_rk4"
    exit 3
fi


#------------------------------------------------------------
#
#     Runs the manufactured solution problem
#
# run the FOM manufactured solution problem with eigen or kokkos
# this test is used to check that things are correct
#------------------------------------------------------------
# check if the build was already done
if [ ! -d ${CPPWORKINGDIR}/build ]; then
    echo "there is no build in the target folder, do that first"
    exit 0
fi

# create folder inside workindir
destDir=${CPPWORKINGDIR}/"data_manuf_solution_"$MESHORDERNAME"_mesh_"${TASKNAME}
[[ ! -d ${destDir} ]] && mkdir ${destDir}

# set name of target executable to use
EXENAME=
[[ $TASKNAME = *"eigen_"* ]] && EXENAME=adr2d_eigen_fom
[[ $TASKNAME = *"kokkos_"* ]] && EXENAME=adr2d_kokkos_fom
# create link to the executable in the build directory
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
[[ $TASKNAME = *"rk4"* ]] && STEPPERNAME="RungeKutta4"
[[ $TASKNAME = *"bdf1"* ]] && STEPPERNAME="bdf1"

python run_fom_ms.py \
       --exe ${EXENAME} \
       --mesh-dir ${meshDir}\
       --stepper-name ${STEPPERNAME}
cd ${TOPDIR}
