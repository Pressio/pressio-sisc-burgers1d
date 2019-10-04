#!/bin/bash

set -e

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
[[ $WIPEEXISTING == yes  ]] && wipe_existing_data_in_target_dir ${PYWORKINGDIR}

# #---------------------------
# # only build
# #---------------------------
# if [ $WHICHTASK = "build" ]; then
#     source ${TOPDIR}/python/build_scripts/build.sh
# fi

#---------------------------
# time
#---------------------------
if [[ $WHICHTASK == *"gemm"* ]];
then
    # create folder inside workindir
    destDir=${PYWORKINGDIR}/"data_"${WHICHTASK}
    [[ ! -d ${destDir} ]] && mkdir ${destDir}

    # # copy all pything scripts there
    cp ${TOPDIR}/common/constants.py ${destDir}/
    cp ${TOPDIR}/python/run_timing.py ${destDir}/
    cp ${TOPDIR}/python/${WHICHTASK}.py ${destDir}/

    # enter there and run
    cd ${destDir}
    python run_timing.py --exe="${WHICHTASK}"
    cd ${TOPDIR}
fi
