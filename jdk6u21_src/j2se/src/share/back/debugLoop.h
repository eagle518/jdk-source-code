/*
 * @(#)debugLoop.h	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_DEBUGLOOP_H
#define JDWP_DEBUGLOOP_H

void debugLoop_initialize(void);
void debugLoop_run(void);
void debugLoop_sync(void);

#endif
