#ifndef COMMON_H
#define COMMON_H

#include <SDL/SDL.h>

// TODO: so I'm not loving how these are so removed from where they are used, 
// especially the layout coordinates...and how it started out MinUI-centric
// but now I'm adding mmenu coords too...maybe it's fine...this is fire, I mean fine
#if defined PLATFORM_TRIMUI
#define PLATFORM_NAME "trimui"
#define PLATFORM_ROOT "/mnt/SDCARD"
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_S 12
#define FONT_M 14
#define FONT_L 16
#define FONT_S_OY 4 // unused but potentially useful
#define FONT_M_OY 5 // unused but potentially useful
#define FONT_L_OY 5 // unused but potentially useful
#define LIST_ROWS 5
#define LIST_Y 38
#define LIST_OX 16
#define LIST_OY 4
#define LIST_ROW_HEIGHT 32
#define LIST_ALT_OX 0 // from right
#define LIST_ALT_OY 0
#define SHADOW_OX 1
#define SHADOW_OY 2
#define BUTTON_SIZE 22
#define BUTTON_TEXT_OY -1
#define A_BUTTON_TEXT_OX 0
#define B_BUTTON_TEXT_OX 1
#define X_BUTTON_TEXT_OX 1
#define Y_BUTTON_TEXT_OX 1
#define RESUME_BUTTON_TEXT_OX X_BUTTON_TEXT_OX
#define PILL_PAD 6
#define PILL_TEXT_OY 2
#define HINT_OX 3
#define HINT_SLEEP "MENU"
#define HINT_RESUME "X"
#define HINT_TEXT_OY 1
#define LOGO_X 10
#define LOGO_Y 10
#define LOCATION_Y 8
#define BATTERY_X 300
#define BATTERY_Y 9
#define RULE_TOP_Y 35
#define RULE_BOTTOM_Y 203
#define BUTTON_ROW_TOP 211
#define BUTTON_ROW_LEFT 8
#define BUTTON_ROW_RIGHT 10
#define BUTTON_ROW_GUTTER 8
#define MIN_VOLUME 0
#define MAX_VOLUME 20
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10
#define SETTINGS_WIDTH 146
#define SETTINGS_HEIGHT 26
#define SETTINGS_ICON_OX 6
#define SETTINGS_ICON_OY 4
#define SETTINGS_BAR_OX 30
#define SETTINGS_BAR_OY 11
#define SETTINGS_BAR_WIDTH 108
#define LIST_SETTINGS_X 171
#define LIST_SETTINGS_Y 4
#define MENU_SETTINGS_X 87
#define MENU_SETTINGS_Y 37
#define TEXTBOX_LINE_HEIGHT 24 // FONT_L
#define MENU_BATTERY_X 303
#define MENU_BATTERY_Y 5
#define MENU_BAR_HEIGHT 30
#define MENU_RULE_TOP_Y 27
#define MENU_RULE_BOTTOM_Y 211
#define PREVIEW_X 148
#define PREVIEW_Y 71
#define PREVIEW_WIDTH 166
#define PREVIEW_HEIGHT 135
#define PREVIEW_INSET 3
#define MENU_TITLE_X 24
#define MENU_TITLE_Y 3
#define MENU_TITLE_WIDTH 272
#define MENU_BUTTON_ROW_TOP 217
#define MENU_WINDOW_X 6
#define MENU_WINDOW_Y 71
#define MENU_WINDOW_WIDTH 140
#define MENU_WINDOW_HEIGHT 135
#define MENU_LIST_X 14
#define MENU_LIST_Y 76
#define MENU_LIST_OY 0
#define MENU_LIST_LINE_HEIGHT 25
#define MENU_LIST_ROW_HEIGHT 27
#define MENU_ARROW_OX 6 // NOTE: from right edge
#define MENU_ARROW_OY 7 
#define SLOTS_X 200
#define SLOTS_Y 197
#define SLOTS_OX 8
#elif defined (PLATFORM_RG350) || (PLATFORM_ODBETA)
#if defined PLATFORM_RG350
#define PLATFORM_NAME "rg350"
#else
#define PLATFORM_NAME "odbeta"
#endif
// TODO: tmp values
#define PLATFORM_ROOT "/media/sdcard"
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_S 12
#define FONT_M 14
#define FONT_L 16
#define FONT_S_OY 4 // unused but potentially useful
#define FONT_M_OY 5 // unused but potentially useful
#define FONT_L_OY 5 // unused but potentially useful
#define LIST_ROWS 5
#define LIST_Y 38
#define LIST_OX 16
#define LIST_OY 4
#define LIST_ROW_HEIGHT 32
#define LIST_ALT_OX 32 // from right
#define LIST_ALT_OY 10
#define SHADOW_OX 1
#define SHADOW_OY 2
#define BUTTON_SIZE 22
#define BUTTON_TEXT_OY -1
#define A_BUTTON_TEXT_OX 0
#define B_BUTTON_TEXT_OX 1
#define X_BUTTON_TEXT_OX 1
#define Y_BUTTON_TEXT_OX 1
#define RESUME_BUTTON_TEXT_OX X_BUTTON_TEXT_OX
#define PILL_PAD 6
#define PILL_TEXT_OY 2
#define HINT_OX 3
#define HINT_SLEEP "POWER"
#define HINT_RESUME "X"
#define HINT_TEXT_OY 1
#define LOGO_X 10
#define LOGO_Y 10
#define LOCATION_Y 8
#define BATTERY_X 300
#define BATTERY_Y 9
#define RULE_TOP_Y 35
#define RULE_BOTTOM_Y 203
#define BUTTON_ROW_TOP 211
#define BUTTON_ROW_LEFT 8
#define BUTTON_ROW_RIGHT 10
#define BUTTON_ROW_GUTTER 8
#define MIN_VOLUME 0
#define MAX_VOLUME 20
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10
#define SETTINGS_WIDTH 146
#define SETTINGS_HEIGHT 26
#define SETTINGS_ICON_OX 6
#define SETTINGS_ICON_OY 4
#define SETTINGS_BAR_OX 30
#define SETTINGS_BAR_OY 11
#define SETTINGS_BAR_WIDTH 108
#define LIST_SETTINGS_X 171
#define LIST_SETTINGS_Y 4
#define MENU_SETTINGS_X 87
#define MENU_SETTINGS_Y 37
#define TEXTBOX_LINE_HEIGHT 24 // FONT_L
#define MENU_BATTERY_X 303
#define MENU_BATTERY_Y 5
#define MENU_BAR_HEIGHT 30
#define MENU_RULE_TOP_Y 27
#define MENU_RULE_BOTTOM_Y 211
#define PREVIEW_X 148
#define PREVIEW_Y 71
#define PREVIEW_WIDTH 166
#define PREVIEW_HEIGHT 135
#define PREVIEW_INSET 3
#define MENU_TITLE_X 24
#define MENU_TITLE_Y 3
#define MENU_TITLE_WIDTH 272
#define MENU_BUTTON_ROW_TOP 217
#define MENU_WINDOW_X 6
#define MENU_WINDOW_Y 71
#define MENU_WINDOW_WIDTH 140
#define MENU_WINDOW_HEIGHT 135
#define MENU_LIST_X 14
#define MENU_LIST_Y 76
#define MENU_LIST_OY 0
#define MENU_LIST_LINE_HEIGHT 25
#define MENU_LIST_ROW_HEIGHT 27
#define MENU_ARROW_OX 6 // NOTE: from right edge
#define MENU_ARROW_OY 7 
#define SLOTS_X 200
#define SLOTS_Y 197
#define SLOTS_OX 8
#elif defined PLATFORM_RETROFW
#define PLATFORM_NAME "retrofw"
#define PLATFORM_ROOT "/media/mmcblk1p1"
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_S 12
#define FONT_M 14
#define FONT_L 16
#define FONT_S_OY 4 // unused but potentially useful
#define FONT_M_OY 5 // unused but potentially useful
#define FONT_L_OY 5 // unused but potentially useful
#define LIST_ROWS 5
#define LIST_Y 38
#define LIST_OX 16
#define LIST_OY 4
#define LIST_ROW_HEIGHT 32
#define LIST_ALT_OX 32 // from right
#define LIST_ALT_OY 10
#define SHADOW_OX 1
#define SHADOW_OY 2
#define BUTTON_SIZE 22
#define BUTTON_TEXT_OY -1
#define A_BUTTON_TEXT_OX 0
#define B_BUTTON_TEXT_OX 1
#define X_BUTTON_TEXT_OX 1
#define Y_BUTTON_TEXT_OX 1
#define RESUME_BUTTON_TEXT_OX X_BUTTON_TEXT_OX
#define PILL_PAD 6
#define PILL_TEXT_OY 2
#define HINT_OX 3
#define HINT_SLEEP "MENU"
#define HINT_RESUME "X"
#define HINT_TEXT_OY 1
#define LOGO_X 10
#define LOGO_Y 10
#define LOCATION_Y 8
#define BATTERY_X 300
#define BATTERY_Y 9
#define RULE_TOP_Y 35
#define RULE_BOTTOM_Y 203
#define BUTTON_ROW_TOP 211
#define BUTTON_ROW_LEFT 8
#define BUTTON_ROW_RIGHT 10
#define BUTTON_ROW_GUTTER 8
#define MIN_VOLUME 0
#define MAX_VOLUME 20
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10
#define SETTINGS_WIDTH 146
#define SETTINGS_HEIGHT 26
#define SETTINGS_ICON_OX 6
#define SETTINGS_ICON_OY 4
#define SETTINGS_BAR_OX 30
#define SETTINGS_BAR_OY 11
#define SETTINGS_BAR_WIDTH 108
#define LIST_SETTINGS_X 171
#define LIST_SETTINGS_Y 4
#define MENU_SETTINGS_X 87
#define MENU_SETTINGS_Y 37
#define TEXTBOX_LINE_HEIGHT 24 // FONT_L
#define MENU_BATTERY_X 303
#define MENU_BATTERY_Y 5
#define MENU_BAR_HEIGHT 30
#define MENU_RULE_TOP_Y 27
#define MENU_RULE_BOTTOM_Y 211
#define PREVIEW_X 148
#define PREVIEW_Y 71
#define PREVIEW_WIDTH 166
#define PREVIEW_HEIGHT 135
#define PREVIEW_INSET 3
#define MENU_TITLE_X 24
#define MENU_TITLE_Y 3
#define MENU_TITLE_WIDTH 272
#define MENU_BUTTON_ROW_TOP 217
#define MENU_WINDOW_X 6
#define MENU_WINDOW_Y 71
#define MENU_WINDOW_WIDTH 140
#define MENU_WINDOW_HEIGHT 135
#define MENU_LIST_X 14
#define MENU_LIST_Y 76
#define MENU_LIST_OY 0
#define MENU_LIST_LINE_HEIGHT 25
#define MENU_LIST_ROW_HEIGHT 27
#define MENU_ARROW_OX 6 // NOTE: from right edge
#define MENU_ARROW_OY 7 
#define SLOTS_X 200
#define SLOTS_Y 197
#define SLOTS_OX 8
#elif defined PLATFORM_RS90
#define PLATFORM_NAME "rs90"
#define PLATFORM_ROOT "/media/mmcblk0p1"
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define LIST_ROWS 4
#define LIST_Y 29
#define LIST_OX 10
#define LIST_OY 2
#define LIST_ROW_HEIGHT 24
#define LIST_ALT_OX 0
#define LIST_ALT_OY 0
#define SHADOW_OX 1
#define SHADOW_OY 1
#define FONT_S 8
#define FONT_M 10
#define FONT_L 12
#define FONT_S_OY 3 // unused but potentially useful
#define FONT_M_OY 3 // unused but potentially useful
#define FONT_L_OY 4 // unused but potentially useful
#define BUTTON_SIZE 16
#define BUTTON_TEXT_OY -2
#define A_BUTTON_TEXT_OX 0
#define B_BUTTON_TEXT_OX 0
#define RESUME_BUTTON_TEXT_OX 0
#define PILL_PAD 5
#define PILL_TEXT_OY 2
#define HINT_OX 2
#define HINT_SLEEP "SELECT"
#define HINT_RESUME "START"
#define HINT_TEXT_OY 1
#define LOGO_X 8
#define LOGO_Y 7
#define LOCATION_Y 6
#define BATTERY_X 224
#define BATTERY_Y 6
#define RULE_TOP_Y 26
#define RULE_BOTTOM_Y 132
#define BUTTON_ROW_TOP 139
#define BUTTON_ROW_LEFT 6
#define BUTTON_ROW_RIGHT 8
#define BUTTON_ROW_GUTTER 6
#define MIN_VOLUME 0
#define MAX_VOLUME 15
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 9
#define SETTINGS_WIDTH 96
#define SETTINGS_HEIGHT 14
#define SETTINGS_ICON_OX 1
#define SETTINGS_ICON_OY 1
#define SETTINGS_BAR_OX 16
#define SETTINGS_BAR_OY 5
#define SETTINGS_BAR_WIDTH 76
#define LIST_SETTINGS_X 139
#define LIST_SETTINGS_Y 6
#define MENU_SETTINGS_X 72
#define MENU_SETTINGS_Y 25
#define TEXTBOX_LINE_HEIGHT 18 // FONT_L
#define MENU_BATTERY_X 228
#define MENU_BATTERY_Y 3
#define MENU_BAR_HEIGHT 23
#define MENU_RULE_TOP_Y 20
#define MENU_RULE_BOTTOM_Y 137
#define PREVIEW_X 112
#define PREVIEW_Y 41
#define PREVIEW_WIDTH 124
#define PREVIEW_HEIGHT 94
#define PREVIEW_INSET 2
#define MENU_TITLE_X 16
#define MENU_TITLE_Y 3
#define MENU_TITLE_WIDTH 208
#define MENU_BUTTON_ROW_TOP 142
#define MENU_WINDOW_X 4
#define MENU_WINDOW_Y 41
#define MENU_WINDOW_WIDTH 106
#define MENU_WINDOW_HEIGHT 94
#define MENU_LIST_X 10
#define MENU_LIST_Y 43
#define MENU_LIST_OY -1
#define MENU_LIST_LINE_HEIGHT 18
#define MENU_LIST_ROW_HEIGHT MENU_LIST_LINE_HEIGHT
#define MENU_ARROW_OX 5 // NOTE: from right edge
#define MENU_ARROW_OY 5 
#define SLOTS_X 144
#define SLOTS_Y 127
#define SLOTS_OX 8
#endif

// TODO: need to differentiate between DirPath and (File)Path?
#define kRootPath PLATFORM_ROOT
#define kRecentlyPlayedPath kRootPath "/Recently Played" // not a physical file/directory
#define kSystemPath kRootPath "/.system/" PLATFORM_NAME
#define kUserdataPath kRootPath "/.userdata/" PLATFORM_NAME
#define kSharedSystemPath kRootPath "/.system/shared"
#define kSharedUserdataPath kRootPath "/.userdata/shared"
#define kRomPath kRootPath "/Roms"
#define kPakPath kSystemPath "/paks"
#define kImagePath kSystemPath "/images"

#define kFontPath kSharedSystemPath "/fonts/BPreplayBold-unhinted.otf"
#define kCJKFontPath kSharedSystemPath "/fonts/rounded-mplus-1c-heavy.ttf"

// TODO: these don't belong in common...but they're tmp so maybe it doesn't matter?
#define kLastPath "/tmp/last.txt" // transient

// TODO: do we want this to be persisted across launches? I think it's used to update recents.txt and won't be available if powering off in emu
#define kChangeDiscPath "/tmp/change_disc.txt"
#define kResumeSlotPath "/tmp/mmenu_slot.txt"

// must be unique except for absent buttons
#if defined PLATFORM_TRIMUI
#define MINUI_UP 		SDLK_UP
#define MINUI_DOWN 		SDLK_DOWN
#define MINUI_LEFT 		SDLK_LEFT
#define MINUI_RIGHT 	SDLK_RIGHT
#define MINUI_A 		SDLK_SPACE
#define MINUI_B 		SDLK_LCTRL
#define MINUI_X 		SDLK_LSHIFT
#define MINUI_Y 		SDLK_LALT
#define MINUI_START 	SDLK_RETURN
#define MINUI_SELECT 	SDLK_RCTRL
#define MINUI_L 		SDLK_TAB
#define MINUI_R 		SDLK_BACKSPACE
#define MINUI_MENU	 	SDLK_ESCAPE
#define MINUI_L2 		0
#define MINUI_R2 		0
#define MINUI_L3 		0
#define MINUI_R3 		0
#define MINUI_POWER 	0
#elif defined (PLATFORM_RG350) || (PLATFORM_ODBETA)
#define MINUI_UP 		SDLK_UP
#define MINUI_DOWN 		SDLK_DOWN
#define MINUI_LEFT 		SDLK_LEFT
#define MINUI_RIGHT 	SDLK_RIGHT
#define MINUI_A 		SDLK_LCTRL
#define MINUI_B 		SDLK_LALT
#define MINUI_X 		SDLK_SPACE
#define MINUI_Y 		SDLK_LSHIFT
#define MINUI_START 	SDLK_RETURN
#define MINUI_SELECT 	SDLK_ESCAPE
#define MINUI_L 		SDLK_TAB
#define MINUI_R 		SDLK_BACKSPACE
#define MINUI_MENU		0
#define MINUI_L2 		SDLK_PAGEUP
#define MINUI_R2 		SDLK_PAGEDOWN
#define MINUI_L3 		SDLK_KP_DIVIDE
#define MINUI_R3 		SDLK_KP_PERIOD
#define MINUI_POWER 	SDLK_HOME
#elif defined PLATFORM_RETROFW
#define MINUI_UP		SDLK_UP			
#define MINUI_DOWN		SDLK_DOWN		
#define MINUI_LEFT		SDLK_LEFT		
#define MINUI_RIGHT		SDLK_RIGHT		
#define MINUI_A			SDLK_LCTRL		
#define MINUI_B			SDLK_LALT		
#define MINUI_X			SDLK_SPACE		
#define MINUI_Y			SDLK_LSHIFT		
#define MINUI_START		SDLK_RETURN		
#define MINUI_SELECT	SDLK_ESCAPE		
#define MINUI_L			SDLK_TAB		
#define MINUI_R			SDLK_BACKSPACE	
#define MINUI_MENU		SDLK_LEFTBRACKET // brightness?
#define MINUI_L2 		0
#define MINUI_R2 		0
#define MINUI_L3 		0
#define MINUI_R3 		0
#define MINUI_POWER		SDLK_END		// sleep?
#elif defined PLATFORM_RS90
#define MINUI_UP 		SDLK_UP
#define MINUI_DOWN 		SDLK_DOWN
#define MINUI_LEFT 		SDLK_LEFT
#define MINUI_RIGHT 	SDLK_RIGHT
#define MINUI_A 		SDLK_LCTRL
#define MINUI_B 		SDLK_LALT
#define MINUI_X 		0
#define MINUI_Y 		0
#define MINUI_START 	SDLK_RETURN
#define MINUI_SELECT 	SDLK_ESCAPE
#define MINUI_L 		SDLK_TAB
#define MINUI_R 		SDLK_BACKSPACE
#define MINUI_MENU 		0
#define MINUI_L2 		0
#define MINUI_R2 		0
#define MINUI_L3 		0
#define MINUI_R3 		0
#define MINUI_POWER 	0
#endif

#define GOLD_TRIAD 0xd2,0xb4,0x6c
#define BRONZE_TRIAD 0x9f,0x89,0x52
#define WHITE_TRIAD 0xff,0xff,0xff
#define GRAY_TRIAD 0x66,0x66,0x66
#define SHADOW25_TRIAD 0xa1,0x87,0x47
#define SHADOW50_TRIAD 0x68,0x5a,0x35

typedef enum ButtonIndex {
	kButtonUp = 0,
	kButtonDown,
	kButtonLeft,
	kButtonRight,
	kButtonA,
	kButtonB,
	kButtonX,
	kButtonY,
	kButtonStart,
	kButtonSelect,
	kButtonL,
	kButtonR,
	kButtonMenu,
	kButtonL2,
	kButtonR2,
	kButtonL3,
	kButtonR3,
	kButtonPower,
	kButtonCount,
} ButtonIndex;

// map actions to platform specific buttons here
// TODO: considering moving resume to START on all platforms?
// but then we need to extend the can_start and can_select behavior too...
#if defined PLATFORM_TRIMUI
#define kButtonSleep kButtonMenu
#define kButtonSleepAlt kButtonMenu
#define kButtonResume kButtonX
#define kButtonAltEmu kButtonY
#elif defined (PLATFORM_RG350) || (PLATFORM_ODBETA)
#define kButtonSleep kButtonPower
#define kButtonSleepAlt kButtonL3
#define kButtonResume kButtonX
#define kButtonAltEmu kButtonY
#elif defined PLATFORM_RETROFW
#define kButtonSleep kButtonMenu
#define kButtonSleepAlt kButtonPower
#define kButtonResume kButtonX
#define kButtonAltEmu kButtonY
#elif defined PLATFORM_RS90
// TODO: this is no longer true, kButtonSleep is actually kButtonSelect+kButtonStart
#define kButtonSleep kButtonSelect
#define kButtonSleepAlt kButtonSelect
#define kButtonResume kButtonStart
#define kButtonAltEmu kButtonSelect
#endif

void Input_reset(void);
void Input_poll(void);
int Input_anyPressed(void);
int Input_justPressed(ButtonIndex btn);
int Input_justRepeated(ButtonIndex btn);
int Input_isPressed(ButtonIndex btn);
int Input_justReleased(ButtonIndex btn);

int prefixMatch(char* pre, char* str);
int suffixMatch(char* suf, char* str);
int exactMatch(char* str1, char* str2);
int hide(char* file_name);

void getDisplayName(const char* in_name, char* out_name);
void getEmuName(const char* in_name, char* out_name);

void normalizeNewline(char* line);
void trimTrailingNewlines(char* line);

int exists(char* path);
void putFile(char* path, char* contents);
void getFile(char* path, char* buffer);
void putInt(char* path, int value);
int getInt(char* path);

void GFX_init(char* font_path);
void GFX_ready(void);
void GFX_quit(void);
void GFX_clear(void);
SDL_Surface* GFX_loadImage(char* path); // only meant for system images, not arbitrary images like save state previews!
void GFX_blitRule(SDL_Surface* surface, int y);
int GFX_blitHint(SDL_Surface* surface, char* htxt, int x, int y);
int GFX_blitPill(SDL_Surface* surface, char* btxt, char* htxt, int x, int y);
int GFX_blitButton(SDL_Surface* surface, char* btxt, char* htxt, int x, int y, int bx);
void GFX_blitMenu(SDL_Surface* surface, char* name, char* path, int conflict, int row, int selected_row, int has_alt, int use_alt);
void GFX_blitWindow(SDL_Surface* surface, int x, int y, int width, int height, int color); // color=0:black,1:white
void GFX_blitBodyCopy(SDL_Surface* surface, char* text, int x, int y, int width, int height); // gold, centered in rect, multiline okay!
int GFX_blitText(SDL_Surface* surface, char* text, int size, int x, int y, int width, int color, int has_shadow); // size=0:small,1:medium,2:large width=0:left,>0:center,<0:right, color=0:white,1:gold, no multiline

SDL_Surface* GFX_getText(char* text, int size, int color);
void GFX_blitBattery(SDL_Surface* surface, int x, int y);
void GFX_blitSettings(SDL_Surface* surface, int x, int y, int icon, int value, int min_value, int max_value); // icon=0:brightness,1:volume,2:mute

void initPlatform(void);
void waitForWake(void);
void fauxSleep(void);
int getSmoothBatteryLevel(void);

#endif