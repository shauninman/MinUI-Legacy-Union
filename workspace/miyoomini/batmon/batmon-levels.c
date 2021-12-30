// NOTE: this is pointless because adc is always max when charging

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <pthread.h>

static int getInt(char* path) {
	int i = 0;
	FILE *file = fopen(path, "r");
	if (file!=NULL) {
		fscanf(file, "%i", &i);
		fclose(file);
	}
	return i;
}

typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;

#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static char* fb0_map;
static SAR_ADC_CONFIG_READ  adcCfg = {0,0};
static int sar_fd = 0;
#define CHARGING_IMAGES_COUNT 6
static SDL_Surface* charging_images[CHARGING_IMAGES_COUNT];
static int previous_adc = 0;
static void blitCharging(void) {
	ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
	if (adcCfg.adc_value==previous_adc) return;
	previous_adc = adcCfg.adc_value;
	
	// NOTE:  copied from common/miyoomini/platform.c:getBatteryLevel()
	int min = 505; // was 439
	int max = 546; // was 500
	int scaled = (adcCfg.adc_value - min) * 6 / (max - min);
	int value = scaled>5 ? 5 : scaled;
	
	if (charging_images[value]==NULL) {
		char charging_path[128];
		sprintf(charging_path, "%s/.system/shared/images/640x480/charging-%i.png", getenv("SDCARD_PATH"), value);
		charging_images[value] = IMG_Load(charging_path); // 24-bit opaque png
	}
	
	SDL_Surface* img = charging_images[value];
	uint8_t* dst = (uint8_t*)fb0_map; // rgba
	uint8_t* src = (uint8_t*)img->pixels; // bgr
	src += ((img->h * img->w) - 1) * 3;
	for (int y=0; y<img->h; y++) {
		for (int x=0; x<img->w; x++) {
			*(dst+0) = *(src+2); // r
			*(dst+1) = *(src+1); // g
			*(dst+2) = *(src+0); // b
			*(dst+3) = 0xf; // alpha
			dst += 4;
			src -= 3;
		}
	}
}

static int is_charging = 1;
static int screen_on = 0;
static unsigned long screen_start;
static void screenOn(void) {
	blitCharging();
	
	screen_start = SDL_GetTicks();
	if (!screen_on) system("echo 100 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
	screen_on = 1;
}
static void screenOff(void) {
	system("echo 0 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
	screen_on = 0;
}
static void checkCharging(void) {
	is_charging = getInt("/sys/devices/gpiochip0/gpio/gpio59/value");
}
static pthread_t charging_pt;
static void* chargingThread(void* arg) {
	while (1) {
		sleep(1);
		checkCharging();
	}
}

static int input_fd;
static pthread_t input_pt;
static void* inputThread(void* arg) {
	struct input_event	event;
	while (read(input_fd, &event, sizeof(event))==sizeof(event)) {
		if (event.type==EV_KEY) screenOn();
	}	
}

int main(void) {
	int fb0_fd = open("/dev/fb0", O_RDWR);
	struct fb_var_screeninfo vinfo;
	ioctl(fb0_fd, FBIOGET_VSCREENINFO, &vinfo);
	int map_size = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8); // 640x480x4
	fb0_map = (char*)mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb0_fd, 0);
	
	memset(fb0_map, 0, map_size); // clear screen
	
	sar_fd = open("/dev/sar", O_WRONLY);
	ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
	for (int i=0; i<CHARGING_IMAGES_COUNT; i++) {
		charging_images[i] = NULL;
	}

	screenOn();
	
	// turn on screen for 3 seconds
	
	// echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
	
	input_fd = open("/dev/input/event0", O_RDONLY);
	
	pthread_create(&input_pt, NULL, &inputThread, NULL);
	pthread_create(&charging_pt, NULL, &chargingThread, NULL);
	
	while (is_charging) {
		if (screen_on) {
			unsigned long now = SDL_GetTicks();
			if (now-screen_start>=3000) screenOff();
		}
	}
	
	close(input_fd);
	
	pthread_cancel(input_pt);
	pthread_join(input_pt, NULL);
	
	pthread_cancel(charging_pt);
	pthread_join(charging_pt, NULL);

	munmap(fb0_map, map_size);
	close(fb0_fd);
	close(sar_fd);
	
	for (int i=0; i<CHARGING_IMAGES_COUNT; i++) {
		if (charging_images[i]==NULL) continue;
		SDL_FreeSurface(charging_images[i]);
	}
	
	system("reboot");
	
    return EXIT_SUCCESS;
}
