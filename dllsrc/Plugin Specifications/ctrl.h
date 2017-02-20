/**********************************************************************************
Ctrl plugin spec for 1964, version #1.0

any questions or suggestions can be emailed to noote@bigfoot.com
**********************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/* Plugin types */
#define PLUGIN_TYPE_RSP				1
#define PLUGIN_TYPE_GFX				2
#define PLUGIN_TYPE_AUDIO			3
#define PLUGIN_TYPE_CONTROLLER		4

/***** Structures *****/
typedef struct {
	WORD Version;        /* Should be set to 1 */
	WORD Type;           /* Set to PLUGIN_TYPE_CONTROLLER */
	char Name[100];      /* Name of the DLL */

	/* If DLL supports memory these memory options then set them to TRUE or FALSE
	   if it does not support it */
	BOOL NormalMemory;    /* a normal BYTE array */ 
	BOOL MemoryBswaped;  /* a normal BYTE array where the memory has been pre
	                          bswap on a dword (32 bits) boundry */
} PLUGIN_INFO;

/******************************************************************
  Function: CloseDLL
  Purpose:  This function is called when the emulator is closing
            down allowing the dll to de-initialise.
  input:    none
  output:   none
*******************************************************************/ 
__declspec(dllexport) void CloseDLL (void);

/******************************************************************
  Function: DllAbout
  Purpose:  This function is optional function that is provided
            to give further information about the DLL.
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
__declspec(dllexport) void DllAbout ( HWND hParent );

/******************************************************************
  Function: DllConfig
  Purpose:  This function is optional function that is provided
            to allow the user to configure the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
__declspec(dllexport) void DllConfig ( HWND hParent );

/******************************************************************
  Function: DllTest
  Purpose:  This function is optional function that is provided
            to allow the user to test the dll
  input:    a handle to the window that calls this function
  output:   none
*******************************************************************/ 
__declspec(dllexport) void DllTest ( HWND hParent );

/******************************************************************
  Function: GetDllInfo
  Purpose:  This function allows the emulator to gather information
            about the dll by filling in the PluginInfo structure.
  input:    a pointer to a PLUGIN_INFO stucture that needs to be
            filled by the function. (see def above)
  output:   none
*******************************************************************/ 
__declspec(dllexport) void GetDllInfo ( PLUGIN_INFO * PluginInfo );

/******************************************************************
  Function: RomClosed
  Purpose:  This function is called when a rom is closed.
  input:    none
  output:   none
*******************************************************************/ 
__declspec(dllexport) void RomClosed (void);

/******************************************************************
  Function: RomOpen
  Purpose:  This function is called when a rom is open. (from the 
            emulation thread)
  input:    none
  output:   none
*******************************************************************/ 
__declspec(dllexport) void RomOpen (void);

/******************************************************************
  Function: CheckController
  Purpose:  This function is called in response to a ???
  input:    the controller id (1 is first controller, 2 is second
            controller...)
  output:   state controller : first WORD is buttons state, second
            is analogic stick movement (first BYTE x, second BYTE y)
*******************************************************************/ 
__declspec(dllexport) unsigned __int32 CheckController (int controller_id);

/******************************************************************
  Function: CheckController
  Purpose:  This function is called in response to a ???
  input:    the controller id (1 is first controller, 2 is second
            controller...)
  output:   state controller : first WORD is buttons state, second
            is analogic stick movement (first BYTE x, second BYTE y)
*******************************************************************/ 
__declspec(dllexport) BOOL Initialization(HWND,HINSTANCE);

/******************************************************************
  Function: CheckController
  Purpose:  This function is called in response to a ???
  input:    the controller id (1 is first controller, 2 is second
            controller...)
  output:   state controller : first WORD is buttons state, second
            is analogic stick movement (first BYTE x, second BYTE y)
*******************************************************************/ 
__declspec(dllexport) void ApplicationSyncAcquire(HWND, int);

#if defined(__cplusplus)
}
#endif