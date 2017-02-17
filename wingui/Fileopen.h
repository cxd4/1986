#ifdef _OS_WINDOWS
#include "resource.h"

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL __export MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL __export About(HWND, unsigned, WORD, LONG);

#define MAXFILENAME 256 	     /* maximum length of file pathname      */
#define MAXCUSTFILTER 40	     /* maximum size of custom filter buffer */
#endif //end #ifdef _OS_WINDOWS