#!/bin/bash

##############################################################
#
# This runs tests for the J * phi product for eigen or Kokkos
#
##############################################################

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

#-------------------------------------
# check if the build was already done
if [ ! -d ${CPPWORKINGDIR}/build ]; then
    echo "there is no build in the target folder, do that first"
    exit 0
fi

# create folder for current task inside workindir
destDir=${CPPWORKINGDIR}/"data_jphi_"$MESHORDERNAME"_full_mesh_"${TASKNAME}
[[ ! -d ${destDir} ]] && mkdir ${destDir}

# determine name of the executable
EXENAME=
[[ $TASKNAME == *"eigen"* ]] && EXENAME=adr2d_eigen_jphi_prod
[[ $TASKNAME == *"kokkos"* ]] && EXENAME=adr2d_kokkos_jphi_prod
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
cp ${TOPDIR}/help_scripts/myutils_common.py ${destDir}/
cp ${TOPDIR}/help_scripts/myutils_jphi.py ${destDir}/
cp ${TOPDIR}/help_scripts/constants_jphi.py ${destDir}/
cp ${TOPDIR}/run_scripts/run_jphi_timing.py ${destDir}

# enter and run
cd ${destDir}
python run_jphi_timing.py -exe ${EXENAME} -mesh-dir ${meshDir}
cd ${TOPDIR}
