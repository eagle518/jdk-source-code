/*
 * @(#)robot_common.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdarg.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "robot_common.h"

const char * ROBOT_ARG0 = "SunAwtRobot";

/*
 * The robot child process doens't link in the DTRACE_ functionality
 * so we define (sigh) our own debug print function
 */
void robot_traceln(const char * format, ...) {
#if defined(DEBUG)    
    va_list	args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
#endif    
}

/*
 * robot_writeBytes - Write a sequence of bytes through the pipe, doing
 *     careful checking to ensure that everything is written
 *     and that errors are indicated properly.
 *
 *
 * This method (and robot_readBytes below) provide a means to communicate
 * data and commands over a pipe.    For the Motif Robot implementation,
 * a child process is used to do all commands to the X server to
 * avoid locking bugs in the AWT (DnD) implementation, as noted above.
 *
 * This method is called from either parent code (to send commands
 * to the child) or child code (to send result data back).
 *
 * role:    A string indicating which role the calling code is fulfilling.
 *        Used only in debugging.
 *
 * filedes: The file to send over.    For parent->child use filedes[0].
 *        For child->parent use filedes[1].    See the wRCmdToChild and wrToParent
 *        methods below.    Quoting from pipe(2):
 *
 *     A read from fildes[0] accesses the data written to fildes[1]
 *     on    a    first-in-first-out    (FIFO)    basis    and    a    read    from
 *     fildes[1] accesses the data written to fildes[0] also    on    a
 *     FIFO basis.
 *
 * bytes:    The bytes to send.
 * nBytesToWrite:    The number of bytes to send.
 *
 * Return value:    0 is okay, -1 an error occurred.
 */
int32_t robot_writeBytes(const char *role, int32_t filedes, char *bytes,
                         int32_t nBytesToWrite) {

    ssize_t result;
    ssize_t offset = 0;

    robot_traceln("%s: writing to fd %d, %d bytes", role, filedes, nBytesToWrite);
    while (nBytesToWrite > 0) {
        robot_traceln("%s: writing %d bytes", role, nBytesToWrite);
        result = write(filedes, bytes + offset, nBytesToWrite);

        if (result > 0) {
            if (result >= nBytesToWrite) {
                return 0;
            }
            robot_traceln("%s: wrote only %d bytes, looping", role, result);
            offset += result;
            nBytesToWrite -= result;
        } else {
            /*
             * If either of these errors happen, then something
             * bad has happened to the child.
             * Need to make a new child and reset the packet.
             */
            switch (errno) {
            case EBADF: 
                robot_traceln("%s: failed EBADF", role);
                return -1;

            case EPIPE:
                robot_traceln("%s: failed EPIPE", role);
                return -1;

            default:
                robot_traceln("%s: failed %d", role, result);
                return 0;
            }
        }
    }

    return 0;
}

/*
 * robot_readBytes - Complementing robot_writeBytes, read the given number of bytes
 *        from the pipe.
 *
 * This method is called from either child code (to read commands
 * from the parent) or parent code (to read results sent back).
 *
 * role:    A string indicating which role the calling code is fulfilling.
 *        Used only in debugging.
 *
 * filedes: The file to send over.    For parent->child use filedes[0].
 *        For child->parent use filedes[1].    See the wRCmdToChild and wrToParent
 *        methods below.    Quoting from pipe(2):
 *
 *     A read from fildes[0] accesses the data written to fildes[1]
 *     on    a    first-in-first-out    (FIFO)    basis    and    a    read    from
 *     fildes[1] accesses the data written to fildes[0] also    on    a
 *     FIFO basis.
 *
 * bytes:    The bytes to read.
 * nToRead:    The number of bytes to read.
 *
 * Return value:    0 is okay, -1 an error occurred.
 */
int32_t robot_readBytes(const char *role, int32_t filedes, char *bytes,
                        size_t nToRead) {
    ssize_t nRead;
    ssize_t offset;
    int32_t code;

    nRead = -1;
    offset = 0;
    code = 0;
    
    memset(bytes, 0, nToRead);

readAgain:
    robot_traceln("%s: about to read fd %d, %d bytes", role, filedes, nToRead);
    
    if( robot_pollForRead(filedes, -1) == RPOLL_ERROR) {
        return -1;
    }
    nRead = read(filedes, bytes + offset, nToRead);
    if (nRead == 0) {
        robot_traceln("%s: no bytes read, exiting", role);
        return -1;
    }

    if (nRead < 0) {
        /* Some error happened.    However if EINTR is set
         * all that happened is a signal interrupted
         * the system call.    In that case there's nothing
         * significant wrong and we should just try
         * the read again.
         */
        if (errno == EINTR) {
            robot_traceln("%s: read gave EINTR", role);
            goto readAgain;
        }
        else {
            robot_traceln("%s: read gave other error, exiting", role);
            return -1;
        }
    }

    if ((size_t)nRead < nToRead) {
        /* The full amount was not read.    Bump by the
         * amount read and try again. */
        robot_traceln("%s: read %d wanted %d", role, nRead, nToRead);
        offset += nRead;
        nToRead -= (size_t) nRead;
        if (nToRead > 0)
            goto readAgain;
    }

    return 0;
}

/*
 * Flushes any data pending on the pipe
 */
void robot_readFlush(int32_t filedes) {
    char	buf[128];
    ssize_t	nRead;

    robot_traceln("robot_readFlush : flushing pipe");
    do {
        if( robot_pollForRead(filedes, -1) == RPOLL_ERROR) {
            return;
        }
        nRead = read(filedes, buf, 128);
    } while (nRead > 0);
}

/*
 * Waits until there is data to be read
 */
RPollResult robot_pollForRead(int32_t fd, int32_t timeout) {
    struct pollfd fdList[10];
    int32_t result;

    memset(&fdList, 0, sizeof(fdList));
    fdList[0].fd = fd;
    fdList[0].events = POLLIN | POLLRDNORM;
    fdList[0].revents = 0;

    /*
     * Using select, cause the child to wait until either
     * 1) Some data is written to the pipe
     * 2) An exception happens to the pipe
     * 3) Timeout (so we can check that the parent is still there)
     */
    for(;;) {
        result = poll(fdList, 1, timeout);

        if (result < 0) {
            /*
             * An error happened.    Depending on errno we can
             * act different ways as follows.
             *
             * EINTR - we got a signal and the system call was
             *        interrupted.    Nothing serious.    Just loop back.
             * Any other error - Something serious happened.    We just
             *        exit as there is nothing further we know how to do.
             */
            if (errno == EINTR || errno == EAGAIN) {
                robot_traceln("robot_pollForRead: EINTR or EAGAIN on select");
                continue;
            } else {
                robot_traceln("robot_pollForRead: other error on select, exiting");
                return RPOLL_ERROR;
            }
        } else if (result == 0) {
            return RPOLL_TIMEOUT;
        } else if (result > 0) {
            /*
             * Otherwise we have valid event on the file descriptor.
             * Either it's the "read" or "exception" list since we're
             * not doing any writing.
             *
             * Check the "exception" first to see if something is
             * wrong with the pipe.
             */
            if ((fdList[0].revents & (POLLERR | POLLHUP)) != 0) {
                robot_traceln("robot_pollForRead: select exception indicator, exiting");
                return RPOLL_ERROR;
            }
            return RPOLL_UNBLOCKED;
        }
    }
}

int32_t robot_isCommandValid(int32_t code) {
    return code >= RCMD_BEGIN && code <= RCMD_END;
}

/* QueryColorMap is taken from multiVis.c, part of the xwd distribution from
 * X.org.  It used to live in robot_child/multiVis.c, but was moved here so
 * it can be shared with awt_DataTransferer.c
 */
int32_t
QueryColorMap(Display *disp,
              Colormap src_cmap,
              Visual *src_vis,
              XColor **src_colors,
              int32_t *rShift, int32_t *gShift, int32_t *bShift)
              
{
     int32_t ncolors,i ;
     unsigned long redMask, greenMask, blueMask;
     int32_t                 redShift, greenShift, blueShift;
     XColor *colors ;

     ncolors = src_vis->map_entries ;
     *src_colors = colors = (XColor *)calloc(ncolors,sizeof(XColor) ) ;

     if(src_vis->class != TrueColor && src_vis->class != DirectColor)
     {
         for(i=0 ; i < ncolors ; i++)
         {
                colors[i].pixel = i ;
                colors[i].pad = 0;
                colors[i].flags = DoRed|DoGreen|DoBlue;
         }
     }
     else /** src is decomposed rgb ***/
     {
        /* Get the X colormap */
        redMask = src_vis->red_mask;
        greenMask = src_vis->green_mask;
        blueMask = src_vis->blue_mask;
        redShift = 0; while (!(redMask&0x1)) {
                redShift++;
                redMask = redMask>>1;
        }
        greenShift = 0; while (!(greenMask&0x1)) {
                greenShift++;
                greenMask = greenMask>>1;
        }
        blueShift = 0; while (!(blueMask&0x1)) {
                blueShift++;
                blueMask = blueMask>>1;
        }
        *rShift = redShift ;
        *gShift = greenShift ;
        *bShift = blueShift ;
        for (i=0; i<ncolors; i++) {
                if( i <= redMask)colors[i].pixel = (i<<redShift) ;
                if( i <= greenMask)colors[i].pixel |= (i<<greenShift) ;
                if( i <= blueMask)colors[i].pixel |= (i<<blueShift) ;
                /***** example :for gecko's 3-3-2 map, blue index should be <= 3
.
                colors[i].pixel = (i<<redShift)|(i<<greenShift)|(i<<blueShift);
                *****/
                colors[i].pad = 0;
                colors[i].flags = DoRed|DoGreen|DoBlue;
        }
      }

      XQueryColors(disp, src_cmap, colors, ncolors);
      return ncolors ;
}

