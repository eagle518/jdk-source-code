#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciMethod.hpp	1.40 04/03/02 02:08:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef COMPILER1
class ciLocalMap;
#endif
#ifdef COMPILER2
class MethodLiveness;
class BitMap;
#endif


// ciMethod
//
// This class represents a methodOop in the HotSpot virtual
// machine.
class ciMethod : public ciObject {
  friend class CompileBroker;
  CI_PACKAGE_ACCESS
  friend class ciEnv;
  friend class ciExceptionHandlerStream;

 public:
#define VM_INTRINSIC_ENUM(id, klass, name, sig)  id,
  enum IntrinsicId {
    _none = 0,                      // not an intrinsic (default answer)
    VM_INTRINSICS_DO(VM_INTRINSIC_ENUM)
    _vm_intrinsics_terminating_enum
  };
#undef VM_INTRINSIC_ENUM

 private:
  // General method information.
  ciFlags          _flags;
  ciSymbol*        _name;
  ciInstanceKlass* _holder;
  ciSignature*     _signature;
  ciMethodData*    _method_data;

  // Code attributes.
  int _code_size;
  int _max_stack;
  int _max_locals;
  IntrinsicId _intrinsic_id;
  int _handler_count;

  bool _uses_monitors;
  bool _balanced_monitors;
  bool _is_compilable;

  // Lazy fields, filled in on demand
  address              _code;
  ciExceptionHandler** _exception_handlers;

#ifdef COMPILER1
  ciLocalMap*     _all_oop_maps;
#endif // COMPILER1
  // Optional liveness analyzer.
#ifdef COMPILER2
  MethodLiveness* _liveness;
  ciTypeFlow*     _flow;
#endif

  ciMethod(methodHandle h_m);
  ciMethod(ciInstanceKlass* holder, ciSymbol* name, ciSymbol* signature);

  methodOop get_methodOop() const {
    methodOop m = (methodOop)get_oop();
    assert(m != NULL, "illegal use of unloaded method");
    return m;
  }

  oop loader() const                             { return _holder->loader(); }

  const char* type_string()                      { return "ciMethod"; }

  void print_impl();

  void load_code();

  void check_is_loaded() const                   { assert(is_loaded(), "not loaded"); }

  void build_method_data(methodHandle h_m);

  void code_at_put(int bci, Bytecodes::Code code) {
    Bytecodes::check(code);
    assert(0 <= bci && bci < code_size(), "valid bci");
    address bcp = _code + bci;
    *bcp = code;
  }

 public:
  // Basic method information.
  ciFlags flags() const                          { check_is_loaded(); return _flags; }
  ciSymbol* name() const                         { return _name; }
  ciInstanceKlass* holder() const                { return _holder; }
  ciMethodData* method_data() const 		 { return _method_data; }
  
  // Signature information.
  ciSignature* signature() const                 { return _signature; }
  ciType*      return_type() const               { return _signature->return_type(); }
  int          arg_size_no_receiver() const      { return _signature->size(); }
  int          arg_size() const                  { return _signature->size() + (_flags.is_static() ? 0 : 1); }

  // Method code and related information.
  address code()                                 { if (_code == NULL) load_code(); return _code; }
  int code_size() const                          { check_is_loaded(); return _code_size; }
  int max_stack() const                          { check_is_loaded(); return _max_stack; }
  int max_locals() const                         { check_is_loaded(); return _max_locals; }
  IntrinsicId intrinsic_id() const               { check_is_loaded(); return _intrinsic_id; }
  bool has_exception_handlers() const            { check_is_loaded(); return _handler_count > 0; }
  int exception_table_length() const             { check_is_loaded(); return _handler_count; }

  Bytecodes::Code java_code_at_bci(int bci) {
    address bcp = code() + bci;
    return Bytecodes::java_code(Bytecodes::cast(*bcp));
  }

  bool    has_linenumber_table() const;          // length unknown until decompression
  u_char* compressed_linenumber_table() const;   // not preserved by gc

  // Runtime information.
  int           vtable_index();
  address       native_entry();
  address       interpreter_entry();

  // Analysis and profiling.
  //
  // Usage note: liveness_at_bci and init_vars should be wrapped in ResourceMarks.
  bool          uses_monitors() const            { return _uses_monitors; } // this one should go away, it has a misleading name
  bool          has_monitor_bytecodes() const    { return _uses_monitors; }
  bool          has_balanced_monitors();
#ifdef COMPILER1
  ciLocalMap*      all_oop_maps();
#endif
#ifdef COMPILER2
  BitMap        liveness_at_bci(int bci);
  BitMap        init_vars();
#endif

  ciTypeFlow*   get_flow_analysis();
  ciTypeFlow*   get_osr_flow_analysis(int osr_bci);  // alternate entry point
  ciCallProfile call_profile_at_bci(int bci);

  int           interpreter_invocation_count();
  int           interpreter_call_site_count(int bci);
  int           interpreter_throwout_count() const;
  
  // Given a certain calling environment, find the monomorphic target
  // for the call.  Return NULL if the call is not monomorphic in
  // its calling environment.
  ciMethod* find_monomorphic_target(ciKlass* caller, ciKlass* callee_holder, ciKlass* actual_receiver);

  // Compilation directives
  bool will_link(ciKlass* accessing_klass, 
		 ciKlass* declared_method_holder,
		 Bytecodes::Code bc);
  bool should_exclude();
  bool should_inline();
  bool should_print_assembly();
  bool break_at_execute();
  bool can_be_compiled();
  bool can_be_osr_compiled(int entry_bci);
  void set_not_compilable();
  bool has_compiled_code();
  int  instructions_size();
  bool is_not_reached(int bci);
  bool was_executed_more_than(int times);
  bool has_unloaded_classes_in_signature();
  bool is_klass_loaded(int refinfo_index, bool must_be_resolved) const;
  bool check_call(int refinfo_index, bool is_static) const;
  void build_method_data();  // make sure it exists in the VM also
  int scale_count(int count);  // make MDO count commensurate with IIC

  // What kind of ciObject is this?
  bool is_method()                               { return true; }

  // Java access flags
  bool is_public      () const                   { return flags().is_public(); }
  bool is_private     () const                   { return flags().is_private(); }
  bool is_protected   () const                   { return flags().is_protected(); }
  bool is_static      () const                   { return flags().is_static(); }
  bool is_final       () const                   { return flags().is_final(); }
  bool is_synchronized() const                   { return flags().is_synchronized(); }
  bool is_native      () const                   { return flags().is_native(); }
  bool is_interface   () const                   { return flags().is_interface(); }
  bool is_abstract    () const                   { return flags().is_abstract(); }
  bool is_strict      () const                   { return flags().is_strict(); }

  // Other flags
  bool is_empty_method() const;
  bool is_vanilla_constructor() const;
  bool is_final_method() const                   { return is_final() || holder()->is_final(); }
  bool has_loops      () const;
  bool has_jsrs       () const;
  bool is_accessor    () const;
  bool is_initializer () const;

  // Print the bytecodes of this method.
  void print_codes();
  void print_codes(int from, int to);

  // Print the name of this method in various incarnations.
  void print_name();
  void print_short_name();
};

