#!/bin/bash

set -e

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
[[ $WIPEEXISTING == yes  ]] && wipe_existing_data_in_target_dir ${CPPWORKINGDIR}

#---------------------------
# build c++ exes
#---------------------------
if [ $WHICHTASK = "build" ]; then
    source ${TOPDIR}/cpp/build_scripts/build.sh
fi

# timing
if [[ $WHICHTASK == *"gemm"* ]];
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
    EXENAME=${WHICHTASK}
    [[ -f ${destDir}/${EXENAME} ]] && rm ${destDir}/${EXENAME}
    ln -s ${CPPWORKINGDIR}/build/${EXENAME} ${destDir}

    # copy the template input
    cp ${TOPDIR}/cpp/src/input.template ${destDir}

    # copy python scripts there
    cp ${TOPDIR}/cpp/run_scripts/myutils.py ${destDir}/
    cp ${TOPDIR}/common/constants.py ${destDir}/
    cp ${TOPDIR}/cpp/run_scripts/run_timing.py ${destDir}/

    # enter and run
    cd ${destDir}
    python run_timing.py --exe ${EXENAME}
    cd ${TOPDIR}
fi
