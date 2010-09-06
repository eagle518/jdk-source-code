#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)universe.hpp	1.158 04/06/15 12:17:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Universe is a name space holding known system classes and objects in the VM.
// 
// Loaded classes are accessible through the SystemDictionary.
// 
// The object heap is allocated and accessed through Universe, and various allocation
// support is provided. Allocation by the interpreter and compiled code is done inline
// and bails out to Scavenge::invoke_and_allocate.

class CollectedHeap;
class DeferredObjAllocEvent;

class Universe: AllStatic {
  friend class MarkSweep;
  friend class oopDesc;
  friend class ClassLoader;
  friend class Arguments;
  friend class SystemDictionary;
  friend class VMStructs;
  friend class CompactingPermGenGen;
  friend class VM_PopulateDumpSharedSpace;

  friend jint  universe_init();
  friend void  universe2_init();
  friend void  universe_post_init();

 public:
  enum JVMPIState {
    _jvmpi_disabled = 0,
    _jvmpi_enabled = 1,
    _jvmpi_disabling = 2
  };

 private:
  // Known classes in the VM
  static klassOop _boolArrayKlassObj;
  static klassOop _byteArrayKlassObj;
  static klassOop _charArrayKlassObj;
  static klassOop _intArrayKlassObj;
  static klassOop _shortArrayKlassObj;
  static klassOop _longArrayKlassObj;
  static klassOop _singleArrayKlassObj;
  static klassOop _doubleArrayKlassObj;
  static klassOop _typeArrayKlassObjs[T_VOID+1];

  static klassOop _symbolKlassObj;
  static klassOop _methodKlassObj;
  static klassOop _constMethodKlassObj;
#ifndef CORE
  static klassOop _methodDataKlassObj;
#endif  // !CORE
  static klassOop _klassKlassObj;
  static klassOop _arrayKlassKlassObj;
  static klassOop _objArrayKlassKlassObj;
  static klassOop _typeArrayKlassKlassObj;
  static klassOop _instanceKlassKlassObj;
  static klassOop _constantPoolKlassObj;
  static klassOop _constantPoolCacheKlassObj;
#ifndef CORE
  static klassOop _compiledICHolderKlassObj;
#endif
  static klassOop _systemObjArrayKlassObj;

  // Known objects in the VM
  static oop          _main_thread_group;             // Reference to the main thread group object
  static oop          _system_thread_group;           // Reference to the system thread group object

  static typeArrayOop _the_empty_byte_array;          // Canonicalized byte array
  static typeArrayOop _the_empty_short_array;         // Canonicalized short array
  static typeArrayOop _the_empty_int_array;           // Canonicalized int array
  static objArrayOop  _the_empty_system_obj_array;    // Canonicalized system obj array
  static objArrayOop  _the_empty_class_klass_array;   // Canonicalized obj array of type java.lang.Class
  static objArrayOop  _the_array_interfaces_array;    // Canonicalized 2-array of cloneable & serializable klasses
  static methodOop    _finalizer_register_method;     // static method for registering finalizable objects
  static methodOop    _reflect_invoke_method;         // method for security checks
  static methodOop    _loader_addClass_method;        // method for registering loaded classes in class loader vector
  static oop          _out_of_memory_error_java_heap; // preallocated error object
  static oop          _out_of_memory_error_perm_gen;  // preallocated error object
  static oop          _out_of_memory_error_array_size;// preallocated error object

  static oop          _null_ptr_exception_instance;   // preallocated exception object
  static oop          _arithmetic_exception_instance; // preallocated exception object
  // The object used as an exception dummy when exceptions are thrown for
  // the vm thread.
  static oop          _vm_exception;

  static oop          _emptySymbol;                   // Canonical empty string ("") symbol

  // The particular choice of collected heap.
  static CollectedHeap* _collectedHeap;

  // array of dummy objects used with +FullGCAlot
  debug_only(static objArrayOop _fullgc_alot_dummy_array;)
 // index of next entry to clear
  debug_only(static int         _fullgc_alot_dummy_next;) 

  // Compiler/dispatch support
  static int  _base_vtable_size;                      // Java vtbl size of klass Object (in words)

  // Version checking
  static bool _is_jdk12x_version;
  static bool _is_jdk13x_version;
  static bool _is_jdk14x_version;
  static bool _is_jdk15x_version;

  // Initialization
  static bool _bootstrapping;                         // true during genesis
  static bool _fully_initialized;                     // true after universe_init and initialize_vtables called

  // JVMPI support (cached values for performance)
  static JVMPIState _jvmpi_alloc_event_enabled;
  static bool       _jvmpi_move_event_enabled;
  static bool       _jvmpi_jni_global_alloc_event_enabled;
  static bool       _jvmpi_jni_global_free_event_enabled;
  static bool       _jvmpi_jni_weak_global_alloc_event_enabled;
  static bool       _jvmpi_jni_weak_global_free_event_enabled;

  // Historic gc information
  static size_t _heap_capacity_at_last_gc;
  static size_t _heap_used_at_last_gc;

  static jint initialize_heap();
  static void fixup_mirrors(TRAPS);

  static void reinitialize_vtable_of(Klass* k, TRAPS);
  static void reinitialize_itables();
  static void compute_base_vtable_size();             // compute vtable size of class Object

  static void genesis(TRAPS);                         // Create the initial world

  // Debugging
  static int _verify_count;                           // number of verifies done

  static void compute_verify_oop_data();

 public:
  // Known classes in the VM
  static klassOop boolArrayKlassObj()                 { return _boolArrayKlassObj;   }
  static klassOop byteArrayKlassObj()                 { return _byteArrayKlassObj;   }
  static klassOop charArrayKlassObj()                 { return _charArrayKlassObj;   }
  static klassOop intArrayKlassObj()                  { return _intArrayKlassObj;    }
  static klassOop shortArrayKlassObj()                { return _shortArrayKlassObj;  }
  static klassOop longArrayKlassObj()                 { return _longArrayKlassObj;   }
  static klassOop singleArrayKlassObj()               { return _singleArrayKlassObj; }
  static klassOop doubleArrayKlassObj()               { return _doubleArrayKlassObj; }

  static klassOop typeArrayKlassObj(BasicType t) {
    assert((uint)t < T_VOID+1, "range check");
    assert(_typeArrayKlassObjs[t] != NULL, "domain check");
    return _typeArrayKlassObjs[t];
  }

  static klassOop symbolKlassObj()                    { return _symbolKlassObj;            }
  static klassOop methodKlassObj()                    { return _methodKlassObj;            }
  static klassOop constMethodKlassObj()               { return _constMethodKlassObj;         }
#ifndef CORE
  static klassOop methodDataKlassObj()                { return _methodDataKlassObj;        }
#endif // !CORE
  static klassOop klassKlassObj()                     { return _klassKlassObj;             }
  static klassOop arrayKlassKlassObj()                { return _arrayKlassKlassObj;        }
  static klassOop objArrayKlassKlassObj()             { return _objArrayKlassKlassObj;     }
  static klassOop typeArrayKlassKlassObj()            { return _typeArrayKlassKlassObj;    }
  static klassOop instanceKlassKlassObj()             { return _instanceKlassKlassObj;     }
  static klassOop constantPoolKlassObj()              { return _constantPoolKlassObj;      }
  static klassOop constantPoolCacheKlassObj()         { return _constantPoolCacheKlassObj; }
#ifndef CORE
  static klassOop compiledICHolderKlassObj()          { return _compiledICHolderKlassObj;  }
#endif
  static klassOop systemObjArrayKlassObj()            { return _systemObjArrayKlassObj;    }

  // Known objects in tbe VM
  static oop      main_thread_group()                 { return _main_thread_group; }
  static void set_main_thread_group(oop group)        { _main_thread_group = group;}

  static oop      system_thread_group()               { return _system_thread_group; }
  static void set_system_thread_group(oop group)      { _system_thread_group = group;}

  static typeArrayOop the_empty_byte_array()          { return _the_empty_byte_array;          }
  static typeArrayOop the_empty_short_array()         { return _the_empty_short_array;         }
  static typeArrayOop the_empty_int_array()           { return _the_empty_int_array;           }
  static objArrayOop  the_empty_system_obj_array ()   { return _the_empty_system_obj_array;    }  
  static objArrayOop  the_empty_class_klass_array ()  { return _the_empty_class_klass_array;   }  
  static objArrayOop  the_array_interfaces_array()    { return _the_array_interfaces_array;    }  
  static methodOop    finalizer_register_method()     { return _finalizer_register_method;     }  
  static methodOop    reflect_invoke_method()         { return _reflect_invoke_method;         }
  static methodOop    loader_addClass_method()        { return _loader_addClass_method;        }
  static oop          out_of_memory_error_java_heap() { return _out_of_memory_error_java_heap;  }
  static oop          out_of_memory_error_perm_gen()  { return _out_of_memory_error_perm_gen;   }
  static oop          out_of_memory_error_array_size(){ return _out_of_memory_error_array_size; }
  static oop          null_ptr_exception_instance()   { return _null_ptr_exception_instance;   }
  static oop          arithmetic_exception_instance() { return _arithmetic_exception_instance; }
  static oop          vm_exception()                  { return _vm_exception; }
  static oop          emptySymbol()                   { return _emptySymbol; }

  // Accessors needed for fast allocation
  static klassOop* boolArrayKlassObj_addr()           { return &_boolArrayKlassObj;   }
  static klassOop* byteArrayKlassObj_addr()           { return &_byteArrayKlassObj;   }
  static klassOop* charArrayKlassObj_addr()           { return &_charArrayKlassObj;   }
  static klassOop* intArrayKlassObj_addr()            { return &_intArrayKlassObj;    }
  static klassOop* shortArrayKlassObj_addr()          { return &_shortArrayKlassObj;  }
  static klassOop* longArrayKlassObj_addr()           { return &_longArrayKlassObj;   }
  static klassOop* singleArrayKlassObj_addr()         { return &_singleArrayKlassObj; }
  static klassOop* doubleArrayKlassObj_addr()         { return &_doubleArrayKlassObj; }

  // The particular choice of collected heap.
  static CollectedHeap* heap() { return _collectedHeap; }

  // JVMPI support
  static bool jvmpi_slow_allocation()                       { return (_jvmpi_alloc_event_enabled != _jvmpi_disabled); }
  static bool jvmpi_alloc_event_enabled()                   { return (_jvmpi_alloc_event_enabled == _jvmpi_enabled); }
  static void set_jvmpi_alloc_event_enabled(JVMPIState b)   { _jvmpi_alloc_event_enabled = b;   }

  static bool jvmpi_move_event_enabled()                    { return _jvmpi_move_event_enabled; }
  static void set_jvmpi_move_event_enabled(bool b)          { _jvmpi_move_event_enabled = b;    }

  static bool jvmpi_jni_global_alloc_event_enabled()                 { return _jvmpi_jni_global_alloc_event_enabled; }
  static void set_jvmpi_jni_global_alloc_event_enabled(bool b)       { _jvmpi_jni_global_alloc_event_enabled = b;    }
  static bool jvmpi_jni_global_free_event_enabled()                  { return _jvmpi_jni_global_free_event_enabled; }
  static void set_jvmpi_jni_global_free_event_enabled(bool b)        { _jvmpi_jni_global_free_event_enabled = b;    }

  static bool jvmpi_jni_weak_global_alloc_event_enabled()            { return _jvmpi_jni_weak_global_alloc_event_enabled; }
  static void set_jvmpi_jni_weak_global_alloc_event_enabled(bool b)  { _jvmpi_jni_weak_global_alloc_event_enabled = b;    }
  static bool jvmpi_jni_weak_global_free_event_enabled()             { return _jvmpi_jni_weak_global_free_event_enabled; }
  static void set_jvmpi_jni_weak_global_free_event_enabled(bool b)   { _jvmpi_jni_weak_global_free_event_enabled = b;    }

  // Historic gc information
  static size_t get_heap_capacity_at_last_gc()         { return _heap_capacity_at_last_gc; }
  static size_t get_heap_free_at_last_gc()             { return _heap_capacity_at_last_gc - _heap_used_at_last_gc; }
  static size_t get_heap_used_at_last_gc()             { return _heap_used_at_last_gc; }
  static void update_heap_info_at_gc();

  // Testers
  static bool is_bootstrapping()                      { return _bootstrapping; }
  static bool is_fully_initialized()                  { return _fully_initialized; }
  static bool is_jdk_version_initialized()            { return _is_jdk12x_version || _is_jdk13x_version || _is_jdk14x_version || _is_jdk15x_version; }
  static bool is_jdk12x_version()                     { return _is_jdk12x_version; }
  static bool is_jdk13x_version()                     { return _is_jdk13x_version; }
  static bool is_jdk14x_version()                     { return _is_jdk14x_version; }
  static bool is_jdk15x_version()                     { return _is_jdk15x_version; }
  // Keep the semantics of this that the version number is >= 1.4
  static bool is_gte_jdk14x_version()                 { assert(is_jdk_version_initialized(), "Not initialized"); return _is_jdk14x_version || _is_jdk15x_version; }
  // Keep the semantics of this that the version number is >= 1.5
  static bool is_gte_jdk15x_version()                 { assert(is_jdk_version_initialized(), "Not initialized"); return _is_jdk15x_version; }

  static inline bool element_type_should_be_aligned(BasicType type);
  static inline bool field_type_should_be_aligned(BasicType type);
  static bool        on_page_boundary(void* addr);
  static bool        is_out_of_memory_error(Handle throwable);
  static void check_alignment(uintx size, uintx alignment, const char* name);

  // Finalizer support.
  static void run_finalizers_on_exit();

  // Iteration

  // Apply "f" to the addresses of all the direct heap pointers maintained
  // as static fields of "Universe".
  static void oops_do(OopClosure* f, bool do_all = false);

  // Apply "f" to all klasses for basic types (classes not present in
  // SystemDictionary).
  static void basic_type_classes_do(void f(klassOop));
  
  // Apply "f" to all system klasses (classes not present in SystemDictionary).
  static void system_classes_do(void f(klassOop));

  // For sharing -- fill in a list of known vtable pointers.
  static void init_self_patching_vtbl_list(void** list, int count);

  // Debugging
  static void verify(bool allow_dirty = true, bool silent = false) PRODUCT_RETURN;
  static int  verify_count()                  { return _verify_count; }
  static void print();
  static void print_on(outputStream* st) ;
  static void print_heap_usage_header()       PRODUCT_RETURN;
  static void print_heap_usage_stamp(size_t prev_mem_used, int level) PRODUCT_RETURN;
  // Change the number of dummy objects kept reachable by the full gc dummy
  // array; this should trigger relocation in a sliding compaction collector.
  debug_only(static bool release_fullgc_alot_dummy();)
  // The non-oop pattern (see compiledIC.hpp, etc)
  static void*   non_oop_word();

  // Detect if either the high or low bits of x are a non-oop pattern
  // is_non_oop is true of the non_oop_word, and of any half-and-half
  // combination of a real oop and the non_oop_word that may arise
  // due to race conditions between patching of oop-bearing instructions,
  // and processors reading those instructions.  Thus, the non_oop_word
  // is a good initial value for an oop in code, if it is eventually
  // to be patched to a real oop.  Then, is_non_oop can detect both
  // the initial state and any transient intermediate states.
  static bool is_non_oop(oop x);

  // Oop verification (see MacroAssembler::verify_oop)
  static uintptr_t verify_oop_mask()          PRODUCT_RETURN0;
  static uintptr_t verify_oop_bits()          PRODUCT_RETURN0;
  static uintptr_t verify_mark_bits()         PRODUCT_RETURN0;
  static uintptr_t verify_mark_mask()         PRODUCT_RETURN0;
  static uintptr_t verify_klass_mask()        PRODUCT_RETURN0;
  static uintptr_t verify_klass_bits()        PRODUCT_RETURN0;

#ifndef CORE
  // Flushing and deoptimization
  static void flush_dependents_on(instanceKlassHandle dependee);
#ifdef HOTSWAP
  // Flushing and deoptimization in case of evolution
  static void flush_evol_dependents_on(instanceKlassHandle dependee);
#endif HOTSWAP
  // Support for fullspeed debugging
  static void flush_dependents_on_method(methodHandle dependee);
#endif

  // Compiler support
  static int base_vtable_size()               { return _base_vtable_size; }

  // jvmpi support
  static void jvmpi_object_alloc(oop obj, size_t bytesize);
  static void jvmpi_object_move(oop from, oop to);

  // deferred JVM/PI OBJECT_ALLOC event support:
  static void jvmpi_post_deferred_obj_alloc_events(
    GrowableArray<DeferredObjAllocEvent *>* deferred_list);
};

class DeferredObjAllocEvent : public CHeapObj {
  private:
    oop    _oop;
    size_t _bytesize;
    jint   _arena_id;

  public:
    DeferredObjAllocEvent(const oop o, const size_t s, const jint id) {
      _oop      = o;
      _bytesize = s;
      _arena_id = id;
    }

    ~DeferredObjAllocEvent() {
    }

    jint   arena_id() { return _arena_id; }
    size_t bytesize() { return _bytesize; }
    oop    get_oop()  { return _oop; }
};
