#!/bin/sh
# /.system/miyoomini/paks/Emus/FC/launch.sh

EMU_TAG=FC
EMU_EXE=fceumm

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"
