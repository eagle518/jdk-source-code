#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)codeBlob.hpp	1.104 04/03/17 12:08:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// CodeBlob - superclass for all entries in the CodeCache.
//
// Suptypes are:
//   nmethod            : Compiled Java methods (include method that calls to native code)
//   RuntimeStub        : Call to VM runtime methods
//   C2IAdapter         : Call to interpreter from compiled code (convert calling convention)
//   I2CAdapter         : Call to compiled code from interpreter (convert calling convention)
//   DeoptimizationBlob : Used for deoptimizatation
//   ExceptionBlob      : Used for stack unrolling
//   SafepointBlob      : Used to handle illegal instruction exceptions
//
//
// Layout:
//   - header
//   - relocation
//   - instruction space
//   - data space
class DeoptimizationBlob;

class CodeBlob VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  const char* _name;
  int        _size;                              // total size of CodeBlob in bytes
  int        _header_size;                       // size of header (depends on subclass)
  int        _relocation_size;                   // size of relocation
  int        _instructions_offset;               // offset to where instructions region begins
  int        _data_offset;                       // offset to where data region begins
  int        _oops_offset;                       // offset to where embedded oop table begins (inside data)
  int        _oops_length;                       // number of embedded oops
  int        _frame_size;                        // size of stack frame
#ifdef COMPILER2
  int        _link_offset;                       // offset to frame ptr; Intel only (sigh)
#endif // COMPILER2
  OopMapSet* _oop_maps;                          // OopMap for this CodeBlob

  friend class OopRecorder;

 public:
  // Returns the space needed for CodeBlob 
  static unsigned int allocation_size(CodeBuffer* cb, int header_size, int oop_size = 0);
  
  // Creation
  // a) simple CodeBlob
  CodeBlob(const char* name, int header_size, int size);

  // b) full CodeBlob
  CodeBlob(
    const char* name,
    CodeBuffer* cb,
    int         header_size,
    int         size,
    int         frame_size,
    OopMapSet*  oop_maps,
    int         oop_size = 0
  );

  // Deletion
  void flush();

  // Typing
  virtual bool is_buffer_blob() const            { return false; }
  virtual bool is_nmethod() const                { return false; }
  virtual bool is_runtime_stub() const           { return false; }
  virtual bool is_i2c_adapter() const            { return false; }
  virtual bool is_c2i_adapter() const            { return false; }
  virtual bool is_osr_adapter() const            { return false; }
  virtual bool is_deoptimization_stub() const    { return false; }  
  virtual bool is_uncommon_trap_stub() const     { return false; }
  virtual bool is_exception_stub() const         { return false; }
  virtual bool is_safepoint_stub() const         { return false; }

  // Fine grain nmethod support: is_nmethod() == is_java_method() || is_native_method() || is_osr_method()
  virtual bool is_java_method() const            { return false; }
  virtual bool is_native_method() const          { return false; }
  virtual bool is_osr_method() const             { return false; } // on-stack replacement method

#ifdef COMPILER2
  enum { not_yet_computed = -2, undefined = -1 };
  int link_offset();                             // Only used on Intel
#endif // COMPILER2

  // Boundaries
  address    header_begin() const                { return (address)    this; }
  address    header_end() const                  { return ((address)   this) + _header_size; };
  relocInfo* relocation_begin() const            { return (relocInfo*) header_end(); };
  relocInfo* relocation_end() const              { return (relocInfo*)(header_end()   + _relocation_size); }
  address    instructions_begin() const          { return (address)    header_begin() + _instructions_offset;  }
  address    instructions_end() const            { return (address)    header_begin() + _data_offset; }
  address    data_begin() const                  { return (address)    header_begin() + _data_offset; }
  address    data_end() const                    { return (address)    header_begin() + _size; }  
  oop*       oops_begin() const                  { return (oop*)      (header_begin() + _oops_offset); }
  oop*       oops_end() const                    { return                oops_begin() + _oops_length; }

  // Offsets
  int relocation_offset() const                  { return _header_size; }
  int instructions_offset() const                { return _instructions_offset; }
  int data_offset() const                        { return _data_offset; }
  int oops_offset() const                        { return _oops_offset; }

  // Sizes
  int size() const                               { return _size; }
  int header_size() const                        { return _header_size; }
  int relocation_size() const                    { return (address) relocation_end() - (address) relocation_begin(); }
  int instructions_size() const                  { return instructions_end() - instructions_begin();  }
  int data_size() const                          { return data_end() - data_begin(); }
  int oops_size() const                          { return (address) oops_end() - (address) oops_begin(); }  

  // Containment  
  bool blob_contains(address addr) const         { return header_begin()       <= addr && addr < data_end(); }
  bool relocation_contains(relocInfo* addr) const{ return relocation_begin()   <= addr && addr < relocation_end(); }     
  bool instructions_contains(address addr) const { return instructions_begin() <= addr && addr < instructions_end(); }
  bool data_contains(address addr) const         { return data_begin()         <= addr && addr < data_end(); }
  bool oops_contains(oop* addr) const            { return oops_begin()         <= addr && addr < oops_end(); }
  bool contains(address addr) const              { return instructions_contains(addr); }

  // Relocation support  
  void fix_relocation_at_move(intptr_t delta);
  void fix_oop_relocations(address begin, address end);
  void fix_oop_relocations(); 
  relocInfo::relocType reloc_type_for_address(address pc);
  bool is_at_poll_return(address pc);
  bool is_at_poll_or_poll_return(address pc);

  // Support for oops in scopes and relocs:
  // Note: index 0 is reserved for null.
  oop  oop_at(int index) const                   { return *oop_addr_at(index); }
  oop* oop_addr_at(int index) const;             // for GC
  void copy_oops(jobject* array, int length);

  // CodeCache support: really only used by the nmethods, but in order to get 
  // asserts and certain bookkeeping to work in the CodeCache they are defined 
  // virtual here.
  virtual bool is_zombie() const                 { return false; }
  virtual bool is_locked_by_vm() const           { return false; }

  virtual bool is_unloaded() const               { return false; }
  virtual bool is_not_entrant() const            { return false; }

  virtual int code_size () const                 { return 0; }
  virtual int exception_size () const            { return 0; }
  virtual int stub_size () const                 { return 0; }
  virtual int scopes_data_size () const          { return 0; }
  virtual int scopes_pcs_size () const           { return 0; }

  virtual int number_of_dependents () const      { return 0; }

  // GC support
  virtual bool is_alive() const                  = 0;
  virtual void follow_roots_or_mark_for_unloading( BoolObjectClosure* is_alive,
                                   OopClosure* keep_alive,
                                   bool unloading_occurred,
                                   bool& marked_for_unloading);
  virtual void oops_do(OopClosure* f) = 0;
  
  // OopMap for frame  
  OopMapSet* oop_maps() const                    { return _oop_maps; }
  void set_oop_maps(OopMapSet* p);
  OopMap* oop_map_for_return_address(address return_address, bool at_call);
  virtual void preserve_callee_argument_oops(frame fr, const RegisterMap* reg_map, OopClosure* f)  { ShouldNotReachHere(); }

  // Frame support
  int  frame_size() const                        { return _frame_size; }
  void set_frame_size(int size)                  { _frame_size = size; }

  // Returns true, if the next frame is responsible for GC'ing oops passed as arguments
  virtual bool caller_must_gc_arguments(JavaThread* thread) const { return false; }

  // Naming
  const char* name() const                       { return _name; }
  void set_name(const char* name)                { _name = name; }

  // Debugging
  virtual void verify()                          PRODUCT_RETURN;
  virtual void print() const                     PRODUCT_RETURN;
  virtual void print_value_on(outputStream* st) const PRODUCT_RETURN;
};


//----------------------------------------------------------------------------------------------------
// BufferBlob: used to hold non-relocatable machine code such as the interpreter, stubroutines, etc.

class BufferBlob: public CodeBlob {
  friend class VMStructs;
 private:
  // Creation support
  BufferBlob(const char* name, int size);

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static BufferBlob* create(const char* name, int buffer_size);

  static void free( BufferBlob *buf );

  // Typing
  bool is_buffer_blob() const                    { return true; }

  // GC/Verification support
  void preserve_callee_argument_oops(frame fr, const RegisterMap* reg_map, OopClosure* f)  { /* nothing to do */ }
  bool is_alive() const                          { return true; }
  void follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive,
                           OopClosure* keep_alive,
                           bool unloading_occurred,
                           bool& marked_for_unloading)
  { /* do nothing */ }

  void oops_do(OopClosure* f)                    { /* do nothing*/ }

  void verify()                                  PRODUCT_RETURN;
  void print() const                             PRODUCT_RETURN;
  void print_value_on(outputStream* st) const    PRODUCT_RETURN;
};


//----------------------------------------------------------------------------------------------------
// RuntimeStub: describes stubs used by compiled code to call a (static) C++ runtime routine

class RuntimeStub: public CodeBlob {
  friend class VMStructs;
 private:
  bool        _caller_must_gc_arguments;

  // Creation support
  RuntimeStub(
    const char* name,
    CodeBuffer* cb,
    int         size,
    int         frame_size,
    OopMapSet*  oop_maps,
    bool        caller_must_gc_arguments
  );

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static RuntimeStub* new_runtime_stub(
    const char* stub_name,
    CodeBuffer* cb,
    int         frame_size,
    OopMapSet*  oop_maps,
    bool        caller_must_gc_arguments
  );

  // Typing
  bool is_runtime_stub() const                   { return true; }
  
  // GC support
  bool caller_must_gc_arguments(JavaThread* thread) const { return _caller_must_gc_arguments; }

  address entry_point()                          { return instructions_begin(); }

  // GC/Verification support
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f)  { /* nothing to do */ }
  bool is_alive() const                          { return true; }   
  void follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive,
                           OopClosure* keep_alive,
                           bool unloading_occurred,
                           bool& marked_for_unloading)
  { /* do nothing */ }
  void oops_do(OopClosure* f) { /* do-nothing*/ }

  void verify()                                  PRODUCT_RETURN;
  void print() const                             PRODUCT_RETURN;
  void print_value_on(outputStream* st) const    PRODUCT_RETURN;
};


//----------------------------------------------------------------------------------------------------
// Super-class for all blobs that exist in only one instance. Implements default behaviour.

class SingletonBlob: public CodeBlob {
  friend class VMStructs;
  public:  
   SingletonBlob(
     const char* name,
     CodeBuffer* cb,
     int         header_size,
     int         size,
     int         frame_size,
     OopMapSet*  oop_maps,
     int         oop_size = 0
   )
   : CodeBlob(name, cb, header_size, size, frame_size, oop_maps, oop_size)
   {};

   bool is_alive() const                         { return true; }
   void follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive,
                            OopClosure* keep_alive,
                            bool unloading_occurred,
                            bool& marked_for_unloading)
   { /* do-nothing*/ }

   void verify()                                 PRODUCT_RETURN; // does nothing
   void print() const                            PRODUCT_RETURN;
   void print_value_on(outputStream* st) const   PRODUCT_RETURN;
};


//----------------------------------------------------------------------------------------------------
// DeoptimizationBlob

class DeoptimizationBlob: public SingletonBlob {
  friend class VMStructs;
 private:
  int _unpack_offset;
  int _unpack_with_exception;
  int _unpack_with_reexecution;

  // Creation support
  DeoptimizationBlob(
    CodeBuffer* cb,
    int         size,
    OopMapSet*  oop_maps,
    int         unpack_offset,
    int         unpack_with_exception_offset,
    int         unpack_with_reexecution_offset,
    int         frame_size
  );

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static DeoptimizationBlob* create(
    CodeBuffer* cb,
    OopMapSet*  oop_maps,
    int         unpack_offset,
    int         unpack_with_exception_offset,
    int         unpack_with_reexecution_offset,
    int         frame_size
  );

  // Typing
  bool is_deoptimization_stub() const { return true; }  
  
  // GC for args
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f) { /* Nothing to do */ }

  // Iteration
  void oops_do(OopClosure* f) {}

  // Printing
  void print_value_on(outputStream* st) const PRODUCT_RETURN;

  address unpack() const                         { return instructions_begin() + _unpack_offset;           }
  address unpack_with_exception() const          { return instructions_begin() + _unpack_with_exception;   }
  address unpack_with_reexecution() const        { return instructions_begin() + _unpack_with_reexecution; }
};


//----------------------------------------------------------------------------------------------------
// UncommonTrapBlob (currently only used by Compiler 2)

#ifdef COMPILER2
class UncommonTrapBlob: public SingletonBlob {
  friend class VMStructs;
 private:
  // Creation support
  UncommonTrapBlob(
    CodeBuffer* cb,
    int         size,
    OopMapSet*  oop_maps,
    int         frame_size
  );

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static UncommonTrapBlob* create(
    CodeBuffer* cb,
    OopMapSet*  oop_maps, 
    int         frame_size
  );

  // GC for args
  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f)  { /* nothing to do */ }

  // Typing
  bool is_uncommon_trap_stub() const             { return true; }

  // Iteration
  void oops_do(OopClosure* f) {}
};
#endif // COMPILER2


//----------------------------------------------------------------------------------------------------
// ExceptionBlob: used for exception unwinding in compiled code (currently only used by Compiler 2)

#ifdef COMPILER2
class ExceptionBlob: public SingletonBlob {
  friend class VMStructs;
 private:
  // Creation support
  ExceptionBlob(
    CodeBuffer* cb,
    int         size,
    OopMapSet*  oop_maps,
    int         frame_size
  );

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static ExceptionBlob* create(
    CodeBuffer* cb,
    OopMapSet*  oop_maps,
    int         frame_size
  );

  // GC for args
  void preserve_callee_argument_oops(frame fr, const RegisterMap* reg_map, OopClosure* f)  { /* nothing to do */ }

  // Typing
  bool is_exception_stub() const                 { return true; }
  
  // Iteration
  void oops_do(OopClosure* f) {}
};
#endif // COMPILER2


//----------------------------------------------------------------------------------------------------
// SafepointBlob: handles illegal_instruction exceptions during a safepoint

class SafepointBlob: public SingletonBlob {
  friend class VMStructs;
 private:
  // Creation support
  SafepointBlob(
    CodeBuffer* cb,
    int         size,
    OopMapSet*  oop_maps,
    int         frame_size
  );

  void* operator new(size_t s, unsigned size);

 public:
  // Creation
  static SafepointBlob* create(
    CodeBuffer* cb,
    OopMapSet*  oop_maps,
    int         frame_size
  );

  // GC for args
  void preserve_callee_argument_oops(frame fr, const RegisterMap* reg_map, OopClosure* f)  { /* nothing to do */ }

  // Returns true, if the next frame is responsible for GC'ing
  // oops passed as arguments. This is normally true, except when
  // we are blocked at a return.
  virtual bool caller_must_gc_arguments(JavaThread* thread) const;

  // Typing
  bool is_safepoint_stub() const                 { return true; }

  // Iteration
  void oops_do(OopClosure* f) {}
};
