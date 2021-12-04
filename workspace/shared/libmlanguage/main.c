#include <stdio.h>
#include <stdlib.h>
#include <mlanguage.h>

// NOTE: called once per boot or when the language is changed

int main (int argc, char *argv[]) {
	ResetLanguage();
	Language lang;
	InitLanguage(&lang);
}