# Union/makefile

all: setup # trimui rg350 miyoomini

setup:
	mkdir -p ./build

trimui:
	make build -f toolchain.makefile PLATFORM=trimui

rg350:
	make build -f toolchain.makefile PLATFORM=rg350

miyoomini:
	make build -f toolchain.makefile PLATFORM=miyoomini

shell:
	make -f toolchain.makefile PLATFORM=$(PLATFORM)