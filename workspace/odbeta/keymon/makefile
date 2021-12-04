FLAGS	= -Os -lasound -lmsettings -lrt -ldl -Wl,--gc-sections -s -std=gnu99

all:
	/opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc keymon.c -o keymon $(FLAGS)
clean:
	rm -rf keymon