#!/bin/sh
# /.system/miyoomini/PKM/launch.sh

EMU_TAG=PKM
EMU_EXE=pokemini

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"

"$EMU_DIR/$EMU_EXE" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"