#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmSymbols.hpp	1.129 04/04/07 12:14:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The classes vmSymbols and vmSymbolHandles are a name spaces for fast lookup of 
// symbols commonly used in the VM. The first class return a symbolOop, while the
// second class returns a SymbolHandle. The underlying data structure is shared
// between the two classes.
//
// Sample usage:
//
//   symbolOop obj       = vmSymbols::java_lang_Object()();
//   SymbolHandle handle = vmSymbolHandles::java_lang_Object();


// Mapping function names to values. New entries should be added below.

#define VM_SYMBOLS_DO(template)                                                                   \
  /* commonly used class names */                                                                 \
  template(java_lang_System,                          "java/lang/System")                         \
  template(java_lang_Object,                          "java/lang/Object")                         \
  template(java_lang_Class,                           "java/lang/Class")                          \
  template(java_lang_String,                          "java/lang/String")                         \
  template(java_lang_Thread,                          "java/lang/Thread")                         \
  template(java_lang_ThreadGroup,                     "java/lang/ThreadGroup")                    \
  template(java_lang_Cloneable,                       "java/lang/Cloneable")                      \
  template(java_lang_Throwable,                       "java/lang/Throwable")                      \
  template(java_lang_ClassLoader,                     "java/lang/ClassLoader")                    \
  template(java_lang_ClassLoader_NativeLibrary,       "java/lang/ClassLoader\x024NativeLibrary")  \
  template(java_lang_ThreadDeath,                     "java/lang/ThreadDeath")                    \
  template(java_lang_Boolean,                         "java/lang/Boolean")                        \
  template(java_lang_Character,                       "java/lang/Character")                      \
  template(java_lang_Float,                           "java/lang/Float")                          \
  template(java_lang_Double,                          "java/lang/Double")                         \
  template(java_lang_Byte,                            "java/lang/Byte")                           \
  template(java_lang_Short,                           "java/lang/Short")                          \
  template(java_lang_Integer,                         "java/lang/Integer")                        \
  template(java_lang_Long,                            "java/lang/Long")                           \
  template(java_lang_Shutdown,                        "java/lang/Shutdown")                       \
  template(java_lang_ref_Reference,                   "java/lang/ref/Reference")                  \
  template(java_lang_ref_SoftReference,               "java/lang/ref/SoftReference")              \
  template(java_lang_ref_WeakReference,               "java/lang/ref/WeakReference")              \
  template(java_lang_ref_FinalReference,              "java/lang/ref/FinalReference")             \
  template(java_lang_ref_PhantomReference,            "java/lang/ref/PhantomReference")           \
  template(java_lang_ref_Finalizer,                   "java/lang/ref/Finalizer")                  \
  template(java_lang_reflect_AccessibleObject,        "java/lang/reflect/AccessibleObject")       \
  template(java_lang_reflect_Method,                  "java/lang/reflect/Method")                 \
  template(java_lang_reflect_Constructor,             "java/lang/reflect/Constructor")            \
  template(java_lang_reflect_Field,                   "java/lang/reflect/Field")                  \
  template(java_lang_StringBuffer,                    "java/lang/StringBuffer")                   \
  template(java_lang_CharSequence,                    "java/lang/CharSequence")                   \
  template(java_security_AccessControlContext,        "java/security/AccessControlContext")       \
  template(java_security_ProtectionDomain,            "java/security/ProtectionDomain")           \
  template(java_io_OutputStream,                      "java/io/OutputStream")                     \
  template(java_io_Reader,                            "java/io/Reader")                           \
  template(java_io_BufferedReader,                    "java/io/BufferedReader")                   \
  template(java_io_FileInputStream,                   "java/io/FileInputStream")                  \
  template(java_io_ByteArrayInputStream,              "java/io/ByteArrayInputStream")             \
  template(java_io_Serializable,                      "java/io/Serializable")                     \
  template(java_util_Properties,                      "java/util/Properties")                     \
  template(java_util_Vector,                          "java/util/Vector")                         \
  template(java_util_AbstractList,                    "java/util/AbstractList")                   \
  template(java_util_Hashtable,                       "java/util/Hashtable")                      \
  template(java_lang_Compiler,                        "java/lang/Compiler")                       \
  template(sun_misc_Signal,                           "sun/misc/Signal")                          \
  template(java_lang_AssertionStatusDirectives,       "java/lang/AssertionStatusDirectives")      \
                                                                                                  \
  /* class file format tags */                                                                    \
  template(tag_source_file,                           "SourceFile")                               \
  template(tag_inner_classes,                         "InnerClasses")                             \
  template(tag_constant_value,                        "ConstantValue")                            \
  template(tag_code,                                  "Code")                                     \
  template(tag_exceptions,                            "Exceptions")                               \
  template(tag_line_number_table,                     "LineNumberTable")                          \
  template(tag_local_variable_table,                  "LocalVariableTable")                       \
  template(tag_local_variable_type_table,             "LocalVariableTypeTable")                   \
  template(tag_stack_map,                             "StackMap")                                 \
  template(tag_synthetic,                             "Synthetic")                                \
  template(tag_deprecated,                            "Deprecated")                               \
  template(tag_source_debug_extension,                "SourceDebugExtension")                     \
  template(tag_signature,                             "Signature")                                \
  template(tag_runtime_visible_annotations,           "RuntimeVisibleAnnotations")                \
  template(tag_runtime_invisible_annotations,         "RuntimeInvisibleAnnotations")              \
  template(tag_runtime_visible_parameter_annotations, "RuntimeVisibleParameterAnnotations")       \
  template(tag_runtime_invisible_parameter_annotations,"RuntimeInvisibleParameterAnnotations")    \
  template(tag_annotation_default,                    "AnnotationDefault")                        \
  template(tag_enclosing_method,                      "EnclosingMethod")                          \
                                                                                                  \
  /* exception klasses: at least all exceptions thrown by the VM have entries here */             \
  template(java_lang_ArithmeticException,             "java/lang/ArithmeticException")            \
  template(java_lang_ArrayIndexOutOfBoundsException,  "java/lang/ArrayIndexOutOfBoundsException") \
  template(java_lang_ArrayStoreException,             "java/lang/ArrayStoreException")            \
  template(java_lang_ClassCastException,              "java/lang/ClassCastException")             \
  template(java_lang_ClassNotFoundException,          "java/lang/ClassNotFoundException")         \
  template(java_lang_CloneNotSupportedException,      "java/lang/CloneNotSupportedException")     \
  template(java_lang_IllegalAccessException,          "java/lang/IllegalAccessException")         \
  template(java_lang_IllegalArgumentException,        "java/lang/IllegalArgumentException")       \
  template(java_lang_IllegalMonitorStateException,    "java/lang/IllegalMonitorStateException")   \
  template(java_lang_IllegalThreadStateException,     "java/lang/IllegalThreadStateException")    \
  template(java_lang_IndexOutOfBoundsException,       "java/lang/IndexOutOfBoundsException")      \
  template(java_lang_InstantiationException,          "java/lang/InstantiationException")         \
  template(java_lang_InstantiationError,              "java/lang/InstantiationError")             \
  template(java_lang_InterruptedException,            "java/lang/InterruptedException")           \
  template(java_lang_LinkageError,                    "java/lang/LinkageError")                   \
  template(java_lang_NegativeArraySizeException,      "java/lang/NegativeArraySizeException")     \
  template(java_lang_NoSuchFieldException,            "java/lang/NoSuchFieldException")           \
  template(java_lang_NoSuchMethodException,           "java/lang/NoSuchMethodException")          \
  template(java_lang_NullPointerException,            "java/lang/NullPointerException")           \
  template(java_lang_StringIndexOutOfBoundsException, "java/lang/StringIndexOutOfBoundsException")\
  template(java_lang_InvalidClassException,           "java/lang/InvalidClassException")          \
  template(java_lang_reflect_InvocationTargetException, "java/lang/reflect/InvocationTargetException") \
  template(java_lang_Exception,                       "java/lang/Exception")                      \
  template(java_lang_RuntimeException,                "java/lang/RuntimeException")               \
  template(java_io_IOException,                       "java/io/IOException")                      \
  template(java_security_PrivilegedActionException,   "java/security/PrivilegedActionException")  \
                                                                                                  \
  /* error klasses: at least all errors thrown by the VM have entries here */                     \
  template(java_lang_AbstractMethodError,             "java/lang/AbstractMethodError")            \
  template(java_lang_ClassCircularityError,           "java/lang/ClassCircularityError")          \
  template(java_lang_ClassFormatError,                "java/lang/ClassFormatError")               \
  template(java_lang_UnsupportedClassVersionError,    "java/lang/UnsupportedClassVersionError")   \
  template(java_lang_Error,                           "java/lang/Error")                          \
  template(java_lang_ExceptionInInitializerError,     "java/lang/ExceptionInInitializerError")    \
  template(java_lang_IllegalAccessError,              "java/lang/IllegalAccessError")             \
  template(java_lang_IncompatibleClassChangeError,    "java/lang/IncompatibleClassChangeError")   \
  template(java_lang_InternalError,                   "java/lang/InternalError")                  \
  template(java_lang_NoClassDefFoundError,            "java/lang/NoClassDefFoundError")           \
  template(java_lang_NoSuchFieldError,                "java/lang/NoSuchFieldError")               \
  template(java_lang_NoSuchMethodError,               "java/lang/NoSuchMethodError")              \
  template(java_lang_OutOfMemoryError,                "java/lang/OutOfMemoryError")               \
  template(java_lang_UnsatisfiedLinkError,            "java/lang/UnsatisfiedLinkError")           \
  template(java_lang_VerifyError,                     "java/lang/VerifyError")                    \
  template(java_lang_StackOverflowError,              "java/lang/StackOverflowError")             \
  template(java_lang_StackTraceElement,               "java/lang/StackTraceElement")              \
                                                                                                  \
  /* handling native inlining */                                                                  \
  template(java_lang_Math,                            "java/lang/Math")                           \
  template(java_lang_Math_sin_name,                   "sin")                                      \
  template(java_lang_Math_sin_signature,              "(D)D")                                     \
  template(java_lang_Math_cos_name,                   "cos")                                      \
  template(java_lang_Math_cos_signature,              "(D)D")                                     \
  template(java_lang_Math_tan_name,                   "tan")                                      \
  template(java_lang_Math_tan_signature,              "(D)D")                                     \
  template(java_lang_Math_atan2_name,                 "atan2")                                    \
  template(java_lang_Math_atan2_signature,            "(DD)D")                                    \
  template(java_lang_Math_sqrt_name,                  "sqrt")                                     \
  template(java_lang_Math_sqrt_signature,             "(D)D")                                     \
  template(java_lang_Math_pow_name,                   "pow")                                      \
  template(java_lang_Math_pow_signature,              "(DD)D")                                    \
  template(java_lang_Float_floatToRawIntBits_name,    "floatToRawIntBits")                        \
  template(java_lang_Float_floatToIntBits_name,       "floatToIntBits")                           \
  template(java_lang_Float_intBitsToFloat_name,       "intBitsToFloat")                           \
  template(java_lang_Double_doubleToRawLongBits_name, "doubleToRawLongBits")                      \
  template(java_lang_Double_doubleToLongBits_name,    "doubleToLongBits")                         \
  template(java_lang_Double_longBitsToDouble_name,    "longBitsToDouble")                         \
  template(java_lang_System_arraycopy_name,           "arraycopy")                                \
  template(java_lang_System_arraycopy_signature,      "(Ljava/lang/Object;ILjava/lang/Object;II)V") \
  template(java_lang_System_currentTimeMillis_name,   "currentTimeMillis")                        \
  template(java_lang_System_currentTimeMillis_signature, "()J")                                   \
  template(java_lang_System_nanoTime_name,             "nanoTime")                                \
  template(java_lang_System_nanoTime_signature,       "()J")                                      \
  template(java_lang_Object_hashCode_name,            "hashCode")                                 \
  template(java_lang_Object_hashCode_signature,       "()I")                                      \
  template(java_lang_String_hashCode_name,            "hashCode")                                 \
  template(java_lang_String_hashCode_signature,       "()I")                                      \
  template(java_lang_System_identityHashCode_name,    "identityHashCode")                         \
  template(java_lang_System_identityHashCode_signature,"(Ljava/lang/Object;)I")                   \
  template(java_lang_Thread_currentThread_name,       "currentThread")                            \
  template(java_lang_Thread_currentThread_signature,  "()Ljava/lang/Thread;")                     \
  template(java_lang_Thread_isInterrupted_name,       "isInterrupted")                            \
  template(java_lang_Thread_isInterrupted_signature,  "(Z)Z")                                     \
  template(java_lang_Class_isInstance_name,           "isInstance")                               \
  template(java_lang_Class_isInstance_signature,      "(Ljava/lang/Object;)Z")                    \
  template(java_lang_Class_getModifiers_name,         "getModifiers")                             \
  template(java_lang_Class_getModifiers_signature,    "()I")                                      \
  template(java_lang_Object_getClass_name,            "getClass")                                 \
  template(java_lang_Object_getClass_signature,       "()Ljava/lang/Class;")                      \
  template(java_lang_String_compareTo_name,           "compareTo")                                \
  template(java_lang_String_compareTo_signature,      "(Ljava/lang/String;)I")                    \
  template(java_util_Vector_elementAt_name,           "elementAt")                                \
  template(java_util_Vector_elementAt_signature,      "(I)Ljava/lang/Object;")                    \
  template(java_nio_Buffer,                           "java/nio/Buffer")                          \
  template(java_nio_Buffer_checkIndex_name,           "checkIndex")                               \
  template(java_nio_Buffer_checkIndex_signature,      "(I)I")                                     \
  template(sun_misc_AtomicLongCSImpl,                 "sun/misc/AtomicLongCSImpl")                \
  template(sun_misc_AtomicLongCSImpl_attemptUpdate_name, "attemptUpdate")                         \
  template(sun_misc_AtomicLongCSImpl_attemptUpdate_signature, "(JJ)Z")                            \
  template(sun_misc_AtomicLongCSImpl_value_name, "value")                                         \
                                                                                                  \
  /* support for Unsafe class */                                                                  \
  template(sun_misc_Unsafe,                           "sun/misc/Unsafe")                          \
                                                                                                  \
  template(sun_misc_Unsafe_getObject_name,            "getObject")                                \
  template(sun_misc_Unsafe_getBoolean_name,           "getBoolean")                               \
  template(sun_misc_Unsafe_getByte_name,              "getByte")                                  \
  template(sun_misc_Unsafe_getShort_name,             "getShort")                                 \
  template(sun_misc_Unsafe_getChar_name,              "getChar")                                  \
  template(sun_misc_Unsafe_getInt_name,               "getInt")                                   \
  template(sun_misc_Unsafe_getLong_name,              "getLong")                                  \
  template(sun_misc_Unsafe_getFloat_name,             "getFloat")                                 \
  template(sun_misc_Unsafe_getDouble_name,            "getDouble")                                \
  template(sun_misc_Unsafe_getAddress_name,           "getAddress")                               \
                                                                                                  \
  template(sun_misc_Unsafe_putObject_name,            "putObject")                                \
  template(sun_misc_Unsafe_putBoolean_name,           "putBoolean")                               \
  template(sun_misc_Unsafe_putByte_name,              "putByte")                                  \
  template(sun_misc_Unsafe_putShort_name,             "putShort")                                 \
  template(sun_misc_Unsafe_putChar_name,              "putChar")                                  \
  template(sun_misc_Unsafe_putInt_name,               "putInt")                                   \
  template(sun_misc_Unsafe_putLong_name,              "putLong")                                  \
  template(sun_misc_Unsafe_putFloat_name,             "putFloat")                                 \
  template(sun_misc_Unsafe_putDouble_name,            "putDouble")                                \
  template(sun_misc_Unsafe_putAddress_name,           "putAddress")                               \
                                                                                                  \
  template(sun_misc_Unsafe_getObjectVolatile_name,    "getObjectVolatile")                        \
  template(sun_misc_Unsafe_getBooleanVolatile_name,   "getBooleanVolatile")                       \
  template(sun_misc_Unsafe_getByteVolatile_name,      "getByteVolatile")                          \
  template(sun_misc_Unsafe_getShortVolatile_name,     "getShortVolatile")                         \
  template(sun_misc_Unsafe_getCharVolatile_name,      "getCharVolatile")                          \
  template(sun_misc_Unsafe_getIntVolatile_name,       "getIntVolatile")                           \
  template(sun_misc_Unsafe_getLongVolatile_name,      "getLongVolatile")                          \
  template(sun_misc_Unsafe_getFloatVolatile_name,     "getFloatVolatile")                         \
  template(sun_misc_Unsafe_getDoubleVolatile_name,    "getDoubleVolatile")                        \
                                                                                                  \
  template(sun_misc_Unsafe_putObjectVolatile_name,    "putObjectVolatile")                        \
  template(sun_misc_Unsafe_putBooleanVolatile_name,   "putBooleanVolatile")                       \
  template(sun_misc_Unsafe_putByteVolatile_name,      "putByteVolatile")                          \
  template(sun_misc_Unsafe_putShortVolatile_name,     "putShortVolatile")                         \
  template(sun_misc_Unsafe_putCharVolatile_name,      "putCharVolatile")                          \
  template(sun_misc_Unsafe_putIntVolatile_name,       "putIntVolatile")                           \
  template(sun_misc_Unsafe_putLongVolatile_name,      "putLongVolatile")                          \
  template(sun_misc_Unsafe_putFloatVolatile_name,     "putFloatVolatile")                         \
  template(sun_misc_Unsafe_putDoubleVolatile_name,    "putDoubleVolatile")                        \
                                                                                                  \
  template(sun_misc_Unsafe_allocateInstance_name,     "allocateInstance")                         \
                                                                                                  \
  template(sun_misc_Unsafe_compareAndSwapObject_name, "compareAndSwapObject")                     \
  template(sun_misc_Unsafe_compareAndSwapLong_name,   "compareAndSwapLong")                       \
  template(sun_misc_Unsafe_compareAndSwapInt_name,    "compareAndSwapInt")                        \
  template(sun_misc_Unsafe_park_name,                 "park")                                     \
  template(sun_misc_Unsafe_unpark_name,               "unpark")                                   \
                                                                                                  \
  /* %%% the following are temporary until the 1.4.0 sun.misc.Unsafe goes away */                 \
  template(sun_misc_Unsafe_getObject_obj32_signature, "(Ljava/lang/Object;I)Ljava/lang/Object;")  \
  template(sun_misc_Unsafe_getBoolean_obj32_signature,"(Ljava/lang/Object;I)Z")                   \
  template(sun_misc_Unsafe_getByte_obj32_signature,   "(Ljava/lang/Object;I)B")                   \
  template(sun_misc_Unsafe_getShort_obj32_signature,  "(Ljava/lang/Object;I)S")                   \
  template(sun_misc_Unsafe_getChar_obj32_signature,   "(Ljava/lang/Object;I)C")                   \
  template(sun_misc_Unsafe_getInt_obj32_signature,    "(Ljava/lang/Object;I)I")                   \
  template(sun_misc_Unsafe_getLong_obj32_signature,   "(Ljava/lang/Object;I)J")                   \
  template(sun_misc_Unsafe_getFloat_obj32_signature,  "(Ljava/lang/Object;I)F")                   \
  template(sun_misc_Unsafe_getDouble_obj32_signature, "(Ljava/lang/Object;I)D")                   \
                                                                                                  \
  /* %%% the following are temporary until the 1.4.0 sun.misc.Unsafe goes away */                 \
  template(sun_misc_Unsafe_putObject_obj32_signature, "(Ljava/lang/Object;ILjava/lang/Object;)V") \
  template(sun_misc_Unsafe_putBoolean_obj32_signature,"(Ljava/lang/Object;IZ)V")                  \
  template(sun_misc_Unsafe_putByte_obj32_signature,   "(Ljava/lang/Object;IB)V")                  \
  template(sun_misc_Unsafe_putShort_obj32_signature,  "(Ljava/lang/Object;IS)V")                  \
  template(sun_misc_Unsafe_putChar_obj32_signature,   "(Ljava/lang/Object;IC)V")                  \
  template(sun_misc_Unsafe_putInt_obj32_signature,    "(Ljava/lang/Object;II)V")                  \
  template(sun_misc_Unsafe_putLong_obj32_signature,   "(Ljava/lang/Object;IJ)V")                  \
  template(sun_misc_Unsafe_putFloat_obj32_signature,  "(Ljava/lang/Object;IF)V")                  \
  template(sun_misc_Unsafe_putDouble_obj32_signature, "(Ljava/lang/Object;ID)V")                  \
                                                                                                  \
  template(sun_misc_Unsafe_getObject_obj_signature,   "(Ljava/lang/Object;J)Ljava/lang/Object;")  \
  template(sun_misc_Unsafe_getBoolean_obj_signature,  "(Ljava/lang/Object;J)Z")                   \
  template(sun_misc_Unsafe_getByte_obj_signature,     "(Ljava/lang/Object;J)B")                   \
  template(sun_misc_Unsafe_getShort_obj_signature,    "(Ljava/lang/Object;J)S")                   \
  template(sun_misc_Unsafe_getChar_obj_signature,     "(Ljava/lang/Object;J)C")                   \
  template(sun_misc_Unsafe_getInt_obj_signature,      "(Ljava/lang/Object;J)I")                   \
  template(sun_misc_Unsafe_getLong_obj_signature,     "(Ljava/lang/Object;J)J")                   \
  template(sun_misc_Unsafe_getFloat_obj_signature,    "(Ljava/lang/Object;J)F")                   \
  template(sun_misc_Unsafe_getDouble_obj_signature,   "(Ljava/lang/Object;J)D")                   \
                                                                                                  \
  template(sun_misc_Unsafe_putObject_obj_signature,   "(Ljava/lang/Object;JLjava/lang/Object;)V") \
  template(sun_misc_Unsafe_putBoolean_obj_signature,  "(Ljava/lang/Object;JZ)V")                  \
  template(sun_misc_Unsafe_putByte_obj_signature,     "(Ljava/lang/Object;JB)V")                  \
  template(sun_misc_Unsafe_putShort_obj_signature,    "(Ljava/lang/Object;JS)V")                  \
  template(sun_misc_Unsafe_putChar_obj_signature,     "(Ljava/lang/Object;JC)V")                  \
  template(sun_misc_Unsafe_putInt_obj_signature,      "(Ljava/lang/Object;JI)V")                  \
  template(sun_misc_Unsafe_putLong_obj_signature,     "(Ljava/lang/Object;JJ)V")                  \
  template(sun_misc_Unsafe_putFloat_obj_signature,    "(Ljava/lang/Object;JF)V")                  \
  template(sun_misc_Unsafe_putDouble_obj_signature,   "(Ljava/lang/Object;JD)V")                  \
                                                                                                  \
  template(sun_misc_Unsafe_getObjectVolatile_obj_signature,   "(Ljava/lang/Object;J)Ljava/lang/Object;")  \
  template(sun_misc_Unsafe_getBooleanVolatile_obj_signature,  "(Ljava/lang/Object;J)Z")           \
  template(sun_misc_Unsafe_getByteVolatile_obj_signature,     "(Ljava/lang/Object;J)B")           \
  template(sun_misc_Unsafe_getShortVolatile_obj_signature,    "(Ljava/lang/Object;J)S")           \
  template(sun_misc_Unsafe_getCharVolatile_obj_signature,     "(Ljava/lang/Object;J)C")           \
  template(sun_misc_Unsafe_getIntVolatile_obj_signature,      "(Ljava/lang/Object;J)I")           \
  template(sun_misc_Unsafe_getLongVolatile_obj_signature,     "(Ljava/lang/Object;J)J")           \
  template(sun_misc_Unsafe_getFloatVolatile_obj_signature,    "(Ljava/lang/Object;J)F")           \
  template(sun_misc_Unsafe_getDoubleVolatile_obj_signature,   "(Ljava/lang/Object;J)D")           \
                                                                                                  \
  template(sun_misc_Unsafe_putObjectVolatile_obj_signature,   "(Ljava/lang/Object;JLjava/lang/Object;)V") \
  template(sun_misc_Unsafe_putBooleanVolatile_obj_signature,  "(Ljava/lang/Object;JZ)V")          \
  template(sun_misc_Unsafe_putByteVolatile_obj_signature,     "(Ljava/lang/Object;JB)V")          \
  template(sun_misc_Unsafe_putShortVolatile_obj_signature,    "(Ljava/lang/Object;JS)V")          \
  template(sun_misc_Unsafe_putCharVolatile_obj_signature,     "(Ljava/lang/Object;JC)V")          \
  template(sun_misc_Unsafe_putIntVolatile_obj_signature,      "(Ljava/lang/Object;JI)V")          \
  template(sun_misc_Unsafe_putLongVolatile_obj_signature,     "(Ljava/lang/Object;JJ)V")          \
  template(sun_misc_Unsafe_putFloatVolatile_obj_signature,    "(Ljava/lang/Object;JF)V")          \
  template(sun_misc_Unsafe_putDoubleVolatile_obj_signature,   "(Ljava/lang/Object;JD)V")          \
                                                                                                  \
  template(sun_misc_Unsafe_getByte_raw_signature,     "(J)B")                                     \
  template(sun_misc_Unsafe_getShort_raw_signature,    "(J)S")                                     \
  template(sun_misc_Unsafe_getChar_raw_signature,     "(J)C")                                     \
  template(sun_misc_Unsafe_getInt_raw_signature,      "(J)I")                                     \
  template(sun_misc_Unsafe_getLong_raw_signature,     "(J)J")                                     \
  template(sun_misc_Unsafe_getFloat_raw_signature,    "(J)F")                                     \
  template(sun_misc_Unsafe_getDouble_raw_signature,   "(J)D")                                     \
  template(sun_misc_Unsafe_getAddress_raw_signature,  "(J)J")                                     \
                                                                                                  \
  template(sun_misc_Unsafe_putByte_raw_signature,     "(JB)V")                                    \
  template(sun_misc_Unsafe_putShort_raw_signature,    "(JS)V")                                    \
  template(sun_misc_Unsafe_putChar_raw_signature,     "(JC)V")                                    \
  template(sun_misc_Unsafe_putInt_raw_signature,      "(JI)V")                                    \
  template(sun_misc_Unsafe_putLong_raw_signature,     "(JJ)V")                                    \
  template(sun_misc_Unsafe_putFloat_raw_signature,    "(JF)V")                                    \
  template(sun_misc_Unsafe_putDouble_raw_signature,   "(JD)V")                                    \
  template(sun_misc_Unsafe_putAddress_raw_signature,  "(JJ)V")                                    \
                                                                                                  \
  template(sun_misc_Unsafe_compareAndSwapObject_obj_signature,   "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z") \
  template(sun_misc_Unsafe_compareAndSwapLong_obj_signature,  "(Ljava/lang/Object;JJJ)Z")         \
  template(sun_misc_Unsafe_compareAndSwapInt_obj_signature,   "(Ljava/lang/Object;JII)Z")         \
  template(sun_misc_Unsafe_park_signature,            "(ZJ)V")                                    \
  template(sun_misc_Unsafe_unpark_signature,           "(Ljava/lang/Object;)V")                    \
                                                                                                  \
  template(sun_misc_Unsafe_allocateInstance_signature,"(Ljava/lang/Class;)Ljava/lang/Object;")    \
                                                                                                  \
  /* Support for reflection based on dynamic bytecode generation (JDK 1.4 and above) */           \
                                                                                                  \
  template(sun_reflect_FieldInfo,                     "sun/reflect/FieldInfo")                    \
  template(sun_reflect_MethodInfo,                    "sun/reflect/MethodInfo")                   \
  template(sun_reflect_MagicAccessorImpl,             "sun/reflect/MagicAccessorImpl")            \
  template(sun_reflect_MethodAccessorImpl,            "sun/reflect/MethodAccessorImpl")           \
  template(sun_reflect_ConstructorAccessorImpl,       "sun/reflect/ConstructorAccessorImpl")      \
  template(sun_reflect_SerializationConstructorAccessorImpl, "sun/reflect/SerializationConstructorAccessorImpl") \
  template(sun_reflect_DelegatingClassLoader,         "sun/reflect/DelegatingClassLoader")        \
  template(sun_reflect_Reflection,                    "sun/reflect/Reflection")                   \
  template(checkedExceptions_name,                    "checkedExceptions")                        \
  template(clazz_name,                                "clazz")                                    \
  template(exceptionTypes_name,                       "exceptionTypes")                           \
  template(getClassAccessFlags_name,                  "getClassAccessFlags")                      \
  template(getClassAccessFlags_signature,             "(Ljava/lang/Class;)I")                     \
  template(modifiers_name,                            "modifiers")                                \
  template(newConstructor_name,                       "newConstructor")                           \
  template(newConstructor_signature,                  "(Lsun/reflect/MethodInfo;)Ljava/lang/reflect/Constructor;") \
  template(newField_name,                             "newField")                                 \
  template(newField_signature,                        "(Lsun/reflect/FieldInfo;)Ljava/lang/reflect/Field;") \
  template(newMethod_name,                            "newMethod")                                \
  template(newMethod_signature,                       "(Lsun/reflect/MethodInfo;)Ljava/lang/reflect/Method;") \
  template(override_name,                             "override")                                 \
  template(parameterTypes_name,                       "parameterTypes")                           \
  template(returnType_name,                           "returnType")                               \
  template(signature_name,                            "signature")                                \
  template(slot_name,                                 "slot")                                     \
                                                                                                  \
  /* Support for annotations (JDK 1.5 and above) */                                               \
                                                                                                  \
  template(annotations_name,                          "annotations")                              \
  template(parameter_annotations_name,                "parameterAnnotations")                     \
  template(annotation_default_name,                   "annotationDefault")                        \
  template(sun_reflect_ConstantPool,                  "sun/reflect/ConstantPool")                 \
  template(constantPoolOop_name,                      "constantPoolOop")                          \
                                                                                                  \
  /* common method names */                                                                       \
  template(object_initializer_name,                   "<init>")                                   \
  template(class_initializer_name,                    "<clinit>")                                 \
  template(println_name,                              "println")                                  \
  template(printStackTrace_name,                      "printStackTrace")                          \
  template(main_name,                                 "main")                                     \
  template(name_name,                                 "name")                                     \
  template(priority_name,                             "priority")                                 \
  template(stillborn_name,                            "stillborn")                                \
  template(group_name,                                "group")                                    \
  template(daemon_name,                               "daemon")                                   \
  template(eetop_name,                                "eetop")                                    \
  template(thread_status_name,                        "threadStatus")                             \
  template(run_method_name,                           "run")                                      \
  template(exit_method_name,                          "exit")                                     \
  template(add_method_name,                           "add")                                     \
  template(parent_name,                               "parent")                                   \
  template(threads_name,                              "threads")                                  \
  template(groups_name,                               "groups")                                   \
  template(maxPriority_name,                          "maxPriority")                              \
  template(destroyed_name,                            "destroyed")                                \
  template(vmAllowSuspension_name,                    "vmAllowSuspension")                        \
  template(nthreads_name,                             "nthreads")                                 \
  template(ngroups_name,                              "ngroups")                                  \
  template(shutdown_method_name,                      "shutdown")                                 \
  template(finalize_method_name,                      "finalize")                                 \
  template(register_method_name,                      "register")                                 \
  template(reference_lock_name,                       "lock")                                     \
  template(reference_discovered_name,                 "discovered")                               \
  template(run_finalizers_on_exit_name,               "runFinalizersOnExit")                      \
  template(uncaughtException_name,                    "uncaughtException")                        \
  template(dispatchUncaughtException_name,            "dispatchUncaughtException")                \
  template(initializeSystemClass_name,                "initializeSystemClass")                    \
  template(loadClass_name,                            "loadClass")                                \
  template(loadClassInternal_name,                    "loadClassInternal")                        \
  template(put_name,                                  "put")                                      \
  template(type_name,                                 "type")                                     \
  template(findNative_name,                           "findNative")                               \
  template(deadChild_name,                            "deadChild")                                \
  template(invoke_name,                               "invoke")                                   \
  template(addClass_name,                             "addClass")                                 \
  template(getFromClass_name,                         "getFromClass")                             \
  template(dispatch_name,                             "dispatch")                                 \
  template(getSystemClassLoader_name,                 "getSystemClassLoader")                     \
  template(fillInStackTrace_name,                     "fillInStackTrace")                         \
  template(setProperty_name,                          "setProperty")                              \
  template(getProperty_name,                          "getProperty")                              \
  template(context_name,                              "context")                                  \
  template(privilegedContext_name,                    "privilegedContext")                        \
  template(contextClassLoader_name,		      "contextClassLoader")			  \
  template(inheritedAccessControlContext_name,        "inheritedAccessControlContext")            \
  template(isPrivileged_name,                         "isPrivileged")                             \
  template(compareTo_name,                            "compareTo")                                \
  template(clone_name,                                 "clone")                                   \
  template(wait_name,                                 "wait")                                     \
  template(checkPackageAccess_name,                   "checkPackageAccess")                       \
  template(stackSize_name,                            "stackSize")                                \
  template(thread_id_name,                            "tid")                                      \
  template(newInstance0_name,                         "newInstance0")                             \
  template(limit_name,                                "limit")                                    \
  template(forName_name,                              "forName")                                  \
  template(forName0_name,                             "forName0")                                 \
  template(isJavaIdentifierStart_name,                "isJavaIdentifierStart")                    \
  template(isJavaIdentifierPart_name,                 "isJavaIdentifierPart")                     \
                                                                                                  \
  /* common signatures names */                                                                   \
  template(void_method_signature,                     "()V")                                      \
  template(int_void_signature,                        "(I)V")                                     \
  template(int_bool_signature,                        "(I)Z")                                     \
  template(float_int_signature,                       "(F)I")                                     \
  template(double_long_signature,                     "(D)J")                                     \
  template(int_float_signature,                       "(I)F")                                     \
  template(long_double_signature,                     "(J)D")                                     \
  template(byte_signature,                            "B")                                        \
  template(char_signature,                            "C")                                        \
  template(double_signature,                          "D")                                        \
  template(float_signature,                           "F")                                        \
  template(int_signature,                             "I")                                        \
  template(long_signature,                            "J")                                        \
  template(short_signature,                           "S")                                        \
  template(bool_signature,                            "Z")                                        \
  template(void_signature,                            "V")                                        \
  template(byte_array_signature,                      "[B")                                       \
  template(char_array_signature,                      "[C")                                       \
  template(register_method_signature,                 "(Ljava/lang/Object;)V")                    \
  template(string_void_signature,                     "(Ljava/lang/String;)V")                    \
  template(string_int_signature,                      "(Ljava/lang/String;)I")                    \
  template(throwable_void_signature,                  "(Ljava/lang/Throwable;)V")                 \
  template(class_void_signature,                      "(Ljava/lang/Class;)V")                     \
  template(throwable_string_void_signature,           "(Ljava/lang/Throwable;Ljava/lang/String;)V")               \
  template(string_array_void_signature,               "([Ljava/lang/String;)V")                                   \
  template(string_array_string_array_void_signature,  "([Ljava/lang/String;[Ljava/lang/String;)V")                \
  template(thread_throwable_void_signature,           "(Ljava/lang/Thread;Ljava/lang/Throwable;)V")               \
  template(thread_void_signature,                     "(Ljava/lang/Thread;)V")                                    \
  template(threadgroup_runnable_void_signature,       "(Ljava/lang/ThreadGroup;Ljava/lang/Runnable;)V")           \
  template(threadgroup_string_void_signature,         "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V")             \
  template(string_class_signature,                    "(Ljava/lang/String;)Ljava/lang/Class;")                    \
  template(object_object_object_signature,            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;") \
  template(string_string_string_signature,            "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;") \
  template(string_string_signature,                   "(Ljava/lang/String;)Ljava/lang/String;")                   \
  template(classloader_string_long_signature,         "(Ljava/lang/ClassLoader;Ljava/lang/String;)J")             \
  template(byte_array_void_signature,                 "([B)V")                                                    \
  template(char_array_void_signature,                 "([C)V")                                                    \
  template(int_int_void_signature,                    "(II)V")                                                    \
  template(void_classloader_signature,                "()Ljava/lang/ClassLoader;")                                \
  template(void_object_signature,                     "()Ljava/lang/Object;")                                     \
  template(void_class_signature,                      "()Ljava/lang/Class;")                                      \
  template(object_array_object_object_signature,      "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;")\
  template(exception_void_signature,                  "(Ljava/lang/Exception;)V")                                 \
  template(protectiondomain_signature,                "[Ljava/security/ProtectionDomain;")                        \
  template(accesscontrolcontext_signature,            "Ljava/security/AccessControlContext;")                     \
  template(class_protectiondomain_signature,          "(Ljava/lang/Class;Ljava/security/ProtectionDomain;)V")     \
  template(thread_array_signature,                    "[Ljava/lang/Thread;")                                      \
  template(threadgroup_signature,                     "Ljava/lang/ThreadGroup;")                                  \
  template(threadgroup_array_signature,               "[Ljava/lang/ThreadGroup;")                                 \
  template(class_array_signature,                     "[Ljava/lang/Class;")                                       \
  template(classloader_signature,		      "Ljava/lang/ClassLoader;")				  \
  template(object_signature,                          "Ljava/lang/Object;")                                       \
  template(class_signature,                           "Ljava/lang/Class;")                                        \
  template(string_signature,                          "Ljava/lang/String;")                                       \
  template(reference_signature,                       "Ljava/lang/ref/Reference;")                                \
                                                                                                                  \
  /* returned by the C1 compiler in case there's not enough memory to allocate a new symbol*/                     \
  template(dummy_symbol_oop,                          "illegal symbol")                                           \
                                                                                                                  \
  /* used by ClassFormatError when class name is not known yet */                                                 \
  template(unknown_class_name,                        "<Unknown>")                                                \
                                                                                                                  \
  /* JVM monitoring and management support */                                                                     \
  template(java_lang_StackTraceElement_array,          "[Ljava/lang/StackTraceElement;")                          \
  template(java_lang_management_ThreadState,           "java/lang/management/ThreadState")                        \
  template(java_lang_management_MemoryUsage,           "java/lang/management/MemoryUsage")                        \
  template(java_lang_management_ThreadInfo,            "java/lang/management/ThreadInfo")                         \
  template(java_lang_management_MemoryPoolMBean,       "java/lang/management/MemoryPoolMBean")                    \
  template(java_lang_management_MemoryManagerMBean,    "java/lang/management/MemoryManagerMBean")                 \
  template(java_lang_management_GarbageCollectorMBean, "java/lang/management/GarbageCollectorMBean")              \
  template(sun_management_ManagementFactory,           "sun/management/ManagementFactory")                        \
  template(sun_management_Sensor,                      "sun/management/Sensor")                                   \
  template(sun_management_Agent,                       "sun/management/Agent")                                    \
  template(createMemoryPoolMBean_name,                 "createMemoryPoolMBean")                                   \
  template(createMemoryManagerMBean_name,              "createMemoryManagerMBean")                                \
  template(createGarbageCollectorMBean_name,           "createGarbageCollectorMBean")                             \
  template(createMemoryPoolMBean_signature,            "(Ljava/lang/String;ZJJ)Ljava/lang/management/MemoryPoolMBean;") \
  template(createMemoryManagerMBean_signature,         "(Ljava/lang/String;)Ljava/lang/management/MemoryManagerMBean;") \
  template(createGarbageCollectorMBean_signature,      "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/management/GarbageCollectorMBean;") \
  template(trigger_name,                               "trigger")                                                 \
  template(clear_name,                                 "clear")                                                   \
  template(trigger_method_signature,                   "(ILjava/lang/management/MemoryUsage;)V")                                                 \
  template(startAgent_name,                            "startAgent")                                              \
  template(java_lang_management_ThreadInfo_constructor_signature, "(Ljava/lang/Thread;ILjava/lang/Object;Ljava/lang/Thread;JJJJ[Ljava/lang/StackTraceElement;)V") \
  template(long_long_long_long_void_signature,         "(JJJJ)V")                                                 \
                                                                                                                  \
  template(java_lang_management_MemoryPoolMXBean,      "java/lang/management/MemoryPoolMXBean")                   \
  template(java_lang_management_MemoryManagerMXBean,   "java/lang/management/MemoryManagerMXBean")                \
  template(java_lang_management_GarbageCollectorMXBean,"java/lang/management/GarbageCollectorMXBean")             \
  template(createMemoryPool_name,                      "createMemoryPool")                                        \
  template(createMemoryManager_name,                   "createMemoryManager")                                     \
  template(createGarbageCollector_name,                "createGarbageCollector")                                  \
  template(createMemoryPool_signature,                 "(Ljava/lang/String;ZJJ)Ljava/lang/management/MemoryPoolMXBean;") \
  template(createMemoryManager_signature,              "(Ljava/lang/String;)Ljava/lang/management/MemoryManagerMXBean;") \
  template(createGarbageCollector_signature,           "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/management/GarbageCollectorMXBean;") \
 
                
// Name macros

#define VM_SYMBOL_ENUM_NAME(name)    name##_enum
#define VM_SYMBOL_ENUM(name, string) VM_SYMBOL_ENUM_NAME(name),


// Declaration macros

#define VM_SYMBOL_DECLARE(name, string) \
  static symbolOop name() { return _symbols[VM_SYMBOL_ENUM_NAME(name)]; }

#define VM_SYMBOL_HANDLE_DECLARE(name, string) \
  static symbolHandle name() { return symbol_handle_at(vmSymbols::VM_SYMBOL_ENUM_NAME(name)); }


// Class vmSymbols

class vmSymbols: AllStatic {
 friend class vmSymbolHandles;
 public:
  // enum for figuring positions and size of array holding symbolOops
  enum {
    VM_SYMBOLS_DO(VM_SYMBOL_ENUM)
    vm_symbols_terminating_enum
  };

 private:
  // The symbol array
  static symbolOop _symbols[];

  // Field signatures indexed by BasicType.
  static symbolOop _type_signatures[T_VOID+1];

 public:
  // Initialization
  static void initialize(TRAPS);
  // Accessing
  VM_SYMBOLS_DO(VM_SYMBOL_DECLARE)
  // GC support
  static void oops_do(OopClosure* f, bool do_all = false);

  static symbolOop type_signature(BasicType t) {
    assert((uint)t < T_VOID+1, "range check");
    assert(_type_signatures[t] != NULL, "domain check");
    return _type_signatures[t];
  }
  // inverse of type_signature; returns T_OBJECT if s is not recognized
  static BasicType signature_type(symbolOop s);
};


// Class vmSymbolHandles

class vmSymbolHandles: AllStatic {
  friend class vmIntrinsics;
  friend class ciObjectFactory;
  static int symbol_handle_count() { return (int)vmSymbols::vm_symbols_terminating_enum; }
  static symbolHandle symbol_handle_at(int n) { return symbolHandle(&vmSymbols::_symbols[n], false); }

 public:
  // Accessing
  VM_SYMBOLS_DO(VM_SYMBOL_HANDLE_DECLARE)

  static symbolHandle type_signature(BasicType t) {
    assert(vmSymbols::type_signature(t) != NULL, "domain check");
    return symbolHandle(&vmSymbols::_type_signatures[t], false);
  }
  // inverse of type_signature; returns T_OBJECT if s is not recognized
  static BasicType signature_type(symbolHandle s) {
    return vmSymbols::signature_type(s());
  }
};


#undef VM_SYMBOL_ENUM
#undef VM_SYMBOL_DECLARE
#undef VM_SYMBOL_HANDLE_DECLARE

// Define here intrinsics known to the runtime and the CI.
// Each intrinsic consists of a public enum name (like _hash),
// followed by a specification of its klass, name, and signature:
//    template(<id>,  <klass>,  <name>,<sig>)
//
// If you add an intrinsic here, you must also add its name
// and signature as VM symbols above.  The VM symbols for
// the intrinsic name and signature are <klass>_<name>_name
// and <klass>_<sig>_signature.
//
// For example:
//    template(_hash,  java_lang_Object,  hashCode,hashCode)
// klass      = vmSymbols::java_lang_Object()
// name       = vmSymbols::java_lang_Object_hashCode_name()
// signature  = vmSymbols::java_lang_Object_hashCode_signature()
//
// (Note that the <name> and <sig> are often, but not always, the same.)

#define VM_INTRINSICS_DO(template)                                                                \
  template(_hash,             java_lang_Object,   hashCode,hashCode)                              \
  template(_getClass,         java_lang_Object,   getClass,getClass)                              \
  template(_identityHash,     java_lang_System,   identityHashCode,identityHashCode)              \
  template(_dsin,             java_lang_Math,     sin,sin)                                        \
  template(_dcos,             java_lang_Math,     cos,cos)                                        \
  template(_dtan,             java_lang_Math,     tan,tan)                                        \
  template(_datan2,           java_lang_Math,     atan2,atan2)                                    \
  template(_dsqrt,            java_lang_Math,     sqrt,sqrt)                                      \
  template(_dpow,             java_lang_Math,     pow,pow)                                        \
  template(_floatToRawIntBits,java_lang_Float,    floatToRawIntBits,float_int)                    \
  template(_floatToIntBits,   java_lang_Float,    floatToIntBits,float_int)                       \
  template(_intBitsToFloat,   java_lang_Float,    intBitsToFloat,int_float)                       \
  template(_doubleToRawLongBits,java_lang_Double, doubleToRawLongBits,double_long)                \
  template(_doubleToLongBits, java_lang_Double,   doubleToLongBits,double_long)                   \
  template(_longBitsToDouble, java_lang_Double,   longBitsToDouble,long_double)                   \
  template(_arraycopy,        java_lang_System,   arraycopy,arraycopy)                            \
  template(_currentTimeMillis,java_lang_System,   currentTimeMillis,currentTimeMillis)            \
  template(_nanoTime,         java_lang_System,   nanoTime,nanoTime)                              \
  template(_currentThread,    java_lang_Thread,   currentThread,currentThread)                    \
  template(_isInterrupted,    java_lang_Thread,   isInterrupted,isInterrupted)                    \
  template(_isInstance,       java_lang_Class,    isInstance,isInstance)                          \
  template(_getModifiers,     java_lang_Class,    getModifiers,getModifiers)                      \
  template(_getClassAccessFlags,sun_reflect_Reflection,getClassAccessFlags,getClassAccessFlags)   \
  template(_compareTo,        java_lang_String,   compareTo,compareTo)                            \
  template(_checkIndex,       java_nio_Buffer,    checkIndex,checkIndex)                          \
  template(_attemptUpdate,    sun_misc_AtomicLongCSImpl, attemptUpdate,attemptUpdate)             \
  /* %%% the following xxx_obj32 are temporary until the 1.4.0 sun.misc.Unsafe goes away */       \
  template(_getObject_obj32,  sun_misc_Unsafe,    getObject,getObject_obj32)                      \
  template(_getBoolean_obj32, sun_misc_Unsafe,    getBoolean,getBoolean_obj32)                    \
  template(_getByte_obj32,    sun_misc_Unsafe,    getByte,getByte_obj32)                          \
  template(_getShort_obj32,   sun_misc_Unsafe,    getShort,getShort_obj32)                        \
  template(_getChar_obj32,    sun_misc_Unsafe,    getChar,getChar_obj32)                          \
  template(_getInt_obj32,     sun_misc_Unsafe,    getInt,getInt_obj32)                            \
  template(_getLong_obj32,    sun_misc_Unsafe,    getLong,getLong_obj32)                          \
  template(_getFloat_obj32,   sun_misc_Unsafe,    getFloat,getFloat_obj32)                        \
  template(_getDouble_obj32,  sun_misc_Unsafe,    getDouble,getDouble_obj32)                      \
  template(_putObject_obj32,  sun_misc_Unsafe,    putObject,putObject_obj32)                      \
  template(_putBoolean_obj32, sun_misc_Unsafe,    putBoolean,putBoolean_obj32)                    \
  template(_putByte_obj32,    sun_misc_Unsafe,    putByte,putByte_obj32)                          \
  template(_putShort_obj32,   sun_misc_Unsafe,    putShort,putShort_obj32)                        \
  template(_putChar_obj32,    sun_misc_Unsafe,    putChar,putChar_obj32)                          \
  template(_putInt_obj32,     sun_misc_Unsafe,    putInt,putInt_obj32)                            \
  template(_putLong_obj32,    sun_misc_Unsafe,    putLong,putLong_obj32)                          \
  template(_putFloat_obj32,   sun_misc_Unsafe,    putFloat,putFloat_obj32)                        \
  template(_putDouble_obj32,  sun_misc_Unsafe,    putDouble,putDouble_obj32)                      \
  template(_getObject_obj,    sun_misc_Unsafe,    getObject,getObject_obj)                        \
  template(_getBoolean_obj,   sun_misc_Unsafe,    getBoolean,getBoolean_obj)                      \
  template(_getByte_obj,      sun_misc_Unsafe,    getByte,getByte_obj)                            \
  template(_getShort_obj,     sun_misc_Unsafe,    getShort,getShort_obj)                          \
  template(_getChar_obj,      sun_misc_Unsafe,    getChar,getChar_obj)                            \
  template(_getInt_obj,       sun_misc_Unsafe,    getInt,getInt_obj)                              \
  template(_getLong_obj,      sun_misc_Unsafe,    getLong,getLong_obj)                            \
  template(_getFloat_obj,     sun_misc_Unsafe,    getFloat,getFloat_obj)                          \
  template(_getDouble_obj,    sun_misc_Unsafe,    getDouble,getDouble_obj)                        \
  template(_putObject_obj,    sun_misc_Unsafe,    putObject,putObject_obj)                        \
  template(_putBoolean_obj,   sun_misc_Unsafe,    putBoolean,putBoolean_obj)                      \
  template(_putByte_obj,      sun_misc_Unsafe,    putByte,putByte_obj)                            \
  template(_putShort_obj,     sun_misc_Unsafe,    putShort,putShort_obj)                          \
  template(_putChar_obj,      sun_misc_Unsafe,    putChar,putChar_obj)                            \
  template(_putInt_obj,       sun_misc_Unsafe,    putInt,putInt_obj)                              \
  template(_putLong_obj,      sun_misc_Unsafe,    putLong,putLong_obj)                            \
  template(_putFloat_obj,     sun_misc_Unsafe,    putFloat,putFloat_obj)                          \
  template(_putDouble_obj,    sun_misc_Unsafe,    putDouble,putDouble_obj)                        \
  template(_getObjectVolatile_obj,    sun_misc_Unsafe,    getObjectVolatile,getObjectVolatile_obj)   \
  template(_getBooleanVolatile_obj,   sun_misc_Unsafe,    getBooleanVolatile,getBooleanVolatile_obj) \
  template(_getByteVolatile_obj,      sun_misc_Unsafe,    getByteVolatile,getByteVolatile_obj)       \
  template(_getShortVolatile_obj,     sun_misc_Unsafe,    getShortVolatile,getShortVolatile_obj)     \
  template(_getCharVolatile_obj,      sun_misc_Unsafe,    getCharVolatile,getCharVolatile_obj)       \
  template(_getIntVolatile_obj,       sun_misc_Unsafe,    getIntVolatile,getIntVolatile_obj)         \
  template(_getLongVolatile_obj,      sun_misc_Unsafe,    getLongVolatile,getLongVolatile_obj)       \
  template(_getFloatVolatile_obj,     sun_misc_Unsafe,    getFloatVolatile,getFloatVolatile_obj)     \
  template(_getDoubleVolatile_obj,    sun_misc_Unsafe,    getDoubleVolatile,getDoubleVolatile_obj)   \
  template(_putObjectVolatile_obj,    sun_misc_Unsafe,    putObjectVolatile,putObjectVolatile_obj)   \
  template(_putBooleanVolatile_obj,   sun_misc_Unsafe,    putBooleanVolatile,putBooleanVolatile_obj) \
  template(_putByteVolatile_obj,      sun_misc_Unsafe,    putByteVolatile,putByteVolatile_obj)       \
  template(_putShortVolatile_obj,     sun_misc_Unsafe,    putShortVolatile,putShortVolatile_obj)     \
  template(_putCharVolatile_obj,      sun_misc_Unsafe,    putCharVolatile,putCharVolatile_obj)       \
  template(_putIntVolatile_obj,       sun_misc_Unsafe,    putIntVolatile,putIntVolatile_obj)         \
  template(_putLongVolatile_obj,      sun_misc_Unsafe,    putLongVolatile,putLongVolatile_obj)       \
  template(_putFloatVolatile_obj,     sun_misc_Unsafe,    putFloatVolatile,putFloatVolatile_obj)     \
  template(_putDoubleVolatile_obj,    sun_misc_Unsafe,    putDoubleVolatile,putDoubleVolatile_obj)   \
  template(_getByte_raw,      sun_misc_Unsafe,    getByte,getByte_raw)                            \
  template(_getShort_raw,     sun_misc_Unsafe,    getShort,getShort_raw)                          \
  template(_getChar_raw,      sun_misc_Unsafe,    getChar,getChar_raw)                            \
  template(_getInt_raw,       sun_misc_Unsafe,    getInt,getInt_raw)                              \
  template(_getLong_raw,      sun_misc_Unsafe,    getLong,getLong_raw)                            \
  template(_getFloat_raw,     sun_misc_Unsafe,    getFloat,getFloat_raw)                          \
  template(_getDouble_raw,    sun_misc_Unsafe,    getDouble,getDouble_raw)                        \
  template(_getAddress_raw,   sun_misc_Unsafe,    getAddress,getAddress_raw)                      \
  template(_putByte_raw,      sun_misc_Unsafe,    putByte,putByte_raw)                            \
  template(_putShort_raw,     sun_misc_Unsafe,    putShort,putShort_raw)                          \
  template(_putChar_raw,      sun_misc_Unsafe,    putChar,putChar_raw)                            \
  template(_putInt_raw,       sun_misc_Unsafe,    putInt,putInt_raw)                              \
  template(_putLong_raw,      sun_misc_Unsafe,    putLong,putLong_raw)                            \
  template(_putFloat_raw,     sun_misc_Unsafe,    putFloat,putFloat_raw)                          \
  template(_putDouble_raw,    sun_misc_Unsafe,    putDouble,putDouble_raw)                        \
  template(_putAddress_raw,   sun_misc_Unsafe,    putAddress,putAddress_raw)                      \
  template(_allocateInstance, sun_misc_Unsafe,    allocateInstance,allocateInstance)              \
  template(_compareAndSwapObject_obj,  sun_misc_Unsafe,    compareAndSwapObject, compareAndSwapObject_obj)   \
  template(_compareAndSwapLong_obj,    sun_misc_Unsafe,    compareAndSwapLong, compareAndSwapLong_obj)       \
  template(_compareAndSwapInt_obj,     sun_misc_Unsafe,    compareAndSwapInt, compareAndSwapInt_obj)         \
  template(_storeStoreBarrier,  sun_misc_Unsafe,    storeStoreBarrier, storeStoreBarrier)         \
  template(_park,            sun_misc_Unsafe,     park, park)                                     \
  template(_unpark,          sun_misc_Unsafe,     unpark, unpark)                                 

    /*end*/

#define VM_INTRINSIC_ENUM(id, klass, name, sig)  id,

// VM Intrinsic ID's uniquely identify some very special methods
class vmIntrinsics: AllStatic {
  friend class vmSymbols;
  friend class ciObjectFactory;

 public:
  // Accessing
  enum ID {
    _none = 0,                      // not an intrinsic (default answer)
    VM_INTRINSICS_DO(VM_INTRINSIC_ENUM)
    _vm_intrinsics_terminating_enum
  };

public:
  static ID ID_from(int raw_id) {
    assert(raw_id >= (int)_none && raw_id < (int)_vm_intrinsics_terminating_enum,
	   "must be a valid intrinsic ID");
    return (ID)raw_id;
  }
};

#undef VM_INTRINSIC_ENUM
