#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)library_call.cpp	1.119 04/07/14 07:30:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_library_call.cpp.incl"

class LibraryIntrinsic : public InlineCallGenerator {
  // Extend the set of intrinsics known to the runtime:
 public:
  enum C2_IntrinsicId {
    _lower_limit = ciMethod::_vm_intrinsics_terminating_enum,

    _Array_newInstance,                // java.lang.reflect.Array.newInstance
    _getCallerClass,                   // sun.reflect.Reflection.getCallerClass
    _AtomicLong_get,                   // sun.util.AtomicLongCSImpl.get
    _AtomicLong_attemptUpdate,         // sun.util.AtomicLongCSImpl.attemptUpdate
    _upper_limit
  };
 private:
  bool           _is_virtual;
  C2_IntrinsicId _intrinsic_id;

 public:
  LibraryIntrinsic(ciMethod* m, bool is_virtual, C2_IntrinsicId id)
    : InlineCallGenerator(m),
      _is_virtual(is_virtual),
      _intrinsic_id(id)
  {
  }
  virtual bool is_intrinsic() const { return true; }
  virtual bool is_virtual()   const { return _is_virtual; }
  virtual JVMState* generate(JVMState* jvms);
  C2_IntrinsicId intrinsic_id() const { return _intrinsic_id; }
};

// Local helper class for LibraryIntrinsic:
class LibraryCallKit : public GraphKit {
 private:
  LibraryIntrinsic* _intrinsic;   // the library intrinsic being called

 public:
  LibraryCallKit(JVMState* caller, LibraryIntrinsic* intrinsic)
    : GraphKit(caller),
      _intrinsic(intrinsic)
  {
  }

  typedef LibraryIntrinsic::C2_IntrinsicId C2_IntrinsicId;

  ciMethod*         caller()    const    { return jvms()->caller()->method(); }
  int               bci()       const    { return jvms()->bci(); }
  LibraryIntrinsic* intrinsic() const    { return _intrinsic; }
  C2_IntrinsicId    intrinsic_id() const { return _intrinsic->intrinsic_id(); }
  ciMethod*         callee()    const    { return _intrinsic->method(); }
  ciSignature*      signature() const    { return callee()->signature(); }
  int               arg_size()  const    { return callee()->arg_size(); }

  bool try_to_inline();

  // Helper functions to inline natives
  void generate_guard(Node* test, RegionNode* region, float true_prob);
  void generate_slow_guard(Node* test, RegionNode* region);
  void generate_fast_guard(Node* test, RegionNode* region);
  Node* generate_current_thread(Node* &tls_output);

  bool inline_string_compareTo();
  Node* pop_math_arg();
  bool runtime_trig(address funcAddr, const char* funcName);
  bool inline_math_native(ciMethod::IntrinsicId id);
  bool inline_sincos( ciMethod::IntrinsicId id );
  bool inline_pow( ciMethod::IntrinsicId id );
  bool inline_unsafe_access(bool is_native_ptr, bool offset_is_long, bool is_store, BasicType type, bool is_volatile);
  bool inline_unsafe_allocate();
  bool inline_native_currentThread();
  bool inline_native_time_funcs(bool isNano);
  bool inline_native_isInterrupted();
  bool inline_native_Class_query(ciMethod::IntrinsicId id);
  bool inline_native_Array_newInstance();
  bool inline_native_Reflection_getCallerClass();
  bool inline_native_AtomicLong_get();
  bool inline_native_AtomicLong_attemptUpdate();
  bool is_method_invoke_or_aux_frame(JVMState* jvms);
  // Helper function for inlining native object hash method
  bool inline_native_hashcode(bool is_virtual, bool is_static);
  bool inline_native_getClass();
  // Helper function for inlining arraycopy
  bool inline_arraycopy();
  bool inline_unsafe_CAS(BasicType type);
  bool inline_fp_conversions(ciMethod::IntrinsicId id);
};


//---------------------------make_vm_intrinsic----------------------------
CallGenerator* Compile::make_vm_intrinsic(ciMethod* m, bool is_virtual) {
  ciMethod::IntrinsicId id = m->intrinsic_id();
  assert(id != ciMethod::_none, "must be a VM intrinsic");

  if (!m->is_loaded()) {
    // do not attempt to inline unloaded methods
    return NULL;
  }

  // Only a few intrinsics implement a virtual dispatch:
  if (is_virtual) {
    switch (id) {
    case ciMethod::_hash:
      break;  // OK, Object.hashCode intrinsic comes in both flavors
    default:
      return NULL;
    }
  }

  // -XX:-InlineNatives disables nearly all intrinsics:
  if (!InlineNatives) {
    switch (id) {
    case ciMethod::_compareTo:
      break;  // InlineNatives does not control String.compareTo
    default:
      return NULL;
    }
  }

  switch (id) {
  case ciMethod::_compareTo:
    if (!SpecialStringCompareTo)  return NULL;
    break;
  case ciMethod::_arraycopy:
    if (!InlineArrayCopy)  return NULL;
    break;
  case ciMethod::_hash:
    if (!InlineObjectHash)  return NULL;
    break;
  case ciMethod::_dtan:
    if (!Matcher::tanDSupported())  return NULL;
    break;
  case ciMethod::_datan2:
    if (!Matcher::atanDSupported())  return NULL;
    break;
  case ciMethod::_dsqrt:
    if (!Matcher::sqrtDSupported())  return NULL;
    break;
  case ciMethod::_dpow:
    if (!Matcher::powDSupported())  return NULL;
    break; 
  case ciMethod::_checkIndex:
    // We do not intrinsify this.  The optimizer does fine with it.
    return NULL;

 default:
    break;
  }

  // -XX:-InlineClassNatives disables natives from the Class class.
  if (m->holder()->name() == ciSymbol::java_lang_Class()) {
    if (!InlineClassNatives)  return NULL;
  }

  // -XX:-InlineThreadNatives disables natives from the Thread class.
  if (m->holder()->name() == ciSymbol::java_lang_Thread()) {
    if (!InlineThreadNatives)  return NULL;
  }

  // -XX:-InlineMathNatives disables natives from the Math,Float and Double classes.
  if (m->holder()->name() == ciSymbol::java_lang_Math() ||
      m->holder()->name() == ciSymbol::java_lang_Float() ||
      m->holder()->name() == ciSymbol::java_lang_Double()) {
    if (!InlineMathNatives)  return NULL;
  }

  // -XX:-InlineUnsafeOps disables natives from the Unsafe class.
  if (m->holder()->name() == ciSymbol::sun_misc_Unsafe()) {
    if (!InlineUnsafeOps)  return NULL;
  }

  return new LibraryIntrinsic(m, is_virtual, (LibraryIntrinsic::C2_IntrinsicId)id);
}

//----------------------register_library_intrinsics-----------------------
// Register here any intrinsics which are not presently known to the runtime.
void Compile::register_library_intrinsics() {
  ciMethod*                    methods[LibraryIntrinsic::_upper_limit];
  LibraryIntrinsic::C2_IntrinsicId ids[LibraryIntrinsic::_upper_limit];
  int ni = 0;

  ciInstanceKlass* object = env()->Object_klass();
  ciInstanceKlass* clazz = env()->Class_klass();

  ciInstanceKlass* system =
    env()->find_system_klass(ciSymbol::java_lang_System())->as_instance_klass();
  assert(system->is_loaded(), "must be able to find");

  ciInstanceKlass* array =
    env()->find_system_klass(ciSymbol::make("java/lang/reflect/Array"))->as_instance_klass();

  ciInstanceKlass* atomicLong =
    env()->find_system_klass(ciSymbol::make("sun/misc/AtomicLongCSImpl"))->as_instance_klass();

#define OBJ "Ljava/lang/Object;"
#define CLS "Ljava/lang/Class;"

  // Collect intrinsics here, as enabled by command line options:

  if(    Universe::is_gte_jdk14x_version()
      && UseNewReflection
      && InlineReflectionGetCallerClass ) {
    ciInstanceKlass* reflection =
      env()->find_system_klass(ciSymbol::make("sun/reflect/Reflection"))->as_instance_klass();
    if (reflection->is_loaded()) {
      methods[ni] =
        reflection->find_method(ciSymbol::make("getCallerClass"),
                                ciSymbol::make("(I)Ljava/lang/Class;"));
      ids[ni++] = LibraryIntrinsic::_getCallerClass;
    }
  }

  if (InlineClassNatives && array->is_loaded()) { // %%% InlineNewInstance ?
    methods[ni] =
      array->find_method(ciSymbol::make("newArray"),
                         ciSymbol::make("("CLS"I)"OBJ""));
    ids[ni++] = LibraryIntrinsic::_Array_newInstance;
  }

  if (InlineAtomicLong && atomicLong->is_loaded()) {
    methods[ni] =
      atomicLong->find_method(ciSymbol::make("get"),
                         ciSymbol::make("()J"));
    ids[ni++] = LibraryIntrinsic::_AtomicLong_get;
    methods[ni] =
      atomicLong->find_method(ciSymbol::make("attemptUpdate"),
                         ciSymbol::make("(JJ)Z"));
    ids[ni++] = LibraryIntrinsic::_AtomicLong_attemptUpdate;
  }

  // Now register them all.
  assert(ni <= LibraryIntrinsic::_upper_limit, "oob");
  while (--ni >= 0) {
    ciMethod* m = methods[ni];
#ifndef PRODUCT
    if (m == NULL)
      tty->print_cr("*** intrinsic %d[%d] not found", ids[ni], ni);
#endif
    register_intrinsic(new LibraryIntrinsic(m, /*!virtual*/ false, ids[ni]));
  }

#undef OBJ
#undef CLS
}


JVMState* LibraryIntrinsic::generate(JVMState* jvms) {
  debug_only(Compile* C = Compile::current());
  assert(!C->need_jvmpi_method_event(), "don't attempt this if jvmpi is on");

  LibraryCallKit kit(jvms, this);
  if (kit.try_to_inline()) {
    if (PrintInlining || PrintOptoInlining) {
      tty->print("  Inlining intrinsic ");
      kit.callee()->print_name();
      tty->cr();
    }
    return kit.transfer_exceptions_into_jvms();
  }

  return NULL;
}

bool LibraryCallKit::try_to_inline() {
  // Handle symbolic names for otherwise undistinguished boolean switches:
  const bool is_store       = true;
  const bool is_native_ptr  = true;
  const bool is_static      = true;
  const bool offset_is_long = true;

  switch (intrinsic_id()) {
  case ciMethod::_hash:
    return inline_native_hashcode(intrinsic()->is_virtual(),      !is_static);
  case ciMethod::_identityHash:
    return inline_native_hashcode(/*!virtual*/ false, is_static);
  case ciMethod::_getClass:
    return inline_native_getClass();

  case ciMethod::_dsin:
  case ciMethod::_dcos:
  case ciMethod::_dtan:
  case ciMethod::_datan2:
  case ciMethod::_dsqrt:
  case ciMethod::_dpow:
    return inline_math_native((ciMethod::IntrinsicId) intrinsic_id());

  case ciMethod::_arraycopy:
    return inline_arraycopy();

  case ciMethod::_compareTo:
    return inline_string_compareTo();

  case ciMethod::_getObject_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_OBJECT, false);
  case ciMethod::_getBoolean_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_BOOLEAN, false);
  case ciMethod::_getByte_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_BYTE, false);
  case ciMethod::_getShort_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_SHORT, false);
  case ciMethod::_getChar_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_CHAR, false);
  case ciMethod::_getInt_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_INT, false);
  case ciMethod::_getLong_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_LONG, false);
  case ciMethod::_getFloat_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_FLOAT, false);
  case ciMethod::_getDouble_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, !is_store, T_DOUBLE, false);

  case ciMethod::_putObject_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_OBJECT, false);
  case ciMethod::_putBoolean_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_BOOLEAN, false);
  case ciMethod::_putByte_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_BYTE, false);
  case ciMethod::_putShort_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_SHORT, false);
  case ciMethod::_putChar_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_CHAR, false);
  case ciMethod::_putInt_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_INT, false);
  case ciMethod::_putLong_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_LONG, false);
  case ciMethod::_putFloat_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_FLOAT, false);
  case ciMethod::_putDouble_obj32:
    return inline_unsafe_access(!is_native_ptr, !offset_is_long, is_store, T_DOUBLE, false);

  case ciMethod::_getObject_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_OBJECT, false);
  case ciMethod::_getBoolean_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_BOOLEAN, false);
  case ciMethod::_getByte_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_BYTE, false);
  case ciMethod::_getShort_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_SHORT, false);
  case ciMethod::_getChar_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_CHAR, false);
  case ciMethod::_getInt_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_INT, false);
  case ciMethod::_getLong_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_LONG, false);
  case ciMethod::_getFloat_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_FLOAT, false);
  case ciMethod::_getDouble_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_DOUBLE, false);

  case ciMethod::_putObject_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_OBJECT, false);
  case ciMethod::_putBoolean_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_BOOLEAN, false);
  case ciMethod::_putByte_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_BYTE, false);
  case ciMethod::_putShort_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_SHORT, false);
  case ciMethod::_putChar_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_CHAR, false);
  case ciMethod::_putInt_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_INT, false);
  case ciMethod::_putLong_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_LONG, false);
  case ciMethod::_putFloat_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_FLOAT, false);
  case ciMethod::_putDouble_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_DOUBLE, false);

  case ciMethod::_getByte_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_BYTE, false);
  case ciMethod::_getShort_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_SHORT, false);
  case ciMethod::_getChar_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_CHAR, false);
  case ciMethod::_getInt_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_INT, false);
  case ciMethod::_getLong_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_LONG, false);
  case ciMethod::_getFloat_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_FLOAT, false);
  case ciMethod::_getDouble_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_DOUBLE, false);
  case ciMethod::_getAddress_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, !is_store, T_ADDRESS, false);

  case ciMethod::_putByte_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_BYTE, false);
  case ciMethod::_putShort_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_SHORT, false);
  case ciMethod::_putChar_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_CHAR, false);
  case ciMethod::_putInt_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_INT, false);
  case ciMethod::_putLong_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_LONG, false);
  case ciMethod::_putFloat_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_FLOAT, false);
  case ciMethod::_putDouble_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_DOUBLE, false);
  case ciMethod::_putAddress_raw:
    return inline_unsafe_access(is_native_ptr, !offset_is_long, is_store, T_ADDRESS, false);

  case ciMethod::_getObjectVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_OBJECT, true);
  case ciMethod::_getBooleanVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_BOOLEAN, true);
  case ciMethod::_getByteVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_BYTE, true);
  case ciMethod::_getShortVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_SHORT, true);
  case ciMethod::_getCharVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_CHAR, true);
  case ciMethod::_getIntVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_INT, true);
  case ciMethod::_getLongVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_LONG, true);
  case ciMethod::_getFloatVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_FLOAT, true);
  case ciMethod::_getDoubleVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, !is_store, T_DOUBLE, true);

  case ciMethod::_putObjectVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_OBJECT, true);
  case ciMethod::_putBooleanVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_BOOLEAN, true);
  case ciMethod::_putByteVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_BYTE, true);
  case ciMethod::_putShortVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_SHORT, true);
  case ciMethod::_putCharVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_CHAR, true);
  case ciMethod::_putIntVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_INT, true);
  case ciMethod::_putLongVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_LONG, true);
  case ciMethod::_putFloatVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_FLOAT, true);
  case ciMethod::_putDoubleVolatile_obj:
    return inline_unsafe_access(!is_native_ptr, offset_is_long, is_store, T_DOUBLE, true);

  case ciMethod::_compareAndSwapObject_obj:
    return inline_unsafe_CAS(T_OBJECT);
  case ciMethod::_compareAndSwapInt_obj:
    return inline_unsafe_CAS(T_INT);
  case ciMethod::_compareAndSwapLong_obj:
    return inline_unsafe_CAS(T_LONG);

  case ciMethod::_currentThread:
    return inline_native_currentThread();
  case ciMethod::_isInterrupted:
    return inline_native_isInterrupted();

  case ciMethod::_currentTimeMillis:
    return inline_native_time_funcs(false);
  case ciMethod::_nanoTime:
    return inline_native_time_funcs(true);
  case ciMethod::_allocateInstance:
    return inline_unsafe_allocate();

  case ciMethod::_isInstance:
    return inline_native_Class_query(ciMethod::_isInstance);
  case ciMethod::_getModifiers:
    return inline_native_Class_query(ciMethod::_getModifiers);
  case ciMethod::_getClassAccessFlags:
    return inline_native_Class_query(ciMethod::_getClassAccessFlags);

  case LibraryIntrinsic::_Array_newInstance:
    return inline_native_Array_newInstance();

  case LibraryIntrinsic::_getCallerClass:
    return inline_native_Reflection_getCallerClass();

  case LibraryIntrinsic::_AtomicLong_get:
    return inline_native_AtomicLong_get();
  case LibraryIntrinsic::_AtomicLong_attemptUpdate:
    return inline_native_AtomicLong_attemptUpdate();

  case ciMethod::_floatToRawIntBits:
  case ciMethod::_floatToIntBits:
  case ciMethod::_intBitsToFloat:
  case ciMethod::_doubleToRawLongBits:
  case ciMethod::_doubleToLongBits:
  case ciMethod::_longBitsToDouble:
    return inline_fp_conversions((ciMethod::IntrinsicId) intrinsic_id());

  default:
    // If you get here, it may be that someone has added a new intrinsic
    // to the list in vmSymbols.hpp without implementing it here.
#ifndef PRODUCT
    if ((PrintMiscellaneous && (Verbose || WizardMode)) || PrintOpto) {
      tty->print_cr("*** Warning: Unimplemented intrinsic %d", intrinsic_id());
    }
#endif
    return false;
  }
}

//------------------------------generate_guard---------------------------
// Helper function for generating guarded fast-slow graph structures.
void LibraryCallKit::generate_guard(Node* test, RegionNode* region, float true_prob) {
  // Build an if node and its projections.
  // If test is true we take the slow path, which we assume is uncommon.
  // IfNode *iff = create_and_map_if( &_gvn, control(), test );
  // replaced with create_and_map_if()
  IfNode *iff = new (2) IfNode( control(), test, true_prob, COUNT_UNKNOWN );
  _gvn.set_type(iff, iff->Value(&_gvn));

  Node *if_slow = _gvn.transform( new (1) IfTrueNode(iff) );
  if( if_slow == top() ) {
    // The slow branch is never taken.  No need to build this guard.
    return;
  }
  region->add_req(if_slow);
  
  Node *if_fast = _gvn.transform( new (1) IfFalseNode(iff) );
  set_control(if_fast);

  return;
}

inline void LibraryCallKit::generate_slow_guard(Node* test, RegionNode* region) {
  generate_guard(test, region, PROB_UNLIKELY_MAG(3));
}
inline void LibraryCallKit::generate_fast_guard(Node* test, RegionNode* region) {
  generate_guard(test, region, PROB_LIKELY_MAG(3));
}

//--------------------------generate_current_thread--------------------
Node* LibraryCallKit::generate_current_thread(Node* &tls_output) {
  ciKlass*    thread_klass = env()->Thread_klass();
  const Type* thread_type  = TypeOopPtr::make_from_klass(thread_klass)->cast_to_ptr_type(TypePtr::NotNull);
  Node* thread = _gvn.transform(new (1) ThreadLocalNode());
  Node* p = basic_plus_adr(top()/*!oop*/, thread, in_bytes(JavaThread::threadObj_offset()));
  Node* threadObj = make_load(NULL, p, thread_type, T_OBJECT);
  tls_output = thread;
  return threadObj;
}

//------------------------------inline_string_compareTo------------------------
bool LibraryCallKit::inline_string_compareTo() {

  const int value_offset = java_lang_String::value_offset_in_bytes();
  const int count_offset = java_lang_String::count_offset_in_bytes();
  const int offset_offset = java_lang_String::offset_offset_in_bytes();

  _sp += 2;
  Node *argument = pop();  // pop non-receiver first:  it was pushed second
  Node *receiver = pop();
  
  // Null check on self without removing any arguments.  The argument
  // null check technically happens in the wrong place, which can lead to
  // invalid stack traces when string compare is inlined into a method
  // which handles NullPointerExceptions.
  _sp += 2;
  receiver = do_null_check(receiver, T_OBJECT);
  if (stopped()) {  
    _sp -= 2;
    return true;
  }
  argument = do_null_check(argument, T_OBJECT);
  _sp -= 2;
  if (stopped()) {
    return true;
  }
  
  ciInstanceKlass* klass = env()->String_klass();
  const TypeInstPtr* string_type =
    TypeInstPtr::make(TypePtr::BotPTR, klass, false, NULL, 0);
  
  Node* compare =
    _gvn.transform(new (7) StrCompNode(
                        control(), 
                        memory(TypeAryPtr::CHARS),
                        memory(string_type->add_offset(value_offset)),
                        memory(string_type->add_offset(count_offset)),
                        memory(string_type->add_offset(offset_offset)),
                        receiver, 
                        argument));
  push(compare);
  return true;
}

//--------------------------pop_math_arg--------------------------------
// Pop a double argument to a math function from the stack
// rounding it if necessary.
Node * LibraryCallKit::pop_math_arg() {
  Node *arg = pop_pair();
  if( Matcher::strict_fp_requires_explicit_rounding && UseSSE<=1 )
    arg = _gvn.transform( new (2) RoundDoubleNode(0, arg) );
  return arg;
}

//------------------------------inline_sincos----------------------------------
// Inline sin/cos instructions, if possible.  If rounding is required, do
// argument reduction which will turn into a fast/slow diamond.
bool LibraryCallKit::inline_sincos( ciMethod::IntrinsicId id ) {
  _sp += arg_size();            // restore stack pointer
  Node* arg = pop_math_arg();
  Node* trig = _gvn.transform((id == ciMethod::_dsin) ? (Node*)new (2) SinDNode(arg) : (Node*)new (2) CosDNode(arg));

  // Rounding required?  Check for argument reduction!
  if( Matcher::strict_fp_requires_explicit_rounding ) {

    static const double     pi_4 =  0.7853981633974483;
    static const double neg_pi_4 = -0.7853981633974483;
    // pi/2 in 80-bit extended precision
    // static const unsigned char pi_2_bits_x[] = {0x35,0xc2,0x68,0x21,0xa2,0xda,0x0f,0xc9,0xff,0x3f,0x00,0x00,0x00,0x00,0x00,0x00};
    // -pi/2 in 80-bit extended precision
    // static const unsigned char neg_pi_2_bits_x[] = {0x35,0xc2,0x68,0x21,0xa2,0xda,0x0f,0xc9,0xff,0xbf,0x00,0x00,0x00,0x00,0x00,0x00};
    // Cutoff value for using this argument reduction technique
    //static const double    pi_2_minus_epsilon =  1.564660403643354;
    //static const double neg_pi_2_plus_epsilon = -1.564660403643354;  
    
    // Pseudocode for sin:
    // if (x <= Math.PI / 4.0) {
    //   if (x >= -Math.PI / 4.0) return  fsin(x);
    //   if (x >= -Math.PI / 2.0) return -fcos(x + Math.PI / 2.0);
    // } else {
    //   if (x <=  Math.PI / 2.0) return  fcos(x - Math.PI / 2.0);
    // }
    // return StrictMath.sin(x);
    
    // Pseudocode for cos:
    // if (x <= Math.PI / 4.0) {
    //   if (x >= -Math.PI / 4.0) return  fcos(x);
    //   if (x >= -Math.PI / 2.0) return  fsin(x + Math.PI / 2.0);
    // } else {
    //   if (x <=  Math.PI / 2.0) return -fsin(x - Math.PI / 2.0);
    // }
    // return StrictMath.cos(x);
    
    // Actually, sticking in an 80-bit Intel value into C2 will be tough; it
    // requires a special machine instruction to load it.  Instead we'll try
    // the 'easy' case.  If we really need the extra range +/- PI/2 we'll
    // probably do the math inside the SIN encoding.
    
    // Make the merge point
    RegionNode *r = new RegionNode(3);
    Node *phi = new PhiNode(r,Type::DOUBLE);

    // Flatten arg so we need only 1 test
    Node *abs = _gvn.transform(new (2) AbsDNode(arg));
    // Node for PI/4 constant
    Node *pi4 = makecon(TypeD::make(pi_4));
    // Check PI/4 : abs(arg)
    Node *cmp = _gvn.transform(new (3) CmpDNode(pi4,abs));
    // Check: If PI/4 < abs(arg) then go slow
    Node *bol = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt ) );
    // Branch either way
    IfNode *iff = create_and_xform_if(control(),bol, PROB_STATIC_FREQUENT, COUNT_UNKNOWN);
    Node *slow_path = opt_iff(r,iff);
    
    // Set fast path result
    phi->set_req(2,trig);

    // Slow path - non-blocking leaf call
    Node *f_mem = make_slow_call( OptoRuntime::Math_D_D_Type(), 
                                  CAST_FROM_FN_PTR(address, ((id == ciMethod::_dsin) ? SharedRuntime::dsin : SharedRuntime::dcos)),
                                  ((id == ciMethod::_dsin) ? "Sin" : "Cos"), 
                                  slow_path, arg, top() );
    Node *call = control()->in(0); 
    Node *slow_result = _gvn.transform( new (1) ProjNode(call,TypeFunc::Parms));
    r->set_req(1,control());
    make_merge_mem( r, call, f_mem );
    phi->set_req(1,slow_result);

    // Post-merge
    set_control(_gvn.transform(r));
    record_for_igvn(r);
    trig = _gvn.transform(phi);

    C->set_has_split_ifs(true); // Has chance for split-if optimization
  }
  // Push result back on JVM stack
  push_pair(trig);
  return true;
}


//------------------------------inline_pow-------------------------------------
// Inline power instructions, if possible.  
bool LibraryCallKit::inline_pow( ciMethod::IntrinsicId id ) {
  // If this inlining ever returned NaN in the past, we do not intrinsify it
  // every again.  NaN results requires StrictMath.pow handling.
  if (too_many_traps(Deoptimization::Reason_intrinsic))  return false;

  // Do not intrinsify on older platforms which lack cmove.
  if (ConditionalMoveLimit == 0)  return false;
  
  // Pseudocode for pow
  // if (x <= 0.0) {
  //   if ((double)((int)y)==y) { // if y is int
  //     result = ((1&(int)y)==0)?-DPow(abs(x), y):DPow(abs(x), y)
  //   } else {
  //     result = NaN;
  //   }
  // } else {
  //   result = DPow(x,y);
  // }
  // if (result != result)?  {
  //   ucommon_trap();
  // }
  // return result;

  _sp += arg_size();        // restore stack pointer
  Node* y = pop_math_arg();  
  Node* x = pop_math_arg();  

  Node *fast_result = _gvn.transform( new (3) PowDNode(0, x, y) );

  // Short form: if not top-level (i.e., Math.pow but inlining Math.pow 
  // inside of something) then skip the fancy tests and just check for
  // NaN result.
  Node *result = NULL;
  if( jvms()->depth() >= 1 ) {
    result = fast_result;
  } else {

    // Set the merge point for If node with condition of (x <= 0.0)
    // There are four possible paths to region node and phi node
    RegionNode *r = new RegionNode(4);
    Node *phi = new PhiNode(r, Type::DOUBLE);
    
    // Build the first if node: if (x <= 0.0)
    // Node for 0 constant
    Node *zeronode = makecon(TypeD::ZERO);
    // Check x:0
    Node *cmp = _gvn.transform(new (3) CmpDNode(x, zeronode));
    // Check: If (x<=0) then go complex path
    Node *bol1 = _gvn.transform( new (2) BoolNode( cmp, BoolTest::le ) );
    // Branch either way 
    IfNode *if1 = create_and_xform_if(control(),bol1, PROB_STATIC_INFREQUENT, COUNT_UNKNOWN);
    Node *opt_test = _gvn.transform(if1);
    //assert( opt_test->is_If(), "Expect an IfNode");
    IfNode *opt_if1 = (IfNode*)opt_test;
    // Fast path taken; set region slot 3
    Node *fast_taken = _gvn.transform( new (1) IfFalseNode(opt_if1) );
    r->set_req(3,fast_taken); // Capture fast-control 
    
    // Fast path not-taken, i.e. slow path
    Node *complex_path = _gvn.transform( new (1) IfTrueNode(opt_if1) );
    
    // Set fast path result
    Node *fast_result = _gvn.transform( new (3) PowDNode(0, y, x) );
    phi->set_req(3, fast_result);
    
    // Complex path
    // Build the second if node (if y is int)
    // Node for (int)y
    Node *inty = _gvn.transform( new (2) ConvD2INode(y));
    // Node for (double)((int) y)
    Node *doubleinty= _gvn.transform( new (2) ConvI2DNode(inty));
    // Check (double)((int) y) : y
    Node *cmpinty= _gvn.transform(new (3) CmpDNode(doubleinty, y));
    // Check if (y isn't int) then go to slow path
    
    Node *bol2 = _gvn.transform( new (2) BoolNode( cmpinty, BoolTest::ne ) );
    // Branch eith way
    IfNode *if2 = create_and_xform_if(complex_path,bol2, PROB_STATIC_INFREQUENT, COUNT_UNKNOWN);
    Node *slow_path = opt_iff(r,if2); // Set region path 2
    
    // Calculate DPow(abs(x), y)*(1 & (int)y)
    // Node for constant 1
    Node *conone = intcon(1);
    // 1& (int)y
    Node *signnode= _gvn.transform( new (3) AndINode(conone, inty) );
    // zero node
    Node *conzero = intcon(0);
    // Check (1&(int)y)==0?
    Node *cmpeq1 = _gvn.transform(new (3) CmpINode(signnode, conzero));
    // Check if (1&(int)y)!=0?, if so the result is negative
    Node *bol3 = _gvn.transform( new (2) BoolNode( cmpeq1, BoolTest::ne ) );
    // abs(x)
    Node *absx=_gvn.transform( new (2) AbsDNode(x));
    // abs(x)^y
    Node *absxpowy = _gvn.transform( new (3) PowDNode(0, y, absx) );
    // -abs(x)^y
    Node *negabsxpowy = _gvn.transform(new (2) NegDNode (absxpowy));
    // (1&(int)y)==1?-DPow(abs(x), y):DPow(abs(x), y)
    Node *signresult = _gvn.transform( CMoveNode::make(NULL, bol3, absxpowy, negabsxpowy, Type::DOUBLE));
    // Set complex path fast result 
    phi->set_req(2, signresult);
    
    static const jlong nan_bits = CONST64(0x7ff8000000000000);
    Node *slow_result = makecon(TypeD::make(*(double*)&nan_bits)); // return NaN
    r->set_req(1,slow_path);
    phi->set_req(1,slow_result);
    
    // Post merge      
    set_control(_gvn.transform(r));
    record_for_igvn(r);
    result=_gvn.transform(phi);
  }

  //-------------------
  //result=(result.isNaN())? uncommon_trap():result;
  // Check: If isNaN() by checking result!=result? then go to Strict Math
  Node* cmpisnan = _gvn.transform(new (3) CmpDNode(result,result));
  // Build the boolean node
  Node* bolisnum = _gvn.transform( new (2) BoolNode(cmpisnan, BoolTest::eq) );

  { BuildCutout unless(this, bolisnum, PROB_STATIC_FREQUENT);
    // End the current control-flow path
    push_pair(x);
    push_pair(y);
    // Math.pow intrinsic returned a NaN, which requires StrictMath.pow 
    // to handle.  Recompile without intrinsifying Math.pow.
    uncommon_trap(Deoptimization::Reason_intrinsic,
                  Deoptimization::Action_make_not_entrant);
  }

  C->set_has_split_ifs(true); // Has chance for split-if optimization

  push_pair(result);
  
  return true;
}

//------------------------------runtime_trig-----------------------------
bool LibraryCallKit::runtime_trig(address funcAddr, const char* funcName) {
  CallRuntimeNode* trig = new CallLeafNode(OptoRuntime::Math_D_D_Type(),
                                           funcAddr,
                                           funcName);

  // Inputs
  _sp += arg_size();        // restore stack pointer
  Node* a = pop_math_arg();
  trig->set_req(TypeFunc::Parms+0, a);
  trig->set_req(TypeFunc::Parms+1, top());
  set_predefined_input_for_runtime_call(trig);

  Node* trig2 = _gvn.transform(trig);

  // Outputs
  set_predefined_output_for_runtime_call(trig2);
  Node* value = _gvn.transform(new(1) ProjNode(trig2, TypeFunc::Parms+0));
#ifdef ASSERT
  Node* value_top = _gvn.transform(new(1) ProjNode(trig2, TypeFunc::Parms+1));
  assert(value_top == top(), "second value must be top");
#endif

  push_pair(value);
  return true;
}
  
//------------------------------inline_math_native-----------------------------
bool LibraryCallKit::inline_math_native(ciMethod::IntrinsicId id) {
  switch (id) {
  case ciMethod::_dsin:
    if (Matcher::sinDSupported()) {
      return inline_sincos(id);
    } else {
      return runtime_trig(CAST_FROM_FN_PTR(address, SharedRuntime::dsin), "SIN");
    }
  case ciMethod::_dcos:
    if (Matcher::cosDSupported()) {
      return inline_sincos(id);
    } else {
      return runtime_trig(CAST_FROM_FN_PTR(address, SharedRuntime::dcos), "COS");
    }
  case ciMethod::_dtan:
    if (Matcher::tanDSupported()) {
      if (PrintInlining || PrintOptoInlining) {
        tty->print_cr("Cannot inline math native java.lang.Math.tanD()");
      }
      return false;             // not yet correctly implemented
      _sp += arg_size();        // restore stack pointer
      Node* b = pop_math_arg();  
      Node* a = _gvn.transform( new (2) TanDNode(0, b) );
      push_pair(a);
      return true;
    }
    return false;
  case ciMethod::_datan2:
    if (Matcher::atanDSupported()) {
      if (PrintInlining || PrintOptoInlining) {
        tty->print_cr("Cannot inline math native java.lang.Math.atanD()");
      }
      return false;             // not yet correctly implemented
      _sp += arg_size();        // restore stack pointer
      Node* a = pop_math_arg();  
      Node* b = pop_math_arg();  
      a = _gvn.transform( new (3) AtanDNode(0, a, b) );
      push_pair(a);
      return true;
    }
    return false;
  case ciMethod::_dsqrt:
    if (Matcher::sqrtDSupported()) {
      _sp += arg_size();        // restore stack pointer
      Node* b = pop_math_arg();  
      Node* a = _gvn.transform( new (2) SqrtDNode(0, b) );
      push_pair(a);
      return true;
    }
    return false;
  case ciMethod::_dpow: return Matcher::powDSupported() ? inline_pow(id) : false;
  default:
    ShouldNotReachHere();
    return false;
  }
}

//----------------------------inline_unsafe_access----------------------------
enum {
  T_ADDRESS_HOLDER = T_LONG
};

// Interpret Unsafe.fieldOffset cookies correctly:
extern jlong Unsafe_field_offset_to_byte_offset(jlong field_offset);

bool LibraryCallKit::inline_unsafe_access(bool is_native_ptr, bool offset_is_long, bool is_store, BasicType type, bool is_volatile) {
  if (callee()->is_static())  return false;  // caller must have the capability!

#ifndef PRODUCT
  {
    ResourceMark rm;
    // Check the signatures, and/or print a cheerful message.
    ciSignature* sig = signature();
    const char* name = callee()->name()->as_utf8();

    if (PrintInlining || PrintOptoInlining)
      tty->print_cr("Inlining Unsafe.%s%s", name, sig->as_symbol()->as_utf8());

#ifdef ASSERT
    if (!is_store) {
      // Object getObject(Object base, int/long offset), etc.
      BasicType rtype = sig->return_type()->basic_type();
      if (rtype == T_ADDRESS_HOLDER && 0 == strcmp(name, "getAddress"))
          rtype = T_ADDRESS;  // it is really a C void*
      assert(rtype == type, "getter must return the expected value");
      if (!is_native_ptr) {
        assert(sig->count() == 2, "oop getter has 2 arguments");
        assert(sig->type_at(0)->basic_type() == T_OBJECT, "getter base is object");
        assert(sig->type_at(1)->basic_type() == (offset_is_long ? T_LONG : T_INT), "getter offset is correct");
      } else {
        assert(sig->count() == 1, "native getter has 1 argument");
        assert(sig->type_at(0)->basic_type() == T_LONG, "getter base is long");
      }
    } else {
      // void putObject(Object base, int/long offset, Object x), etc.
      assert(sig->return_type()->basic_type() == T_VOID, "putter must not return a value");
      if (!is_native_ptr) {
        assert(sig->count() == 3, "oop putter has 3 arguments");
        assert(sig->type_at(0)->basic_type() == T_OBJECT, "putter base is object");
        assert(sig->type_at(1)->basic_type() == (offset_is_long ? T_LONG : T_INT), "putter offset is correct");
      } else {
        assert(sig->count() == 2, "native putter has 2 arguments");
        assert(sig->type_at(0)->basic_type() == T_LONG, "putter base is long");
      }
      BasicType vtype = sig->type_at(sig->count()-1)->basic_type();
      if (vtype == T_ADDRESS_HOLDER && 0 == strcmp(name, "putAddress"))
        vtype = T_ADDRESS;  // it is really a C void*
      assert(vtype == type, "putter must accept the expected value");
    }
#endif // ASSERT
 }
#endif //PRODUCT

  C->set_has_unsafe_access(true);  // Mark eventual nmethod as "unsafe".

  int type_words = type2size[ (type == T_ADDRESS) ? T_LONG : type ];

  // Argument words:  "this" plus (oop/offset) or (lo/hi) args plus maybe 1 or 2 value words
  int nargs = 1 + (is_native_ptr ? 2 : offset_is_long ? 3 : 2) + (is_store ? type_words : 0);

  debug_only(int saved_sp = _sp);
  _sp += nargs;

  Node* val;
  debug_only(val = (Node*)(uintptr_t)-1);

  if (is_store) {
    // Get the value being stored.  (Pop it first; it was pushed last.) 
    switch (type) {
    case T_DOUBLE:
    case T_LONG:
    case T_ADDRESS:
      val = pop_pair();
      break;
    default:
      val = pop();
    }
  }

  // Build address expression
  Node *adr;
  Node *heap_base_oop = top();
  if (!is_native_ptr) {
    // The offset is a value produced by Unsafe.staticFieldOffset or Unsafe.objectFieldOffset
    Node *offset   = offset_is_long ? pop_pair() : pop();
    // The base is either a Java object or a value produced by Unsafe.staticFieldBase
    Node *base     = pop();
    // We currently rely on the cookies produced by Unsafe.xxxFieldOffset
    // to be plain byte offsets, which are also the same as those accepted
    // by oopDesc::field_base.
    assert(Unsafe_field_offset_to_byte_offset(11) == 11,
           "fieldOffset must be byte-scaled");
    // If this is a NULL+long form, we have to switch to a rawptr.
    if (_gvn.type(base) == TypePtr::NULL_PTR) {
      if (!offset_is_long)
        offset = _gvn.transform(new (2) ConvI2LNode(offset));
      adr = _gvn.transform( new (2) CastL2PNode(offset) );
    } else {
      if (offset_is_long) {
        // 32-bit machines ignore the high half!
        NOT_LP64(  offset = _gvn.transform( new (2) ConvL2INode(offset) ) );
      } else {
        // 64-bit machines require ConXNode to be long.
        LP64_ONLY( offset = _gvn.transform( new (2) ConvI2LNode(offset) ) );
      }
      adr = _gvn.transform( new (4) AddPNode(base,base,offset) );
    }
    heap_base_oop = base;
  } else {
    Node *ptr      = pop_pair();
    adr = _gvn.transform( new (2) CastL2PNode(ptr) );
  }

  // Pop receiver last:  it was pushed first.
  Node *receiver = pop();

  assert(saved_sp == _sp, "must have correct argument count");

  const TypePtr *adr_type = _gvn.type(adr)->isa_ptr();

  // First guess at the value type.
  const Type *value_type = Type::get_const_basic_type(type);

  // Try to categorize the address.  If it comes up as TypeJavPtr::BOTTOM,
  // there was not enough information to nail it down.
  Compile::AliasType* alias_type = C->alias_type(adr_type);
  assert(alias_type->index() != Compile::AliasIdxBot, "no bare pointers here");

  // We will need memory barriers unless we can determine a unique
  // alias category for this reference.  (Note:  If for some reason
  // the barriers get omitted and the unsafe reference begins to "pollute"
  // the alias analysis of the rest of the graph, either Compile::can_alias
  // or Compile::must_alias will throw a diagnostic assert.)
  bool need_mem_bar = (alias_type->adr_type() == TypeOopPtr::BOTTOM);

  if (!is_store && type == T_OBJECT) {
    // Attempt to infer a sharper value type from the offset and base type.
    ciKlass* sharpened_klass = NULL;

    // See if it is an instance field, with an object type.
    if (alias_type->field() != NULL) {
      assert(!is_native_ptr, "native pointer op cannot use a java address");
      if (alias_type->field()->type()->is_klass()) {
        sharpened_klass = alias_type->field()->type()->as_klass();
      }
    }

    // See if it is a narrow oop array.
    if (adr_type->isa_aryptr()) {
      if (adr_type->offset() >= objArrayOopDesc::header_size() * wordSize) {
        const TypeOopPtr *elem_type = adr_type->is_aryptr()->elem()->isa_oopptr();
        if (elem_type != NULL) {
          sharpened_klass = elem_type->klass();
        }
      }
    }

    if (sharpened_klass != NULL) {
      const TypeOopPtr* tjp = TypeOopPtr::make_from_klass(sharpened_klass);

      // Sharpen the value type.
      value_type = tjp;

#ifndef PRODUCT
      if (PrintInlining || PrintOptoInlining) {
        tty->print("  from base type:  ");   adr_type->dump();
        tty->print("  sharpened value: "); value_type->dump();
      }
#endif
    }
  }

  // Null check on self without removing any arguments.  The argument
  // null check technically happens in the wrong place, which can lead to
  // invalid stack traces when the primitive is inlined into a method
  // which handles NullPointerExceptions.
  _sp += nargs;
  do_null_check(receiver, T_OBJECT);
  _sp -= nargs;
  if (stopped()) {
    return true;
  }
  // Heap pointers get a null-check from the interpreter,
  // as a courtesy.  However, this is not guaranteed by Unsafe,
  // and it is not possible to fully distinguish unintended nulls
  // from intended ones in this API.

  if (is_volatile) {
    // We need to emit leading and trailing CPU membars (see below) in
    // addition to memory membars when is_volatile. This is a little
    // too strong, but avoids the need to insert per-alias-type
    // volatile membars (for stores; compare Parse::do_put_xxx), which
    // we cannot do effctively here because we probably only have a
    // rough approximation of type.
    need_mem_bar = true;
    // If we did not already discover that the field is volatile,
    // for the alias type to believe that it is.
    if (!alias_type->is_volatile())
      alias_type->set_volatile(true);
    // For Stores, place a memory ordering barrier now.
    if (is_store)
      insert_mem_bar(new MemBarReleaseNode());
  }

  // Memory barrier to prevent normal and 'unsafe' accesses from
  // bypassing each other.  Happens after null checks, so the
  // exception paths do not take memory state from the memory barrier,
  // so there's no problems making a strong assert about mixing users
  // of safe & unsafe memory.  Otherwise fails in a CTW of rt.jar
  // around 5701, class sun/reflect/UnsafeBooleanFieldAccessorImpl.
  if (need_mem_bar) insert_mem_bar(new MemBarCPUOrderNode());

  if (!is_store) {
    Node* p = make_load(control(), adr, value_type, type, adr_type );
    // load value and push onto stack
    switch (type) {
    case T_BOOLEAN:
    case T_CHAR:
    case T_BYTE:
    case T_SHORT:
    case T_INT:
    case T_FLOAT:
    case T_OBJECT:
      push( p );
      break;
    case T_ADDRESS:
      // Cast to an int type.
      p = _gvn.transform( new (2) CastP2LNode(NULL,p) );
      push_pair(p);
      break;
    case T_DOUBLE:
    case T_LONG:
      push_pair( p );
      break;
    default: ShouldNotReachHere();
    }
  } else {
    // place effect of store into memory
    switch (type) {
    case T_DOUBLE:
      val = dstore_rounding(val);
      break;
    case T_ADDRESS:
      // Repackage the long as a pointer.
      val = _gvn.transform( new (2) CastL2PNode(val) );
      break;
    }
    Node *store = store_to_memory(control(), adr, val, type, adr_type);

    if (type == T_OBJECT
        && !_gvn.type(heap_base_oop)->higher_equal(TypePtr::NULL_PTR)) {
      if (!TypePtr::NULL_PTR->higher_equal(_gvn.type(heap_base_oop))) {
        store_barrier(store, adr, val);
      } else {
        // Base pointer may or may not be null, so put out a conditional
        // store barrier.  (Yech.)
        Node* oldctl = control();
        Node* cmp = _gvn.transform( new (3) CmpPNode(heap_base_oop, null()) );
        Node* bol = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
        IfNode* iff = create_and_map_if(oldctl, bol, PROB_MAX, COUNT_UNKNOWN);
        enum {
          heap_store_path = 1,
          null_base_path,
          num_paths
        };
        RegionNode* rgn = new RegionNode(num_paths);
        // fall-through path (base is null, offset is memory address)
        rgn->set_req(null_base_path, _gvn.transform( new (1) IfFalseNode(iff) ));
        Node* newrawmem = PhiNode::make(rgn, memory(Compile::AliasIdxRaw));
        set_control(_gvn.transform( new (1) IfTrueNode(iff) ));
        store_barrier(store, adr, val);
        if (memory(Compile::AliasIdxRaw) == newrawmem->in(null_base_path)) {
          // The store barrier did nothing, after all.
          set_control(oldctl);
        } else {
          // Finish heap_store_path:
          rgn->set_req(heap_store_path, control());
          set_control(_gvn.transform(rgn));
          newrawmem->set_req(heap_store_path, memory(Compile::AliasIdxRaw));
          set_memory(_gvn.transform(newrawmem), Compile::AliasIdxRaw);
        }
      }
    }
  }

  if (is_volatile) {
    if (!is_store) 
      insert_mem_bar(new MemBarAcquireNode());
    else
      insert_mem_bar(new MemBarVolatileNode());
  }

  if (need_mem_bar) insert_mem_bar(new MemBarCPUOrderNode());

  return true;
}

bool LibraryCallKit::inline_unsafe_CAS(BasicType type) {
  // This basic scheme here is the same as inline_unsafe_access, but
  // differs in enough details that combining them would make the code
  // overly confusing.  (This is a true fact! I originally combined
  // them, but even I was confused by it!) As much code/comments as
  // possible are retained from inline_unsafe_access though to make
  // the correspondances clearer. - dl
  
  if (callee()->is_static())  return false;  // caller must have the capability!

#ifndef PRODUCT
  {
    ResourceMark rm;
    // Check the signatures, and/or print a cheerful message.
    ciSignature* sig = signature();
    const char* name = callee()->name()->as_utf8();

    if (PrintInlining || PrintOptoInlining)
      tty->print_cr("Inlining Unsafe.%s%s", name, sig->as_symbol()->as_utf8());

#ifdef ASSERT
    BasicType rtype = sig->return_type()->basic_type();
    assert(rtype == T_BOOLEAN, "CAS must return boolean");
    assert(sig->count() == 4, "CAS has 4 arguments");
    assert(sig->type_at(0)->basic_type() == T_OBJECT, "CAS base is object");
    assert(sig->type_at(1)->basic_type() == T_LONG, "CAS offset is long");
#endif // ASSERT
  }
#endif //PRODUCT

  // number of stack slots per value argument (1 or 2)
  int type_words = type2size[type];

  // Cannot inline wide CAS on machines that don't support it natively
  if (type2aelembytes[type] > BytesPerInt && !VM_Version::supports_cx8())
    return false;

  C->set_has_unsafe_access(true);  // Mark eventual method as "unsafe".
    
  // Argument words:  "this" plus oop plus offset plus oldvalue plus newvalue;
  int nargs = 1 + 1 + 2  + type_words + type_words;

  // pop arguments: newval, oldval, offset, base, and receiver
  debug_only(int saved_sp = _sp);
  _sp += nargs;
  Node* newval   = (type_words == 1) ? pop() : pop_pair();
  Node* oldval   = (type_words == 1) ? pop() : pop_pair();
  Node *offset   = pop_pair();
  Node *base     = pop();
  Node *receiver = pop(); 
  assert(saved_sp == _sp, "must have correct argument count");

  //  Null check receiver.
  _sp += nargs;
  do_null_check(receiver, T_OBJECT);
  _sp -= nargs;
  if (stopped()) {
    return true;
  }

  // Build field offset expression.
  // We currently rely on the cookies produced by Unsafe.xxxFieldOffset
  // to be plain byte offsets, which are also the same as those accepted
  // by oopDesc::field_base.
  assert(Unsafe_field_offset_to_byte_offset(11) == 11, "fieldOffset must be byte-scaled");
  Node * adr;
  // If this is a NULL+long form, we have to switch to a rawptr.
  if (_gvn.type(base) == TypePtr::NULL_PTR) {
    adr = _gvn.transform( new (2) CastL2PNode(offset) );
  } else {
    // 32-bit machines ignore the high half of long offsets
    NOT_LP64(  offset = _gvn.transform( new (2) ConvL2INode(offset) ) );
    adr = _gvn.transform( new (4) AddPNode(base,base,offset) );
  }

  const TypePtr *adr_type = _gvn.type(adr)->isa_ptr();

  // (Unlike inline_unsafe_access, there seems no point in trying
  // to refine types. Just use the coarse types here.
  const Type *value_type = Type::get_const_basic_type(type);
  Compile::AliasType* alias_type = C->alias_type(adr_type);
  assert(alias_type->index() != Compile::AliasIdxBot, "no bare pointers here");
  int alias_idx = C->get_alias_index(adr_type);

  // Memory-model-wise, a CAS acts like a little synchronized block,
  // so needs barriers on each side.  These don't't translate into
  // actual barriers on most machines, but we still need rest of
  // compiler to respect ordering.

  insert_mem_bar(new MemBarReleaseNode());
  insert_mem_bar(new MemBarCPUOrderNode());

  // 4984716: MemBars must be inserted before this 
  //          memory node in order to avoid a false
  //          dependency which will confuse the scheduler.
  Node *mem = memory(alias_idx);

  // For now, we handle only those cases that actually exist: ints,
  // longs, and Object. Adding others should be straightforward.
  Node* cas;
  switch(type) {
  case T_INT:
    cas = _gvn.transform(new (5) CompareAndSwapINode(control(), mem, adr, newval, oldval));
    break;
  case T_LONG:
    cas = _gvn.transform(new (5) CompareAndSwapLNode(control(), mem, adr, newval, oldval));
    break;
  case T_OBJECT:
    cas = _gvn.transform(new (5) CompareAndSwapPNode(control(), mem, adr, newval, oldval));
    // reference stores need a store barrier.
    // (They don't if CAS fails, but it isn't worth checking.)
    store_barrier(cas, adr, newval);
    break;
  default:
    ShouldNotReachHere();
    break;
  }

  // SCMemProjNodes represent the memory state of CAS. Their main
  // role is to prevent CAS nodes from being optimized away when their
  // results aren't used.
  Node* proj = _gvn.transform( new (1) SCMemProjNode(cas));
  set_memory(proj, alias_idx);

  // Add the trailing membar surrounding the access
  insert_mem_bar(new MemBarCPUOrderNode());
  insert_mem_bar(new MemBarAcquireNode());

  push(cas);
  return true;
}

bool LibraryCallKit::inline_unsafe_allocate() {
  if (callee()->is_static())  return false;  // caller must have the capability!

  // Object allocateInstance(Class cls).
  // Argument words:  "this" plus 1 class argument
  int nargs = 1 + 1;
  assert(signature()->count() == nargs-1, "alloc has 1 argument");

  debug_only(int saved_sp = _sp);
  _sp += nargs;

  Node *cls = pop();

  // Pop receiver last:  it was pushed first.
  Node *receiver = pop();

  assert(saved_sp == _sp, "must have correct argument count");

  const TypeInstPtr *cls_mirror = _gvn.type(cls)->isa_instptr();

  ciType* k = !cls_mirror ? NULL : cls_mirror->mirror_type();
  if( k == NULL || !k->is_instance_klass() )  return false;

  ciInstanceKlass* ik = k->as_instance_klass();
  if( !ik->is_initialized() )  return false;
  if( ik->is_abstract() || ik->is_interface() ||
      ik->name() == ciSymbol::java_lang_Class() )
    return false;

  if (PrintInlining || PrintOptoInlining) {
    tty->print("Inlining Unsafe.allocateInstance on constant operand ");
    ik->print_name();
    tty->cr();
  }

  // Null check on self without removing any arguments.  The argument
  // null check technically happens in the wrong place, which can lead to
  // invalid stack traces when the primitive is inlined into a method
  // which handles NullPointerExceptions.
  _sp += nargs;
  do_null_check(receiver, T_OBJECT);
  _sp -= nargs;
  if (stopped()) {
    return true;
  }

  Node* obj = new_instance(ik);
  push(obj);

  return true;
}

//------------------------inline_native_time_funcs--------------
// inline code for System.currentTimeMillis() and System.nanoTime()
// these have the same type and signature
bool LibraryCallKit::inline_native_time_funcs(bool isNano) {
  address funcAddr = isNano ? CAST_FROM_FN_PTR(address, os::javaTimeNanos) :
                              CAST_FROM_FN_PTR(address, os::javaTimeMillis);
  const char * funcName = isNano ? "nanoTime" : "currentTimeMillis";
  CallRuntimeNode* time = new CallLeafNode(OptoRuntime::current_time_millis_Type(),
                                           funcAddr,
                                           funcName);

  // Inputs
  set_predefined_input_for_runtime_call(time);  
  Node* time2 = _gvn.transform(time);

  // Outputs
  set_predefined_output_for_runtime_call(time2);
  Node* value = _gvn.transform(new(1) ProjNode(time2, TypeFunc::Parms+0));
#ifdef ASSERT
  Node* value_top = _gvn.transform(new(1) ProjNode(time2, TypeFunc::Parms + 1));
  assert(value_top == top(), "second value must be top");
#endif
  push_pair(value);
  return true;
}

//------------------------inline_native_currentThread------------------
bool LibraryCallKit::inline_native_currentThread() {
  Node* junk = NULL;
  push(generate_current_thread(junk));
  return true; 
}

//------------------------inline_native_isInterrupted------------------
bool LibraryCallKit::inline_native_isInterrupted() {
  const int nargs = 1+1;  // receiver + boolean
  assert(nargs == arg_size(), "sanity");
  // Add a fast path to t.isInterrupted(clear_int):
  //   (t == Thread.current() && (!TLS._osthread._interrupted || !clear_int))
  //   ? TLS._osthread._interrupted : /*slow path:*/ t.isInterrupted(clear_int)
  // So, in the common case that the interrupt bit is false,
  // we avoid making a call into the VM.  Even if the interrupt bit
  // is true, if the clear_int argument is false, we avoid the VM call.
  // However, if the receiver is not currentThread, we must call the VM,
  // because there must be some locking done around the operation.

  // We only go to the fast case code if we pass two guards.
  // Paths which do not pass are accumulated in the slow_region.
  RegionNode* slow_region = new RegionNode(1);
  record_for_igvn(slow_region);
  RegionNode* result_rgn = new RegionNode(1+3); // fast1, fast2, slow
  PhiNode*    result_val = new PhiNode(result_rgn, TypeInt::BOOL);
  enum { no_int_result_path   = 1,
         no_clear_result_path = 2,
         slow_result_path     = 3
  };
  record_for_igvn(result_rgn);

  // (a) Receiving thread must be the current thread.
  Node* rec_thr = argument(0);
  Node* tls_ptr = NULL;
  Node* cur_thr = generate_current_thread(tls_ptr);
  Node* cmp_thr = _gvn.transform( new (3) CmpPNode(cur_thr, rec_thr) );
  Node* bol_thr = _gvn.transform( new (2) BoolNode(cmp_thr, BoolTest::ne) );

  bool known_current_thread = (_gvn.type(bol_thr) == TypeInt::ZERO);
  if (!known_current_thread)
    generate_slow_guard(bol_thr, slow_region);

  // (b) Interrupt bit on TLS must be false.
  Node* p = basic_plus_adr(top()/*!oop*/, tls_ptr, in_bytes(JavaThread::osthread_offset()));
  Node* osthread = make_load(NULL, p, TypeRawPtr::NOTNULL, T_ADDRESS);
  p = basic_plus_adr(top()/*!oop*/, osthread, in_bytes(OSThread::interrupted_offset()));
  Node* int_bit = make_load(NULL, p, TypeInt::BOOL, T_INT);
  Node* cmp_bit = _gvn.transform( new (3) CmpINode(int_bit, intcon(0)) );
  Node* bol_bit = _gvn.transform( new (2) BoolNode(cmp_bit, BoolTest::ne) );

  IfNode* iff_bit = create_and_map_if(control(), bol_bit, PROB_UNLIKELY_MAG(3), COUNT_UNKNOWN);

  // First fast path:  if (!TLS._interrupted) return false;
  Node* false_bit = _gvn.transform( new (1) IfFalseNode(iff_bit) );
  result_rgn->set_req(no_int_result_path, false_bit);
  result_val->set_req(no_int_result_path, intcon(0));

  // drop through to next case
  set_control( _gvn.transform(new (1) IfTrueNode(iff_bit)) );

  // (c) Or, if interrupt bit is set and clear_int is false, use 2nd fast path.
  Node* clr_arg = argument(1);
  Node* cmp_arg = _gvn.transform( new (3) CmpINode(clr_arg, intcon(0)) );
  Node* bol_arg = _gvn.transform( new (2) BoolNode(cmp_arg, BoolTest::ne) );
  IfNode* iff_arg = create_and_map_if(control(), bol_arg, PROB_FAIR, COUNT_UNKNOWN);

  // Second fast path:  ... else if (!clear_int) return true;
  Node* false_arg = _gvn.transform( new (1) IfFalseNode(iff_arg) );
  result_rgn->set_req(no_clear_result_path, false_arg);
  result_val->set_req(no_clear_result_path, intcon(1));

  // drop through to next case
  set_control( _gvn.transform(new (1) IfTrueNode(iff_arg)) );

  // (d) Otherwise, go to the slow path.
  slow_region->add_req(control());
  set_control( _gvn.transform(slow_region) );

  if (stopped()) {
    // There is no slow path.
    result_rgn->set_req(slow_result_path, top());
    result_val->set_req(slow_result_path, top());
  } else {

    CallJavaNode* slow_call
      = new CallStaticJavaNode(TypeFunc::make(callee()),
                               OptoRuntime::resolve_opt_virtual_call_Java(),
                               callee(), bci());
    slow_call->set_optimized_virtual(true);  // it's a private non-static

    set_arguments_for_java_call(slow_call);
    set_edges_for_java_call(slow_call);
    Node* slow_val = set_results_for_java_call(slow_call);
    // control() set by set_results_for_java_call

    // If we know that the result of the slow call will be true, tell the optimizer!
    if (known_current_thread)  slow_val = intcon(1);

    Node* fast_io  = slow_call->in(TypeFunc::I_O);
    Node* fast_mem = slow_call->in(TypeFunc::Memory);
    // These two phis are pre-filled with copies of of the fast IO and Memory
    Node* io_phi   = PhiNode::make(result_rgn, fast_io,  Type::ABIO);
    Node* mem_phi  = PhiNode::make(result_rgn, fast_mem, Type::MEMORY, TypePtr::BOTTOM);

    result_rgn->set_req(slow_result_path, control());
    io_phi    ->set_req(slow_result_path, i_o());
    mem_phi   ->set_req(slow_result_path, reset_memory());
    result_val->set_req(slow_result_path, slow_val);

    set_all_memory( _gvn.transform(mem_phi) );
    set_i_o(        _gvn.transform(io_phi) );
  }

  set_control( _gvn.transform(result_rgn) );
  push(        _gvn.transform(result_val) );

  C->set_has_split_ifs(true); // Has chance for split-if optimization

  return true;
}

//-------------------------inline_native_Class_query-------------------
bool LibraryCallKit::inline_native_Class_query(ciMethod::IntrinsicId id) {
  // other candidates for Class query intrinsics: isAssignableFrom, isInterface,
  //     isArray, isPrimitive, getName, getSuperclass, getComponentType
  debug_only(int saved_sp = _sp);
  NOT_PRODUCT(const char* iname = NULL);

  int nargs;
  Node* prim_return_value = top();  // what happens if it's a primitive class?

  switch (id) {
  case ciMethod::_isInstance:
    nargs = 1+1;  // the Class mirror, plus the object getting queried about
    // nothing is an instance of a primitive type
    prim_return_value = intcon(0);
    NOT_PRODUCT(iname = "Class.isInstance");
    break;
  case ciMethod::_getModifiers:
    nargs = 1+0;  // just the Class mirror
    prim_return_value = intcon(JVM_ACC_ABSTRACT | JVM_ACC_FINAL | JVM_ACC_PUBLIC);
    NOT_PRODUCT(iname = "Class.getModifiers");
    break;
  case ciMethod::_getClassAccessFlags:
    nargs = 1+0;  // just the Class mirror
    prim_return_value = intcon(JVM_ACC_ABSTRACT | JVM_ACC_FINAL | JVM_ACC_PUBLIC);
    NOT_PRODUCT(iname = "Reflection.getClassAccessFlags");
    break;
  default:
    ShouldNotReachHere();
  }

  // Restore the stack and pop off the arguments.
  _sp += nargs;

  Node *obj = nargs > 1 ? pop() : top();

  // Pop receiver last:  it was pushed first.
  Node *mirror = pop();

  assert(saved_sp == _sp, "arguments must be popped");

  const TypeInstPtr *mirror_con = _gvn.type(mirror)->isa_instptr();
  if (mirror_con == NULL)  return false;  // cannot happen?

#ifndef PRODUCT
  if (PrintInlining || PrintOptoInlining) {
    ciType* k = mirror_con->mirror_type();
    if (k) {
      tty->print("Inlining %s on constant Class ", iname);
      k->print_name();
      tty->cr();
    } else {
      tty->print_cr("Inlining %s on non-constant Class", iname);
    }
  }
#endif

  // Null-check the mirror, and the mirror's klass ptr (in case it is a primitive).
  RegionNode* region = new RegionNode(4);
  record_for_igvn(region);
  Node* phi = new PhiNode(region, TypeInt::BOOL);

  // The mirror will never be null of Reflection.getClassAccessFlags, however
  // it may be null for Class.isInstance or Class.getModifiers. Throw a NPE
  // if it is. See bug 4774291.

  // For Reflection.getClassAccessFlags(), the null check occurs in
  // the wrong place; see inline_unsafe_access(), above, for a similar
  // situation
  _sp += nargs;  // set original stack for use by uncommon_trap
  Node* cast = do_null_check(mirror, T_OBJECT);
  _sp -= nargs;
  // If cast is dead, only null-path is taken
  if (stopped())  return true;
  mirror = _gvn.transform(cast);

  // Now load the mirror's klass metaobject.  Worst-case type is a little odd:
  // NULL is allowed as a result (usually klass loads never produce a NULL).
  Node* p   = basic_plus_adr(cast, cast, java_lang_Class::klass_offset_in_bytes());
  const TypeKlassPtr *tkl = TypeKlassPtr::OBJECT->meet(TypePtr::NULL_PTR)->is_klassptr();
  Node* kls = _gvn.transform(new (3) LoadKlassNode(0, memory(TypeRawPtr::BOTTOM), p, TypeInstPtr::KLASS,tkl));

  // set region->in(3) in case the mirror is a primitive (e.g, int.class)
  cast = null_check_oop(region, kls); // does region->set_req(3)
  phi->set_req(3, prim_return_value);  // if kls is null, we have a primitive mirror
  set_control(cast->in(0));  // fall through if klass not null (not a primitive)
  kls = _gvn.transform(cast);

  // now that we have the non-null klass, we can perform the real query
  Node* query_value = top();
  switch (id) {
  case ciMethod::_isInstance:
    // nothing is an instance of a primitive type
    query_value = gen_instanceof( obj, kls );
    break;
  case ciMethod::_getModifiers:
    p = basic_plus_adr(kls, kls, Klass::modifier_flags_offset_in_bytes() + sizeof(oopDesc));
    query_value = make_load(NULL, p, TypeInt::INT, T_INT);
    break;
  case ciMethod::_getClassAccessFlags:
    p = basic_plus_adr(kls, kls, Klass::access_flags_offset_in_bytes() + sizeof(oopDesc));
    query_value = make_load(NULL, p, TypeInt::INT, T_INT);
    break;
  default:
    ShouldNotReachHere();
  }
  phi->set_req(1, query_value);
  region->set_req(1, control());

  // pull together the cases:  1: (klass.query()), 2: (null.query()), 3: (prim.query())
  set_control(_gvn.transform(region));
  push(_gvn.transform(phi));

  return true;
}

//------------------inline_native_Array_newInstance--------------------
bool LibraryCallKit::inline_native_Array_newInstance() {

  // Restore the stack and pop off the arguments.
  _sp += 2;

  Node *count_val = pop();

  // Pop component class last:  it was pushed first.
  Node *mirror = pop();

  const TypeInstPtr *mirror_con = _gvn.type(mirror)->isa_instptr();

  ciType* k = !mirror_con ? NULL : mirror_con->mirror_type();
  if( k == NULL )  return false;
  if( !k->is_loaded() )  return false;
  if (k->basic_type() == T_VOID)   return false;  // No such thing as a "void[]".

  if (PrintInlining || PrintOptoInlining) {
    tty->print("Inlining Array.newInstance on constant Class ");
    k->print_name();
    tty->cr();
  }

  BasicType elem_type = k->basic_type();
  if (elem_type == T_ARRAY)  elem_type = T_OBJECT;
  const Type* etype;
  if (elem_type != T_OBJECT) {
    etype = Type::get_const_basic_type(elem_type);
  } else {
    etype = TypeOopPtr::make_from_klass_raw(k->as_klass());
  }
  const TypeKlassPtr* array_klass = TypeKlassPtr::make(ciArrayKlass::make(k));

  Node* obj = new_array(count_val, elem_type, etype, array_klass);

  push(obj);

  return true;
}

//------------------------------inline_native_hashcode--------------------
// Build special case code for calls to hashCode on an object.
bool LibraryCallKit::inline_native_hashcode(bool is_virtual, bool is_static) {
  ciMethod*       method = callee();
  const TypeFunc* tf     = TypeFunc::make(callee());
  _sp++;
  Node *obj = pop();
  assert(is_static == callee()->is_static(), "correct intrinsic selection");
  assert(!(is_virtual && is_static), "either virtual, special, or static");
  RegionNode *zero_result = NULL;

  if (!is_static) {
    _sp++;                        // Restore the stack
    // Null check without removing any arguments.
    obj = do_null_check(obj, T_OBJECT);
    // NOW we can remove the argument
    _sp--;
    // Check for locking null object
    if (stopped()) {  return true;  }
  } else {
    // Do a null check, and return zero if null.
    zero_result = new RegionNode(3 + 1);
    obj = null_check_oop(zero_result, obj);
    Node *not_null_ctrl = obj->in(0);
    if (not_null_ctrl == top()) {
      // System.identityHashCode(null) == 0
      push(_gvn.intcon(0));
      set_control(zero_result->in(3));
      return true;
    }
    set_control(not_null_ctrl);
    obj = _gvn.transform(obj);
  }

  // This call may be virtual (invokevirtual) or bound (invokespecial).
  // For each case we generate slightly different code.

  // We only go to the fast case code if we pass a number of guards.  The
  // paths which do not pass are accumulated in the slow_region.
  RegionNode *slow_region = new RegionNode(1);
  record_for_igvn(slow_region);

  // If this is a virtual call, we generate a funny guard.  We pull out
  // the vtable entry corresponding to hashCode() from the target object.
  // If the target method which we are calling happens to be the native
  // Object hashCode() method, we pass the guard.  We do not need this
  // guard for non-virtual calls -- the caller is known to be the native
  // Object hashCode().
  if (is_virtual) {
    // Get the klass of the argument.
    Node *klass_addr = basic_plus_adr( obj, obj, oopDesc::klass_offset_in_bytes() );
    Node* obj_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(klass_addr), klass_addr, TypeInstPtr::KLASS));

    // Get the methodOop out of the appropriate vtable entry.
    int vtable_index  = method->vtable_index();
    int entry_offset  = (instanceKlass::vtable_start_offset() +
                       vtable_index*vtableEntry::size()) * wordSize +
                       vtableEntry::method_offset_in_bytes();
    Node *entry_addr  = basic_plus_adr( obj_klass, obj_klass, entry_offset );
    Node *target_call = make_load(NULL, entry_addr, TypeInstPtr::NOTNULL, T_OBJECT);

    // Compare the target call with the native hashCode
    // const TypeKlassPtr* native_call_addr = TypeKlassPtr::make(method->klass());
    const TypeInstPtr*   native_call_addr = TypeInstPtr::make(method);

    Node *native_call = makecon(native_call_addr);
    Node *chk_native  = _gvn.transform( new (3) CmpPNode( target_call, native_call));
    Node *test_native = _gvn.transform( new (2) BoolNode( chk_native, BoolTest::ne) );
    
    generate_slow_guard(test_native, slow_region);
  }

  // Get the header out of the object, use LoadMarkNode when available
  Node *header_addr = basic_plus_adr( obj, obj, oopDesc::mark_offset_in_bytes());
  Node *header = make_load(NULL, header_addr, TypeRawPtr::BOTTOM, T_ADDRESS);
  header = _gvn.transform( new (2) CastP2XNode(NULL, header) );

  // Test the header to see if it is unlocked.
  Node *lock_mask      = _gvn.MakeConX(markOopDesc::lock_mask_in_place);
  Node *lmasked_header = _gvn.transform( new (3) AndXNode(header, lock_mask) );
  Node *unlocked_val   = _gvn.MakeConX(markOopDesc::unlocked_value);
  Node *chk_unlocked   = _gvn.transform( new (3) CmpXNode( lmasked_header, unlocked_val));
  Node *test_unlocked  = _gvn.transform( new (2) BoolNode( chk_unlocked, BoolTest::ne) );

  generate_slow_guard(test_unlocked, slow_region);

  // Get the hash value and check to see that it has been properly assigned.
  // We depend on hash_mask being at most 32 bits and avoid the use of
  // hash_mask_in_place because it could be larger than 32 bits in a 64-bit
  // vm: see markOop.hpp.
  Node *hash_mask      = _gvn.intcon(markOopDesc::hash_mask);
  Node *hash_shift     = _gvn.intcon(markOopDesc::hash_shift);
  Node *hshifted_header= _gvn.transform( new (3) URShiftXNode(header, hash_shift) );
#ifdef _LP64
  // This hack lets the hash bits live anywhere in the mark object now, as long
  // as the shift drops the relevent bits into the low 32 bits.  Note that
  // Java spec says that HashCode is an int so there's no point in capturing
  // an 'X'-sized hashcode (32 in 32-bit build or 64 in 64-bit build).
  hshifted_header      = _gvn.transform( new (2) ConvL2INode( hshifted_header ) );
#endif
  Node *hash_val       = _gvn.transform( new (3) AndINode(hshifted_header, hash_mask) );

  Node *no_hash_val    = _gvn.intcon(markOopDesc::no_hash);
  Node *chk_assigned   = _gvn.transform( new (3) CmpINode( hash_val, no_hash_val));
  Node *test_assigned  = _gvn.transform( new (2) BoolNode( chk_assigned, BoolTest::eq) );

  generate_slow_guard(test_assigned, slow_region);

  Node *fast_control = control();

  CallJavaNode *slow_call;
  Node *slow_control;
  Node *slow_result;

  // Generate code for the slow case.  We make a call to hashCode().
  if (is_virtual) {
    slow_call = new CallDynamicJavaNode(tf, OptoRuntime::resolve_virtual_call_Java(),
                                        method, bci());
  } else if (is_static) {
    slow_call = new CallStaticJavaNode(tf, OptoRuntime::resolve_static_call_Java(),
                                       method, bci());
  } else {
    CallStaticJavaNode *call = new CallStaticJavaNode(tf,
                                       OptoRuntime::resolve_opt_virtual_call_Java(),
                                       method, bci());
    call->set_optimized_virtual(true);
    slow_call = call;
  }

  set_control(_gvn.transform(slow_region));
  slow_call->set_req( TypeFunc::Parms+0,   obj);
  set_edges_for_java_call(slow_call);
  slow_result  = set_results_for_java_call(slow_call);
  slow_control = control();  // control set by set_results_for_java_call

  // Make the merge region/phi, which combine the fast and slow
  // controls/results.
  RegionNode *result_region = zero_result ? zero_result : new RegionNode(3);
  record_for_igvn(result_region);
  result_region->set_req(1,slow_control);
  result_region->set_req(2,fast_control);
  _gvn.set_type_bottom(result_region);
  set_control(result_region);

  Node *phi = new PhiNode(result_region,Type::ABIO);
  phi->set_req(1,i_o()                      );
  phi->set_req(2,slow_call->in(TypeFunc::I_O));
  if (zero_result)  phi->set_req(3,phi->in(2));
  _gvn.set_type_bottom(phi);
  set_i_o(_gvn.transform(phi));

  phi = new PhiNode(result_region,Type::MEMORY, TypePtr::BOTTOM);
  phi->set_req(1,reset_memory()                                 );
  phi->set_req(2,_gvn.transform(slow_call->in(TypeFunc::Memory)));
  if (zero_result)  phi->set_req(3,phi->in(2));
  _gvn.set_type_bottom(phi);
  set_all_memory(_gvn.transform(phi));

  Node *phi_res = new PhiNode(result_region,TypeInt::INT);
  phi_res->set_req(1,slow_result);
  phi_res->set_req(2,hash_val);
  if (zero_result)  // System.identityHashCode(null) == 0
    phi_res->set_req(3,_gvn.intcon(0));

  push(_gvn.transform( phi_res ));

  return true;
}

//---------------------------inline_native_getClass----------------------------
// Build special case code for calls to hashCode on an object.
bool LibraryCallKit::inline_native_getClass() {
  Node* obj = null_check_receiver(callee());
  if (stopped())  return true;

  ciKlass*    mirror_klass = env()->Class_klass();
  const Type* mirror_type  = TypeOopPtr::make_from_klass(mirror_klass)->cast_to_ptr_type(TypePtr::NotNull);

  Node* p;

  p = basic_plus_adr(obj, obj, oopDesc::klass_offset_in_bytes());
  Node* obj_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(p), p, TypeInstPtr::KLASS));

  p = basic_plus_adr(obj_klass, obj_klass, Klass::java_mirror_offset_in_bytes() + sizeof(oopDesc));
  Node* mirror = make_load(NULL, p, mirror_type, T_OBJECT);

  push(mirror);
  return true;
}

//-----------------inline_native_Reflection_getCallerClass---------------------
// In the presence of deep enough inlining, getCallerClass() becomes a no-op.
//
// NOTE that this code must perform the same logic as
// vframeStream::security_get_caller_frame in that it must skip
// Method.invoke() and auxiliary frames.




bool LibraryCallKit::inline_native_Reflection_getCallerClass() {
  ciMethod*       method = callee();

  if ((PrintInlining || PrintOptoInlining) && Verbose) {
    tty->print_cr("Attempting to inline sun.reflect.Reflection.getCallerClass");
  }

  debug_only(int saved_sp = _sp);

  // Argument words:  (int depth)
  int nargs = 1;

  _sp += nargs;
  Node* caller_depth_node = pop();

  assert(saved_sp == _sp, "must have correct argument count");
  
  // The depth value must be a constant in order for the runtime call
  // to be eliminated.
  const TypeInt* caller_depth_type = _gvn.type(caller_depth_node)->isa_int();
  if (caller_depth_type == NULL || !caller_depth_type->is_con()) {
    if ((PrintInlining || PrintOptoInlining) && Verbose) {
      tty->print_cr("  Bailing out because caller depth was not a constant");
    }
    return false;
  }
  // Note that the JVM state at this point does not include the
  // getCallerClass() frame which we are trying to inline. The
  // semantics of getCallerClass(), however, are that the "first"
  // frame is the getCallerClass() frame, so we subtract one from the
  // requested depth before continuing. We don't inline requests of
  // getCallerClass(0).
  int caller_depth = caller_depth_type->get_con() - 1;
  if (caller_depth < 0) {
    if ((PrintInlining || PrintOptoInlining) && Verbose) {
      tty->print_cr("  Bailing out because caller depth was %d", caller_depth);
    }
    return false;
  }

  if (!jvms()->has_method()) {
    if ((PrintInlining || PrintOptoInlining) && Verbose) {
      tty->print_cr("  Bailing out because intrinsic was inlined at top level");
    }
    return false;
  }
  int _depth = jvms()->depth();  // cache call chain depth

  // Walk back up the JVM state to find the caller at the required
  // depth. NOTE that this code must perform the same logic as
  // vframeStream::security_get_caller_frame in that it must skip
  // Method.invoke() and auxiliary frames. Note also that depth is
  // 1-based (1 is the bottom of the inlining).
  int inlining_depth = _depth;
  JVMState* caller_jvms = NULL;

  if (inlining_depth > 0) {
    caller_jvms = jvms();
    assert(caller_jvms = jvms()->of_depth(inlining_depth), "inlining_depth == our depth");
    do {
      // The following if-tests should be performed in this order
      if (is_method_invoke_or_aux_frame(caller_jvms)) {
        // Skip a Method.invoke() or auxiliary frame
      } else if (caller_depth > 0) {
        // Skip real frame
        --caller_depth;
      } else {
        // We're done: reached desired caller after skipping.
        break;
      }
      caller_jvms = caller_jvms->caller();
      --inlining_depth;
    } while (inlining_depth > 0);
  }

  if (inlining_depth == 0) {
    if ((PrintInlining || PrintOptoInlining) && Verbose) {
      tty->print_cr("  Bailing out because caller depth (%d) exceeded inlining depth (%d)", caller_depth_type->get_con(), _depth);
      tty->print_cr("  JVM state at this point:");
      for (int i = _depth; i >= 1; i--) {
        tty->print_cr("   %d) %s", i, jvms()->of_depth(i)->method()->name()->as_utf8());
      }
    }
    return false; // Reached end of inlining
  }

  // Acquire method holder as java.lang.Class
  ciInstanceKlass* caller_klass  = caller_jvms->method()->holder();
  ciInstance*      caller_mirror = caller_klass->java_mirror();
  // Push this as a constant
  push(makecon(TypeInstPtr::make(caller_mirror)));
  if ((PrintInlining || PrintOptoInlining) && Verbose) {
    tty->print_cr("  Succeeded: caller = %s.%s, caller depth = %d, depth = %d", caller_klass->name()->as_utf8(), caller_jvms->method()->name()->as_utf8(), caller_depth_type->get_con(), _depth);
    tty->print_cr("  JVM state at this point:");
    for (int i = _depth; i >= 1; i--) {
      tty->print_cr("   %d) %s", i, jvms()->of_depth(i)->method()->name()->as_utf8());
    }
  }
  return true;
}

// Helper routine for above
bool LibraryCallKit::is_method_invoke_or_aux_frame(JVMState* jvms) {
  // %%% These methods should be given permanent IntrinsicIDs.
  return jvms->method()->equals(C->get_Method_invoke()) ||
    jvms->method()->holder()->is_subclass_of(C->get_MethodAccessorImpl());
}

static int value_field_offset = -1;  // offset of the "value" field of AtomicLongCSImpl.  This is needed by
                                     // inline_native_AtomicLong_attemptUpdate() but it has no way of
                                     // computing it since there is no lookup field by name function in the
                                     // CI interface.  This is computed and set by inline_native_AtomicLong_get().
                                     // Using a static variable here is safe even if we have multiple compilation
                                     // threads because the offset is constant.  At worst the same offset will be
                                     // computed and  stored multiple

bool LibraryCallKit::inline_native_AtomicLong_get() {
  // Restore the stack and pop off the argument
  _sp+=1;
  Node *obj = pop();

  // get the offset of the "value" field. Since the CI interfaces
  // does not provide a way to look up a field by name, we scan the bytecodes
  // to get the field index.  We expect the first 2 instructions of the method
  // to be:
  //    0 aload_0
  //    1 getfield "value"
  ciMethod* method = callee();
  if (value_field_offset == -1)
  {
    ciField* value_field;
    ciByteCodeStream iter(method);
    Bytecodes::Code bc = iter.next();

    if ((bc != Bytecodes::_aload_0) &&
              ((bc != Bytecodes::_aload) || (iter.get_index() != 0)))
      return false;
    bc = iter.next();
    if (bc != Bytecodes::_getfield)
      return false;
    bool ignore;
    value_field = iter.get_field(ignore);
    value_field_offset = value_field->offset_in_bytes();
  }

  // Null check without removing any arguments.
  _sp++;
  obj = do_null_check(obj, T_OBJECT);
  _sp--;
  // Check for locking null object
  if (stopped()) return true;

  Node *adr = basic_plus_adr(obj, obj, value_field_offset);
  const TypePtr *adr_type = _gvn.type(adr)->is_ptr();
  int alias_idx = C->get_alias_index(adr_type);

  Node *result = _gvn.transform(new (3) LoadLLockedNode(control(), memory(alias_idx), adr));

  push_pair(result);

  return true;
}

bool LibraryCallKit::inline_native_AtomicLong_attemptUpdate() {
  // Restore the stack and pop off the arguments
  _sp+=5;
  Node *newVal = pop_pair();
  Node *oldVal = pop_pair();
  Node *obj = pop();

  // we need the offset of the "value" field which was computed when
  // inlining the get() method.  Give up if we don't have it.
  if (value_field_offset == -1)
    return false;

  // Null check without removing any arguments.
  _sp+=5;
  obj = do_null_check(obj, T_OBJECT);
  _sp-=5;
  // Check for locking null object
  if (stopped()) return true;

  Node *adr = basic_plus_adr(obj, obj, value_field_offset);
  const TypePtr *adr_type = _gvn.type(adr)->is_ptr();
  int alias_idx = C->get_alias_index(adr_type);

  Node *result = _gvn.transform(new (3) StoreLConditionalNode(control(), memory(alias_idx), adr, newVal, oldVal));
  Node *store_proj = _gvn.transform( new (1) SCMemProjNode(result));
  set_memory(store_proj, alias_idx);

  push(result);
  return true;
}

bool LibraryCallKit::inline_fp_conversions(ciMethod::IntrinsicId id) {
  // restore the arguments
  _sp += arg_size();

  switch (id) {
  case ciMethod::_floatToRawIntBits:
    push(_gvn.transform( new (2) MoveF2INode(pop())));
    break;

  case ciMethod::_intBitsToFloat:
    push(_gvn.transform( new (2) MoveI2FNode(pop())));
    break;

  case ciMethod::_doubleToRawLongBits:
    push_pair(_gvn.transform( new (2) MoveD2LNode(pop_pair())));
    break;

  case ciMethod::_longBitsToDouble:
    push_pair(_gvn.transform( new (2) MoveL2DNode(pop_pair())));
    break;

  case ciMethod::_doubleToLongBits: {
    Node* value = pop_pair();

    // two paths (plus control) merge in a wood
    RegionNode *r = new RegionNode(3);
    Node *phi = new PhiNode(r, TypeLong::LONG);

    Node *cmpisnan = _gvn.transform( new (3) CmpDNode(value, value));
    // Build the boolean node
    Node *bolisnan = _gvn.transform( new (2) BoolNode( cmpisnan, BoolTest::ne ) );
    
    // Branch either way.
    // NaN case is less traveled, which makes all the difference.
    IfNode *ifisnan = create_and_xform_if(control(), bolisnan, PROB_STATIC_FREQUENT, COUNT_UNKNOWN);
    Node *opt_isnan = _gvn.transform(ifisnan);
    assert( opt_isnan->is_If(), "Expect an IfNode");
    IfNode *opt_ifisnan = (IfNode*)opt_isnan;
    Node *iftrue = _gvn.transform( new (1) IfTrueNode(opt_ifisnan) );
    
    set_control(iftrue);

    static const jlong nan_bits = CONST64(0x7ff8000000000000);
    Node *slow_result = makecon(TypeLong::make(nan_bits)); // return NaN
    phi->set_req(1, _gvn.transform( slow_result ));
    r->set_req(1, iftrue);

    // Else fall through
    Node *iffalse = _gvn.transform( new (1) IfFalseNode(opt_ifisnan) );
    set_control(iffalse);
    
    phi->set_req(2, _gvn.transform( new (2) MoveD2LNode(value)));
    r->set_req(2, iffalse);
    
    // Post merge      
    set_control(_gvn.transform(r));
    record_for_igvn(r);

    Node* result = _gvn.transform(phi);
    assert(result->bottom_type()->isa_long(), "must be");
    push_pair(result);

    C->set_has_split_ifs(true); // Has chance for split-if optimization
    
    break;
  }

  case ciMethod::_floatToIntBits: {
    Node* value = pop();

    // two paths (plus control) merge in a wood
    RegionNode *r = new RegionNode(3);
    Node *phi = new PhiNode(r, TypeInt::INT);

    Node *cmpisnan = _gvn.transform( new (3) CmpFNode(value, value));
    // Build the boolean node
    Node *bolisnan = _gvn.transform( new (2) BoolNode( cmpisnan, BoolTest::ne ) );
    
    // Branch either way.
    // NaN case is less traveled, which makes all the difference.
    IfNode *ifisnan = create_and_xform_if(control(), bolisnan, PROB_STATIC_FREQUENT, COUNT_UNKNOWN);
    Node *opt_isnan = _gvn.transform(ifisnan);
    assert( opt_isnan->is_If(), "Expect an IfNode");
    IfNode *opt_ifisnan = (IfNode*)opt_isnan;
    Node *iftrue = _gvn.transform( new (1) IfTrueNode(opt_ifisnan) );
    
    set_control(iftrue);

    static const jint nan_bits = 0x7fc00000;
    Node *slow_result = makecon(TypeInt::make(nan_bits)); // return NaN
    phi->set_req(1, _gvn.transform( slow_result ));
    r->set_req(1, iftrue);

    // Else fall through
    Node *iffalse = _gvn.transform( new (1) IfFalseNode(opt_ifisnan) );
    set_control(iffalse);
    
    phi->set_req(2, _gvn.transform( new (2) MoveF2INode(value)));
    r->set_req(2, iffalse);
    
    // Post merge      
    set_control(_gvn.transform(r));
    record_for_igvn(r);

    Node* result = _gvn.transform(phi);
    assert(result->bottom_type()->isa_int(), "must be");
    push(result);

    C->set_has_split_ifs(true); // Has chance for split-if optimization
    
    break;
  }

  default:
    ShouldNotReachHere();
  }

  return true;
}


//------------------------------type2basic-------------------------------------
BasicType type2basic(const Type *t) {
  if (t == TypeInt::CHAR) {
    return T_CHAR;
  } else if (t == TypeInt::INT) {
    return T_INT;
  } else if (t == TypeInt::BYTE || t == TypeInt::BOOL) {
    return T_BYTE;
  } else if (t == TypeInt::SHORT) {
    return T_SHORT;
  } else if (t == TypeInt::DOUBLE) {
    return T_DOUBLE;
  } else if (t == TypeInt::FLOAT) {
    return T_FLOAT;
  } else if (t == TypeLong::LONG) {
    return T_LONG;
  } else if (t->isa_aryptr()) {
    assert( t->isa_instptr() == NULL, "instance and array oops are not ambiguous")
    return T_ARRAY;
  } else if (t->isa_instptr()) {
    return T_OBJECT;
  } else if (t->isa_klassptr()) {
    ShouldNotReachHere();
    return T_OBJECT;
  }

  return T_VOID;
}

//------------------------------basictype2arraycopy----------------------------
address basictype2arraycopy(BasicType t, bool zero_offsets) {
  switch (t) {
  case T_BYTE:
    return zero_offsets ? OptoRuntime::arrayof_jbyte_arraycopy()
                        : OptoRuntime::jbyte_arraycopy();
  case T_CHAR:
  case T_SHORT:
    return zero_offsets ? OptoRuntime::arrayof_jshort_arraycopy()
                        : OptoRuntime::jshort_arraycopy();
  case T_INT:
  case T_FLOAT:
    return zero_offsets ? OptoRuntime::arrayof_jint_arraycopy()
                        : OptoRuntime::jint_arraycopy();
  case T_DOUBLE:
  case T_LONG:
    return zero_offsets ? OptoRuntime::arrayof_jlong_arraycopy()
                        : OptoRuntime::jlong_arraycopy();
  case T_ARRAY:
  case T_OBJECT:
    // Not configurable due to write barrier
    return zero_offsets ? CAST_FROM_FN_PTR(address, OptoRuntime::arrayof_oop_copy)
                        : CAST_FROM_FN_PTR(address, OptoRuntime::oop_copy);
  default:
    ShouldNotReachHere();
    return NULL;
  }
}

//------------------------------inline_arraycopy-----------------------
bool LibraryCallKit::inline_arraycopy() {
  ciMethod*       method = callee();
  const TypeFunc* tf     = TypeFunc::make(callee());

  // Restore the stack and pop off the arguments.
  _sp += 5;
  Node *length = pop();
  Node *dest_offset = pop();
  Node *dest = pop();
  Node *src_offset = pop();
  Node *src = pop();

  // Compile time checks.  If any of these checks cannot be verified at compile time,
  // we do not make a fast path for this call.  Instead, we let the call remain as it
  // is.  The checks we choose to mandate at compile time are:
  //
  // (1) src is an array.
  // (2) dest is an array.
  // (3) The component types of the src and dest arrays are verifiably compatible.
  //     Specifically:
  //     (a) if both component types are primitive types, then they are the same
  //         primitive type.
  //     (b) if both component types are reference types, generate a runtime check
  //         that the src and dest types are identical.
  //     (c) the component types of src and dest are not mixed primitive/reference.

  // Are both src and dest verifiably arrays?
  const Type *src_type = src->Value(&_gvn);
  const Type *dest_type = dest->Value(&_gvn);
  const TypeAryPtr *top_src = src_type->isa_aryptr();
  const TypeAryPtr *top_dest = dest_type->isa_aryptr();
  if (!top_src || !top_dest) {
    // The top level type of either src or dest is not known to be an
    // array.  Punt.
    return false;
  }

  // Walk the array chain until we get to a component type.

  // !!!!! Currently our comparison of array component types is done based on
  // the compiler's type system rather than the VM's type system.  This could
  // get us in trouble if, for example, the compiler figures out that an array
  // of ints is only assigned values in byte range.  The solution for this is
  // to use the VM's type system for this check.  Unfortunately, the TypeAry
  // objects do not contain pointers to Klasses in the VM type system.  These
  // members should be added and this test should be modified.
  const TypeAryPtr *p_src = top_src;
  const TypeAryPtr *p_dest = top_dest;
  const Type *t_src = NULL;
  const Type *t_dest = NULL;
  bool generate_type_check = false;
  do {
    t_src = p_src->elem();
    t_dest = p_dest->elem();
    p_src = t_src->isa_aryptr();
    p_dest = t_dest->isa_aryptr();
  } while (p_src && p_dest);

  if (!p_src && !p_dest) {
    // Neither t_src nor t_dest is an array type.

    const TypeInstPtr *oop_src = t_src->isa_instptr();
    const TypeInstPtr *oop_dest = t_dest->isa_instptr();
    if (oop_src && oop_dest) {
      generate_type_check = true;
    } else if (oop_src || oop_dest) {
      // One of the arrays has a primitive component type, the other has
      // a reference component type.  Punt.
      return false;
    } else {
      // Both arrays have primitive component type.  Check to see that these
      // types are identical.
      BasicType dest_type = type2basic(t_dest);
      if (type2basic(t_src) != dest_type  ||
          T_VOID == dest_type) {
        // The component types are not the same or are not recognized.  Punt.
        return false;
      }
    }
  } else {
    // One of t_src and t_dest is not an array type.  The only case we
    // allow here is the case where the destination array has a component
    // type of java.lang.Object and the source array has an array component
    // type.
    const TypeInstPtr *oop_dest = t_dest->isa_instptr();

    if (!oop_dest || oop_dest->klass()->is_java_lang_Object()) {
      // The destination array does not have a java.lang.Object component type.
      return false;
    } else {
      generate_type_check = true;
    }
  }

  // Figure out the size and type of the elements we will be copying.
  size_t elem_size;
  const Type *elem_type = top_dest->elem();
  BasicType basic_elem_type = type2basic(elem_type);

  elem_size = type2aelembytes[basic_elem_type];
  if (basic_elem_type == T_VOID) {
    // We do not recognize the top level array component type.
    // This case will probably be caught by the not equal T_VOID test in
    // the preceding loop, but this test is here for robust and obvious
    // correctness.
    return false;
  }

  //---------------------------------------------------------------------------
  // We will make a fast path for this call to arraycopy.

  // We have the following tests left to perform:
  // 
  // (1) src must not be null.
  // (2) dest must not be null.
  // (3) src klassOop and dest klassOop must be identical
  // (4) src_offset must not be negative.
  // (5) dest_offset must not be negative.
  // (6) length must not be negative.
  // (7) src_offset + length must not exceed length of src.
  // (8) dest_offset + length must not exceed length of dest.

  // !!!!! Null Checks (1) and (2)
  // We currently perform our null checks with the do_null_check routine.
  // This means that the null exceptions will be reported in the caller
  // rather than (correctly) reported inside of the native arraycopy call.
  // This should be corrected, given time.  We do our null check with the
  // stack pointer restored.
  _sp += 5;
  src = do_null_check(src, T_ARRAY);
  // Check for locking null object
  if (stopped()) {
    _sp -= 5;
    return true;
  }
  dest = do_null_check(dest, T_ARRAY);
  _sp -= 5;
  // Check for locking null object
  if (stopped()) {  return true;  }

  RegionNode *slow_region = new RegionNode(1);
  record_for_igvn(slow_region);

  // (3) src klassOop and dest klassOop must be identical
  if (generate_type_check) {
    // Get the klassOop for both src and dest
    Node* klass_offset = _gvn.MakeConX(oopDesc::klass_offset_in_bytes());

    Node* p1 = _gvn.transform( new (4) AddPNode(src , src , klass_offset) );
    Node* p2 = _gvn.transform( new (4) AddPNode(dest, dest, klass_offset) );
    Node* src_klass  = _gvn.transform(new (3) LoadKlassNode(0, memory(p1), p1, TypeInstPtr::KLASS));
    Node* dest_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(p2), p2, TypeInstPtr::KLASS));

    // Generate the subtype check
    Node *not_subtype_ctrl = gen_subtype_check( src_klass, dest_klass );
    // Plug failing path into the VM-call path
    slow_region->add_req( not_subtype_ctrl );
  }

  // (4) src_offset must not be negative.
  Node *cmp  = _gvn.transform( new (3) CmpINode( src_offset, _gvn.intcon(0)));
  Node *test = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt) );
  generate_slow_guard(test, slow_region);

  // (5) dest_offset must not be negative.
  cmp  = _gvn.transform( new (3) CmpINode( dest_offset, _gvn.intcon(0)));
  test = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt) );
  generate_slow_guard(test, slow_region);

  // (6) length must not be negative.
  cmp  = _gvn.transform( new (3) CmpINode( length, _gvn.intcon(0)));
  test = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt) );
  generate_slow_guard(test, slow_region);

  // (7) src_offset + length must not exceed length of src.
  Node *last_access  = _gvn.transform( new (3) AddINode(src_offset, length));
  Node *array_length = this->array_length(src);

  cmp  = _gvn.transform( new (3) CmpUNode( array_length, last_access));
  test = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt) );
  generate_slow_guard(test, slow_region);

  // (8) dest_offset + length must not exceed length of dest.
  last_access  = _gvn.transform( new (3) AddINode(dest_offset, length));
  array_length = this->array_length(dest);

  cmp  = _gvn.transform( new (3) CmpUNode( array_length, last_access));
  test = _gvn.transform( new (2) BoolNode( cmp, BoolTest::lt) );
  generate_slow_guard(test, slow_region);

  Node* slow_control;
  Node* slow_i_o;
  Node* slow_memory;
  {
    PreserveJVMState pjvms(this);

    // Generate code for the slow case.  We make a call to the native arraycopy method.

    CallStaticJavaNode* slow_call;
    slow_call = new CallStaticJavaNode(tf, OptoRuntime::resolve_static_call_Java(),
                                       method, bci());

    // Set fixed predefined input arguments
    set_control(_gvn.transform(slow_region));
    slow_call->set_req( TypeFunc::Parms+0,   src);
    slow_call->set_req( TypeFunc::Parms+1,   src_offset);
    slow_call->set_req( TypeFunc::Parms+2,   dest);
    slow_call->set_req( TypeFunc::Parms+3,   dest_offset);
    slow_call->set_req( TypeFunc::Parms+4,   length);
    set_edges_for_java_call(slow_call);
    set_results_for_java_call(slow_call);
    if (stopped()) {
      slow_control = slow_i_o = slow_memory = top();
    } else {
      slow_control = control();
      slow_i_o = i_o();
      slow_memory = reset_memory();
    }
  }

  // Check for nothing to copy.
  RegionNode *fast_region = new RegionNode(1);
  record_for_igvn(fast_region);

  cmp  = _gvn.transform( new (3) CmpINode(length, _gvn.intcon(0)) );
  test = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
  generate_fast_guard(test, fast_region);

  Node* fast_control;
  Node* fast_i_o;
  Node* fast_memory;
  {
    PreserveJVMState pjvms(this);

    set_control(_gvn.transform(fast_region));

    // Compute the starting address in each array by scaling the offset
    // and adding it to the array base.
    Node *src_base  = basic_plus_adr( src, src,
				      arrayOopDesc::base_offset_in_bytes(basic_elem_type) );
    Node *dest_base = basic_plus_adr( dest, dest,
				      arrayOopDesc::base_offset_in_bytes(basic_elem_type) );

    bool zero_offsets = src_offset->Value(&_gvn) == TypeInt::ZERO &&
                        dest_offset->Value(&_gvn) == TypeInt::ZERO;

#ifdef _LP64
    // 64-bit pointer math is done in 64-bits
    src_offset               = _gvn.transform( new (2) ConvI2LNode( src_offset  ) );
    dest_offset              = _gvn.transform( new (2) ConvI2LNode( dest_offset ) );
#endif
    Node *scale              = _gvn.MakeConX(elem_size);
    Node *scaled_src_offset  = _gvn.transform( new (3) MulXNode( src_offset, scale) );
    Node *scaled_dest_offset = _gvn.transform( new (3) MulXNode(dest_offset, scale) );

    Node *src_start  = _gvn.transform( new (4) AddPNode(  src,  src_base,  scaled_src_offset) );
    Node *dest_start = _gvn.transform( new (4) AddPNode( dest, dest_base, scaled_dest_offset) );

#ifdef _LP64
    // We want a size_t length
    length = _gvn.transform( new (2) ConvI2LNode( length ) );
#endif
    // Figure out the arraycopy runtime method to call
    address copyfunc_addr = basictype2arraycopy(basic_elem_type, zero_offsets);
    CallRuntimeNode* fast_call =
      new CallLeafNoFPNode(OptoRuntime::arraycopy_Type(),
                           copyfunc_addr,
                           "arraycopy");

    fast_call->set_req( TypeFunc::Parms+0, src_start);
    fast_call->set_req( TypeFunc::Parms+1, dest_start);
    fast_call->set_req( TypeFunc::Parms+2, length);
#ifdef _LP64
    fast_call->set_req( TypeFunc::Parms+3, top()); 
#endif

    set_predefined_input_for_runtime_call(fast_call);

    Node *fast_call2 = _gvn.transform(fast_call);

    // Set fixed predefined return values
    set_predefined_output_for_runtime_call(fast_call2);

    if (stopped()) {
      fast_control = fast_i_o = fast_memory = top();
    } else {
      fast_control = control();
      fast_i_o = i_o();
      fast_memory = reset_memory();
    }
  }

  // Make the fast merge region
  RegionNode *fast_result_region = new RegionNode(3);
  record_for_igvn(fast_result_region);
  fast_result_region->set_req(1,fast_control);
  fast_result_region->set_req(2,control());
  set_control(_gvn.transform(fast_result_region));

  Node *phi = new PhiNode(fast_result_region, Type::ABIO);
  phi->set_req(1,fast_i_o);
  phi->set_req(2,i_o());
  _gvn.set_type_bottom(phi);
  set_i_o(_gvn.transform(phi));

  phi = new PhiNode(fast_result_region, Type::MEMORY, TypePtr::BOTTOM);
  phi->set_req(1,fast_memory);
  phi->set_req(2,reset_memory());
  _gvn.set_type_bottom(phi);
  set_all_memory(_gvn.transform(phi));

  // Make the fast-slow merge region, which combines the fast and slow controls.
  RegionNode *result_region = new RegionNode(3);
  record_for_igvn(result_region);
  result_region->set_req(1,slow_control);
  result_region->set_req(2,control());
  set_control(_gvn.transform(result_region));

  phi = new PhiNode(result_region,Type::ABIO);
  phi->set_req(1,slow_i_o);
  phi->set_req(2,i_o());
  _gvn.set_type_bottom(phi);
  set_i_o(_gvn.transform(phi));

  phi = new PhiNode(result_region,Type::MEMORY, TypePtr::BOTTOM);
  phi->set_req(1,slow_memory);
  phi->set_req(2,reset_memory());
  _gvn.set_type_bottom(phi);
  set_all_memory(_gvn.transform(phi));

  return true;
}

