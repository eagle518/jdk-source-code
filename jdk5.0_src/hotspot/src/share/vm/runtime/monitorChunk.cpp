#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)monitorChunk.cpp	1.13 03/12/23 16:43:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_monitorChunk.cpp.incl"

MonitorChunk::MonitorChunk(int number_on_monitors) {
  _number_of_monitors = number_on_monitors;
  _monitors           = NEW_C_HEAP_ARRAY(BasicObjectLock, number_on_monitors);
  _next               = NULL;
}


MonitorChunk::~MonitorChunk() {
  FreeHeap(monitors());
}


void MonitorChunk::oops_do(OopClosure* f) {
  for (int index = 0; index < number_of_monitors(); index++) {
    at(index)->oops_do(f);
  }
}

