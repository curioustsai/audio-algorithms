#!/bin/sh

#bail out on the first error
set -e

#load the defaults
. ${UBNT_CMAKE_DIR}/cmake/utils/defaults.sh

#load the settings for the downloads
. ${UBNT_CMAKE_DIR}/cmake/conf/barebonesBuilder.in

ARTIFACTORY_SERVER_DEFAULT=https://pdx-artifacts.rad.ubnt.com
ARTIFACTORY_SERVER=${ARTIFACTORY_SERVER_CUSTOM:-$ARTIFACTORY_SERVER_DEFAULT}
#download everything

gen=$UBNT_PLATFORM_TYPE
if [ "${gen}" != "x86" ] ; then
	for what in barebones toolchains; do
		${UBNT_CMAKE_DIR}/cmake/utils/getCachedArtifact.sh \
			$ARTIFACTORY_SERVER/list/unifi-video-firmware-dev/fw/${what}/uvc/$(eval echo \$PATH_${gen}_${what}) \
			$(eval echo \$NAME_${gen}_${what}) \
			$(eval echo \$HASH_${gen}_${what}) \
			${UBNT_FW_CACHE}/${gen}/${what}
	done
fi
