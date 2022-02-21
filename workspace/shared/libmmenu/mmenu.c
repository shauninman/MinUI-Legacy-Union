#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <msettings.h>
#include <mlanguage.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/common.h"
#include "../common/platform.h"

#include "mmenu.h"

static Language lang;
static SDL_Surface* overlay;
static SDL_Surface* screen;
static SDL_Surface* slot_overlay;
static SDL_Surface* slot_dots;
static SDL_Surface* slot_dot_selected;
static SDL_Surface* arrow;
static SDL_Surface* arrow_highlighted;
#ifdef PLATFORM_RS90
static SDL_Surface* btn_select_start;
#endif
static SDL_Surface* no_preview;
static SDL_Surface* empty_slot;

enum {
	kItemContinue,
	kItemSave,
	kItemLoad,
	kItemAdvanced,
	kItemExitGame,
};
#define kItemCount 5
static char* items[kItemCount];

#define kSlotCount 8
static int slot = 0;

__attribute__((constructor)) static void init(void) {
	Union_init();
	
#ifdef PLATFORM_TRIMUI
	void* libtinyalsa = dlopen("libtinyalsa.so", RTLD_LAZY | RTLD_GLOBAL); // mixer
#elif PLATFORM_RS90
	void* libtinyalsa = dlopen("libasound.so", RTLD_LAZY | RTLD_GLOBAL); // mixer
#endif
	void* librt = dlopen("librt.so.1", RTLD_LAZY | RTLD_GLOBAL); // shm
	void* libmsettings = dlopen("libmsettings.so", RTLD_LAZY | RTLD_GLOBAL);
	InitSettings();

	void* libmlanguage = dlopen("libmlanguage.so", RTLD_LAZY | RTLD_GLOBAL);
	InitLanguage(&lang);
	
	items[kItemContinue] 	= lang.continue_;
	items[kItemSave] 		= lang.save;
	items[kItemLoad] 		= lang.load;
	items[kItemAdvanced] 	= lang.advanced;
	items[kItemExitGame] 	= lang.exit;
	
	GFX_init(lang.CJK!=NULL);
	
	overlay = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	SDL_SetAlpha(overlay, SDL_SRCALPHA, 0x80);
	SDL_FillRect(overlay, NULL, 0);
	
	slot_overlay = GFX_loadImage("slot-overlay.png");
	slot_dots = GFX_loadImage("slot-dots.png");
	slot_dot_selected = GFX_loadImage("slot-dot-selected.png");
	arrow = GFX_loadImage("arrow.png");
	arrow_highlighted = GFX_loadImage("arrow-highlighted.png");
#ifdef PLATFORM_RS90
	btn_select_start = GFX_loadImage("btn-select-start.png");
#endif
	no_preview = GFX_getText(lang.no_preview, 0, 1);
	empty_slot = GFX_getText(lang.empty_slot, 0, 1);
}
__attribute__((destructor)) static void quit(void) {
	SDL_FreeSurface(overlay);
	SDL_FreeSurface(slot_overlay);
	SDL_FreeSurface(slot_dots);
	SDL_FreeSurface(slot_dot_selected);
	SDL_FreeSurface(arrow);
	SDL_FreeSurface(arrow_highlighted);
#ifdef PLATFORM_RS90
	SDL_FreeSurface(btn_select_start);
#endif
	SDL_FreeSurface(no_preview);
	SDL_FreeSurface(empty_slot);
}

typedef struct __attribute__((__packed__)) uint24_t {
	uint8_t a,b,c;
} uint24_t;
static SDL_Surface* createThumbnail(SDL_Surface* src_img) {
	SDL_Surface* dst_img = SDL_CreateRGBSurface(0,Screen.width/2, Screen.height/2,src_img->format->BitsPerPixel,src_img->format->Rmask,src_img->format->Gmask,src_img->format->Bmask,src_img->format->Amask);

	uint8_t* src_px = src_img->pixels;
	uint8_t* dst_px = dst_img->pixels;
	int step = dst_img->format->BytesPerPixel;
	int step2 = step * 2;
	int stride = src_img->pitch;
	for (int y=0; y<dst_img->h; y++) {
		for (int x=0; x<dst_img->w; x++) {
			switch(step) {
				case 1:
					*dst_px = *src_px;
					break;
				case 2:
					*(uint16_t*)dst_px = *(uint16_t*)src_px;
					break;
				case 3:
					*(uint24_t*)dst_px = *(uint24_t*)src_px;
					break;
				case 4:
					*(uint32_t*)dst_px = *(uint32_t*)src_px;
					break;
			}
			dst_px += step;
			src_px += step2;
		}
		src_px += stride;
	}

	return dst_img;
}


MenuReturnStatus ShowMenu(char* rom_path, char* save_path_template, SDL_Surface* optional_snapshot) {
	screen = SDL_GetVideoSurface();
	// if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen); // fix for regba?
	
	GFX_ready();
	
	SDL_EnableKeyRepeat(300,100);
	
	// path and string things
	char* tmp;
	char rom_file[256]; // with extension
	char rom_name[256]; // without extension or cruft
	char slot_path[256];
	char emu_name[256];
	char mmenu_dir[256];
	getDisplayName(rom_path, rom_name);
	
	// filename
	tmp = strrchr(rom_path,'/');
	if (tmp==NULL) tmp = rom_path;
	else tmp += 1;
	strcpy(rom_file, tmp);
	
	getEmuName(rom_path, emu_name);
	sprintf(mmenu_dir, "%s/.mmenu/%s", Paths.userdataDir, emu_name); // /.userdata/<platform>/.mmenu/<EMU>
	sprintf(slot_path, "%s/%s.txt", mmenu_dir, rom_file); // /.userdata/<platform>/.mmenu/<EMU>/<romname>.ext.txt
	mkdir(mmenu_dir, 0755);

	// does this game have an m3u?
	int rom_disc = -1;
	int disc = rom_disc;
	int total_discs = 0;
	char disc_name[16];
	char* disc_paths[9]; // up to 9 paths, Arc the Lad Collection is 7 discs
	char ext[8];
	tmp = strrchr(rom_path, '.');
	strncpy(ext, tmp, 8);
	for (int i=0; i<4; i++) {
		ext[i] = tolower(ext[i]);
	}
	// only check for m3u if rom is a cue
	if (strncmp(ext, ".cue", 8)==0) {
		// construct m3u path based on parent directory
		char m3u_path[256];
		strcpy(m3u_path, rom_path);
		tmp = strrchr(m3u_path, '/') + 1;
		tmp[0] = '\0';
	
		// path to parent directory
		char base_path[256];
		strcpy(base_path, m3u_path);
	
		tmp = strrchr(m3u_path, '/');
		tmp[0] = '\0';
	
		// get parent directory name
		char dir_name[256];
		tmp = strrchr(m3u_path, '/');
		strcpy(dir_name, tmp);
	
		// dir_name is also our m3u file name
		tmp = m3u_path + strlen(m3u_path); 
		strcpy(tmp, dir_name);
	
		// add extension
		tmp = m3u_path + strlen(m3u_path);
		strcpy(tmp, ".m3u");
	
		if (exists(m3u_path)) {
			//read m3u file
			FILE* file = fopen(m3u_path, "r");
			if (file) {
				char line[256];
				while (fgets(line,256,file)!=NULL) {
					int len = strlen(line);
					if (len>0 && line[len-1]=='\n') {
						line[len-1] = 0; // trim newline
						len -= 1;
						if (len>0 && line[len-1]=='\r') {
							line[len-1] = 0; // trim Windows newline
							len -= 1;
						}
					}
					if (len==0) continue; // skip empty lines
			
					char disc_path[256];
					strcpy(disc_path, base_path);
					tmp = disc_path + strlen(disc_path);
					strcpy(tmp, line);
					
					// found a valid disc path
					if (exists(disc_path)) {
						disc_paths[total_discs] = strdup(disc_path);
						// matched our current disc
						if (exactMatch(disc_path, rom_path)) {
							rom_disc = total_discs;
							disc = rom_disc;
							sprintf(disc_name, "Disc %i", disc+1);
						}
						total_discs += 1;
					}
				}
				fclose(file);
			}
		}
	}
	
	// cache static elements
	
	// NOTE: original screen copying logic
	// SDL_Surface* copy = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	// SDL_BlitSurface(screen, NULL, copy, NULL);

	// NOTE: copying the screen to a new surface caused a 15fps drop in PocketSNES
	// on the 280M running stock OpenDingux after opening the menu, 
	// tried ConvertSurface and DisplaySurface too
	// only this direct copy worked without tanking the framerate
	int copy_bytes = screen->h * screen->pitch;
	void* copy_pixels = malloc(copy_bytes);
	memcpy(copy_pixels, screen->pixels, copy_bytes);
	SDL_Surface* copy = SDL_CreateRGBSurfaceFrom(copy_pixels, screen->w,screen->h, screen->format->BitsPerPixel, screen->pitch, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	
	SDL_Surface* cache = SDL_CreateRGBSurface(SDL_SWSURFACE, Screen.width, Screen.height, 16, 0, 0, 0, 0);
	SDL_BlitSurface(copy, NULL, cache, NULL);
	SDL_BlitSurface(overlay, NULL, cache, NULL);

	SDL_FillRect(cache, &(SDL_Rect){0,0,Screen.width,Screen.menu.bar_height}, 0);
	GFX_blitRule(cache, Screen.menu.rule.top_y);

	GFX_blitText(cache, rom_name, 1, Screen.menu.title.x, Screen.menu.title.y, Screen.menu.title.width, 1, 0);
	
	GFX_blitWindow(cache, Screen.menu.window.x, Screen.menu.window.y, Screen.menu.window.width, Screen.menu.window.height, 1);
	
	SDL_FillRect(cache, &(SDL_Rect){0,Screen.height-Screen.menu.bar_height,Screen.width,Screen.menu.bar_height}, 0);
	GFX_blitRule(cache, Screen.menu.rule.bottom_y);
	
	Input_reset();
	
	int gold_rgb = SDL_MapRGB(screen->format, GOLD_TRIAD);
	SDL_Color gold = (SDL_Color){GOLD_TRIAD};
	SDL_Color white = (SDL_Color){WHITE_TRIAD};
	
	int status = kStatusContinue;
	int selected = 0; // resets every launch
	if (exists(slot_path)) {
		char tmp[16];
		getFile(slot_path, tmp);
		slot = atoi(tmp);
	}
	
	char save_path[256];
	char bmp_path[324];
	int save_exists = 0;
	int preview_exists = 0;
	
	int quit = 0;
	int dirty = 1;
	int show_setting = 0; // 1=brightness,2=volume
	int setting_value = 0;
	int setting_min = 0;
	int setting_max = 0;
	int select_is_locked = 0; // rs90-only
	unsigned long cancel_start = SDL_GetTicks();
	int was_charging = isCharging();
	unsigned long charge_start = SDL_GetTicks();
	while (!quit) {
		unsigned long frame_start = SDL_GetTicks();
		int select_was_pressed = Input_isPressed(kButtonSelect); // rs90-only
		
		Input_poll();
		
		// rs90-only
		if (select_was_pressed && Input_justReleased(kButtonSelect) && Input_justPressed(kButtonL)) {
			select_is_locked = 1;
		}
		else if (select_is_locked && Input_justReleased(kButtonL)) {
			select_is_locked = 0;
		}
		
		if (Input_justPressed(kButtonUp)) {
			selected -= 1;
			if (selected<0) selected += kItemCount;
			dirty = 1;
		}
		else if (Input_justPressed(kButtonDown)) {
			selected += 1;
			if (selected==kItemCount) selected -= kItemCount;
			dirty = 1;
		}
		else if (Input_justPressed(kButtonLeft)) {
			if (total_discs && selected==kItemContinue) {
				disc -= 1;
				if (disc<0) disc += total_discs;
				dirty = 1;
				sprintf(disc_name, lang.disc_num, disc+1);
			}
			else if (selected==kItemSave || selected==kItemLoad) {
				slot -= 1;
				if (slot<0) slot += kSlotCount;
				dirty = 1;
			}
		}
		else if (Input_justPressed(kButtonRight)) {
			if (total_discs && selected==kItemContinue) {
				disc += 1;
				if (disc==total_discs) disc -= total_discs;
				dirty = 1;
				sprintf(disc_name, lang.disc_num, disc+1);
			}
			else if (selected==kItemSave || selected==kItemLoad) {
				slot += 1;
				if (slot==kSlotCount) slot -= kSlotCount;
				dirty = 1;
			}
		}
		
		if (dirty && (selected==kItemSave || selected==kItemLoad)) {
			sprintf(save_path, save_path_template, slot);
			sprintf(bmp_path, "%s/%s.%d.bmp", mmenu_dir, rom_file, slot);
		
			save_exists = exists(save_path);
			preview_exists = save_exists && exists(bmp_path);
			// printf("save_path: %s (%i)\n", save_path, save_exists);
			// printf("bmp_path: %s (%i)\n", bmp_path, preview_exists);
		}
		
		if (Input_justPressed(kButtonB)) {
			status = kStatusContinue;
			quit = 1;
		}
		else if (Input_justPressed(kButtonA)) {
			switch(selected) {
				case kItemContinue:
				if (total_discs && rom_disc!=disc) {
						status = kStatusChangeDisc;
						char* disc_path = disc_paths[disc];
						char last_path[256];
						getFile(kLastPath, last_path);
						if (!exactMatch(last_path, Paths.fauxRecentDir)) {
							putFile(kLastPath, disc_path);
						}
						putFile(kChangeDiscPath, disc_path);
					}
					else {
						status = kStatusContinue;
					}
				break;
				case kItemSave:
					status = kStatusSaveSlot + slot;
					SDL_Surface* preview = createThumbnail(optional_snapshot ? optional_snapshot : copy);
					SDL_RWops* out = SDL_RWFromFile(bmp_path, "wb");
					SDL_SaveBMP_RW(preview, out, 1);
					SDL_FreeSurface(preview);
				break;
				case kItemLoad:
					status = kStatusLoadSlot + slot;
				break;
				case kItemAdvanced:
					status = kStatusOpenMenu;
				break;
				case kItemExitGame:
					status = kStatusExitGame;
				break;
			}
			
			if (selected==kItemSave || selected==kItemLoad) {
				char slot_str[8];
				sprintf(slot_str, "%d", slot);
				putFile(slot_path, slot_str);
			}
			quit = 1;
			break;
		}
		
		unsigned long now = SDL_GetTicks();
		if (Input_anyPressed()) cancel_start = now;

		#define kChargeDelay 1000
		if (dirty || now-charge_start>=kChargeDelay) {
			int is_charging = isCharging();
			if (was_charging!=is_charging) {
				was_charging = is_charging;
				dirty = 1;
			}
			charge_start = now;
		}

		#define kSleepDelay 30000
		if (now-cancel_start>=kSleepDelay && preventAutosleep()) cancel_start = now;
		
#ifdef PLATFORM_RS90
		if (now-cancel_start>=kSleepDelay || (Input_isPressed(kButtonSelect) && Input_justPressed(kButtonStart)) || (Input_justPressed(kButtonSelect) && Input_isPressed(kButtonStart))) 
#else
		if (now-cancel_start>=kSleepDelay || Input_justPressed(kButtonSleep) || Input_justPressed(kButtonSleepAlt))
#endif
		{
			fauxSleep();
			cancel_start = SDL_GetTicks();
			dirty = 1;
		}
		
		int old_setting = show_setting;
		int old_value = setting_value;
		show_setting = 0;
		if (Input_isPressed(kButtonStart) && Input_isPressed(kButtonSelect)) {
			// buh
		}
		else if (Input_isPressed(kButtonStart)) {
			show_setting = 1;
			setting_value = GetBrightness();
			setting_min = MIN_BRIGHTNESS;
			setting_max = MAX_BRIGHTNESS;
		}
		else if (Input_isPressed(kButtonSelect) || select_is_locked) {
			show_setting = 2;
			setting_value = GetVolume();
			setting_min = MIN_VOLUME;
			setting_max = MAX_VOLUME;
		}
		if (old_setting!=show_setting || old_value!=setting_value) dirty = 1;
		
		if (dirty) {
			dirty = 0;
			SDL_BlitSurface(cache, NULL, screen, NULL);
			GFX_blitBattery(screen, Screen.menu.battery.x, Screen.menu.battery.y);
			
			if (show_setting) {
				GFX_blitSettings(screen, Screen.menu.settings.x, Screen.menu.settings.y, show_setting==1?0:(setting_value>0?1:2), setting_value,setting_min,setting_max);
			}
			
			// list
			SDL_Surface* text;
			for (int i=0; i<kItemCount; i++) {
				char* item = items[i];
				
				int color = 1; // gold
				if (i==selected) {
					SDL_FillRect(screen, &(SDL_Rect){Screen.menu.window.x,Screen.menu.list.y+(i*Screen.menu.list.line_height)-((Screen.menu.list.row_height-Screen.menu.list.line_height)/2),Screen.menu.window.width,Screen.menu.list.row_height}, gold_rgb);
					color = 0; // white
				}
				
				GFX_blitText(screen, item, 2, Screen.menu.list.x, Screen.menu.list.y+(i*Screen.menu.list.line_height)+Screen.menu.list.oy, 0, color, i==selected);
				
				if (i==kItemSave || i==kItemLoad || (total_discs && i==kItemContinue)) {
					SDL_BlitSurface(i==selected?arrow_highlighted:arrow, NULL, screen, &(SDL_Rect){Screen.menu.window.x+Screen.menu.window.width-(arrow->w+Screen.menu.arrow.ox),Screen.menu.list.y+(i*Screen.menu.list.line_height)+Screen.menu.arrow.oy});
				}
			}
			
			// disc change
			if (total_discs && selected==kItemContinue) {
				GFX_blitWindow(screen, Screen.menu.preview.x, Screen.menu.preview.y, Screen.menu.preview.width, Screen.menu.list.row_height+(Screen.menu.disc.oy*2), 1);
				GFX_blitText(screen, disc_name, 2, Screen.menu.preview.x+Screen.menu.disc.ox, Screen.menu.list.y+Screen.menu.list.oy, 0, 1, 0);
			}
			// slot preview
			else if (selected==kItemSave || selected==kItemLoad) {
				// preview window
				SDL_Rect preview_rect = {Screen.menu.preview.x+Screen.menu.preview.inset,Screen.menu.preview.y+Screen.menu.preview.inset};
				GFX_blitWindow(screen, Screen.menu.preview.x, Screen.menu.preview.y, Screen.menu.preview.width, Screen.menu.preview.height, 1);
				
				if (preview_exists) { // has save, has preview
					SDL_Surface* preview = IMG_Load(bmp_path);
					if (!preview) printf("IMG_Load: %s\n", IMG_GetError());
					SDL_BlitSurface(preview, NULL, screen, &preview_rect);
					SDL_FreeSurface(preview);
				}
				else {
					int hw = Screen.width / 2;
					int hh = Screen.height / 2;
					SDL_FillRect(screen, &(SDL_Rect){Screen.menu.preview.x+Screen.menu.preview.inset,Screen.menu.preview.y+Screen.menu.preview.inset,hw,hh}, 0);
					if (save_exists) { // has save but no preview
						SDL_BlitSurface(no_preview, NULL, screen, &(SDL_Rect){
							Screen.menu.preview.x+Screen.menu.preview.inset+(hw-no_preview->w)/2,
							Screen.menu.preview.y+Screen.menu.preview.inset+(hh-no_preview->h)/2
						});
					}
					else { // no save
						SDL_BlitSurface(empty_slot, NULL, screen, &(SDL_Rect){
							Screen.menu.preview.x+Screen.menu.preview.inset+(hw-empty_slot->w)/2,
							Screen.menu.preview.y+Screen.menu.preview.inset+(hh-empty_slot->h)/2
						});
					}
				}
				
				SDL_BlitSurface(slot_overlay, NULL, screen, &preview_rect);
				SDL_BlitSurface(slot_dots, NULL, screen, &(SDL_Rect){Screen.menu.slots.x,Screen.menu.slots.y});
				SDL_BlitSurface(slot_dot_selected, NULL, screen, &(SDL_Rect){Screen.menu.slots.x+(Screen.menu.slots.ox*slot),Screen.menu.slots.y});
			}
			
			// TODO: can this be cached?
#ifdef PLATFORM_RS90
			SDL_BlitSurface(btn_select_start, NULL, screen, &(SDL_Rect){8,Screen.menu.buttons.top-2});
			GFX_blitHint(screen, lang.sleep, 48, Screen.menu.buttons.top+Screen.hint.text_oy);
#else
			GFX_blitPill(screen, HINT_SLEEP, lang.sleep, Screen.buttons.left, Screen.menu.buttons.top);
#endif	
			// TODO: change ACT to OKAY?
			int button_width = GFX_blitButton(screen, "A", lang.act, -Screen.buttons.right, Screen.menu.buttons.top, Screen.button.text.ox_A);
			GFX_blitButton(screen, "B", lang.back, -(Screen.buttons.right+button_width+Screen.buttons.gutter),Screen.menu.buttons.top, Screen.button.text.ox_B);
			// TODO: /can this be cached?
			
			SDL_Flip(screen);
		}
		
		// slow down to 60fps
		unsigned long frame_duration = SDL_GetTicks() - frame_start;
		#define kTargetFrameDuration 17
		if (frame_duration<kTargetFrameDuration) SDL_Delay(kTargetFrameDuration-frame_duration);
	}
	
	// redraw original screen before returning
	if (status!=kStatusOpenMenu) {
		// reset all buffers (1 more than triple because it seems to be possible to miss one?)
		for (int i=0; i<4; i++) {
			SDL_FillRect(screen, NULL, 0); // TODO: is this still necessary?
			SDL_BlitSurface(copy, NULL, screen, NULL);
			SDL_Flip(screen);
		}
	}

	SDL_FreeSurface(cache);
	// NOTE: copy->pixels was manually malloc'd so it must be manually freed too
	SDL_FreeSurface(copy);
	free(copy_pixels); 
	
	SDL_EnableKeyRepeat(0,0);
	
	// if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen); // fix for regba?
	
	return status;
}

int ResumeSlot(void) {
	if (!exists(kResumeSlotPath)) return -1;
	
	char tmp[16];
	getFile(kResumeSlotPath, tmp);
	unlink(kResumeSlotPath);
	slot = atoi(tmp); // update slot so mmenu has it preselected as well
	return slot;
}

int ChangeDisc(char* disc_path) {
	if (!exists(kChangeDiscPath)) return 0;
	getFile(kChangeDiscPath, disc_path);
	return 1;
}