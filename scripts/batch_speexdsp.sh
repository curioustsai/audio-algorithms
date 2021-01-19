#!/bin/bash

working_dir=`pwd`
target_folder="/home/richard/workspace/test_cases/ui_database/unifi_video"
output_folder="/home/richard/workspace/test_cases/ui_database/unifi_video_speexdsp"
compare_folder="/home/richard/workspace/test_cases/ui_database/unifi_video_orig_speex"

printHelp() {
	echo "Usage: batch simulation for speexdsp"
	echo "$SCRIPT -t <taret folder> -o <output folder>"
	echo ""
	echo "    -t    target folder"
	echo "    -o    output folder"
}


OPTIONS="t:o:h"
ARGS=`getopt $OPTIONS $*`
set -- $ARGS
for i
do
	case "$i" in
	-t)
		target_folder=$2; shift
		shift
		;;
	-o)
		output_folder=$2; shift
		shift
		;;
esac
done

speexdsp="$working_dir/build/x86/MinSizeRel/rootfs/bin/ui_speexdsp"

if [ ! -z $output_folder ]; then
	mkdir -p $output_folder
fi

if [ ! -z $compare_folder ]; then
	mkdir -p $compare_folder
fi


for file in $target_folder/*.wav
do
	echo "process "$file""

	bname="$(basename -- "$file")"

	# Options:
	# 	-h,--help                   Print this help message and exit
	# 	-i,--inFile TEXT:FILE REQUIRED specify an input file
	# 	-r,--refFile TEXT:FILE      specify an reference file
	# 	-o,--outFile TEXT REQUIRED  specify an output file
	# 	--enable-agc                enable agc
	# 	--nsLevel INT:NUMBER        noise suppression level
	# 	--agcTarget INT:NUMBER      agc target sample value
	# 	--tailLength INT:NUMBER     tail length in samples
	# 	--framesize INT:NUMBER      frame size in samples

	$speexdsp -i ${file} -o ${output_folder}/${bname} --nsLevel 15 --framesize 1024

	# echo "process $output_folder/"$bname""
	sox -M -c 1 "$target_folder/$bname" -c 1 "$output_folder/$bname" "$compare_folder/$bname"
done

