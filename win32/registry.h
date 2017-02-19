#ifndef _REGISTRY_H__
#define _REGISTRY_H__

typedef struct 
{
	int WindowXPos;
	int WindowYPos;
	int Maximized;
	int ClientWidth;
	int ThreadPriority;
	char ROMPath[MAX_PATH];
	char AudioPlugin[80];
	char InputPlugin[80];
	char VideoPlugin[80];
}  RegSettingsTyp; 

RegSettingsTyp gRegSettings;


#endif