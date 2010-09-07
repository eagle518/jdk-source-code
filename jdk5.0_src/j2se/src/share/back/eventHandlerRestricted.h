/*
 * @(#)eventHandlerRestricted.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_EVENTHANDLERRESTRICTED_H
#define JDWP_EVENTHANDLERRESTRICTED_H

/**
 * eventHandler functionality restricted to use only by it's
 * component - eventFilter.
 */

typedef jboolean (*IteratorFunction)(JNIEnv *env,
                                     HandlerNode *node, 
                                     void *arg);
jboolean eventHandlerRestricted_iterator(EventIndex ei,
                              IteratorFunction func, void *arg);

/* HandlerNode data has three components:
 *    public info                (HandlerNode)  as declared in eventHandler.h
 *    eventHandler private data  (EventHandlerPrivate_Data) as declared below
 *    eventFilter private data   declared privately in eventFilter.c
 *
 * These three components are stored sequentially within the node.
 */

/* this is HandlerNode PRIVATE data  --
 * present in this .h file only for defining EventHandlerRestricted_HandlerNode
 */
typedef struct EventHandlerPrivate_Data_ {
    struct HandlerNode_      *private_next;
    struct HandlerNode_      *private_prev;
    struct HandlerChain_     *private_chain;
    HandlerFunction private_handlerFunction;
} EventHandlerPrivate_Data;

/* this structure should only be used outside of eventHandler
 * for proper address computation
 */
typedef struct EventHandlerRestricted_HandlerNode_ {
    HandlerNode                 hn;
    EventHandlerPrivate_Data    private_ehpd;
} EventHandlerRestricted_HandlerNode;

#endif

