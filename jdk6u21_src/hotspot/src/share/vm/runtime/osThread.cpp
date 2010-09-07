/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_osThread.cpp.incl"


OSThread::OSThread(OSThreadStartFunc start_proc, void* start_parm) {
  pd_initialize();
  set_start_proc(start_proc);
  set_start_parm(start_parm);
  set_interrupted(false);
}

OSThread::~OSThread() {
  pd_destroy();
}

// Printing
void OSThread::print_on(outputStream *st) const {
  st->print("nid=0x%lx ", thread_id());
  switch (_state) {
    case ALLOCATED:               st->print("allocated ");                 break;
    case INITIALIZED:             st->print("initialized ");               break;
    case RUNNABLE:                st->print("runnable ");                  break;
    case MONITOR_WAIT:            st->print("waiting for monitor entry "); break;
    case CONDVAR_WAIT:            st->print("waiting on condition ");      break;
    case OBJECT_WAIT:             st->print("in Object.wait() ");          break;
    case BREAKPOINTED:            st->print("at breakpoint");               break;
    case SLEEPING:                st->print("sleeping");                    break;
    case ZOMBIE:                  st->print("zombie");                      break;
    default:                      st->print("unknown state %d", _state); break;
  }
}
