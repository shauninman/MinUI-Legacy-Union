#!/bin/sh
# /.system/miyoomini/paks/MinUI.pak/launch.sh

# TODO: move platform init stuff to /sbin scripts?

# init_lcd
cat /proc/ls
sleep 0.5

# init backlight
echo 0 > /sys/class/pwm/pwmchip0/export
echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
echo 6 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

# init charger detection
if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
	echo 59 > /sys/class/gpio/export
	echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
fi

# paths
export SYSTEM_PATH=$SDCARD_PATH/.system/$SYSTEM_NAME
export USERDATA_PATH=$SDCARD_PATH/.userdata/$SYSTEM_NAME
export SHARED_SYSTEM_PATH=$SDCARD_PATH/.system/shared
export SHARED_USERDATA_PATH=$SDCARD_PATH/.userdata/shared
export LOGS_PATH=$USERDATA_PATH/logs
export PAKS_PATH=$SYSTEM_PATH/paks

rm -f "$SDCARD_PATH/update.log"

killall keymon

export LD_LIBRARY_PATH="$SYSTEM_PATH/lib:$LD_LIBRARY_PATH"
export PATH="$SYSTEM_PATH/bin:$PATH"

# EITHER mount shared data without a write cache (bad performance)
#mount --bind $SHARED_USERDATA_PATH $SHARED_USERDATA_PATH >> $LOGS_PATH/MinUI.txt 2>&1
#mount -o remount,sync $SHARED_USERDATA_PATH >> $LOGS_PATH/MinUI.txt 2>&1

# OR disable write caching for the whole sd card (even worse performance)
#mount -o remount,sync $SDCARD_PATH >> $LOGS_PATH/MinUI.txt 2>&1

lumon & # adjust lcd luma and saturation

CHARGING=`cat /sys/devices/gpiochip0/gpio/gpio59/value`
if [ "$CHARGING" == "1" ]; then
	batmon
fi

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

# init datetime
DATETIME_PATH=$SHARED_USERDATA_PATH/datetime.txt
if [ -f "$DATETIME_PATH" ]; then
	DATETIME=`cat "$DATETIME_PATH"`
	date +'%F %T' -s "$DATETIME"
	DATETIME=`date +'%s'`
	DATETIME=$((DATETIME + 6 * 60 * 60))
	date -s "@$DATETIME"
fi

cd $(dirname "$0")

touch /tmp/minui_exec && sync

while [ -f /tmp/minui_exec ]; do
	echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

	"./MinUI" &> "$LOGS_PATH/MinUI.txt"
	
	echo `date +'%F %T'` > "$DATETIME_PATH"
	echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
	sync

	NEXT="$USERDATA_PATH/next.sh"
	if [ -f $NEXT ]; then
		CMD=`cat $NEXT`
		rm -f $NEXT
		eval $CMD
		
		echo `date +'%F %T'` > "$DATETIME_PATH"
		sync
	fi
done

reboot # just in case