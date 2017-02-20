#include <windows.h>
#include <stdio.h>

void __cdecl LogDyna(char *debug, ...)
{
	va_list argptr;
	char	text[1024];
	FILE *stream;

	stream = fopen("c:/dyna.log", "at");
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

	stream = fopen("c:/dyna.log", "wt");
	if (stream == NULL) return;

	fprintf(stream," -- DYNA LOG -- \n\n");	
	fclose(stream);

}