/*
 * @(#)FrameID.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_FRAMEID_H
#define JDWP_FRAMEID_H

typedef jlong FrameID;

FrameID createFrameID(jthread thread, FrameNumber fnum);
FrameNumber getFrameNumber(FrameID frame);
jvmtiError validateFrameID(jthread thread, FrameID frame);

#endif

