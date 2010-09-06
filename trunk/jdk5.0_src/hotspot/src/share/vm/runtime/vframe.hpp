#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vframe.hpp	1.72 04/01/14 19:41:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// vframes are virtual stack frames representing source level activations.
// A single frame may hold several source level activations in the case of 
// optimized code. The debugging stored with the optimized code enables
// us to unfold a frame as a stack of vframes.
// A cVFrame represents an activation of a non-java method.

// The vframe inheritance hierarchy:
// - vframe
//   - javaVFrame
//     - interpretedVFrame
//     - compiledVFrame     ; (used for both compiled Java methods and native stubs)
//   - externalVFrame
//     - entryVFrame        ; special frame created when calling Java from C

// - BasicLock

class vframe: public ResourceObj {
 protected:
  frame        _fr;      // Raw frame behind the virtual frame.
  RegisterMap  _reg_map; // Register map for the raw frame (used to handle callee-saved registers).
  JavaThread*  _thread;  // The thread owning the raw frame.

  vframe(const frame* fr, const RegisterMap* reg_map, JavaThread* thread);
  vframe(const frame* fr, JavaThread* thread);
 public:
  // Factory method for creating vframes
  static vframe* new_vframe(const frame* f, const RegisterMap *reg_map, JavaThread* thread);
  
  // Accessors
  frame              fr()           const { return _fr;       }

// ???? Does this need to be a copy?
  frame*             frame_pointer() { return &_fr;       }
  const RegisterMap* register_map() const { return &_reg_map; }
  JavaThread*        thread()       const { return _thread;   }

  // Returns the sender vframe
  virtual vframe* sender() const;

  // Returns the next javaVFrame on the stack (skipping all other kinds of frame)
  javaVFrame *java_sender() const;

  // Answers if the this is the top vframe in the frame, i.e., if the sender vframe 
  // is in the caller frame
  virtual bool is_top() const { return true; }

  // Returns top vframe within same frame (see is_top())	
  virtual vframe* top() const;

  // Type testing operations
  virtual bool is_entry_frame()       const { return false; }  
  virtual bool is_java_frame()        const { return false; }
  virtual bool is_interpreted_frame() const { return false; }
  virtual bool is_compiled_frame()    const { return false; }

#ifndef PRODUCT
  // printing operations
  virtual void print_value() const;
  virtual void print();
#endif
};


class javaVFrame: public vframe {
 public:
  // JVM state
  virtual methodOop                    method()         const = 0;
  virtual int                          bci()            const = 0;
  virtual StackValueCollection*        locals()         const = 0;
  virtual StackValueCollection*        expressions()    const = 0;  
  // the order returned by monitors() is from oldest -> youngest#4418568
  virtual GrowableArray<MonitorInfo*>* monitors()       const = 0;

  // Debugging support via JVMDI.
  // NOTE that this is not guaranteed to give correct results for compiled vframes.
  // Deoptimize first if necessary.
  virtual void set_locals(StackValueCollection* values) const = 0;

  // Test operation 
  bool is_java_frame() const { return true; }

 protected:
  javaVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread) : vframe(fr, reg_map, thread) {}
  javaVFrame(const frame* fr, JavaThread* thread) : vframe(fr, thread) {}

 public:
  // casting
  static javaVFrame* cast(vframe* vf) {
    assert(vf == NULL || vf->is_java_frame(), "must be java frame");
    return (javaVFrame*) vf;
  }

  // fabricate heavyweight monitors for lightweight monitors
  void jvmpi_fab_heavy_monitors(bool query, int* index, int frame_count, GrowableArray<ObjectMonitor*>* fab_list);

  // printing used during stack dumps
  void print_lock_info(int frame_count);

#ifndef PRODUCT
 public:
  // printing operations
  void print();
  void print_value() const;
  void print_activation(int index) const;

  // verify operations
  virtual void verify() const;

  // Structural compare
  bool structural_compare(javaVFrame* other);
#endif
  friend class vframe;
};

class interpretedVFrame: public javaVFrame {
 public:
  // JVM state
  methodOop                    method()         const;
  int                          bci()            const;
  StackValueCollection*        locals()         const;
  StackValueCollection*        expressions()    const;
  GrowableArray<MonitorInfo*>* monitors()       const;

  void set_locals(StackValueCollection* values) const;

  // Test operation
  bool is_interpreted_frame() const { return true; }

 protected:
  interpretedVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread) : javaVFrame(fr, reg_map, thread) {};

 public:
  // Accessors for Byte Code Pointer
  u_char* bcp() const;
  void set_bcp(u_char* bcp);

  // casting
  static interpretedVFrame* cast(vframe* vf) {
    assert(vf == NULL || vf->is_interpreted_frame(), "must be interpreted frame");
    return (interpretedVFrame*) vf;
  }

 private:
  static const int bcp_offset;
  intptr_t* locals_addr_at(int offset) const;
  intptr_t* expression_stack_addr_at(int offset) const;

  // returns where the parameters starts relative to the frame pointer
  int start_of_parameters() const;
  
#ifndef PRODUCT
 public:
  // verify operations
  void verify() const;
#endif
  friend class vframe;
};


class externalVFrame: public vframe {
 protected:
  externalVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread) : vframe(fr, reg_map, thread) {}

#ifndef PRODUCT
 public:
  // printing operations
  void print_value() const;
  void print();
#endif
  friend class vframe;
};

class entryVFrame: public externalVFrame {
 public:
  bool is_entry_frame() const { return true; }

 protected:
  entryVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread);

 public:
  // casting
  static entryVFrame* cast(vframe* vf) {
    assert(vf == NULL || vf->is_entry_frame(), "must be entry frame");
    return (entryVFrame*) vf;
  }

#ifndef PRODUCT
 public:
  // printing 
  void print_value() const;
  void print();
#endif
  friend class vframe;
};


// A MonitorInfo is a ResourceObject that describes a the pair:
// 1) the owner of the monitor
// 2) the monitor lock
class MonitorInfo : public ResourceObj {
 private:
  oop        _owner; // the object owning the monitor
  BasicLock* _lock;
 public:
  // Constructor
  MonitorInfo(oop owner, BasicLock* lock) {
    _owner = owner;
    _lock  = lock;
  }
  // Accessors
  oop        owner() const { return _owner; }
  BasicLock* lock()  const { return _lock;  }
};

class vframeStreamCommon : StackObj {
 protected:
  // common 
  frame        _frame;
  JavaThread*  _thread;
  RegisterMap  _reg_map;
  enum { interpreted_mode, compiled_mode, at_end_mode } _mode;

  // compiled
  nmethod* _code;
  int _sender_decode_offset;

  // Cached information
  methodOop _method;
  int       _bci;

  // Should VM activations be ignored or not
  bool _stop_at_java_call_stub;

#ifndef CORE
         bool fill_in_compiled_inlined_sender();
         void fill_from_compiled_frame(nmethod* code, int decode_offset);
         void fill_from_compiled_native_frame(nmethod* code);
#endif

  void fill_from_interpreter_frame();
  virtual bool fill_from_frame();

  // Helper routine for security_get_caller_frame
  void skip_method_invoke_and_aux_frames();

 public:
  // Constructor
  vframeStreamCommon(JavaThread* thread) : _reg_map(thread, false) {
    _thread = thread;
  }

  // Accessors
  methodOop method() const { return _method; }
  int bci() const { return _bci; }
  intptr_t* frame_id() const { return _frame.id(); }
  address frame_pc() const { return _frame.pc(); }

  // Frame type
  bool is_interpreted_frame() const { return _frame.is_interpreted_frame(); }
  bool is_entry_frame() const       { return _frame.is_entry_frame(); }

  // Iteration
  void next();
  bool at_end() const { return _mode == at_end_mode; }

  // Implements security traversal. Skips depth no. of frame including
  // special security frames
  void security_get_caller_frame(int depth);

  // Helper routine for JVM_LatestUserDefinedLoader -- needed for 1.4
  // reflection implementation
  void skip_reflection_related_frames();
};

class vframeStream : public vframeStreamCommon {
 public:
  // Constructors
  vframeStream(JavaThread* thread, bool stop_at_java_call_stub = false);

  // top_frame may not be at safepoint, start with sender
  vframeStream(JavaThread* thread, frame top_frame, bool stop_at_java_call_stub = false);
};
