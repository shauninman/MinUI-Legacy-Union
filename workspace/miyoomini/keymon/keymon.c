#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/mman.h>

#include <msettings.h>

//	Button Defines
#define	BUTTON_MENU		KEY_ESC
#define	BUTTON_SELECT	KEY_RIGHTCTRL
#define	BUTTON_START	KEY_ENTER
#define	BUTTON_L1		KEY_E
#define	BUTTON_R1		KEY_T
#define	BUTTON_L2		KEY_TAB
#define	BUTTON_R2		KEY_BACKSPACE

//	for keyshm
#define VOLUME		0
#define BRIGHTNESS	1
#define VOLMAX		20
#define BRIMAX		10

//	for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

//	for button_flag
#define SELECT_BIT	0
#define START_BIT	1
#define SELECT		(1<<SELECT_BIT)
#define START		(1<<START_BIT)

//	for DEBUG
//#define	DEBUG
#ifdef	DEBUG
#define ERROR(str)	fprintf(stderr,str"\n"); quit(EXIT_FAILURE)
#else
#define ERROR(str)	quit(EXIT_FAILURE)
#endif

//	Global Variables
struct input_event	ev;
int	input_fd = 0;
//
//	Quit
//
void quit(int exitcode) {
	QuitSettings();
	
	if (input_fd > 0) close(input_fd);
	exit(exitcode);
}

int main (int argc, char *argv[]) {
	// Set Initial Volume / Brightness
	InitSettings();
	SetVolume(GetVolume());
	SetBrightness(GetBrightness());

	fflush(stdout);
	
	input_fd = open("/dev/input/event0", O_RDONLY);

	// Main Loop
	register uint32_t val;
	register uint32_t pressedbuttons = 0;
	register uint32_t button_flag = 0;
	uint32_t repeat_START = 0; //	for suspend
	uint32_t repeat_LR = 0;
	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		val = ev.value;
		if (( ev.type != EV_KEY ) || ( val > REPEAT )) continue;
		if ( val < REPEAT ) {
			pressedbuttons += val;
			if (( val == RELEASED )&&( pressedbuttons > 0 )) pressedbuttons--;
		}
		switch (ev.code) {
		case BUTTON_SELECT:
			if ( val != REPEAT ) {
				button_flag = button_flag & (~SELECT) | (val<<SELECT_BIT);
			}
			break;
		case BUTTON_START:
			if ( val != REPEAT ) {
				button_flag = button_flag & (~START) | (val<<START_BIT);
				repeat_START = (pressedbuttons == 1 ? val : 0);
			} 
			break;
		case BUTTON_L1:
		case BUTTON_L2:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_LR;
				repeat_LR ^= PRESSED;
			} else {
				repeat_LR = 0;
			}
			if ( val == PRESSED ) {
				switch (button_flag) {
				case SELECT:
					// SELECT + L : volume down
					val = GetVolume();
					if (val>0) SetVolume(--val);
					break;
				case START:
					// START + L : brightness down
					val = GetBrightness();
					if (val>0) SetBrightness(--val);
					break;
				default:
					break;
				}
			}
			break;
		case BUTTON_R1:
		case BUTTON_R2:
			if ( val == REPEAT ) {
				// Adjust repeat speed to 1/2
				val = repeat_LR;
				repeat_LR ^= PRESSED;
			} else {
				repeat_LR = 0;
			}
			if ( val == PRESSED ) {
				switch (button_flag) {
				case SELECT:
					// SELECT + R : volume up
					val = GetVolume();
					if (val<VOLMAX) SetVolume(++val);
					break;
				case START:
					// START + R : brightness up
					val = GetBrightness();
					if (val<BRIMAX) SetBrightness(++val);
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	ERROR("Failed to read input event");
}
