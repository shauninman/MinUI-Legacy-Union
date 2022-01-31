# Union/makefile

all: setup trimui rg350 miyoomini

setup:
	mkdir -p ./build
	cp -R ./skeleton/. ./build/PAYLOAD
	cd ./build && find . -type f -name '.keep' -delete
	cp -R ./workspace/shared/libmlanguage/languages ./build/PAYLOAD/.system/shared

common:
	make build -f toolchain.makefile PLATFORM=$(PLATFORM)
	cp ./workspace/$(PLATFORM)/keymon/keymon ./build/PAYLOAD/.system/$(PLATFORM)/bin/
	cp ./workspace/shared/libmlanguage/build/$(PLATFORM)/mlanguage ./build/PAYLOAD/.system/$(PLATFORM)/bin/
	cp ./workspace/shared/libmlanguage/build/$(PLATFORM)/libmlanguage.so ./build/PAYLOAD/.system/$(PLATFORM)/lib/
	cp ./workspace/shared/libmmenu/build/$(PLATFORM)/libmmenu.so ./build/PAYLOAD/.system/$(PLATFORM)/lib/
	cp ./workspace/$(PLATFORM)/libmsettings/libmsettings.so ./build/PAYLOAD/.system/$(PLATFORM)/lib/
	cp ./workspace/shared/minui/build/$(PLATFORM)/MinUI ./build/PAYLOAD/.system/$(PLATFORM)/paks/MinUI.pak/

trimui:
	make common PLATFORM=$@
	cp ./workspace/$@/third-party/SDL-1.2/build/.libs/libSDL-1.2.so.0.11.5 ./build/PAYLOAD/.system/$@/lib/libSDL-1.2.so.0
	cp ./workspace/shared/third-party/DinguxCommander/output/$@/DinguxCommander ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp -r ./workspace/shared/third-party/DinguxCommander/res ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp ./workspace/$@/third-party/gambatte-dms/gambatte-dms ./build/PAYLOAD/.system/$@/paks/Emus/GB.pak/

rg350:
	make common PLATFORM=$@
	cp ./workspace/shared/third-party/DinguxCommander/output/$@/DinguxCommander ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp -r ./workspace/shared/third-party/DinguxCommander/res ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp ./workspace/shared/third-party/gambatte-dms/build/gcw0/gambatte ./build/PAYLOAD/.system/$@/paks/Emus/GB.pak/

miyoomini:
	make common PLATFORM=$@
	cp ./workspace/$@/lumon/lumon ./build/PAYLOAD/.system/$@/bin/
	cp ./workspace/$@/batmon/batmon ./build/PAYLOAD/.system/$@/bin/
	cp ./workspace/$@/third-party/SDL-1.2/build/.libs/libSDL-1.2.so.0.11.5 ./build/PAYLOAD/.system/$@/lib/libSDL-1.2.so.0
	cp ./workspace/shared/third-party/DinguxCommander/output/$@/DinguxCommander ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp -r ./workspace/shared/third-party/DinguxCommander/res ./build/PAYLOAD/.system/$@/paks/Tools/Commander.pak/
	cp ./workspace/shared/third-party/gambatte-dms/build/$@/gambatte ./build/PAYLOAD/.system/$@/paks/Emus/GB.pak/
	cp ./workspace/$@/third-party/picogpsp/picogpsp ./build/PAYLOAD/.system/$@/paks/Emus/GBA.pak/
	
shell:
	make -f toolchain.makefile PLATFORM=$(PLATFORM)

clean:
	rm -rf ./build