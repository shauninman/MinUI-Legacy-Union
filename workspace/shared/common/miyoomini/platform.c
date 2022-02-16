// miyoomini/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

#include <msettings.h>

int isCharging(void) {
	return getInt("/sys/devices/gpiochip0/gpio/gpio59/value");
}
int getBatteryLevel(void) {
	int min = 505; // was 439
	int max = 546; // was 500
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

#define GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
static char governor[128];

void enterSleep(void) {
	SetRawVolume(-60);
	SetRawBrightness(0);
	
	// save current governor (either ondemand or performance)
	getFile(GOVERNOR_PATH, governor);
	trimTrailingNewlines(governor);
	
	system("echo powersave > " GOVERNOR_PATH);
}
void exitSleep(void) {
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());
	
	// restore previous governor
	char cmd[128];
	sprintf(cmd, "echo %s > %s", governor, GOVERNOR_PATH);
	fprintf(stdout, "restoring governor %s.\n", governor);
	
	system(cmd);
}

int preventAutosleep(void) {
	return isCharging();
}