#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)linkResolver.hpp	1.66 03/12/23 16:40:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// All the necessary definitions for run-time link resolution.

// LinkInfo & its subclasses provide all the information gathered
// for a particular link after resolving it. A link is any reference
// made from within the bytecodes of a method to an object outside of
// that method. If the info is invalid, the link has not been resolved
// successfully.

class LinkInfo VALUE_OBJ_CLASS_SPEC {
};


// Link information for getfield/putfield & getstatic/putstatic bytecodes.

class FieldAccessInfo: public LinkInfo {
 protected:
  KlassHandle  _klass;
  symbolHandle _name;
  AccessFlags  _access_flags;
  int          _field_index;  // original index in the klass
  int          _field_offset;
  BasicType    _field_type;
  
 public:
  void         set(KlassHandle klass, symbolHandle name, int field_index, int field_offset,
                 BasicType field_type, AccessFlags access_flags);
  KlassHandle  klass() const                     { return _klass; }
  symbolHandle name() const                      { return _name; }
  int          field_index() const               { return _field_index; }
  int          field_offset() const              { return _field_offset; }
  BasicType    field_type() const                { return _field_type; }
  AccessFlags  access_flags() const              { return _access_flags; }

  // debugging
  void print()  PRODUCT_RETURN;
};


// Link information for all calls.

class CallInfo: public LinkInfo {
 private:
  KlassHandle  _resolved_klass;         // static receiver klass
  KlassHandle  _selected_klass;         // dynamic receiver class (same as static, or subklass)
  methodHandle _resolved_method;        // static target method
  methodHandle _selected_method;        // dynamic (actual) target method
  int          _vtable_index;           // vtable index of selected method

  void         set(KlassHandle resolved_klass,                             methodHandle resolved_method                                                , TRAPS);
  void         set(KlassHandle resolved_klass, KlassHandle selected_klass, methodHandle resolved_method, methodHandle selected_method                  , TRAPS);
  void         set(KlassHandle resolved_klass, KlassHandle selected_klass, methodHandle resolved_method, methodHandle selected_method, int vtable_index, TRAPS);

  friend class LinkResolver;

 public:
  KlassHandle  resolved_klass() const            { return _resolved_klass; }
  KlassHandle  selected_klass() const            { return _selected_klass; }
  methodHandle resolved_method() const           { return _resolved_method; }
  methodHandle selected_method() const           { return _selected_method; }

  BasicType    result_type() const               { return selected_method()->result_type(); }
  bool         has_vtable_index() const          { return _vtable_index >= 0; }
  // For interface calls the vtable index is -1 unless the method is
  // declared in java/lang/Object and is therefore not in the itable.
  // In this case the call must be treated like a virtual call.
  int          vtable_index() const              { return _vtable_index; }
};


// The LinkResolver is used to resolve constant-pool references at run-time.
// It does all necessary link-time checks & throws exceptions if necessary.

class LinkResolver: AllStatic {
 private:
  static void lookup_method_in_klasses          (methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS);
  static void lookup_instance_method_in_klasses (methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS);
  static void lookup_method_in_interfaces       (methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS);
  
  static int vtable_index_of_miranda_method(KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS);

  static void resolve_klass           (KlassHandle& result, constantPoolHandle  pool, int index, TRAPS); 
  static void resolve_klass_no_update (KlassHandle& result, constantPoolHandle pool, int index, TRAPS); // no update of constantPool entry
  
  static void resolve_pool  (KlassHandle& resolved_klass, symbolHandle& method_name, symbolHandle& method_signature, KlassHandle& current_klass, constantPoolHandle pool, int index, TRAPS);
  
  static void resolve_interface_method(methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
  static void resolve_method          (methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
  
  static void linktime_resolve_static_method    (methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
  static void linktime_resolve_special_method   (methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
  static void linktime_resolve_virtual_method   (methodHandle &resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature,KlassHandle current_klass, bool check_access, TRAPS);
  static void linktime_resolve_interface_method (methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
    
  static void runtime_resolve_special_method    (CallInfo& result, methodHandle resolved_method, KlassHandle resolved_klass, KlassHandle current_klass, bool check_access, TRAPS);
  static void runtime_resolve_virtual_method    (CallInfo& result, methodHandle resolved_method, KlassHandle resolved_klass, Handle recv, KlassHandle recv_klass, bool check_null_and_abstract, TRAPS);
  static void runtime_resolve_interface_method  (CallInfo& result, methodHandle resolved_method, KlassHandle resolved_klass, Handle recv, KlassHandle recv_klass, bool check_null_and_abstract, TRAPS);
  
  static void check_field_accessability   (KlassHandle ref_klass, KlassHandle resolved_klass, KlassHandle sel_klass, fieldDescriptor& fd, TRAPS);
  static void check_method_accessability  (KlassHandle ref_klass, KlassHandle resolved_klass, KlassHandle sel_klass, methodHandle sel_method, TRAPS);  
  
 public:
  // constant pool resolving
  static void check_klass_accessability(KlassHandle ref_klass, KlassHandle sel_klass, TRAPS);  

  // static resolving for all calls except interface calls
  static void resolve_method          (methodHandle& method_result, KlassHandle& klass_result, constantPoolHandle pool, int index, TRAPS);
  static void resolve_interface_method(methodHandle& method_result, KlassHandle& klass_result, constantPoolHandle pool, int index, TRAPS);
  
  // runtime/static resolving for fields
  static void resolve_field(FieldAccessInfo& result, constantPoolHandle pool, int index, Bytecodes::Code byte, bool check_only, TRAPS);
  // takes an extra bool argument "update_pool" to decide whether to update the constantPool during klass resolution.
  static void resolve_field(FieldAccessInfo& result, constantPoolHandle pool, int index, Bytecodes::Code byte, bool check_only, bool update_pool, TRAPS);

  // runtime resolving:
  //   resolved_klass = specified class (i.e., static receiver class)
  //   current_klass  = sending method holder (i.e., class containing the method containing the call being resolved)
  static void resolve_static_call   (CallInfo& result,              KlassHandle& resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, bool initialize_klass, TRAPS);
  static void resolve_special_call  (CallInfo& result,              KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS);
  static void resolve_virtual_call  (CallInfo& result, Handle recv, KlassHandle recv_klass, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, bool check_null_and_abstract, TRAPS);
  static void resolve_interface_call(CallInfo& result, Handle recv, KlassHandle recv_klass, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access, bool check_null_and_abstract, TRAPS);

  // same as above for compile-time resolution; but returns null handle instead of throwing an exception on error
  // also, does not initialize klass (i.e., no side effects)
  static methodHandle resolve_virtual_call_or_null  (KlassHandle receiver_klass, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass);
  static methodHandle resolve_interface_call_or_null(KlassHandle receiver_klass, KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass);
  static methodHandle resolve_static_call_or_null   (KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass);
  static methodHandle resolve_special_call_or_null  (KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass);

  // static resolving for compiler (does not throw exceptions, returns null handle if unsuccessful)
  static methodHandle linktime_resolve_virtual_method_or_null  (KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access);
  static methodHandle linktime_resolve_interface_method_or_null(KlassHandle resolved_klass, symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, bool check_access);

  // runtime resolving from constant pool
  static void resolve_invokestatic   (CallInfo& result,              constantPoolHandle pool, int index, TRAPS);
  static void resolve_invokespecial  (CallInfo& result,              constantPoolHandle pool, int index, TRAPS);
  static void resolve_invokevirtual  (CallInfo& result, Handle recv, constantPoolHandle pool, int index, TRAPS);
  static void resolve_invokeinterface(CallInfo& result, Handle recv, constantPoolHandle pool, int index, TRAPS);

  static void resolve_invoke         (CallInfo& result, Handle recv, constantPoolHandle pool, int index, Bytecodes::Code byte, TRAPS);
};

