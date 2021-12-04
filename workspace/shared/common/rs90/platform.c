// rs90/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

///////////////////////////////////////

#define BATTERY_PATH "/sys/class/power_supply/jz-battery"

int getBatteryLevel(void) {
	static int min;
	static int max;
	static int ready;
	if (!ready) {
		min = getInt(BATTERY_PATH "/voltage_min_design");
		max = getInt(BATTERY_PATH "/voltage_max_design");
		ready = 1;
	}
	int value = getInt(BATTERY_PATH "/voltage_now");

	int scaled = (value - min) * 6 / (max - min);
	if (scaled>5) return 5;
	else return scaled;
}

///////////////////////////////////////

static void toggleScreen(int state) {
	static int screen_state = -1;
	if (state==screen_state) return;
	putInt("/sys/class/graphics/fb0/blank", state ? 0 : 4);
	screen_state = state;
}

void initPlatform(void) {
	// required for the screen to turn off properly the first time it sleeps
	toggleScreen(1);
}
void enterSleep(void) {
	toggleScreen(0);
}
void exitSleep(void) {
	toggleScreen(1);
}

int preventAutosleep(void) {
	return getInt("/sys/class/power_supply/usb-charger/online"); // TODO: test
}