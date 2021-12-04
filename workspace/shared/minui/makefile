.PHONY: build
.PHONY: clean
	
TARGET=MinUI

###########################################################

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif
ifeq (,$(CROSS_COMPILE))
$(error missing CROSS_COMPILE for this toolchain)
endif

###########################################################

CC = $(CROSS_COMPILE)gcc
SYSROOT := $(shell $(CC) --print-sysroot)
INCLUDEDIR = $(SYSROOT)/usr/include
CFLAGS = -I$(INCLUDEDIR) -I. -I../common -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z) -std=gnu99
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lmsettings -lmlanguage -lrt -lz -lm

ifeq (trimui,$(PLATFORM))
LDFLAGS += -ltinyalsa
endif
ifeq (rs90,$(PLATFORM))
LDFLAGS += -lasound
endif

OPTM=-O3

build:
	mkdir -p build/$(PLATFORM)
	$(CC) -o build/$(PLATFORM)/$(TARGET) main.c ../common/common.c ../common/$(PLATFORM)/platform.c $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic
clean:
	rm -f build/$(PLATFORM)/$(TARGET)