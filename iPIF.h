#ifndef __PIF_H
#define __PIF_H

#include "options.h"

#define PIF_RAM_PHYS		0x7C0
#define IPIF_EXIT			6

extern uint8 * PIF;

typedef struct
{
	int enabled;
	int mempak;
	int use_mempak;
} t_controller;


void Init_iPIF();
void Close_iPIF();
void iPifCheck();

void ReadControllerPak(int device, char *cmd);
void WriteControllerPak(int device, char *cmd);
int ControllerCommand(unsigned __int8 *cmd, int device);

void ReadEEprom(char *dest, long offset);
void WriteEEprom(char *src, long offset);
int EEpromCommand(unsigned __int8 *cmd, int device);

void LogPIFData(char *data, int input);

#endif