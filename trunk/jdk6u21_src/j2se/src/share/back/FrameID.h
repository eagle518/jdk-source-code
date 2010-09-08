/*
 * @(#)FrameID.h	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_FRAMEID_H
#define JDWP_FRAMEID_H

typedef jlong FrameID;

FrameID createFrameID(jthread thread, FrameNumber fnum);
FrameNumber getFrameNumber(FrameID frame);
jdwpError validateFrameID(jthread thread, FrameID frame);

#endif

