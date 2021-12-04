// retrofw/platform.c
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "common.h"

///////////////////////////////////////

int getBatteryLevel(void) {
	int value = getInt("/proc/jz/battery");
	
	if ((value > 10000) || (value < 0)) return 5;
	if (value > 4000) return 5;
	if (value > 3900) return 4;
	if (value > 3800) return 3;
	if (value > 3700) return 2;
	if (value > 3520) return 1;
	return 0;
}

int preventAutosleep(void) {
	// this isn't working...
	// char value[16];
	// getFile("/sys/devices/platform/musb_hdrc.0/uh_cable", value);
	// return exactMatch(value, "usb");
	return 0;
}

///////////////////////////////////////

#define BACKLIGHT_PATH "/proc/jz/backlight"

// TODO: use Get/SetBrightness(), see trimui
static int backlight = 0;
static void toggleScreen(int state) {
	static int screen_state = -1;
	if (state==screen_state) return;
	else if (state==0) {
		backlight = getInt(BACKLIGHT_PATH);
		putInt(BACKLIGHT_PATH, 0);
	}
	else {
		putInt(BACKLIGHT_PATH, backlight);
	}
	screen_state = state;
}
void initPlatform(void) {
	backlight = getInt(BACKLIGHT_PATH);
}
void enterSleep(void) {
	toggleScreen(0);
}
void exitSleep(void) {
	toggleScreen(1);
}