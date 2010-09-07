/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef _DISPATCHER_
#define _DISPATCHER_

#include "Handler.hpp"

/** This class understands the commands supported by the system and
    calls the appropriate handler routines. */

class Dispatcher {
public:
  static void dispatch(char* cmd, Handler* handler);
};

#endif  // #defined _DISPATCHER_
