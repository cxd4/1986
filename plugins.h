#ifndef _PLUGINS_H
#define _PLUGINS_H

#define PLUGIN_TYPE_RSP				1
#define PLUGIN_TYPE_GFX				2
#define PLUGIN_TYPE_AUDIO			3
#define PLUGIN_TYPE_CONTROLLER		4

typedef struct {
	uint16 Version;
	uint16 Type;
	char Name[100];

	int NormalMemory;
	int MemoryBswaped;
} PLUGIN_INFO;


typedef struct {
	HWND hWnd;
	HWND hStatusBar;
	int MemoryBswaped;
	_int8 * RDRAM;
	_int8 * DMEM;
	_int8 * IMEM;
	uint32 * MI_INTR_RG;
	uint32 * DPC_START_RG;
	uint32 * DPC_END_RG;
	uint32 * DPC_CURRENT_RG;
	uint32 * DPC_STATUS_RG;
	uint32 * DPC_CLOCK_RG;
	uint32 * DPC_BUFBUSY_RG;
	uint32 * DPC_PIPEBUSY_RG;
	uint32 * DPC_TMEM_RG;

	uint32 * VI_STATUS_RG;
	uint32 * VI_ORIGIN_RG;
	uint32 * VI_WIDTH_RG;
	uint32 * VI_INTR_RG;
	uint32 * VI_V_CURRENT_LINE_RG;
	uint32 * VI_TIMING_RG;
	uint32 * VI_V_SYNC_RG;
	uint32 * VI_H_SYNC_RG;
	uint32 * VI_LEAP_RG;
	uint32 * VI_H_START_RG;
	uint32 * VI_V_START_RG;
	uint32 * VI_V_BURST_RG;
	uint32 * VI_X_SCALE_RG;
	uint32 * VI_Y_SCALE_RG;
	void (*CheckInterrupts)( void );
} GFX_INFO;

#endif // _PLUGINS_H