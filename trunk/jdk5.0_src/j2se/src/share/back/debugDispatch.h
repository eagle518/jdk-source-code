/*
 * @(#)debugDispatch.h	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_DEBUGDISPATCH_H
#define JDWP_DEBUGDISPATCH_H

/*
 * Type of all command handler functions. First argument is the 
 * input stream. Second argument is the output sent back to the 
 * originator, but only if JNI_TRUE is returned. If JNI_FALSE
 * is returned, no reply is made.
 */
struct PacketInputStream;
struct PacketOutputStream;

typedef jboolean (*CommandHandler)(struct PacketInputStream *, 
                                  struct PacketOutputStream *);
void debugDispatch_initialize(void);
void debugDispatch_reset(void);
CommandHandler debugDispatch_getHandler(int cmdSet, int cmd) ;

#endif
