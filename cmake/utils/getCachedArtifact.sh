#!/bin/sh

#bail out on the smallest error
set -e

#grab the parameters
ARTIFACTORY_FOLDER=$1
ARTIFACTORY_FILE_NAME=$2
ARTIFACTORY_MD5=$3
DEST_FOLDER=$4

#establish where the ARTIFACTORY downloads will be stored
if [ "${UBNT_ARTIFACTORY_DOWNLOADS}" = "" ]
then
	UBNT_ARTIFACTORY_DOWNLOADS=${HOME}/.ubnt/artifactory
	echo "UBNT_ARTIFACTORY_DOWNLOADS is not set, using \"${UBNT_ARTIFACTORY_DOWNLOADS}\""
fi
mkdir -p "${UBNT_ARTIFACTORY_DOWNLOADS}"

#compute the ARTIFACTORY URL
ARTIFACTORY_URL=${ARTIFACTORY_FOLDER}/${ARTIFACTORY_FILE_NAME}

# flag to determine if we should use download or not, default no
DOWNLOAD_REQUIRED=false

# There are 2 files relates to cached barebones.  The one is barebones itself,
# the other is the md5sum of barebones.  md5sum of barebones will be given by
# barebones config file (barebonesBuilder.in). The way we check/use these files
# (md5su/barebones) is:
#  case 1. md5sum matches with barebones -> use cached barebones, this is what
#          we expect
#  case 2. md5sum mismatches barebones -> download barebones again
#  case 3. md5sum or barebones missing -> download barebones again
#
# BEWARE: if you forgot to change the "HASH_gen*_barebones" environment
# variable, but did change the "PATH_gen*_barebones" variable, the build
# will succeed even though it requested config is incorrect.

# check case 3
if [ ! -f "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_MD5}" -o ! -f "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}" ]
then
	DOWNLOAD_REQUIRED=true

else
	# check case 2
	MARKER_INVALID_CACHE="${UBNT_ARTIFACTORY_DOWNLOADS}/.INVALID_CACHE"
	rm -f ${MARKER_INVALID_CACHE}
	(cd "${UBNT_ARTIFACTORY_DOWNLOADS}" && (md5sum -c "${ARTIFACTORY_MD5}" > /dev/null || (md5sum "${ARTIFACTORY_FILE_NAME}" && touch "${MARKER_INVALID_CACHE}")))
	if [ -f "${MARKER_INVALID_CACHE}" ]
	then
		DOWNLOAD_REQUIRED=true
		echo "MD5 checksum mismatch, download ${ARTIFACTORY_FILE_NAME} again"
		rm -f "${MARKER_INVALID_CACHE}"
	fi
fi

if [ $DOWNLOAD_REQUIRED = true ]
then
	#clean up any residuals because of set -e above
	[ -f "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_MD5}" ] && rm "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_MD5}"
	[ -f "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}"  ] && rm "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}"
	[ -d "${TEMP_FOLDER}" ] && rm -rf "${TEMP_FOLDER}"

	echo "Downloading: ${ARTIFACTORY_URL}"

	#create the temp folder where we are going to download the file from ARTIFACTORY
	TEMP_FOLDER=`mktemp -d /tmp/artifactory.XXXXXXXX`

	#create the MD5 file
	echo "${ARTIFACTORY_MD5}  ${ARTIFACTORY_FILE_NAME}" >"${TEMP_FOLDER}/${ARTIFACTORY_MD5}"

	#grab the file from ARTIFACTORY
	wget "${ARTIFACTORY_URL}" -P "${TEMP_FOLDER}"/

	#validate the MD5
	(cd "${TEMP_FOLDER}" && (md5sum -c "${ARTIFACTORY_MD5}" || (md5sum "${ARTIFACTORY_FILE_NAME}" && false)))

	#move both the archive and the MD5 file in the ARTIFACTORY cache
	mv "${TEMP_FOLDER}"/* "${UBNT_ARTIFACTORY_DOWNLOADS}"/

	#remove the temp folder
	rm -rf "${TEMP_FOLDER}"
else
	echo "Using cached: ${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}"
fi

#extract the toolchain if required
if [ ! -f "${DEST_FOLDER}/${ARTIFACTORY_MD5}" ]
then
	#make sure the destination folder is not created
	if [ -d "${DEST_FOLDER}" ]
	then
		echo "Removing stale \"${DEST_FOLDER}\"."
		rm -rf "${DEST_FOLDER}"
	fi

	echo "Extracting: ${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}"

	#create a temp destination folder
	TEMP_FOLDER=`mktemp -d /tmp/artifactory.XXXXXXXX`

	#untar everything in the temp folder
	tar xaf "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_FILE_NAME}" -C "${TEMP_FOLDER}"

	#create the destination folder
	mkdir -p "${DEST_FOLDER}"

	#transfer everything from temp folder to the new folder
	mv "${TEMP_FOLDER}"/*/* "${TEMP_FOLDER}"/*/.* "${DEST_FOLDER}"/ 2>/dev/null || true

	#copy the MD5 file in the dest folder
	cp "${UBNT_ARTIFACTORY_DOWNLOADS}/${ARTIFACTORY_MD5}" "${DEST_FOLDER}"/

	#remove the temp folder
	rm -rf "${TEMP_FOLDER}"
fi
