#!/bin/sh
# /.system/rg350/paks/MinUI.pak/launch.sh

export SYSTEM_PATH=$SDCARD_PATH/.system/$SYSTEM_NAME
export USERDATA_PATH=$SDCARD_PATH/.userdata/$SYSTEM_NAME
export SHARED_SYSTEM_PATH=$SDCARD_PATH/.system/shared
export SHARED_USERDATA_PATH=$SDCARD_PATH/.userdata/shared
export LOGS_PATH=$USERDATA_PATH/logs
export PAKS_PATH=$SYSTEM_PATH/paks

export LD_LIBRARY_PATH="$SYSTEM_PATH/lib:$LD_LIBRARY_PATH"
export PATH="$SYSTEM_PATH/bin:$PATH"

# turn off LEDS (the screen is already on)
echo 0 > /sys/class/leds/power/brightness

# headphones source always reports "on" on stock dingux so we use inverse of speaker
SPEAKER=`amixer -c 1 cget numid=6,iface=MIXER | awk -F"=" 'NR == 3 {print $2;}'`
if [ "$SPEAKER" == "off" ]; then
	export HEADPHONES="on"
else
	export HEADPHONES="off"
fi

killall -s KILL pwswd

a=`ps | grep keymon | grep -v grep`
if [ "$a" == "" ]; then
	nice -10 keymon &
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
