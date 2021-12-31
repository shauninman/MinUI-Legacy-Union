#!/bin/sh
# /.system/miyoomini/paks/MinUI.pak/launch.sh

# TODO: move platform init stuff to /sbin scripts?

# init_lcd
cat /proc/ls
sleep 0.25

# init backlight
echo 0 > /sys/class/pwm/pwmchip0/export
echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
echo 100 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
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

CHARGING=`cat /sys/devices/gpiochip0/gpio/gpio59/value`
if [ "$CHARGING" == "1" ]; then
	batmon
fi

a=`ps | grep keymon | grep -v grep`
if [ "$a" == "" ]; then
	keymon & # &> "$LOGS_PATH/keymon.txt" &
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

# overclock 
# echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
# cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
# cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

while [ -f /tmp/minui_exec ]; do
	echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

	"./MinUI" &> "$LOGS_PATH/MinUI.txt"
	sync
	
	echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

	NEXT="$USERDATA_PATH/next.sh"
	if [ -f $NEXT ]; then
		sysmon freemma

		CMD=`cat $NEXT`
		rm -f $NEXT
		eval $CMD
		sync
	fi
	
	sysmon freemma
done

reboot # just in case