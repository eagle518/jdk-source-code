#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cInterpreter.cpp	1.18 03/12/23 16:40:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 *
 * This code was converted from CVM sources to C++ and the Hotspot VM
 *
 */

/*
 * Note:
 * In order to eliminate the overhead of testing JVMPI and JVMDI flags
 * during non debuging execution, we generate two version of the Interpreter.
 * The first one is generated via the dependency in the includeDB mechanism 
 * and is read in as part of the _cInterpreter.cpp.incl line below.
 *
 * The second and JVMDI/PI enabled interpreter is brought in below after
 * the line defining VM_JVMTI to 1.
 * 
 * On startup, the assembly generated to enter the Interpreter will be
 * pointed at either InterpretMethod or InterpretMethodWithChecks depending
 * on the state of the jvmpi or jvmdi flags..
 */
#undef VM_JVMPI
#undef VM_JVMTI

#include "incls/_precompiled.incl"
#include "incls/_cInterpreter.cpp.incl"

#ifdef CC_INTERP

#define VM_JVMPI 1
#define VM_JVMTI 1

// Build the Interpreter that is used if JVMDI or JVMPI are enabled
#include "cInterpretMethod.hpp"

// This constructor should only be used to contruct the object to signal
// interpreter initialization. All other instances should be created by
// the frame manager.
cInterpreter::cInterpreter(messages msg) {
  if (msg != initialize) ShouldNotReachHere(); 
  _msg = msg; 
  _self_link = this;
  _prev_link = NULL;
}

// Dummy function so we can determine if a pc is within the interpreter.
// This is really a hack. Seems like adding state to thread ala last_Java_sp, etc.
// would be cleaner.
//
void cInterpreter::End_Of_Interpreter(void) {
}

#endif
