#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdiFrame.cpp	1.12 03/12/23 16:43:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdiFrame.cpp.incl"


// These classes are used to emulate JVMDI jframeID.
// JVMTI does not use this concept, and when JVMDI is removed
// this file should be as well.
// NEEDS_CLEANUP

// JVMDI calls cannot usually examine the frames in the stack of a running JavaThread, 
// except under the following scenarios:
//
// 1. The thread has been externally suspended by the JVMDI.
// 2. The thread has called (and is still inside) the JVMDI event hook function.
// 3. The thread is in a native method and examination is occurring from within that native method.
// 
// In these cases the frames of the thread are examinable and modifiable using JVMDI calls.
// Threads which JVMDI can examine I'll call 'walkable.'
//
// JVMDI refers to a frame by a pointer sized jframeID value and
// gives the implementor full choice about what
// it represents. jframeID's are valid only as long as the thread they refer to remains walkable.
// Once a thread leaves the walkable state all its jframeID's become stale.
//
// The calls in JVMDI (GetFrameLocation) introduce another wrinkle, by requiring that a jframeID 
// map directly to its thread.
//
// This leads to the following implementation possibilities:
//
// 1. The simplest implementation of jframeID would be the frame's SP. However, this leaves no way to
// detect stale jframeID's or (for hotspot) easily map back to the appropriate thread.
//
// 2. Another simple implementation would use a pointer to a
// JvmdiFrame object, which could be marked stale,
// but would need to be collected at some point (this is the approach used by classic).
// Once the JvmdiFrame object
// was collected you would lose the ability to detect a pointer to it as stale.
// By dereferencing a stale pointer
// you might introduce bugs that show up much later and are difficult to detect.
//
// 3. A more complex alternative can detect stale jframeID's
// which in the presence of buggy JVMDI user code might help licensee's more. 
// A sequence number is used as the jframeID. This makes the
// jframeID unique for a particular frame. The JvmdiFrame
// object is used as a wrapper around the vframe.
// It also contains a "magic" value to make it
// easier to detect stale JvmdiFrames.
//
// We use (3).
//


///////////////////////////////////////////////////////////////
//
// class JvmdiFrameUtil - definition
//
// Utility routines for JvmdiFrame/jframeID

class JvmdiFrameUtil : public AllStatic {
 private:
  static jframeID _last_jframeID;
    
 public:
  static jframeID next_jframeID();

  // helper method to find JvmdiFrame that contains fp
  static bool equalJFrameID(void* fid, JvmdiFrame *jvmti_frame);

  static jframeID invalid_jframeID()            { return (jframeID)-1; }
  static bool is_valid(jframeID fid)            { return (intptr_t)fid >= 0; }
};


///////////////////////////////////////////////////////////////
//
// class JvmdiFrame

class JvmdiFrame : public CHeapObj {
 private:
  // Depth of 0 indicates the topmost frame on the stack
  jint          _depth;
  jframeID      _frameID;

 public:
  JvmdiFrame(int depth, jframeID fid)   {
    assert(depth >= 0, "depth >= 0");

    _depth  = depth;
    _frameID = fid;
  }
  ~JvmdiFrame()            { _frameID = JvmdiFrameUtil::invalid_jframeID(); }
  jint get_depth()         { return _depth; }
  jframeID get_jframeID()  { return _frameID; }
  bool is_valid()          { return JvmdiFrameUtil::is_valid(_frameID); }
};


///////////////////////////////////////////////////////////////
//
// class JvmdiFrameUtil - implementation

jframeID JvmdiFrameUtil::_last_jframeID = 0;


jframeID JvmdiFrameUtil::next_jframeID() {
  // More than one thread could be incrementing jframeID counter
  // _last_jframeID so hold JvmdiFrame_lock before incrementing it.    
  MutexLocker mu(JvmdiFrame_lock);
  jint next = 1 + (intptr_t)(_last_jframeID);
  if (next < 0) {
    next = 1;
  }
  _last_jframeID = (jframeID)next;
  return _last_jframeID;
}


bool JvmdiFrameUtil::equalJFrameID(void* fid, JvmdiFrame *jvmdi_frame) {
  jframeID f1 = (jframeID)fid;
  jframeID f2 = jvmdi_frame->get_jframeID();
  return f1 == f2;
}


///////////////////////////////////////////////////////////////
//
// class JvmdiCachedFrames

JvmdiCachedFrames::JvmdiCachedFrames(JavaThread* thread) {
  _thread = thread;
  _jvmdi_frames = NULL;
}


// All access or modification to the cached frames must be protected, either
//    by the target thread being the current thread or
//    by the target thread being suspended and the current thread holding
//    the JvmdiCachedFrame_lock 
// Test for use by asserts
bool JvmdiCachedFrames::is_protected() {
  return JavaThread::current() == get_thread() ||
         (get_thread()->is_being_ext_suspended() && JvmdiCachedFrame_lock->is_locked());
}


bool JvmdiCachedFrames::get_depth(jframeID fid, jint *depth_p) {
  assert(is_protected(), "Not protected");

  if (_jvmdi_frames == NULL) {
    return false;
  }
  int index = _jvmdi_frames->find(fid, JvmdiFrameUtil::equalJFrameID);
  if (index == -1) {
    return false;
  }
  JvmdiFrame *jvmdi_frame = _jvmdi_frames->at(index);
  assert(jvmdi_frame->is_valid(), "sanity check");
  *depth_p = jvmdi_frame->get_depth();
  return true;
}


bool JvmdiCachedFrames::assure_depth(int depth) {
  assert(is_protected(), "Not protected");

  if (!get_thread()->has_last_Java_frame()) {
    return false;
  }
  if (_jvmdi_frames == NULL) {
    _jvmdi_frames = new (ResourceObj::C_HEAP) GrowableArray<JvmdiFrame *>(5,true);
  }
  int initial_length = _jvmdi_frames->length();
  if (initial_length > depth) {
    return true;
  }
  ResourceMark rm;
  RegisterMap reg_map(get_thread());
  vframe *vf = get_thread()->last_java_vframe(&reg_map);
  int d = 0;
  while (true) {
    if (vf == NULL) {
      return false;
    }
    if (d >= initial_length) {
      JvmdiFrame *jf = new JvmdiFrame(d, JvmdiFrameUtil::next_jframeID());
      _jvmdi_frames->append(jf);
    }
    if (d >= depth) {
      return true;
    }
    vf = vf->java_sender();
    d++;
  }
}


// return true and the jframeID if there is a frame at the specified depth
bool JvmdiCachedFrames::depth_to_jframeID(int depth, jframeID* frame_ptr) {
  MutexLockerEx ml(get_thread() == Thread::current() ? NULL : JvmdiCachedFrame_lock);
  if (assure_depth(depth)) {
    JvmdiFrame *jvmdi_frame = _jvmdi_frames->at(depth);
    assert(jvmdi_frame->is_valid(), "sanity check");
    *frame_ptr = (jframeID)jvmdi_frame->get_jframeID();
    return true;
  } else {
    return false;
  }
}


void JvmdiCachedFrames::clear_cached_frames() {
  MutexLockerEx ml(get_thread() == Thread::current() ? NULL : JvmdiCachedFrame_lock);
  assert(is_protected(), "Not protected");
  if (_jvmdi_frames != NULL) {
    int len = _jvmdi_frames->length();
    for (int i=0; i< len; i++) {
      delete _jvmdi_frames->at(i);
    }
    _jvmdi_frames->clear();
  }
}


void JvmdiCachedFrames::destroy() {
  assert(JavaThread::current() == get_thread(), "Not protected");
  if (_jvmdi_frames != NULL) {
    clear_cached_frames();
    _jvmdi_frames->clear_and_deallocate();
    FreeHeap(_jvmdi_frames);
    _jvmdi_frames = NULL;
  }
  _thread = NULL;
}


///////////////////////////////////////////////////////////////
//
// class JvmdiConvertJFrameID

// search thru all suspended threads for a jframeID that matchs
bool JvmdiConvertJFrameID::get_thread_and_depth(jframeID fid, JavaThread **jt_p, jint *depth_p) {
  // try the current thread first
  JvmtiThreadState *state = JavaThread::current()->jvmti_thread_state();
  if (state != NULL) {
    if (state->jvmdi_cached_frames()->get_depth(fid, depth_p)) {
      *jt_p = state->get_thread();
      return true;
    }
  }
  MutexLocker mu(JvmtiThreadState_lock);
  for (state = JvmtiThreadState::first(); state != NULL; state = state->next()) {
    uint32_t debug_bits = 0;
    // only suspended threads can hold valid jframeIDs
    if (JvmtiEnv::is_thread_fully_suspended(state->get_thread(), true, &debug_bits)) {
      MutexLocker ml(JvmdiCachedFrame_lock);
      if (state->jvmdi_cached_frames()->get_depth(fid, depth_p)) {
        *jt_p = state->get_thread();
        return true;
      }
    }
  }
  return false;
}
