#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdiFrame.hpp	1.4 03/12/23 16:43:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMDIFRAME_H_
#define _JAVA_JVMDIFRAME_H_

// These classes are used to emulate JVMDI jframeID.
// JVMTI does not use this concept, and when JVMDI is removed
// this file should be as well.
// NEEDS_CLEANUP

// forward declaration
class JvmdiFrame;


///////////////////////////////////////////////////////////////
//
// class JvmdiCachedFrames
//
// Per thread cached JVMDI frames.
//
// Part of JvmtiThreadState.  Holds the array of frames that have been
// exported out to the JVMDI client.

class JvmdiCachedFrames VALUE_OBJ_CLASS_SPEC {
 private:
  JavaThread* _thread;
  jframeID    _event_jframeID; //current frame jframeID for event posting.

  // JvmdiFrames that have been for this thread
  GrowableArray<JvmdiFrame *>* _jvmdi_frames;

 private:
  // assure the the stack is of the specified depth (and _jvmdi_frames is filled-in)
  bool assure_depth(int depth);

  bool is_protected();

  // get_depth should only be called by the friend class.
  friend class JvmdiConvertJFrameID;
  // convert between JvmdiFrame and jframeID (true if found)
  bool get_depth(jframeID fid, jint *depth_p);


 public:
  JvmdiCachedFrames(JavaThread* thread);

  JavaThread* get_thread() { return _thread; }

  // convert a frame depth to a jframeID (false if not that deep)
  bool depth_to_jframeID(int depth, jframeID* frame_ptr);

  // cached JVMDI frames are now invalid
  void clear_cached_frames();

  // thread state is going away, destroy our data
  void destroy();
    
  // Cache the current jframeID before thread transition to native.
  void set_event_jframeID() { depth_to_jframeID(0, &_event_jframeID); }
  // Return the cached jframeID. Can be called from thread in native.
  jframeID event_jframeID()  { return _event_jframeID; }
};

///////////////////////////////////////////////////////////////
//
// class JvmdiConvertJFrameID
//
// jframeID to thread and depth conversion

class JvmdiConvertJFrameID : public AllStatic {
 public:
  // JVMTI provides both thread and depth, but JVMDI only gives jframeID - hence we need this.
  // From a jframeID, go searching to find its thread and depth.
  // Return true for success
  static bool get_thread_and_depth(jframeID fid, JavaThread **jt_p, jint *depth_p); 
};


#endif   /* _JAVA_JVMDIFRAME_H_ */
