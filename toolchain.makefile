# Union/toolchain.makefile

.PHONY: build

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif

HOST_WORKSPACE=$(shell pwd)/workspace
GUEST_WORKSPACE=/root/workspace

all: toolchains/$(PLATFORM)-toolchain/.build
	docker run -it --rm -v $(HOST_WORKSPACE):$(GUEST_WORKSPACE) $(PLATFORM)-toolchain /bin/bash

toolchains/$(PLATFORM)-toolchain/.build:
	cd toolchains/$(PLATFORM)-toolchain && make .build

clean:
	cd toolchains/$(PLATFORM)-toolchain && make clean

build:
	docker run -it --rm -v $(HOST_WORKSPACE):$(GUEST_WORKSPACE) $(PLATFORM)-toolchain /bin/bash -c 'source ~/.bashrc && cd ~/workspace && make'