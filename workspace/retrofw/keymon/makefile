FLAGS	= -Os -lmsettings -lrt -ldl -Wl,--gc-sections -s

all:
	/opt/retrofw-toolchain/bin/mipsel-linux-gcc keymon.c -o keymon $(FLAGS)
clean:
	rm -rf keymon