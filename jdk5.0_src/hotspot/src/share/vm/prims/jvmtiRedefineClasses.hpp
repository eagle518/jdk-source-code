#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiRedefineClasses.hpp	1.4 04/02/17 20:16:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_RedefineClasses: public VM_Operation {
 private:
  static objArrayOop _old_methods;
  static objArrayOop _new_methods;  
  static klassOop _evolving_koop;
  static constantPoolOop _old_constants;

  jint _class_count;
  const jvmtiClassDefinition *_class_defs;
  instanceKlassHandle *_k_h_new;
  jvmtiError _res;

  jvmtiError load_new_class_versions(TRAPS);

  jvmtiError compare_class_versions(instanceKlassHandle k_h_old, instanceKlassHandle k_h_new);

  void patch_indexes_for_fields(instanceKlassHandle k_h, instanceKlassHandle k_h_new);

  // Unevolving classes may point to methods of the evolving class directly
  // from their constantpool caches and vtables/itables. Fix this. 
  static void adjust_cpool_cache_and_vtable(klassOop k_oop, oop loader);

  void check_methods_and_mark_as_old();
  void transfer_old_native_function_registrations();
  void flush_method_jmethod_id_cache();

  // Install the redefinition of a class 
  void redefine_single_class(jclass j_clazz, instanceKlassHandle k_h_new, TRAPS); 

 public:
  VM_RedefineClasses(jint class_count, const jvmtiClassDefinition *class_defs);

  bool doit_prologue();
  void doit();
  void doit_epilogue();

  const char* name() const                       { return "VM_RedefineClasses"; }
  bool allow_nested_vm_operations() const        { return true; }
  jvmtiError check_error()                       { return _res; }
};
