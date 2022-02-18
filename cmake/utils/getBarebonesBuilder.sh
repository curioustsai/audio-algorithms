#!/bin/sh

#bail out on the first error
set -e

#load the defaults
. ${UBNT_CMAKE_DIR}/utils/defaults.sh

#load the settings for the downloads
. ${UBNT_CMAKE_DIR}/conf/barebonesBuilder.in

ARTIFACTORY_SERVER_DEFAULT=https://pdx-artifacts.rad.ubnt.com
ARTIFACTORY_SERVER=${ARTIFACTORY_SERVER_CUSTOM:-$ARTIFACTORY_SERVER_DEFAULT}

if [ x"$UBNT_PLATFORM_TYPE" = x"__empty__" -o x"$UBNT_PLATFORM_TYPE" = x"" ]; then
    #download everything
    generations=(gen2 gen3l gen3m gen3s gen4l gen4c gen4s gen4v)
else
    #download only needed for the currently built platform
    generations=($UBNT_PLATFORM_TYPE)
fi

for gen in "${generations[@]}"; do
	for what in barebones toolchains; do
		${UBNT_CMAKE_DIR}/utils/getCachedArtifact.sh \
			$ARTIFACTORY_SERVER/unifi-video-firmware-dev/fw/${what}/uvc/$(eval echo \$PATH_${gen}_${what}) \
			$(eval echo \$NAME_${gen}_${what}) \
			$(eval echo \$HASH_${gen}_${what}) \
			${UBNT_FW_CACHE}/${gen}/${what}
	done
done
