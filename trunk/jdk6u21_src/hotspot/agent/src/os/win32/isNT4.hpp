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

#ifndef _ISNT4_H_
#define _ISNT4_H_

// We need to special-case the Windows NT 4.0 implementations of some
// of the debugging routines because the Tool Help API is not
// available on this platform.

bool isNT4();

#endif  // #defined _ISNT4_H_
