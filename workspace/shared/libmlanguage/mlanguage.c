#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "mlanguage.h"
#include "mlanguage_base.h"

#define kDefaultLanguage "English"

static int exists(char* path) {
	return access(path, F_OK)==0;
}
static void put_file(char* path, char* contents) {
	FILE* file = fopen(path, "w");
	if (file) {
		fputs(contents, file);
		fclose(file);
	}
}
static void get_file(char* path, char* buffer) {
	FILE *file = fopen(path, "r");
	if (file) {
		fseek(file, 0L, SEEK_END);
		size_t size = ftell(file);
		rewind(file);
		fread(buffer, sizeof(char), size, file);
		fclose(file);
		buffer[size] = '\0';
	}
}

static void normalizeNewline(char* line) {
	int len = strlen(line);
	if (len>1 && line[len-1]=='\n' && line[len-2]=='\r') { // windows!
		line[len-2] = '\n';
		line[len-1] = '\0';
	}
}
static void trimTrailingNewlines(char* str) {
	int len = strlen(str);
	while (len>0 && str[len-1]=='\n') {
		str[len-1] = '\0'; // trim newline
		len -= 1;
	}
}

static void set(Language* lang, char* key, char* value) {
	char** string_array = (void*)lang;
	for (int i=0; i<kLanguageKeysCount; i++) {
		if (strcmp(key, language_keys[i])==0) {
			if (value!=NULL) string_array[i] = strdup(value);
			break;
		}
	}
}

#define SHM_KEY "/SharedLanguage"
static int shm_fd = -1;
static int shm_size; // size depends on language
static unsigned char* shm_mmap = NULL;

static char language_name[64];

static char PlatformRoot[256];
static char LanguagePath[256];
static char LanguagesPath[256];

void InitLanguage(Language* language) {
	sprintf(PlatformRoot, "%s", getenv("SDCARD_PATH"));
	sprintf(LanguagePath, "%s/.userdata/language.txt", PlatformRoot);
	sprintf(LanguagesPath, "%s/.system/shared/languages", PlatformRoot);
	
	shm_fd = shm_open(SHM_KEY, O_RDWR | O_CREAT | O_EXCL, 0644); // see if it exists
	if (shm_fd==-1 && errno==EEXIST) { // already exists, client
		puts("Language client");
		
		shm_fd = shm_open(SHM_KEY, O_RDWR, 0644);
		struct stat stat;
		fstat(shm_fd, &stat);
		shm_size = stat.st_size;
		shm_mmap = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	}
	else { // doesn't exist, host
		puts("Language host");
		
		if (exists(LanguagePath)) get_file(LanguagePath, language_name); // load preferred language name
		else strcpy(language_name, kDefaultLanguage); // set language to default
		trimTrailingNewlines(language_name); // just in case
		
		Language lang;
		char** lang_array = (void*)&lang;
		for (int i=0; i<kLanguageKeysCount; i++) {
			lang_array[i] = NULL;
		}
	
		// load localized strings
		char language_path[256];
		sprintf(language_path, "%s/%s.txt", LanguagesPath, language_name);
		// puts(language_path);
		if (exists(language_path)) {
			FILE* file = fopen(language_path, "r");
			if (file) {
				char line[256];
				char key[64];
				char val[1024];
				key[0] = '\0';
				val[0] = '\0';
				char* tmp;
				while (fgets(line,256,file)!=NULL) {
					if (line[0]==';') continue; // skip comment
					normalizeNewline(line);
			
					tmp = strchr(line, '=');
					if (tmp) { // new pair
						trimTrailingNewlines(val);
						if (strlen(val)>0) set(&lang, key,val);
						int len = tmp-line;
						strncpy(key,line,len);
						key[len] = '\0';
						strcpy(val,tmp+1);
					}
					else {
						strcpy(val+strlen(val), line);
					}
				}
				trimTrailingNewlines(val);
				if (strlen(val)>0) set(&lang, key,val);
		
				fclose(file);
				
				// puts("loaded");
				// for (int i=0; i<kLanguageKeysCount; i++) {
				// 	if (lang_array[i]) printf("[%s]:%s\n", language_keys[i], lang_array[i]);
				// }
			}
		}
	
		// build buffer (using defaults from base to fill in gaps)
		int buffer_capacity = 128;
		char* buffer = malloc(buffer_capacity * sizeof(char));
		uint16_t* buffer_index = (uint16_t*)buffer;
		int buffer_size = kLanguageKeysCount * (sizeof(uint16_t) / sizeof(char));

		char** base_array = (void*)&base;
		for (int i=0; i<kLanguageKeysCount; i++) {
			char* value = lang_array[i] ? lang_array[i] : base_array[i];
			value = value ? value : "";
			int len = strlen(value) + 1;
			buffer_index[i] = buffer_size;
			if (buffer_size+len>=buffer_capacity) {
				buffer_capacity *= 2;
				buffer = realloc(buffer, buffer_capacity * sizeof(char));
				buffer_index = (uint16_t*)buffer;
			}
			strncpy(buffer+buffer_size, value, len);
			buffer_size += len;
		
			if (lang_array[i]) free(lang_array[i]);
		}
		
		// copy buffer to shared memory
		shm_size = buffer_size;
		ftruncate(shm_fd, shm_size);
		shm_mmap = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
		memcpy(shm_mmap, buffer, shm_size);
		
		// printf("mapped %i\n", shm_size);
		
		free(buffer);
	}
	
	// hook shared memory up to provided Language struct
	char** language_array = (void*)language;
	uint16_t* shm_index = (uint16_t*)shm_mmap;
	for (int i=0; i<kLanguageKeysCount; i++) {
		char* value = shm_mmap+shm_index[i];
		if (value && strcmp(value,"")==0) value = NULL;
		language_array[i] = value;
		// printf("[%s]:%s\n", language_keys[i],value);
	}
	// puts(language->CJK?"CJK":"Latin");
}

void QuitLanguage(void) {
	munmap(shm_mmap, shm_size);
}

void ResetLanguage(void) {
	shm_unlink(SHM_KEY);
}