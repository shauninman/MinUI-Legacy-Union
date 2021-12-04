#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include <msettings.h>

#define MIN_VOLUME 0
#define MAX_VOLUME 15
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 9

#define BUTTON_START 	KEY_ENTER
#define BUTTON_SELECT 	KEY_ESC
#define BUTTON_L 		KEY_TAB
#define BUTTON_R 		KEY_BACKSPACE

typedef enum ButtonIndex {
	kButtonStart,
	kButtonSelect,
	kButtonL,
	kButtonR,
	kButtonCount,
} ButtonIndex;

typedef struct ButtonState {
	int is_pressed;
	int just_pressed;
	int just_released;
} ButtonState;
static ButtonState buttons[kButtonCount];

typedef struct JackState {
	int is_plugged;
	int just_plugged;
	int just_unplugged;
} JackState;
static JackState jack;

#define justPressed(btn) buttons[(btn)].just_pressed
#define isPressed(btn) buttons[(btn)].is_pressed
#define justReleased(btn) buttons[(btn)].just_released
	
#define INPUT_COUNT 3
static int handles[INPUT_COUNT];

int main (int argc, char *argv[]) {
	InitSettings();
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());
	
	char path[32];
	
	for (int i=0; i<INPUT_COUNT; i++) {
		sprintf(path, "/dev/input/event%i", i);
		handles[i] = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	
	inline void handle_event(struct input_event event) {
		// printf("event.type:%i .code:%i .value:%i\n", event.type, event.code, event.value);
		if (event.type==EV_KEY) {
			int i = -1;
			switch (event.code) {
				case BUTTON_START: 	i = kButtonStart; break;
				case BUTTON_SELECT: i = kButtonSelect; break;
				case BUTTON_L: 		i = kButtonL; break;
				case BUTTON_R: 		i = kButtonR; break;
				default: return;
			}
			
			ButtonState* button = &buttons[i];
			if (event.value>0) {
				if (!button->is_pressed) {
					button->is_pressed = 1;
					button->just_pressed = 1;
				}
			}
			else {
				if (button->is_pressed) {
					button->just_released = 1;
					button->is_pressed = 0;
				}
			}
		}
		else if (event.type==EV_SW && event.code==SW_HEADPHONE_INSERT) {
			if (event.value) {
				jack.is_plugged = 1;
				jack.just_plugged = 1;
			}
			else {
				jack.just_unplugged = 1;
				jack.is_plugged = 0;
			}
		}
	}
	
	struct input_event events[32];
	int len;
	while (1) {
		// reset input
		for (int i=0; i<kButtonCount; i++) {
			buttons[i].just_pressed = 0;
			buttons[i].just_released = 0;
		}
		jack.just_plugged = 0;
		jack.just_unplugged = 0;
		
		// capture input
		for (int i=0; i<INPUT_COUNT; i++) {
			while ((len=read(handles[i], events, (sizeof events))) > 0) {
				len /= sizeof(events[0]);
				for (int j=0; j<len; j++) {
					handle_event(events[j]);
				}
			}
		}
		
		// reset buttons on sleep
		if ((isPressed(kButtonSelect) && justPressed(kButtonStart)) || (isPressed(kButtonStart) && justPressed(kButtonSelect))) {
			for (int i=0; i<kButtonCount; i++) {
				buttons[i].just_pressed = 0;
				buttons[i].is_pressed = 0;
				buttons[i].just_released = 0;
			}
			continue;
		}
		
		// handle input
		if (justReleased(kButtonSelect) && justPressed(kButtonL)) {
			int val = GetVolume();
			if (val>MIN_VOLUME) SetVolume(--val);
		}
		else if (isPressed(kButtonSelect) && justPressed(kButtonR)) {
			int val = GetVolume();
			if (val<MAX_VOLUME) SetVolume(++val);
		}
		
		if (isPressed(kButtonStart)) {
			if (justPressed(kButtonL)) {
				int val = GetBrightness();
				if (val>MIN_BRIGHTNESS) SetBrightness(--val);
			}
			else if (justPressed(kButtonR)) {
				int val = GetBrightness();
				if (val<MAX_BRIGHTNESS) SetBrightness(++val);
			}
		}
		
		if (jack.just_plugged) {
			SetJack(1);
		}
		if (jack.just_unplugged) {
			SetJack(0);
		}
		
		usleep(16666); // 60fps
	}
}
