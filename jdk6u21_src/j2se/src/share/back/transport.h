/*
 * @(#)transport.h	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

