#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadCritical.hpp	1.7 03/12/23 16:44:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ThreadCritical is used to protect short non-blocking critical sections.
// This class must use no vm facilities that require initialization.
// It is used very early in the vm's initialization, in allocation
// code and other areas. ThreadCritical regions are reentrant.
//
// Due to race conditions during vm exit, some of the os level
// synchronization primitives may not be deallocated at exit. It
// is a good plan to implement the platform dependant sections of
// code with resources that are recoverable during process
// cleanup by the os. Calling the initialize method before use
// is also problematic, it is best to use preinitialized primitives
// if possible. As an example:
//
// mutex_t  mp  =  DEFAULTMUTEX;
//
// Also note that this class is declared as a StackObj to enforce
// block structured short locks. It cannot be declared a ResourceObj
// or CHeapObj, due to initialization issues.

class ThreadCritical : public StackObj {
 friend class os;
 private:
  static void initialize();
  static void release();

 public:
  ThreadCritical();
  ~ThreadCritical();
};


