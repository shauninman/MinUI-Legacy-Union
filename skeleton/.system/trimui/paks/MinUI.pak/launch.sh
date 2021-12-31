#!/bin/sh
# /.system/trimui/paks/MinUI.pak/launch.sh

export SYSTEM_PATH=$SDCARD_PATH/.system/$SYSTEM_NAME
export USERDATA_PATH=$SDCARD_PATH/.userdata/$SYSTEM_NAME
export SHARED_SYSTEM_PATH=$SDCARD_PATH/.system/shared
export SHARED_USERDATA_PATH=$SDCARD_PATH/.userdata/shared
export LOGS_PATH=$USERDATA_PATH/logs
export PAKS_PATH=$SYSTEM_PATH/paks

# TODO: tmp?
/etc/init.d/adbd start &

# clean up from .tmp_update
notify 100 quit
killall -s KILL updateui
rm -f "$SDCARD_PATH/update.log"

killall keymon

export LD_LIBRARY_PATH="$SYSTEM_PATH/lib:$LD_LIBRARY_PATH"
export PATH="$SYSTEM_PATH/bin:$PATH"

a=`ps | grep keymon | grep -v grep`
if [ "$a" == "" ]; then
	keymon &
fi

a=`ps | grep mlanguage | grep -v grep`
if [ "$a" == "" ]; then
	mlanguage
fi

mkdir -p "$LOGS_PATH"
mkdir -p "$USERDATA_PATH/.mmenu"
mkdir -p "$SHARED_USERDATA_PATH/.minui"

cd $(dirname "$0")

touch /tmp/minui_exec && sync

while [ -f /tmp/minui_exec ]; do
	"./MinUI" &> "$LOGS_PATH/MinUI.txt"
	sync

	NEXT="$USERDATA_PATH/next.sh"
	if [ -f $NEXT ]; then
		CMD=`cat $NEXT`
		rm -f $NEXT
		eval $CMD
		sync
	fi
done