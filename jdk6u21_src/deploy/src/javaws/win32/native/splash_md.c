/*
 * @(#)splash_md.c	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "util.h"
#include <setjmp.h>
#include <locale.h>

/* This is the socket we listen on.
 */
static SOCKET server;

/* This is the message that's sent when there's a pending connection
 * on our server socket.  See the call.
 */
#define WM_CONNECT WM_USER + 1

static void* hSplashLib = NULL;

void* SplashProcAddress(const char* name) {
    if (!hSplashLib) {
	char *path = sysGetLibPath("splashscreen");
        hSplashLib = LoadLibrary(path);
    }
    if (hSplashLib) {
        return GetProcAddress(hSplashLib, name);
    } else {
        return NULL;
    }
}

void SplashFreeLibrary() {
    if (hSplashLib) {
        FreeLibrary(hSplashLib);
        hSplashLib = NULL;
    }
}

/*
 * Prototypes of pointers to functions in splashscreen shared lib
 */
typedef int (*SplashLoadFile_t)(const char* filename);
typedef void (*SplashInit_t)(void);
typedef void (*SplashClose_t)(void);

/*
 * This macro invokes a function from the shared lib.
 * it locates a function with SplashProcAddress on demand.
 * if SplashProcAddress fails, def value is returned.
 *
 * it is further wrapped with INVOKEV (works with functions which return
 * void and INVOKE (for all other functions). INVOKEV looks a bit ugly,
 * that's due being unable to return a value of type void in C. INVOKEV
 * works around this by using semicolon instead of return operator.
 */
#define _INVOKE(name,def,ret) \
    static void* proc = NULL; \
    if (!proc) { proc = SplashProcAddress(#name); } \
    if (!proc) { return def; } \
    ret ((name##_t)proc)

#define INVOKE(name,def) _INVOKE(name,def,return)
#define INVOKEV(name) _INVOKE(name,;,;)

void    DoSplashInit(void) {
    INVOKEV(SplashInit)();
}

int     DoSplashLoadFile(const char* filename) {
    INVOKE(SplashLoadFile,0)(filename);
}

void    DoSplashClose(void) {
    INVOKEV(SplashClose)();
}

/* Number of seconds to wait before taking the splash screen down.
 * Normally we'll get a "Z" message before the timeout occurs however
 * if something's gone wrong or the VM is REALLY slow coming up we
 * exit after this many seconds.
 */
#define TIMEOUT 20


/* 
 * Clean up any resources that will not be cleaned up properly as 
 * a side-effect of just exiting, and then exit.
 */
void splashExit() {
    DoSplashClose();
    exit(0);
}


/* 
 * Print an (obscure) error message and exit.  
 */
void errorExit(char *msg) {
    char *msg1 = getMsgString(MSG_SPLASH_EXIT);
    fprintf(stderr, msg1);
    if (errno != 0) {
	perror(msg);
    }
    else {
	fprintf(stderr, "\t%s\n", msg);
    }
    splashExit();
}

VOID CALLBACK timeoutProc(HWND hWnd, UINT uMsg, UINT uTimerID, DWORD dwTime)
{
    splashExit();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int width = 0, height = 0;

    switch (message) {
        case WM_CONNECT:
        {
            SOCKADDR_IN iname;
            int length = sizeof(iname);
            SOCKET client;
            char cmd[1];

            if ((client = accept(server, (SOCKADDR *)&iname, &length)) == -1) {
                errorExit(getMsgString(MSG_ACCEPT_FAILED));
            }
            if (recv(client, cmd, 1, 0) == 1) {
                switch(cmd[0]) {
                case 'Z':
                    sysCloseSocket(client);
                    splashExit();
                    break;
                default:
                    errorExit(getMsgString(MSG_SPLASH_CMND));
                }
            }
        }

        case WM_DESTROY:
        {
            splashExit();
            break;
        }

        default:
	{
            return DefWindowProc(hWnd, message, wParam, lParam);
	}
    }

    return 0;
}



int sysSplash(int splashPort, char *splashFile)
{
    WNDCLASS wce = {0};
    HWND hWnd;
    MSG msg;
    int port;
    int ud = CW_USEDEFAULT;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
        errorExit(getMsgString(MSG_LISTENER_FAILED));
    }


    /* Send our ephemeral port back to the parent process.  The parent
     * process port number is splashPort.  We send the port number back
     * to the parent as a 6 character string.
     */

    {
        SOCKADDR_IN iname = {0};
        SOCKET parent;
        HWND hWnd = NULL;
        char data[6];

        if (splashPort <= 0) {
            errorExit(getMsgString(MSG_SPLASH_PORT));
        }

        if ((parent = sysCreateClientSocket(splashPort)) == INVALID_SOCKET) {
            errorExit(getMsgString(MSG_SPLASH_SOCKET));
        }

        sprintf(data, "%d", port);
        if (send(parent, data, 6, 0) != 6) {
            errorExit(getMsgString(MSG_SPLASH_SEND));
        }

        sysCloseSocket(parent);

    }

    /* Check for NO Splash mode */
    if (splashFile == NULL) {
        return 0;
    }

    // create a dummy window to get events on:
    wce.style = 0;
    wce.lpfnWndProc = (WNDPROC)WndProc;
    wce.hInstance = hInstance;
    wce.hCursor = LoadCursor(NULL, IDC_ARROW);
    wce.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wce.lpszClassName = "SplashDummy";
    RegisterClass(&wce);

    hWnd = CreateWindow("SplashDummy", "Win32Splash", WS_OVERLAPPEDWINDOW,
	   ud, ud, ud, ud, NULL, NULL, hInstance, NULL);

    /* Set a timer that will exit this process after TIMEOUT seconds
     */
    if (SetTimer(hWnd, 0, TIMEOUT * 1000, (TIMERPROC)timeoutProc) == 0) {
        errorExit(getMsgString(MSG_SPLASH_TIMER));
    }

    DoSplashInit();
    DoSplashLoadFile(splashFile);

    /* Ask Win32 to notify us when there are pending connections on the
     * server socket.  WM_CONNECT is an "event" defined here.
     */
    WSAAsyncSelect(server, hWnd, WM_CONNECT, FD_ACCEPT);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //return msg.wParam;
    return 0;

}

