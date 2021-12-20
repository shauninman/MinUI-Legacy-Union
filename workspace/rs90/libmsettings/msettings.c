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

#include <ctype.h>
#include <alsa/asoundlib.h>

// based on https://github.com/pcercuei/pwswd
static int parse_simple_id(const char *str, snd_mixer_selem_id_t *sid) {
	int c, size;
	char buf[128];
	char *ptr = buf;

	while (*str == ' ' || *str == '\t')
		str++;
	if (!(*str))
		return -EINVAL;
	size = 1;	/* for '\0' */
	if (*str != '"' && *str != '\'') {
		while (*str && *str != ',') {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
	} else {
		c = *str++;
		while (*str && *str != c) {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
		if (*str == c)
			str++;
	}
	if (*str == '\0') {
		snd_mixer_selem_id_set_index(sid, 0);
		*ptr = 0;
		goto _set;
	}
	if (*str != ',')
		return -EINVAL;
	*ptr = 0;	/* terminate the string */
	str++;
	if (!isdigit(*str))
		return -EINVAL;
	snd_mixer_selem_id_set_index(sid, atoi(str));
_set:
	snd_mixer_selem_id_set_name(sid, buf);
	return 0;
}

static snd_mixer_selem_id_t* mixer_id = NULL;
static snd_mixer_t* mixer = NULL;
static snd_mixer_elem_t* mixer_elem;
static void Volume_init(void) {
	snd_mixer_selem_id_alloca(&mixer_id);
	
	if (parse_simple_id("Master", mixer_id)) {
		fprintf(stderr, "Wrong control identifier.\n");
		return;
	}

	if (snd_mixer_open(&mixer, 0) < 0) {
		fprintf(stderr, "Mixer open error.\n");
		return;
	}

	if (snd_mixer_attach(mixer, "default") < 0) {
		fprintf(stderr, "Mixer attach error.\n");
		snd_mixer_close(mixer);
		return;
	}

	if (snd_mixer_selem_register(mixer, NULL, NULL) < 0) {
		fprintf(stderr, "Mixer register error.\n");
		snd_mixer_close(mixer);
		return;
	}

	if (snd_mixer_load(mixer) < 0) {
		fprintf(stderr, "Mixer load error.\n");
		snd_mixer_close(mixer);
		return;
	}

	mixer_elem = snd_mixer_find_selem(mixer, mixer_id);
	if (!mixer_elem) {
		fprintf(stderr, "Unable to find control.\n");
		snd_mixer_close(mixer);
		return;
	}
}
static void Volume_quit(void) {
	// TODO: this segfaults?
	// if (mixer!=NULL) snd_mixer_close(mixer);
	// if (mixer_id!=NULL) snd_mixer_selem_id_free(mixer_id);
} // unused

static int Volume_get(void) {
	long value;

	if (snd_mixer_selem_get_playback_volume(mixer_elem, 0, &value)) {
		fprintf(stderr, "Unable to read volume.\n");
		return -1;
	}

	return value;
} // unused
static void Volume_set(int val) {
	int value = val;

	if (snd_mixer_selem_set_playback_switch_all(mixer_elem, value>0)) {
		fprintf(stderr, "Unable to toggle mute.\n");
	}

	if (snd_mixer_selem_set_playback_volume_all(mixer_elem, value)) {
		fprintf(stderr, "Unable to set volume.\n");
	}
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
	// printf("brightness: %i\nheadphones (%i): %i\nspeaker: %i\n", settings->brightness, settings->jack, settings->headphones, settings->speaker);
	Volume_init();
}
void QuitSettings(void) {
	munmap(settings, shm_size);
	if (is_host) shm_unlink(SHM_KEY);
	// Volume_quit();
}
static inline void SaveSettings(void) {
	int fd = open(SettingsPath, O_CREAT|O_WRONLY, 0644);
	if (fd>=0) {
		write(fd, settings, shm_size);
		close(fd);
		sync();
	}
}

int GetBrightness(void) {
	return settings->brightness;
}
void SetBrightness(int value) { // 1-9
	int fd = open("/sys/class/backlight/backlight/brightness", O_WRONLY);
	if (fd>=0) {
		dprintf(fd,"%d",value);
		close(fd);
	}
	settings->brightness = value;
	SaveSettings();
}

int GetVolume(void) {
	return settings->jack ? settings->headphones : settings->speaker;
}
void SetVolume(int value) { // 0-15
	if (settings->jack) settings->headphones = value;
	else settings->speaker = value;
	Volume_set(value);
	SaveSettings();
}

int GetJack(void) {
	return settings->jack;
}
void SetJack(int value) {
	settings->jack = value;
	// putenv(value ? "HEADPHONES=on" : "HEADPHONES=off");
	SetVolume(GetVolume());
}