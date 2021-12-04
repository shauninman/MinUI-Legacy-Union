# Union/makefile

HOST_WORKSPACE=$(shell pwd)/workspace
GUEST_WORKSPACE=/root/workspace

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif

ifeq (trimui,$(PLATFORM))
GUEST_WORKSPACE=/home/trimui/workspace
endif

all: toolchains/$(PLATFORM)-toolchain/.build
	docker run -it --rm -v $(HOST_WORKSPACE):$(GUEST_WORKSPACE) $(PLATFORM)-toolchain /bin/bash

toolchains/$(PLATFORM)-toolchain/.build:
	cd toolchains/$(PLATFORM)-toolchain && make .build

clean:
	cd toolchains/$(PLATFORM)-toolchain && make clean