/*
 * @(#)robot_proc.c	1.29 04/06/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define XK_MISCELLANY  /* see <X11/keysymdef.h> */

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XI.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "robot_common.h"
#include "list.h"
#include "wsutils.h"
#include "multiVis.h"

static int32_t pipeToParent = -1;
static Display * robotDisplay = NULL;
static pid_t parent;
static void robot_writeParentResult(char *bytes, int32_t nBytesToWrite);
static int32_t num_buttons = 3; /* default to 3 mouse buttons */

static XImage * getWindowImage(Display * display, Window window,
                               int32_t x, int32_t y,
                               int32_t w, int32_t h) {
    XImage         *image;
    int32_t                 transparentOverlays;
    int32_t                 numVisuals;
    XVisualInfo         *pVisuals;
    int32_t                 numOverlayVisuals;
    OverlayInfo         *pOverlayVisuals;
    int32_t                 numImageVisuals;
    XVisualInfo         **pImageVisuals;
    list_ptr            vis_regions;    /* list of regions to read from */
    list_ptr            vis_image_regions ;
    int32_t        	allImage = 0 ;
    int32_t         format = ZPixmap;

    /* prevent user from moving stuff around during the capture */
    XGrabServer(display);

    /*
     * The following two functions live in multiVis.c-- they are pretty
     * much verbatim taken from the source to the xwd utility from the
     * X11 source. This version of the xwd source was somewhat better written
     * for reuse compared to Sun's version.
     *
     *        ftp.x.org/pub/R6.3/xc/programs/xwd
     *
     * We use these functions since they do the very tough job of capturing
     * the screen correctly when it contains multiple visuals. They take into
     * account the depth/colormap of each visual and produce a capture as a
     * 24-bit RGB image so we don't have to fool around with colormaps etc.
     */

    GetMultiVisualRegions(
        display, 
        window,
        x, y, w, h,
        &transparentOverlays,
        &numVisuals, 
        &pVisuals,
        &numOverlayVisuals,
        &pOverlayVisuals,
        &numImageVisuals,
        &pImageVisuals,
        &vis_regions,
        &vis_image_regions,
        &allImage );

    image = ReadAreaToImage(
        display, 
        window,
        x, y, w, h,
        numVisuals,
        pVisuals,
        numOverlayVisuals,
        pOverlayVisuals,
        numImageVisuals, 
        pImageVisuals,
        vis_regions,
        vis_image_regions,
        format,
        allImage );

    /* allow user to do stuff again */
    XUngrabServer(display);

    /*
     * we make sure the grab/ungrab is flushed so the parent process
     * won't deadlock attempting to write stuff to the console as it
     * reads the pixels back
     */
    XSync(display, False);
    return image;
}

/*
 * robot_getPixels - child side of grabbing pixels from the screen.
 */
static void robot_getPixels(RCmdGetPixels *g) {

    XImage *image;
    int32_t x;
    int32_t y;
    RResultPixelHeader header;
    jint *ary;               /* Array of jints for sending pixel values back 
                              * to parent process. 
                              */
    Window rootWindow;

    header.code = RES_PIXELHEADER;
    header.nRows = g->height;
    header.nCols = g->width;
    
    rootWindow = XRootWindow(robotDisplay, g->screen);
    image = getWindowImage(robotDisplay, rootWindow, g->x, g->y, g->width, g->height);

    /* Array to use to crunch around the pixel values */
    ary = (jint*)malloc(g->width * g->height * sizeof (jint));
    if (ary == NULL) {
        header.code = RES_OUTOFMEMORY;
        goto Exit;
    }
    
    /* convert to Java ARGB pixels */
    for (y = 0; y < g->height; y++) {
        for (x = 0; x < g->width; x++) {
            jint pixel = (jint) XGetPixel(image, x, y); /* Note ignore upper
                                                         * 32-bits on 64-bit
                                                         * OSes.
                                                         */

            pixel |= 0xff000000; /* alpha - full opacity */

            ary[(y * g->width) + x] = pixel;
        }
    }

Exit:
    /*
     * Send the data back to the parent.    We are defining
     * here the format of the data.    NOTE THAT THE PARENT
     * MUST HAVE EXACTLY MATCHING CODE TO RECEIVE THE DATA.
     *
     * Format:
     *
     *     Header (width, height, masks)
     *     width*height number of unsized longs encoded
     *            properly for direct use in Java RGB color model.
     *            (alpha, red, green, blue)
     */

    /* Write the header */
    robot_writeParentResult((char *)&header, sizeof(header));

    if (header.code != RES_OUTOFMEMORY) {
        /* Send the data to the parent process. */
        robot_writeParentResult((char *)ary,
                                g->width * g->height * sizeof (jint));
        robot_traceln("CHILD: completed writing of pixel data");
    }
    /*
     * Free image
     */
    XDestroyImage(image);
    if (ary != NULL) {
        free(ary);
    }
}

static void robot_setup(RCmdSetup * cmd, const char * displayName) {
    RResultSetup    result;
    int32_t  major_opcode, first_event, first_error;
    int32_t  event_basep, error_basep, majorp, minorp;
    Bool isXTestAvailable;
    Bool isXInputAvailable;
    int32_t numDevices, devIdx, clsIdx;
    XDeviceInfo* devices;
    XDeviceInfo* aDevice;
    XButtonInfo* bInfo;

    robot_traceln("CHILD: robot_setup");

    if (robotDisplay != NULL) {
        /* we've already been setup properly */
        isXTestAvailable = True;
        goto Exit;
    }

    /* open display connection */
    robotDisplay = XOpenDisplay(displayName);

    /* check if XTest is available */
    isXTestAvailable = XQueryExtension(robotDisplay, XTestExtensionName, &major_opcode, &first_event, &first_error);
    robot_traceln("CHILD: XQueryExtension(XTest) returns major_opcode = %d, first_event = %d, first_error = %d",
        major_opcode, first_event, first_error);
    if (!isXTestAvailable) {
        perror("XTEST extension not installed on this X server");
        goto Exit;
    }
    
    /* check if XTest version is OK */
    XTestQueryExtension(robotDisplay, &event_basep, &error_basep, &majorp, &minorp);
    robot_traceln("CHILD: XTestQueryExtension returns event_basep = %d, error_basep = %d, majorp = %d, minorp = %d", 
    event_basep, error_basep, majorp, minorp);
    if (majorp < 2 || (majorp == 2 && minorp < 2)) {
        /* bad version*/
        robot_traceln("XTEST version is %d.%d \n", majorp, minorp);
        if (majorp == 2 && minorp == 1) {
            robot_traceln("XTEST is 2.1 - no grab is available\n");
        } else {
            isXTestAvailable = False;
            goto Exit;
        }
    } else {

        /* allow XTest calls even if someone else has the grab; e.g. during
         * a window resize operation. Works only with XTEST2.2*/
         
         XTestGrabControl(robotDisplay, True);
    }

    /* 4700242:
     * If XTest is asked to press a non-existant mouse button
     * (i.e. press Button3 on a system configured with a 2-button mouse),
     * then the robot child process dies.  To avoid this, we use the XInput
     * extension to query for the number of buttons on the XPointer, and check
     * before calling XTestFakeButtonEvent().
     */
    isXInputAvailable = XQueryExtension(robotDisplay, INAME, &major_opcode, &first_event, &first_error);
    robot_traceln("CHILD: XQueryExtension(XInput) returns major_opcode = %d, first_event = %d, first_error = %d",
        major_opcode, first_event, first_error);
    if (isXInputAvailable) {
        devices = XListInputDevices(robotDisplay, &numDevices);
        for (devIdx = 0; devIdx < numDevices; devIdx++) {
            aDevice = &(devices[devIdx]);
            if (aDevice->use == IsXPointer) {
                for (clsIdx = 0; clsIdx < aDevice->num_classes; clsIdx++) {
                    if (aDevice->inputclassinfo[clsIdx].class == ButtonClass) {
                        bInfo = (XButtonInfo*)(&(aDevice->inputclassinfo[clsIdx]));
                        num_buttons = bInfo->num_buttons;
                        robot_traceln("CHILD: XPointer has %d buttons",
                                      num_buttons);
                        break;
                    }
                }
                break;
            }
        }
    }
    else {
        robot_traceln("CHILD: XQueryExtension(XInput) unavailable - assuming %d mouse buttons", num_buttons);
    }

Exit:
    result.code = RES_SETUP;
    result.isXTestAvailable = isXTestAvailable;
    robot_writeParentResult((char *)&result, sizeof(result));

    if (!isXTestAvailable) {
        /* no point in continuing now */
        _exit(1);
    }
}

static void robot_mouseMove(RCmdMove * cmd) {    
    robot_traceln("CHILD: robot_mouseMove screen = %d, x = %d, y = %d", cmd->screen, cmd->x, cmd->y);
    XTestFakeMotionEvent(robotDisplay, cmd->screen, cmd->x, cmd->y, CurrentTime);
    XFlush(robotDisplay);
}

static void robot_mouseButtonEvent(RCmdButton * cmd, Boolean isPressed) {
    robot_traceln("CHILD: robot_mouseButtonEvent mask = 0x%x, isPressed = %d", cmd->buttonMask, isPressed);
    if (cmd->buttonMask & java_awt_event_InputEvent_BUTTON1_MASK) {
        XTestFakeButtonEvent(robotDisplay, 1, isPressed, CurrentTime);
    }
    if ((cmd->buttonMask & java_awt_event_InputEvent_BUTTON2_MASK) &&
        (num_buttons >= 2)) {
        XTestFakeButtonEvent(robotDisplay, 2, isPressed, CurrentTime);
    }
    if ((cmd->buttonMask & java_awt_event_InputEvent_BUTTON3_MASK) &&
        (num_buttons >= 3)) {
        XTestFakeButtonEvent(robotDisplay, 3, isPressed, CurrentTime);
    }
    XFlush(robotDisplay);
}

static void robot_mouseWheelEvent(RCmdWheel * cmd) {

/* Mouse wheel is implemented as a button press of button 4 and 5, so it */
/* probably could have been hacked into robot_mouseButtonEvent, but it's */
/* cleaner to give it its own command type, in case the implementation   */
/* needs to be changed later.  -bchristi, 6/20/01                        */

    int32_t repeat = abs(cmd->wheelAmt);
    int32_t button = cmd->wheelAmt < 0 ? 4 : 5;  /* wheel up:   button 4 */
                                                 /* wheel down: button 5 */
    int32_t loopIdx;

    robot_traceln("CHILD: robot_mouseWheelEvent wheelAmt = 0x%x", cmd->wheelAmt);

    for (loopIdx = 0; loopIdx < repeat; loopIdx++) { /* do nothing for   */
                                                     /* wheelAmt == 0    */
        XTestFakeButtonEvent(robotDisplay, button, True, CurrentTime);
        XTestFakeButtonEvent(robotDisplay, button, False, CurrentTime);
    }
    XFlush(robotDisplay);
}

static void robot_keyEvent(RCmdKey * cmd, Boolean isDown) {
    robot_traceln("CHILD: robot_keyEvent key = 0x%x, isDown = %d", cmd->keySym, isDown);

    XTestFakeKeyEvent(robotDisplay,
        XKeysymToKeycode(robotDisplay, cmd->keySym),
        isDown,
        CurrentTime);

    XFlush(robotDisplay);
}



/*******************************************************************************/



/*
 * Checks if the parent process is still alive, and suicides if it isn't
 */
static void robot_checkParentAlive() {    
    if (kill(parent, 0) == -1) {
        /* If the parent doesn't exist, then we have no reason to exist. */
        robot_traceln("CHILD: parent %d does not exist. Exiting.", parent);
        _exit(1);
    }
}
   
/*
 * robot_readParentCommand - Convenience to read commands from parent.
 */
static void robot_readParentCommand(RCmdBase *cmd) {

    switch (robot_readBytes("CHILD", pipeToParent, (char *)cmd, sizeof (RCmdBase))) {
    case 0:
        break;
    case -1:
        _exit(1);
    }
}

/*
 * robot_writeParentResult - Convenience for child to send data to parent.
 */
static void robot_writeParentResult(char *bytes, int32_t nBytesToWrite) {
    robot_traceln("CHILD: robot_writeParentResult pid %d", parent);
    if (kill(parent, 0) == -1) {
        /* If the parent doesn't exist, then we have no reason to exist. */
        robot_traceln("CHILD: parent %d does not exist.    Exiting.", parent);
        _exit(1);
    }
    switch (robot_writeBytes("CHILD", pipeToParent, bytes, nBytesToWrite)) {
    case -1:
        _exit(1);
        break;
    }
}

/*
 * robot_executeCommands - Loop in child that reads cmds off the pipe
 *         and does whatever they say to do.
 *
 * The outline of this function is:
 *
 *     in a loop:
 *         poll     - wait for new cmd
 *         error checking & appropriate action
 *         read         - get cmd data off the pipe
 *         error checking & appropriate action
 *         do command
 *
 * Once the child process enters robot_executeCommands(), it does not leave.
 * The exit from here is the various _exit() calls scattered about
 * that handle error conditions and notice when the parent process is gone.
 *
 * NOTE: _exit() is used since the regular exit() will do things
 * that will cause unexpected results in the parent process.
 */
static void robot_executeCommands(const char * displayName) {

    RPollResult result;
    RCmdBase cmd;

    for(;;) {
        result = robot_pollForRead(pipeToParent, 1000);

        /* first ensure I'm not orphaned */
        robot_checkParentAlive();

        if (result == RPOLL_ERROR) {
            /* some nasty error so bail */
            robot_traceln("CHILD: error on poll, exiting");
            _exit(1);
        } else if (result == RPOLL_TIMEOUT) {
            /* we timed out, so continue polling for input */
/*	    robot_traceln("CHILD: select timeout, looping"); */
            continue;
        }

        /*
        * Coming here we know there are no errors and
        * that there are bytes ready to read.
        *
        * The next step is to read in the cmd and dispatch
        * to the code that handles the cmd.
        */
        robot_readParentCommand(&cmd);

        switch (cmd.code) {
            case RCMD_SETUP:
                robot_setup( (RCmdSetup *)&cmd, displayName );
                break;
            case RCMD_MOVE:
                robot_mouseMove( (RCmdMove *)&cmd );
                break;
            case RCMD_BPRESS:
                robot_mouseButtonEvent( (RCmdButton *)&cmd, True );
                break;
            case RCMD_BRELEASE:
                robot_mouseButtonEvent( (RCmdButton *)&cmd, False );
                break;
            case RCMD_WHEEL:
                robot_mouseWheelEvent( (RCmdWheel *)&cmd);
                break;
            case RCMD_KPRESS:
                robot_keyEvent( (RCmdKey *)&cmd, True);
                break;
            case RCMD_KRELEASE:
                robot_keyEvent( (RCmdKey *)&cmd, False);
                break;
            case RCMD_GETPIXELS:
                robot_getPixels( (RCmdGetPixels *)&cmd);
                break;
            default:
                robot_traceln("!!! INTERNAL ERROR : Invalid command : %d !!!", cmd.code);
            break;
        } /* switch */
    } /* for */
}

int32_t main(int argc, char **argv) {
    char * displayName;
    robot_traceln("CHILD: process started: argc = %d, pid = %d", argc, getpid());
    if (argc == 3 && strcmp(argv[0], ROBOT_ARG0) == 0) {
        /* right number of args and argv[0] is our magic cookie ... */
        parent = getppid();
        pipeToParent = atoi(argv[1]);
        displayName = argv[2];
        robot_traceln("CHILD: parent = %d, pipe = %d, display = %s", parent, pipeToParent, displayName);
        robot_executeCommands(displayName);
    } else {
        /* user probably tried to start it on the command line-- bad user! */
        perror("Invalid command line: process must be started by the AWT");
        return -1;
    }
    return 0;
}
