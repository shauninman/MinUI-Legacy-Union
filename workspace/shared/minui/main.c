#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <msettings.h>
#include <mlanguage.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "../common/common.h"
#include "../common/platform.h"

///////////////////////////////////////

#define dump(msg) puts((msg));fflush(stdout);

///////////////////////////////////////

typedef struct Array {
	int count;
	int capacity;
	void** items;
} Array;

static Array* Array_new(void) {
	Array* self = malloc(sizeof(Array));
	self->count = 0;
	self->capacity = 8;
	self->items = malloc(sizeof(void*) * self->capacity);
	return self;
}
static void Array_push(Array* self, void* item) {
	if (self->count>=self->capacity) {
		self->capacity *= 2;
		self->items = realloc(self->items, sizeof(void*) * self->capacity);
	}
	self->items[self->count++] = item;
}
static void Array_unshift(Array* self, void* item) {
	if (self->count==0) return Array_push(self, item);
	Array_push(self, NULL); // ensures we have enough capacity
	for (int i=self->count-2; i>=0; i--) {
		self->items[i+1] = self->items[i];
	}
	self->items[0] = item;
}
static void* Array_pop(Array* self) {
	if (self->count==0) return NULL;
	return self->items[--self->count];
}
static void Array_reverse(Array* self) {
	int end = self->count-1;
	int mid = self->count/2;
	for (int i=0; i<mid; i++) {
		void* item = self->items[i];
		self->items[i] = self->items[end-i];
		self->items[end-i] = item;
	}
}
static void Array_free(Array* self) {
	free(self->items); 
	free(self);
}

static int StringArray_indexOf(Array* self, char* str) {
	for (int i=0; i<self->count; i++) {
		if (exactMatch(self->items[i], str)) return i;
	}
	return -1;
}
static void StringArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

typedef struct Hash {
	Array* keys;
	Array* values;
} Hash; // not really a hash

static Hash* Hash_new(void) {
	Hash* self = malloc(sizeof(Hash));
	self->keys = Array_new();
	self->values = Array_new();
	return self;
}
static void Hash_free(Hash* self) {
	StringArray_free(self->keys);
	StringArray_free(self->values);
	free(self);
}
static void Hash_set(Hash* self, char* key, char* value) {
	Array_push(self->keys, strdup(key));
	Array_push(self->values, strdup(value));
}
static char* Hash_get(Hash* self, char* key) {
	int i = StringArray_indexOf(self->keys, key);
	if (i==-1) return NULL;
	return self->values->items[i];
}

///////////////////////////////////////

enum EntryType {
	kEntryDir,
	kEntryPak,
	kEntryRom,
};
typedef struct Entry {
	char* path;
	char* name;
	int type;
	int alpha; // index in parent Directory's alphas Array, which points to the index of an Entry in its entries Array :sweat_smile:
	int conflict;
	
	int has_alt;
	int use_alt;
} Entry;

static Entry* Entry_new(char* path, int type) {
	char display_name[256];
	getDisplayName(path, display_name);
	Entry* self = malloc(sizeof(Entry));
	self->path = strdup(path);
	self->name = strdup(display_name);
	self->type = type;
	self->alpha = 0;
	self->conflict = 0;
	self->has_alt = type!=kEntryRom?0:-1;
	self->use_alt = type!=kEntryRom?0:-1;
	return self;
}
static void Entry_free(Entry* self) {
	free(self->path);
	free(self->name);
	free(self);
}

static int EntryArray_indexOf(Array* self, char* path) {
	for (int i=0; i<self->count; i++) {
		Entry* entry = self->items[i];
		if (exactMatch(entry->path, path)) return i;
	}
	return -1;
}
static int EntryArray_sortEntry(const void* a, const void* b) {
	Entry* item1 = *(Entry**)a;
	Entry* item2 = *(Entry**)b;
	return strcasecmp(item1->name, item2->name);
}
static void EntryArray_sort(Array* self) {
	qsort(self->items, self->count, sizeof(void*), EntryArray_sortEntry);
}

static void EntryArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Entry_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

#define kIntArrayMax 27
typedef struct IntArray {
	int count;
	int items[kIntArrayMax];
} IntArray;
static IntArray* IntArray_new(void) {
	IntArray* self = malloc(sizeof(IntArray));
	self->count = 0;
	memset(self->items, 0, sizeof(int) * kIntArrayMax);
	return self;
}
static void IntArray_push(IntArray* self, int i) {
	self->items[self->count++] = i;
}
static void IntArray_free(IntArray* self) {
	free(self);
}

///////////////////////////////////////

typedef struct Directory {
	char* path;
	char* name;
	Array* entries;
	IntArray* alphas;
	// rendering
	int selected;
	int start;
	int end;
} Directory;

static int getIndexChar(char* str) {
	char i = 0;
	char c = tolower(str[0]);
	if (c>='a' && c<='z') i = (c-'a')+1;
	return i;
}

static void Directory_index(Directory* self) {
	Entry* prior = NULL;
	int alpha = -1;
	int index = 0;
	for (int i=0; i<self->entries->count; i++) {
		Entry* entry = self->entries->items[i];
		if (prior!=NULL && exactMatch(prior->name, entry->name)) {
			prior->conflict = 1;
			entry->conflict = 1;
		}
		int a = getIndexChar(entry->name);
		if (a!=alpha) {
			index = self->alphas->count;
			IntArray_push(self->alphas, i);
			alpha = a;
		}
		entry->alpha = index;
		
		prior = entry;
	}
}

static Array* getRoot(void);
static Array* getRecents(void);
static Array* getDiscs(char* path);
static Array* getEntries(char* path);

static Directory* Directory_new(char* path, int selected) {
	char display_name[256];
	getDisplayName(path, display_name);
	
	Directory* self = malloc(sizeof(Directory));
	self->path = strdup(path);
	self->name = strdup(display_name);
	if (exactMatch(path, Paths.rootDir)) {
		self->entries = getRoot();
	}
	else if (exactMatch(path, Paths.fauxRecentDir)) {
		self->entries = getRecents();
	}
	else if (suffixMatch(".m3u", path)) {
		self->entries = getDiscs(path);
	}
	else {
		self->entries = getEntries(path);
	}
	self->alphas = IntArray_new();
	self->selected = selected;
	Directory_index(self);
	return self;
}
static void Directory_free(Directory* self) {
	free(self->path);
	free(self->name);
	EntryArray_free(self->entries);
	IntArray_free(self->alphas);
	free(self);
}

static void DirectoryArray_pop(Array* self) {
	Directory_free(Array_pop(self));
}
static void DirectoryArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Directory_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

typedef struct Recent {
	char* path; // NOTE: this is without the Paths.rootDir prefix!
	int available;
} Recent;
static int hasPak(char* pak_name);
static int hasEmu(char* emu_name);
static Recent* Recent_new(char* path) {
	Recent* self = malloc(sizeof(Recent));

	char sd_path[256]; // only need to get emu name
	sprintf(sd_path, "%s%s", Paths.rootDir, path);

	char emu_name[256];
	getEmuName(sd_path, emu_name);
	
	self->path = strdup(path);
	self->available = hasEmu(emu_name);
	return self;
}
static void Recent_free(Recent* self) {
	free(self->path);
	free(self);
}

static int RecentArray_indexOf(Array* self, char* str) {
	for (int i=0; i<self->count; i++) {
		Recent* item = self->items[i];
		if (exactMatch(item->path, str)) return i;
	}
	return -1;
}
static void RecentArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Recent_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

static Language lang;
static Directory* top;
static Array* stack; // DirectoryArray
static Array* recents; // RecentArray

static int quit = 0;
static int can_resume = 0;
static int should_resume = 0; // set to 1 on kButtonResume but only if can_resume==1
static char slot_path[256];

// NOTE: I have no idea what to call these...
static int restore_relative = -1; // root Directory
static int restore_selected = 0; // Recently Played entry
static int restore_start = 0;
static int restore_end = 0;

///////////////////////////////////////

#define kMaxRecents 60 // a multiple of all menu rows
static void saveRecents(void) {
	FILE* file = fopen(Paths.sharedRecent, "w");
	if (file) {
		for (int i=0; i<recents->count; i++) {
			Recent* recent = recents->items[i];
			fputs(recent->path, file);
			putc('\n', file);
		}
		fclose(file);
	}
}
static void addRecent(char* path) {
	path += strlen(Paths.rootDir); // makes paths platform agnostic
	int id = RecentArray_indexOf(recents, path);
	if (id==-1) { // add
		while (recents->count>=kMaxRecents) {
			Recent_free(Array_pop(recents));
		}
		Array_unshift(recents, Recent_new(path));
	}
	else if (id>0) { // bump to top
		for (int i=id; i>0; i--) {
			void* tmp = recents->items[i-1];
			recents->items[i-1] = recents->items[i];
			recents->items[i] = tmp;
		}
	}
	saveRecents();
}

static int hasPak(char* pak_name) {
	char pak_path[256];
	sprintf(pak_path, "%s/%s.pak/launch.sh", Paths.paksDir, pak_name);
	return exists(pak_path);
}
static int hasEmu(char* emu_name) {
	char pak_path[256];
	sprintf(pak_path, "%s/Emus/%s.pak/launch.sh", Paths.paksDir, emu_name);
	return exists(pak_path);
}
static int hasAlt(char* emu_name) {
	char pak_path[256];
	sprintf(pak_path, "%s/Emus/%s.pak/has-alt", Paths.paksDir, emu_name);
	return exists(pak_path);
}

static int Entry_hasAlt(Entry* self) {
	// has_alt can be set by getEntries()
	// but won't be set by getRecents()
	// otherwise delayed until selected
	if (self->has_alt==-1) {
		// check
		char emu_name[256];
		getEmuName(self->path, emu_name);
		self->has_alt = hasAlt(emu_name);
	}
	return self->has_alt;
}
static int Entry_useAlt(Entry* self) {
	// has to be checked on an individual basis
	// but delayed until selected
	
	if (self->use_alt==-1) {
		// check
		char emu_name[256];
		getEmuName(self->path, emu_name);
		
		char rom_name[256];
		char* tmp = strrchr(self->path, '/');
		if (tmp) strcpy(rom_name, tmp+1);
		
		char use_alt[256];
		sprintf(use_alt, "%s/.mmenu/%s/%s.use-alt", Paths.userdataDir, emu_name, rom_name);
		
		self->use_alt = exists(use_alt);
	}
	return self->use_alt;
}
static int Entry_toggleAlt(Entry* self) {
	if (!Entry_hasAlt(self)) return 0;

	self->use_alt = !Entry_useAlt(self);
	
	char emu_name[256];
	getEmuName(self->path, emu_name);
	
	char rom_name[256];
	char* tmp = strrchr(self->path, '/');
	if (tmp) strcpy(rom_name, tmp+1);
	
	char use_alt_path[256];
	sprintf(use_alt_path, "%s/.mmenu/%s/%s.use-alt", Paths.userdataDir, emu_name, rom_name);
	
	if (self->use_alt) close(open(use_alt_path, O_RDWR|O_CREAT, 0777)); // basically touch
	else unlink(use_alt_path);
		
	return 1;
}

static int hasRecents(void) {
	int has = 0;
	
	Array* parent_paths = Array_new();
	if (exists(kChangeDiscPath)) {
		char disc_path[256];
		getFile(kChangeDiscPath, disc_path);
		if (exists(disc_path)) {
			Recent* recent = Recent_new(disc_path);
			if (recent->available) has += 1;
			Array_push(recents, recent);
		
			char parent_path[256];
			strcpy(parent_path, disc_path);
			char* tmp = strrchr(parent_path, '/') + 1;
			tmp[0] = '\0';
			Array_push(parent_paths, strdup(parent_path));
		}
		unlink(kChangeDiscPath);
	}
	
	FILE* file = fopen(Paths.sharedRecent, "r"); // newest at top
	if (file) {
		char line[256];
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			char sd_path[256];
			sprintf(sd_path, "%s%s", Paths.rootDir, line);
			if (exists(sd_path)) {
				if (recents->count<kMaxRecents) {
					if (suffixMatch(".cue", line)) {
						char parent_path[256];
						strcpy(parent_path, line);
						char* tmp = strrchr(parent_path, '/') + 1;
						tmp[0] = '\0';
						
						int found = 0;
						for (int i=0; i<parent_paths->count; i++) {
							char* path = parent_paths->items[i];
							if (prefixMatch(path, parent_path)) {
								found = 1;
								break;
							}
						}
						if (found) continue;
						
						Array_push(parent_paths, strdup(parent_path));
					}
					Recent* recent = Recent_new(line);
					if (recent->available) has += 1;
					Array_push(recents, recent);
				}
			}
		}
		fclose(file);
	}
	
	saveRecents();
	
	StringArray_free(parent_paths);
	return has>0;
}
static int hasRoms(char* dir_name) {
	int has = 0;
	char emu_name[256];
	char rom_path[256];

	getEmuName(dir_name, emu_name);
	
	// check for emu pak
	if (!hasEmu(emu_name)) return has;
	
	// check for at least one non-hidden file (we're going to assume it's a rom)
	sprintf(rom_path, "%s/%s/", Paths.romsDir, dir_name);
	DIR *dh = opendir(rom_path);
	if (dh!=NULL) {
		struct dirent *dp;
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			has = 1;
			break;
		}
		closedir(dh);
	}
	// if (!has) printf("No roms for %s!\n", dir_name);
	return has;
}
static Array* getRoot(void) {
	// TODO: finish!
	
	Array* entries = Array_new();
	
	int has_recents = hasRecents();
	
	if (has_recents) Array_push(entries, Entry_new(Paths.fauxRecentDir, kEntryDir));
	
	DIR *dh = opendir(Paths.romsDir);
	if (dh!=NULL) {
		struct dirent *dp;
		char* tmp;
		char full_path[256];
		sprintf(full_path, "%s/", Paths.romsDir);
		tmp = full_path + strlen(full_path);
		Array* emus = Array_new();
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			if (hasRoms(dp->d_name)) {
				strcpy(tmp, dp->d_name);
				Array_push(emus, Entry_new(full_path, kEntryDir));
			}
		}
		EntryArray_sort(emus);
		for (int i=0; i<emus->count; i++) {
			Entry* emu = emus->items[i];
			Array_push(entries, emus->items[i]);
		}
		Array_free(emus); // just free the array part, root now owns emus entries
		closedir(dh);
	}
	
	char tools_path[256];
	sprintf(tools_path, "%s/Tools", Paths.paksDir);
	Array_push(entries, Entry_new(tools_path, kEntryDir));
	
	return entries;
}
static Array* getRecents(void) {
	Array* entries = Array_new();
	for (int i=0; i<recents->count; i++) {
		Recent* recent = recents->items[i];
		if (!recent->available) continue;
		
		char sd_path[256];
		sprintf(sd_path, "%s%s", Paths.rootDir, recent->path);
		int type = suffixMatch(".pak", sd_path) ? kEntryPak : kEntryRom;
		Array_push(entries, Entry_new(sd_path, type));
	}
	return entries;
}
static Array* getDiscs(char* path){
	
	// TODO: does path have Paths.rootDir prefix?
	
	Array* entries = Array_new();
	
	char base_path[256];
	strcpy(base_path, path);
	char* tmp = strrchr(base_path, '/') + 1;
	tmp[0] = '\0';
	
	// TODO: limit number of discs supported (to 9?)
	FILE* file = fopen(path, "r");
	if (file) {
		char line[256];
		int disc = 0;
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			char disc_path[256];
			sprintf(disc_path, "%s%s", base_path, line);
						
			if (exists(disc_path)) {
				disc += 1;
				Entry* entry = Entry_new(disc_path, kEntryRom);
				free(entry->name);
				char name[16];
				sprintf(name, lang.disc_num, disc);
				entry->name = strdup(name);
				Array_push(entries, entry);
			}
		}
		fclose(file);
	}
	return entries;
}
static Array* getEntries(char* path){
	Array* entries = Array_new();
	DIR *dh = opendir(path);
	if (dh!=NULL) {
		struct dirent *dp;
		char* tmp;
		char full_path[256];
		sprintf(full_path, "%s/", path);
		tmp = full_path + strlen(full_path);
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			strcpy(tmp, dp->d_name);
			int is_dir = dp->d_type==DT_DIR;
			int type;
			if (is_dir) {
				// TODO: this should make sure launch.sh exists
				if (suffixMatch(".pak", dp->d_name)) {
					type = kEntryPak;
				}
				else {
					type = kEntryDir;
				}
			}
			else {
				type = kEntryRom;
			}
			Array_push(entries, Entry_new(full_path, type));
		}
		closedir(dh);
	}
	EntryArray_sort(entries);
	return entries;
}

///////////////////////////////////////

static void queueNext(char* cmd) {
	char next_path[256];
	sprintf(next_path, "%s/next.sh", Paths.userdataDir);
	putFile(next_path, cmd);
	quit = 1;
}

static int hasCue(char* dir_path, char* cue_path) {
	char* tmp = strrchr(dir_path, '/') + 1; // folder name
	sprintf(cue_path, "%s/%s.cue", dir_path, tmp);
	return exists(cue_path);
}

///////////////////////////////////////

static void readyResumePath(char* rom_path, int type) {
	can_resume = 0;
	char path[256];
	strcpy(path, rom_path);
	
	if (!prefixMatch(Paths.romsDir, path)) return;
	
	char auto_path[256];
	if (type==kEntryDir) {
		if (!hasCue(path, auto_path)) return;
		strcpy(path, auto_path);
	}
	
	char emu_name[256];
	getEmuName(path, emu_name);
	
	char* tmp;
	char rom_file[256];
	tmp = strrchr(path, '/') + 1;
	strcpy(rom_file, tmp);
	
	char mmenu_dir[256];
	sprintf(mmenu_dir, "%s/.mmenu/%s", Paths.userdataDir, emu_name); // /.userdata/<platform>/.mmenu/<EMU>
	sprintf(slot_path, "%s/%s.txt", mmenu_dir, rom_file); // /.userdata/<platform>/.mmenu/<EMU>/<romname>.ext.txt
	
	can_resume = exists(slot_path);
}
// NOTE: this has been disabled, didn't feel right
static void readyResumeRecent(void) {
	for (int i=0; i<recents->count; i++) {
		Recent* recent = recents->items[i];
		if (!recent->available) continue;
		
		char sd_path[256];
		sprintf(sd_path, "%s%s", Paths.rootDir, recent->path);
		int type = suffixMatch(".pak", sd_path) ? kEntryPak : kEntryRom;
		readyResumePath(sd_path, type);
		break;
	}
} 
static void readyResume(Entry* entry) {
	// if (exactMatch(Paths.fauxRecentDir, entry->path)) return readyResumeRecent(); // special case
	readyResumePath(entry->path, entry->type);
}

static void saveLast(char* path);
static void loadLast(void);

static void openPak(char* path) {
	char cmd[256];
	sprintf(cmd, "\"%s/launch.sh\"", path);
	
	// TODO: revisit this path (will games even be supported? how?)
	// if (prefixMatch(Paths.rootDir "/Games", path)) {
	// 	addRecent(path);
	// }
	saveLast(path);
	queueNext(cmd);
}
static void openRom(char* path, char* last) {
	char emu_name[256];
	getEmuName(path, emu_name);
	
	char cmd[256];
	sprintf(cmd, "\"%s/Emus/%s.pak/launch.sh\" \"%s\"", Paths.paksDir, emu_name, path);

	if (should_resume) {
		char slot[16];
		getFile(slot_path, slot);
		putFile(kResumeSlotPath, slot);
		should_resume = 0;
	}
	
	addRecent(path);
	saveLast(last==NULL ? path : last);
	queueNext(cmd);
}
static int openRecent(void) {
	// gotta look it up first (could we store this?)
	for (int i=0; i<recents->count; i++) {
		Recent* recent = recents->items[i];
		if (!recent->available) continue;
		
		char sd_path[256];
		sprintf(sd_path, "%s%s", Paths.rootDir, recent->path);
		
		// if (suffixMatch(".pak", path)) {
		// 	openPak(sd_path);
		// }
		// else {
			openRom(sd_path, Paths.rootDir);
		// }
		return 1;
	}
	return 0;
}
static void openDirectory(char* path, int auto_launch) {
	if (should_resume && exactMatch(Paths.fauxRecentDir, path)) {
		if (openRecent()) return;
	}
	
	char auto_path[256];
	if (hasCue(path, auto_path) && auto_launch) {
		openRom(auto_path, path);
		return;
	}

	char* tmp = strrchr(auto_path, '.') + 1; // extension
	strcpy(tmp, "m3u"); // replace with m3u
	if (exists(auto_path)) {
		path = auto_path;
	}
	
	int selected = 0;
	int start = selected;
	int end = 0;
	if (top) {
		if (top->selected==restore_relative) {
			selected = restore_selected;
			start = restore_start;
			end = restore_end;
		}
	}
	
	top = Directory_new(path, selected);
	top->start = start;
	top->end = end ? end : ((top->entries->count<Screen.main.list.row_count) ? top->entries->count : Screen.main.list.row_count);

	Array_push(stack, top);
}
static void closeDirectory(void) {
	restore_selected = top->selected;
	restore_start = top->start;
	restore_end = top->end;
	DirectoryArray_pop(stack);
	top = stack->items[stack->count-1];
	restore_relative = top->selected;
}

static void Entry_open(Entry* self) {
	if (self->type==kEntryRom) {
		openRom(self->path, NULL);
	}
	else if (self->type==kEntryPak) {
		openPak(self->path);
	}
	else if (self->type==kEntryDir) {
		openDirectory(self->path, 1);
	}
}

///////////////////////////////////////

static void saveLast(char* path) {
	// special case for recently played
	if (exactMatch(top->path, Paths.fauxRecentDir)) {
		// NOTE: that we don't have to save the file because
		// your most recently played game will always be at
		// the top which is also the default selection
		path = Paths.fauxRecentDir;
	}
	putFile(kLastPath, path);
}
static void loadLast(void) { // call after loading root directory
	if (!exists(kLastPath)) return;

	char last_path[256];
	getFile(kLastPath, last_path);
	
	Array* last = Array_new();
	while (!exactMatch(last_path, Paths.rootDir)) {
		Array_push(last, strdup(last_path));
		
		char* slash = strrchr(last_path, '/');
		last_path[(slash-last_path)] = '\0';
	}
	
	while (last->count>0) {
		char* path = Array_pop(last);
		for (int i=0; i<top->entries->count; i++) {
			Entry* entry = top->entries->items[i];
			if (exactMatch(entry->path, path)) {
				top->selected = i;
				if (i>=top->end) {
					top->start = i;
					top->end = top->start + Screen.main.list.row_count;
					if (top->end>top->entries->count) {
						top->end = top->entries->count;
						top->start = top->end - Screen.main.list.row_count;
					}
				}
				if (last->count==0 && !exactMatch(entry->path, Paths.fauxRecentDir)) break; // don't show contents of auto-launch dirs
				
				if (entry->type==kEntryDir) {
					openDirectory(entry->path, 0);
				}
			}
		}
		free(path); // we took ownership when we popped it
	}
	
	StringArray_free(last);
}

///////////////////////////////////////

static void Menu_init(void) {
	stack = Array_new(); // array of open Directories
	recents = Array_new();

	openDirectory(Paths.rootDir, 0);
	loadLast(); // restore state when available
}
static void Menu_quit(void) {
	RecentArray_free(recents);
	DirectoryArray_free(stack);
}

///////////////////////////////////////

int main (int argc, char *argv[]) {
	puts("MinUI");
	fflush(stdout);
	
	Union_init();
	initPlatform();
	
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	
	SDL_ShowCursor(0);
	SDL_EnableKeyRepeat(300,100);
	
	InitSettings();
	InitLanguage(&lang);
	
	SDL_Surface* screen = SDL_SetVideoMode(Screen.width, Screen.height, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	GFX_init(lang.CJK!=NULL);
	GFX_ready();
	
	// TODO: uncomment once I sort out per-platform can-sleep setting
	// // wake training
	// SDL_FillRect(screen, NULL, 0);
	// GFX_blitBodyCopy(screen, lang.wake,0,0,Screen.width,Screen.height);
	// SDL_Flip(screen);
	// waitForWake();
	
	SDL_Surface* logo = GFX_loadImage("logo.png");
#ifdef PLATFORM_RS90
	SDL_Surface* btn_select_start = GFX_loadImage("btn-select-start.png");
#endif
	
	Menu_init();
	
	Input_reset();
	int dirty = 1;
	int was_charging = isCharging();
	unsigned long charge_start = SDL_GetTicks();
	int show_setting = 0; // 1=brightness,2=volume
	int setting_value = 0;
	int setting_min = 0;
	int setting_max = 0;
	int select_is_locked = 0; // rs90-only
	int select_was_locked = 0;
	int delay_start = 0;
	int delay_select = 0;
	int allow_resume = 1;
	int allow_favorite = 1;
#ifdef PLATFORM_RS90
	unsigned long start_press_start;
	unsigned long select_press_start;
#endif
	unsigned long cancel_start = SDL_GetTicks();
	while (!quit) {
		unsigned long frame_start = SDL_GetTicks();
		int select_was_pressed = Input_isPressed(kButtonSelect); // rs90-only
		
		Input_poll();
		
#ifdef PLATFORM_RS90
		select_was_locked = select_is_locked;
		if (select_was_pressed && Input_justReleased(kButtonSelect) && Input_justPressed(kButtonL)) {
			select_is_locked = 1;
		}
		else if (select_is_locked && Input_justReleased(kButtonL)) {
			select_is_locked = 0;
		}
#endif
				
		int selected = top->selected;
		int total = top->entries->count;
		if (Input_justRepeated(kButtonUp)) {
			selected -= 1;
			if (selected<0) {
				selected = total-1;
				int start = total - Screen.main.list.row_count;
				top->start = (start<0) ? 0 : start;
				top->end = total;
			}
			else if (selected<top->start) {
				top->start -= 1;
				top->end -= 1;
			}
		}
		else if (Input_justRepeated(kButtonDown)) {
			selected += 1;
			if (selected>=total) {
				selected = 0;
				top->start = 0;
				top->end = (total<Screen.main.list.row_count) ? total : Screen.main.list.row_count;
			}
			else if (selected>=top->end) {
				top->start += 1;
				top->end += 1;
			}
		}
		if (Input_justRepeated(kButtonLeft)) {
			selected -= Screen.main.list.row_count;
			if (selected<0) {
				selected = 0;
				top->start = 0;
				top->end = (total<Screen.main.list.row_count) ? total : Screen.main.list.row_count;
			}
			else if (selected<top->start) {
				top->start -= Screen.main.list.row_count;
				if (top->start<0) top->start = 0;
				top->end = top->start + Screen.main.list.row_count;
			}
		}
		else if (Input_justRepeated(kButtonRight)) {
			selected += Screen.main.list.row_count;
			if (selected>=total) {
				selected = total-1;
				int start = total - Screen.main.list.row_count;
				top->start = (start<0) ? 0 : start;
				top->end = total;
			}
			else if (selected>=top->end) {
				top->end += Screen.main.list.row_count;
				if (top->end>total) top->end = total;
				top->start = top->end - Screen.main.list.row_count;
			}
		}
		
		// NOTE: && !Input_justReleased(kButtonSelect) is for RS90
		if (!Input_isPressed(kButtonStart) && !Input_isPressed(kButtonSelect) && !Input_justReleased(kButtonSelect)) { 
			if (Input_justRepeated(kButtonL)) { // previous alpha
				Entry* entry = top->entries->items[selected];
				int i = entry->alpha-1;
				if (i>=0) {
					selected = top->alphas->items[i];
					if (total>Screen.main.list.row_count) {
						top->start = selected;
						top->end = top->start + Screen.main.list.row_count;
						if (top->end>total) top->end = total;
						top->start = top->end - Screen.main.list.row_count;
					}
				}
			}
			else if (Input_justRepeated(kButtonR)) { // next alpha
				Entry* entry = top->entries->items[selected];
				int i = entry->alpha+1;
				if (i<top->alphas->count) {
					selected = top->alphas->items[i];
					if (total>Screen.main.list.row_count) {
						top->start = selected;
						top->end = top->start + Screen.main.list.row_count;
						if (top->end>total) top->end = total;
						top->start = top->end - Screen.main.list.row_count;
					}
				}
			}
		}
		
		if (selected!=top->selected) {
			top->selected = selected;
			dirty = 1;
		}
		
		if (dirty) readyResume(top->entries->items[top->selected]);
#ifdef PLATFORM_RS90
		if (Input_justPressed(kButtonStart)) {
			delay_start = 1;
			start_press_start = SDL_GetTicks();
		}
		else if (Input_isPressed(kButtonStart)) {
			if (SDL_GetTicks()-start_press_start>200) {
				delay_start = 0;
				allow_resume = 0;
			}
		}
		else if (Input_justReleased(kButtonStart)) {
			delay_start = 0;
		}
		
		// START is overloaded on rs90 so we need to prevent misfires
		if (Input_isPressed(kButtonStart) && (Input_justPressed(kButtonL) || Input_justPressed(kButtonR))) {
			allow_resume = 0;
		}
		
		// TODO: this needs extra love because of the weird SELECT+L relationship
		if (Input_justPressed(kButtonSelect) && !select_was_locked) {
			delay_select = 1;
			select_press_start = SDL_GetTicks();
		}
		else if (Input_isPressed(kButtonSelect) || select_is_locked) {
			if (SDL_GetTicks()-select_press_start>200) {
				delay_select = 0;
				allow_favorite = 0;
			}
		}
		else if (Input_justReleased(kButtonSelect) && !select_is_locked) {
			delay_select = 0;
		}
	
		// SELECT is overloaded on rs90 so we need to prevent misfires
		if ((Input_isPressed(kButtonSelect) || select_is_locked) && (Input_justPressed(kButtonL) || Input_justPressed(kButtonR))) {
			allow_favorite = 0;
		}
#endif		
		if (Input_justReleased(kButtonResume)) {
			if (allow_resume && can_resume) {
				should_resume = 1;
				Entry_open(top->entries->items[top->selected]);
				dirty = 1;
			}
			allow_resume = 1;
		}
		else if (Input_justPressed(kButtonAltEmu)) {
			if (Entry_toggleAlt(top->entries->items[top->selected])) dirty = 1;
		}
		else if (Input_justReleased(kButtonSelect) && !select_is_locked) { // TODO: kButtonFavorite?
			// TODO: 
			allow_favorite = 1;
		}
		else if (Input_justPressed(kButtonA)) {
			Entry_open(top->entries->items[top->selected]);
			dirty = 1;
	
			readyResume(top->entries->items[top->selected]);
		}
		else if (Input_justPressed(kButtonB) && stack->count>1) {
			closeDirectory();
			dirty = 1;
			// can_resume = 0;
			readyResume(top->entries->items[top->selected]);
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
		else if (Input_isPressed(kButtonStart) && !delay_start) {
			show_setting = 1;
			setting_value = GetBrightness();
			setting_min = MIN_BRIGHTNESS;
			setting_max = MAX_BRIGHTNESS;
		}
		else if ((Input_isPressed(kButtonSelect) || select_is_locked) && !delay_select) {
			show_setting = 2;
			setting_value = GetVolume();
			setting_min = MIN_VOLUME;
			setting_max = MAX_VOLUME;
		}
		if (old_setting!=show_setting || old_value!=setting_value) dirty = 1;
		
		if (dirty) {
			dirty = 0;
			
			SDL_FillRect(screen, NULL, 0);
			SDL_BlitSurface(logo, NULL, screen, &(SDL_Rect){Screen.main.logo.x,Screen.main.logo.y});

			if (show_setting) {
				GFX_blitSettings(screen, Screen.main.settings.x, Screen.main.settings.y, show_setting==1?0:(setting_value>0?1:2), setting_value,setting_min,setting_max);
			}
			else {
				GFX_blitBattery(screen, Screen.main.battery.x, Screen.main.battery.y);
			}
			GFX_blitRule(screen, Screen.main.rule.top_y);
			
			int selected_row = top->selected - top->start;
			for (int i=top->start,j=0; i<top->end; i++,j++) {
				Entry* entry = top->entries->items[i];
				int has_alt = j==selected_row && Entry_hasAlt(entry);
				int use_alt = has_alt && Entry_useAlt(entry);
				GFX_blitMenu(screen, entry->name, entry->path, entry->conflict, j, selected_row, has_alt, use_alt);
			}
			
			GFX_blitRule(screen, Screen.main.rule.bottom_y);
			if (can_resume) {
				if (strlen(HINT_RESUME)>1) GFX_blitPill(screen, HINT_RESUME, lang.resume, Screen.buttons.left, Screen.buttons.top);
				else GFX_blitButton(screen, HINT_RESUME, lang.resume, Screen.buttons.left, Screen.buttons.top, Screen.button.text.ox_R);
			}
			else {
#ifdef PLATFORM_RS90
				SDL_BlitSurface(btn_select_start, NULL, screen, &(SDL_Rect){8,Screen.buttons.top-2});
				GFX_blitHint(screen, lang.sleep, 48, Screen.buttons.top+Screen.hint.text_oy);
#else
				GFX_blitPill(screen, HINT_SLEEP, lang.sleep, Screen.buttons.left, Screen.buttons.top);
#endif	
			}
			
			int button_width = GFX_blitButton(screen, "A", lang.open, -Screen.buttons.right, Screen.buttons.top, Screen.button.text.ox_A);
			if (stack->count>1) {
				GFX_blitButton(screen, "B", lang.back, -(Screen.buttons.right+button_width+Screen.buttons.gutter),Screen.buttons.top, Screen.button.text.ox_B);
			}
			SDL_Flip(screen);
		}
		
		// slow down to 60fps
		unsigned long frame_duration = SDL_GetTicks() - frame_start;
		#define kTargetFrameDuration 17
		if (frame_duration<kTargetFrameDuration) SDL_Delay(kTargetFrameDuration-frame_duration);
	}
	
	// TODO: tmp
	char snap_path[256];
	sprintf(snap_path, "%s/snap.%s.bmp", Paths.rootDir, Paths.systemName);
	SDL_RWops* out = SDL_RWFromFile(snap_path, "wb");
	SDL_SaveBMP_RW(screen, out, 1);
	
	SDL_FillRect(screen, NULL, 0);
	SDL_Flip(screen);
				
	SDL_FreeSurface(logo);
#ifdef PLATFORM_RS90
	SDL_FreeSurface(btn_select_start);
#endif
	
	Menu_quit();
	GFX_quit();
	quitPlatform(); // TODO: why not Platform_quit()!? 

	SDL_Quit(); // TODO: why isn't this in GFX_quit?
	QuitSettings();
	QuitLanguage();
}