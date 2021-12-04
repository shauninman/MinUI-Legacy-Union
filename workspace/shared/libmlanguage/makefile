.PHONY: build
.PHONY: clean

TARGET=mlanguage

###########################################################

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif
ifeq (,$(CROSS_COMPILE))
$(error missing CROSS_COMPILE for this toolchain)
endif
ifeq (,$(PREFIX))
$(error missing PREFIX for installation in toolchain)
endif

###########################################################

CC = $(CROSS_COMPILE)gcc
CFLAGS = -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z) -std=gnu99

build: 
	mkdir -p build/$(PLATFORM)
	./generate.sh
	$(CC) -c -Werror -fpic $(TARGET).c $(CFLAGS) -Wl,--no-as-needed -ldl -lrt
	$(CC) -shared -o build/$(PLATFORM)/lib$(TARGET).so $(TARGET).o
	cp $(TARGET).h $(PREFIX)/include
	cp build/$(PLATFORM)/lib$(TARGET).so $(PREFIX)/lib
	$(CC) -o build/$(PLATFORM)/$(TARGET) main.c $(CFLAGS) -lmlanguage -ldl -lrt -Ofast
clean:
	rm -f *.o
	rm -f mlanguage.h
	rm -f mlanguage_base.h
	rm -rf build
	rm -f $(PREFIX)/include/$(TARGET).h
	rm -f $(PREFIX)/lib/lib$(TARGET).so
	