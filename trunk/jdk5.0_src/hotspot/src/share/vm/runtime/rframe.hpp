#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)rframe.hpp	1.17 03/12/23 16:44:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// rframes ("recompiler frames") decorate stack frames with some extra information
// needed by the recompiler.  The recompiler views the stack (at the time of recompilation) 
// as a list of rframes.

class RFrame : public ResourceObj {
 protected:
  const frame _fr;                  // my frame
  JavaThread* const _thread;        // thread where frame resides.
  RFrame* _caller;                  // caller / callee rframes (or NULL)
  RFrame*const _callee;
  const int _num;                   // stack frame number (0 = most recent)
  int _invocations;                 // current invocation estimate (for this frame)
                                    // (i.e., how often was this frame called)
  int _distance;                    // recompilation search "distance" (measured in # of interpreted frames)

  RFrame(frame fr, JavaThread* thread, RFrame*const callee);
  virtual void init() = 0;          // compute invocations, loopDepth, etc.
  void print(const char* name);

 public:

  static RFrame* new_RFrame(frame fr, JavaThread* thread, RFrame*const callee);

  virtual bool is_interpreted() const     { return false; }
  virtual bool is_compiled() const        { return false; }
  int distance() const                    { return _distance; }
  void set_distance(int d);
  int invocations() const                 { return _invocations; }
  int num() const                         { return _num; }
  frame fr() const                        { return _fr; }
  JavaThread* thread() const              { return _thread; }
  virtual int cost() const = 0;           // estimated inlining cost (size)
  virtual methodHandle top_method() const  = 0;
  virtual javaVFrame* top_vframe() const = 0;
  virtual nmethod* nm() const             { ShouldNotCallThis(); return NULL; }

  RFrame* caller();
  RFrame* callee() const                  { return _callee; }
  RFrame* parent() const;                 // rframe containing lexical scope (if any)
  virtual void print()                    = 0;

  static int computeSends(methodOop m);
  static int computeSends(nmethod* nm);
  static int computeCumulSends(methodOop m);
  static int computeCumulSends(nmethod* nm);
};

class CompiledRFrame : public RFrame {    // frame containing a compiled method
 protected:
  nmethod*    _nm;
  javaVFrame* _vf;                        // top vframe; may be NULL (for most recent frame)
  methodHandle _method;                   // top method

  CompiledRFrame(frame fr, JavaThread* thread, RFrame*const  callee);
  void init();
  friend class RFrame;

 public:
  CompiledRFrame(frame fr, JavaThread* thread); // for nmethod triggering its counter (callee == NULL)
  bool is_compiled() const                 { return true; }
  methodHandle top_method() const          { return _method; }
  javaVFrame* top_vframe() const           { return _vf; }
  nmethod* nm() const                      { return _nm; }
  int cost() const;
  void print();
};

class InterpretedRFrame : public RFrame {    // interpreter frame
 protected:
  javaVFrame* _vf;                           // may be NULL (for most recent frame)
  methodHandle   _method;
 
  InterpretedRFrame(frame fr, JavaThread* thread, RFrame*const  callee);
  void init();
  friend class RFrame;

 public:
  InterpretedRFrame(frame fr, JavaThread* thread, methodHandle m); // constructor for method triggering its invocation counter
  bool is_interpreted() const                { return true; }
  methodHandle top_method() const            { return _method; }
  javaVFrame* top_vframe() const             { return _vf; }
  int cost() const;
  void print();
};

// treat deoptimized frames as interpreted
class DeoptimizedRFrame : public InterpretedRFrame {
 protected: 
  DeoptimizedRFrame(frame fr, JavaThread* thread, RFrame*const  callee);
  friend class RFrame;
 public:
  void print();
};


