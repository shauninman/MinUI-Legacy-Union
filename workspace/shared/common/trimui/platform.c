// trimui/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

#include <msettings.h>

int getBatteryLevel(void) {
	int min = 40;
	int max = 48;
	int value = getInt("/sys/devices/soc/1c23400.battery/adc");
	int scaled = (value - min) * 6 / (max - min);
	if (scaled>5) return 5;
	else return scaled;
}

void initPlatform(void) {
	// buh
}
void enterSleep(void) {
	SetRawVolume(0);
	SetRawBrightness(0);
}
void exitSleep(void) {
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());
}

int preventAutosleep(void) {
	// TODO: return 1 if /tmp/prevent-sleep (created by USB Bridge) exists?
	return 0;
}