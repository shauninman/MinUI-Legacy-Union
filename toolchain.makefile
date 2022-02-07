# Union/toolchain.makefile

.PHONY: build

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. make PLATFORM=trimui)
endif

HOST_WORKSPACE=$(shell pwd)/workspace
GUEST_WORKSPACE=/root/workspace

INIT_IF_NECESSARY=toolchains/$(PLATFORM)-toolchain/.build

all: $(INIT_IF_NECESSARY)
	docker run -it --rm -v $(HOST_WORKSPACE):$(GUEST_WORKSPACE) $(PLATFORM)-toolchain /bin/bash

$(INIT_IF_NECESSARY):
	cd toolchains/$(PLATFORM)-toolchain && make .build

clean:
	cd toolchains/$(PLATFORM)-toolchain && make clean

build: $(INIT_IF_NECESSARY)
	docker run -it --rm -v $(HOST_WORKSPACE):$(GUEST_WORKSPACE) $(PLATFORM)-toolchain /bin/bash -c 'source ~/.bashrc && cd ~/workspace && make'