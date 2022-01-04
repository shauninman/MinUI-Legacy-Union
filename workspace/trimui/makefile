# trimui/makefile
sys:
	cd ./third-party/SDL-1.2 && ./make.sh
	cd ../shared/third-party/DinguxCommander && make -j
	cd ./third-party/gambatte-dms && make

clean:
	cd ./third-party/SDL-1.2 && make distclean
	cd ../shared/third-party/DinguxCommander && make -j
	cd ./third-party/gambatte-dms && make clean