#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciInstanceKlass.hpp	1.19 03/12/23 16:39:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciInstanceKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an instanceKlass.  It may or may not
// be loaded.
class ciInstanceKlass : public ciKlass {
  CI_PACKAGE_ACCESS
  friend class ciEnv;
  friend class ciMethod;
  friend class ciField;
  friend class ciByteCodeStream;
  friend class ciBytecodeStream;  // the lowercase 'c' version

private:
  bool                   _is_shared;

  jobject                _loader;
  jobject                _protection_domain;

  bool                   _is_initialized;
  bool                   _is_being_initialized;
  bool                   _has_finalizer;
  bool                   _has_subklass;
  ciFlags                _flags;
  jint                   _size_helper;
  jint                   _nonstatic_field_size;
  
  // Lazy fields get filled in only upon request.
  ciInstanceKlass*       _super;
  ciInstance*            _java_mirror;

  ciConstantPoolCache*   _field_cache;

  ciInstanceKlass*       _implementor;
  bool                   _checked_implementor;

protected:
  ciInstanceKlass(KlassHandle h_k);
  ciInstanceKlass(ciSymbol* name, jobject loader, jobject protection_domain)
    : ciKlass(name, ciInstanceKlassKlass::make()), _implementor(NULL), _checked_implementor(false) {
    assert(name->byte_at(0) != '[', "not an instance klass");
    _loader = loader;
    _protection_domain = protection_domain;
    _is_initialized = false;
    _field_cache = NULL;
  }

  instanceKlass* get_instanceKlass() const {
    return (instanceKlass*)get_Klass();
  }

  oop loader();
  jobject loader_handle();

  oop protection_domain();
  jobject protection_domain_handle();

  const char* type_string() { return "ciInstanceKlass"; }

  void print_impl();

  ciConstantPoolCache* field_cache();

  bool compute_shared_is_initialized();
  bool compute_shared_has_subklass();

public:
  // Has this klass been initialized?
  bool                   is_initialized() {
    if (_is_shared && !_is_initialized) {
      return is_loaded() && compute_shared_is_initialized();
    }
    return _is_initialized;
  }
  bool is_being_initialized() const { 
    return _is_being_initialized;
  }

  // General klass information.
  ciFlags                flags()          {
    assert(is_loaded(), "must be loaded");
    return _flags;
  }
  bool                   has_finalizer()  {
    assert(is_loaded(), "must be loaded");
    return _has_finalizer; }
  bool                   has_subklass()   {
    assert(is_loaded(), "must be loaded");
    if (_is_shared && !_has_subklass) {
      if (flags().is_final()) {
        return false;
      } else {
        return compute_shared_has_subklass();
      }
    }
    return _has_subklass;
  }
  jint                   size_helper()  {
    assert(is_loaded(), "must be loaded");
    return _size_helper; }
  jint                   nonstatic_field_size()  {
    assert(is_loaded(), "must be loaded");
    return _nonstatic_field_size; }
  ciInstanceKlass*       super();

  ciInstanceKlass* get_canonical_holder(int offset);
  ciField* get_field_by_offset(int field_offset, bool is_static);

  ciInstanceKlass* unique_concrete_subklass();

  bool contains_field_offset(int offset) {
      return (offset/wordSize) >= instanceOopDesc::header_size()
             && (offset/wordSize)-instanceOopDesc::header_size() < nonstatic_field_size();
  }

  // Get the instance of java.lang.Class corresponding to
  // this klass.  This instance is used for locking of
  // synchronized static methods of this klass.
  ciInstance*            java_mirror();

  // Java access flags
  bool is_public      () { return flags().is_public(); }
  bool is_final       () { return flags().is_final(); }
  bool is_super       () { return flags().is_super(); }
  bool is_interface   () { return flags().is_interface(); }
  bool is_abstract    () { return flags().is_abstract(); }

  ciMethod* find_method(ciSymbol* name, ciSymbol* signature);
  // Note:  To find a method from name and type strings, use ciSymbol::make,
  // but consider adding to vmSymbols.hpp instead.
  ciMethod* find_method(int method_index);  // slot # from reflection API

  ciInstanceKlass* implementor();

  // Is the defining class loader of this class the default loader?
  bool uses_default_loader();

  bool is_java_lang_Object();

  // What kind of ciObject is this?
  bool is_instance_klass() { return true; }
  bool is_java_klass()     { return true; }
};

