/*
 * @(#)standardHandlers.h	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_STANDARDHANDLERS_H
#define JDWP_STANDARDHANDLERS_H

#include "eventHandler.h"

HandlerFunction standardHandlers_defaultHandler(EventIndex ei);

void standardHandlers_onConnect(void);
void standardHandlers_onDisconnect(void);

#endif

