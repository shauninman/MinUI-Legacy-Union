// odbeta/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

///////////////////////////////////////

#define BATTERY_PATH "/sys/class/power_supply/jz-battery"
int isCharging(void) {
	return getInt("/sys/class/power_supply/usb-charger/online");
}
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

int preventAutosleep(void) {
	return isCharging();
}

///////////////////////////////////////

static void toggleScreen(int state) {
	static int screen_state = -1;
	if (state==screen_state) return;
	putInt("/sys/class/graphics/fb0/blank", state ? 0 : 4);
	screen_state = state;
}

static void LED_set(int val) {
	char cmd[256];
	sprintf(cmd, "echo %i > /sys/class/leds/power/brightness", val);
	system(cmd);
}

void initPlatform(void) {
	// required for the screen to turn off properly the first time it sleeps
	toggleScreen(1);
}
void quitPlatform(void) {
	// buh
}

void enterSleep(void) {
	LED_set(1);
	toggleScreen(0);
}
void exitSleep(void) {
	LED_set(0);
	toggleScreen(1);
}