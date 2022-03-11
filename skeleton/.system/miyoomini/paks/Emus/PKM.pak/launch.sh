#!/bin/sh
# /.system/miyoomini/paks/Emus/PKM/launch.sh

EMU_TAG=PKM
EMU_EXE=pokemini

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" DMG &> "$LOGS_PATH/$EMU_TAG.txt"
