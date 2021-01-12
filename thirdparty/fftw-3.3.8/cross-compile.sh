platform="x86"
fftw3=`pwd`

printHelp() {
	echo "Usage: cross compile for speexdsp"
	echo "$SCRIPT [-p <platform>]"
	echo ""
	echo "    -p    default: x86, support gen4l, gen3l, gen3m, gen3s"
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

if [ ! -z build/$platform ]; then
	mkdir -p build/$platform
fi

env

if [ "$platform" == "gen4l" ]; then
	export PATH="$PATH:/home/richard/.ubnt/cache/gen4l/toolchains/bin/"
	export CFLAG='-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables'
	./configure --host=aarch64-linux-gnu --build=x86_64-pc-linux-gnu \
				--enable-single --enable-neon \
				--prefix=${fftw3}/build/${platform}/

elif [ "$platform" == "gen3l" ]; then
	export PATH="$PATH:/home/richard/.ubnt/cache/gen3l/toolchains/bin/"
	export CFLAG='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--enable-single --enable-neon \
				--prefix=${fftw3}/build/${platform}/

elif [ "$platform" == "gen3m" ]; then
	export PATH="$PATH:/home/richard/.ubnt/cache/gen3m/toolchains/bin/"
	export CFLAG='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--enable-single --enable-neon \
				--prefix=${fftw3}/build/${platform}/

# elif [ "$platform" == "gen3s" ]; then
# 	export PATH="$PATH:/home/richard/.ubnt/cache/gen3s/toolchains/bin"
# 	export CFLAG='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
# 	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu --enable-single --enable-neon --prefix=${fftw3}/build/${platform}/

elif [ "$platform" == "x86" ]; then
	# export CC=/usr/bin/gcc
	export CFLAG='-g -O0'
	./configure --build=x86_64-pc-linux-gnu --enable-single --prefix=${fftw3}/build/${platform}/
fi

make clean -j 8 && compiledb make -j 8 && make install
