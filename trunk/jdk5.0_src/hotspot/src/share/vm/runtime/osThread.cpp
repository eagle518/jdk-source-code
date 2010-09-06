#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)osThread.cpp	1.22 03/12/23 16:44:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
void OSThread::print() {
  tty->print("nid=0x%lx ", thread_id());
  switch (get_state()) {
    case ALLOCATED:               tty->print("allocated ");                 break;
    case INITIALIZED:             tty->print("initialized ");               break;
    case RUNNABLE:                tty->print("runnable ");                  break;
    case MONITOR_WAIT:            tty->print("waiting for monitor entry "); break;
    case CONDVAR_WAIT:            tty->print("waiting on condition ");      break;
    case OBJECT_WAIT:             tty->print("in Object.wait() ");          break;
    case BREAKPOINTED:            tty->print("at breakpoint");        	    break;
    case SLEEPING:            	  tty->print("sleeping");        	    break;
    case ZOMBIE:            	  tty->print("zombie");        	    	    break;
    default:                      tty->print("unknown state %d ", get_state()); break;
  }
}

