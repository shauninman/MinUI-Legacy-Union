#!/bin/sh
# /.system/miyoomini/paks/Tools/Power Off.pak/launch.sh

DATETIME_PATH=$SHARED_USERDATA_PATH/datetime.txt
echo `date +'%F %T'` > "$DATETIME_PATH"
rm -f /tmp/minui_exec

sync
reboot
