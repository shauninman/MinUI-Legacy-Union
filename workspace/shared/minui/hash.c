#include <string.h>
#include <math.h>

///////////////////////////////////////
// header

typedef struct Hash {
	int size;
	int count;
	int capacity;
	uint32_t* map;
	char** keys;
	char** values;
} Hash;
Hash* Hash_new(void);
void  Hash_free(Hash* self);
char* Hash_set(Hash* self, char* key, char* value);
char* Hash_get(Hash* self, char* key);

///////////////////////////////////////
// source

// based on http://isthe.com/chongo/tech/comp/fnv/
#define HASH_ROOT 0x01000193
#define HASH_SEED 0x811C9DC5
static uint32_t hash_key(const char* key, uint32_t hash) {
	while (*key) hash = ((unsigned char)*key++ ^ hash) * HASH_ROOT;
	return hash;
}

// from https://github.com/jamesroutley/write-a-hash-table/
static int isPrime(const int n) {
    if (n < 2) return -1; // undefined
    if (n < 4) return 1;
    if ((n % 2) == 0) return 0;
    for (int i=3; i<=floor(sqrt((double) n)); i+=2) {
        if ((n%i)==0) return 0;
    }
    return 1;
}
static int nextPrime(int n) {
    while (isPrime(n)!=1) n++;
    return n;
}

// based on https://stackoverflow.com/a/53040740/145965
static int nextPowerOfTwo(int n) {
    int i = 0;
    for (--n; n>0; n>>=1, ++i);
    return 1 << i;
}

Hash* Hash_new(void) {
	int msize;
	Hash* self = malloc(sizeof(Hash));
	
	self->count = 0;
	self->size = 16;
	self->capacity = nextPrime(self->size * 10 / 7);
	
	msize = sizeof(uint32_t) * self->capacity;
	self->map = malloc(msize);
	memset(self->map, 0, msize);

	msize = sizeof(char*) * self->size;
	self->keys = malloc(msize);
	memset(self->keys, 0, msize);

	msize = sizeof(char*) * self->size;
	self->values = malloc(msize);
	memset(self->values, 0, msize);

	return self;
}
void Hash_free(Hash* self) { // caller is responsible for malloc/free of each key/value
	free(self->values);
	free(self->keys);
	free(self->map);
	
	free(self);
}

static uint32_t Hash_index(Hash* self, char* key) {
	uint32_t hash = hash_key(key, HASH_SEED);
	uint32_t h = hash % self->capacity;
	uint32_t i = self->map[h];
	
	int len = strlen(key);
	while (1) {
		// not set
		if (i==0) return h;
				
		// match
		if (i!=-1 && strncmp(key, self->keys[i-1], len)==0) return h;
		
		// unset or collision
		hash = hash_key(key, hash);
		h = hash % self->capacity;
		i = self->map[h];
	}
	return 0;
}
static int Hash_getIndex(Hash* self, char* key) {
	uint32_t h = Hash_index(self, key);
	uint32_t i = self->map[h];
	return i>0 ?i-1 : -1;
}
static void Hash_resize(Hash* self, int newCapacity) {
	newCapacity = nextPrime(newCapacity);
	if (newCapacity<23) return;
	
	free(self->map);
	self->capacity = newCapacity;
	int msize = sizeof(uint32_t) * newCapacity;
	self->map = malloc(msize);
	memset(self->map, 0, msize);
	
	for (int i=0; i<self->count; i++) {
		uint32_t h = Hash_index(self, self->keys[i]);
		self->map[h] = i + 1;
	}
}
static void Hash_insert(Hash* self, uint32_t i, char* key, char* value) {
	if (self->count * 100 / self->capacity > 70) Hash_resize(self, self->capacity*2);
	
	int l = self->count;
	self->count += 1;
	self->map[i] = self->count;
	
	if (self->count>=self->size) {
		int offset = self->size;
		while (self->size<=self->count) self->size *= 2;
	
		self->keys = realloc(self->keys, sizeof(char*) * self->size);
		self->values = realloc(self->values, sizeof(char*) * self->size);
		memset(self->values + offset, 0, sizeof(char*) * (self->size - offset));
	}
		
	self->keys[l] = key;
	self->values[l] = value;
}
static char* Hash_delete(Hash* self, uint32_t i) {
	uint32_t h = self->map[i] - 1;
	
	char* old_value = self->values[h];
	
	self->keys[h] = NULL;
	self->values[h] = NULL;
	self->map[i] = -1;
	
	if (self->count-1==h) self->count -= 1; // removed last

	if (self->count * 100 / self->capacity < 10) Hash_resize(self, self->capacity/2);
	
	return old_value;
}

char* Hash_set(Hash* self, char* key, char* value) { // caller is responsible for malloc/free of key/value/old_value
	uint32_t i = Hash_index(self, key);
	uint32_t h = self->map[i];
	char* old_value = NULL;
	if (value==NULL) {
		if (h==0 || h==-1) return old_value;
		old_value = Hash_delete(self, i);
	}
	else {
		if (h==0) Hash_insert(self, i, key, value);
		else {
			old_value = self->values[h];
			self->values[h] = value;
		}
	}
	return old_value;
}
char* Hash_get(Hash* self, char* key) {
	uint32_t h = Hash_index(self, key);
	uint32_t i = self->map[h];
	return i>0 ? self->values[i-1] : NULL;
}