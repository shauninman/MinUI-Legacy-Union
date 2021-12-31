#!/bin/sh
# /.system/rg350/paks/Emus/GB/launch.sh

EMU_TAG=GB
EMU_EXE=gambatte

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
exec "$EMU_DIR/$EMU_EXE" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"
