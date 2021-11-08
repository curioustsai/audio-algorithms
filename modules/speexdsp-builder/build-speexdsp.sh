#!/bin/bash
set -e

#------------- get parameters section ------------------------------------------

printHelp() {
	echo "Usage: "
	echo "$SCRIPT -p <platform> [-h <platform host>] [-t <toolchain directory>] [-b <buildType>] [-l <libsType>] [-j <njobs>] [-c] [-h]"
	echo ""
	echo "    -p    Platform which can be: ${UBNT_PLATFORMS_LIST[*]}. There is no default value. This is mandatory."
	echo "    -h    Host name of compiler command in toolchain."
	echo "    -b    The build type which can be Debug, Release or MinSizeRel. Default is MinSizeRel."
	echo "    -l    How the libraries will be built. Either STATIC or SHARED. Default is STATIC."
	echo "    -j    How many compilation units will be done simultaneously. Default is 8."
	echo "    -t    The directory of toolchain"
	echo "    -r    The position of project root"
	echo "    -c    Do clean before build"
	echo "    -v    Enable verbose build"
	echo "    -f    The compile flags used for specific UBNT platform"
	echo "    -g    Audio log level: 'debug', 'info', 'warn', 'off'. Default is 'warn'"
	echo "    -h    Prints this help and exits."
	echo ""
}

UBNT_BUILD_TYPE=MinSizeRel
UBNT_ROOTFS_DIR=$UBNT_BUILD_TYPE/rootfs
UBNT_WORK_DIR=$UBNT_BUILD_TYPE/work
UBNT_PLATFORM_TYPE=__empty__
UBNT_PLATFORMS_LIST=(gen2 gen3l gen3m gen3s gen4l gen4c gen4s x86)
UBNT_LIBRARIES_TYPE="STATIC"
UBNT_JOB_COUNT=4
UBNT_DO_CLEAN=false
UBNT_VERBOSE=false

SCRIPT=$0
OPTIONS="p:h:w:t:b:l:j:r:f:g:cvh"
ARGS=`getopt $OPTIONS $*`
set -- $ARGS
for i
do
	case "$i"
	in
		-p|--platform)	# Set the compiler host name
			UBNT_PLATFORM_TYPE=$2; shift 2
			;;
		-h|--host)
			HOST=$2; shift 2
			;;
		-t|--toolchain)	# Set the tool-chain location
			UBNT_TOOLCHAIN_ROOT=$2; shift 2
			;;
		-b|--buildtype)
			UBNT_BUILD_TYPE=$2; shift 2
			;;
		-l|--librarytype)
			UBNT_LIBRARIES_TYPE=$2; shift 2
			;;
		-j|--jobcount)
			UBNT_JOB_COUNT=$2; shift 2
			;;
		-r|--rootdir)	# Set the project root position
			ROOT=$2; shift 2
			;;
		-f)	# Set the UBNT_COMPILER_OPTION into build
			ARCHFLAGS=$(echo $2 | sed "s/@/ /g"); shift 2
			;;
		-c)	# Clean all projects before make
			UBNT_DO_CLEAN=true
			;;
        -v|--verbose)
            UBNT_VERBOSE=true
            ;;
	esac
done

#validation of the user input
if [[ ! "${UBNT_PLATFORMS_LIST[*]}" =~ "${UBNT_PLATFORM_TYPE}" ]]
then
	printHelp
	exit 1
fi
if [ "${UBNT_BUILD_TYPE}" != "Debug" ] && [ "${UBNT_BUILD_TYPE}" != "Release" ] && [ "${UBNT_BUILD_TYPE}" != "MinSizeRel" ]
then
	printHelp
	exit 2
fi
if [ "${UBNT_LIBRARIES_TYPE}" != "STATIC" ] && [ "${UBNT_LIBRARIES_TYPE}" != "SHARED" ]
then
	printHelp
	exit 3
fi

if [[ $UBNT_PLATFORM_TYPE != "x86" ]]; then
	AR="$UBNT_TOOLCHAIN_ROOT/$HOST-ar"
	CC="$UBNT_TOOLCHAIN_ROOT/$HOST-gcc"
	CXX="$UBNT_TOOLCHAIN_ROOT/$HOST-g++"
	RANLIB="$UBNT_TOOLCHAIN_ROOT/$HOST-ranlib"
else
	AR="ar"
	CC="gcc"
	CXX="g++"
	RANLIB="ranlib"
fi
WORKDIR=$ROOT/build/${UBNT_PLATFORM_TYPE}

# Thirdparty repo dir
SpeexdspDir=$ROOT/thirdparty/speexdsp

# Clean all projects first when user set -c.
# Clean projects may be required when rebuild at different platform
if [ $UBNT_DO_CLEAN = true ]; then
	make clean -C $SpeexdspDir
fi

if [ $UBNT_VERBOSE = true ]; then
    echo "host: $HOST"
    echo "ar: $AR"
    echo "cc: $CC"
    echo "cxx: $CXX"
    echo "flags: $ARCHFLAGS"
    echo "workdir: $WORKDIR"
    echo "ubnt_fw_cache: $TOOLCHAIN_ROOT"
fi

mkdir -p $WORKDIR/$UBNT_WORK_DIR/lib
mkdir -p $WORKDIR/$UBNT_ROOTFS_DIR/bin
mkdir -p $WORKDIR/$UBNT_ROOTFS_DIR/lib
mkdir -p $WORKDIR/$UBNT_ROOTFS_DIR/include/speex

# git apply patch
cd $SpeexdspDir
if [ ! -d $SpeexdspDir/patches ]; then
    mkdir $SpeexdspDir/patches
    git apply ${ROOT}/modules/speexdsp-builder/patches/100-fastNS_fxAGC.patch
    cp ${ROOT}/modules/speexdsp-builder/patches/100-fastNS_fxAGC.patch $SpeexdspDir/patches
fi

# check if we need reconfigure
reconfigure="yes"
if [[ -f $SpeexdspDir/configured_platform ]]; then
    SpeexdspConfig=`cat $SpeexdspDir/configured_platform`
    if [[ $SpeexdspConfig == $UBNT_PLATFORM_TYPE ]]; then
        reconfigure="no"
    fi
fi

# configure; make; make install
if [[ $reconfigure == "yes" ]]; then
    if [[ $UBNT_PLATFORM_TYPE == "x86" ]]; then
        CFLAGS="-g -O3" ./configure --prefix=${WORKDIR}/$UBNT_WORK_DIR \
             --enable-static --enable-shared
    else
        CXX=$CXX CXXFLAGS="$ARCHFLAGS -O3 " ./configure --prefix=${WORKDIR}/$UBNT_WORK_DIR \
            --enable-shared --enable-static -host=$HOST --build=x86-linux-gnu
    fi
    echo $UBNT_PLATFORM_TYPE > configured_platform
fi
make -j $UBNT_JOB_COUNT
make install
cp ${WORKDIR}/${UBNT_WORK_DIR}/lib/libspeexdsp.a ${WORKDIR}/${UBNT_ROOTFS_DIR}/lib
cp ${WORKDIR}/${UBNT_WORK_DIR}/include/speex/* ${WORKDIR}/${UBNT_ROOTFS_DIR}/include/speex
