#!/bin/sh
# /.system/miyoomini/paks/Emus/PS/launch.sh

EMU_TAG=PS
EMU_EXE=pcsx_rearmed

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"

# echo 0 | gdbstatic -batch-silent -ex run -ex "set logging overwrite on" -ex "set logging file gdb.txt" -ex "set logging on" -ex "backtrace full" -ex "set logging off" -ex quit --args picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"

