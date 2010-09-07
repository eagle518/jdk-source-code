/*
 * @(#)standardHandlers.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_STANDARDHANDLERS_H
#define JDWP_STANDARDHANDLERS_H

#include "eventHandler.h"

HandlerFunction standardHandlers_defaultHandler(EventIndex ei);

void standardHandlers_onConnect(void);
void standardHandlers_onDisconnect(void);

#endif

