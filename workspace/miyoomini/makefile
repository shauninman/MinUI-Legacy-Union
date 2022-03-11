# miyoomini/makefile
all: sys emu

sys:
	cd ./lumon && make
	cd ./batmon && make
	cd ./third-party/SDL-1.2 && ./make.sh
	cd ../shared/third-party/DinguxCommander && make -j

emu:
	cd ./third-party/picoarch && make platform=$(PLATFORM) -j4

clean:
	cd ./lumon && make clean
	cd ./batmon && make clean
	cd ./third-party/SDL-1.2 && make distclean
	cd ../shared/third-party/DinguxCommander && make clean
	cd ./third-party/picoarch && make platform=$(PLATFORM) clean
	