#!/bin/sh
# /.system/miyoomini/paks/Emus/SFC/launch.sh

EMU_TAG=SFC
# EMU_EXE=snes9x
EMU_EXE=snes9x2002
EMU_EXE=snes9x2005_plus

ROM="$1"

EMU_DIR=$(dirname "$0")
mkdir -p "$SDCARD_PATH/Bios/$EMU_TAG"
mkdir -p "$SDCARD_PATH/Saves/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"

# echo 0 | gdbstatic -batch-silent -ex run -ex "set logging overwrite on" -ex "set logging file gdb.txt" -ex "set logging on" -ex "backtrace full" -ex "set logging off" -ex quit --args picoarch "$SYSTEM_PATH/lib/retroarch/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"
