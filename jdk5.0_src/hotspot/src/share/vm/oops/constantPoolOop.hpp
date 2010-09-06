#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)constantPoolOop.hpp	1.84 04/02/23 13:35:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A constantPool is an array containing class constants as described in the
// class file.
//
// Most of the constant pool entries are written during class parsing, which
// is safe.  For klass and string types, the constant pool entry is
// modified when the entry is resolved.  If a klass or string constant pool
// entry is read without a lock, only the resolved state guarantees that
// the entry in the constant pool is a klass or String object and
// not a symbolOop.

#ifdef CC_INTERP
class cInterpreter;
#endif /* CC_INTERP */

class constantPoolOopDesc : public arrayOopDesc {
  friend class VMStructs;
#ifdef CC_INTERP
  friend class cInterpreter;  // Directly extracts an oop in the pool for fast instanceof/checkcast
#endif /* CC_INTERP */
 private:
  typeArrayOop         _tags; // the tag array describing the constant pool's contents
  constantPoolCacheOop _cache;         // the cache holding interpreter runtime information
  klassOop             _pool_holder;   // the corresponding class

  typeArrayOop tags() const                    { return _tags; }
  void set_tags(typeArrayOop tags)             { oop_store_without_check((oop*)&_tags, tags); }
  void tag_at_put(int which, jbyte t)          { tags()->byte_at_put(which, t); }
  void release_tag_at_put(int which, jbyte t)  { tags()->release_byte_at_put(which, t); }

 private:
  intptr_t* base() const { return (intptr_t*) (((char*) this) + sizeof(constantPoolOopDesc)); }
  oop* tags_addr()	 { return (oop*)&_tags; }
  oop* cache_addr()      { return (oop*)&_cache; }

  oop* obj_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return (oop*) &base()[which];
  }

  jint* int_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return (jint*) &base()[which];
  }

  jlong* long_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return (jlong*) &base()[which];
  }

  jfloat* float_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return (jfloat*) &base()[which];
  }

  jdouble* double_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return (jdouble*) &base()[which];
  }
 
  friend class ObjectDumper;                // JVMPI support

 public:
  // Klass holding pool
  klassOop pool_holder() const              { return _pool_holder; }
  void set_pool_holder(klassOop k)          { oop_store_without_check((oop*)&_pool_holder, (oop) k); }
  oop* pool_holder_addr()                   { return (oop*)&_pool_holder; }

  // Interpreter runtime support
  constantPoolCacheOop cache() const        { return _cache; }
  void set_cache(constantPoolCacheOop cache){ oop_store((oop*)&_cache, cache); }

  // Assembly code support
  static int tags_offset_in_bytes()         { return (intptr_t)&((constantPoolOopDesc*)NULL)->_tags; }
  static int cache_offset_in_bytes()        { return (intptr_t)&((constantPoolOopDesc*)NULL)->_cache; }
  static int pool_holder_offset_in_bytes()  { return (intptr_t)&((constantPoolOopDesc*)NULL)->_pool_holder; }

  // Storing constants

  void klass_at_put(int which, klassOop k) { 
    oop_store_without_check((volatile oop *)obj_at_addr(which), oop(k));
    // The interpreter assumes when the tag is stored, the klass is resolved
    // and the klassOop is a klass rather than a symbolOop, so we need
    // hardware store ordering here.
    release_tag_at_put(which, JVM_CONSTANT_Class);
  }

  // For temporary use while constructing constant pool
  void klass_index_at_put(int which, int name_index) {
    tag_at_put(which, JVM_CONSTANT_ClassIndex);
    *int_at_addr(which) = name_index; 
  }

  // Temporary until actual use
  void unresolved_klass_at_put(int which, symbolOop s) {
    tag_at_put(which, JVM_CONSTANT_UnresolvedClass);
    oop_store_without_check(obj_at_addr(which), oop(s));
  }

  // Temporary until actual use
  void unresolved_string_at_put(int which, symbolOop s) {
    tag_at_put(which, JVM_CONSTANT_UnresolvedString);
    oop_store_without_check(obj_at_addr(which), oop(s));
  }

  void int_at_put(int which, jint i) { 
    tag_at_put(which, JVM_CONSTANT_Integer);
    *int_at_addr(which) = i; 
  }
  
  void long_at_put(int which, jlong l) { 
    tag_at_put(which, JVM_CONSTANT_Long);
    // *long_at_addr(which) = l; 
    Bytes::put_native_u8((address)long_at_addr(which), *((u8*) &l));
  }

  void float_at_put(int which, jfloat f) { 
    tag_at_put(which, JVM_CONSTANT_Float);
    *float_at_addr(which) = f; 
  }

  void double_at_put(int which, jdouble d) { 
    tag_at_put(which, JVM_CONSTANT_Double);
    // *double_at_addr(which) = d; 
    // u8 temp = *(u8*) &d;
    Bytes::put_native_u8((address) double_at_addr(which), *((u8*) &d));
  }

  void symbol_at_put(int which, symbolOop s) {
    tag_at_put(which, JVM_CONSTANT_Utf8);
    oop_store_without_check(obj_at_addr(which), oop(s));
  }

  void string_at_put(int which, oop str) {
    oop_store((volatile oop*)obj_at_addr(which), str);
    release_tag_at_put(which, JVM_CONSTANT_String);
  }

  void string_index_at_put(int which, int string_index) {    // For temporary use while constructing constant pool
    tag_at_put(which, JVM_CONSTANT_StringIndex);
    *int_at_addr(which) = string_index; 
  }

  void field_at_put(int which, int class_index, int name_and_type_index) {
    tag_at_put(which, JVM_CONSTANT_Fieldref);
    *int_at_addr(which) = ((jint) name_and_type_index<<16) | class_index;
  }

  void method_at_put(int which, int class_index, int name_and_type_index) {
    tag_at_put(which, JVM_CONSTANT_Methodref);
    *int_at_addr(which) = ((jint) name_and_type_index<<16) | class_index;
  }

  void interface_method_at_put(int which, int class_index, int name_and_type_index) {
    tag_at_put(which, JVM_CONSTANT_InterfaceMethodref);
    *int_at_addr(which) = ((jint) name_and_type_index<<16) | class_index;  // Not so nice
  }

  void name_and_type_at_put(int which, int name_index, int signature_index) {
    tag_at_put(which, JVM_CONSTANT_NameAndType);
    *int_at_addr(which) = ((jint) signature_index<<16) | name_index;  // Not so nice
  }
  
  // Tag query

  constantTag tag_at(int which) const { return (constantTag)tags()->byte_at_acquire(which); }

  // Fetching constants

  klassOop klass_at(int which, TRAPS) { 
    constantPoolHandle h_this(THREAD, this);
    return klass_at_impl(h_this, which, CHECK_0); 
  }  
    
  symbolOop klass_name_at(int which);  // Returns the name, w/o resolving.

  klassOop resolved_klass_at(int which) {  // Used by Compiler 
    guarantee(tag_at(which).is_klass(), "Corrupted constant pool");
    // Must do an acquire here in case another thread resolved the klass
    // behind our back, lest we later load stale values thru the oop.
    return klassOop((oop)OrderAccess::load_ptr_acquire(obj_at_addr(which)));
  }

  // This method should only be used with a cpool lock or during parsing or gc
  symbolOop unresolved_klass_at(int which) {     // Temporary until actual use
    symbolOop s = symbolOop((oop)OrderAccess::load_ptr_acquire(obj_at_addr(which)));
    // check that the klass is still unresolved.
    assert(tag_at(which).is_unresolved_klass(), "Corrupted constant pool");
    return s;
  }

#ifdef HOTSWAP
  symbolOop klass_at_noresolve(int which) { return klass_name_at(which); }
#endif HOTSWAP

  jint int_at(int which) {
    assert(tag_at(which).is_int(), "Corrupted constant pool");
    return *int_at_addr(which);
  }

  jlong long_at(int which) {
    assert(tag_at(which).is_long(), "Corrupted constant pool");
    // return *long_at_addr(which);
    u8 tmp = Bytes::get_native_u8((address)&base()[which]);
    return *((jlong*)&tmp);
  }

  jfloat float_at(int which) {
    assert(tag_at(which).is_float(), "Corrupted constant pool");
    return *float_at_addr(which);
  }

  jdouble double_at(int which) {
    assert(tag_at(which).is_double(), "Corrupted constant pool");
    u8 tmp = Bytes::get_native_u8((address)&base()[which]);
    return *((jdouble*)&tmp);
  }

  symbolOop symbol_at(int which) {
    assert(tag_at(which).is_utf8(), "Corrupted constant pool");
    return symbolOop(*obj_at_addr(which));
  }

  oop string_at(int which, TRAPS) {
    constantPoolHandle h_this(THREAD, this);
    return string_at_impl(h_this, which, CHECK_0); 
  }

  // only called when we are sure a string entry is already resolved (via an
  // earlier string_at call.
  oop resolved_string_at(int which) {
    assert(tag_at(which).is_string(), "Corrupted constant pool");
    // Must do an acquire here in case another thread resolved the klass
    // behind our back, lest we later load stale values thru the oop.
    return (oop)OrderAccess::load_ptr_acquire(obj_at_addr(which));
  }

  // This method should only be used with a cpool lock or during parsing or gc
  symbolOop unresolved_string_at(int which) {    // Temporary until actual use
    symbolOop s = symbolOop((oop)OrderAccess::load_ptr_acquire(obj_at_addr(which)));
    // check that the string is still unresolved.
    assert(tag_at(which).is_unresolved_string(), "Corrupted constant pool");
    return s;
  }

  // Returns an UTF8 for a CONSTANT_String entry at a given index.
  // UTF8 char* representation was chosen to avoid conversion of
  // java_lang_Strings at resolved entries into symbolOops
  // or vice versa.
  char* string_at_noresolve(int which);

  jint name_and_type_at(int which) {
    assert(tag_at(which).is_name_and_type(), "Corrupted constant pool");
    return *int_at_addr(which);
  }

  // The following methods (klass_ref_at, klass_ref_at_noresolve, name_ref_at,
  // signature_ref_at, klass_ref_index_at, name_and_type_ref_index_at,
  // name_ref_index_at, signature_ref_index_at) all expect constant pool indices
  // from the bytecodes to be passed in, which are actually potentially byte-swapped
  // contstant pool cache indices. See field_or_method_at.

  // Lookup for entries consisting of (klass_index, name_and_type index)
  klassOop klass_ref_at(int which, TRAPS);
#ifdef HOTSWAP
  symbolOop klass_ref_at_noresolve(int which);
#endif HOTSWAP
  symbolOop name_ref_at(int which);
  symbolOop signature_ref_at(int which);    // the type descriptor
  
  int klass_ref_index_at(int which);
  int name_and_type_ref_index_at(int which);

  // Lookup for entries consisting of (name_index, signature_index)
  int name_ref_index_at(int which);
  int signature_ref_index_at(int which);

  BasicType basic_type_for_signature_at(int which);

  // Resolve string constants (to prevent allocation during compilation)
  void resolve_string_constants(TRAPS) {
    constantPoolHandle h_this(THREAD, this);
    resolve_string_constants_impl(h_this, CHECK); 
  }

  // Klass name matches name at offset
  bool klass_name_at_matches(instanceKlassHandle k, int which);

  // Sizing
  static int header_size()             { return sizeof(constantPoolOopDesc)/HeapWordSize; }
  static int object_size(int length)   { return align_object_size(header_size() + length); }
  int object_size()                    { return object_size(length()); }

  friend class constantPoolKlass;
  friend class ClassFileParser;
  friend class SystemDictionary;

  // Used by compiler to prevent classloading. 
  static klassOop klass_at_if_loaded          (constantPoolHandle this_oop, int which);  
  static klassOop klass_ref_at_if_loaded      (constantPoolHandle this_oop, int which);
  // Same as above - but does LinkResolving.
  static klassOop klass_ref_at_if_loaded_check(constantPoolHandle this_oop, int which, TRAPS);  

  // Routines currently used for annotations (only called by jvm.cpp) but which might be used in the
  // future by other Java code. These take constant pool indices rather than possibly-byte-swapped
  // constant pool cache indices as do the peer methods above.
  symbolOop uncached_name_ref_at(int which);
  symbolOop uncached_signature_ref_at(int which);
  int       uncached_klass_ref_index_at(int which);
  int       uncached_name_and_type_ref_index_at(int which);

  // Sharing
  int pre_resolve_shared_klasses(TRAPS);
  void shared_symbols_iterate(OopClosure* closure0);
  void shared_tags_iterate(OopClosure* closure0);
  void shared_strings_iterate(OopClosure* closure0);

  // Debugging
  const char* printable_name_at(int which) PRODUCT_RETURN0;

 private:

  // Takes either a constant pool cache index in possibly byte-swapped
  // byte order (which comes from the bytecodes after rewriting) or,
  // if "uncached" is true, a vanilla constant pool index
  jint field_or_method_at(int which, bool uncached) {
    int i = -1;
    if (uncached || cache() == NULL) {
      i = which;
    } else {
      // change byte-ordering and go via cache
      i = cache()->entry_at(Bytes::swap_u2(which))->constant_pool_index();
    }
    assert(tag_at(i).is_field_or_method(), "Corrupted constant pool");
    return *int_at_addr(i);
  }

  // Used while constructing constant pool (only by ClassFileParser)
  jint klass_index_at(int which) {
    assert(tag_at(which).is_klass_index(), "Corrupted constant pool");
    return *int_at_addr(which);
  }

  jint string_index_at(int which) {
    assert(tag_at(which).is_string_index(), "Corrupted constant pool");
    return *int_at_addr(which);
  }

  // Performs the LinkResolver checks
  static void verify_constant_pool_resolve(constantPoolHandle this_oop, KlassHandle klass, TRAPS);

  // Implementation of methods that needs an exposed 'this' pointer, in order to 
  // handle GC while executing the method
  static klassOop klass_at_impl(constantPoolHandle this_oop, int which, TRAPS);
  static oop string_at_impl(constantPoolHandle this_oop, int which, TRAPS);

  // Resolve string constants (to prevent allocation during compilation)
  static void resolve_string_constants_impl(constantPoolHandle this_oop, TRAPS);
};
