platform="x86"
project_root="${PWD}/thirdparty/libsndfile"

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

cd ${project_root}
if [ ! -d build/$platform ]; then
	mkdir -p build/$platform
fi

make clean -j 8 

# env
set -x

if [ "$platform" == "gen4l" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen4l/toolchains/bin/"
	export CFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	export CPPFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	./configure --host=aarch64-linux-gnu --build=x86_64-pc-linux-gnu \
                --enable-static=yes \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen4c" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen4c/toolchains/bin/"
	export CFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	export CPPFLAGS="-pipe -march=armv8-a -mcpu=cortex-a53+crypto -mlittle-endian -funwind-tables"
	./configure --host=aarch64-linux-gnu --build=x86_64-pc-linux-gnu \
                --enable-static=yes \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen4s" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen4s/toolchains/bin/"
	export CFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -Wno-psabi"
	export CPPFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -Wno-psabi"
	./configure --host=arm-linux-gnueabihf --build=x86_64-pc-linux-gnu \
                --enable-static=yes \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "gen3l" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen3l/toolchains/bin/"
	export CFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	export CPPFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
    export STAGING_DIR="${HOME}/.ubnt/cache/gen3l/barebones/staging_dir"
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--enable-static=yes \
				--enable-shared=no --prefix="${project_root}/build/${platform}"

elif [ "$platform" == "gen3m" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen3m/toolchains/bin/"
	export CFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
	export CPPFLAGS='-mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables'
    export STAGING_DIR="${HOME}/.ubnt/cache/gen3m/barebones/staging_dir"
	./configure --host=arm-openwrt-linux --build=x86_64-pc-linux-gnu \
				--enable-static=yes \
				--enable-shared=no --prefix="${project_root}/build/${platform}"

elif [ "$platform" == "gen3s" ]; then
	export PATH="$PATH:${HOME}/.ubnt/cache/gen3s/toolchains/bin/"
	export CFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables -Wno-psabi"
	export CPPFLAGS="-mthumb -march=armv7-a -mtune=cortex-a7 -mlittle-endian -mfloat-abi=hard -mfpu=neon-vfpv4 -Wno-deprecated-declarations -Wa,-mimplicit-it=thumb -funwind-tables -Wno-psabi"
	./configure --host=arm-linux-gnueabihf --build=x86_64-pc-linux-gnu \
				--enable-static=yes \
				--enable-shared=no --prefix=${project_root}/build/${platform}/

elif [ "$platform" == "x86" ]; then
	export CC=/usr/bin/gcc
	export CFLAGS='-g -O0'
	./configure --enable-shared=no --enable-static=yes --prefix=${project_root}/build/${platform}/
fi

make -j 8 && make install

