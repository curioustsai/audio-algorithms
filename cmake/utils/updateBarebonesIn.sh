#!/bin/bash
# This script is just a helper to easily update the BBB conf

UBNT_CMAKE_DIR=$(readlink -m "$(dirname "$0")/..")
. "$UBNT_CMAKE_DIR/utils/defaults.sh"
BBB_CONF_PATH=$UBNT_CMAKE_CONF_DIR
JOB_NAME=unifi-protect/uvc/uvc-firmware/develop

usage()
{
	[[ -n $1 ]] && echo "error: $1" >&2
	echo -e ""
	echo -e "Usage: $0 [OPTIONS]"
	echo -e "OPTIONS"
	echo -e "\t -h: prints this help"
	echo -e "\t -j <jenkins Job number>: required"
	echo -e "\t -m <jenkins job naMe>: default $JOB_NAME"
	echo -e "\t -p <path to barebonesBuilder.in>: default $BBB_CONF_PATH_DEFAULT"
	echo ""
	[[ -n $1 ]] && exit 1
}


while getopts "shj:m:p:" o
do
	case "$o" in
	j)
		JOB_NUMBER=$OPTARG
		;;
	m)
		JOB_NAME=$OPTARG
		;;
	p)
		BBB_CONF_PATH=$OPTARG
		;;
	h)
		usage ""
		exit 0
		;;
	*)
		usage "unrecognized option $o"
		;;
	esac
done

[[ -z $JOB_NUMBER ]] && usage "missing required arg: -j <job number>"

BBB_CONF_FILE=$BBB_CONF_PATH/barebonesBuilder.in
[[ ! -e $BBB_CONF_FILE ]] && usage "$(ls "$BBB_CONF_FILE" 2>&1)"

sed -i -r "s@(^job_number=).*@\1$JOB_NUMBER@" "$BBB_CONF_FILE"
sed -i -r "s@(^job_name=).*@\1$JOB_NAME@" "$BBB_CONF_FILE"

# Replace "/" with "/job/" in $JOB_NAME to fulfill Jenkins URL grammar
FULL_JOB_NAME=${JOB_NAME//\//\/job/}
JENKINS_URL=https://pdx-build-aerith.rad.ubnt.com/job/$FULL_JOB_NAME/$JOB_NUMBER/artifact

for plat in 2 3l 3m 3s 4l 4c 4s; do
	set -e
	MD5FILE=openwrt-gen${plat}-rel.tar.gz.md5
	TMP_FILE=$(mktemp --tmpdir barebonesBuilder-gen${plat}.XXXXX)
	wget -nv "$JENKINS_URL/openwrt-gen${plat}-rel/artifacts/$MD5FILE" -O "$TMP_FILE"
	MD5SUM=$(awk '{print $1}' "$TMP_FILE")
	set +e
	sed -i -r "s/(^HASH_gen${plat}_barebones=).*/\1$MD5SUM/" "$BBB_CONF_FILE"
	ret="$?"
	rm "$TMP_FILE"
	[[ $ret -ne 0 ]] && break
done
grep -e "^job_name" -e "^job_number" -e "^HASH_gen.*_barebones=" "$BBB_CONF_FILE"
