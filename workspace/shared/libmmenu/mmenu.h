#ifndef menu_h__
#define menu_h__

typedef enum MenuReturnStatus {
	kStatusContinue = 0,
	kStatusSaveSlot = 1,
	kStatusLoadSlot = 11,
	kStatusOpenMenu = 23,
	kStatusChangeDisc = 24,
	kStatusExitGame = 31,
} MenuReturnStatus;

typedef struct SDL_Surface SDL_Surface;
typedef MenuReturnStatus (*ShowMenu_t)(char* rom_path, char* save_path_template, SDL_Surface* optional_snapshot);
MenuReturnStatus ShowMenu(char* rom_path, char* save_path_template, SDL_Surface* optional_snapshot);

typedef int (*ResumeSlot_t)(void);
int ResumeSlot(void);

typedef int (*ChangeDisc_t)(char* disc_path);
int ChangeDisc(char* disc_path);

#endif  // menu_h__
