#!/bin/sh
# /.system/miyoomini/paks/Emus/GB/launch.sh

EMU_TAG=GB
EMU_EXE=gambatte

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
# GFX_BLOCKING=1
picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" DMG &> "$LOGS_PATH/$EMU_TAG.txt"
