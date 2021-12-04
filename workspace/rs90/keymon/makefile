FLAGS	= -Os -lasound -lmsettings -lrt -ldl -Wl,--gc-sections -s

all:
	/opt/rs90-toolchain/usr/bin/mipsel-linux-gcc keymon.c -o keymon $(FLAGS)
clean:
	rm -rf keymon