#!/bin/sh
# /.system/miyoomini/GBA/launch.sh

EMU_TAG=GBA
EMU_EXE=picogpsp

# TODO: swap on?

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"

"$EMU_DIR/$EMU_EXE" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"

# TODO: swap off?
