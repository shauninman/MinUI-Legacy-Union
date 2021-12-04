CC = $(CROSS_COMPILE)gcc

FLAGS	= -Os -lasound -lmsettings -lrt -ldl -Wl,--gc-sections -s

all:
	$(CC) keymon.c -o keymon $(FLAGS)
clean:
	rm -rf keymon