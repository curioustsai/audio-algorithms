PWD=`pwd`

for p in gen3s gen3m gen3l gen4l;
do
	cp $PWD/build/$p/MinSizeRel/rootfs/include/audio_events/* $mw/sources/ubnt_smart_audio/include/ubnt_smart_audio/
	[ ! -z $mw/sources/ubnt_smart_audio/lib/$p/ ] &&  mkdir -p $mw/sources/ubnt_smart_audio/lib/$p/
	cp $PWD/build/$p/MinSizeRel/rootfs/lib/libaudio_events.a $mw/sources/ubnt_smart_audio/lib/$p/libaudio_events.a
done
