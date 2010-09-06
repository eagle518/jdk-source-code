#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)javaClasses.cpp	1.217 04/05/27 11:50:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_javaClasses.cpp.incl"

// Helpful macro for computing field offsets at run time rather than hardcoding them
#define COMPUTE_OFFSET(klass_name_as_C_str, dest_offset, klass_oop, name_symbol, signature_symbol) \
{                                                                                                  \
  fieldDescriptor fd;                                                                              \
  instanceKlass* ik = instanceKlass::cast(klass_oop);                                              \
  if (!ik->find_local_field(name_symbol, signature_symbol, &fd)) {                                 \
    fatal("Invalid layout of " klass_name_as_C_str);                                               \
  }                                                                                                \
  dest_offset = fd.offset();                                                                       \
}

// Same as above but for "optional" offsets that might not be present in certain JDK versions
#define COMPUTE_OPTIONAL_OFFSET(klass_name_as_C_str, dest_offset, klass_oop, name_symbol, signature_symbol) \
{                                                                                                  \
  fieldDescriptor fd;                                                                              \
  instanceKlass* ik = instanceKlass::cast(klass_oop);                                              \
  if (ik->find_local_field(name_symbol, signature_symbol, &fd)) {                                  \
    dest_offset = fd.offset();                                                                     \
  }                                                                                                \
}

oop java_lang_String::basic_create_oop(typeArrayOop buffer, bool tenured, TRAPS) {
  typeArrayHandle h_buffer(THREAD, buffer);
  oop obj;
  if (tenured) {
    obj = instanceKlass::cast(SystemDictionary::string_klass())->allocate_permanent_instance(CHECK_0);
  } else {
    obj = instanceKlass::cast(SystemDictionary::string_klass())->allocate_instance(CHECK_0);
  }
  obj->obj_field_put(value_offset,  h_buffer());
  obj->int_field_put(offset_offset, 0);
  obj->int_field_put(count_offset,  h_buffer()->length());

  return obj;
}

Handle java_lang_String::basic_create(typeArrayOop buffer, bool tenured, TRAPS) {
  return Handle(THREAD, basic_create_oop(buffer, tenured, THREAD));
}

oop java_lang_String::create_oop_from_unicode(jchar* unicode, int length, TRAPS) {
  typeArrayOop buffer = oopFactory::new_charArray(length, CHECK_0);
  for (int index = 0; index < length; index++) {
    buffer->char_at_put(index, unicode[index]);
  }
  return basic_create_oop(buffer, false, CHECK_0);
}

Handle java_lang_String::create_from_unicode(jchar* unicode, int length, TRAPS) {
  return Handle(THREAD, create_oop_from_unicode(unicode, length, THREAD));
}

Handle java_lang_String::create_tenured_from_unicode(jchar* unicode, int length, TRAPS) {
  typeArrayOop buffer = oopFactory::new_permanent_charArray(length, CHECK_NH);
  for (int index = 0; index < length; index++) {
    buffer->char_at_put(index, unicode[index]);
  }
  return basic_create(buffer, true, CHECK_NH);
}

oop java_lang_String::create_oop_from_str(const char* utf8_str, TRAPS) {
  if (utf8_str == NULL) return NULL;
  typeArrayOop buffer = oopFactory::new_charArray(utf8_str, CHECK_0);
  return basic_create_oop(buffer, false, CHECK_0);
}

Handle java_lang_String::create_from_str(const char* utf8_str, TRAPS) {
  return Handle(THREAD, create_oop_from_str(utf8_str, THREAD));
}

// Converts a C string to a Java String based on current encoding
Handle java_lang_String::create_from_platform_depended_str(const char* str, TRAPS) {
  assert(str != NULL, "bad arguments");

  typedef jstring (*to_java_string_fn_t)(JNIEnv*, const char *);
  static to_java_string_fn_t _to_java_string_fn = NULL;

  if (_to_java_string_fn == NULL) {
    void *lib_handle = os::native_java_library();
    _to_java_string_fn = CAST_TO_FN_PTR(to_java_string_fn_t, hpi::dll_lookup(lib_handle, "NewStringPlatform"));
    if (_to_java_string_fn == NULL) fatal("NewStringPlatform missing");
  }

  jstring js = NULL;
  { JavaThread* thread = (JavaThread*)THREAD;
    assert(thread->is_Java_thread(), "must be java thread");
    ThreadToNativeFromVM ttn(thread);
    HandleMark hm(thread);    
    js = (_to_java_string_fn)(thread->jni_environment(), str);
  }
  return Handle(THREAD, JNIHandles::resolve(js));
}


Handle java_lang_String::create_from_symbol(symbolHandle symbol, TRAPS) {
  int length = UTF8::unicode_length((char*)symbol->bytes(), symbol->utf8_length());
  typeArrayOop value = oopFactory::new_charArray(length, CHECK_NH);
  if (length > 0) {
    UTF8::convert_to_unicode((char*)symbol->bytes(), value->char_at_addr(0), length);
  }
  return basic_create(value, false, CHECK_NH);
}


Handle java_lang_String::char_converter(Handle java_string, jchar from_char, jchar to_char, TRAPS) {
  // Typical usage is to convert all '/' to '.' in string.
  typeArrayOop value  = (typeArrayOop)java_string->obj_field(value_offset);
  int          offset = java_string->int_field(offset_offset);
  int          length = java_string->int_field(count_offset);

  // First check if any from_char exist
  bool found = false;
  int index = 0; // Declared outside for portability
  for (index = 0; index < length && !found; index++) {
    if (value->char_at(index + offset) == from_char) found = true;;
  }
  // No from_char, so do not copy.
  if (!found) return java_string;

  // Create new UNICODE buffer
  typeArrayHandle from_buffer(THREAD, value);
  typeArrayOop b_oop  = oopFactory::new_charArray(length, CHECK_NH);
  typeArrayHandle to_buffer  (THREAD, b_oop);

  // Copy contents  
  for (index = 0; index < length; index++) {
    jchar c = from_buffer->char_at(index + offset);
    if (c == from_char) c = to_char;
    to_buffer->char_at_put(index, c);
  }  
        
  return basic_create(to_buffer(), false, CHECK_NH);
}


jchar* java_lang_String::as_unicode_string(oop java_string, int& length) {
  assert(is_instance(java_string), "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  length = java_string->int_field(count_offset);

  jchar* result = NEW_RESOURCE_ARRAY(jchar, length);
  for (int index = 0; index < length; index++) {
    result[index] = typeArrayOop(value)->char_at(index + offset);
  }
  return result;
}


int java_lang_String::length(oop java_string) {
  assert(is_instance(java_string), "must be java_string");
  return java_string->int_field(count_offset);
}


int java_lang_String::offset(oop java_string) {
  assert(is_instance(java_string), "must be java_string");
  return java_string->int_field(offset_offset);
}


typeArrayOop java_lang_String::value(oop java_string) {
  assert(is_instance(java_string), "must be java_string");
  return (typeArrayOop) java_string->obj_field(value_offset);
}


symbolHandle java_lang_String::as_symbol(Handle java_string, TRAPS) {
  assert(is_instance(java_string()), "must be java_string");
  typeArrayOop value = typeArrayOop(java_string->obj_field(value_offset));
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);

  ResourceMark rm(THREAD);
  symbolHandle result;

  if (length > 0) {
    int utf8_length = UNICODE::utf8_length(value->char_at_addr(offset), length);
    char* chars = NEW_RESOURCE_ARRAY(char, utf8_length + 1);
    UNICODE::convert_to_utf8(value->char_at_addr(offset), length, chars);
    // Allocate the symbol
    result = oopFactory::new_symbol_handle(chars, utf8_length, CHECK_(symbolHandle()));  
  } else {
    result = oopFactory::new_symbol_handle("", 0, CHECK_(symbolHandle()));  
  }
  return result;
}

int java_lang_String::utf8_length(oop java_string) {
  assert(is_instance(java_string), "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);
  jchar* position = (length == 0) ? NULL : typeArrayOop(value)->char_at_addr(offset);
  return UNICODE::utf8_length(position, length);
}

char* java_lang_String::as_utf8_string(oop java_string) {
  assert(is_instance(java_string), "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);
  jchar* position = (length == 0) ? NULL : typeArrayOop(value)->char_at_addr(offset);
  return UNICODE::as_utf8(position, length);
}


char* java_lang_String::as_utf8_string(oop java_string, int start, int len) {
  assert(is_instance(java_string), "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);
  assert(start+len <= length, "just checking");
  jchar* position = typeArrayOop(value)->char_at_addr(offset+start);
  return UNICODE::as_utf8(position, len);
}

  
bool java_lang_String::is_instance(oop obj) {
  return obj != NULL && obj->klass() == SystemDictionary::string_klass();
}

bool java_lang_String::equals(oop java_string, jchar* chars, int len) {
  assert(SharedSkipVerify ||
         java_string->klass() == SystemDictionary::string_klass(),
         "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);
  if (length != len) return false;
  for (int i = 0; i < len; i++) {
    if (typeArrayOop(value)->char_at(i + offset) != chars[i]) {
      return false;
    }
  }
  return true;
}

void java_lang_String::print(Handle java_string, outputStream* st) {
  assert(java_string->klass() == SystemDictionary::string_klass(), "must be java_string");
  oop value  = java_string->obj_field(value_offset);
  int offset = java_string->int_field(offset_offset);
  int length = java_string->int_field(count_offset);

  int end = MIN2(length, 100); 
  if (value == NULL) {
    tty->print_cr("NULL"); // Thos can happen if, e.g., printing a String object before its initializer has been called
  } else {
    st->print("\"");
    for (int index = 0; index < length; index++) {
      st->print("%c", typeArrayOop(value)->char_at(index + offset));
    }
    st->print("\"");
  }
}


oop java_lang_Class::create_mirror(KlassHandle k, TRAPS) {
  assert(k->java_mirror() == NULL, "should only assign mirror once");
  // Use this moment of initialization to cache modifier_flags also,
  // to support Class.getModifiers().  Instance classes recalculate
  // the cached flags after the class file is parsed, but before the
  // class is put into the system dictionary.
  int computed_modifiers = k->compute_modifier_flags(CHECK_0);
  k->set_modifier_flags(computed_modifiers);
  if (SystemDictionary::class_klass_loaded()) {
    // Allocate mirror (java.lang.Class instance)
    oop mirror = instanceKlass::cast(SystemDictionary::class_klass())->allocate_permanent_instance(CHECK_0);
    // Setup indirections
    mirror->obj_field_put(klass_offset,  k());
    k->set_java_mirror(mirror);
    return mirror;
  } else {
    return NULL;
  }
}


oop java_lang_Class::create_basic_type_mirror(const char* basic_type_name, TRAPS) {
  // This should be improved by adding a field at the Java level or by
  // introducing a new VM klass (see comment in ClassFileParser)
  return instanceKlass::cast(SystemDictionary::class_klass())->allocate_permanent_instance(CHECK_0);
}


klassOop java_lang_Class::as_klassOop(oop java_class) {
  //%note memory_2
  klassOop k = klassOop(java_class->obj_field(klass_offset));
  assert(k == NULL || k->is_klass(), "type check");
  return k;
}


methodOop java_lang_Class::resolved_constructor(oop java_class) {
  oop constructor = java_class->obj_field(resolved_constructor_offset);
  assert(constructor == NULL || constructor->is_method(), "should be method");
  return methodOop(constructor);
}


void java_lang_Class::set_resolved_constructor(oop java_class, methodOop constructor) {
  assert(constructor->is_method(), "should be method");
  java_class->obj_field_put(resolved_constructor_offset, constructor);
}


bool java_lang_Class::is_primitive(oop java_class) {
  klassOop k = klassOop(java_class->obj_field(klass_offset)); 
  assert((k != NULL) == (SystemDictionary::java_mirror_type(java_class) == T_OBJECT), "Consistency check");
  return k == NULL;
}


BasicType java_lang_Class::primitive_type(oop java_class) {
  assert(java_lang_Class::is_primitive(java_class), "just checking");
  return SystemDictionary::java_mirror_type(java_class);
}


oop java_lang_Class::primitive_mirror(BasicType t) {
  oop mirror = SystemDictionary::java_mirror(t);
  assert(mirror != NULL && mirror->is_a(SystemDictionary::class_klass()), "must be a Class");
  assert(java_lang_Class::is_primitive(mirror), "must be primitive");
  return mirror;
}


// Note: JDK1.1 and before had a privateInfo_offset field which was used for the
//       platform thread structure, and a eetop offset which was used for thread
//       local storage (and unused by the HotSpot VM). In JDK1.2 the two structures 
//       merged, so in the HotSpot VM we just use the eetop field for the thread 
//       instead of the privateInfo_offset.
//
// Note: The stackSize field is only present starting in 1.4.

int java_lang_Thread::_name_offset = 0;
int java_lang_Thread::_group_offset = 0;
int java_lang_Thread::_contextClassLoader_offset = 0;
int java_lang_Thread::_inheritedAccessControlContext_offset = 0;
int java_lang_Thread::_priority_offset = 0;
int java_lang_Thread::_eetop_offset = 0;
int java_lang_Thread::_daemon_offset = 0;
int java_lang_Thread::_stillborn_offset = 0;
int java_lang_Thread::_stackSize_offset = 0;
int java_lang_Thread::_tid_offset = 0;
int java_lang_Thread::_thread_status_offset = 0;


void java_lang_Thread::compute_offsets() {
  assert(_group_offset == 0, "offsets should be initialized only once");

  klassOop k = SystemDictionary::thread_klass();
  COMPUTE_OFFSET("java.lang.Thread", _name_offset,      k, vmSymbols::name_name(),      vmSymbols::char_array_signature());
  COMPUTE_OFFSET("java.lang.Thread", _group_offset,     k, vmSymbols::group_name(),     vmSymbols::threadgroup_signature());
  COMPUTE_OFFSET("java.lang.Thread", _contextClassLoader_offset, k, vmSymbols::contextClassLoader_name(), vmSymbols::classloader_signature());
  COMPUTE_OFFSET("java.lang.Thread", _inheritedAccessControlContext_offset, k, vmSymbols::inheritedAccessControlContext_name(), vmSymbols::accesscontrolcontext_signature());
  COMPUTE_OFFSET("java.lang.Thread", _priority_offset,  k, vmSymbols::priority_name(),  vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.Thread", _daemon_offset,    k, vmSymbols::daemon_name(),    vmSymbols::bool_signature());
  COMPUTE_OFFSET("java.lang.Thread", _eetop_offset,     k, vmSymbols::eetop_name(),     vmSymbols::long_signature());
  COMPUTE_OFFSET("java.lang.Thread", _stillborn_offset, k, vmSymbols::stillborn_name(), vmSymbols::bool_signature());
  // The stackSize field is only present starting in 1.4, so don't go fatal. 
  COMPUTE_OPTIONAL_OFFSET("java.lang.Thread", _stackSize_offset, k, vmSymbols::stackSize_name(), vmSymbols::long_signature());
  // The tid field is only present starting in 1.5, so don't go fatal. 
  COMPUTE_OPTIONAL_OFFSET("java.lang.Thread", _tid_offset, k, vmSymbols::thread_id_name(), vmSymbols::long_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.Thread", _thread_status_offset, k, vmSymbols::thread_status_name(), vmSymbols::int_signature());
}


JavaThread* java_lang_Thread::thread(oop java_thread) {
  return (JavaThread*) java_thread->obj_field(_eetop_offset);
}


void java_lang_Thread::set_thread(oop java_thread, JavaThread* thread) {
  // We are storing a JavaThread* (malloc'ed data) into a long field in the thread 
  // object. The store has to be 64-bit wide so we use a pointer store, but we 
  // cannot call oopDesc::obj_field_put since it includes a write barrier!
  oop* addr = java_thread->obj_field_addr(_eetop_offset);
  *addr = (oop) thread;
}


typeArrayOop java_lang_Thread::name(oop java_thread) {
  oop name = java_thread->obj_field(_name_offset);  
  assert(name == NULL || (name->is_typeArray() && typeArrayKlass::cast(name->klass())->type() == T_CHAR), "just checking");
  return typeArrayOop(name);
}


void java_lang_Thread::set_name(oop java_thread, typeArrayOop name) {
  assert(java_thread->obj_field(_name_offset) == NULL, "name should be NULL");
  java_thread->obj_field_put(_name_offset, name);
}


ThreadPriority java_lang_Thread::priority(oop java_thread) {
  return (ThreadPriority)java_thread->int_field(_priority_offset);
}


void java_lang_Thread::set_priority(oop java_thread, ThreadPriority priority) {
  java_thread->int_field_put(_priority_offset, priority);
}


oop java_lang_Thread::threadGroup(oop java_thread) {
  return java_thread->obj_field(_group_offset);
}


bool java_lang_Thread::is_stillborn(oop java_thread) {
  return java_thread->bool_field(_stillborn_offset);
}


// We never have reason to turn the stillborn bit off
void java_lang_Thread::set_stillborn(oop java_thread) {
  java_thread->bool_field_put(_stillborn_offset, true);
}


bool java_lang_Thread::is_alive(oop java_thread) {
  JavaThread* thr = java_lang_Thread::thread(java_thread);
  return (thr != NULL);
}


bool java_lang_Thread::is_daemon(oop java_thread) {
  return java_thread->bool_field(_daemon_offset);
}


void java_lang_Thread::set_daemon(oop java_thread) {
  java_thread->bool_field_put(_daemon_offset, true);
}

oop java_lang_Thread::context_class_loader(oop java_thread) {
  return java_thread->obj_field(_contextClassLoader_offset);
}

oop java_lang_Thread::inherited_access_control_context(oop java_thread) {
  return java_thread->obj_field(_inheritedAccessControlContext_offset);
}


jlong java_lang_Thread::stackSize(oop java_thread) {
  // The stackSize field is only present starting in 1.4
  if (_stackSize_offset > 0) {
    assert(Universe::is_gte_jdk14x_version(), "sanity check");
    return java_thread->long_field(_stackSize_offset);
  } else {
    return 0;
  }
}

// Write the thread status value to threadStatus field in java.lang.Thread java class.
void java_lang_Thread::set_thread_status(oop java_thread,
                                         java_lang_Thread::ThreadStatus status) {
  assert(JavaThread::current()->thread_state() == _thread_in_vm, "Java Thread is not running in vm");
  // The threadStatus is only present starting in 1.5
  if (_thread_status_offset > 0) {
    java_thread->int_field_put(_thread_status_offset, status);
  }
}

// Read thread status value from threadStatus field in java.lang.Thread java class.
java_lang_Thread::ThreadStatus java_lang_Thread::get_thread_status(oop java_thread) {
  assert(Thread::current()->is_VM_thread() ||
         JavaThread::current()->thread_state() == _thread_in_vm,
         "Java Thread is not running in vm");
  // The threadStatus is only present starting in 1.5
  if (_thread_status_offset > 0) {
    return (java_lang_Thread::ThreadStatus)java_thread->int_field(_thread_status_offset);
  } else {
    // All we can easily figure out is if it is alive, but that is
    // enough info for a valid unknown status.
    // These aren't restricted to valid set ThreadStatus values, so
    // use JVMTI values and cast.
    JavaThread* thr = java_lang_Thread::thread(java_thread);
    if (thr == NULL) {
      // the thread hasn't run yet or is in the process of exiting
      return NEW;
    } 
    return (java_lang_Thread::ThreadStatus)JVMTI_THREAD_STATE_ALIVE;
  }
}


jlong java_lang_Thread::thread_id(oop java_thread) {
  // The thread ID field is only present starting in 1.5
  if (_tid_offset > 0) {
    return java_thread->long_field(_tid_offset);
  } else {
    return 0;
  }
}

int java_lang_ThreadGroup::_parent_offset = 0;
int java_lang_ThreadGroup::_name_offset = 0;
int java_lang_ThreadGroup::_threads_offset = 0;
int java_lang_ThreadGroup::_groups_offset = 0;
int java_lang_ThreadGroup::_maxPriority_offset = 0;
int java_lang_ThreadGroup::_destroyed_offset = 0;
int java_lang_ThreadGroup::_daemon_offset = 0;
int java_lang_ThreadGroup::_vmAllowSuspension_offset = 0;
int java_lang_ThreadGroup::_nthreads_offset = 0;
int java_lang_ThreadGroup::_ngroups_offset = 0;

oop  java_lang_ThreadGroup::parent(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->obj_field(_parent_offset);
}

// ("name as oop" accessor is not necessary)

typeArrayOop java_lang_ThreadGroup::name(oop java_thread_group) {
  oop name = java_thread_group->obj_field(_name_offset);
  // ThreadGroup.name can be null
  return name == NULL ? NULL : java_lang_String::value(name);
}

int java_lang_ThreadGroup::nthreads(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->int_field(_nthreads_offset);
}

objArrayOop java_lang_ThreadGroup::threads(oop java_thread_group) {
  oop threads = java_thread_group->obj_field(_threads_offset);
  assert(threads != NULL, "threadgroups should have threads");
  assert(threads->is_objArray(), "just checking"); // Todo: Add better type checking code
  return objArrayOop(threads);
}

int java_lang_ThreadGroup::ngroups(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->int_field(_ngroups_offset);
}

objArrayOop java_lang_ThreadGroup::groups(oop java_thread_group) {
  oop groups = java_thread_group->obj_field(_groups_offset);
  assert(groups == NULL || groups->is_objArray(), "just checking"); // Todo: Add better type checking code
  return objArrayOop(groups);
}

ThreadPriority java_lang_ThreadGroup::maxPriority(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return (ThreadPriority) java_thread_group->int_field(_maxPriority_offset);
}

bool java_lang_ThreadGroup::is_destroyed(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->bool_field(_destroyed_offset);
}

bool java_lang_ThreadGroup::is_daemon(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->bool_field(_daemon_offset);
}

bool java_lang_ThreadGroup::is_vmAllowSuspension(oop java_thread_group) {
  assert(java_thread_group->is_oop(), "thread group must be oop");
  return java_thread_group->bool_field(_vmAllowSuspension_offset);
}

void java_lang_ThreadGroup::compute_offsets() {
  assert(_parent_offset == 0, "offsets should be initialized only once");

  klassOop k = SystemDictionary::threadGroup_klass();

  COMPUTE_OFFSET("java.lang.ThreadGroup", _parent_offset,      k, vmSymbols::parent_name(),      vmSymbols::threadgroup_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _name_offset,        k, vmSymbols::name_name(),        vmSymbols::string_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _threads_offset,     k, vmSymbols::threads_name(),     vmSymbols::thread_array_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _groups_offset,      k, vmSymbols::groups_name(),      vmSymbols::threadgroup_array_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _maxPriority_offset, k, vmSymbols::maxPriority_name(), vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _destroyed_offset,   k, vmSymbols::destroyed_name(),   vmSymbols::bool_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _daemon_offset,      k, vmSymbols::daemon_name(),      vmSymbols::bool_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _vmAllowSuspension_offset, k, vmSymbols::vmAllowSuspension_name(), vmSymbols::bool_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _nthreads_offset,    k, vmSymbols::nthreads_name(),    vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.ThreadGroup", _ngroups_offset,     k, vmSymbols::ngroups_name(),     vmSymbols::int_signature());
}

oop java_lang_Throwable::backtrace(oop throwable) {
  return throwable->obj_field_acquire(backtrace_offset);
}


void java_lang_Throwable::set_backtrace(oop throwable, oop value) {
  throwable->release_obj_field_put(backtrace_offset, value);
}


oop java_lang_Throwable::message(oop throwable) {
  return throwable->obj_field(detailMessage_offset);
}


oop java_lang_Throwable::message(Handle throwable) {
  return throwable->obj_field(detailMessage_offset);
}


void java_lang_Throwable::set_message(oop throwable, oop value) {
  throwable->obj_field_put(detailMessage_offset, value);
}


void java_lang_Throwable::clear_stacktrace(oop throwable) {
  assert(Universe::is_gte_jdk14x_version(), "should only be called in >= 1.4");
  throwable->obj_field_put(stackTrace_offset, NULL);
}


void java_lang_Throwable::print(oop throwable, outputStream* st) {
  ResourceMark rm;
  klassOop k = throwable->klass();
  assert(k != NULL, "just checking");
  st->print("%s", instanceKlass::cast(k)->external_name());
  oop msg = message(throwable);
  if (msg != NULL) {
    st->print(": %s", java_lang_String::as_utf8_string(msg));
  }
}


void java_lang_Throwable::print(Handle throwable, outputStream* st) {
  ResourceMark rm;
  klassOop k = throwable->klass();
  assert(k != NULL, "just checking");
  st->print("%s", instanceKlass::cast(k)->external_name());
  oop msg = message(throwable);
  if (msg != NULL) {
    st->print(": %s", java_lang_String::as_utf8_string(msg));
  }
}

void java_lang_Throwable::print_stack_element(Handle stream, methodOop method, int bci) {  
  ResourceMark rm;
  // Get strings and string lengths
  instanceKlass* klass = instanceKlass::cast(method->method_holder());
  const char* klass_name  = klass->external_name();
  int buf_len = (int)strlen(klass_name);
  char* source_file_name;
  if (klass->source_file_name() == NULL) {
    source_file_name = NULL;
  } else {
    source_file_name = klass->source_file_name()->as_C_string();
    buf_len += (int)strlen(source_file_name);
  }
  char* method_name = method->name()->as_C_string();
  buf_len += (int)strlen(method_name);

  // Allocate temporary buffer with extra space for formatting and line number
  char* buf = NEW_RESOURCE_ARRAY(char, buf_len + 64);

  // Print stack trace line in buffer
  sprintf(buf, "\tat %s.%s", klass_name, method_name);
  if (method->is_native()) {
    strcat(buf, "(Native Method)");
  } else {    
    int line_number = method->line_number_from_bci(bci);
    if (source_file_name != NULL && (line_number != -1)) {
      // Sourcename and linenumber
      sprintf(buf + (int)strlen(buf), "(%s:%d)", source_file_name, line_number);
    } else if (source_file_name != NULL) {
      // Just sourcename
      sprintf(buf + (int)strlen(buf), "(%s)", source_file_name);      
    } else {
      // Neither soucename and linenumber
      sprintf(buf + (int)strlen(buf), "(Unknown Source)");
    }
    if (WizardMode && method->code() != NULL) {
      sprintf(buf + (int)strlen(buf), "(nmethod %#x)", method->code());
    }
  }

  // Print buffer on stream
  print_to_stream(stream, buf);
}


void java_lang_Throwable::print_stack_usage(Handle stream) {  
  print_to_stream(stream, "\t<<no stack trace available>>");
}


void java_lang_Throwable::print_to_stream(Handle stream, const char* str) {
  if (stream == NULL) {
    tty->print_cr("%s", str);
  } else {
    EXCEPTION_MARK;
    JavaValue result(T_VOID);
    Handle arg (THREAD, oopFactory::new_charArray(str, THREAD));
    if (!HAS_PENDING_EXCEPTION) {
      JavaCalls::call_virtual(&result, 
                              stream, 
                              KlassHandle(THREAD, stream->klass()),
                              vmSymbolHandles::println_name(), 
                              vmSymbolHandles::char_array_void_signature(), 
                              arg, 
                              THREAD);
    }
    // Ignore any exceptions. we are in the middle of exception handling. Same as classic VM.
    if (HAS_PENDING_EXCEPTION) CLEAR_PENDING_EXCEPTION;
  }

}


void java_lang_Throwable::print_stack_trace(oop throwable, oop print_stream) {
  // Note: this is no longer used in Merlin, but we support it for compatibility.
  Thread *thread = Thread::current();
  Handle stream(thread, print_stream);
  objArrayHandle result (thread, objArrayOop(backtrace(throwable)));
  if (result.is_null()) {
    print_stack_usage(stream);
    return;
  }
  
  while (result.not_null()) {
    objArrayHandle methods (thread,
                            objArrayOop(result->obj_at(trace_methods_offset)));
    typeArrayHandle bcis (thread, 
                          typeArrayOop(result->obj_at(trace_bcis_offset)));

    if (methods.is_null() || bcis.is_null()) {
      print_stack_usage(stream);
      return;
    }

    int length = methods()->length();
    for (int index = 0; index < length; index++) {
      methodOop method = methodOop(methods()->obj_at(index));
      if (method == NULL) return;
      int bci = bcis->ushort_at(index);
      print_stack_element(stream, method, bci);
    }
    result = objArrayHandle(thread, objArrayOop(result->obj_at(trace_next_offset)));
  }
}


void java_lang_Throwable::fill_in_stack_trace(Handle throwable, TRAPS) {
  if (!StackTraceInThrowable) return;
  ResourceMark rm(THREAD);
  int total_count = 0;
  int chunk_count = 0;

  // Start out by clearing the backtrace for this object, in case the VM
  // runs out of memory while allocating the stack trace
  set_backtrace(throwable(), NULL);
  if (Universe::is_gte_jdk14x_version()) {
    // New since 1.4, clear lazily constructed Java level stacktrace if
    // refilling occurs
    clear_stacktrace(throwable());
  }

  objArrayOop  h_oop = oopFactory::new_objArray(
                     SystemDictionary::object_klass(), trace_size, CHECK);
  objArrayHandle backtrace  (THREAD, h_oop);
  objArrayHandle head       (THREAD, h_oop);
  objArrayOop m_oop = oopFactory::new_objArray(
                     SystemDictionary::object_klass(), trace_chunk_size, CHECK);
  objArrayHandle methods (THREAD, m_oop);
  typeArrayOop b = oopFactory::new_shortArray(trace_chunk_size, CHECK);
  typeArrayHandle bcis(THREAD, b);
  head->obj_at_put(trace_methods_offset, methods());
  head->obj_at_put(trace_bcis_offset, bcis());

  vframeStream st((JavaThread*) THREAD); 

  // skip fillInStackTrace
  if (!st.at_end() && (st.method()->name() == vmSymbols::fillInStackTrace_name())) {
    st.next();
  }

  // skip <init> methods of the exceptions klass. If there is <init> methods that
  // belongs to a superclass of the exception  we are going to skipping them in the
  // stack trace. This is simlar to classic VM.
  
  while (!st.at_end() && 
          (st.method()->name() == vmSymbols::object_initializer_name() &&  
           throwable->is_a(st.method()->method_holder())
          )
        ) {    
    st.next();
  }

  // collect the rest
  for (;!st.at_end(); st.next()) {    
    methodOop m = st.method(); // The method is not stored GC safe
    int bci     = st.bci();

    if (chunk_count >= trace_chunk_size) {
      // Need to preserve m across GC points
      methodHandle mh(THREAD, m);
      // Create a new chunk
      objArrayOop  nh_oop = oopFactory::new_objArray(SystemDictionary::object_klass(), trace_size, CHECK);
      objArrayHandle new_head (THREAD, nh_oop);
      head->obj_at_put(trace_next_offset, new_head());
      m_oop = oopFactory::new_objArray(SystemDictionary::object_klass(), trace_chunk_size, CHECK);
      methods = objArrayHandle(THREAD, m_oop);
      b    = oopFactory::new_shortArray(trace_chunk_size, CHECK);
      bcis = typeArrayHandle(THREAD, b);
      new_head->obj_at_put(trace_methods_offset, methods());
      new_head->obj_at_put(trace_bcis_offset, bcis());
      chunk_count = 0;
      head = new_head;
      m = mh(); // Update m after GC points
    }

    // add element
    bcis->ushort_at_put(chunk_count, bci);
    methods->obj_at_put(chunk_count, m);

    chunk_count++;
    total_count++;

    // Bail-out case for too deep stacks
    if (MaxJavaStackTraceDepth == total_count) break;
  }

  // Put completed stack trace into throwable object
  set_backtrace(throwable(), backtrace());
}


void java_lang_Throwable::fill_in_stack_trace(Handle throwable) {
  // No-op if stack trace is disabled
  if (!StackTraceInThrowable) {
    return;
  }

  // Disable stack traces for OutOfMemoryErrors
  if (Universe::is_out_of_memory_error(throwable)) {
    return;
  }

  PRESERVE_EXCEPTION_MARK;

  JavaThread* thread = JavaThread::active();
  fill_in_stack_trace(throwable, thread);
  // ignore exceptions thrown during stack trace filling
  CLEAR_PENDING_EXCEPTION;  
}


int java_lang_Throwable::get_stack_trace_depth(oop throwable, TRAPS) {
  if (throwable == NULL) {
    THROW_0(vmSymbols::java_lang_NullPointerException());
  }
  objArrayOop chunk = objArrayOop(backtrace(throwable));
  int depth = 0;
  if (chunk != NULL) {
    // Iterate over chunks and count full ones
    while (true) {
      objArrayOop next = objArrayOop(chunk->obj_at(trace_next_offset));
      if (next == NULL) break;
      depth += trace_chunk_size;
      chunk = next;
    }
    assert(chunk != NULL && chunk->obj_at(trace_next_offset) == NULL, "sanity check");
    // Count element in remaining partial chunk
    objArrayOop methods = objArrayOop(chunk->obj_at(trace_methods_offset));
    typeArrayOop bcis = typeArrayOop(chunk->obj_at(trace_bcis_offset));
    assert(methods != NULL && bcis != NULL, "sanity check");
    for (int i = 0; i < methods->length(); i++) {
      if (methods->obj_at(i) == NULL) break;
      depth++;
    }
  }
  return depth;
}


oop java_lang_Throwable::get_stack_trace_element(oop throwable, int index, TRAPS) {
  if (throwable == NULL) {
    THROW_0(vmSymbols::java_lang_NullPointerException());
  }
  if (index < 0) {
    THROW_(vmSymbols::java_lang_IndexOutOfBoundsException(), NULL);
  }
  // Compute how many chunks to skip and index into actual chunk
  objArrayOop chunk = objArrayOop(backtrace(throwable));
  int skip_chunks = index / trace_chunk_size;
  int chunk_index = index % trace_chunk_size;
  while (chunk != NULL && skip_chunks > 0) {
    chunk = objArrayOop(chunk->obj_at(trace_next_offset));
	skip_chunks--;
  }
  if (chunk == NULL) {
    THROW_(vmSymbols::java_lang_IndexOutOfBoundsException(), NULL);
  }
  // Get method,bci from chunk
  objArrayOop methods = objArrayOop(chunk->obj_at(trace_methods_offset));
  typeArrayOop bcis = typeArrayOop(chunk->obj_at(trace_bcis_offset));
  assert(methods != NULL && bcis != NULL, "sanity check");
  methodHandle method(THREAD, methodOop(methods->obj_at(chunk_index)));
  int bci = bcis->ushort_at(chunk_index);
  // Chunk can be partial full
  if (method.is_null()) {
    THROW_(vmSymbols::java_lang_IndexOutOfBoundsException(), NULL);
  }

  oop element = java_lang_StackTraceElement::create(method, bci, CHECK_0);
  return element;
}

oop java_lang_StackTraceElement::create(methodHandle method, int bci, TRAPS) {
  // Allocate java.lang.StackTraceElement instance
  klassOop k = SystemDictionary::stackTraceElement_klass();
  instanceKlassHandle ik (THREAD, k);
  if (ik->should_be_initialized()) {
    ik->initialize(CHECK_0);
  }

  Handle element = ik->allocate_instance_handle(CHECK_0);
  // Fill in class name
  ResourceMark rm(THREAD);
  const char* str = instanceKlass::cast(method->method_holder())->external_name();
  oop classname = StringTable::intern((char*) str, CHECK_0);
  java_lang_StackTraceElement::set_className(element(), classname);
  // Fill in method name
  oop methodname = StringTable::intern(method->name(), CHECK_0);
  java_lang_StackTraceElement::set_methodName(element(), methodname);
  // Fill in source file name
  symbolOop source = instanceKlass::cast(method->method_holder())->source_file_name();
  oop filename = StringTable::intern(source, CHECK_0);
  java_lang_StackTraceElement::set_fileName(element(), filename);
  // File in source line number
  int line_number;
  if (method->is_native()) {
    // Negative value different from -1 below, enabling Java code in 
    // class java.lang.StackTraceElement to distinguish "native" from
    // "no LineNumberTable".
    line_number = -2;
  } else {
    // Returns -1 if no LineNumberTable, and otherwise actual line number
    line_number = method->line_number_from_bci(bci);
  }
  java_lang_StackTraceElement::set_lineNumber(element(), line_number);

  return element();
}


void java_lang_reflect_AccessibleObject::compute_offsets() {
  klassOop k = SystemDictionary::reflect_accessible_object_klass();
  COMPUTE_OFFSET("java.lang.reflect.AccessibleObject", override_offset, k, vmSymbols::override_name(), vmSymbols::bool_signature());
}

jboolean java_lang_reflect_AccessibleObject::override(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return (jboolean) reflect->bool_field(override_offset);
}

void java_lang_reflect_AccessibleObject::set_override(oop reflect, jboolean value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  reflect->bool_field_put(override_offset, (int) value);
}

void java_lang_reflect_Method::compute_offsets() {
  klassOop k = SystemDictionary::reflect_method_klass();
  COMPUTE_OFFSET("java.lang.reflect.Method", clazz_offset,          k, vmSymbols::clazz_name(),          vmSymbols::class_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", name_offset,           k, vmSymbols::name_name(),           vmSymbols::string_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", returnType_offset,     k, vmSymbols::returnType_name(),     vmSymbols::class_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", parameterTypes_offset, k, vmSymbols::parameterTypes_name(), vmSymbols::class_array_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", exceptionTypes_offset, k, vmSymbols::exceptionTypes_name(), vmSymbols::class_array_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", slot_offset,           k, vmSymbols::slot_name(),           vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.reflect.Method", modifiers_offset,      k, vmSymbols::modifiers_name(),      vmSymbols::int_signature());
  // The generic signature and annotations fields are only present in 1.5
  signature_offset = -1;
  annotations_offset = -1;
  parameter_annotations_offset = -1;
  annotation_default_offset = -1;
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Method", signature_offset,             k, vmSymbols::signature_name(),             vmSymbols::string_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Method", annotations_offset,           k, vmSymbols::annotations_name(),           vmSymbols::byte_array_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Method", parameter_annotations_offset, k, vmSymbols::parameter_annotations_name(), vmSymbols::byte_array_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Method", annotation_default_offset,    k, vmSymbols::annotation_default_name(),    vmSymbols::byte_array_signature());
}

Handle java_lang_reflect_Method::create(TRAPS) {  
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  klassOop klass = SystemDictionary::reflect_method_klass();
  // This class is eagerly initialized during VM initialization, since we keep a refence
  // to one of the methods
  assert(instanceKlass::cast(klass)->is_initialized(), "must be initialized");  
  return instanceKlass::cast(klass)->allocate_instance_handle(CHECK_NH);
}

oop java_lang_reflect_Method::clazz(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->obj_field(clazz_offset);
}

void java_lang_reflect_Method::set_clazz(oop reflect, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
   reflect->obj_field_put(clazz_offset, value);
}

int java_lang_reflect_Method::slot(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->int_field(slot_offset);
}

void java_lang_reflect_Method::set_slot(oop reflect, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  reflect->int_field_put(slot_offset, value);
}

oop java_lang_reflect_Method::name(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return method->obj_field(name_offset);
}

void java_lang_reflect_Method::set_name(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  method->obj_field_put(name_offset, value);
}

oop java_lang_reflect_Method::return_type(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return method->obj_field(returnType_offset);
}

void java_lang_reflect_Method::set_return_type(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  method->obj_field_put(returnType_offset, value);
}

oop java_lang_reflect_Method::parameter_types(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return method->obj_field(parameterTypes_offset);
}

void java_lang_reflect_Method::set_parameter_types(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  method->obj_field_put(parameterTypes_offset, value);
}

oop java_lang_reflect_Method::exception_types(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return method->obj_field(exceptionTypes_offset);
}

void java_lang_reflect_Method::set_exception_types(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  method->obj_field_put(exceptionTypes_offset, value);
}

int java_lang_reflect_Method::modifiers(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return method->int_field(modifiers_offset);
}

void java_lang_reflect_Method::set_modifiers(oop method, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  method->int_field_put(modifiers_offset, value);
}

bool java_lang_reflect_Method::has_signature_field() {
  return (signature_offset >= 0);
}

oop java_lang_reflect_Method::signature(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  return method->obj_field(signature_offset);
}

void java_lang_reflect_Method::set_signature(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  method->obj_field_put(signature_offset, value);
}

bool java_lang_reflect_Method::has_annotations_field() {
  return (annotations_offset >= 0);
}

oop java_lang_reflect_Method::annotations(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  return method->obj_field(annotations_offset);
}

void java_lang_reflect_Method::set_annotations(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  method->obj_field_put(annotations_offset, value);
}

bool java_lang_reflect_Method::has_parameter_annotations_field() {
  return (parameter_annotations_offset >= 0);
}

oop java_lang_reflect_Method::parameter_annotations(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_parameter_annotations_field(), "parameter annotations field must be present");
  return method->obj_field(parameter_annotations_offset);
}

void java_lang_reflect_Method::set_parameter_annotations(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_parameter_annotations_field(), "parameter annotations field must be present");
  method->obj_field_put(parameter_annotations_offset, value);
}

bool java_lang_reflect_Method::has_annotation_default_field() {
  return (annotation_default_offset >= 0);
}

oop java_lang_reflect_Method::annotation_default(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotation_default_field(), "annotation default field must be present");
  return method->obj_field(annotation_default_offset);
}

void java_lang_reflect_Method::set_annotation_default(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotation_default_field(), "annotation default field must be present");
  method->obj_field_put(annotation_default_offset, value);
}

void java_lang_reflect_Constructor::compute_offsets() {
  klassOop k = SystemDictionary::reflect_constructor_klass();
  COMPUTE_OFFSET("java.lang.reflect.Constructor", clazz_offset,          k, vmSymbols::clazz_name(),          vmSymbols::class_signature());
  COMPUTE_OFFSET("java.lang.reflect.Constructor", parameterTypes_offset, k, vmSymbols::parameterTypes_name(), vmSymbols::class_array_signature());
  COMPUTE_OFFSET("java.lang.reflect.Constructor", exceptionTypes_offset, k, vmSymbols::exceptionTypes_name(), vmSymbols::class_array_signature());
  COMPUTE_OFFSET("java.lang.reflect.Constructor", slot_offset,           k, vmSymbols::slot_name(),           vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.reflect.Constructor", modifiers_offset,      k, vmSymbols::modifiers_name(),      vmSymbols::int_signature());
  // The generic signature and annotations fields are only present in 1.5
  signature_offset = -1;
  annotations_offset = -1;
  parameter_annotations_offset = -1;
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Constructor", signature_offset,             k, vmSymbols::signature_name(),             vmSymbols::string_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Constructor", annotations_offset,           k, vmSymbols::annotations_name(),           vmSymbols::byte_array_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Constructor", parameter_annotations_offset, k, vmSymbols::parameter_annotations_name(), vmSymbols::byte_array_signature());
}

Handle java_lang_reflect_Constructor::create(TRAPS) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  symbolHandle name = vmSymbolHandles::java_lang_reflect_Constructor();
  klassOop k = SystemDictionary::resolve_or_fail(name, true, CHECK_NH);
  instanceKlassHandle klass (THREAD, k);
  // Ensure it is initialized
  klass->initialize(CHECK_NH);
  return klass->allocate_instance_handle(CHECK_NH);
}

oop java_lang_reflect_Constructor::clazz(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->obj_field(clazz_offset);
}

void java_lang_reflect_Constructor::set_clazz(oop reflect, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
   reflect->obj_field_put(clazz_offset, value);
}

oop java_lang_reflect_Constructor::parameter_types(oop constructor) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return constructor->obj_field(parameterTypes_offset);
}

void java_lang_reflect_Constructor::set_parameter_types(oop constructor, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  constructor->obj_field_put(parameterTypes_offset, value);
}

oop java_lang_reflect_Constructor::exception_types(oop constructor) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return constructor->obj_field(exceptionTypes_offset);
}

void java_lang_reflect_Constructor::set_exception_types(oop constructor, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  constructor->obj_field_put(exceptionTypes_offset, value);
}

int java_lang_reflect_Constructor::slot(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->int_field(slot_offset);
}

void java_lang_reflect_Constructor::set_slot(oop reflect, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  reflect->int_field_put(slot_offset, value);
}

int java_lang_reflect_Constructor::modifiers(oop constructor) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return constructor->int_field(modifiers_offset);
}

void java_lang_reflect_Constructor::set_modifiers(oop constructor, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  constructor->int_field_put(modifiers_offset, value);
}

bool java_lang_reflect_Constructor::has_signature_field() {
  return (signature_offset >= 0);
}

oop java_lang_reflect_Constructor::signature(oop constructor) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  return constructor->obj_field(signature_offset);
}

void java_lang_reflect_Constructor::set_signature(oop constructor, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  constructor->obj_field_put(signature_offset, value);
}

bool java_lang_reflect_Constructor::has_annotations_field() {
  return (annotations_offset >= 0);
}

oop java_lang_reflect_Constructor::annotations(oop constructor) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  return constructor->obj_field(annotations_offset);
}

void java_lang_reflect_Constructor::set_annotations(oop constructor, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  constructor->obj_field_put(annotations_offset, value);
}

bool java_lang_reflect_Constructor::has_parameter_annotations_field() {
  return (parameter_annotations_offset >= 0);
}

oop java_lang_reflect_Constructor::parameter_annotations(oop method) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_parameter_annotations_field(), "parameter annotations field must be present");
  return method->obj_field(parameter_annotations_offset);
}

void java_lang_reflect_Constructor::set_parameter_annotations(oop method, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_parameter_annotations_field(), "parameter annotations field must be present");
  method->obj_field_put(parameter_annotations_offset, value);
}

void java_lang_reflect_Field::compute_offsets() {
  klassOop k = SystemDictionary::reflect_field_klass();
  COMPUTE_OFFSET("java.lang.reflect.Field", clazz_offset,     k, vmSymbols::clazz_name(),     vmSymbols::class_signature());
  COMPUTE_OFFSET("java.lang.reflect.Field", name_offset,      k, vmSymbols::name_name(),      vmSymbols::string_signature());
  COMPUTE_OFFSET("java.lang.reflect.Field", type_offset,      k, vmSymbols::type_name(),      vmSymbols::class_signature());
  COMPUTE_OFFSET("java.lang.reflect.Field", slot_offset,      k, vmSymbols::slot_name(),      vmSymbols::int_signature());
  COMPUTE_OFFSET("java.lang.reflect.Field", modifiers_offset, k, vmSymbols::modifiers_name(), vmSymbols::int_signature());
  // The generic signature and annotations fields are only present in 1.5
  signature_offset = -1;
  annotations_offset = -1;
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Field", signature_offset, k, vmSymbols::signature_name(), vmSymbols::string_signature());
  COMPUTE_OPTIONAL_OFFSET("java.lang.reflect.Field", annotations_offset,  k, vmSymbols::annotations_name(),  vmSymbols::byte_array_signature());
}

Handle java_lang_reflect_Field::create(TRAPS) {  
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  symbolHandle name = vmSymbolHandles::java_lang_reflect_Field();
  klassOop k = SystemDictionary::resolve_or_fail(name, true, CHECK_NH);
  instanceKlassHandle klass (THREAD, k);
  // Ensure it is initialized
  klass->initialize(CHECK_NH);
  return klass->allocate_instance_handle(CHECK_NH);
}

oop java_lang_reflect_Field::clazz(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->obj_field(clazz_offset);
}

void java_lang_reflect_Field::set_clazz(oop reflect, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
   reflect->obj_field_put(clazz_offset, value);
}

oop java_lang_reflect_Field::name(oop field) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return field->obj_field(name_offset);
}

void java_lang_reflect_Field::set_name(oop field, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  field->obj_field_put(name_offset, value);
}

oop java_lang_reflect_Field::type(oop field) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return field->obj_field(type_offset);
}

void java_lang_reflect_Field::set_type(oop field, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  field->obj_field_put(type_offset, value);
}

int java_lang_reflect_Field::slot(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->int_field(slot_offset);
}

void java_lang_reflect_Field::set_slot(oop reflect, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  reflect->int_field_put(slot_offset, value);
}

int java_lang_reflect_Field::modifiers(oop field) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return field->int_field(modifiers_offset);
}

void java_lang_reflect_Field::set_modifiers(oop field, int value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  field->int_field_put(modifiers_offset, value);
}

bool java_lang_reflect_Field::has_signature_field() {
  return (signature_offset >= 0);
}

oop java_lang_reflect_Field::signature(oop field) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  return field->obj_field(signature_offset);
}

void java_lang_reflect_Field::set_signature(oop field, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_signature_field(), "signature field must be present");
  field->obj_field_put(signature_offset, value);
}

bool java_lang_reflect_Field::has_annotations_field() {
  return (annotations_offset >= 0);
}

oop java_lang_reflect_Field::annotations(oop field) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  return field->obj_field(annotations_offset);
}

void java_lang_reflect_Field::set_annotations(oop field, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  assert(has_annotations_field(), "annotations field must be present");
  field->obj_field_put(annotations_offset, value);
}


void sun_reflect_ConstantPool::compute_offsets() {
  klassOop k = SystemDictionary::reflect_constant_pool_klass();
  // This null test can be removed post beta
  if (k != NULL) {
    COMPUTE_OFFSET("sun.reflect.ConstantPool", cp_oop_offset, k, vmSymbols::constantPoolOop_name(), vmSymbols::object_signature());
  }
}


Handle sun_reflect_ConstantPool::create(TRAPS) {  
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  klassOop k = SystemDictionary::reflect_constant_pool_klass();
  instanceKlassHandle klass (THREAD, k);
  // Ensure it is initialized
  klass->initialize(CHECK_NH);
  return klass->allocate_instance_handle(CHECK_NH);
}


oop sun_reflect_ConstantPool::cp_oop(oop reflect) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  return reflect->obj_field(cp_oop_offset);
}


void sun_reflect_ConstantPool::set_cp_oop(oop reflect, oop value) {
  assert(Universe::is_fully_initialized(), "Need to find another solution to the reflection problem");
  reflect->obj_field_put(cp_oop_offset, value);
}


oop java_lang_boxing_object::initialize_and_allocate(klassOop k, TRAPS) {
 instanceKlassHandle h (THREAD, k);
 if (!h->is_initialized()) h->initialize(CHECK_0);
 return h->allocate_instance(THREAD);
}


oop java_lang_boxing_object::create(BasicType type, jvalue* value, TRAPS) {
  oop box;
  switch (type) {
    case T_BOOLEAN:
      box = initialize_and_allocate(SystemDictionary::boolean_klass(), CHECK_0);
      box->bool_field_put(value_offset, value->z);
      break;
    case T_CHAR:
      box = initialize_and_allocate(SystemDictionary::char_klass(), CHECK_0);
      box->char_field_put(value_offset, value->c);
      break;
    case T_FLOAT:
      box = initialize_and_allocate(SystemDictionary::float_klass(), CHECK_0);
      box->float_field_put(value_offset, value->f);
      break;
    case T_DOUBLE:
      box = initialize_and_allocate(SystemDictionary::double_klass(), CHECK_0);
      box->double_field_put(value_offset, value->d);
      break;
    case T_BYTE:
      box = initialize_and_allocate(SystemDictionary::byte_klass(), CHECK_0);
      box->byte_field_put(value_offset, value->b);
      break;
    case T_SHORT:
      box = initialize_and_allocate(SystemDictionary::short_klass(), CHECK_0);
      box->short_field_put(value_offset, value->s);
      break;
    case T_INT:
      box = initialize_and_allocate(SystemDictionary::int_klass(), CHECK_0);
      box->int_field_put(value_offset, value->i);
      break;
    case T_LONG:
      box = initialize_and_allocate(SystemDictionary::long_klass(), CHECK_0);
      box->long_field_put(value_offset, value->j);
      break;
    default:
      return NULL;
  }
  return box;
}


BasicType java_lang_boxing_object::get_value(oop box, jvalue* value) {
  klassOop k = box->klass();
  if (k == SystemDictionary::boolean_klass()) {
    value->z = box->bool_field(value_offset);
    return T_BOOLEAN;
  }
  if (k == SystemDictionary::char_klass()) {
    value->c = box->char_field(value_offset);
    return T_CHAR;
  }
  if (k == SystemDictionary::float_klass()) {
    value->f = box->float_field(value_offset);
    return T_FLOAT;
  }
  if (k == SystemDictionary::double_klass()) {
    value->d = box->double_field(value_offset);
    return T_DOUBLE;
  }
  if (k == SystemDictionary::byte_klass()) {
    value->b = box->byte_field(value_offset);
    return T_BYTE;
  }
  if (k == SystemDictionary::short_klass()) {
    value->s = box->short_field(value_offset);
    return T_SHORT;
  }
  if (k == SystemDictionary::int_klass()) {
    value->i = box->int_field(value_offset);
    return T_INT;
  }
  if (k == SystemDictionary::long_klass()) {
    value->j = box->long_field(value_offset);
    return T_LONG;
  }
  return T_ILLEGAL;
}


BasicType java_lang_boxing_object::set_value(oop box, jvalue* value) {
  klassOop k = box->klass();
  if (k == SystemDictionary::boolean_klass()) {
    box->bool_field_put(value_offset, value->z);
    return T_BOOLEAN;
  }
  if (k == SystemDictionary::char_klass()) {
    box->char_field_put(value_offset, value->c);
    return T_CHAR;
  }
  if (k == SystemDictionary::float_klass()) {
    box->float_field_put(value_offset, value->f);
    return T_FLOAT;
  }
  if (k == SystemDictionary::double_klass()) {
    box->double_field_put(value_offset, value->d);
    return T_DOUBLE;
  }
  if (k == SystemDictionary::byte_klass()) {
    box->byte_field_put(value_offset, value->b);
    return T_BYTE;
  }
  if (k == SystemDictionary::short_klass()) {
    box->short_field_put(value_offset, value->s);
    return T_SHORT;
  }
  if (k == SystemDictionary::int_klass()) {
    box->int_field_put(value_offset, value->i);
    return T_INT;
  }
  if (k == SystemDictionary::long_klass()) {
    box->long_field_put(value_offset, value->j);
    return T_LONG;
  }
  return T_ILLEGAL;
}


// Support for java_lang_ref_Reference

void java_lang_ref_Reference::set_referent(oop ref, oop value) {
  ref->obj_field_put(referent_offset, value);
}

oop* java_lang_ref_Reference::referent_addr(oop ref) {
  return ref->obj_field_addr(referent_offset);
}

void java_lang_ref_Reference::set_next(oop ref, oop value) {
  ref->obj_field_put(next_offset, value);
}

oop* java_lang_ref_Reference::next_addr(oop ref) {
  return ref->obj_field_addr(next_offset);
}

void java_lang_ref_Reference::set_discovered(oop ref, oop value) {
  ref->obj_field_put(discovered_offset, value);
}

oop* java_lang_ref_Reference::discovered_addr(oop ref) {
  return ref->obj_field_addr(discovered_offset);
}

oop* java_lang_ref_Reference::pending_list_lock_addr() {
  instanceKlass* ik = instanceKlass::cast(SystemDictionary::reference_klass());
  return (oop*)(((char *)ik->start_of_static_fields()) + static_lock_offset);
}

oop* java_lang_ref_Reference::pending_list_addr() {
  instanceKlass* ik = instanceKlass::cast(SystemDictionary::reference_klass());
  return (oop *)(((char *)ik->start_of_static_fields()) + static_pending_offset);
}


// Support for java_lang_ref_SoftReference

jlong java_lang_ref_SoftReference::timestamp(oop ref) {
  return ref->long_field(timestamp_offset);
}

jlong java_lang_ref_SoftReference::clock() {
  instanceKlass* ik = instanceKlass::cast(SystemDictionary::soft_reference_klass());
  int offset = ik->offset_of_static_fields() + static_clock_offset;

  return SystemDictionary::soft_reference_klass()->long_field(offset);
}

void java_lang_ref_SoftReference::set_clock(jlong value) {
  instanceKlass* ik = instanceKlass::cast(SystemDictionary::soft_reference_klass());
  int offset = ik->offset_of_static_fields() + static_clock_offset;

  SystemDictionary::soft_reference_klass()->long_field_put(offset, value);
}


// Support for java_security_AccessControlContext

int java_security_AccessControlContext::_context_offset = 0;
int java_security_AccessControlContext::_privilegedContext_offset = 0;
int java_security_AccessControlContext::_isPrivileged_offset = 0;


void java_security_AccessControlContext::compute_offsets() {
  assert(_isPrivileged_offset == 0, "offsets should be initialized only once");
  fieldDescriptor fd;
  instanceKlass* ik = instanceKlass::cast(SystemDictionary::AccessControlContext_klass());

  if (!ik->find_local_field(vmSymbols::context_name(), vmSymbols::protectiondomain_signature(), &fd)) {
    fatal("Invalid layout of java.security.AccessControlContext");
  }
  _context_offset = fd.offset();

  if (!ik->find_local_field(vmSymbols::privilegedContext_name(), vmSymbols::accesscontrolcontext_signature(), &fd)) {
    fatal("Invalid layout of java.security.AccessControlContext");
  }
  _privilegedContext_offset = fd.offset();

  if (!ik->find_local_field(vmSymbols::isPrivileged_name(), vmSymbols::bool_signature(), &fd)) {
    fatal("Invalid layout of java.security.AccessControlContext");
  }
  _isPrivileged_offset = fd.offset();
}


oop java_security_AccessControlContext::create(objArrayHandle context, bool isPrivileged, Handle privileged_context, TRAPS) {  
  assert(_isPrivileged_offset != 0, "offsets should have been initialized");
  // Ensure klass is initialized
  instanceKlass::cast(SystemDictionary::AccessControlContext_klass())->initialize(CHECK_0);
  // Allocate result
  oop result = instanceKlass::cast(SystemDictionary::AccessControlContext_klass())->allocate_instance(CHECK_0);
  // Fill in values
  result->obj_field_put(_context_offset, context());
  result->obj_field_put(_privilegedContext_offset, privileged_context());
  result->bool_field_put(_isPrivileged_offset, isPrivileged);
  return result;
}


// Support for java_lang_ClassLoader

oop java_lang_ClassLoader::parent(oop loader) {
  assert(loader->is_oop(), "loader must be oop");
  return loader->obj_field(parent_offset);
}


bool java_lang_ClassLoader::is_trusted_loader(oop loader) {
  // Fix for 4474172; see evaluation for more details
  loader = non_reflection_class_loader(loader);

  oop cl = SystemDictionary::java_system_loader();
  while(cl != NULL) {
    if (cl == loader) return true;
    cl = parent(cl);
  }
  return false;
}

oop java_lang_ClassLoader::non_reflection_class_loader(oop loader) {
  if (loader != NULL) {
    // See whether this is one of the class loaders associated with
    // the generated bytecodes for reflection, and if so, "magically"
    // delegate to its parent to prevent class loading from occurring
    // in places where applications using reflection didn't expect it.
    klassOop delegating_cl_class = SystemDictionary::reflect_delegating_classloader_klass();
    // This might be null in non-1.4 JDKs
    if (delegating_cl_class != NULL && loader->is_a(delegating_cl_class)) {
      return parent(loader);
    }
  }
  return loader;
}


// Support for java_lang_System

void java_lang_System::compute_offsets() {
  assert(offset_of_static_fields == 0, "offsets should be initialized only once");

  instanceKlass* ik = instanceKlass::cast(SystemDictionary::system_klass());
  offset_of_static_fields = ik->offset_of_static_fields();
}

int java_lang_System::in_offset_in_bytes() {
  return (offset_of_static_fields + static_in_offset);
}


int java_lang_System::out_offset_in_bytes() {
  return (offset_of_static_fields + static_out_offset);
}


int java_lang_System::err_offset_in_bytes() {
  return (offset_of_static_fields + static_err_offset);
}



int java_lang_String::value_offset;
int java_lang_String::offset_offset;
int java_lang_String::count_offset;
int java_lang_String::hash_offset;
int java_lang_Class::klass_offset;
int java_lang_Class::resolved_constructor_offset;
int java_lang_Class::number_of_fake_oop_fields;
int java_lang_Throwable::backtrace_offset;
int java_lang_Throwable::detailMessage_offset;
int java_lang_Throwable::cause_offset;
int java_lang_Throwable::stackTrace_offset;
int java_lang_reflect_AccessibleObject::override_offset;
int java_lang_reflect_Method::clazz_offset;
int java_lang_reflect_Method::name_offset;
int java_lang_reflect_Method::returnType_offset;
int java_lang_reflect_Method::parameterTypes_offset;
int java_lang_reflect_Method::exceptionTypes_offset;
int java_lang_reflect_Method::slot_offset;
int java_lang_reflect_Method::modifiers_offset;
int java_lang_reflect_Method::signature_offset;
int java_lang_reflect_Method::annotations_offset;
int java_lang_reflect_Method::parameter_annotations_offset;
int java_lang_reflect_Method::annotation_default_offset;
int java_lang_reflect_Constructor::clazz_offset;
int java_lang_reflect_Constructor::parameterTypes_offset;
int java_lang_reflect_Constructor::exceptionTypes_offset;
int java_lang_reflect_Constructor::slot_offset;
int java_lang_reflect_Constructor::modifiers_offset;
int java_lang_reflect_Constructor::signature_offset;
int java_lang_reflect_Constructor::annotations_offset;
int java_lang_reflect_Constructor::parameter_annotations_offset;
int java_lang_reflect_Field::clazz_offset;
int java_lang_reflect_Field::name_offset;
int java_lang_reflect_Field::type_offset;
int java_lang_reflect_Field::slot_offset;
int java_lang_reflect_Field::modifiers_offset;
int java_lang_reflect_Field::signature_offset;
int java_lang_reflect_Field::annotations_offset;
int sun_reflect_ConstantPool::cp_oop_offset;
int java_lang_boxing_object::value_offset;
int java_lang_ref_Reference::referent_offset;
int java_lang_ref_Reference::queue_offset;
int java_lang_ref_Reference::next_offset;
int java_lang_ref_Reference::discovered_offset;
int java_lang_ref_Reference::static_lock_offset;
int java_lang_ref_Reference::static_pending_offset;
int java_lang_ref_Reference::number_of_fake_oop_fields;
int java_lang_ref_SoftReference::timestamp_offset;
int java_lang_ref_SoftReference::static_clock_offset;
int java_lang_ClassLoader::parent_offset;
int java_lang_System::offset_of_static_fields;
int java_lang_System::static_in_offset;
int java_lang_System::static_out_offset;
int java_lang_System::static_err_offset;
int java_lang_StackTraceElement::className_offset;
int java_lang_StackTraceElement::methodName_offset;
int java_lang_StackTraceElement::fileName_offset;
int java_lang_StackTraceElement::lineNumber_offset;
int java_lang_AssertionStatusDirectives::classes_offset;
int java_lang_AssertionStatusDirectives::classEnabled_offset;
int java_lang_AssertionStatusDirectives::packages_offset;
int java_lang_AssertionStatusDirectives::packageEnabled_offset;
int java_lang_AssertionStatusDirectives::deflt_offset;
int java_nio_Buffer::_limit_offset;
int sun_misc_AtomicLongCSImpl::_value_offset;


// Support for java_lang_StackTraceElement

void java_lang_StackTraceElement::set_fileName(oop element, oop value) {
  element->obj_field_put(fileName_offset, value);
}

void java_lang_StackTraceElement::set_className(oop element, oop value) {
  element->obj_field_put(className_offset, value);
}

void java_lang_StackTraceElement::set_methodName(oop element, oop value) {
  element->obj_field_put(methodName_offset, value);
}

void java_lang_StackTraceElement::set_lineNumber(oop element, int value) {
  element->int_field_put(lineNumber_offset, value);
}
  
  
// Support for java Assertions - java_lang_AssertionStatusDirectives.

void java_lang_AssertionStatusDirectives::set_classes(oop o, oop val) {
  o->obj_field_put(classes_offset, val);
}

void java_lang_AssertionStatusDirectives::set_classEnabled(oop o, oop val) {
  o->obj_field_put(classEnabled_offset, val);
}

void java_lang_AssertionStatusDirectives::set_packages(oop o, oop val) {
  o->obj_field_put(packages_offset, val);
}

void java_lang_AssertionStatusDirectives::set_packageEnabled(oop o, oop val) {
  o->obj_field_put(packageEnabled_offset, val);
}

void java_lang_AssertionStatusDirectives::set_deflt(oop o, bool val) {
  o->bool_field_put(deflt_offset, val);
}


// Support for intrinsification of java.nio.Buffer.checkIndex
int java_nio_Buffer::limit_offset() {
  return _limit_offset;
}


void java_nio_Buffer::compute_offsets() {
  klassOop k = SystemDictionary::java_nio_Buffer_klass();
  COMPUTE_OFFSET("java.nio.Buffer", _limit_offset, k, vmSymbols::limit_name(), vmSymbols::int_signature());
}

// Support for intrinsification of sun.misc.AtomicLongCSImpl.attemptUpdate
int sun_misc_AtomicLongCSImpl::value_offset() {
  assert(SystemDictionary::sun_misc_AtomicLongCSImpl_klass() != NULL, "can't call this");
  return _value_offset;
}


void sun_misc_AtomicLongCSImpl::compute_offsets() {
  klassOop k = SystemDictionary::sun_misc_AtomicLongCSImpl_klass();
  // If this class is not present, its value field offset won't be referenced.
  if (k != NULL) {
    COMPUTE_OFFSET("sun.misc.AtomicLongCSImpl", _value_offset, k, vmSymbols::sun_misc_AtomicLongCSImpl_value_name(), vmSymbols::long_signature());
  }
}


// Compute hard-coded offsets
// Invoked before SystemDictionary::initialize, so pre-loaded classes
// are not available to determine the offset_of_static_fields.
void JavaClasses::compute_hard_coded_offsets() {
  const int x = wordSize;  			
  const int header = instanceOopDesc::header_size_in_bytes();

  // Do the String Class
  java_lang_String::value_offset  = java_lang_String::hc_value_offset  * x + header;
  java_lang_String::offset_offset = java_lang_String::hc_offset_offset * x + header;
  java_lang_String::count_offset  = java_lang_String::offset_offset + sizeof (jint);
  java_lang_String::hash_offset   = java_lang_String::count_offset + sizeof (jint);

  // Do the Class Class
  java_lang_Class::klass_offset = java_lang_Class::hc_klass_offset * x + header;
  java_lang_Class::resolved_constructor_offset = java_lang_Class::hc_resolved_constructor_offset * x + header;

  // This is NOT an offset
  java_lang_Class::number_of_fake_oop_fields = java_lang_Class::hc_number_of_fake_oop_fields;

  // Throwable Class
  java_lang_Throwable::backtrace_offset  = java_lang_Throwable::hc_backtrace_offset  * x + header;
  java_lang_Throwable::detailMessage_offset = java_lang_Throwable::hc_detailMessage_offset * x + header;
  java_lang_Throwable::cause_offset      = java_lang_Throwable::hc_cause_offset      * x + header;
  java_lang_Throwable::stackTrace_offset = java_lang_Throwable::hc_stackTrace_offset * x + header;

  // java_lang_boxing_object
  java_lang_boxing_object::value_offset = java_lang_boxing_object::hc_value_offset * x + header;

  // java_lang_ref_Reference:
  java_lang_ref_Reference::referent_offset = java_lang_ref_Reference::hc_referent_offset * x + header;
  java_lang_ref_Reference::queue_offset = java_lang_ref_Reference::hc_queue_offset * x + header;
  java_lang_ref_Reference::next_offset  = java_lang_ref_Reference::hc_next_offset * x + header;
  java_lang_ref_Reference::discovered_offset  = java_lang_ref_Reference::hc_discovered_offset * x + header;
  java_lang_ref_Reference::static_lock_offset = java_lang_ref_Reference::hc_static_lock_offset *  x;
  java_lang_ref_Reference::static_pending_offset = java_lang_ref_Reference::hc_static_pending_offset * x;
  // Artificial fields for java_lang_ref_Reference
  // The first field is for the discovered field added in 1.4
  java_lang_ref_Reference::number_of_fake_oop_fields = 1;

  // java_lang_ref_SoftReference Class
  java_lang_ref_SoftReference::timestamp_offset = java_lang_ref_SoftReference::hc_timestamp_offset * x + header;
  // Don't multiply static fields because they are always in wordSize units
  java_lang_ref_SoftReference::static_clock_offset = java_lang_ref_SoftReference::hc_static_clock_offset * x;

  // java_lang_ClassLoader
  java_lang_ClassLoader::parent_offset = java_lang_ClassLoader::hc_parent_offset * x + header;

  // java_lang_System
  java_lang_System::static_in_offset  = java_lang_System::hc_static_in_offset  * x;
  java_lang_System::static_out_offset = java_lang_System::hc_static_out_offset * x;
  java_lang_System::static_err_offset = java_lang_System::hc_static_err_offset * x;

  // java_lang_StackTraceElement
  java_lang_StackTraceElement::className_offset  = java_lang_StackTraceElement::hc_className_offset  * x + header;
  java_lang_StackTraceElement::methodName_offset = java_lang_StackTraceElement::hc_methodName_offset * x + header;
  java_lang_StackTraceElement::fileName_offset   = java_lang_StackTraceElement::hc_fileName_offset   * x + header;
  java_lang_StackTraceElement::lineNumber_offset = java_lang_StackTraceElement::hc_lineNumber_offset * x + header;
  java_lang_AssertionStatusDirectives::classes_offset = java_lang_AssertionStatusDirectives::hc_classes_offset * x + header;
  java_lang_AssertionStatusDirectives::classEnabled_offset = java_lang_AssertionStatusDirectives::hc_classEnabled_offset * x + header;
  java_lang_AssertionStatusDirectives::packages_offset = java_lang_AssertionStatusDirectives::hc_packages_offset * x + header;
  java_lang_AssertionStatusDirectives::packageEnabled_offset = java_lang_AssertionStatusDirectives::hc_packageEnabled_offset * x + header;
  java_lang_AssertionStatusDirectives::deflt_offset = java_lang_AssertionStatusDirectives::hc_deflt_offset * x + header;

}
  

// Compute non-hard-coded field offsets of all the classes in this file
void JavaClasses::compute_offsets() {

  java_lang_System::compute_offsets();
  java_lang_Thread::compute_offsets();
  java_lang_ThreadGroup::compute_offsets();
  java_security_AccessControlContext::compute_offsets();
  // Initialize reflection classes. The layouts of these classes
  // changed with the new reflection implementation in JDK 1.4, and
  // since the Universe doesn't know what JDK version it is until this
  // point we defer computation of these offsets until now.
  java_lang_reflect_AccessibleObject::compute_offsets();
  java_lang_reflect_Method::compute_offsets();
  java_lang_reflect_Constructor::compute_offsets();
  java_lang_reflect_Field::compute_offsets();
  if (Universe::is_gte_jdk14x_version()) {
    java_nio_Buffer::compute_offsets();
  }
  if (Universe::is_gte_jdk15x_version()) {
    sun_reflect_ConstantPool::compute_offsets();
  }
  sun_misc_AtomicLongCSImpl::compute_offsets();
}

#ifndef PRODUCT

// These functions exist to assert the validity of hard-coded field offsets to guard 
// against changes in the class files

bool JavaClasses::check_offset(const char *klass_name, int hardcoded_offset, const char *field_name, const char* field_sig) {
  EXCEPTION_MARK;
  fieldDescriptor fd;
  symbolHandle klass_sym = oopFactory::new_symbol_handle(klass_name, CATCH);
  klassOop k = SystemDictionary::resolve_or_fail(klass_sym, true, CATCH);
  instanceKlassHandle h_klass (THREAD, k);
  //instanceKlassHandle h_klass(klass);
  symbolHandle f_name = oopFactory::new_symbol_handle(field_name, CATCH);
  symbolHandle f_sig  = oopFactory::new_symbol_handle(field_sig, CATCH);
  if (!h_klass->find_local_field(f_name(), f_sig(), &fd)) {
    tty->print_cr("Nonstatic field %s.%s not found", klass_name, field_name);
    return false;
  }
  if (fd.is_static()) {
    tty->print_cr("Nonstatic field %s.%s appears to be static", klass_name, field_name);
    return false;
  }
  if (fd.offset() == hardcoded_offset ) {
    return true;
  } else {
    tty->print_cr("Offset of nonstatic field %s.%s is hardcoded as %d but should really be %d.", klass_name, field_name, hardcoded_offset, fd.offset() - instanceOopDesc::header_size_in_bytes());
    return false;
  }
}


bool JavaClasses::check_static_offset(const char *klass_name, int hardcoded_offset, const char *field_name, const char* field_sig) {
  EXCEPTION_MARK;
  fieldDescriptor fd;
  symbolHandle klass_sym = oopFactory::new_symbol_handle(klass_name, CATCH);
  klassOop k = SystemDictionary::resolve_or_fail(klass_sym, true, CATCH);
  instanceKlassHandle h_klass (THREAD, k);
  symbolHandle f_name = oopFactory::new_symbol_handle(field_name, CATCH);
  symbolHandle f_sig  = oopFactory::new_symbol_handle(field_sig, CATCH);
  if (!h_klass->find_local_field(f_name(), f_sig(), &fd)) {
    tty->print_cr("Static field %s.%s not found", klass_name, field_name);
    return false;
  }
  if (!fd.is_static()) {
    tty->print_cr("Static field %s.%s appears to be nonstatic", klass_name, field_name);
    return false;
  }
  if (fd.offset() == hardcoded_offset + h_klass->offset_of_static_fields()) {
    return true;
  } else {
    tty->print_cr("Offset of static field %s.%s is hardcoded as %d but should really be %d.", klass_name, field_name, hardcoded_offset, fd.offset() - h_klass->offset_of_static_fields());
    return false;
  }
}


// Check the hard-coded field offsets of all the classes in this file

void JavaClasses::check_offsets() {
  bool valid = true;

#define CHECK_OFFSET(klass_name, cpp_klass_name, field_name, field_sig) \
  valid &= check_offset(klass_name, cpp_klass_name :: field_name ## _offset, #field_name, field_sig)

#define CHECK_STATIC_OFFSET(klass_name, cpp_klass_name, field_name, field_sig) \
  valid &= check_static_offset(klass_name, cpp_klass_name :: static_ ## field_name ## _offset, #field_name, field_sig)

  // java.lang.String

  CHECK_OFFSET("java/lang/String", java_lang_String, value, "[C");
  CHECK_OFFSET("java/lang/String", java_lang_String, offset, "I");
  CHECK_OFFSET("java/lang/String", java_lang_String, count, "I");
  
  // java.lang.Class

  // CHECK_OFFSET("java/lang/Class", klass); // %%% this needs to be checked
  // CHECK_OFFSET("java/lang/Class", resolved_constructor); // %%% this needs to be checked

  // java.lang.Throwable

  CHECK_OFFSET("java/lang/Throwable", java_lang_Throwable, backtrace, "Ljava/lang/Object;");
  CHECK_OFFSET("java/lang/Throwable", java_lang_Throwable, detailMessage, "Ljava/lang/String;");
  
  // Temporarily disabled for Merlin, enable later
  if (false && Universe::is_jdk14x_version()) {
    CHECK_OFFSET("java/lang/Throwable", java_lang_Throwable, stackTrace, "Ljava/lang/String;");
  }

  // java.lang.StackTraceElement

  // Temporarily disabled for Merlin, enable later
  if (false && Universe::is_jdk14x_version()) {
    CHECK_OFFSET("java/lang/StackTraceElement", java_lang_StackTraceElement, className,  "Ljava/lang/String;");
    CHECK_OFFSET("java/lang/StackTraceElement", java_lang_StackTraceElement, methodName, "Ljava/lang/String;");
    CHECK_OFFSET("java/lang/StackTraceElement", java_lang_StackTraceElement, fileName,   "Ljava/lang/String;");
    CHECK_OFFSET("java/lang/StackTraceElement", java_lang_StackTraceElement, lineNumber, "I");
  }

  // Boxed primitive objects (java_lang_boxing_object)

  CHECK_OFFSET("java/lang/Boolean",   java_lang_boxing_object, value, "Z");
  CHECK_OFFSET("java/lang/Character", java_lang_boxing_object, value, "C");
  CHECK_OFFSET("java/lang/Float",     java_lang_boxing_object, value, "F");
  CHECK_OFFSET("java/lang/Double",    java_lang_boxing_object, value, "D");
  CHECK_OFFSET("java/lang/Byte",      java_lang_boxing_object, value, "B");
  CHECK_OFFSET("java/lang/Short",     java_lang_boxing_object, value, "S");
  CHECK_OFFSET("java/lang/Integer",   java_lang_boxing_object, value, "I");
  CHECK_OFFSET("java/lang/Long",      java_lang_boxing_object, value, "J");

  // java.lang.ClassLoader

  CHECK_OFFSET("java/lang/ClassLoader", java_lang_ClassLoader, parent,      "Ljava/lang/ClassLoader;");

  // java.lang.System

  CHECK_STATIC_OFFSET("java/lang/System", java_lang_System,  in, "Ljava/io/InputStream;");
  CHECK_STATIC_OFFSET("java/lang/System", java_lang_System, out, "Ljava/io/PrintStream;");
  CHECK_STATIC_OFFSET("java/lang/System", java_lang_System, err, "Ljava/io/PrintStream;");

  // java.lang.ref.Reference

  CHECK_OFFSET("java/lang/ref/Reference", java_lang_ref_Reference, referent, "Ljava/lang/Object;");
  CHECK_OFFSET("java/lang/ref/Reference", java_lang_ref_Reference, queue, "Ljava/lang/ref/ReferenceQueue;");
  CHECK_OFFSET("java/lang/ref/Reference", java_lang_ref_Reference, next, "Ljava/lang/ref/Reference;");
  CHECK_STATIC_OFFSET("java/lang/ref/Reference", java_lang_ref_Reference, lock, "Ljava/lang/ref/Reference$Lock;");
  CHECK_STATIC_OFFSET("java/lang/ref/Reference", java_lang_ref_Reference, pending, "Ljava/lang/ref/Reference;");

  // java.lang.ref.SoftReference

  CHECK_OFFSET("java/lang/ref/SoftReference", java_lang_ref_SoftReference, timestamp, "J");
  CHECK_STATIC_OFFSET("java/lang/ref/SoftReference", java_lang_ref_SoftReference, clock, "J");

  // java.lang.AssertionStatusDirectives
  // 
  // The CheckAssertionStatusDirectives boolean can be removed from here and
  // globals.hpp after the AssertionStatusDirectives class has been integrated
  // into merlin "for some time."  Without it, the vm will fail with early
  // merlin builds.

  if (CheckAssertionStatusDirectives && Universe::is_gte_jdk14x_version()) {
    const char* nm = "java/lang/AssertionStatusDirectives";
    const char* sig = "[Ljava/lang/String;";
    CHECK_OFFSET(nm, java_lang_AssertionStatusDirectives, classes, sig);
    CHECK_OFFSET(nm, java_lang_AssertionStatusDirectives, classEnabled, "[Z");
    CHECK_OFFSET(nm, java_lang_AssertionStatusDirectives, packages, sig);
    CHECK_OFFSET(nm, java_lang_AssertionStatusDirectives, packageEnabled, "[Z");
    CHECK_OFFSET(nm, java_lang_AssertionStatusDirectives, deflt, "Z");
  }

  if (!valid) vm_exit_during_initialization("Hard-coded field offset verification failed");
}

#endif // PRODUCT


void javaClasses_init() {
  JavaClasses::compute_offsets();
  JavaClasses::check_offsets();
}
