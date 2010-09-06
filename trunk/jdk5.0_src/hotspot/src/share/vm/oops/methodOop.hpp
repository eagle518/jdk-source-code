#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodOop.hpp	1.189 04/04/27 10:57:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A methodOop represents a Java method. 
//
// Memory layout (each line represents a word). Note that most applications load thousands of methods,
// so keeping the size of this structure small has a big impact on footprint.
//
// We put all oops and method_size first for better gc cache locality.
//
// The actual bytecodes are inlined after the end of the methodOopDesc struct.
//
// There are bits in the access_flags telling whether inlined tables are present.
// Note that accessing the line number and local variable tables is not performance critical at all.
// Accessing the checked exceptions table is used by reflection, so we put that last to make access
// to it fast.
//
// The line number table is compressed and inlined following the byte codes. It is found as the first 
// byte following the byte codes. The checked exceptions table and the local variable table are inlined 
// after the line number table, and indexed from the end of the method. We do not compress the checked
// exceptions table since the average length is less than 2, and do not bother to compress the local 
// variable table either since it is mostly absent.
//
// Note that native_function and signature_handler has to be at fixed offsets (required by the interpreter)
//
// |------------------------------------------------------|
// | header                                               |
// | klass                                                |
// |------------------------------------------------------|
// | constMethodOop                 (oop)                 |
// | constants                      (oop)                 |
// |------------------------------------------------------|
// | methodData                     (oop)                 | C2 Only
// | interp_invocation_count                              | C2 Only
// |------------------------------------------------------|
// | access_flags                                         |
// | vtable_index                                         |
// |------------------------------------------------------|
// | result_index (C++ interpreter only)                  |
// |------------------------------------------------------|
// | method_size             | max_stack                  |
// | max_locals              | size_of_parameters         |
// |------------------------------------------------------|
// | tier1_done              | throwout_count             | C2 Only
// |------------------------------------------------------|
// | parameter_info          | num_breakpoints            |
// |------------------------------------------------------|
// | invocation_counter                                   |
// | backedge_counter                                     |
// |------------------------------------------------------|
// | code                           (pointer)             |
// | interpreter_entry              (pointer)             |
// | from_compiled_code_entry_point (pointer)             |
// |------------------------------------------------------|
// | native_function       (present only if native)       |
// | signature_handler     (present only if native)       |
// |------------------------------------------------------|


class CheckedExceptionElement;
class LocalVariableTableElement;


class methodOopDesc : public oopDesc {
 friend class methodKlass;
 friend class VMStructs;
 private:
  constMethodOop    _constMethod;                // Method read-only data.
  constantPoolOop   _constants;                  // Constant pool
#ifdef COMPILER2
  methodDataOop     _method_data;
  int               _interpreter_invocation_count; // Count of times invoked
#endif // COMPILER2
  AccessFlags       _access_flags;               // Access flags
  int               _vtable_index;               // vtable index of this method (-1 for static methods)
                                                 // note: can have vtables with >2**16 elements (because of inheritance)
#ifdef CC_INTERP
  int               _result_index;               // C++ interpreter needs for converting results to/from stack
#endif
  u2                _method_size;                // size of this object
  u2                _max_stack;                  // Maximum number of entries on the expression stack
  u2                _max_locals;                 // Number of local variables used by this method
  u2                _size_of_parameters;         // size of the parameter block (receiver + arguments) in words

#ifdef COMPILER2
  u1                _tier1_compile_done;         // Set by C2 if a tier 1 compile has been done
  u2                _interpreter_throwout_count; // Count of times method was exited via exception while interpreting
#endif
  u2                _parameter_info;             // Platform specific information about parameters - for C1 parameter passing
  u2                _number_of_breakpoints;      // fullspeed debugging support
#ifndef CORE
  InvocationCounter _invocation_counter;         // Incremented before each activation of the method - used to trigger frequency-based optimizations
  InvocationCounter _backedge_counter;           // Incremented before each backedge taken - used to trigger frequencey-based optimizations
#endif
#ifndef PRODUCT
#ifndef CORE
  int               _compiled_invocation_count;  // Number of nmethod invocations so far (for perf. debugging)
#endif
#endif
  nmethod* volatile _code;                       // Points to the corresponding piece of native code
  address           _interpreter_entry;          // entry point for interpretation (used by vtables)
#ifndef CORE
  volatile address  _from_compiled_code_entry_point; // address to dispatch to (either nmethod or interpreter entry)
                                                 // Must read/write in order
#endif

 public:
  // accessors for instance variables
  constMethodOop constMethod() const             { return _constMethod; }
  void set_constMethod(constMethodOop xconst)    { oop_store_without_check((oop*)&_constMethod, (oop)xconst); }

  // access flag
  AccessFlags access_flags() const               { return _access_flags;  }
  void set_access_flags(AccessFlags flags)       { _access_flags = flags; }

  // name
  symbolOop name() const                         { return _constants->symbol_at(name_index()); }
  int name_index() const                         { return constMethod()->name_index();         }
  void set_name_index(int index)                 { constMethod()->set_name_index(index);       }

  // signature
  symbolOop signature() const                    { return _constants->symbol_at(signature_index()); }
  int signature_index() const                    { return constMethod()->signature_index();         }
  void set_signature_index(int index)            { constMethod()->set_signature_index(index);       }

  // generics support
  symbolOop generic_signature() const            { int idx = generic_signature_index(); return ((idx != 0) ? _constants->symbol_at(idx) : NULL); }
  int generic_signature_index() const            { return constMethod()->generic_signature_index(); }
  void set_generic_signature_index(int index)    { constMethod()->set_generic_signature_index(index); }

  // annotations support
  typeArrayOop annotations() const;
  typeArrayOop parameter_annotations() const;
  typeArrayOop annotation_default() const;

#ifdef CC_INTERP
  void set_result_index(BasicType type);
  int  result_index()                            { return _result_index; }
#endif

  // Helper routine: get klass name + "." + method name + signature as
  // C string, for the purpose of providing more useful NoSuchMethodErrors
  // and fatal error handling. The string is allocated in resource
  // area if a buffer is not provided by the caller.
  char* name_and_sig_as_C_string();
  char* name_and_sig_as_C_string(char* buf, int size);

  // Static routine in the situations we don't have a methodOop
  static char* name_and_sig_as_C_string(Klass* klass, symbolOop method_name, symbolOop signature);
  static char* name_and_sig_as_C_string(Klass* klass, symbolOop method_name, symbolOop signature, char* buf, int size);

  // JVMDI breakpoints
  Bytecodes::Code orig_bytecode_at(int bci);
  void        set_orig_bytecode_at(int bci, Bytecodes::Code code);
  void set_breakpoint(int bci);
  void clear_breakpoint(int bci);
  void clear_all_breakpoints();
  // Tracking number of breakpoints, for fullspeed debugging.
  // Only mutated by VM thread.
  u2   number_of_breakpoints() const             { return _number_of_breakpoints; }
  void incr_number_of_breakpoints()              { ++_number_of_breakpoints; }
  void decr_number_of_breakpoints()              { --_number_of_breakpoints; }
  // Initialization only
  void clear_number_of_breakpoints()             { _number_of_breakpoints = 0; }

  // index into instanceKlass methods() array
  u2 method_index() const           { return constMethod()->method_index(); }
  void set_method_index(u2 index)   { constMethod()->set_method_index(index); }

  // code size
  int code_size() const                  { return constMethod()->code_size(); }

  // method size
  int method_size() const                        { return _method_size; }
  void set_method_size(int size) {
    assert(0 <= size && size < (1 << 16), "invalid method size");
    _method_size = size;
  }

  // constant pool for klassOop holding this method
  constantPoolOop constants() const              { return _constants; }
  void set_constants(constantPoolOop c)          { oop_store_without_check((oop*)&_constants, c); }

  // max stack
  int  max_stack() const                         { return _max_stack; }
  void set_max_stack(int size)                   { _max_stack = size; }

  // max locals
  int  max_locals() const                        { return _max_locals; }
  void set_max_locals(int size)                  { _max_locals = size; }

#ifdef COMPILER2
  bool tier1_compile_done()          { return _tier1_compile_done != 0;}
  void set_tier1_compile_done()      { _tier1_compile_done = 1;}

  // Count of times method was exited via exception while interpreting
  void interpreter_throwout_increment() { 
    if (_interpreter_throwout_count < 65534) {
      _interpreter_throwout_count++;
    }
  }   

  int  interpreter_throwout_count() const        { return _interpreter_throwout_count; }
  void set_interpreter_throwout_count(int count) { _interpreter_throwout_count = count; }
#endif // COMPILER2


  // entry point when called from compiled code
#ifndef CORE
  volatile address from_compiled_code_entry_point() const { return (address)OrderAccess::load_ptr_acquire(&_from_compiled_code_entry_point); }
  void set_from_compiled_code_entry_point(address a) { _from_compiled_code_entry_point = a; }
#endif // CORE

  // size of parameters
  int  size_of_parameters() const                { return _size_of_parameters; }

  // exception handler table
  typeArrayOop exception_table() const
                                   { return constMethod()->exception_table(); }
  void set_exception_table(typeArrayOop e)
                                     { constMethod()->set_exception_table(e); }
  bool has_exception_handler() const
                             { return constMethod()->has_exception_handler(); }

  // Finds the first entry point bci of an exception handler for an
  // exception of klass ex_klass thrown at throw_bci. A value of NULL
  // for ex_klass indicates that the exception klass is not known; in
  // this case it matches any constraint class. Returns -1 if the
  // exception cannot be handled in this method. The handler
  // constraint classes are loaded if necessary. Note that this may 
  // throw an exception if loading of the constraint classes causes
  // an IllegalAccessError (bugid 4307310) or an OutOfMemoryError. 
  // If an exception is thrown, returns the bci of the
  // exception handler which caused the exception to be thrown, which
  // is needed for proper retries. See, for example,
  // InterpreterRuntime::exception_handler_for_exception.
  int fast_exception_handler_bci_for(KlassHandle ex_klass, int throw_bci, TRAPS);

  // stackmap table
  typeArrayOop stackmap_u1() const               { return constMethod()->stackmap_u1(); }
  typeArrayOop stackmap_u2() const               { return constMethod()->stackmap_u2(); }
  void set_stackmap_u1(typeArrayOop t)           { constMethod()->set_stackmap_u1(t);   }
  void set_stackmap_u2(typeArrayOop t)           { constMethod()->set_stackmap_u2(t);   }
  bool has_stackmap_table() const                { return constMethod()->has_stackmap_table(); }
  void delete_stackmap()                         { constMethod()->delete_stackmap();    }

#ifndef CORE
  // invocation counter
  InvocationCounter* invocation_counter()        { return &_invocation_counter; }
  InvocationCounter* backedge_counter()          { return &_backedge_counter; }
  int invocation_count() const                   { return _invocation_counter.count(); }
  int backedge_count() const                     { return _backedge_counter.count(); }
  bool was_executed_more_than(int n) const;
  bool was_never_executed() const                { return !was_executed_more_than(0); }

  // method data access
  methodDataOop method_data() const              { 
    COMPILER1_ONLY(return NULL;)
    COMPILER2_ONLY(return _method_data;)
  }
  void set_method_data(methodDataOop data)       { 
    COMPILER1_ONLY(ShouldNotReachHere();) 
    COMPILER2_ONLY(oop_store_without_check((oop*)&_method_data, (oop)data);) 
  }
  static void build_interpreter_method_data(methodHandle method, TRAPS);

#ifdef COMPILER2
  int interpreter_invocation_count() const       { return _interpreter_invocation_count; }
  void set_interpreter_invocation_count(int count) { _interpreter_invocation_count = count; }
  int increment_interpreter_invocation_count() { return ++_interpreter_invocation_count; }
#endif // COMPILER2

#ifndef PRODUCT
  int  compiled_invocation_count() const         { return _compiled_invocation_count; }
  void set_compiled_invocation_count(int count)  { _compiled_invocation_count = count; }
#endif // not PRODUCT
#endif // not CORE

  // Clear (non-shared space) pointers which could not be relevent
  // if this (shared) method were mapped into another JVM.
  void remove_unshareable_info();

  // nmethod/verified compiler entry
#ifndef CORE
  address verified_code_entry();
  bool check_code() const;	// Not inline to avoid circular ref
  nmethod* volatile code() const                 { assert( check_code(), "" ); return (nmethod *)OrderAccess::load_ptr_acquire(&_code); }
#else
  nmethod* volatile code() const                 { return (nmethod *)OrderAccess::load_ptr_acquire(&_code); }
#endif // not CORE
  void set_code(nmethod* code);
  void init_code();
  static void link_method(methodHandle method);
  void update_compiled_code_entry_point(bool lazy);

  // vtable index
  int  vtable_index() const                      { return _vtable_index; }
  void set_vtable_index(int index)               { _vtable_index = index; }

  // interpreter entry
  address interpreter_entry() const              { return _interpreter_entry; }
  void set_interpreter_entry(address entry)      { _interpreter_entry = entry; }
  int  interpreter_kind(void) {
     return constMethod()->interpreter_kind();
  }
  void set_interpreter_kind();
  void set_interpreter_kind(int kind) {
    constMethod()->set_interpreter_kind(kind);
  }

  // parameter handler support
  int parameter_info() const                     { return _parameter_info; }
  void set_parameter_info(int info) {
    assert(0 <= info && info < (1 << 16), "invalid parameter info");
    _parameter_info = info;
  }

  // native function (used for native methods only)
  address native_function() const                { return *(native_function_addr()); }
  void set_native_function(address function);    // Must specify a real function (not NULL).
                                                 // Use clear_native_function() to unregister.
  bool has_native_function() const;
  void clear_native_function();

  // signature handler (used for native methods only)
  address signature_handler() const              { return *(signature_handler_addr()); }
  void set_signature_handler(address handler);

  // Interpreter oopmap support
  void mask_for(int bci, InterpreterOopMap* mask);

#ifndef CORE
#ifndef PRODUCT
  // operations on invocation counter
  void print_invocation_count() const;
#endif
#endif

  // byte codes
  address code_base() const           { return constMethod()->code_base(); }
  bool    contains(address bcp) const { return constMethod()->contains(bcp); }

  void print_codes() const                       PRODUCT_RETURN; // prints byte codes
  void print_codes(int from, int to) const       PRODUCT_RETURN;

  // checked exceptions
  int checked_exceptions_length() const
                         { return constMethod()->checked_exceptions_length(); }
  CheckedExceptionElement* checked_exceptions_start() const
                          { return constMethod()->checked_exceptions_start(); }

  // localvariable table
  bool has_localvariable_table() const
                          { return constMethod()->has_localvariable_table(); }
  int localvariable_table_length() const
                        { return constMethod()->localvariable_table_length(); }
  LocalVariableTableElement* localvariable_table_start() const
                         { return constMethod()->localvariable_table_start(); }

  bool has_linenumber_table() const
                              { return constMethod()->has_linenumber_table(); }
  u_char* compressed_linenumber_table() const
                       { return constMethod()->compressed_linenumber_table(); }

  // method holder (the klassOop holding this method)
  klassOop method_holder() const                 { return _constants->pool_holder(); }

  void compute_size_of_parameters(Thread *thread); // word size of parameters (receiver if any + arguments)
  symbolOop klass_name() const;                  // returns the name of the method holder
  BasicType result_type() const;                 // type of the method result
  bool is_returning_oop() const                  { BasicType r = result_type(); return (r == T_OBJECT || r == T_ARRAY); }
  bool is_returning_fp() const                   { BasicType r = result_type(); return (r == T_FLOAT || r == T_DOUBLE); }

  // Checked exceptions thrown by this method (resolved to mirrors)
  objArrayHandle resolved_checked_exceptions(TRAPS) { return resolved_checked_exceptions_impl(this, THREAD); }

  // Access flags
  bool is_public() const                         { return access_flags().is_public();      }
  bool is_private() const                        { return access_flags().is_private();     }
  bool is_protected() const                      { return access_flags().is_protected();   }
  bool is_package_private() const                { return !is_public() && !is_private() && !is_protected(); }
  bool is_static() const                         { return access_flags().is_static();      }
  bool is_final() const                          { return access_flags().is_final();       }
  bool is_synchronized() const                   { return access_flags().is_synchronized();}
  bool is_native() const                         { return access_flags().is_native();      }
  bool is_abstract() const                       { return access_flags().is_abstract();    }
  bool is_strict() const                         { return access_flags().is_strict();      }
  bool is_synthetic() const                      { return access_flags().is_synthetic();   }
  
  // returns true if contains only return operation
  bool is_empty_method() const;

  // returns true if this is a vanilla constructor
  bool is_vanilla_constructor() const;

   // checks method and its method holder
  bool is_final_method() const;
  bool is_strict_method() const;

  // returns true if the method has any backward branches.
  bool has_loops() { 
    return access_flags().loops_flag_init() ? access_flags().has_loops() : compute_has_loops_flag(); 
  };

  bool compute_has_loops_flag();
  
  bool has_jsrs() { 
    return access_flags().has_jsrs();
  };
  void set_has_jsrs() {
    _access_flags.set_has_jsrs();
  }

  // returns true if the method has any monitors.
  bool has_monitors() const                      { return is_synchronized() || access_flags().has_monitor_bytecodes(); } 
  bool has_monitor_bytecodes() const             { return access_flags().has_monitor_bytecodes(); }

  void set_has_monitor_bytecodes()               { _access_flags.set_has_monitor_bytecodes(); }
  
  // monitor matching. This returns a conservative estimate of whether the monitorenter/monitorexit bytecodes
  // propererly nest in the method. It might return false, even though they actually nest properly, since the info.
  // has not been computed yet.
  bool guaranteed_monitor_matching() const       { return access_flags().is_monitor_matching(); }
  void set_guaranteed_monitor_matching()         { _access_flags.set_monitor_matching(); }

  // returns true if the method is an accessor function (setter/getter).
  bool is_accessor() const;

  // returns true if the method is an initializer (<init> or <clinit>).
  bool is_initializer() const;

  // compiled code support
  bool has_compiled_code() const                 { return code() != NULL; }

  // sizing
  static int object_size(bool is_native);
  static int header_size()                       { return sizeof(methodOopDesc)/HeapWordSize; }
  int object_size() const                        { return method_size(); }

  bool object_is_parsable() const                { return method_size() > 0; }

  // interpreter support
  static ByteSize const_offset()                 { return byte_offset_of(methodOopDesc, _constMethod       ); }
  static ByteSize constants_offset()             { return byte_offset_of(methodOopDesc, _constants         ); }
  static ByteSize access_flags_offset()          { return byte_offset_of(methodOopDesc, _access_flags      ); }
#ifdef CC_INTERP
  static ByteSize result_index_offset()          { return byte_offset_of(methodOopDesc, _result_index ); }
#endif /* CC_INTERP */
  static ByteSize size_of_locals_offset()        { return byte_offset_of(methodOopDesc, _max_locals        ); }
  static ByteSize size_of_parameters_offset()    { return byte_offset_of(methodOopDesc, _size_of_parameters); }
#ifndef CORE
  static ByteSize from_compiled_code_entry_point_offset() { return byte_offset_of(methodOopDesc, _from_compiled_code_entry_point); }  
  static ByteSize invocation_counter_offset()    { return byte_offset_of(methodOopDesc, _invocation_counter); }
  static ByteSize backedge_counter_offset()      { return byte_offset_of(methodOopDesc, _backedge_counter); }
  static ByteSize method_data_offset()           { 
    COMPILER1_ONLY(ShouldNotReachHere(); return in_ByteSize(0);)
    COMPILER2_ONLY(return byte_offset_of(methodOopDesc, _method_data);)
  }
#ifdef COMPILER2
  static ByteSize interpreter_invocation_counter_offset() { return byte_offset_of(methodOopDesc, _interpreter_invocation_count); }
#endif // COMPILER2
#ifndef PRODUCT
  static ByteSize compiled_invocation_counter_offset() { return byte_offset_of(methodOopDesc, _compiled_invocation_count); }
#endif // not PRODUCT
#endif // not CORE
  static ByteSize compiled_code_offset()         { return byte_offset_of(methodOopDesc, _code              ); }
  static ByteSize native_function_offset()       { return in_ByteSize(sizeof(methodOopDesc));                 }
  static ByteSize interpreter_entry_offset()     { return byte_offset_of(methodOopDesc, _interpreter_entry ); }
#ifdef COMPILER1
  static ByteSize parameter_info_offset()        { return byte_offset_of(methodOopDesc, _parameter_info    ); }
#endif
  static ByteSize signature_handler_offset()     { return in_ByteSize(sizeof(methodOopDesc) + wordSize);      }
  static ByteSize max_stack_offset()             { return byte_offset_of(methodOopDesc, _max_stack         ); } 

#ifdef COMPILER2
  // for code generation
  static int method_data_offset_in_bytes()       { return (intptr_t)&(((methodOop) oop(NULL))->_method_data); }
  static int interpreter_invocation_counter_offset_in_bytes()       
                                                 { return (intptr_t)&(((methodOop) oop(NULL))->_interpreter_invocation_count); }
#endif // COMPILER2

  // Static methods that are used to implement member methods where an exposed this pointer
  // is needed due to possible GCs
  static objArrayHandle resolved_checked_exceptions_impl(methodOop this_oop, TRAPS);

  // Returns the byte code index from the byte code pointer
  int     bci_from(address bcp) const;
  address bcp_from(int     bci) const;

  // Returns the line number for a bci if debugging information for the method is prowided,
  // -1 is returned otherwise.
  int line_number_from_bci(int bci) const;

  // Reflection support
  bool is_overridden_in(klassOop k) const;

  // Dynamic class reloading (evolution) support
  bool is_old_version() const                       { return access_flags().is_old_version(); }
  void set_old_version()                            { _access_flags.set_is_old_version(); }
  bool is_non_emcp_with_new_version() const         { return access_flags().is_non_emcp_with_new_version(); }
  void set_non_emcp_with_new_version()              { _access_flags.set_is_non_emcp_with_new_version(); }

  // Rewriting support
  static methodHandle clone_with_new_data(methodHandle m, u_char* new_code, int new_code_length, 
                                          u_char* new_compressed_linenumber_table, int new_compressed_linenumber_size, TRAPS);

  // JNI identifier
  jmethodID jmethod_id();

  // Lookup jmethodID for this method.  Return NULL if not found.
  jmethodID find_jmethod_id_or_null();

  // (Note:  Here is the only reason this file requires vmSymbols.hpp.)
#define VM_INTRINSIC_ENUM(id, klass, name, sig)  id,
  enum IntrinsicId {
    _none = 0,                      // not an intrinsic (default answer)
    VM_INTRINSICS_DO(VM_INTRINSIC_ENUM)
    _vm_intrinsics_terminating_enum
  };
#undef VM_INTRINSIC_ENUM

  // Support for inlining of intrinsic methods
  IntrinsicId intrinsic_id() const;              // returns zero if not an intrinsic

  // On-stack replacement support   
  bool has_osr_nmethod()                         { return instanceKlass::cast(method_holder())->lookup_osr_nmethod(this, InvocationEntryBci) != NULL; }
  nmethod* lookup_osr_nmethod_for(int bci)       { return instanceKlass::cast(method_holder())->lookup_osr_nmethod(this, bci); }

  // Inline cache support
  void cleanup_inline_caches();

  // Find if klass for method is loaded
  bool is_klass_loaded_by_klass_index(int klass_index) const;
  bool is_klass_loaded(int refinfo_index, bool must_be_resolved = false) const;

  // Indicates whether compilation failed earlier for this method, or
  // whether it is not compilable for another reason like having a
  // breakpoint set in it.
  bool is_not_compilable(int comp_level = CompLevel_highest_tier) const;
  void set_not_compilable(int comp_level = CompLevel_highest_tier);

  bool is_not_osr_compilable() const             { return is_not_compilable() || access_flags().is_not_osr_compilable(); }
  void set_not_osr_compilable()                  { _access_flags.set_not_osr_compilable(); }

  bool is_not_tier1_compilable() const           { return access_flags().is_not_tier1_compilable(); }
  void set_not_tier1_compilable()                { _access_flags.set_not_tier1_compilable(); }

  // Background compilation support
  bool queued_for_compilation() const            { return access_flags().queued_for_compilation();    }
  void set_queued_for_compilation()              { _access_flags.set_queued_for_compilation(); }
  void clear_queued_for_compilation()            { _access_flags.clear_queued_for_compilation(); }

  static methodOop method_from_bcp(address bcp);

#ifndef CORE
  // Resolve all classes in signature, return 'true' if successful
  static bool load_signature_classes(methodHandle m, TRAPS);

  // Return if true if not all classes references in signature, including return type, has been loaded
  static bool has_unloaded_classes_in_signature(methodHandle m, TRAPS);
#endif // CORE

  // Printing
  void print_short_name(outputStream* st)        /*PRODUCT_RETURN*/; // prints as klassname::methodname; Exposed so field engineers can debug VM
  void print_name(outputStream* st)              PRODUCT_RETURN; // prints as "virtual void foo(int)"

  // Helper routine used for method sorting
  static void sort_methods(objArrayOop methods,
                           objArrayOop methods_annotations,
                           objArrayOop methods_parameter_annotations,
                           objArrayOop methods_default_annotations);

 private:
  // size of parameters
  void set_size_of_parameters(int size)          { _size_of_parameters = size; }

  // Inlined elements
  address* native_function_addr() const          { assert(is_native(), "must be native"); return (address*) (this+1); }
  address* signature_handler_addr() const        { return native_function_addr() + 1; }

  // Garbage collection support
  oop*  adr_constMethod() const                  { return (oop*)&_constMethod;     }
  oop*  adr_constants() const                    { return (oop*)&_constants;       }
#ifdef COMPILER2
  oop*  adr_method_data() const                  { return (oop*)&_method_data;     }
#endif // COMPILER2
};


// Utility class for compressing line number tables

class CompressedLineNumberWriteStream: public CompressedWriteStream {
 private:
  int _bci;
  int _line;
 public:
  // Constructor
  CompressedLineNumberWriteStream(int initial_size);
  // Write (bci, line number) pair to stream
  void write_pair(int bci, int line);
  // Write end-of-stream marker
  void write_terminator()                        { write_byte(0); }
};


// Utility class for decompressing line number tables

class CompressedLineNumberReadStream: public CompressedReadStream {
 private:
  int _bci;
  int _line;
 public:
  // Constructor
  CompressedLineNumberReadStream(u_char* buffer);
  // Read (bci, line number) pair from stream. Returns false at end-of-stream.
  bool read_pair();
  // Accessing bci and line number (after calling read_pair)
  int bci() const                               { return _bci; }
  int line() const                              { return _line; }
};


/// Fast Breakpoints.

// If this structure gets more complicated (because bpts get numerous),
// move it into its own header.

// There is presently no provision for concurrent access
// to breakpoint lists, which is only OK for JVMDI because
// breakpoints are written only at safepoints, and are read
// concurrently only outside of safepoints.

class BreakpointInfo : public CHeapObj {
  friend class VMStructs;
 private:
  Bytecodes::Code  _orig_bytecode;
  int              _bci;
  u2               _name_index;       // of method
  u2               _signature_index;  // of method
  BreakpointInfo*  _next;             // simple storage allocation

 public:
  BreakpointInfo(methodOop m, int bci);

  // accessors
  Bytecodes::Code orig_bytecode()                     { return _orig_bytecode; }
  void        set_orig_bytecode(Bytecodes::Code code) { _orig_bytecode = code; }
  int         bci()                                   { return _bci; }

  BreakpointInfo*          next() const               { return _next; }
  void                 set_next(BreakpointInfo* n)    { _next = n; }

  // helps for searchers
  bool match(methodOop m, int bci) {
    return bci == _bci && match(m);
  }

  bool match(methodOop m) {
    return _name_index == m->name_index() &&
      _signature_index == m->signature_index();
  }

  void set(methodOop method);
  void clear(methodOop method);
};


