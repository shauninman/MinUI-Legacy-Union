ifeq (,$(CROSS_COMPILE))
$(error missing CROSS_COMPILE for this toolchain)
endif

CC = $(CROSS_COMPILE)gcc

FLAGS	= -Os -lasound -lmsettings -lrt -ldl -Wl,--gc-sections -s -std=gnu99

all:
	$(CC) keymon.c -o keymon $(FLAGS)
clean:
	rm -rf keymon