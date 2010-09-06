#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nativeLookup.cpp	1.62 03/12/23 16:43:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nativeLookup.cpp.incl"


static void mangle_name_on(outputStream* st, symbolOop name, int begin, int end) {
  char* bytes = (char*)name->bytes() + begin;
  char* end_bytes = (char*)name->bytes() + end;
  while (bytes < end_bytes) {
    jchar c;
    bytes = UTF8::next(bytes, &c);
    if (c <= 0x7f && isalnum(c)) {
      st->put((char) c);
    } else {
           if (c == '_') st->print("_1");
      else if (c == '/') st->print("_");
      else if (c == ';') st->print("_2");
      else if (c == '[') st->print("_3");
      else               st->print("_%.5x", c);
    }
  }
}


static void mangle_name_on(outputStream* st, symbolOop name) {
  mangle_name_on(st, name, 0, name->utf8_length());
}


char* NativeLookup::pure_jni_name(methodHandle method) {
  stringStream st;
  // Prefix
  st.print("Java_");
  // Klass name
  mangle_name_on(&st, method->klass_name());
  st.print("_");
  // Method name
  mangle_name_on(&st, method->name());
  return st.as_string();
}


char* NativeLookup::long_jni_name(methodHandle method) {
  // Signature ignore the wrapping parenteses and the trailing return type
  stringStream st;
  symbolOop signature = method->signature();
  st.print("__");
  // find ')'
  int end;
  for (end = 0; end < signature->utf8_length() && signature->byte_at(end) != ')'; end++);
  // skip first '('
  mangle_name_on(&st, signature, 1, end);
  return st.as_string();
}

JNIEXPORT void JNICALL JVM_RegisterUnsafeMethods(JNIEnv *env, jclass unsafecls);
JNIEXPORT void JNICALL JVM_RegisterPerfMethods(JNIEnv *env, jclass perfclass);

static address lookup_special_native(char* jni_name) {
  // NB: To ignore the jni prefix and jni postfix strstr is used matching.
  // Intercept ObjectOutputStream getPrimitiveFieldValues for faster serialization
  if (strstr(jni_name, "Java_java_io_ObjectOutputStream_getPrimitiveFieldValues") != NULL) {
    return CAST_FROM_FN_PTR(address, JVM_GetPrimitiveFieldValues);
  }
  // Intercept ObjectInputStream setPrimitiveFieldValues for faster serialization
  if (strstr(jni_name, "Java_java_io_ObjectInputStream_setPrimitiveFieldValues") != NULL) {
    return CAST_FROM_FN_PTR(address, JVM_SetPrimitiveFieldValues);
  }
  if (strstr(jni_name, "Java_sun_misc_Unsafe_registerNatives") != NULL) {
    return CAST_FROM_FN_PTR(address, JVM_RegisterUnsafeMethods);
  }
  if (strstr(jni_name, "Java_sun_misc_Perf_registerNatives") != NULL) {
    return CAST_FROM_FN_PTR(address, JVM_RegisterPerfMethods);
  }

  return NULL;
}

address NativeLookup::lookup_style(methodHandle method, char* pure_name, const char* long_name, int args_size, bool os_style, bool& in_base_library, TRAPS) {
  address entry;
  // Compute complete JNI name for style
  stringStream st;
  if (os_style) os::print_jni_name_prefix_on(&st, args_size);
  st.print_raw(pure_name);
  st.print_raw(long_name);
  if (os_style) os::print_jni_name_suffix_on(&st, args_size);
  char* jni_name = st.as_string();

  // If the loader is null we have a system class, so we attempt a lookup in
  // the native Java library. This takes care of any bootstrapping problems.
  // Note: It is critical for bootstrapping that Java_java_lang_ClassLoader_00024NativeLibrary_find
  // gets found the first time around - otherwise an infinite loop can occure. This is
  // another VM/library dependency
  Handle loader(THREAD,
                instanceKlass::cast(method->method_holder())->class_loader());
  if (loader.is_null()) {
    entry = lookup_special_native(jni_name);
    if (entry == NULL) {
       entry = (address) hpi::dll_lookup(os::native_java_library(), jni_name);
    }
    if (entry != NULL) {
      in_base_library = true;
      return entry;
    }
  }  

  // Otherwise call static method findNative in ClassLoader
  KlassHandle   klass (THREAD, SystemDictionary::classloader_klass());
  Handle name_arg = java_lang_String::create_from_str(jni_name, CHECK_0);

  JavaValue result(T_LONG);
  JavaCalls::call_static(&result, 
                         klass,                        
                         vmSymbolHandles::findNative_name(), 
                         vmSymbolHandles::classloader_string_long_signature(),
                         // Arguments    
                         loader, 
                         name_arg,
                         CHECK_0);  
  entry = (address) result.get_jlong();

  if (entry == NULL) {
    // findNative didn't find it, if there are any agent libraries look in them
    AgentLibrary* agent;
    for (agent = Arguments::agents(); agent != NULL; agent = agent->next()) {
      entry = (address) hpi::dll_lookup(agent->os_lib(), jni_name);
      if (entry != NULL) {
        return entry;
      }
    }
  }

  return entry;
}


address NativeLookup::lookup_base(methodHandle method, bool& in_base_library, TRAPS) {
  address entry = NULL;
  in_base_library = false;
  ResourceMark rm(THREAD);
  // Compute pure name
  char* pure_name = pure_jni_name(method);

  // Compute argument size
  int args_size = 1                             // JNIEnv
                + (method->is_static() ? 1 : 0) // class for static methods 
                + method->size_of_parameters(); // actual parameters  
  

  // 1) Try JNI short style
  entry = lookup_style(method, pure_name, "",        args_size, true,  in_base_library, CHECK_0);
  if (entry != NULL) return entry;

  // Compute long name
  char* long_name = long_jni_name(method);

  // 2) Try JNI long style
  entry = lookup_style(method, pure_name, long_name, args_size, true,  in_base_library, CHECK_0);
  if (entry != NULL) return entry;

  // 3) Try JNI short style without os prefix/suffix
  entry = lookup_style(method, pure_name, "",        args_size, false, in_base_library, CHECK_0);
  if (entry != NULL) return entry;

  // 4) Try JNI long style without os prefix/suffix
  entry = lookup_style(method, pure_name, long_name, args_size, false, in_base_library, CHECK_0);
  if (entry != NULL) return entry;

  // Native function not found, throw UnsatisfiedLinkError
  THROW_MSG_0(vmSymbols::java_lang_UnsatisfiedLinkError(), method->name()->as_C_string()); 
}


address NativeLookup::lookup(methodHandle method, bool& in_base_library, TRAPS) {
  if (!method->has_native_function()) {
    address entry = lookup_base(method, in_base_library, CHECK_0);
    method->set_native_function(entry);
    // -verbose:jni printing
    if (PrintJNIResolving) {
      ResourceMark rm(THREAD);
      tty->print_cr("[Dynamic-linking native method %s.%s ... JNI]", 
        Klass::cast(method->method_holder())->external_name(), 
        method->name()->as_C_string());
    }
  }
  return method->native_function();
}

address NativeLookup::base_library_lookup(const char* class_name, const char* method_name, const char* signature) {
  EXCEPTION_MARK;
  bool in_base_library = true;  // SharedRuntime inits some math methods.
  symbolHandle c_name = oopFactory::new_symbol_handle(class_name,  CATCH);
  symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
  symbolHandle s_name = oopFactory::new_symbol_handle(signature,   CATCH);

  // Find the class
  klassOop k = SystemDictionary::resolve_or_fail(c_name, true, CATCH);
  instanceKlassHandle klass (THREAD, k);

  // Find method and invoke standard lookup
  methodHandle method (THREAD,
                       klass->uncached_lookup_method(m_name(), s_name()));
  address result = lookup(method, in_base_library, CATCH);
  assert(in_base_library, "must be in basic library");
  guarantee(result != NULL, "must be non NULL");
  return result;
}
