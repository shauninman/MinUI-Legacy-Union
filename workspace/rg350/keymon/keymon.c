#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/uinput.h>

#include <msettings.h>

#define MIN_VOLUME 0
#define MAX_VOLUME 20
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10

#define BUTTON_START 	KEY_ENTER
#define BUTTON_SELECT 	KEY_ESC
#define BUTTON_L 		KEY_TAB
#define BUTTON_R 		KEY_BACKSPACE

#define BUTTON_POWER		KEY_POWER
#define BUTTON_VOLUME_UP	KEY_VOLUMEUP
#define BUTTON_VOLUME_DOWN	KEY_VOLUMEDOWN

typedef enum ButtonIndex {
	kButtonStart,
	kButtonSelect,
	kButtonL,
	kButtonR,
	kButtonPower,
	kButtonVolumeUp,
	kButtonVolumeDown,
	kButtonCount,
} ButtonIndex;

typedef struct ButtonState {
	int is_pressed;
	int just_pressed;
	int just_released;
	int just_repeated;
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
#define justRepeated(btn) buttons[(btn)].just_repeated
	
#define INPUT_COUNT 3 // skip event1 which is haptic
static int handles[INPUT_COUNT];
static int uinput;

static struct uinput_user_dev uud = {
	.name="Union uinput",
	.id={BUS_USB,1,1,1}
};
static int inject(unsigned short type, unsigned short code, int value) {
	struct input_event event;
	event.type = type;
	event.code = code;
	event.value = value;
	event.time.tv_sec = time(0);
	event.time.tv_usec = 0;
	return write(uinput, &event, sizeof(struct input_event));
}

static void LED_set(int val) {
	char cmd[256];
	sprintf(cmd, "echo %i > /sys/class/leds/power/brightness", val);
	system(cmd);
}

#define POWEROFF_TIMEOUT 3
int main (int argc, char *argv[]) {
	InitSettings();
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());
	
	LED_set(0);
	
	char path[32];

	for (int i=0; i<INPUT_COUNT; i++) {
		if (i==1) continue; // haptic
		sprintf(path, "/dev/input/event%i", i);
		handles[i] = open(path, O_RDONLY | O_NONBLOCK); // | O_CLOEXEC);
		// O_CLOEXEC isn't available on stock dingux?
		// that should be okay since keymon is never killed except during poweroff
	}
	
	// setup uinput for inject
	uinput = open("/dev/uinput", O_RDWR);
	write(uinput, &uud, sizeof(uud));
	ioctl(uinput, UI_SET_EVBIT, EV_KEY);
	ioctl(uinput, UI_SET_KEYBIT, KEY_HOME);
	ioctl(uinput, UI_DEV_CREATE);
	
	inline void handle_event(struct input_event event) {
		// printf("event.type:%i .code:%i .value:%i\n", event.type, event.code, event.value);
		if (event.type==EV_KEY) {
			int i = -1;
			switch (event.code) {
				case BUTTON_START: 			i = kButtonStart; break;
				case BUTTON_SELECT: 		i = kButtonSelect; break;
				case BUTTON_L: 				i = kButtonL; break;
				case BUTTON_R: 				i = kButtonR; break;
				case BUTTON_POWER: 			i = kButtonPower; break;
				case BUTTON_VOLUME_UP: 		i = kButtonVolumeUp; break;
				case BUTTON_VOLUME_DOWN:	i = kButtonVolumeDown; break;
				default: return;
			}
			
			ButtonState* button = &buttons[i];
			if (event.value>0) {
				if (!button->is_pressed) {
					button->is_pressed = 1;
					button->just_pressed = 1;
					button->just_repeated = 1;
				}
				else {
					button->just_repeated = !button->just_repeated;
				}
			}
			else {
				if (button->is_pressed) {
					button->just_released = 1;
					button->is_pressed = 0;
					button->just_repeated = 0;
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
	
	inline void blank(void) {
		FILE* file = fopen("/sys/class/graphics/fb0/blank", "w");
		if (file) {
			fprintf(file, "1\n");
			fclose(file);
		}
	}
	
	struct input_event events[32];
	int len;
	int chord = 0;
	unsigned long power_start;
	struct timeval tv;
	int poweroff = 0;
	while (1) {
		// reset input
		for (int i=0; i<kButtonCount; i++) {
			buttons[i].just_pressed = 0;
			buttons[i].just_released = 0;
			buttons[i].just_repeated = 0;
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
		
		// TODO: fairly certain this is RS90 specific...
		// // reset buttons on sleep
		// if ((isPressed(kButtonSelect) && justPressed(kButtonStart)) || (isPressed(kButtonStart) && justPressed(kButtonSelect))) {
		// 	for (int i=0; i<kButtonCount; i++) {
		// 		buttons[i].just_pressed = 0;
		// 		buttons[i].is_pressed = 0;
		// 		buttons[i].just_released = 0;
		// 	}
		// 	continue;
		// }
		
		// handle input
		if (justPressed(kButtonPower)) {
			chord = 0;
			gettimeofday(&tv, NULL);
			power_start = tv.tv_sec * 1000000 + tv.tv_usec;
		}
		
		if (!chord && !poweroff && isPressed(kButtonPower)) {
			gettimeofday(&tv, NULL);
			unsigned long power_now = tv.tv_sec * 1000000 + tv.tv_usec;
			
			if (power_now-power_start>=POWEROFF_TIMEOUT*1000000) {
				poweroff = 1;
				ioctl(handles[0], EVIOCGRAB, 1);
				blank();
			}
		}
		
		if (poweroff) {
			if (justReleased(kButtonPower)) break;
			else continue;
		}
		
		if (
			(isPressed(kButtonSelect) && justRepeated(kButtonL)) || 
			(!isPressed(kButtonPower) && justRepeated(kButtonVolumeDown))
		) {
			int val = GetVolume();
			if (val>MIN_VOLUME) SetVolume(--val);
			chord = 1;
		}
		else if (
			(isPressed(kButtonSelect) && justRepeated(kButtonR)) || 
			(!isPressed(kButtonPower) && justRepeated(kButtonVolumeUp))
		) {
			int val = GetVolume();
			if (val<MAX_VOLUME) SetVolume(++val);
			chord = 1;
		}
		
		if (
			(isPressed(kButtonStart) && justRepeated(kButtonL)) || 
			(isPressed(kButtonPower) && justRepeated(kButtonVolumeDown))
		) { 
			int val = GetBrightness();
			if (val>MIN_BRIGHTNESS) SetBrightness(--val);
			chord = 1;
		}
		else if (
			(isPressed(kButtonStart) && justRepeated(kButtonR)) || 
			(isPressed(kButtonPower) && justRepeated(kButtonVolumeUp))
		) {
			int val = GetBrightness();
			if (val<MAX_BRIGHTNESS) SetBrightness(++val);
			chord = 1;
		}
		
		if (!chord && justReleased(kButtonPower)) {
			inject(EV_KEY, KEY_HOME, 1);
			inject(EV_KEY, KEY_HOME, 0);
			inject(EV_SYN, SYN_REPORT, 0);
		}
		
		if (jack.just_plugged) {
			SetJack(1);
		}
		if (jack.just_unplugged) {
			SetJack(0);
		}
		
		usleep(16666); // 60fps
	}
	
	LED_set(1);
	execlp("/sbin/poweroff", "/sbin/poweroff", (char*)NULL);
}
