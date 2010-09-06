#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod.hpp	1.139 03/12/23 16:39:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

// This class is used internally by nmethods, to cache
// exception/pc/handler information.

class ExceptionCache : public CHeapObj {
  friend class VMStructs;
 private:
  static address _unwind_handler; 
  enum { cache_size = 16 };
  klassOop _exception_type;
  address  _pc[cache_size];
  address  _handler[cache_size];
  int      _count;
  ExceptionCache* _next;

  address pc_at(int index)                     { assert(index >= 0 && index < count(),""); return _pc[index]; }
  void    set_pc_at(int index, address a)      { assert(index >= 0 && index < cache_size,""); _pc[index] = a; }
  address handler_at(int index)                { assert(index >= 0 && index < count(),""); return _handler[index]; }
  void    set_handler_at(int index, address a) { assert(index >= 0 && index < cache_size,""); _handler[index] = a; }
  int     count()                              { return _count; }
  void    increment_count()                    { _count++; }

 public:

  ExceptionCache(Handle exception, address pc, address handler);

  klassOop  exception_type()                { return _exception_type; }
  klassOop* exception_type_addr()           { return &_exception_type; }
  ExceptionCache* next()                    { return _next; }
  void      set_next(ExceptionCache *ec)    { _next = ec; }

  address match(Handle exception, address pc);
  bool    match_exception_with_space(Handle exception) ;
  address test_address(address addr);
  bool    add_address_and_handler(address addr, address handler) ;

  static address unwind_handler() { return _unwind_handler; }
};


// cache pc descs found in earlier inquiries
class PcDescCache VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  enum { cache_size = 4 };
  PcDesc* _pc_descs[cache_size]; // last cache_size pc_descs found
 public:
  PcDescCache();
  PcDesc* pc_desc_at(nmethod* nm, address pc, bool at_call) const;
  PcDesc* pc_desc_at(nmethod* nm, address pc) const;
  void add_pc_desc(PcDesc* pc_desc);
};


// nmethods (native methods) are the compiled code versions of Java methods.

struct nmFlags {
  friend class VMStructs;
  unsigned int version:8;                 // version number (0 = first version)
  unsigned int level:4;                   // optimization level
  unsigned int age:4;                     // age (in # of sweep steps)

  unsigned int state:2;                   // {alive, zombie, unloaded)

  unsigned int isUncommonRecompiled:1;    // recompiled because of uncommon trap?
  unsigned int isToBeRecompiled:1;        // to be recompiled as soon as it matures
  unsigned int hasFlushedDependencies:1;  // Used for maintenance of dependencies
  unsigned int markedForReclamation:1;    // Used by NMethodSweeper
  unsigned int markedForUnloading:1;      // Used by GC

  unsigned int has_unsafe_access:1;       // May fault due to unsafe access.

  void clear();
};


// A nmethod contains:    
//  - header                 (the nmethod structure)
//  [Relocation]
//  - relocation information  
//  - constant part          (doubles, longs and floats used in nmethod)
//  [Code]
//  - code body
//  - exception handler
//  - stub code
//  [Debugging information]
//  - oop array
//  - data array
//  - pcs
//  [Exception handler table]
//  - handler entry point array
//  [Implicit Null Pointer exception table]
//  - implicit null table array

class ExceptionHandlerTable;
class ImplicitExceptionTable;
class ExceptionRangeTable;
class AbstractCompiler;
class nmethod : public CodeBlob {
  friend class VMStructs;
  friend class NMethodSweeper;
 private:
  // Shared fields for all nmethod's
  static int _zombie_instruction_size;

  methodOop _method;
  int       _entry_bci;        // != InvocationEntryBci if this nmethod is an on-stack replacement method

  nmethod*  _link;             // To support simple linked-list chaining of nmethods

  AbstractCompiler* _compiler; // The compiler which compiled this nmethod

  // Offset for differents nmethod parts
  int _exception_offset;
  int _stub_offset;
  int _scopes_data_offset;
  int _scopes_pcs_offset;
  int _handler_table_offset;
  int _nul_chk_table_offset;
  int _nmethod_end_offset;
   
  int _first_dependent;
  int _number_of_dependents;
  
  int _compile_id;                     // which compilation made this nmethod

  // offsets for entry points
  address _entry_point;                // entry point with class check
  address _verified_entry_point;       // entry point without class check
  address _interpreter_entry_point;    // entry point when coming from interpreter
  address _osr_entry_point;            // entry point for on stack replacement
  int     _frame_start_offset;

  nmFlags flags;           // various flags to keep track of nmethod state
  int _markedForDeoptimization;        // Used for stack deoptimization
  enum { alive        = 0,
         not_entrant  = 1, // uncommon trap has happend but activations may still exist
         zombie       = 2, 
         unloaded     = 3 };

  // used by jvmti to track if an unload event has been posted for this nmethod.
  bool _unload_reported;

  NOT_PRODUCT(bool _has_debug_info; )

  // Nmethod Flushing lock (if non-zero, then the nmethod is not removed)
  jint  _lock_count;

  // not_entrant method removal. Each mark_sweep pass will update 
  // this mark to current sweep invocation count if it is seen on the
  // stack.  An not_entrant method can be removed when there is no
  // more activations, i.e., when the _stack_traversal_mark is less than
  // current sweep traversal index.
  long _stack_traversal_mark;

  ExceptionCache *_exception_cache;
  PcDescCache     _pc_desc_cache;

  PcDesc* _cached_pcdesc0;

  C2IAdapter*  _c2i_adapter;   // When deopting this nmethod the c2i adapter used to call the method
  // This flag ought to be identical to not_entrant|zombie state but there are
  // some problematic uses of not_entrant so use this flag until after tiger
  // NEEDS CLEANUP
  volatile bool _patched_for_deopt;     // True when nmethod is patched for deopt

  friend class nmethodLocker;
  
  // Creation support
  nmethod(methodOop method,          
          int nmethod_size,
          int entry_bci,
          int iep_offset,
          int ep_offset,
          int vep_offset,
          int code_offset,
          int osr_offset,          
          DebugInformationRecorder *recorder,
          CodeBuffer *code_buffer,
          int frame_size,
          OopMapSet* oop_maps,
          ExceptionHandlerTable* handler_table,
          ImplicitExceptionTable* nul_chk_table,
          ExceptionRangeTable* range_table,
          AbstractCompiler* compiler);

  // helper methods
  void* operator new(size_t size, int nmethod_size);
  void check_store();
  void resolve_JNIHandles();

  bool is_dependent_on_entry(klassOop dependee, klassOop klass, methodOop method = NULL);
  const char* reloc_string_for(u_char* begin, u_char* end);
  void make_not_entrant_or_zombie(int state);

  // used to check that writes to nmFlags are done consistently.
  static void check_safepoint() PRODUCT_RETURN;
 
  // Used to manipulate the exception cache
  void add_exception_cache_entry(ExceptionCache* new_entry);
  ExceptionCache* exception_cache_entry_for_exception(Handle exception);

  // tell external interfaces that a compiled method has been unloaded
  inline void post_compiled_method_unload(BoolObjectClosure* is_alive);

 public:
  // create nmethod with entry_bci
  static nmethod* new_nmethod(methodHandle method,
                              int entry_bci,
                              int iep_offset,
                              int ep_offset,
                              int vep_offset,
                              int code_offset,
                              int osr_offset,
                              DebugInformationRecorder* recorder, 
                              CodeBuffer *code_buffer, int frame_size, 
                              OopMapSet* oop_maps, 
                              ExceptionHandlerTable* handler_table, 
                              ImplicitExceptionTable* nul_chk_table,
                              ExceptionRangeTable* range_table,
                              AbstractCompiler* compiler);

  // deoptimize nmethod
  void deoptimize_nmethod(address active_return);

  // accessors
  methodOop method() const                        { return _method; }
  AbstractCompiler* compiler() const              { return _compiler; }

  C2IAdapter* c2i_adapter() const                 { return _c2i_adapter; }
  void set_c2i_adapter(C2IAdapter* adapter)       { _c2i_adapter = adapter; }

#ifndef PRODUCT
  bool has_debug_info() const                     { return _has_debug_info; }
  void set_has_debug_info(bool f)                 { _has_debug_info = false; }
#endif // NOT PRODUCT

  // type info
  bool is_nmethod() const                         { return true; }
  bool is_java_method() const                     { return !method()->is_native(); }
  bool is_native_method() const                   { return method()->is_native(); }
  bool is_osr_method() const                      { return _entry_bci != InvocationEntryBci; }
  bool is_osr_only_method() const                 { return is_osr_method(); }
    
  // boundaries for different parts
  address code_begin         () const             { return _entry_point; }
  address code_end           () const             { return           header_begin() + _exception_offset     ; }
  address exception_begin    () const             { return           header_begin() + _exception_offset     ; }
  address exception_end      () const             { return           header_begin() + _stub_offset          ; }
  address stub_begin         () const             { return           header_begin() + _stub_offset          ; }
  address stub_end           () const             { return           header_begin() + _scopes_data_offset   ; }
  address scopes_data_begin  () const             { return           header_begin() + _scopes_data_offset   ; }
  address scopes_data_end    () const             { return           header_begin() + _scopes_pcs_offset    ; }
  PcDesc* scopes_pcs_begin   () const             { return (PcDesc*)(header_begin() + _scopes_pcs_offset   ); }
  PcDesc* scopes_pcs_end     () const             { return (PcDesc*)(header_begin() + _handler_table_offset); }
  address handler_table_begin() const             { return           header_begin() + _handler_table_offset ; }
  address handler_table_end  () const             { return           header_begin() + _nul_chk_table_offset   ; }
  address nul_chk_table_begin() const             { return           header_begin() + _nul_chk_table_offset ; }
  address nul_chk_table_end  () const             { return           header_begin() + _nmethod_end_offset   ; }

  int code_size         () const                  { return      code_end         () -      code_begin         (); }
  int exception_size    () const                  { return      exception_end    () -      exception_begin    (); }
  int stub_size         () const                  { return      stub_end         () -      stub_begin         (); }
  int scopes_data_size  () const                  { return      scopes_data_end  () -      scopes_data_begin  (); }
  int scopes_pcs_size   () const                  { return (intptr_t)scopes_pcs_end   () - (intptr_t)scopes_pcs_begin   (); }
  int handler_table_size() const                  { return      handler_table_end() -      handler_table_begin(); }
  int nul_chk_table_size() const                  { return      nul_chk_table_end() -      nul_chk_table_begin(); }

  int total_size        () const;

  bool code_contains         (address addr) const { return code_begin         () <= addr && addr < code_end         (); }
  bool exception_contains    (address addr) const { return exception_begin    () <= addr && addr < exception_end    (); }
  bool stub_contains         (address addr) const { return stub_begin         () <= addr && addr < stub_end         (); }
  bool scopes_data_contains  (address addr) const { return scopes_data_begin  () <= addr && addr < scopes_data_end  (); }
  bool scopes_pcs_contains   (PcDesc* addr) const { return scopes_pcs_begin   () <= addr && addr < scopes_pcs_end   (); }
  bool handler_table_contains(address addr) const { return handler_table_begin() <= addr && addr < handler_table_end(); }
  bool nul_chk_table_contains(address addr) const { return nul_chk_table_begin() <= addr && addr < nul_chk_table_end(); }

  int first_dependent() const                     { return _first_dependent; }
  int number_of_dependents() const                { return _number_of_dependents; }
  int dependent_limit() const                     { return _first_dependent + _number_of_dependents; }

  // entry points
  address entry_point() const                     { return _entry_point;             } // normal entry point
  address verified_entry_point() const            { return _verified_entry_point;    } // if klass is correct

  // return the address of the interpreter_entry_point, null if there is not one.
  address interpreter_entry_point_or_null()	  { return _interpreter_entry_point; }

  // if interpreter entry point is not created it will create one, by
  // compiling an adapter
  address interpreter_entry_point();

  // flag accessing and manipulation
  bool  is_in_use() const                         { return flags.state == alive; }
  bool  is_alive() const                          { return flags.state == alive || flags.state == not_entrant; }
  bool  is_not_entrant() const                    { return flags.state == not_entrant; }
  bool  is_zombie() const                         { return flags.state == zombie; }
  bool  is_unloaded() const                       { return flags.state == unloaded;   }      
  bool  is_patched_for_deopt() const              { return _patched_for_deopt; }

  // Make the nmethod non entrant. The nmethod will continue to be alive.
  // It is used when an uncommon trap happens.
  void  make_not_entrant()                        { make_not_entrant_or_zombie(not_entrant); }
  void  make_zombie()                             { make_not_entrant_or_zombie(zombie); }
  void  make_unloaded();
  void  set_patched_for_deopt()                   { _patched_for_deopt = true; }

  // used by jvmti to track if the unload event has been reported
  bool  unload_reported()			  { return _unload_reported; }
  void  set_unload_reported()			  { _unload_reported = true; }
  
  bool  is_marked_for_deoptimization() const      { return _markedForDeoptimization; }
  void  mark_for_deoptimization()                 { _markedForDeoptimization = 1; }

  bool  is_marked_for_unloading() const           { return flags.markedForUnloading; }
  void  mark_for_unloading(BoolObjectClosure* is_alive);

  void flush_dependencies(BoolObjectClosure* is_alive);
  bool  has_flushed_dependencies()                { return flags.hasFlushedDependencies; }
  void  set_has_flushed_dependencies()            {
    check_safepoint();
    assert(!has_flushed_dependencies(), "should only happen once");
    flags.hasFlushedDependencies = 1;
  }

  bool  is_marked_for_reclamation() const         { return flags.markedForReclamation; }
  void  mark_for_reclamation()                    { check_safepoint(); flags.markedForReclamation = 1; }
  void  unmark_for_reclamation()                  { check_safepoint(); flags.markedForReclamation = 0; }

  bool  has_unsafe_access() const                 { return flags.has_unsafe_access; }
  void  set_has_unsafe_access(bool z)             { flags.has_unsafe_access = z; }

  int   level() const                             { return flags.level; }
  void  set_level(int newLevel)                   { check_safepoint(); flags.level = newLevel; }

  int   version() const                           { return flags.version; }
  void  set_version(int v);

  // Sweeper support
  long  stack_traversal_mark()                    { return _stack_traversal_mark; }
  void  set_stack_traversal_mark(long l)          { _stack_traversal_mark = l; } 

  // Exception cache support
  ExceptionCache* exception_cache() const         { return _exception_cache; }
  void set_exception_cache(ExceptionCache *ec)    { _exception_cache = ec; }
  address handler_for_exception_and_pc(Handle exception, address pc);
  void add_handler_for_exception_and_pc(Handle exception, address pc, address handler);
  void remove_from_exception_cache(ExceptionCache* ec);

  // C1 exception handling support
  COMPILER1_ONLY(ExceptionRangeTable* exception_range_table() { return (ExceptionRangeTable*)handler_table_begin(); })

  // implicit exceptions support
  address continuation_for_implicit_exception(address pc);

  // On-stack replacement support
  int   osr_entry_bci() const                     { assert(_entry_bci != InvocationEntryBci, "wrong kind of nmethod"); return _entry_bci; }
  void  invalidate_osr_method();
  nmethod* link() const                           { return _link; }
  void     set_link(nmethod *n)                   { _link = n; }

  // tells whether frames described by this nmethod can be deoptimized
  // note: native wrappers cannot be deoptimized.
  bool can_be_deoptimized() const { return is_java_method(); }
  
  // Inline cache support
  void clear_inline_caches()                      CORE_RETURN;  
  void cleanup_inline_caches()                    CORE_RETURN;
  bool inlinecache_check_contains(address addr) const {
    intptr_t offset = (intptr_t)addr - (intptr_t)instructions_begin();
    return offset < (intptr_t)_frame_start_offset;
  }

  // unlink and deallocate this nmethod 
  // Only NMethodSweeper class is expected to use this. NMethodSweeper is not
  // expected to use any other private methods/data in this class. 

 protected:
  void flush();

 public:
  // If returning true, it is unsafe to remove this nmethod even though it is a zombie
  // nmethod, since the VM might have a reference to it. Should only be called from a  safepoint.
  bool is_locked_by_vm() const                    { return _lock_count >0; }
 
  // See comment at definition of _last_seen_on_stack
  void mark_as_seen_on_stack();
  bool can_not_entrant_be_converted();

  // Evolution support. We make old (discarded) compiled methods point to new methodOops.
  void set_method(methodOop method) { _method = method; }

  // GC support
  void follow_roots_or_mark_for_unloading(
    BoolObjectClosure* is_alive, OopClosure* keep_alive,
    bool unloading_occurred, bool& marked_for_unloading);
  void follow_root_or_mark_for_unloading(
    BoolObjectClosure* is_alive, OopClosure* keep_alive,
    oop* root, bool unloading_occurred, bool& marked_for_unloading);

  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map,
				     OopClosure* f);
  void oops_do(OopClosure* f);

  // ScopeDesc for an instruction
  ScopeDesc* scope_desc_at(address pc, bool at_call);

  // ScopeDesc when either call or non-call is allowed
  ScopeDesc* scope_desc_at(address pc);

 private:
  ScopeDesc* scope_desc_in(address begin, address end);

 public:
  // ScopeDesc retrieval operation
  PcDesc* pc_desc_at(address pc, bool at_call);
  PcDesc* pc_desc_at(address pc);

 public:
  // copying of debugging information
  void copy_pc_at(int index, PcDesc* pc);
  void copy_scopes_data(address buffer, int size);

  // verify operations
  void verify()                                   PRODUCT_RETURN;
  void verify_scopes()                            PRODUCT_RETURN;
  void verify_interrupt_point(address interrupt_point) PRODUCT_RETURN;
  
  // printing support
  void print()                          const     PRODUCT_RETURN;
  void print_code()                               PRODUCT_RETURN;
  void print_relocations()                        PRODUCT_RETURN;
  void print_pcs()                                PRODUCT_RETURN;
  void print_scopes()                             PRODUCT_RETURN;
  void print_dependencies()                       PRODUCT_RETURN;
  void print_value_on(outputStream* st) const     PRODUCT_RETURN;
  void print_calls(outputStream* st)              PRODUCT_RETURN;
  void print_handler_table()                      PRODUCT_RETURN;
  void print_nul_chk_table()                      PRODUCT_RETURN;
  void print_nmethod(bool print_code)             PRODUCT_RETURN;
  // Prints a comment for one native instruction (reloc info, pc desc)
  void print_code_comment_on(outputStream* st, int column, address begin, address end) PRODUCT_RETURN;

  // Compiler task identification.  Note that all OSR methods
  // are numbered in an independent sequence if CICountOSR is true,
  // and native method wrappers are also numbered independently if
  // CICountNative is true.
  int  compile_id() const                         { return _compile_id; }
  void set_compile_id(int id)                     { _compile_id = id; }
  const char* compile_kind() const;

  // For debugging
  // CompiledIC*    IC_at(char* p) const;
  // PrimitiveIC*   primitiveIC_at(char* p) const;
  oop embeddedOop_at(address p);

  // tells if this compiled method is dependent on
  bool is_dependent_on(klassOop dependee);  

  // Evolution support. Tells if this compiled method is dependent on any of
  // methods m() of class dependee, such that if m() in dependee is replaced,
  // this compiled method will have to be deoptimized.
  bool is_evol_dependent_on(klassOop dependee);

  // Fast breakpoint support. Tells if this compiled method is
  // dependent on the given method. Returns true if this nmethod
  // corresponds to the given method as well.
  bool is_dependent_on_method(methodOop dependee);  

  // is it ok to patch at address?
  bool is_patchable_at(address instr_address);
  
  // support for code generation
  static int entry_point_offset()                 { return (intptr_t)&((nmethod*)NULL)->_entry_point; }
  static int verified_entry_point_offset()        { return (intptr_t)&((nmethod*)NULL)->_verified_entry_point; }
  static int osr_entry_point_offset()             { return (intptr_t)&((nmethod*)NULL)->_osr_entry_point; }
  static int interpreter_entry_point_offset()     { return (intptr_t)&((nmethod*)NULL)->_interpreter_entry_point; }
  static int entry_bci_offset()                   { return (intptr_t)&((nmethod*)NULL)->_entry_bci; }
  static int method_offset_in_bytes()             { return (intptr_t)&((nmethod*)NULL)->_method; }

  // Machine dependent stuff
  #include "incls/_nmethod_pd.hpp.incl"

};

// Locks an nmethod so its code will not get removed, even if it is a zombie/not_entrant method
class nmethodLocker : public StackObj {
  nmethod* _nm;
 public:
  nmethodLocker(address pc);
  nmethodLocker(nmethod *nm);
  ~nmethodLocker();
};


