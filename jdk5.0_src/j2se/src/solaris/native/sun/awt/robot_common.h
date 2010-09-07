/*
 * @(#)robot_common.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _ROBOT_COMMON_H
#define _ROBOT_COMMON_H

#include "java_awt_event_InputEvent.h"
#include "java_awt_event_KeyEvent.h"
#include "java_awt_event_MouseEvent.h"
#include <X11/Xlib.h>
#include "gdefs.h"

enum {
    RCMD_BEGIN = 0,
    RCMD_SETUP = RCMD_BEGIN,
    RCMD_MOVE,
    RCMD_BPRESS,
    RCMD_BRELEASE,
    RCMD_WHEEL,
    RCMD_KPRESS,
    RCMD_KRELEASE,
    RCMD_GETPIXELS,
    RCMD_END = RCMD_GETPIXELS
};


/* MAKE SURE that all RCmdXXX's are padded to the same length */

typedef struct RCmdBase {
    int32_t code;
    int32_t pad1;
    int32_t pad2;
    int32_t pad3;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdBase;

typedef struct RCmdSetup {
    int32_t code;
    int32_t pad1;
    int32_t pad2;
    int32_t pad3;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdSetup;

typedef struct RCmdMove {
    int32_t code;
    int32_t screen;
    int32_t x;
    int32_t y;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdMove;

typedef struct RCmdButton {
    int32_t code;
    int32_t buttonMask;
    int32_t pad2;
    int32_t pad3;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdButton;

typedef struct RCmdWheel {
    int32_t code;
    int32_t wheelAmt;
    int32_t pad2;
    int32_t pad3;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdWheel;

typedef struct RCmdKey {
    int32_t code;
    int32_t keySym;             /* This is to be an X11 KeySym. */
    int32_t pad2;
    int32_t pad3;
    int32_t pad4;
    int32_t pad5;
    int32_t pad6;
} RCmdKey;

typedef struct RCmdGetPixels{
    int32_t code;
    int32_t screen;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t pad6;
} RCmdGetPixels;

enum {
    RES_OUTOFMEMORY,
    RES_SETUP,
    RES_PIXELHEADER,
    RES_PIXELDATA,
    RES_RGBVAL
};

typedef struct RResultSetup {
    uint32_t code;
    int32_t isXTestAvailable;
} RResultSetup;

typedef struct RResultPixelHeader {
    uint32_t code;
    uint32_t nRows;
    uint32_t nCols;
} RResultPixelHeader;

typedef enum RPollResult {
    RPOLL_ERROR,
    RPOLL_TIMEOUT,
    RPOLL_UNBLOCKED
} RPollResult;

extern const char * ROBOT_ARG0; /* special cookie passed to child process */

extern int32_t robot_isCommandValid(int32_t code);
extern void robot_traceln(const char * format, ...);
extern int32_t robot_writeBytes(const char *role, int32_t filedes, char *bytes,                                 int32_t nBytesToWrite);
extern int32_t robot_readBytes(const char *role, int32_t filedes, char *bytes,
                               size_t nToRead);
extern RPollResult robot_pollForRead(int32_t fd, int32_t timeout);
extern void robot_readFlush();

extern int QueryColorMap(Display *disp,
                         Colormap src_cmap,
                         Visual *src_vis, 
                         XColor **src_colors,
                         int *rShift, int *gShift, int *bShift);
           
#endif /* _ROBOT_COMMON_H */
