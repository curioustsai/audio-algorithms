platform="x86"
project_root="${PWD}/thirdparty/soundtouch"
platform_list=(gen4l gen4c gen4s gen3l gen3m gen3s x86)

printHelp() {
	echo "Usage: cross compile for sproject"
	echo "$SCRIPT [-p <platform>]"
	echo ""
	echo "    -p    default: x86, support gen4l, gen4c, gen4s, gen3l, gen3m, gen3s"
}

OPTIONS="p:m:h"
ARGS=`getopt $OPTIONS $*`
set -- $ARGS
for i
do
	case "$i" in
	-p)
		platform=$2; shift
		shift
		;;
	-h)
		printHelp
		exit 0
		;;
esac
done

if [[ ! "${platform_list[*]}" =~ "${platform}" ]]; then
	printHelp
	exit 1
fi

cd ${project_root}
if [ ! -d build/$platform ]; then
	mkdir -p build/$platform
fi

make clean -j 8 

# env
set -x

if [ "$platform" == "gen4l" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen4l/toolchains/bin/"
	export CPPFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	./configure --host=aarch64-linux-gnu --build=x86_64-pc-linux-gnu \
                --enable-integer-samples \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen4c" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen4c/toolchains/bin/"
	export CPPFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	./configure --host=aarch64-linux-gnu --build=x86_64-pc-linux-gnu \
                --enable-integer-samples \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen4s" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen4s/toolchains/bin/"
	export CPPFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -Wno-psabi"
	./configure --host=arm-linux-gnueabihf --build=x86_64-pc-linux-gnu \
                --enable-integer-samples \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen3l" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen3l/toolchains/bin/"
	export CPPFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--with-fft=gpl-fftw3 --enable-fixed-point \
				--enable-shared=no --prefix="${project_root}/build/${platform}"

elif [ "$platform" == "gen3m" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen3m/toolchains/bin/"
	export CPPFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--with-fft=gpl-fftw3 --enable-fixed-point \
				--enable-shared=no --prefix="${project_root}/build/${platform}"

elif [ "$platform" == "gen3s" ]; then
	export PATH="$PATH:${HOME}/.ubnt_audio/cache/gen3s/toolchains/bin/"
	export CPPFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables -Wno-psabi"
	./configure --host=arm-linux-gnueabihf --build=x86_64-pc-linux-gnu \
                --enable-integer-samples \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "x86" ]; then
	export CC=/usr/bin/gcc
	export CFLAGS='-g -O0'
	./configure --enable-shared=no --enable-integer-samples --prefix=${project_root}/build/${platform}/
fi

make -j 8 && make install
