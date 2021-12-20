#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <string.h>

#include "msettings.h"

///////////////////////////////////////

static void Volume_set(int val) {
	char cmd[256];
	sprintf(cmd, "amixer -q sset 'PCM' %i", val);
	system(cmd);
}

///////////////////////////////////////

typedef struct Settings {
	int version; // future proofing
	int brightness;
	int headphones;
	int speaker;
	int unused[3]; // for future use
	int jack; // NOTE: doesn't really need to be persisted but still needs to be shared
} Settings;
static Settings DefaultSettings = {
	.version = 1,
	.brightness = 2,
	.headphones = 4,
	.speaker = 8,
	.jack = 0,
};
static Settings* settings;

#define SHM_KEY "/SharedSettings"
static char SettingsPath[256];
static int shm_fd = -1;
static int is_host = 0;
static int shm_size = sizeof(Settings);

void InitSettings(void) {
	sprintf(SettingsPath, "%s/.userdata/%s/msettings.bin", getenv("SDCARD_PATH"), getenv("SYSTEM_NAME"));
	
	shm_fd = shm_open(SHM_KEY, O_RDWR | O_CREAT | O_EXCL, 0644); // see if it exists
	if (shm_fd==-1 && errno==EEXIST) { // already exists
		puts("Settings client");
		shm_fd = shm_open(SHM_KEY, O_RDWR, 0644);
		settings = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	}
	else { // host
		puts("Settings host");
		is_host = 1;
		// we created it so set initial size and populate
		ftruncate(shm_fd, shm_size);
		settings = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
		
		int fd = open(SettingsPath, O_RDONLY);
		if (fd>=0) {
			read(fd, settings, shm_size);
			// TODO: use settings->version for future proofing
			close(fd);
		}
		else {
			// load defaults
			memcpy(settings, &DefaultSettings, shm_size);
		}

		// get inital headphone state
		char* headphones = getenv("HEADPHONES");
		settings->jack = (strcmp(headphones, "on")==0);
	}
	printf("brightness: %i\nheadphones (%i): %i\nspeaker: %i\n", settings->brightness, settings->jack, settings->headphones, settings->speaker);
}
void QuitSettings(void) {
	munmap(settings, shm_size);
	if (is_host) shm_unlink(SHM_KEY);
}
static inline void SaveSettings(void) {
	int fd = open(SettingsPath, O_CREAT|O_WRONLY, 0644);
	if (fd>=0) {
		write(fd, settings, shm_size);
		close(fd);
		sync();
	}
}

int GetBrightness(void) { // 0-10
	return settings->brightness;
}
void SetBrightness(int value) {
// TODO:
}

int GetVolume(void) { // 0-20
	return settings->jack ? settings->headphones : settings->speaker;
}
void SetVolume(int value) {
	// TODO:
}

int GetJack(void) {
	// TODO: /proc/jz/hp_1009==1 (in keymon?)
	return settings->jack;
}
void SetJack(int value) {
	settings->jack = value;
	SetVolume(GetVolume());
}