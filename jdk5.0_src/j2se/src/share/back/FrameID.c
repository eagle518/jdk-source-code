/*
 * @(#)FrameID.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "FrameID.h"
#include "threadControl.h"

/* FrameID: */

/* ------------------------------------------------------------ */
/* | thread frame generation (48 bits)| frame number (16 bits)| */
/* ------------------------------------------------------------ */

#define FNUM_BWIDTH 16
#define FNUM_BMASK ((1<<FNUM_BWIDTH)-1)

FrameID 
createFrameID(jthread thread, FrameNumber fnum)
{
    FrameID frame;
    jlong frameGeneration;
    
    frameGeneration = threadControl_getFrameGeneration(thread);
    frame = (frameGeneration<<FNUM_BWIDTH) | (jlong)fnum;
    return frame;
}

FrameNumber 
getFrameNumber(FrameID frame)
{
    /*LINTED*/
    return (FrameNumber)(((jint)frame) & FNUM_BMASK);
}

jvmtiError 
validateFrameID(jthread thread, FrameID frame)
{
    jlong frameGeneration;
    
    frameGeneration = threadControl_getFrameGeneration(thread);
    if ( frameGeneration != (frame>>FNUM_BWIDTH)  ) {
	return JVMTI_ERROR_INTERNAL;
    }
    return JVMTI_ERROR_NONE;
}

