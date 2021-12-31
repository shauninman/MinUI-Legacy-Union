#!/bin/sh
# /.system/rg350/paks/Tools/Power Off.pak/launch.sh

sync
echo 1 > /sys/class/leds/power/brightness
rm -f /tmp/minui_exec
poweroff
