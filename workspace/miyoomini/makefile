# miyoomini/makefile
all: sys emu

sys:
	cd ./lumon && make
	cd ./batmon && make
	cd ./third-party/DinguxCommander && make -j

emu:
	cd ../shared/third-party/gambatte-dms && PLATFORM=miyoomini ./build_minui.sh

clean:
	cd ./batmon && make clean
	cd ../shared/third-party/gambatte-dms && ./clean.sh