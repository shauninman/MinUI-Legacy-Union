#!/bin/sh
# /.system/rgb10max/paks/MinUI.pak/launch.sh

# paths
export SYSTEM_PATH=$SDCARD_PATH/.system/$SYSTEM_NAME
export USERDATA_PATH=$SDCARD_PATH/.userdata/$SYSTEM_NAME
export SHARED_SYSTEM_PATH=$SDCARD_PATH/.system/shared
export SHARED_USERDATA_PATH=$SDCARD_PATH/.userdata/shared
export LOGS_PATH=$USERDATA_PATH/logs
export PAKS_PATH=$SYSTEM_PATH/paks

export LD_LIBRARY_PATH="$SYSTEM_PATH/lib:$LD_LIBRARY_PATH"
export PATH="$SYSTEM_PATH/bin:$PATH"

mkdir -p "$LOGS_PATH"
mkdir -p "$USERDATA_PATH/.mmenu"
mkdir -p "$SHARED_USERDATA_PATH/.minui"

cd $(dirname "$0")

# touch /tmp/minui_exec && sync
# NEXT_PATH="$USERDATA_PATH/next.sh"
# while [ -f /tmp/minui_exec ]; do
# 	"./MinUI" &> "$LOGS_PATH/MinUI.txt"
# 	sync
#
# 	if [ -f $NEXT_PATH ]; then
# 		CMD=`cat $NEXT_PATH`
# 		rm -f $NEXT_PATH
# 		eval $CMD
# 		sync
# 	fi
# done

poweroff