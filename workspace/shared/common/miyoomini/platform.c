// miyoomini/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

#include <msettings.h>

int getBatteryLevel(void) {
	int min = 439;
	int max = 500;
	int value = getInt("/tmp/adc");
	int scaled = (value - min) * 6 / (max - min);
	if (scaled>5) return 5;
	else return scaled;
}

void initPlatform(void) {
	// buh
}
void quitPlatform(void) {
	// buh
}
void enterSleep(void) {
	SetRawVolume(40); // range is 40-100 :facepalm:
	SetRawBrightness(0);
}
void exitSleep(void) {
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());
}

int preventAutosleep(void) {
	return 0;
}