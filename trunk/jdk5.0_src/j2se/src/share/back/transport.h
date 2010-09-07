/*
 * @(#)transport.h	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_TRANSPORT_H
#define JDWP_TRANSPORT_H

#include "jdwpTransport.h"

void transport_initialize(void);
void transport_reset(void);
jdwpError transport_startTransport(jboolean isServer, char *name, char *address, long timeout);

jint transport_receivePacket(jdwpPacket *);
jint transport_sendPacket(jdwpPacket *);
jboolean transport_is_open(void);
void transport_waitForConnection(void);
void transport_close(void);

#endif

