#!/bin/sh
# /storage/.config/custom_start.sh

export SCREEN_WIDTH=854
export SCREEN_HEIGHT=480
export SDCARD_PATH=/storage/roms
export SYSTEM_NAME=rgb10max

LAUNCH_PATH="$SDCARD_PATH/.system/$SYSTEM_NAME/paks/MinUI.pak/launch.sh"

if [ -f "$LAUNCH_PATH" ]; then
	# TODO: will we need to do this for all shell scripts? maybe with a find in the rgb10max folder?
	chmod +x "$LAUNCH_PATH" && "$LAUNCH_PATH"
fi

# don't let emuelec touch our card
# poweroff