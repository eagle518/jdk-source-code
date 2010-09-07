/*
 * @(#)awt_Robot.c	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "awt_Component.h"
#include "awt_GraphicsEnv.h"
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Intrinsic.h>
#include <X11/Xmd.h>
#include <X11/extensions/xtestext1.h>
#include "sun_awt_motif_MRobotPeer.h"
#include <jni.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stropts.h>
#include "robot_common.h"
#include "canvas.h"
#ifdef __linux__
#include <sys/socket.h>
#endif

extern struct X11GraphicsConfigIDs x11GraphicsConfigIDs;

#define MAX_DIGITS  64
#if defined(DEBUG) 
static const char * ROBOT_EXE = "awt_robot_g";
#else
static const char * ROBOT_EXE = "awt_robot";
#endif
static char RobotChildExePath[FILENAME_MAX+1] = "";
static pid_t child    = -1;
static int pipeToChild = -1;

/*
 * This is a workaround for bug# 4198687. [David.Herron@eng]
 * The bug is that DnD code (awt_XmDnD.c) leaves AWT_LOCK locked
 * for long periods of time.    Lacking a way around that, this
 * workaround side steps by creating a child process whose job
 * is creating events.
 *
 * While developing DnD test cases it was noticed that the Robot
 * would lock up and stop dispatching events.    Upon investigation
 * this sequence was found:
 *
 *        The code was stopped at the AWT_LOCK calls in awt_Robot.c (this file).
 *
 *        In awt_XmDnD.c the AWT_LOCK is left locked for long periods
 *        of time.    You can see this by inspecting the source code.
 *        Jeff Dunn characterised this as a bug and asked
 *        me to file a bug report against it.
 *
 *        This problem shows up in Robot because;
 *
 *                The test case creates a DnD situation.
 *
 *                In the DnD situation AWT_LOCK is locked.
 *
 *                Robot tries to dispatch further events
 *                but cannot because it stops at AWT_LOCK
 *                (which awt_XmDnD.c is holding locked).
 *
 *                The test case completely stops at that time and
 *                is unable to continue because it is blocked from
 *                dispatching the events that would let it continue.
 *
 * It is necessary to use AWT_LOCK because X11 is not thread safe.
 * If we run Robot without using AWT_LOCK other problems occur.
 * But, given the situation just described, there is not a good
 * solution to using Robot within DnD scenarios.
 *
 * This workaround sidesteps the problem in this way;
 *
 *        A child process is created, with a pipe to that
 *        process so we can send it information.
 *
 *        A series of "command cmds" are defined below.
 *        These are data structures encapsulating the information
 *        needed to synthesize each event.    There is a different
 *        command cmd for each native method in Robot.
 *
 *        When a Robot method is called, the command cmd
 *        is formed and written over the pipe to the child process.
 *
 *        The child process has it's own connection to the display.
 *        The child process is in a loop, reading command cmds,
 *        and synthesizing events to the display.    It stays in
 *        native code to do this.
 *
 *        Error checking is scattered throughout this so that if
 *        the child were to die a new one is created, and if the
 *        parent dies the child exits quickly.
 *
 * Since it is a separate process doing event synthesis the Xlib
 * calls are in a completely different address space.    There is
 * no need for locking since the data structures are separate.
 *
 * NOTE: Currently the command cmds are only written to
 * the child.    There is no data sent back, and no handshaking
 * by the parent to wait for the child to finish processing
 * the command.    As of this writing the other half of the pipe
 * is still open and could be used as a back channel for data
 * returning to the parent process.
 *
 * NOTE: Each command cmd is the same number of data
 * bytes, so that there is no trouble with handshaking
 * on cmd boundries.
 */

static void robot_setupPipe(int fd) {
    int flags = 0;
    int returnValue = 0;
    
    flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
	/*                fprintf(stderr, 
	 *        "ERROR OCCURRED IN robot_setupPipe(). "
	 *        "File output may be buffered.\n");
	 */
	    return;
    }

    flags |= O_NDELAY|O_SYNC|O_DSYNC|O_RSYNC;
    if (fcntl(fd, F_SETFL, flags) != -1) {
	/* fprintf(stderr, 
	 *     "ERROR OCCURRED IN robot_setupPipe(). "
	 *        "File output may be buffered.\n");
	 */
	    return;
    }
} /* setFdNoBuffer() */

/*
 * robot_makeChild - Ensure the child process exists.
 */
static void robot_makeChild() {
    int     pipes[2] = { -1, -1 };

    /*
     * First check if child exists.
     */
    if (child != -1) {
        /*
         * Sending signal-0 only does error checking.
         * If it returns -1 the child does not exist.
         */
        if (kill(child, 0) != -1) {
            return;
        }
    }

    /*
     * It does not exist.    Set up the pipe, fork, etc.
     */
#ifdef __linux__
    socketpair(AF_UNIX, SOCK_STREAM, 0, pipes);
#else
    pipe(pipes);
#endif
    pipeToChild = pipes[0];

#ifdef __linux__
    child = fork();     /* fork1() only re-creates the one thread */
#else
    child = fork1();     /* fork1() only re-creates the one thread
		            * That's all we need.
		            */
#endif
    /* The child goes off to handle cmds.
     * Nothing for the parent to do but continue
     * about its own business.
     */
    if (child == 0) {
        /* in child */
	int pipeToParent;
	char arg1[MAX_DIGITS+1];
	char * arg2;

	pipeToParent = dup(pipes[1]);
	sprintf(arg1, "%d", pipeToParent);
	arg2 = DisplayString(awt_display);
	execl(RobotChildExePath, ROBOT_ARG0, arg1, arg2, NULL);
	perror("Couldn't execl robot child process");
    } else {
        /* in parent */
        /* SIGPIPE would make us crash.    So we ignore it */
        sigignore(SIGPIPE);
        robot_setupPipe(pipeToChild);
    }
}

/*
 * robot_writeChildCommand - Convenience to send commands to child process.
 */
static void robot_writeChildCommand(RCmdBase *cmd) {
    int res;
    int nTries;

    robot_makeChild();    /* This only (re)creates a child when absolutely necessary */
    for (res = robot_writeBytes("PARENT", pipeToChild, (char *)(cmd), sizeof (RCmdBase)),
	 nTries = 0;
             res != 0 && nTries < 10;
             res = robot_writeBytes("PARENT", pipeToChild, ((char *)cmd), sizeof (RCmdBase)),
	 nTries++) {
        robot_makeChild();
    }

}

/*
 * robot_getChildResult - Convenience for parent to receive from child.
 */
static int robot_getChildResult(char *bytes, int nBytesToRead) {
    return robot_readBytes("PARENT", pipeToChild, bytes, nBytesToRead);
}

static void robot_flushChildResult() {
    robot_readFlush(pipeToChild);
}

/*********************************************************************************************/

#ifdef XAWT
#define FUNC_NAME(name) Java_sun_awt_X11_XRobotPeer_ ## name
#else
#define FUNC_NAME(name) Java_sun_awt_motif_MRobotPeer_ ## name
#endif

/*
 * Given the JDK/JRE install directory form the name of the robot child process
 * e.g. Directory /foo/kestrel/jre -> Executable /foo/kestrel/jre/bin/awt_robot
 */
JNIEXPORT void JNICALL 
FUNC_NAME(buildChildProcessName) (
    JNIEnv *env, jclass cls, jstring installDir) {
    char * cdir;	

    cdir = (char *) JNU_GetStringPlatformChars(env, installDir, NULL);
    if (cdir == NULL) {
	return;
    }
    sprintf(RobotChildExePath, "%s/%s", cdir, ROBOT_EXE);
    DTRACE_PRINTLN1("Robot child process name is : %s", RobotChildExePath);
    JNU_ReleaseStringPlatformChars(env, installDir, cdir);
}

JNIEXPORT void JNICALL 
FUNC_NAME(setup) (JNIEnv * env, jclass cls) {
    RCmdSetup cmd;
    RResultSetup result;

    DTRACE_PRINTLN("MRobotPeer.setup()");
    cmd.code = RCMD_SETUP;
    robot_writeChildCommand((RCmdBase *)&cmd);
    DTRACE_PRINTLN("PARENT: Waiting for setup result ...");
    robot_getChildResult((char *)&result, sizeof result);
    DTRACE_PRINTLN1("PARENT: XTest available = %d", result.isXTestAvailable);

    if (!result.isXTestAvailable) {
	JNU_ThrowByName(env, "java/awt/AWTException", "java.awt.Robot requires your X server support the XTEST extension version 2.2");
    }
}

JNIEXPORT void JNICALL 
FUNC_NAME(getRGBPixelsImpl)(
		             JNIEnv *env,
		             jclass cls,
		             jobject xgc,
		             jint x,
		             jint y,
		             jint width,
		             jint height,
		             jintArray pixelArray) {
    RCmdGetPixels cmd;
    RResultPixelHeader result;
    int row;
    int col;
    unsigned int *ary;
    AwtGraphicsConfigDataPtr adata;

    DTRACE_PRINTLN7("%lx: MRobotPeer.getRGBPixelsImpl(%lx, %d, %d, %d, %d, %x)", cls, xgc, x, y, width, height, pixelArray);
    /* avoid a lot of work for empty rectangles */
    if ( (width * height) == 0 ) {
	return;
    }
    DASSERT(width * height > 0); /* only allow positive size */

    adata = (AwtGraphicsConfigDataPtr) JNU_GetLongFieldAsPtr(env, xgc, x11GraphicsConfigIDs.aData);
    DASSERT(adata != NULL);

    cmd.code = RCMD_GETPIXELS;
    cmd.screen = adata->awt_visInfo.screen;
    cmd.x = x;
    cmd.y = y;
    cmd.width = width;
    cmd.height = height;

    robot_writeChildCommand((RCmdBase *)&cmd);

    /*
     * For getPixels, there is data to receive
     * from the child process.    The following sequence
     * must match the sequence in the getPixels method.
     *
     * The robot_readBytes function automatically provides
     * synchronization with the sending of data
     * back to us.
     */

    DTRACE_PRINTLN("PARENT: Waiting for pixels ...");

    robot_getChildResult((char *)&result, sizeof(result));

    DTRACE_PRINTLN2("PARENT: Got pixels header %d x %d", result.nRows, result.nCols);

    if (result.code == RES_OUTOFMEMORY) {
	JNU_ThrowOutOfMemoryError(env, "OutOfMemoryError");
	return;
    }

    ary = (unsigned int *)malloc(height * width * sizeof(jint));
    if (ary == NULL) {
	JNU_ThrowOutOfMemoryError(env, "OutOfMemoryError");
	/* eat up the data in the pipe so things remain in sync */
	robot_flushChildResult();
	return;
    }

    DASSERT(result.nRows == height && result.nCols == width);
    robot_getChildResult((char *)ary, (jint) (height * width * sizeof (jint)));

    (*env)->SetIntArrayRegion(env, pixelArray, 0, height * width, (jint *)ary);

    free((char *)ary);
}

JNIEXPORT void JNICALL 
FUNC_NAME(keyPressImpl) (JNIEnv *env,
				     jclass class,
				     jint keycode) {
    RCmdKey cmd;

    DTRACE_PRINTLN1("MRobotPeer.keyPress(%d)", keycode);

    cmd.code = RCMD_KPRESS;
    cmd.keySym = awt_getX11KeySym(keycode);
    if (cmd.keySym != 0) {
	robot_writeChildCommand((RCmdBase *)&cmd);
    } else {
    /* couldn't find a key mapping so it must be invalid */
	JNU_ThrowIllegalArgumentException(env, "Invalid key code");
    }
}

JNIEXPORT void JNICALL
FUNC_NAME(keyReleaseImpl) (JNIEnv *env,
					 jclass class,
					 jint keycode) {
    RCmdKey cmd;

    DTRACE_PRINTLN1("MRobotPeer.keyRelease(%d)", keycode);

    cmd.code = RCMD_KRELEASE;
    cmd.keySym = awt_getX11KeySym(keycode);
    if (cmd.keySym != 0) {
	robot_writeChildCommand((RCmdBase *)&cmd);
    } else {
    /* couldn't find a key mapping so it must be invalid */
	JNU_ThrowIllegalArgumentException(env, "Invalid key code");
    }
}

JNIEXPORT void JNICALL 
FUNC_NAME(mouseMoveImpl) (JNIEnv *env,
					jclass class,
					jobject x11GraphicsConfig,
					jint root_x,
					jint root_y) {
    RCmdMove cmd;
    AwtGraphicsConfigDataPtr adata;

    DTRACE_PRINTLN2("MRobotPeer.mouseMoveImpl(%d, %d)", root_x, root_y);
    
    adata = (AwtGraphicsConfigDataPtr) JNU_GetLongFieldAsPtr(env, x11GraphicsConfig, x11GraphicsConfigIDs.aData);
    DASSERT(adata != NULL);

    cmd.code = RCMD_MOVE;
    cmd.x = root_x;
    cmd.y = root_y;
    cmd.screen = adata->awt_visInfo.screen;

    robot_writeChildCommand((RCmdBase *)&cmd);
}

JNIEXPORT void JNICALL
FUNC_NAME(mousePressImpl) (JNIEnv *env,
					jclass class,
					jint buttonMask) {
    RCmdButton cmd;

    DTRACE_PRINTLN1("MRobotPeer.mousePressImpl(%x)", buttonMask);

    cmd.code = RCMD_BPRESS;
    cmd.buttonMask = buttonMask;
    robot_writeChildCommand((RCmdBase *)&cmd);
}

JNIEXPORT void JNICALL
FUNC_NAME(mouseReleaseImpl) (JNIEnv *env,
					     jclass class,
					     jint buttonMask) {
    RCmdButton cmd;

    DTRACE_PRINTLN1("MRobotPeer.mouseReleaseImpl(%x)", buttonMask);

    cmd.code = RCMD_BRELEASE;
    cmd.buttonMask = buttonMask;
    robot_writeChildCommand((RCmdBase *)&cmd);
}

JNIEXPORT void JNICALL
FUNC_NAME(mouseWheelImpl) (JNIEnv *env,
					     jclass class,
					     jint wheelAmt) {
    RCmdWheel cmd;

    DTRACE_PRINTLN1("MRobotPeer.mouseWheelImpl(%x)", wheelAmt);

    cmd.code = RCMD_WHEEL;
    cmd.wheelAmt = wheelAmt;
    robot_writeChildCommand((RCmdBase *)&cmd);
}
