/*
 * @(#)splash_md.c	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * The Java Web Start splash screen "server" for X11. 
 */

/*
 * The code for loading a JPEG file was lifted from example.c and
 * from jdatasrc.c, look there and in libjpeg.doc for more information
 * about how libjpeg is supposed to be used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <libintl.h>
#include "system.h"
#include <X11/Xlib.h>


static void* hSplashLib = NULL;

void* SplashProcAddress(const char* name) {
    if (!hSplashLib) {
	char *path = sysGetLibPath("splashscreen");
        hSplashLib = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    }
    if (hSplashLib) {
        void* sym = dlsym(hSplashLib, name);
        return sym;
    } else {
        return NULL;
    }
}

void SplashFreeLibrary() {
    if (hSplashLib) {
        dlclose(hSplashLib);
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

void alarmHandler(int n) 
{
    if (n == SIGALRM) {
	splashExit();
    }
}

int sysSplash(int splashPort, char *splashFile)
{
    Display *display;
    SOCKET server;
    int port;
    int i;
    size_t mainSize;

    if (splashPort <= 0) {
	errorExit(getMsgString(MSG_SPLASH_PORT));
    }


    if ((server = sysCreateListenerSocket(&port)) == INVALID_SOCKET) {
        errorExit(getMsgString(MSG_LISTENER_FAILED));
    }

    /* Send our ephemeral port back to the parent process.  The parent 
     * process port number is argv[1].  We send the port number back
     * to the parent as a 6 character string.
     */
    {
	SOCKADDR_IN iname = {0};
	SOCKET parent;
	char data[6]; 

	parent = sysCreateClientSocket(splashPort);

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

    if ((display = XOpenDisplay(NULL)) == 0) {
        errorExit(getMsgString(MSG_SPLASH_X11_CONNECT));
    }

    DoSplashInit();
    DoSplashLoadFile(splashFile);

    /* Set a timer that will exit this process after TIMEOUT seconds 
     */
    alarm(TIMEOUT);

    {
        SOCKADDR_IN iname = {0};
        SOCKET client;
        int length = sizeof(SOCKADDR_IN);
        char cmd[1];

        if ((client = accept(server, (SOCKADDR *)&iname, &length)) == -1) {
            errorExit(getMsgString(MSG_ACCEPT_FAILED));
	}

        if (recv(client, cmd, 1, 0) == 1) {
            if (cmd[0] == 'Z') {
                sysCloseSocket(client);
                splashExit();
            } else { 
                errorExit(getMsgString(MSG_SPLASH_CMND));
            }
        }
    }

    return 0;
}


