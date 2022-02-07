.PHONY: clean

###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

###########################################################

all: sys

lib:
	cd ./$(PLATFORM)/libmsettings && make
	cd ./shared/libmlanguage && make
	cd ./shared/libmmenu && make
	
sys: lib
	cd ./$(PLATFORM)/keymon && make
	cd ./$(PLATFORM) && make
	cd ./shared/minui && make

clean:
	rm -rf ./build
	cd ./$(PLATFORM)/libmsettings && make clean
	cd ./shared/libmlanguage && make clean
	cd ./shared/libmmenu && make clean
	
	cd ./$(PLATFORM)/keymon && make clean
	cd ./$(PLATFORM) && make clean
	cd ./shared/minui && make clean