/*
 * Copyright (c) 2000, 2001, Oracle and/or its affiliates. All rights reserved.
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

#ifndef _IO_UTILS_
#define _IO_UTILS_

bool scanInt(char** data, int* num);
bool scanUnsignedLong(char** data, unsigned long* num);
bool scanAddress(char** data, unsigned long* addr);

// Binary utils (for poke)
bool scanAndSkipBinEscapeChar(char** data);
bool scanBinUnsignedLong(char** data, unsigned long* num);

#endif  // #defined _IO_UTILS_
