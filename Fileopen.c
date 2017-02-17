/****************************************************************************

    PROGRAM: FileOpen.c

    PURPOSE: Demonstrates usage of common Open and Save As dialogs

    FUNCTIONS:

        WinMain() - calls initialization function, processes message loop
        InitApplication() - initializes window data and registers window
        InitInstance() - saves instance handle and creates main window
        MainWndProc() - processes messages
        About() - processes messages for "About" dialog box

****************************************************************************/
#ifdef _OS_WINDOWS

#include "windows.h"
#include "fileopen.h"
#include "commdlg.h"
#include "../handles.h"

HANDLE hInst;

HANDLE hAccTable;				/* handle to accelerator table */

HWND hwnd;                                      /* handle to main window */

/* new variables for common dialogs */

OPENFILENAME ofn;
char szFilterSpec [128] =                       /* file type filters */
             "N64 ROMs (*.V64, *.BIN)\0*.V64;*.BIN\0All Files (*.*)\0*.*\0";

char szFileName[MAXFILENAME];
char szFileTitle[MAXFILENAME];

char szBaseWindowTitle[] = "1964 v0.0.6";
char szWindowTitle[80];

/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;
HANDLE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;
{
    MSG msg;

    if (!hPrevInstance)
        if (!InitApplication(hInstance))
            return (FALSE);

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    while (GetMessage(&msg, NULL, NULL, NULL)) {

    /* Only translate message if it is not an accelerator message */

        if (!TranslateAccelerator(hwnd, hAccTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (msg.wParam);
}


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

    PURPOSE: Initializes window data and registers window class

****************************************************************************/

BOOL InitApplication(hInstance)
HANDLE hInstance;
{
    WNDCLASS  wc;

    wc.style = NULL;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  "FileOpenMenu";
    wc.lpszClassName = "FileOpenWClass";

    return (RegisterClass(&wc));
}


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
    HANDLE          hInstance;
    int             nCmdShow;
{
    RECT            Rect;

    hInst = hInstance;

    hAccTable = LoadAccelerators(hInst, "FileOpenAcc");

    hwnd = CreateWindow(
        "FileOpenWClass",
        szBaseWindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd)
        return (FALSE);

    GetClientRect(hwnd, (LPRECT) &Rect);

    /* Create a child window */

    hEditWnd = CreateWindow("Edit",
        NULL,
        WS_CHILD | WS_VISIBLE |
        ES_MULTILINE |
        WS_VSCROLL | WS_HSCROLL |
        ES_AUTOHSCROLL | ES_AUTOVSCROLL,
        0,
        0,
        (Rect.right-Rect.left),
        (Rect.bottom-Rect.top),
        hwnd,
        IDD_FORMVIEW,                          /* Child control i.d. */
        hInst,
        NULL);

    if (!hEditWnd) {
        DestroyWindow(hwnd);
        return (NULL);
    }

    /* fill in non-variant fields of OPENFILENAME struct. */
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner	  = hwnd;
    ofn.lpstrFilter	  = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter	  = 0;
    ofn.nFilterIndex	  = 1;
    ofn.lpstrFile         = szFileName;
    ofn.nMaxFile	  = MAXFILENAME;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrFileTitle    = szFileTitle;
    ofn.nMaxFileTitle     = MAXFILENAME;
    ofn.lpstrTitle        = NULL;
    ofn.lpstrDefExt       = "TXT";
    ofn.Flags             = 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return (TRUE);

}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

    COMMENTS:

        WM_COMMAND processing:

            IDM_OPEN - query to save current file if there is one and it
                       has been changed, open a new file.

            IDM_ABOUT - display "About" box.

****************************************************************************/

long FAR PASCAL __export MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;


{
	char* argv[2];
    FARPROC lpProcAbout;

    switch (message) {
        case WM_COMMAND:
            switch (wParam) {
                case IDM_ABOUT:
                    lpProcAbout = MakeProcInstance(About, hInst);
                    DialogBox(hInst, "ABOUTBOX", hWnd, lpProcAbout);
                    FreeProcInstance(lpProcAbout);
                    break;

                case IDM_OPEN:

                    /* Use standard open dialog */

		    if (!GetOpenFileName ((LPOPENFILENAME)&ofn))
			return FALSE;

                    lstrcpy(szWindowTitle, szBaseWindowTitle);
                    lstrcat(szWindowTitle, " - ");
                    lstrcat(szWindowTitle, szFileTitle);
					lstrcat(szWindowTitle, "   <CTRL-C> to Quit");
                    SetWindowText(hWnd, szWindowTitle);
					MessageBox(hWnd,
						"Warning: We are losing the handle to the default console screen buffer. As a result, you will not see the instructions at the lower half of the screen until we fix this. Until then, please use the non-GUI build to debug because that one works properly and is significantly faster. Read \"Help...About\" for more information. This GUI build is provided here as both a demo and as a template for GUI gurus to improve.", 
						"Note:", MB_OK);

					AllocConsole();
					argv[1] = szFileName;
					main(2, argv);
                    /* Let user know that "open" functionality is not yet
                       implemented */
		    MessageBox (
                          hWnd,
			  "Command not implemented",
			  "1964",
			  MB_ICONASTERISK | MB_OK);
                    break;

                case IDM_SAVEAS:

                    /* Use standard save dialog */

                    if (!GetSaveFileName ((LPOPENFILENAME)&ofn))
			return FALSE;

                    lstrcpy(szWindowTitle, szBaseWindowTitle);
                    lstrcat(szWindowTitle, " - ");
                    lstrcat(szWindowTitle, szFileTitle);
                    SetWindowText(hWnd, szWindowTitle);

                    /* Let user know that "save" functionality is not yet
                       implemented */
		    MessageBox (
                          hWnd,
			  "Command not implemented",
			  "1964",
			  MB_ICONASTERISK | MB_OK);
                    break;

                case IDM_NEW:
                case IDM_SAVE:
                case IDM_PRINT:
                    MessageBox (
                          GetFocus(),
                          "Command not implemented",
                          "1964",
                          MB_ICONASTERISK | MB_OK);
                    break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                /* edit menu commands */

                case IDM_UNDO:
                case IDM_CUT:
                case IDM_COPY:
                case IDM_PASTE:
                case IDM_CLEAR:
                    MessageBox (
                          GetFocus(),
                          "Command not implemented",
                          "1964",
                          MB_ICONASTERISK | MB_OK);
                    break;

                case IDC_EDIT:
                    if (HIWORD (lParam) == EN_ERRSPACE) {
                        MessageBox (
                              GetFocus ()
                            , "Out of memory."
                            , "1964"
                            , MB_ICONHAND | MB_OK
                        );
                    }
                    break;

            }
            break;

        case WM_SETFOCUS:
            SetFocus (hEditWnd);
            break;

        case WM_SIZE:
            MoveWindow(hEditWnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}

/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

****************************************************************************/

BOOL FAR PASCAL __export About(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
	    if (wParam == IDOK
                || wParam == IDCANCEL) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}

#endif //end #ifdef _OS_WINDOWS