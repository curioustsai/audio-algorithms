#!/bin/sh

#bail out on the first error
set -e

#if there is no UBNT_CMAKE_DIR, or is wrongfully positioned, than bail out.
#for now, we just look for the CMakeLists.txt and that one must have a project
#called `all` inside it
if [ ! -f ${UBNT_CMAKE_DIR}/CMakeLists.txt ]
then
	echo "UBNT_CMAKE_DIR is mandatory but is not defined or it has the wrong value"
	exit 1
fi

if ! cat ${UBNT_CMAKE_DIR}/CMakeLists.txt|grep "project(all)" >/dev/null
then
	echo "Detected CMakeLists.txt file does not contain the proper project name"
	exit 1
fi

#local variables used to compute various defaults when they are missing
if [ -z ${UBNT_BASE_FOLDER} ]
then
	export UBNT_BASE_FOLDER=${HOME}/.ubnt
	echo "UBNT_BASE_FOLDER is not set, using \"${UBNT_BASE_FOLDER}\""
else
	echo "UBNT_BASE_FOLDER is set to \"${UBNT_BASE_FOLDER}\""
fi

#establish where we are gonna dld stuff
if [ -z ${UBNT_ARTIFACTORY_DOWNLOADS} ]
then
	export UBNT_ARTIFACTORY_DOWNLOADS=${UBNT_BASE_FOLDER}/artifactory
	echo "UBNT_ARTIFACTORY_DOWNLOADS is not set, using \"${UBNT_ARTIFACTORY_DOWNLOADS}\""
fi

#establish UBNT_FW_CACHE
if [ -z ${UBNT_FW_CACHE} ]
then
	export UBNT_FW_CACHE=${UBNT_BASE_FOLDER}/cache
	echo "UBNT_FW_CACHE is not set, using \"${UBNT_FW_CACHE}\""
fi

#where is cmake conf dir
if [ -z "$UBNT_CMAKE_CONF_DIR" ]
then
	export UBNT_CMAKE_CONF_DIR=$(readlink -m "${UBNT_CMAKE_DIR}"/conf)
	echo "UBNT_CMAKE_CONF_DIR: ${UBNT_CMAKE_CONF_DIR}"
fi
