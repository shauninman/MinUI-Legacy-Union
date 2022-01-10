# rg350/makefile
all: sys emu

sys:
	cd ../shared/third-party/DinguxCommander && make -j

emu:
	cd ../shared/third-party/gambatte-dms && PLATFORM=gcw0 ./build_minui.sh

clean:
	cd ../shared/third-party/DinguxCommander && make -j
	cd ../shared/third-party/gambatte-dms && ./clean.sh