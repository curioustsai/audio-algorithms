#!/bin/sh

##### Input Argument ###############################
## $1 : compile_commands.json path in FW Repo     ##
## $2 : compile_commands.json path in MW Repo     ##
####################################################

BBB_COMPILE_CMD_JSON=$(ls ${BBB}/build_dir/*/ubnt-middleware/compile_commands.json)
BBB_COMPILE_CMD_JSON=${1:-${BBB_COMPILE_CMD_JSON}}
UBNT_WORK_DIR=${UBNT_WORK_DIR:-.}
BBB_FLAGS_SORT=${UBNT_WORK_DIR}/bbb_flags
LOCAL_COMPILE_CMD_JSON=${UBNT_WORK_DIR}/compile_commands.json
LOCAL_COMPILE_CMD_JSON=${2:-${LOCAL_COMPILE_CMD_JSON}}
LOCAL_FLAGS_SORT=${UBNT_WORK_DIR}/flags
FLAGS_DIFF=${UBNT_WORK_DIR}/flags_diff

fetchCompileOpt()
{
	local input=$1
	local output=$2
	sed -n '0,/command/ s/command/&/p' ${input} | \
		tr   ' ' '\n'       | \
		sed  '/^$/d'        | \
		sed  '/\//d'        | \
		sed  '/^-o/d'       | \
		sed  '/^-c/d'       | \
		sed  '/"command"/d' | \
		sort > ${output};
}

if [ ! -z "${BBB_COMPILE_CMD_JSON}" -a -f "${BBB_COMPILE_CMD_JSON}" ]
then
	echo "  Fetch compile options from BBB : ${BBB_COMPILE_CMD_JSON}"
	fetchCompileOpt "${BBB_COMPILE_CMD_JSON}" "${BBB_FLAGS_SORT}"
else
	echo "  No BBB compile_commands.json"
	exit 1
fi

if [ -f "${LOCAL_COMPILE_CMD_JSON}" ]
then
	echo "  Fetch compile options from Local : ${LOCAL_COMPILE_CMD_JSON}"
	fetchCompileOpt "${LOCAL_COMPILE_CMD_JSON}" "${LOCAL_FLAGS_SORT}"
else
	echo "  No compile_commands.json"
	exit 1
fi

diff ${BBB_FLAGS_SORT} ${LOCAL_FLAGS_SORT} > ${FLAGS_DIFF} || true

if [ -s ${FLAGS_DIFF} ]
then
	echo -e "\nCompile options difference between BBB and MW Repo"
	echo ++++++++++++++++++++++++++++++++++++++++++++++++++
	cat ${FLAGS_DIFF}
	echo ++++++++++++++++++++++++++++++++++++++++++++++++++
else
	echo -e "[OK] NO inconsistent compile option\n"
fi
