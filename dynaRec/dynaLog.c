#include <windows.h>
#include <stdio.h>
#include "../globals.h"

void __cdecl LogDyna(char *debug, ...)
{
	va_list argptr;
	char	text[1024];
	FILE *stream;
	char filename[256];

	strcpy(filename, main_directory);
	strcat(filename, "dyna.log");


	stream = fopen(filename, "at");
	if (stream == NULL) return;

	va_start(argptr, debug);
	vsprintf(text, debug, argptr);
	va_end(argptr);

	fprintf(stream,"%s", text);	
	fclose(stream);
}


void InitLogDyna()
{
	FILE *stream;
	char filename[256];

	strcpy(filename, main_directory);
	strcat(filename, "dyna.log");


	stream = fopen(filename, "wt");
	if (stream == NULL) return;

	fprintf(stream," -- DYNA LOG -- \n\n");	
	fclose(stream);

}