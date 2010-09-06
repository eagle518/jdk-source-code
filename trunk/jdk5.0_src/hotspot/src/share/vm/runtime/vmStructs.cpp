#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vmStructs.cpp	1.130 04/07/05 14:55:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vmStructs.cpp.incl"

// Note: the cross-product of (c1, c2, product, nonproduct, ...),
// (nonstatic, static), and (unchecked, checked) has not been taken.
// Only the macros currently needed have been defined.

// A field whose type is not checked is given a null string as the
// type name, indicating an "opaque" type to the serviceability agent.

// NOTE: there is an interdependency between this file and
// HotSpotCTypeDataBase.java, which parses the type strings.


//--------------------------------------------------------------------------------
// VM_STRUCTS
//
// This list enumerates all of the fields the serviceability agent
// needs to know about. Be sure to see also the type table below this one.
// NOTE that there are platform-specific additions to this table in
// vmStructs_<os>_<cpu>.hpp.

#define VM_STRUCTS(nonstatic_field, \
                   static_field, \
                   unchecked_nonstatic_field, \
                   volatile_nonstatic_field, \
                   nonproduct_nonstatic_field, \
                   nonproduct_noncore_nonstatic_field, \
                   c1_nonstatic_field, \
                   c2_nonstatic_field, \
                   noncore_nonstatic_field, \
                   volatile_noncore_nonstatic_field, \
                   noncore_static_field, \
                   unchecked_c1_static_field, \
                   unchecked_c2_static_field, \
                   last_entry) \
                                                                                                                                     \
  /******************************************************************/                                                               \
  /* OopDesc and Klass hierarchies (NOTE: methodDataOop incomplete) */                                                               \
  /******************************************************************/                                                               \
                                                                                                                                     \
  volatile_nonstatic_field(oopDesc,            _mark,                                         markOop)                               \
  nonstatic_field(oopDesc,                     _klass,                                        klassOop)                              \
     static_field(oopDesc,                     _bs,					      BarrierSet*)                           \
  nonstatic_field(arrayKlass,                  _dimension,                                    int)                                   \
  nonstatic_field(arrayKlass,                  _higher_dimension,                             klassOop)                              \
  nonstatic_field(arrayKlass,                  _lower_dimension,                              klassOop)                              \
  nonstatic_field(arrayKlass,                  _vtable_len,                                   int)                                   \
  nonstatic_field(arrayKlass,                  _alloc_size,                                   juint)                                 \
  nonstatic_field(arrayKlass,                  _array_header_in_bytes,                        int)                                   \
  nonstatic_field(arrayOopDesc,                _length,                                       int)                                   \
  noncore_nonstatic_field(compiledICHolderKlass,  _alloc_size,                                juint)                                 \
  noncore_nonstatic_field(compiledICHolderOopDesc,_holder_method,                             methodOop)                             \
  noncore_nonstatic_field(compiledICHolderOopDesc,_holder_klass,                              klassOop)                              \
  nonstatic_field(constantPoolOopDesc,         _tags,                                         typeArrayOop)                          \
  nonstatic_field(constantPoolOopDesc,         _cache,                                        constantPoolCacheOop)                  \
  nonstatic_field(constantPoolOopDesc,         _pool_holder,                                  klassOop)                              \
  nonstatic_field(constantPoolCacheOopDesc,    _constant_pool,                                constantPoolOop)                       \
  nonstatic_field(instanceKlass,               _array_klasses,                                klassOop)                              \
  nonstatic_field(instanceKlass,               _methods,                                      objArrayOop)                           \
  nonstatic_field(instanceKlass,               _method_ordering,                              typeArrayOop)                          \
  nonstatic_field(instanceKlass,               _local_interfaces,                             objArrayOop)                           \
  nonstatic_field(instanceKlass,               _transitive_interfaces,                        objArrayOop)                           \
  nonstatic_field(instanceKlass,               _nof_implementors,                             int)                                   \
  nonstatic_field(instanceKlass,               _implementor,                                  klassOop)                              \
  nonstatic_field(instanceKlass,               _fields,                                       typeArrayOop)                          \
  nonstatic_field(instanceKlass,               _constants,                                    constantPoolOop)                       \
  nonstatic_field(instanceKlass,               _class_loader,                                 oop)                                   \
  nonstatic_field(instanceKlass,               _protection_domain,                            oop)                                   \
  nonstatic_field(instanceKlass,               _signers,                                      objArrayOop)                           \
  nonstatic_field(instanceKlass,               _source_file_name,                             symbolOop)                             \
  nonstatic_field(instanceKlass,               _source_debug_extension,                       symbolOop)                             \
  nonstatic_field(instanceKlass,               _inner_classes,                                typeArrayOop)                          \
  nonstatic_field(instanceKlass,               _nonstatic_field_size,                         int)                                   \
  nonstatic_field(instanceKlass,               _static_field_size,                            int)                                   \
  nonstatic_field(instanceKlass,               _static_oop_field_size,                        int)                                   \
  nonstatic_field(instanceKlass,               _nonstatic_oop_map_size,                       int)                                   \
  nonstatic_field(instanceKlass,               _is_marked_dependent,                          bool)                                  \
  nonstatic_field(instanceKlass,               _init_state,                                   instanceKlass::ClassState)             \
  nonstatic_field(instanceKlass,               _init_thread,                                  Thread*)                               \
  nonstatic_field(instanceKlass,               _vtable_len,                                   int)                                   \
  nonstatic_field(instanceKlass,               _itable_len,                                   int)                                   \
  nonstatic_field(instanceKlass,               _reference_type,                               ReferenceType)                         \
  volatile_nonstatic_field(instanceKlass,      _oop_map_cache,                                OopMapCache*)                          \
  nonstatic_field(instanceKlass,               _jni_ids,                                      JNIid*)                                \
  nonstatic_field(instanceKlass,               _jni_id_map,                                   jniIdMapBase*)                         \
  nonstatic_field(instanceKlass,               _osr_nmethods_head,                            nmethod*)                              \
  nonstatic_field(instanceKlass,               _breakpoints,                                  BreakpointInfo*)                       \
  nonstatic_field(instanceKlass,               _previous_version,                             klassOop)                              \
  nonstatic_field(instanceKlass,               _generic_signature,                            symbolOop)                             \
  nonstatic_field(instanceKlass,               _class_annotations,                            typeArrayOop)                          \
  nonstatic_field(instanceKlass,               _fields_annotations,                            objArrayOop)                           \
  nonstatic_field(instanceKlass,               _methods_annotations,                           objArrayOop)                           \
  nonstatic_field(instanceKlass,               _methods_parameter_annotations,                 objArrayOop)                           \
  nonstatic_field(instanceKlass,               _methods_default_annotations,                   objArrayOop)                           \
  nonstatic_field(Klass,                       _super_check_offset,                           juint)                                 \
  nonstatic_field(Klass,                       _secondary_super_cache,                        klassOop)                              \
  nonstatic_field(Klass,                       _secondary_supers,                             objArrayOop)                           \
  nonstatic_field(Klass,                       _primary_supers[0],                            klassOop)                              \
  nonstatic_field(Klass,                       _java_mirror,                                  oop)                                   \
  nonstatic_field(Klass,                       _modifier_flags,                               jint)                                  \
  nonstatic_field(Klass,                       _super,                                        klassOop)                              \
  nonstatic_field(Klass,                       _size_helper,                                  int)                                   \
  nonstatic_field(Klass,                       _name,                                         symbolOop)                             \
  nonstatic_field(Klass,                       _access_flags,                                 AccessFlags)                           \
  nonstatic_field(Klass,                       _subklass,                                     klassOop)                              \
  nonstatic_field(Klass,                       _next_sibling,                                 klassOop)                              \
  nonproduct_nonstatic_field(Klass,            _verify_count,                                 int)                                   \
  nonstatic_field(Klass,                       _alloc_count,                                  juint)                                 \
  nonstatic_field(Klass,                       _is_objArray,                                  bool)                                  \
  nonstatic_field(klassKlass,                  _alloc_size,                                   juint)                                 \
  nonstatic_field(methodKlass,                 _alloc_size,                                   juint)                                 \
  noncore_nonstatic_field(methodDataOopDesc,   _size,                                         int)                                   \
  noncore_nonstatic_field(methodDataOopDesc,   _method,                                       methodOop)                             \
  nonstatic_field(methodOopDesc,               _constMethod,                                  constMethodOop)                        \
  nonstatic_field(methodOopDesc,               _constants,                                    constantPoolOop)                       \
  c2_nonstatic_field(methodOopDesc,            _method_data,                                  methodDataOop)                         \
  c2_nonstatic_field(methodOopDesc,            _interpreter_invocation_count,                 int)                                   \
  nonstatic_field(methodOopDesc,               _access_flags,                                 AccessFlags)                           \
  nonstatic_field(methodOopDesc,               _vtable_index,                                 int)                                   \
  nonstatic_field(methodOopDesc,               _method_size,                                  u2)                                    \
  nonstatic_field(methodOopDesc,               _max_stack,                                    u2)                                    \
  nonstatic_field(methodOopDesc,               _max_locals,                                   u2)                                    \
  nonstatic_field(methodOopDesc,               _size_of_parameters,                           u2)                                    \
  c2_nonstatic_field(methodOopDesc,            _interpreter_throwout_count,                   u2)                                    \
  nonstatic_field(methodOopDesc,               _parameter_info,                               u2)                                    \
  nonstatic_field(methodOopDesc,               _number_of_breakpoints,                        u2)                                    \
  noncore_nonstatic_field(methodOopDesc,       _invocation_counter,                           InvocationCounter)                     \
  noncore_nonstatic_field(methodOopDesc,       _backedge_counter,                             InvocationCounter)                     \
  nonproduct_noncore_nonstatic_field(methodOopDesc, _compiled_invocation_count,               int)                                   \
  volatile_nonstatic_field(methodOopDesc,      _code,                                         nmethod*)                              \
  nonstatic_field(methodOopDesc,               _interpreter_entry,                            address)                               \
  volatile_noncore_nonstatic_field(methodOopDesc, _from_compiled_code_entry_point,            address)                               \
  volatile_nonstatic_field(constMethodOopDesc,  _fingerprint,                                  uint64_t)                              \
  nonstatic_field(constMethodOopDesc,          _method,                                       methodOop)                             \
  nonstatic_field(constMethodOopDesc,          _exception_table,                              typeArrayOop)                          \
  nonstatic_field(constMethodOopDesc,          _constMethod_size,                             int)                                   \
  nonstatic_field(constMethodOopDesc,          _interpreter_kind,                             jbyte)                                 \
  nonstatic_field(constMethodOopDesc,          _flags,                                        jbyte)                                 \
  nonstatic_field(constMethodOopDesc,          _code_size,                                    u2)                                    \
  nonstatic_field(constMethodOopDesc,          _name_index,                                   u2)                                    \
  nonstatic_field(constMethodOopDesc,          _signature_index,                              u2)                                    \
  nonstatic_field(constMethodOopDesc,          _method_index,                                 u2)                                    \
  nonstatic_field(constMethodOopDesc,          _generic_signature_index,                      u2)                                    \
  nonstatic_field(constMethodOopDesc,          _stackmap_u1,                                  typeArrayOop)                          \
  nonstatic_field(constMethodOopDesc,          _stackmap_u2,                                  typeArrayOop)                          \
  nonstatic_field(objArrayKlass,               _element_klass,                                klassOop)                              \
  nonstatic_field(objArrayKlass,               _bottom_klass,                                 klassOop)                              \
  nonstatic_field(symbolKlass,                 _alloc_size,                                   juint)                                 \
  nonstatic_field(symbolOopDesc,               _length,                                       unsigned short)                        \
  unchecked_nonstatic_field(symbolOopDesc,     _body,                                         sizeof(jbyte)) /* NOTE: no type */     \
  nonstatic_field(typeArrayKlass,              _type,                                         int)                                   \
  nonstatic_field(typeArrayKlass,              _scale,                                        int)                                   \
  nonstatic_field(typeArrayKlass,              _max_length,                                   int)                                   \
                                                                                                                                     \
  /***********************/                                                                                                          \
  /* Constant Pool Cache */                                                                                                          \
  /***********************/                                                                                                          \
                                                                                                                                     \
  volatile_nonstatic_field(ConstantPoolCacheEntry,      _indices,                                      intx)                         \
  volatile_nonstatic_field(ConstantPoolCacheEntry,      _f1,                                           oop)                          \
  volatile_nonstatic_field(ConstantPoolCacheEntry,      _f2,                                           intx)                         \
  volatile_nonstatic_field(ConstantPoolCacheEntry,      _flags,                                        intx)                         \
                                                                                                                                     \
  /********************************/                                                                                                 \
  /* MethodOop-related structures */                                                                                                 \
  /********************************/                                                                                                 \
                                                                                                                                     \
  nonstatic_field(CheckedExceptionElement,     class_cp_index,                                u2)                                    \
  nonstatic_field(LocalVariableTableElement,   start_bci,                                     u2)                                    \
  nonstatic_field(LocalVariableTableElement,   length,                                        u2)                                    \
  nonstatic_field(LocalVariableTableElement,   name_cp_index,                                 u2)                                    \
  nonstatic_field(LocalVariableTableElement,   descriptor_cp_index,                           u2)                                    \
  nonstatic_field(LocalVariableTableElement,   signature_cp_index,                            u2)                                    \
  nonstatic_field(LocalVariableTableElement,   slot,                                          u2)                                    \
  nonstatic_field(BreakpointInfo,              _orig_bytecode,                                Bytecodes::Code)                       \
  nonstatic_field(BreakpointInfo,              _bci,                                          int)                                   \
  nonstatic_field(BreakpointInfo,              _name_index,                                   u2)                                    \
  nonstatic_field(BreakpointInfo,              _signature_index,                              u2)                                    \
  nonstatic_field(BreakpointInfo,              _next,                                         BreakpointInfo*)                       \
  /***********/                                                                                                                      \
  /* JNI IDs */                                                                                                                      \
  /***********/                                                                                                                      \
                                                                                                                                     \
  nonstatic_field(JNIid,                       _holder,                                       klassOop)                              \
  nonstatic_field(JNIid,                       _next,                                         JNIid*)                                \
  nonstatic_field(JNIid,                       _offset,                                       int)                                   \
  /************/                                                                                                                     \
  /* Universe */                                                                                                                     \
  /************/                                                                                                                     \
                                                                                                                                     \
     static_field(Universe,                    _boolArrayKlassObj,                            klassOop)                              \
     static_field(Universe,                    _byteArrayKlassObj,                            klassOop)                              \
     static_field(Universe,                    _charArrayKlassObj,                            klassOop)                              \
     static_field(Universe,                    _intArrayKlassObj,                             klassOop)                              \
     static_field(Universe,                    _shortArrayKlassObj,                           klassOop)                              \
     static_field(Universe,                    _longArrayKlassObj,                            klassOop)                              \
     static_field(Universe,                    _singleArrayKlassObj,                          klassOop)                              \
     static_field(Universe,                    _doubleArrayKlassObj,                          klassOop)                              \
     static_field(Universe,                    _symbolKlassObj,                               klassOop)                              \
     static_field(Universe,                    _methodKlassObj,                               klassOop)                              \
     static_field(Universe,                    _constMethodKlassObj,                          klassOop)                              \
  noncore_static_field(Universe,               _methodDataKlassObj,                           klassOop)                              \
     static_field(Universe,                    _klassKlassObj,                                klassOop)                              \
     static_field(Universe,                    _arrayKlassKlassObj,                           klassOop)                              \
     static_field(Universe,                    _objArrayKlassKlassObj,                        klassOop)                              \
     static_field(Universe,                    _typeArrayKlassKlassObj,                       klassOop)                              \
     static_field(Universe,                    _instanceKlassKlassObj,                        klassOop)                              \
     static_field(Universe,                    _constantPoolKlassObj,                         klassOop)                              \
     static_field(Universe,                    _constantPoolCacheKlassObj,                    klassOop)                              \
  noncore_static_field(Universe,               _compiledICHolderKlassObj,                     klassOop)                              \
     static_field(Universe,                    _systemObjArrayKlassObj,                       klassOop)                              \
     static_field(Universe,                    _main_thread_group,                            oop)                                   \
     static_field(Universe,                    _system_thread_group,                          oop)                                   \
     static_field(Universe,                    _the_empty_byte_array,                         typeArrayOop)                          \
     static_field(Universe,                    _the_empty_short_array,                        typeArrayOop)                          \
     static_field(Universe,                    _the_empty_int_array,                          typeArrayOop)                          \
     static_field(Universe,                    _the_empty_system_obj_array,                   objArrayOop)                           \
     static_field(Universe,                    _the_empty_class_klass_array,                  objArrayOop)                           \
     static_field(Universe,                    _finalizer_register_method,                    methodOop)                             \
     static_field(Universe,                    _reflect_invoke_method,                        methodOop)                             \
     static_field(Universe,                    _loader_addClass_method,                       methodOop)                             \
     static_field(Universe,                    _out_of_memory_error_java_heap,                oop)                                   \
     static_field(Universe,                    _out_of_memory_error_perm_gen,                 oop)                                   \
     static_field(Universe,                    _out_of_memory_error_array_size,               oop)                                   \
     static_field(Universe,                    _null_ptr_exception_instance,                  oop)                                   \
     static_field(Universe,                    _arithmetic_exception_instance,                oop)                                   \
     static_field(Universe,                    _vm_exception,                                 oop)                                   \
     static_field(Universe,                    _collectedHeap,                                CollectedHeap*)                        \
     static_field(Universe,                    _base_vtable_size,                             int)                                   \
     static_field(Universe,                    _is_jdk12x_version,                            bool)                                  \
     static_field(Universe,                    _is_jdk13x_version,                            bool)                                  \
     static_field(Universe,                    _bootstrapping,                                bool)                                  \
     static_field(Universe,                    _fully_initialized,                            bool)                                  \
     static_field(Universe,                    _jvmpi_alloc_event_enabled,                    Universe::JVMPIState)                  \
     static_field(Universe,                    _jvmpi_move_event_enabled,                     bool)                                  \
     static_field(Universe,                    _jvmpi_jni_global_alloc_event_enabled,         bool)                                  \
     static_field(Universe,                    _jvmpi_jni_global_free_event_enabled,          bool)                                  \
     static_field(Universe,                    _jvmpi_jni_weak_global_alloc_event_enabled,    bool)                                  \
     static_field(Universe,                    _jvmpi_jni_weak_global_free_event_enabled,     bool)                                  \
     static_field(Universe,                    _verify_count,                                 int)                                   \
                                                                                                                                     \
  /**********************************************************************************/                                               \
  /* Generation and Space hierarchies                                               */                                               \
  /**********************************************************************************/                                               \
                                                                                                                                     \
  unchecked_nonstatic_field(ageTable,          sizes,                                         sizeof(ageTable::sizes))               \
                                                                                                                                     \
  nonstatic_field(BarrierSet,                  _max_covered_regions,                          int)                                   \
                                                                                                                                     \
  nonstatic_field(BlockOffsetTable,            _bottom,                                       HeapWord*)                             \
  nonstatic_field(BlockOffsetTable,            _end,                                          HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(BlockOffsetSharedArray,      _reserved,                                     MemRegion)                             \
  nonstatic_field(BlockOffsetSharedArray,      _end,                                          HeapWord*)                             \
  nonstatic_field(BlockOffsetSharedArray,      _vs,                                           VirtualSpace)                          \
  nonstatic_field(BlockOffsetSharedArray,      _offset_array,                                 u_char*)                               \
                                                                                                                                     \
  nonstatic_field(BlockOffsetArray,            _array,                                        BlockOffsetSharedArray*)               \
  nonstatic_field(BlockOffsetArray,            _sp,                                           Space*)                                \
  nonstatic_field(BlockOffsetArrayContigSpace, _next_offset_threshold,                        HeapWord*)                             \
  nonstatic_field(BlockOffsetArrayContigSpace, _next_offset_index,                            size_t)                                \
                                                                                                                                     \
  nonstatic_field(BlockOffsetArrayNonContigSpace, _unallocated_block,                         HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(CarRememberedSet,            _tg,                                           TrainGeneration*)                      \
  nonstatic_field(CarRememberedSet,            _ct,                                           CardTableRS*)                          \
  nonstatic_field(CarRememberedSet,            _size,                                         unsigned)                              \
  nonstatic_field(CarRememberedSet,            _capacity_mask,                                unsigned)                              \
  nonstatic_field(CarRememberedSet,            _set,                                          jbyte**)                               \
  nonstatic_field(CarRememberedSet,            _being_processed,                              bool)                                  \
                                                                                                                                     \
  nonstatic_field(CarSpace,                    _next_car,                                     CarSpace*)                             \
  nonstatic_field(CarSpace,                    _remembered_set,                               CarRememberedSet*)                     \
  nonstatic_field(CarSpace,                    _blocks,                                       size_t)                                \
  nonstatic_field(CarSpace,                    _desc,                                         CarTableDesc*)                         \
  nonstatic_field(CarSpace,                    _marked,                                       bool)                                  \
  nonstatic_field(CarSpace,                    _oversized,                                    bool)                                  \
     static_field(CarSpace,                    _car_size,                                     size_t)                                \
     static_field(CarSpace,                    _car_size_in_words,                            size_t)                                \
     static_field(CarSpace,                    _oversized_car_size_in_words,                  size_t)                                \
                                                                                                                                     \
  nonstatic_field(CarTable,                    _LogOfCarSpaceSize,                            size_t)                                \
  nonstatic_field(CarTable,                    _table,                                        CarTableDesc*)                         \
  nonstatic_field(CarTable,                    _table_base,                                   CarTableDesc*)                         \
  nonstatic_field(CarTable,                    _table_size,                                   size_t)                                \
                                                                                                                                     \
  nonstatic_field(CarTableDesc,                _train_number,                                 julong)                                \
  nonstatic_field(CarTableDesc,                _car_number,                                   juint)                                 \
  nonstatic_field(CarTableDesc,                _train,                                        Train*)                                \
  nonstatic_field(CarTableDesc,                _space,                                        CarSpace*)                             \
  nonstatic_field(CarTableDesc,                _target,                                       bool)                                  \
                                                                                                                                     \
  nonstatic_field(CardGeneration,              _rs,                                           GenRemSet*)                            \
  nonstatic_field(CardGeneration,              _bts,                                          BlockOffsetSharedArray*)               \
                                                                                                                                     \
  nonstatic_field(CardTableModRefBS,           _whole_heap,                                   MemRegion)                             \
  nonstatic_field(CardTableModRefBS,           _byte_map,                                     jbyte*)                                \
  nonstatic_field(CardTableModRefBS,           _byte_map_size,                                size_t)                                \
  nonstatic_field(CardTableModRefBS,           _last_valid_index,                             size_t)                                \
  nonstatic_field(CardTableModRefBS,           _guard_index,                                  size_t)                                \
  nonstatic_field(CardTableModRefBS,           _cur_covered_regions,                          int)                                   \
  nonstatic_field(CardTableModRefBS,           _covered,                                      MemRegion*)                            \
  nonstatic_field(CardTableModRefBS,           _committed,                                    MemRegion*)                            \
  nonstatic_field(CardTableModRefBS,           _guard_region,                                 MemRegion)                             \
  nonstatic_field(CardTableModRefBS,           byte_map_base,                                 jbyte*)                                \
                                                                                                                                     \
  nonstatic_field(CardTableRS,                 _ct_bs,	                                      CardTableModRefBS)                     \
                                                                                                                                     \
  nonstatic_field(CollectedHeap,               _reserved,                                     MemRegion)                             \
  nonstatic_field(SharedHeap,                  _perm_gen,                                     PermGen*)                              \
  nonstatic_field(CollectedHeap,               _barrier_set,                                  BarrierSet*)                           \
  nonstatic_field(CollectedHeap,               _is_gc_active,                                 bool)                                  \
  nonstatic_field(CollectedHeap,               _max_heap_capacity,                            size_t)                                \
                                                                                                                                     \
  nonstatic_field(CompactibleSpace,            _compaction_top,                               HeapWord*)                             \
  nonstatic_field(CompactibleSpace,            _first_dead,                                   HeapWord*)                             \
  nonstatic_field(CompactibleSpace,            _end_of_live,                                  HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(FreeChunk,                   _next,                                         FreeChunk*)                            \
  nonstatic_field(FreeChunk,                   _prev,                                         FreeChunk*)                            \
  nonstatic_field(FreeChunk,                   _size,                                         size_t)                                \
                                                                                                                                     \
  nonstatic_field(CompactibleFreeListSpace,    _bt,                                           BlockOffsetArrayNonContigSpace)        \
                                                                                                                                     \
  nonstatic_field(CompactingPermGen,           _gen,                                          OneContigSpaceCardGeneration*)         \
                                                                                                                                     \
  nonstatic_field(CMSPermGen,                  _gen,                                          ConcurrentMarkSweepGeneration*)        \
                                                                                                                                     \
  nonstatic_field(ContiguousSpace,             _top,                                          HeapWord*)                             \
  nonstatic_field(ContiguousSpace,             _saved_mark_word,                              HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(DefNewGeneration,            _next_gen,                                     Generation*)                           \
  nonstatic_field(DefNewGeneration,            _tenuring_threshold,                           int)                                   \
  nonstatic_field(DefNewGeneration,            _age_table,                                    ageTable)                              \
  nonstatic_field(DefNewGeneration,            _eden_space,                                   EdenSpace*)                            \
  nonstatic_field(DefNewGeneration,            _from_space,                                   ContiguousSpace*)                      \
  nonstatic_field(DefNewGeneration,            _to_space,                                     ContiguousSpace*)                      \
                                                                                                                                     \
  nonstatic_field(EdenSpace,                   _gen,                                          DefNewGeneration*)                     \
                                                                                                                                     \
  nonstatic_field(Generation,                  _reserved,                                     MemRegion)                             \
  nonstatic_field(Generation,                  _virtual_space,                                VirtualSpace)                          \
  nonstatic_field(Generation,                  _level,                                        int)                                   \
  nonstatic_field(Generation,                  _stat_record,                                  Generation::StatRecord)                \
                                                                                                                                     \
  nonstatic_field(Generation::StatRecord,      invocations,                                   int)                                   \
  nonstatic_field(Generation::StatRecord,      accumulated_time,                              elapsedTimer)                          \
                                                                                                                                     \
  nonstatic_field(GenerationSpec,              _name,                                         Generation::Name)                      \
  nonstatic_field(GenerationSpec,              _init_size,                                    size_t)                                \
  nonstatic_field(GenerationSpec,              _max_size,                                     size_t)                                \
                                                                                                                                     \
     static_field(GenCollectedHeap,            _gch,                                          GenCollectedHeap*)                     \
  nonstatic_field(GenCollectedHeap,            _n_gens,                                       int)                                   \
  unchecked_nonstatic_field(GenCollectedHeap,  _gens,                                         sizeof(GenCollectedHeap::_gens)) /* NOTE: no type */ \
  nonstatic_field(GenCollectedHeap,            _gen_specs,                                    GenerationSpec**)                      \
                                                                                                                                     \
  nonstatic_field(HeapWord,                    i,                                             char*)                                 \
                                                                                                                                     \
  nonstatic_field(MemRegion,                   _start,                                        HeapWord*)                             \
  nonstatic_field(MemRegion,                   _word_size,                                    size_t)                                \
                                                                                                                                     \
  nonstatic_field(OffsetTableContigSpace,      _offsets,                                      BlockOffsetArray)                      \
                                                                                                                                     \
  nonstatic_field(OneContigSpaceCardGeneration, _min_heap_delta_bytes,                        size_t)                                \
  nonstatic_field(OneContigSpaceCardGeneration, _the_space,                                   ContiguousSpace*)                      \
  nonstatic_field(OneContigSpaceCardGeneration, _last_gc,                                     WaterMark)                             \
                                                                                                                                     \
  nonstatic_field(CompactingPermGenGen,        _ro_vs,                                        VirtualSpace)                          \
  nonstatic_field(CompactingPermGenGen,        _rw_vs,                                        VirtualSpace)                          \
  nonstatic_field(CompactingPermGenGen,        _md_vs,                                        VirtualSpace)                          \
  nonstatic_field(CompactingPermGenGen,        _mc_vs,                                        VirtualSpace)                          \
     static_field(CompactingPermGenGen,        unshared_bottom,                               HeapWord*)                             \
     static_field(CompactingPermGenGen,        unshared_end,                                  HeapWord*)                             \
     static_field(CompactingPermGenGen,        shared_bottom,                                 HeapWord*)                             \
     static_field(CompactingPermGenGen,        readonly_bottom,                               HeapWord*)                             \
     static_field(CompactingPermGenGen,        readonly_end,                                  HeapWord*)                             \
     static_field(CompactingPermGenGen,        readwrite_bottom,                              HeapWord*)                             \
     static_field(CompactingPermGenGen,        readwrite_end,                                 HeapWord*)                             \
     static_field(CompactingPermGenGen,        miscdata_bottom,                               HeapWord*)                             \
     static_field(CompactingPermGenGen,        miscdata_end,                                  HeapWord*)                             \
     static_field(CompactingPermGenGen,        misccode_bottom,                               HeapWord*)                             \
     static_field(CompactingPermGenGen,        misccode_end,                                  HeapWord*)                             \
     static_field(CompactingPermGenGen,        shared_end,                                    HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(PermGen,                     _capacity_expansion_limit,                     size_t)                                \
                                                                                                                                     \
  nonstatic_field(PermanentGenerationSpec,     _name,                                         PermGen::Name)                         \
  nonstatic_field(PermanentGenerationSpec,     _init_size,                                    size_t)                                \
  nonstatic_field(PermanentGenerationSpec,     _max_size,                                     size_t)                                \
                                                                                                                                     \
  nonstatic_field(Space,                       _bottom,                                       HeapWord*)                             \
  nonstatic_field(Space,                       _end,                                          HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(TenuredGeneration,           _shrink_factor,                                size_t)                                \
  nonstatic_field(TenuredGeneration,           _capacity_at_prologue,                         size_t)                                \
  nonstatic_field(ThreadLocalAllocBuffer,      _start,                                        HeapWord*)                             \
  nonstatic_field(ThreadLocalAllocBuffer,      _top,                                          HeapWord*)                             \
  nonstatic_field(ThreadLocalAllocBuffer,      _end,                                          HeapWord*)                             \
  nonstatic_field(ThreadLocalAllocBuffer,      _size,                                         size_t)                                \
  nonstatic_field(ThreadLocalAllocBuffer,      _refill_waste_limit,                           size_t)                                \
     static_field(ThreadLocalAllocBuffer,      _target_refills,                               unsigned)                              \
                                                                                                                                     \
  nonstatic_field(TrainGeneration,             _first_train,                                  Train*)                                \
  nonstatic_field(TrainGeneration,             _last_train,                                   Train*)                                \
  nonstatic_field(TrainGeneration,             _special_nonoop_train,                         Train*)                                \
  nonstatic_field(TrainGeneration,             _relinked_oversized_car,                       CarSpace*)                             \
  nonstatic_field(TrainGeneration,             _relinked_train,                               Train*)                                \
  nonstatic_field(TrainGeneration,             _car_tab,                                      CarTable)                              \
  nonstatic_field(TrainGeneration,             _first_car_desc,                               CarTableDesc*)                         \
  nonstatic_field(TrainGeneration,             _car_free_list,                                CarSpace*)                             \
  nonstatic_field(TrainGeneration,             _train_free_list,                              Train*)                                \
  nonstatic_field(TrainGeneration,             _total_promoted,                               julong)                                \
  nonstatic_field(TrainGeneration,             _total_processed,                              julong)                                \
  nonstatic_field(TrainGeneration,             _delta_promoted,                               julong)                                \
  nonstatic_field(TrainGeneration,             _delta_processed,                              julong)                                \
  nonstatic_field(TrainGeneration,             _tick_interval,                                int)                                   \
  nonstatic_field(TrainGeneration,             _next_invoke_count,                            int)                                   \
  nonstatic_field(TrainGeneration,             _delay_adjustment_count,                       int)                                   \
                                                                                                                                     \
  nonstatic_field(ConcurrentMarkSweepGeneration, _cmsSpace,                                   CompactibleFreeListSpace*)             \
                                                                                                                                     \
  nonstatic_field(Train,                       _tg,                                           TrainGeneration*)                      \
  nonstatic_field(Train,                       _first_car,                                    CarSpace*)                             \
  nonstatic_field(Train,                       _last_car,                                     CarSpace*)                             \
  nonstatic_field(Train,                       _next_train,                                   Train*)                                \
  nonstatic_field(Train,                       _prev_train,                                   Train*)                                \
  nonstatic_field(Train,                       _saved_top_mark,                               WaterMark)                             \
                                                                                                                                     \
  nonstatic_field(VirtualSpace,                _low_boundary,                                 char*)                                 \
  nonstatic_field(VirtualSpace,                _high_boundary,                                char*)                                 \
  nonstatic_field(VirtualSpace,                _low,                                          char*)                                 \
  nonstatic_field(VirtualSpace,                _high,                                         char*)                                 \
  nonstatic_field(VirtualSpace,                _lower_high,                                   char*)                                 \
  nonstatic_field(VirtualSpace,                _middle_high,                                  char*)                                 \
  nonstatic_field(VirtualSpace,                _upper_high,                                   char*)                                 \
  nonstatic_field(WaterMark,                   _point,                                        HeapWord*)                             \
  nonstatic_field(WaterMark,                   _space,                                        Space*)                                \
                                                                                                                                     \
  /**********************/                                                                                                           \
  /* Parallel GC fields */                                                                                                           \
  /**********************/                                                                                                           \
                                                                                                                                     \
  nonstatic_field(PSVirtualSpace,              _alignment,                                    const size_t)                          \
  nonstatic_field(PSVirtualSpace,              _reserved_low_addr,                            char*)                                 \
  nonstatic_field(PSVirtualSpace,              _reserved_high_addr,                           char*)                                 \
  nonstatic_field(PSVirtualSpace,              _committed_low_addr,                           char*)                                 \
  nonstatic_field(PSVirtualSpace,              _committed_high_addr,                          char*)                                 \
                                                                                                                                     \
  nonstatic_field(ImmutableSpace,              _bottom,                                       HeapWord*)                             \
  nonstatic_field(ImmutableSpace,              _end,                                          HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(MutableSpace,                _top,                                          HeapWord*)                             \
                                                                                                                                     \
  nonstatic_field(PSYoungGen,                  _reserved,                                     MemRegion)                             \
  nonstatic_field(PSYoungGen,                  _virtual_space,                                PSVirtualSpace*)                       \
  nonstatic_field(PSYoungGen,                  _eden_space,                                   MutableSpace*)                         \
  nonstatic_field(PSYoungGen,                  _from_space,                                   MutableSpace*)                         \
  nonstatic_field(PSYoungGen,                  _to_space,                                     MutableSpace*)                         \
  nonstatic_field(PSYoungGen,                  _init_gen_size,                                const size_t)                          \
  nonstatic_field(PSYoungGen,                  _min_gen_size,                                 const size_t)                          \
  nonstatic_field(PSYoungGen,                  _max_gen_size,                                 const size_t)                          \
                                                                                                                                     \
  nonstatic_field(PSOldGen,                    _reserved,                                     MemRegion)                             \
  nonstatic_field(PSOldGen,                    _virtual_space,                                PSVirtualSpace*)                       \
  nonstatic_field(PSOldGen,                    _object_space,                                 MutableSpace*)                         \
  nonstatic_field(PSOldGen,                    _init_gen_size,                                const size_t)                          \
  nonstatic_field(PSOldGen,                    _min_gen_size,                                 const size_t)                          \
  nonstatic_field(PSOldGen,                    _max_gen_size,                                 const size_t)                          \
                                                                                                                                     \
  nonstatic_field(PSPermGen,                   _last_used,                                    size_t)                                \
                                                                                                                                     \
     static_field(ParallelScavengeHeap,        _young_gen,                                    PSYoungGen*)                           \
     static_field(ParallelScavengeHeap,        _old_gen,                                      PSOldGen*)                             \
     static_field(ParallelScavengeHeap,        _perm_gen,                                     PSPermGen*)                            \
     static_field(ParallelScavengeHeap,        _psh,                                          ParallelScavengeHeap*)                 \
                                                                                                                                     \
  /************************/                                                                                                         \
  /* PerfMemory - jvmstat */                                                                                                         \
  /************************/                                                                                                         \
                                                                                                                                     \
  nonstatic_field(PerfDataPrologue,            magic,                                         jint)                                  \
  nonstatic_field(PerfDataPrologue,            byte_order,                                    jbyte)                                 \
  nonstatic_field(PerfDataPrologue,            major_version,                                 jbyte)                                 \
  nonstatic_field(PerfDataPrologue,            minor_version,                                 jbyte)                                 \
  nonstatic_field(PerfDataPrologue,            accessible,                                    jbyte)                                 \
  nonstatic_field(PerfDataPrologue,            used,                                          jint)                                  \
  nonstatic_field(PerfDataPrologue,            overflow,                                      jint)                                  \
  nonstatic_field(PerfDataPrologue,            mod_time_stamp,                                jlong)                                 \
  nonstatic_field(PerfDataPrologue,            entry_offset,                                  jint)                                  \
  nonstatic_field(PerfDataPrologue,            num_entries,                                   jint)                                  \
                                                                                                                                     \
  nonstatic_field(PerfDataEntry,               entry_length,                                  jint)                                  \
  nonstatic_field(PerfDataEntry,               name_offset,                                   jint)                                  \
  nonstatic_field(PerfDataEntry,               vector_length,                                 jint)                                  \
  nonstatic_field(PerfDataEntry,               data_type,                                     jbyte)                                 \
  nonstatic_field(PerfDataEntry,               flags,                                         jbyte)                                 \
  nonstatic_field(PerfDataEntry,               data_units,                                    jbyte)                                 \
  nonstatic_field(PerfDataEntry,               data_variability,                              jbyte)                                 \
  nonstatic_field(PerfDataEntry,               data_offset,                                   jint)                                  \
                                                                                                                                     \
     static_field(PerfMemory,                  _start,                                        char*)                                 \
     static_field(PerfMemory,                  _end,                                          char*)                                 \
     static_field(PerfMemory,                  _top,                                          char*)                                 \
     static_field(PerfMemory,                  _capacity,                                     size_t)                                \
     static_field(PerfMemory,                  _prologue,                                     PerfDataPrologue*)                     \
     static_field(PerfMemory,                  _initialized,                                  jint)                                  \
                                                                                                                                     \
  /***************/                                                                                                                  \
  /* SymbolTable */                                                                                                                  \
  /***************/                                                                                                                  \
                                                                                                                                     \
     static_field(SymbolTable,                  _the_table,                                   SymbolTable*)                          \
                                                                                                                                     \
  /********************/                                                                                                             \
  /* SystemDictionary */                                                                                                             \
  /********************/                                                                                                             \
                                                                                                                                     \
      static_field(SystemDictionary,            _dictionary,                                   Dictionary*)                          \
      static_field(SystemDictionary,            _placeholders,                                 PlaceholderTable*)                    \
      static_field(SystemDictionary,            _shared_dictionary,                            Dictionary*)                          \
      static_field(SystemDictionary,            _system_loader_lock_obj,                       oop)                                  \
      static_field(SystemDictionary,            _loader_constraints,                           LoaderConstraintTable*)               \
      static_field(SystemDictionary,            _java_system_loader,                           oop)                                  \
                                                                                                                                     \
  /*******************/                                                                                                              \
  /* HashtableBucket */                                                                                                              \
  /*******************/                                                                                                              \
                                                                                                                                     \
  nonstatic_field(HashtableBucket,             _entry,                                        BasicHashtableEntry*)                  \
                                                                                                                                     \
  /******************/                                                                                                               \
  /* HashtableEntry */                                                                                                               \
  /******************/                                                                                                               \
                                                                                                                                     \
  nonstatic_field(BasicHashtableEntry,         _next,                                         BasicHashtableEntry*)                  \
  nonstatic_field(BasicHashtableEntry,         _hash,                                         unsigned int)                          \
  nonstatic_field(HashtableEntry,              _literal,                                      oop)                                   \
                                                                                                                                     \
  /*************/                                                                                                                    \
  /* Hashtable */                                                                                                                    \
  /*************/                                                                                                                    \
                                                                                                                                     \
  nonstatic_field(BasicHashtable,              _table_size,                                   int)                                   \
  nonstatic_field(BasicHashtable,              _buckets,                                      HashtableBucket*)                      \
  nonstatic_field(BasicHashtable,              _free_list,                                    BasicHashtableEntry*)                  \
  nonstatic_field(BasicHashtable,              _first_free_entry,                             char*)                                 \
  nonstatic_field(BasicHashtable,              _end_block,                                    char*)                                 \
  nonstatic_field(BasicHashtable,              _entry_size,                                   int)                                   \
                                                                                                                                     \
  /*******************/                                                                                                              \
  /* DictionaryEntry */                                                                                                              \
  /*******************/                                                                                                              \
                                                                                                                                     \
  nonstatic_field(DictionaryEntry,             _loader,                                       oop)                                   \
  nonstatic_field(DictionaryEntry,             _pd_set,                                       ProtectionDomainEntry*)                \
                                                                                                                                     \
  /********************/                                                                                                             \
                                                                                                                                     \
  nonstatic_field(PlaceholderEntry,            _loader,                                       oop)                                   \
                                                                                                                                     \
  /**************************/                                                                                                       \
  /* ProctectionDomainEntry */                                                                                                       \
  /**************************/                                                                                                       \
                                                                                                                                     \
  nonstatic_field(ProtectionDomainEntry,       _next,                                         ProtectionDomainEntry*)                \
  nonstatic_field(ProtectionDomainEntry,       _protection_domain,                            oop)                                   \
                                                                                                                                     \
  /*************************/                                                                                                        \
  /* LoaderConstraintEntry */                                                                                                        \
  /*************************/                                                                                                        \
                                                                                                                                     \
  nonstatic_field(LoaderConstraintEntry,       _name,                                         symbolOop)                             \
  nonstatic_field(LoaderConstraintEntry,       _num_loaders,                                  int)                                   \
  nonstatic_field(LoaderConstraintEntry,       _max_loaders,                                  int)                                   \
  nonstatic_field(LoaderConstraintEntry,       _loaders,                                      oop*)                                  \
                                                                                                                                     \
  /********************************/                                                                                                 \
  /* CodeCache (NOTE: incomplete) */                                                                                                 \
  /********************************/                                                                                                 \
                                                                                                                                     \
     static_field(CodeCache,                   _heap,                                         CodeHeap*)                             \
                                                                                                                                     \
  /*******************************/                                                                                                  \
  /* CodeHeap (NOTE: incomplete) */                                                                                                  \
  /*******************************/                                                                                                  \
                                                                                                                                     \
  nonstatic_field(CodeHeap,                    _memory,                                       VirtualSpace)                          \
  nonstatic_field(CodeHeap,                    _segmap,                                       VirtualSpace)                          \
  nonstatic_field(CodeHeap,                    _log2_segment_size,                            int)                                   \
  nonstatic_field(HeapBlock,                   _header,                                       HeapBlock::Header)                     \
  nonstatic_field(HeapBlock::Header,           _length,                                       size_t)                                \
  nonstatic_field(HeapBlock::Header,           _used,                                         bool)                                  \
                                                                                                                                     \
  /**********************************/                                                                                               \
  /* Interpreter (NOTE: incomplete) */                                                                                               \
  /**********************************/                                                                                               \
                                                                                                                                     \
     static_field(AbstractInterpreter,         _code,                                         StubQueue*)                            \
                                                                                                                                     \
  /****************************/                                                                                                     \
  /* Stubs (NOTE: incomplete) */                                                                                                     \
  /****************************/                                                                                                     \
                                                                                                                                     \
  nonstatic_field(StubQueue,                   _stub_buffer,                                  address)                               \
  nonstatic_field(StubQueue,                   _buffer_limit,                                 int)                                   \
  nonstatic_field(StubQueue,                   _queue_begin,                                  int)                                   \
  nonstatic_field(StubQueue,                   _queue_end,                                    int)                                   \
  nonstatic_field(StubQueue,                   _number_of_stubs,                              int)                                   \
  nonstatic_field(InterpreterCodelet,          _size,                                         int)                                   \
  nonstatic_field(InterpreterCodelet,          _description,                                  const char*)                           \
  nonstatic_field(InterpreterCodelet,          _is_safepoint_safe,                            bool)                                  \
  nonstatic_field(InterpreterCodelet,          _bytecode,                                     Bytecodes::Code)                       \
                                                                                                                                     \
  /***********************************/                                                                                              \
  /* StubRoutines (NOTE: incomplete) */                                                                                              \
  /***********************************/                                                                                              \
                                                                                                                                     \
     static_field(StubRoutines,                _call_stub_return_address,                     address)                               \
                                                                                                                                     \
  /***************************************/                                                                                          \
  /* PcDesc and other compiled code info */                                                                                          \
  /***************************************/                                                                                          \
                                                                                                                                     \
  noncore_nonstatic_field(PcDesc,              _pc_offset,                                    int)                                   \
  noncore_nonstatic_field(PcDesc,              _scope_decode_offset,                          int)                                   \
                                                                                                                                     \
  /***************************************************/                                                                              \
  /* CodeBlobs (NOTE: incomplete, but only a little) */                                                                              \
  /***************************************************/                                                                              \
                                                                                                                                     \
  nonstatic_field(CodeBlob,                    _name,                                         const char*)                           \
  nonstatic_field(CodeBlob,                    _size,                                         int)                                   \
  nonstatic_field(CodeBlob,                    _header_size,                                  int)                                   \
  nonstatic_field(CodeBlob,                    _relocation_size,                              int)                                   \
  nonstatic_field(CodeBlob,                    _instructions_offset,                          int)                                   \
  nonstatic_field(CodeBlob,                    _data_offset,                                  int)                                   \
  nonstatic_field(CodeBlob,                    _oops_offset,                                  int)                                   \
  nonstatic_field(CodeBlob,                    _oops_length,                                  int)                                   \
  nonstatic_field(CodeBlob,                    _frame_size,                                   int)                                   \
  c2_nonstatic_field(CodeBlob,                 _link_offset,                                  int)                                   \
  nonstatic_field(CodeBlob,                    _oop_maps,                                     OopMapSet*)                            \
                                                                                                                                     \
  /**************************************************/                                                                               \
  /* NMethods (NOTE: incomplete, but only a little) */                                                                               \
  /**************************************************/                                                                               \
                                                                                                                                     \
     noncore_static_field(nmethod,             _zombie_instruction_size,                      int)                                   \
  noncore_nonstatic_field(nmethod,             _method,                                       methodOop)                             \
  noncore_nonstatic_field(nmethod,             _entry_bci,                                    int)                                   \
  noncore_nonstatic_field(nmethod,             _link,                                         nmethod*)                              \
  noncore_nonstatic_field(nmethod,             _exception_offset,                             int)                                   \
  noncore_nonstatic_field(nmethod,             _stub_offset,                                  int)                                   \
  noncore_nonstatic_field(nmethod,             _scopes_data_offset,                           int)                                   \
  noncore_nonstatic_field(nmethod,             _scopes_pcs_offset,                            int)                                   \
  noncore_nonstatic_field(nmethod,             _handler_table_offset,                         int)                                   \
  noncore_nonstatic_field(nmethod,             _nul_chk_table_offset,                         int)                                   \
  noncore_nonstatic_field(nmethod,             _nmethod_end_offset,                           int)                                   \
  noncore_nonstatic_field(nmethod,             _first_dependent,                              int)                                   \
  noncore_nonstatic_field(nmethod,             _number_of_dependents,                         int)                                   \
  noncore_nonstatic_field(nmethod,             _entry_point,                                  address)                               \
  noncore_nonstatic_field(nmethod,             _verified_entry_point,                         address)                               \
  noncore_nonstatic_field(nmethod,             _interpreter_entry_point,                      address)                               \
  noncore_nonstatic_field(nmethod,             _osr_entry_point,                              address)                               \
  noncore_nonstatic_field(nmethod,             _frame_start_offset,                           int)                                   \
  noncore_nonstatic_field(nmethod,             _lock_count,                                   jint)                                  \
  noncore_nonstatic_field(nmethod,             _stack_traversal_mark,                         long)                                  \
                                                                                                                                     \
  /********************************/                                                                                                 \
  /* JavaCalls (NOTE: incomplete) */                                                                                                 \
  /********************************/                                                                                                 \
                                                                                                                                     \
  nonstatic_field(JavaCallWrapper,             _anchor,                                       JavaFrameAnchor)                       \
  /********************************/                                                                                                 \
  /* JavaFrameAnchor (NOTE: incomplete) */                                                                                           \
  /********************************/                                                                                                 \
  volatile_nonstatic_field(JavaFrameAnchor,    _last_Java_sp,                                 intptr_t*)                             \
  volatile_nonstatic_field(JavaFrameAnchor,    _last_Java_pc,                                 address)                               \
  volatile_nonstatic_field(JavaFrameAnchor,    _not_at_call_id,                               intptr_t*)                             \
  volatile_nonstatic_field(JavaFrameAnchor,    _flags,                                        int)                                   \
                                                                                                                                     \
  /******************************/                                                                                                   \
  /* Threads (NOTE: incomplete) */                                                                                                   \
  /******************************/                                                                                                   \
                                                                                                                                     \
     static_field(Threads,                     _thread_list,                                  JavaThread*)                           \
     static_field(Threads,                     _number_of_threads,                            int)                                   \
     static_field(Threads,                     _number_of_non_daemon_threads,                 int)                                   \
     static_field(Threads,                     _return_code,                                  int)                                   \
                                                                                                                                     \
   volatile_nonstatic_field(Thread,            _suspend_flags,                                uint32_t)                              \
  nonstatic_field(Thread,                      _active_handles,                               JNIHandleBlock*)                       \
  nonstatic_field(Thread,                      _highest_lock,                                 address)                               \
  nonstatic_field(Thread,                      _tlab,                                         ThreadLocalAllocBuffer)                \
  nonstatic_field(Thread,                      _current_pending_monitor,                      ObjectMonitor*)                        \
  nonstatic_field(Thread,                      _current_pending_monitor_is_from_java,         bool)                                  \
  nonstatic_field(Thread,                      _current_waiting_monitor,                      ObjectMonitor*)                        \
  nonstatic_field(NamedThread,                 _name,                                         char*)                                 \
  nonstatic_field(JavaThread,                  _next,                                         JavaThread*)                           \
  nonstatic_field(JavaThread,                  _threadObj,                                    oop)                                   \
  nonstatic_field(JavaThread,                  _anchor,                                       JavaFrameAnchor)                       \
   volatile_nonstatic_field(JavaThread,        _thread_state,                                 JavaThreadState)                       \
  nonstatic_field(JavaThread,                  _osthread,                                     OSThread*)                             \
  nonstatic_field(JavaThread,                  _stack_base,                                   address)                               \
  nonstatic_field(JavaThread,                  _stack_size,                                   size_t)                                \
     static_field(ConcurrentMarkSweepThread,   _first_thread,                                 ConcurrentMarkSweepThread*)            \
     static_field(ConcurrentMarkSweepThread,   _last_thread,                                  ConcurrentMarkSweepThread*)            \
     static_field(ConcurrentMarkSweepThread,   _slt,                                          SurrogateLockerThread*)                \
  nonstatic_field(ConcurrentMarkSweepThread,   _next,                                         ConcurrentMarkSweepThread*)            \
                                                                                                                                     \
  /************/                                                                                                                     \
  /* OSThread */                                                                                                                     \
  /************/                                                                                                                     \
                                                                                                                                     \
  nonstatic_field(OSThread,                    _interrupted,                                  jint)                                  \
                                                                                                                                     \
  /************************/                                                                                                         \
  /* OopMap and OopMapSet */                                                                                                         \
  /************************/                                                                                                         \
                                                                                                                                     \
  nonstatic_field(OopMap,                      _pc_offset,                                    int)                                   \
  nonstatic_field(OopMap,                      _at_call,                                      bool)                                  \
  nonstatic_field(OopMap,                      _omv_count,                                    int)                                   \
  nonstatic_field(OopMap,                      _omv_data_size,                                int)                                   \
  nonstatic_field(OopMap,                      _omv_data,                                     unsigned char*)                        \
  nonstatic_field(OopMap,                      _write_stream,                                 CompressedWriteStream*)                \
  nonstatic_field(OopMapSet,                   _om_count,                                     int)                                   \
  nonstatic_field(OopMapSet,                   _om_size,                                      int)                                   \
  nonstatic_field(OopMapSet,                   _om_data,                                      OopMap**)                              \
                                                                                                                                     \
  /*********************************/                                                                                                \
  /* JNIHandles and JNIHandleBlock */                                                                                                \
  /*********************************/                                                                                                \
     static_field(JNIHandles,                  _global_handles,                               JNIHandleBlock*)                       \
     static_field(JNIHandles,                  _weak_global_handles,                          JNIHandleBlock*)                       \
     static_field(JNIHandles,                  _deleted_handle,                               oop)                                   \
                                                                                                                                     \
  unchecked_nonstatic_field(JNIHandleBlock,    _handles,                                      JNIHandleBlock::block_size_in_oops * sizeof(Oop)) /* Note: no type */ \
  nonstatic_field(JNIHandleBlock,              _top,                                          int)                                   \
  nonstatic_field(JNIHandleBlock,              _next,                                         JNIHandleBlock*)                       \
                                                                                                                                     \
  /********************/                                                                                                             \
  /* CompressedStream */                                                                                                             \
  /********************/                                                                                                             \
                                                                                                                                     \
  nonstatic_field(CompressedStream,            _buffer,                                       u_char*)                               \
  nonstatic_field(CompressedStream,            _position,                                     int)                                   \
                                                                                                                                     \
  /*********************************/                                                                                                \
  /* SharedInfo (NOTE: incomplete) */                                                                                                \
  /*********************************/                                                                                                \
                                                                                                                                     \
     static_field(SharedInfo,                  stack0,                                        OptoReg::Name)                         \
                                                                                                                                     \
  /*******************************/                                                                                                  \
  /* Runtime1 (NOTE: incomplete) */                                                                                                  \
  /*******************************/                                                                                                  \
                                                                                                                                     \
  unchecked_c1_static_field(Runtime1,          _blobs,                                        sizeof(Runtime1::_blobs)) /* NOTE: no type */ \
                                                                                                                                     \
  /************/                                                                                                                     \
  /* Monitors */                                                                                                                     \
  /************/                                                                                                                     \
                                                                                                                                     \
  volatile_nonstatic_field(ObjectMonitor,      _header,                                       markOop)                               \
  unchecked_nonstatic_field(ObjectMonitor,     _object,                                       sizeof(void *)) /* NOTE: no type */    \
  unchecked_nonstatic_field(ObjectMonitor,     _owner,                                        sizeof(void *)) /* NOTE: no type */    \
  unchecked_nonstatic_field(ObjectMonitor,     _queue,                                        sizeof(void *)) /* NOTE: no type */    \
  volatile_nonstatic_field(ObjectMonitor,      _count,                                        intptr_t)                              \
  volatile_nonstatic_field(ObjectMonitor,      _waiters,                                      intptr_t)                              \
  volatile_nonstatic_field(ObjectMonitor,      _recursions,                                   intptr_t)                              \
  volatile_nonstatic_field(BasicLock,          _displaced_header,                             markOop)                               \
  nonstatic_field(BasicObjectLock,             _lock,                                         BasicLock)                             \
  nonstatic_field(BasicObjectLock,             _obj,                                          oop)                                   \
  static_field(ObjectSynchronizer,             gBlockList,                                    ObjectMonitor*)                        \
                                                                                                                                     \
  /*********************/                                                                                                            \
  /* Matcher (C2 only) */                                                                                                            \
  /*********************/                                                                                                            \
                                                                                                                                     \
  unchecked_c2_static_field(Matcher,           _regEncode,                                    sizeof(Matcher::_regEncode)) /* NOTE: no type */ \
                                                                                                                                     \
  /*********************/                                                                                                            \
  /* -XX flags         */                                                                                                            \
  /*********************/                                                                                                            \
                                                                                                                                     \
  nonstatic_field(Flag,                        type,                                          const char*)                           \
  nonstatic_field(Flag,                        name,                                          const char*)                           \
  unchecked_nonstatic_field(Flag,              addr,                                          sizeof(void*)) /* NOTE: no type */     \
  nonstatic_field(Flag,                        kind,                                          const char*)                           \
  static_field(Flag,                           flags,                                         Flag*)                                 \
  static_field(Flag,                           numFlags,                                      size_t)                                \
                                                                                                                                     \
  /*********************/                                                                                                            \
  /* vm version info   */                                                                                                            \
  /*********************/                                                                                                            \
                                                                                                                                     \
  static_field(Abstract_VM_Version,            _s_vm_release,                                 const char*)                           \
  static_field(Abstract_VM_Version,            _s_internal_vm_info_string,                    const char*)                           \
                                                                                                                                     \
  /*************/                                                                                                                    \
  /* Arguments */                                                                                                                    \
  /*************/                                                                                                                    \
                                                                                                                                     \
  static_field(Arguments,                      _jvm_flags,                                    char*)                                 \
  static_field(Arguments,                      _jvm_args,                                     char*)                                 \
  static_field(Arguments,                      _java_command,                                 char*)                                 \
                                                                                                                                     \
  /************************/                                                                                                         \
  /* Miscellaneous fields */                                                                                                         \
  /************************/                                                                                                         \
                                                                                                                                     \
  nonstatic_field(AccessFlags,                 _flags,                                        jint)                                  \
  nonstatic_field(elapsedTimer,                _counter,                                      jlong)                                 \
  nonstatic_field(elapsedTimer,                _active,                                       bool)                                  \
  noncore_nonstatic_field(InvocationCounter,   _counter,                                      unsigned int)

  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_STRUCTS_OS_CPU macro (and must  */
  /* be present there)                                                */

//--------------------------------------------------------------------------------
// VM_TYPES
//
// This list must enumerate at least all of the types in the above
// list. For the types in the above list, the entry below must have
// exactly the same spacing since string comparisons are done in the
// code which verifies the consistency of these tables (in the debug
// build).
//
// In addition to the above types, this list is required to enumerate
// the JNI's java types, which are used to indicate the size of Java
// fields in this VM to the SA. Further, oop types are currently
// distinguished by name (i.e., ends with "oop") over in the SA.
//
// The declare_toplevel_type macro should be used to declare types
// which do not have a superclass.
//
// The declare_integer_type and declare_unsigned_integer_type macros
// are required in order to properly identify C integer types over in
// the SA. They should be used for any type which is otherwise opaque
// and which it is necessary to coerce into an integer value. This
// includes, for example, the type uintptr_t. Note that while they
// will properly identify the type's size regardless of the platform,
// since it is does not seem possible to deduce or check signedness at
// compile time using the pointer comparison tricks, it is currently
// required that the given types have the same signedness across all
// platforms.
//
// NOTE that there are platform-specific additions to this table in
// vmStructs_<os>_<cpu>.hpp.

#define VM_TYPES(declare_type, declare_toplevel_type, declare_oop_type, declare_integer_type, declare_unsigned_integer_type, declare_noncore_type, declare_noncore_toplevel_type, declare_noncore_oop_type, declare_noncore_integer_type, declare_noncore_unsigned_integer_type, declare_c1_toplevel_type, declare_c2_type, declare_c2_toplevel_type, last_entry) \
                                                                          \
  /*************************************************************/         \
  /* Java primitive types -- required by the SA implementation */         \
  /* in order to determine the size of Java fields in this VM  */         \
  /* (the implementation looks up these names specifically)    */         \
  /* NOTE: since we fetch these sizes from the remote VM, we   */         \
  /* have a bootstrapping sequence during which it is not      */         \
  /* valid to fetch Java values from the remote process, only  */         \
  /* C integer values (of known size). NOTE also that we do    */         \
  /* NOT include "Java unsigned" types like juint here; since  */         \
  /* Java does not have unsigned primitive types, those can    */         \
  /* not be mapped directly and are considered to be C integer */         \
  /* types in this system (see the "other types" section,      */         \
  /* below.)                                                   */         \
  /*************************************************************/         \
                                                                          \
  declare_toplevel_type(jboolean)                                         \
  declare_toplevel_type(jbyte)                                            \
  declare_toplevel_type(jchar)                                            \
  declare_toplevel_type(jdouble)                                          \
  declare_toplevel_type(jfloat)                                           \
  declare_toplevel_type(jint)                                             \
  declare_toplevel_type(jlong)                                            \
  declare_toplevel_type(jshort)                                           \
                                                                          \
  /*********************************************************************/ \
  /* C integer types. User-defined typedefs (like "size_t" or          */ \
  /* "intptr_t") are guaranteed to be present with the same names over */ \
  /* in the SA's type database. Names like "unsigned short" are not    */ \
  /* guaranteed to be visible through the SA's type database lookup    */ \
  /* mechanism, though they will have a Type object created for them   */ \
  /* and are valid types for Fields.                                   */ \
  /*********************************************************************/ \
                                                                          \
  /* FIXME: "bool" is #defined in globalDefinitions.hpp to be "jint". */  \
  /* This has required unwanted specialization of some of the SA's code. */ \
  /* declare_integer_type(bool) */                                        \
  declare_integer_type(int)                                               \
  declare_integer_type(long)                                              \
  declare_unsigned_integer_type(unsigned char)                            \
  declare_unsigned_integer_type(unsigned int)                             \
  declare_unsigned_integer_type(unsigned short)                           \
  declare_unsigned_integer_type(unsigned long)                            \
  /* The compiler thinks this is a different type than */                 \
  /* unsigned short on Win32 */                                           \
  declare_unsigned_integer_type(u2)                                       \
  declare_unsigned_integer_type(unsigned)                                 \
                                                                          \
  /*****************************/                                         \
  /* C primitive pointer types */                                         \
  /*****************************/                                         \
                                                                          \
  declare_toplevel_type(int*)                                             \
  declare_toplevel_type(char*)                                            \
  declare_toplevel_type(const char*)                                      \
  declare_toplevel_type(u_char*)                                          \
  declare_toplevel_type(unsigned char*)                                   \
                                                                          \
  /*******************************************************************/   \
  /* Types which it will be handy to have available over in the SA   */   \
  /* in order to do platform-independent address -> integer coercion */   \
  /* (note: these will be looked up by name)                         */   \
  /*******************************************************************/   \
                                                                          \
  declare_unsigned_integer_type(size_t)                                   \
  declare_unsigned_integer_type(const size_t)                             \
  declare_integer_type(intx)                                              \
  declare_integer_type(intptr_t)                                          \
  declare_unsigned_integer_type(uintx)                                    \
  declare_unsigned_integer_type(uintptr_t)                                \
  declare_unsigned_integer_type(uint32_t)                                 \
  declare_unsigned_integer_type(uint64_t)                                 \
                                                                          \
  /*******************************************************************************/ \
  /* OopDesc and Klass hierarchies (NOTE: missing methodDataOop-related classes) */ \
  /*******************************************************************************/ \
                                                                          \
  declare_toplevel_type(oopDesc)                                          \
  declare_toplevel_type(Klass_vtbl)                                       \
           declare_type(Klass, Klass_vtbl)                                \
           declare_type(arrayKlass, Klass)                                \
           declare_type(arrayKlassKlass, klassKlass)                      \
           declare_type(arrayOopDesc, oopDesc)                            \
   declare_noncore_type(compiledICHolderKlass, Klass)                     \
   declare_noncore_type(compiledICHolderOopDesc, oopDesc)                 \
           declare_type(constantPoolKlass, arrayKlass)                    \
           declare_type(constantPoolOopDesc, arrayOopDesc)                \
           declare_type(constantPoolCacheKlass, arrayKlass)               \
           declare_type(constantPoolCacheOopDesc, arrayOopDesc)           \
           declare_type(instanceKlass, Klass)                             \
           declare_type(instanceKlassKlass, klassKlass)                   \
           declare_type(instanceOopDesc, oopDesc)                         \
           declare_type(instanceRefKlass, instanceKlass)                  \
           declare_type(klassKlass, Klass)                                \
           declare_type(klassOopDesc, oopDesc)                            \
           declare_type(markOopDesc, oopDesc)                             \
   declare_noncore_type(methodDataKlass, Klass)                           \
   declare_noncore_type(methodDataOopDesc, oopDesc)                       \
           declare_type(methodKlass, Klass)                               \
           declare_type(constMethodKlass, Klass)                          \
           declare_type(methodOopDesc, oopDesc)                           \
           declare_type(objArrayKlass, arrayKlass)                        \
           declare_type(objArrayKlassKlass, arrayKlassKlass)              \
           declare_type(objArrayOopDesc, arrayOopDesc)                    \
           declare_type(constMethodOopDesc, oopDesc)                      \
           declare_type(symbolKlass, Klass)                               \
           declare_type(symbolOopDesc, oopDesc)                           \
           declare_type(typeArrayKlass, arrayKlass)                       \
           declare_type(typeArrayKlassKlass, arrayKlassKlass)             \
           declare_type(typeArrayOopDesc, arrayOopDesc)                   \
                                                                          \
  /********/                                                              \
  /* Oops */                                                              \
  /********/                                                              \
                                                                          \
  declare_oop_type(constantPoolOop)                                       \
  declare_oop_type(constantPoolCacheOop)                                  \
  declare_oop_type(klassOop)                                              \
  declare_oop_type(markOop)                                               \
  declare_oop_type(methodOop)                                             \
  declare_noncore_oop_type(methodDataOop)                                 \
  declare_oop_type(objArrayOop)                                           \
  declare_oop_type(oop)                                                   \
  declare_oop_type(constMethodOop)                                        \
  declare_oop_type(symbolOop)                                             \
  declare_oop_type(typeArrayOop)                                          \
                                                                          \
  /*************************************/                                 \
  /* MethodOop-related data structures */                                 \
  /*************************************/                                 \
                                                                          \
  declare_toplevel_type(CheckedExceptionElement)                          \
  declare_toplevel_type(LocalVariableTableElement)                        \
                                                                          \
  /******************************************/                            \
  /* Generation and space hierarchies       */                            \
  /* (needed for run-time type information) */                            \
  /******************************************/                            \
                                                                          \
  declare_toplevel_type(CollectedHeap)                                    \
           declare_type(ParallelScavengeHeap,         CollectedHeap)      \
           declare_type(SharedHeap,                   CollectedHeap)      \
           declare_type(GenCollectedHeap,             SharedHeap)         \
  declare_toplevel_type(Generation)                                       \
           declare_type(DefNewGeneration,             Generation)         \
           declare_type(ParNewGeneration,             DefNewGeneration)   \
           declare_type(CardGeneration,               Generation)         \
           declare_type(OneContigSpaceCardGeneration, CardGeneration)     \
           declare_type(TrainGeneration,              CardGeneration)     \
           declare_type(ConcurrentMarkSweepGeneration, CardGeneration)    \
           declare_type(CMSPermGenGen,     ConcurrentMarkSweepGeneration) \
           declare_type(TenuredGeneration,            OneContigSpaceCardGeneration) \
           declare_type(CompactingPermGenGen,         OneContigSpaceCardGeneration) \
  declare_toplevel_type(Space)                                            \
           declare_type(CompactibleSpace,             Space)              \
           declare_type(CompactibleFreeListSpace,     CompactibleSpace)   \
           declare_type(ContiguousSpace,              CompactibleSpace)   \
           declare_type(EdenSpace,                    ContiguousSpace)    \
           declare_type(OffsetTableContigSpace,       ContiguousSpace)    \
           declare_type(TenuredSpace,                 OffsetTableContigSpace) \
           declare_type(ContigPermSpace,              OffsetTableContigSpace) \
           declare_type(CarSpace,                     OffsetTableContigSpace) \
  declare_toplevel_type(FreeChunk)                                        \
  declare_toplevel_type(Train)                                            \
  declare_toplevel_type(PermGen)                                          \
           declare_type(CompactingPermGen,            PermGen)            \
           declare_type(CMSPermGen,                   PermGen)            \
  declare_toplevel_type(BarrierSet)                                       \
           declare_type(ModRefBarrierSet,             BarrierSet)         \
           declare_type(CardTableModRefBS,            ModRefBarrierSet)   \
  declare_toplevel_type(GenRemSet)                                        \
           declare_type(CardTableRS,                  GenRemSet)	  \
  declare_toplevel_type(BlockOffsetSharedArray)                           \
  declare_toplevel_type(BlockOffsetTable)                                 \
           declare_type(BlockOffsetArray,             BlockOffsetTable)   \
           declare_type(BlockOffsetArrayContigSpace,  BlockOffsetArray)   \
           declare_type(BlockOffsetArrayNonContigSpace, BlockOffsetArray) \
                                                                          \
  /* Miscellaneous other GC types */                                      \
                                                                          \
  declare_toplevel_type(ageTable)                                         \
  declare_toplevel_type(CarRememberedSet)                                 \
  declare_toplevel_type(CarTable)                                         \
  declare_toplevel_type(CarTableDesc)                                     \
  declare_toplevel_type(Generation::StatRecord)                           \
  declare_toplevel_type(GenerationSpec)                                   \
  declare_toplevel_type(HeapWord)                                         \
  declare_toplevel_type(MemRegion)                                        \
  declare_toplevel_type(PermanentGenerationSpec)                          \
  declare_toplevel_type(ThreadLocalAllocBuffer)                           \
  declare_toplevel_type(VirtualSpace)                                     \
  declare_toplevel_type(WaterMark)                                        \
                                                                          \
  /* Pointers to Garbage Collection types */                              \
                                                                          \
  declare_toplevel_type(BarrierSet*)                                      \
  declare_toplevel_type(BlockOffsetSharedArray*)                          \
  declare_toplevel_type(CarRememberedSet*)                                \
  declare_toplevel_type(CarSpace*)                                        \
  declare_toplevel_type(GenRemSet*) 		                          \
  declare_toplevel_type(CardTableRS*) 		                          \
  declare_toplevel_type(CarTableDesc*)                                    \
  declare_toplevel_type(CollectedHeap*)                                   \
  declare_toplevel_type(ContiguousSpace*)                                 \
  declare_toplevel_type(CompactibleFreeListSpace*)                        \
  declare_toplevel_type(FreeChunk*)                                       \
  declare_toplevel_type(DefNewGeneration*)                                \
  declare_toplevel_type(EdenSpace*)                                       \
  declare_toplevel_type(GenCollectedHeap*)                                \
  declare_toplevel_type(Generation*)                                      \
  declare_toplevel_type(GenerationSpec**)                                 \
  declare_toplevel_type(HeapWord*)                                        \
  declare_toplevel_type(MemRegion*)                                       \
  declare_toplevel_type(OneContigSpaceCardGeneration*)                    \
  declare_toplevel_type(ConcurrentMarkSweepGeneration*)                   \
  declare_toplevel_type(PermGen*)                                         \
  declare_toplevel_type(Space*)                                           \
  declare_toplevel_type(ThreadLocalAllocBuffer*)                          \
  declare_toplevel_type(Train*)                                           \
  declare_toplevel_type(TrainGeneration*)                                 \
                                                                          \
  /*****************************************/                             \
  /* Parallel GC - space, gen abstractions */                             \
  /*****************************************/                             \
                                                                          \
  declare_toplevel_type(PSVirtualSpace)                                   \
  declare_toplevel_type(ImmutableSpace)                                   \
           declare_type(MutableSpace, ImmutableSpace)                     \
  declare_toplevel_type(PSYoungGen)                                       \
           declare_type(ASPSYoungGen, PSYoungGen)                         \
  declare_toplevel_type(PSOldGen)                                         \
           declare_type(ASPSOldGen, PSOldGen)                             \
           declare_type(PSPermGen, PSOldGen)                              \
                                                                          \
  /*****************************/                                         \
  /* Parallel GC pointer types */                                         \
  /*****************************/                                         \
                                                                          \
  declare_toplevel_type(PSVirtualSpace*)                                  \
  declare_toplevel_type(ImmutableSpace*)                                  \
  declare_toplevel_type(MutableSpace*)                                    \
  declare_toplevel_type(PSYoungGen*)                                      \
  declare_toplevel_type(ASPSYoungGen*)                                    \
  declare_toplevel_type(PSOldGen*)                                        \
  declare_toplevel_type(ASPSOldGen*)                                      \
  declare_toplevel_type(PSPermGen*)                                       \
  declare_toplevel_type(ParallelScavengeHeap*)                            \
                                                                          \
  /************************/                                              \
  /* PerfMemory - jvmstat */                                              \
  /************************/                                              \
                                                                          \
  declare_toplevel_type(PerfDataPrologue)                                 \
  declare_toplevel_type(PerfDataPrologue*)                                \
  declare_toplevel_type(PerfDataEntry)                                    \
  declare_toplevel_type(PerfMemory)                                       \
                                                                          \
  /*********************************/                                     \
  /* SymbolTable, SystemDictionary */                                     \
  /*********************************/                                     \
                                                                          \
  declare_toplevel_type(BasicHashtable)                                   \
    declare_type(Hashtable, BasicHashtable)                               \
    declare_type(SymbolTable, Hashtable)                                  \
    declare_type(LoaderConstraintTable, Hashtable)                        \
    declare_type(TwoOopHashtable, Hashtable)                              \
    declare_type(Dictionary, TwoOopHashtable)                             \
    declare_type(PlaceholderTable, TwoOopHashtable)                       \
  declare_toplevel_type(Hashtable*)                                       \
  declare_toplevel_type(SymbolTable*)                                     \
  declare_toplevel_type(LoaderConstraintTable*)                           \
  declare_toplevel_type(TwoOopHashtable*)                                 \
  declare_toplevel_type(Dictionary*)                                      \
  declare_toplevel_type(PlaceholderTable*)                                \
  declare_toplevel_type(BasicHashtableEntry)                              \
  declare_toplevel_type(BasicHashtableEntry*)                             \
    declare_type(HashtableEntry, BasicHashtableEntry)                     \
    declare_type(DictionaryEntry, HashtableEntry)                         \
    declare_type(PlaceholderEntry, HashtableEntry)                        \
    declare_type(LoaderConstraintEntry, HashtableEntry)                   \
  declare_toplevel_type(HashtableEntry*)                                  \
  declare_toplevel_type(DictionaryEntry*)                                 \
  declare_toplevel_type(HashtableBucket)                                  \
  declare_toplevel_type(HashtableBucket*)                                 \
  declare_toplevel_type(SystemDictionary)                                 \
  declare_toplevel_type(ProtectionDomainEntry)                            \
  declare_toplevel_type(ProtectionDomainEntry*)                           \
                                                                          \
  /***********************************************************/           \
  /* Thread hierarchy (needed for run-time type information) */           \
  /***********************************************************/           \
                                                                          \
  declare_toplevel_type(Threads)                                          \
  declare_toplevel_type(ThreadShadow)                                     \
           declare_type(Thread, ThreadShadow)                             \
           declare_type(NamedThread, Thread)                              \
              declare_type(ConcurrentMarkSweepThread, NamedThread)        \
           declare_type(WatcherThread, Thread)                            \
           declare_type(JavaThread, Thread)                               \
           declare_type(JVMPIDaemonThread, JavaThread)                    \
           declare_type(JvmtiAgentThread, JavaThread)                     \
           declare_type(SurrogateLockerThread, JavaThread)                \
           declare_type(LowMemoryDetectorThread, JavaThread)              \
  declare_noncore_type(CompilerThread, JavaThread)                        \
  declare_toplevel_type(OSThread)                                         \
  declare_toplevel_type(JavaFrameAnchor)                                  \
                                                                          \
  /***************/                                                       \
  /* Interpreter */                                                       \
  /***************/                                                       \
                                                                          \
  declare_toplevel_type(AbstractInterpreter)                              \
                                                                          \
  /*********/                                                             \
  /* Stubs */                                                             \
  /*********/                                                             \
                                                                          \
  declare_toplevel_type(StubQueue)                                        \
  declare_toplevel_type(StubRoutines)                                     \
  declare_toplevel_type(Stub)                                             \
           declare_type(InterpreterCodelet, Stub)                         \
                                                                          \
  /*************/                                                         \
  /* JavaCalls */                                                         \
  /*************/                                                         \
                                                                          \
  declare_toplevel_type(JavaCallWrapper)                                  \
                                                                          \
  /*************/                                                         \
  /* CodeCache */                                                         \
  /*************/                                                         \
                                                                          \
  declare_toplevel_type(CodeCache)                                        \
                                                                          \
  /************/                                                          \
  /* CodeHeap */                                                          \
  /************/                                                          \
                                                                          \
  declare_toplevel_type(CodeHeap)                                         \
  declare_toplevel_type(CodeHeap*)                                        \
  declare_toplevel_type(HeapBlock)                                        \
  declare_toplevel_type(HeapBlock::Header)                                \
           declare_type(FreeBlock, HeapBlock)                             \
                                                                          \
  /*************************************************************/         \
  /* CodeBlob hierarchy (needed for run-time type information) */         \
  /*************************************************************/         \
                                                                          \
  declare_toplevel_type(CodeBlob)                                         \
  declare_type(BufferBlob,            CodeBlob)                           \
  declare_noncore_type(nmethod,       CodeBlob)                           \
  declare_type(RuntimeStub,           CodeBlob)                           \
  declare_type(SingletonBlob,         CodeBlob)                           \
  declare_type(SafepointBlob,         SingletonBlob)                      \
  declare_type(DeoptimizationBlob,    SingletonBlob)                      \
  declare_type(OSRAdapter,            CodeBlob)                           \
  declare_c2_type(C2IAdapter,         CodeBlob)                           \
  declare_c2_type(ExceptionBlob,      SingletonBlob)                      \
  declare_c2_type(I2CAdapter,         CodeBlob)                           \
  declare_c2_type(UncommonTrapBlob,   CodeBlob)                           \
                                                                          \
  /***************************************/                               \
  /* PcDesc and other compiled code info */                               \
  /***************************************/                               \
                                                                          \
  declare_noncore_toplevel_type(PcDesc)                                   \
                                                                          \
  /************************/                                              \
  /* OopMap and OopMapSet */                                              \
  /************************/                                              \
                                                                          \
  declare_toplevel_type(OopMap)                                           \
  declare_toplevel_type(OopMapSet)                                        \
                                                                          \
  /********************/                                                  \
  /* CompressedStream */                                                  \
  /********************/                                                  \
                                                                          \
  declare_toplevel_type(CompressedStream)                                 \
                                                                          \
  /**************/                                                        \
  /* SharedInfo */                                                        \
  /**************/                                                        \
                                                                          \
  declare_toplevel_type(SharedInfo)                                       \
                                                                          \
  /*********************************/                                     \
  /* JNIHandles and JNIHandleBlock */                                     \
  /*********************************/                                     \
                                                                          \
  declare_toplevel_type(JNIHandles)                                       \
  declare_toplevel_type(JNIHandleBlock)                                   \
                                                                          \
  /**********************/                                                \
  /* Runtime1 (C1 only) */                                                \
  /**********************/                                                \
                                                                          \
  declare_c1_toplevel_type(Runtime1)                                      \
                                                                          \
  /************/                                                          \
  /* Monitors */                                                          \
  /************/                                                          \
                                                                          \
  declare_toplevel_type(ObjectMonitor)                                    \
  declare_toplevel_type(ObjectSynchronizer)                               \
  declare_toplevel_type(BasicLock)                                        \
  declare_toplevel_type(BasicObjectLock)                                  \
                                                                          \
  /*********************/                                                 \
  /* Matcher (C2 only) */                                                 \
  /*********************/                                                 \
                                                                          \
  /* NOTE: this is not really a toplevel type, but we only need */        \
  /* this one -- FIXME later if necessary */                              \
  declare_c2_toplevel_type(Matcher)                                       \
                                                                          \
  /********************/                                                  \
  /* -XX flags        */                                                  \
  /********************/                                                  \
                                                                          \
  declare_toplevel_type(Flag)                                             \
  declare_toplevel_type(Flag*)                                            \
                                                                          \
  /********************/                                                  \
  /* vm version       */                                                  \
  /********************/                                                  \
                                                                          \
  declare_toplevel_type(Abstract_VM_Version)                              \
                                                                          \
  /*************/                                                         \
  /* Arguments */                                                         \
  /*************/                                                         \
                                                                          \
  declare_toplevel_type(Arguments)                                        \
                                                                          \
  /***************/                                                       \
  /* Other types */                                                       \
  /***************/                                                       \
                                                                          \
  /* all enum types */                                                    \
                                                                          \
   declare_integer_type(Bytecodes::Code)                                  \
   declare_integer_type(Generation::Name)                                 \
   declare_integer_type(instanceKlass::ClassState)                        \
   declare_integer_type(JavaThreadState)                                  \
   declare_noncore_integer_type(Location::Type)                           \
   declare_noncore_integer_type(Location::Where)                          \
   declare_integer_type(PermGen::Name)                                    \
   declare_integer_type(Universe::JVMPIState)                             \
                                                                          \
   declare_integer_type(AccessFlags)  /* FIXME: wrong type (not integer) */\
  declare_toplevel_type(address)      /* FIXME: should this be an integer type? */\
  declare_toplevel_type(BreakpointInfo)                                   \
  declare_toplevel_type(BreakpointInfo*)                                  \
  declare_toplevel_type(CodeBlob*)                                        \
  declare_toplevel_type(CompressedWriteStream*)                           \
  declare_toplevel_type(ConstantPoolCacheEntry)                           \
  declare_toplevel_type(elapsedTimer)                                     \
  declare_toplevel_type(intptr_t*)                                        \
   declare_noncore_unsigned_integer_type(InvocationCounter) /* FIXME: wrong type (not integer) */ \
  declare_toplevel_type(JavaThread*)                                      \
  declare_toplevel_type(SurrogateLockerThread*)                           \
  declare_toplevel_type(ConcurrentMarkSweepThread*)                       \
  declare_toplevel_type(jbyte*)                                           \
  declare_toplevel_type(jbyte**)                                          \
  declare_toplevel_type(jint*)                                            \
  declare_toplevel_type(jniIdMapBase*)                                    \
  declare_unsigned_integer_type(juint)                                    \
  declare_unsigned_integer_type(julong)                                   \
  declare_toplevel_type(JNIHandleBlock*)                                  \
  declare_toplevel_type(JNIid)                                            \
  declare_toplevel_type(JNIid*)                                           \
  declare_toplevel_type(Mutex*)                                           \
  declare_toplevel_type(nmethod*)                                         \
  declare_toplevel_type(ObjectMonitor*)                                   \
  declare_toplevel_type(oop*)                                             \
  declare_toplevel_type(OopMap**)                                         \
  declare_toplevel_type(OopMapCache*)                                     \
  declare_toplevel_type(OopMapSet*)                                       \
  declare_integer_type(OptoReg::Name)                                     \
  declare_toplevel_type(OSThread*)                                        \
   declare_integer_type(ReferenceType)                                    \
  declare_toplevel_type(StubQueue*)                                       \
  declare_toplevel_type(Thread*)                                          \
  declare_toplevel_type(Universe)

  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_TYPES_OS_CPU macro (and must be */
  /* present there)                                                   */

//--------------------------------------------------------------------------------
// VM_INT_CONSTANTS
//
// This table contains integer constants required over in the
// serviceability agent. The "declare_constant" macro is used for all
// enums, etc., while "declare_preprocessor_constant" must be used for
// all #defined constants.

#define VM_INT_CONSTANTS(declare_constant, declare_preprocessor_constant, declare_noncore_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry) \
                                                                          \
  /******************/                                                    \
  /* Useful globals */                                                    \
  /******************/                                                    \
                                                                          \
  declare_constant(UseTLAB)                                               \
                                                                          \
  /**************/                                                        \
  /* Stack bias */                                                        \
  /**************/                                                        \
                                                                          \
  declare_preprocessor_constant("STACK_BIAS", STACK_BIAS)                 \
                                                                          \
  /****************/                                                      \
  /* Object sizes */                                                      \
  /****************/                                                      \
                                                                          \
  declare_constant(LogBytesPerWord)                                       \
  declare_constant(BytesPerLong)                                          \
                                                                          \
  /********************/                                                  \
  /* Object alignment */                                                  \
  /********************/                                                  \
                                                                          \
  declare_constant(MinObjAlignmentInBytes)                                \
                                                                          \
  /************************************************/                      \
  /* Shared JavaThread constants (see thread.hpp) */                      \
  /************************************************/                      \
                                                                          \
  declare_constant(JavaFrameAnchor::not_at_call_flag)                     \
                                                                          \
  /********************************************/                          \
  /* Generation and Space Hierarchy Constants */                          \
  /********************************************/                          \
                                                                          \
  declare_constant(ageTable::table_size)                                  \
                                                                          \
  declare_constant(BarrierSet::ModRef)                                    \
  declare_constant(BarrierSet::CardTableModRef)                           \
  declare_constant(BarrierSet::Other)                                     \
                                                                          \
  declare_constant(BlockOffsetSharedArray::LogN)                          \
  declare_constant(BlockOffsetSharedArray::LogN_words)                    \
  declare_constant(BlockOffsetSharedArray::N_bytes)                       \
  declare_constant(BlockOffsetSharedArray::N_words)                       \
                                                                          \
  declare_constant(BlockOffsetArray::N_words)                             \
                                                                          \
  declare_constant(CarRememberedSet::initial_capacity)                    \
                                                                          \
  declare_constant(CardTableModRefBS::clean_card)                         \
  declare_constant(CardTableModRefBS::last_card)                          \
  declare_constant(CardTableModRefBS::dirty_card)                         \
  declare_constant(CardTableModRefBS::Precise)                            \
  declare_constant(CardTableModRefBS::ObjHeadPreciseArray)                \
  declare_constant(CardTableModRefBS::card_shift)                         \
  declare_constant(CardTableModRefBS::card_size)                          \
  declare_constant(CardTableModRefBS::card_size_in_words)                 \
                                                                          \
  declare_constant(CardTableRS::youngergen_card)	                  \
                                                                          \
  declare_constant(CarTableDesc::invalid_train_number)                    \
  declare_constant(CarTableDesc::newgen_train_number)                     \
  declare_constant(CarTableDesc::special_nonoop_train_number)             \
  declare_constant(CarTableDesc::initial_train_number)                    \
  declare_constant(CarTableDesc::invalid_car_number)                      \
  declare_constant(CarTableDesc::initial_car_number)                      \
                                                                          \
  declare_constant(CollectedHeap::Abstract)                               \
  declare_constant(CollectedHeap::SharedHeap)                             \
  declare_constant(CollectedHeap::GenCollectedHeap)                       \
                                                                          \
  declare_constant(GenCollectedHeap::max_gens)                            \
                                                                          \
  /* constants from Generation::Name enum */                              \
                                                                          \
  declare_constant(Generation::DefNew)                                    \
  declare_constant(Generation::ParNew)                                    \
  declare_constant(Generation::MarkSweepCompact)                          \
  declare_constant(Generation::TrainGen)                                  \
  declare_constant(Generation::ConcurrentMarkSweep)                       \
  declare_constant(Generation::Other)                                     \
                                                                          \
  declare_constant(Generation::LogOfGenGrain)                             \
  declare_constant(Generation::GenGrain)                                  \
                                                                          \
  declare_constant(HeapWordSize)                                          \
  declare_constant(LogHeapWordSize)                                       \
  declare_constant(HeapWordsPerOop)                                       \
                                                                          \
  /* constants from PermGen::Name enum */                                 \
                                                                          \
  declare_constant(PermGen::MarkSweepCompact)                             \
  declare_constant(PermGen::MarkSweep)                                    \
  declare_constant(PermGen::ConcurrentMarkSweep)                          \
                                                                          \
  /************************/                                              \
  /* PerfMemory - jvmstat */                                              \
  /************************/                                              \
                                                                          \
  declare_preprocessor_constant("PERFDATA_MAJOR_VERSION", PERFDATA_MAJOR_VERSION) \
  declare_preprocessor_constant("PERFDATA_MINOR_VERSION", PERFDATA_MINOR_VERSION) \
  declare_preprocessor_constant("PERFDATA_BIG_ENDIAN", PERFDATA_BIG_ENDIAN)       \
  declare_preprocessor_constant("PERFDATA_LITTLE_ENDIAN", PERFDATA_LITTLE_ENDIAN) \
                                                                          \
  /***************/                                                       \
  /* SymbolTable */                                                       \
  /***************/                                                       \
                                                                          \
  declare_constant(SymbolTable::symbol_table_size)                        \
                                                                          \
  /********************/                                                  \
  /* SystemDictionary */                                                  \
  /********************/                                                  \
                                                                          \
  declare_constant(SystemDictionary::_loader_constraint_size)             \
  declare_constant(SystemDictionary::_nof_buckets)                        \
                                                                          \
  /***********************************/                                   \
  /* LoaderConstraintTable constants */                                   \
  /***********************************/                                   \
                                                                          \
  declare_constant(LoaderConstraintTable::_loader_constraint_size)        \
  declare_constant(LoaderConstraintTable::_nof_buckets)                   \
                                                                          \
  /************************************************************/          \
  /* HotSpot specific JVM_ACC constants from global anon enum */          \
  /************************************************************/          \
                                                                          \
  declare_constant(JVM_ACC_WRITTEN_FLAGS)                                 \
  declare_constant(JVM_ACC_MONITOR_MATCH)                                 \
  declare_constant(JVM_ACC_HAS_MONITOR_BYTECODES)                         \
  declare_constant(JVM_ACC_HAS_LOOPS)                                     \
  declare_constant(JVM_ACC_LOOPS_FLAG_INIT)                               \
  declare_constant(JVM_ACC_QUEUED)                                        \
  declare_constant(JVM_ACC_NOT_OSR_COMPILABLE)                            \
  declare_constant(JVM_ACC_HAS_LINE_NUMBER_TABLE)                         \
  declare_constant(JVM_ACC_HAS_CHECKED_EXCEPTIONS)                        \
  declare_constant(JVM_ACC_HAS_JSRS)                                      \
  declare_constant(JVM_ACC_IS_OLD_VERSION)                                \
  declare_constant(JVM_ACC_IS_NON_EMCP_WITH_NEW_VERSION)                  \
  declare_constant(JVM_ACC_CAN_BE_FASTPATH_ALLOCATED)                     \
  declare_constant(JVM_ACC_HAS_MIRANDA_METHODS)                           \
  declare_constant(JVM_ACC_HAS_VANILLA_CONSTRUCTOR)                       \
  declare_constant(JVM_ACC_HAS_FINALIZER)                                 \
  declare_constant(JVM_ACC_IS_CLONEABLE)                                  \
  declare_constant(JVM_ACC_HAS_LOCAL_VARIABLE_TABLE)                      \
  declare_constant(JVM_ACC_PROMOTED_FLAGS)                                \
  declare_constant(JVM_ACC_FIELD_ACCESS_WATCHED)                          \
  declare_constant(JVM_ACC_FIELD_MODIFICATION_WATCHED)                    \
                                                                          \
  /*****************************/                                         \
  /* Thread::SuspendFlags enum */                                         \
  /*****************************/                                         \
                                                                          \
  declare_constant(Thread::_vm_suspended)                                 \
  declare_constant(Thread::_is_baby_thread)                               \
  declare_constant(Thread::_suspended_as_baby)                            \
  declare_constant(Thread::_external_suspend)                             \
  declare_constant(Thread::_ext_suspended)                                \
  declare_constant(Thread::_has_async_exception)                          \
                                                                          \
  /*******************/                                                   \
  /* JavaThreadState */                                                   \
  /*******************/                                                   \
                                                                          \
  declare_constant(_thread_uninitialized)                                 \
  declare_constant(_thread_new)                                           \
  declare_constant(_thread_new_trans)                                     \
  declare_constant(_thread_in_native)                                     \
  declare_constant(_thread_in_native_trans)                               \
  declare_constant(_thread_in_vm)                                         \
  declare_constant(_thread_in_vm_trans)                                   \
  declare_constant(_thread_in_Java)                                       \
  declare_constant(_thread_in_Java_trans)                                 \
  declare_constant(_thread_blocked)                                       \
  declare_constant(_thread_blocked_trans)                                 \
                                                                          \
  /******************************/                                        \
  /* Klass - fast subtype check */                                        \
  /******************************/                                        \
                                                                          \
  declare_constant(Klass::_primary_super_limit)                           \
                                                                          \
  /********************************/                                      \
  /* constMethodOopDesc anon-enum */                                      \
  /********************************/                                      \
                                                                          \
  declare_constant(constMethodOopDesc::_has_linenumber_table)             \
  declare_constant(constMethodOopDesc::_has_checked_exceptions)           \
  declare_constant(constMethodOopDesc::_has_localvariable_table)          \
                                                                          \
  /*************************************/                                 \
  /* instanceKlass FieldOffset enum    */                                 \
  /*************************************/                                 \
                                                                          \
  declare_constant(instanceKlass::access_flags_offset)                    \
  declare_constant(instanceKlass::name_index_offset)                      \
  declare_constant(instanceKlass::signature_index_offset)                 \
  declare_constant(instanceKlass::initval_index_offset)                   \
  declare_constant(instanceKlass::low_offset)                             \
  declare_constant(instanceKlass::high_offset)                            \
  declare_constant(instanceKlass::generic_signature_offset)               \
  declare_constant(instanceKlass::next_offset)                            \
                                                                          \
  /************************************************/                      \
  /* instanceKlass InnerClassAttributeOffset enum */                      \
  /************************************************/                      \
                                                                          \
  declare_constant(instanceKlass::inner_class_inner_class_info_offset)    \
  declare_constant(instanceKlass::inner_class_outer_class_info_offset)    \
  declare_constant(instanceKlass::inner_class_inner_name_offset)          \
  declare_constant(instanceKlass::inner_class_access_flags_offset)        \
  declare_constant(instanceKlass::inner_class_next_offset)                \
                                                                          \
  /*********************************/                                     \
  /* instanceKlass ClassState enum */                                     \
  /*********************************/                                     \
                                                                          \
  declare_constant(instanceKlass::unparsable_by_gc)                       \
  declare_constant(instanceKlass::allocated)                              \
  declare_constant(instanceKlass::loaded)                                 \
  declare_constant(instanceKlass::linked)                                 \
  declare_constant(instanceKlass::being_initialized)                      \
  declare_constant(instanceKlass::fully_initialized)                      \
  declare_constant(instanceKlass::initialization_error)                   \
                                                                          \
  /*********************************/                                     \
  /* symbolOop - symbol max length */                                     \
  /*********************************/                                     \
                                                                          \
  declare_constant(symbolOopDesc::max_symbol_length)                      \
                                                                          \
  /*********************************************/                         \
  /* ConstantPoolCacheEntry FlagBitValues enum */                         \
  /*********************************************/                         \
                                                                          \
  declare_constant(ConstantPoolCacheEntry::hotSwapBit)                    \
  declare_constant(ConstantPoolCacheEntry::methodInterface)               \
  declare_constant(ConstantPoolCacheEntry::volatileField)                 \
  declare_constant(ConstantPoolCacheEntry::vfinalMethod)                  \
  declare_constant(ConstantPoolCacheEntry::finalField)                    \
                                                                          \
  /******************************************/                            \
  /* ConstantPoolCacheEntry FlagValues enum */                            \
  /******************************************/                            \
                                                                          \
  declare_constant(ConstantPoolCacheEntry::tosBits)                       \
                                                                          \
  /*********************************/                                     \
  /* java_lang_Class field offsets */                                     \
  /*********************************/                                     \
                                                                          \
  declare_constant(java_lang_Class::hc_klass_offset)                      \
  declare_constant(java_lang_Class::hc_resolved_constructor_offset)       \
  declare_constant(java_lang_Class::hc_number_of_fake_oop_fields)         \
                                                                          \
  /***************************************/                               \
  /* java_lang_Thread::ThreadStatus enum */                               \
  /***************************************/                               \
                                                                          \
  declare_constant(java_lang_Thread::NEW)                                 \
  declare_constant(java_lang_Thread::RUNNABLE)                            \
  declare_constant(java_lang_Thread::SLEEPING)                            \
  declare_constant(java_lang_Thread::IN_OBJECT_WAIT)                      \
  declare_constant(java_lang_Thread::IN_OBJECT_WAIT_TIMED)                \
  declare_constant(java_lang_Thread::PARKED)                              \
  declare_constant(java_lang_Thread::PARKED_TIMED)                        \
  declare_constant(java_lang_Thread::BLOCKED_ON_MONITOR_ENTER)            \
  declare_constant(java_lang_Thread::TERMINATED)                          \
                                                                          \
  /******************************/                                        \
  /* Debug info (non-core only) */                                        \
  /******************************/                                        \
                                                                          \
  declare_noncore_constant(Location::OFFSET_MASK)                         \
  declare_noncore_constant(Location::OFFSET_SHIFT)                        \
  declare_noncore_constant(Location::TYPE_MASK)                           \
  declare_noncore_constant(Location::TYPE_SHIFT)                          \
  declare_noncore_constant(Location::WHERE_MASK)                          \
  declare_noncore_constant(Location::WHERE_SHIFT)                         \
                                                                          \
  /* constants from Location::Type enum  */                               \
                                                                          \
  declare_noncore_constant(Location::normal)                              \
  declare_noncore_constant(Location::oop)                                 \
  declare_noncore_constant(Location::int_in_long)                         \
  declare_noncore_constant(Location::lng)                                 \
  declare_noncore_constant(Location::float_in_dbl)                        \
  declare_noncore_constant(Location::dbl)                                 \
  declare_noncore_constant(Location::addr)                                \
  declare_noncore_constant(Location::invalid)                             \
                                                                          \
  /* constants from Location::Where enum */                               \
                                                                          \
  declare_noncore_constant(Location::on_stack)                            \
  declare_noncore_constant(Location::in_register)                         \
                                                                          \
  /*********************/                                                 \
  /* Matcher (C2 only) */                                                 \
  /*********************/                                                 \
                                                                          \
  declare_c2_preprocessor_constant("Matcher::interpreter_frame_pointer_reg", Matcher::interpreter_frame_pointer_reg()) \
                                                                          \
  /*********************************************/                         \
  /* MethodCompilation (globalDefinitions.hpp) */                         \
  /*********************************************/                         \
                                                                          \
  declare_constant(InvocationEntryBci)                                    \
  declare_constant(InvalidOSREntryBci)                                    \
                                                                          \
  /***************/                                                       \
  /* OopMapValue */                                                       \
  /***************/                                                       \
                                                                          \
  declare_constant(OopMapValue::type_bits)                                \
  declare_constant(OopMapValue::register_bits)                            \
  declare_constant(OopMapValue::type_shift)                               \
  declare_constant(OopMapValue::register_shift)                           \
  declare_constant(OopMapValue::type_mask)                                \
  declare_constant(OopMapValue::type_mask_in_place)                       \
  declare_constant(OopMapValue::register_mask)                            \
  declare_constant(OopMapValue::register_mask_in_place)                   \
  declare_constant(OopMapValue::unused_value)                             \
  declare_constant(OopMapValue::oop_value)                                \
  declare_constant(OopMapValue::value_value)                              \
  declare_constant(OopMapValue::dead_value)                               \
  declare_constant(OopMapValue::callee_saved_value)                       \
  declare_constant(OopMapValue::derived_oop_value)                        \
                                                                          \
  /******************/                                                    \
  /* JNIHandleBlock */                                                    \
  /******************/                                                    \
                                                                          \
  declare_constant(JNIHandleBlock::block_size_in_oops)                    \
                                                                          \
  /**********************/                                                \
  /* ObjectSynchronizer */                                                \
  /**********************/                                                \
                                                                          \
  declare_constant(ObjectSynchronizer::BLOCK_SIZE)                        \
                                                                          \
  /********************************/                                      \
  /* Calling convention constants */                                      \
  /********************************/                                      \
                                                                          \
  declare_constant(RegisterImpl::number_of_registers)                     \
  declare_c1_constant(REG_COUNT)                                          \
  declare_c2_preprocessor_constant("REG_COUNT", REG_COUNT)                \
  declare_c2_preprocessor_constant("SAVED_ON_ENTRY_REG_COUNT", SAVED_ON_ENTRY_REG_COUNT) \
  declare_c2_preprocessor_constant("C_SAVED_ON_ENTRY_REG_COUNT", C_SAVED_ON_ENTRY_REG_COUNT)

  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_INT_CONSTANTS_OS_CPU macro (and */
  /* must be present there)                                           */

//--------------------------------------------------------------------------------
// VM_LONG_CONSTANTS
//
// This table contains long constants required over in the
// serviceability agent. The "declare_constant" macro is used for all
// enums, etc., while "declare_preprocessor_constant" must be used for
// all #defined constants.

#define VM_LONG_CONSTANTS(declare_constant, declare_preprocessor_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry) \
                                                                          \
  /*********************/                                                 \
  /* MarkOop constants */                                                 \
  /*********************/                                                 \
                                                                          \
  /* Note: some of these are declared as long constants just for */       \
  /* consistency. The mask constants are the only ones requiring */       \
  /* 64 bits (on 64-bit platforms). */                                    \
  declare_constant(markOopDesc::age_bits)                                 \
  declare_constant(markOopDesc::lock_bits)                                \
  declare_constant(markOopDesc::max_hash_bits)                            \
  declare_constant(markOopDesc::hash_bits)                                \
  declare_constant(markOopDesc::lock_shift)                               \
  declare_constant(markOopDesc::age_shift)                                \
  declare_constant(markOopDesc::hash_shift)                               \
  declare_constant(markOopDesc::lock_mask)                                \
  declare_constant(markOopDesc::lock_mask_in_place)                       \
  declare_constant(markOopDesc::age_mask)                                 \
  declare_constant(markOopDesc::age_mask_in_place)                        \
  declare_constant(markOopDesc::hash_mask)                                \
  declare_constant(markOopDesc::hash_mask_in_place)                       \
  declare_constant(markOopDesc::locked_value)                             \
  declare_constant(markOopDesc::unlocked_value)                           \
  declare_constant(markOopDesc::monitor_value)                            \
  declare_constant(markOopDesc::marked_value)                             \
  declare_constant(markOopDesc::no_hash)                                  \
  declare_constant(markOopDesc::no_hash_in_place)                         \
  declare_constant(markOopDesc::no_lock_in_place)                         \
  declare_constant(markOopDesc::max_age)

  /* NOTE that we do not use the last_entry() macro here; it is used   */
  /* in vmStructs_<os>_<cpu>.hpp's VM_LONG_CONSTANTS_OS_CPU macro (and */
  /* must be present there)                                            */


//--------------------------------------------------------------------------------
// Macros operating on the above lists
//--------------------------------------------------------------------------------

// This utility macro quotes the passed string
#define QUOTE(x) #x

//--------------------------------------------------------------------------------
// VMStructEntry macros
//

// This macro generates a VMStructEntry line for a nonstatic field
#define GENERATE_NONSTATIC_VM_STRUCT_ENTRY(typeName, fieldName, type)              \
 { QUOTE(typeName), QUOTE(fieldName), QUOTE(type), 0, offsetof(typeName, fieldName), NULL },

// This macro generates a VMStructEntry line for a static field
#define GENERATE_STATIC_VM_STRUCT_ENTRY(typeName, fieldName, type)                 \
 { QUOTE(typeName), QUOTE(fieldName), QUOTE(type), 1, 0, &typeName::fieldName },

// This macro generates a VMStructEntry line for an unchecked
// nonstatic field, in which the size of the type is also specified.
// The type string is given as NULL, indicating an "opaque" type.
#define GENERATE_UNCHECKED_NONSTATIC_VM_STRUCT_ENTRY(typeName, fieldName, size)    \
 { QUOTE(typeName), QUOTE(fieldName), NULL, 0, offsetof(typeName, fieldName), NULL },

// This macro generates a VMStructEntry line for an unchecked
// static field, in which the size of the type is also specified.
// The type string is given as NULL, indicating an "opaque" type.
#define GENERATE_UNCHECKED_STATIC_VM_STRUCT_ENTRY(typeName, fieldName, size)       \
 { QUOTE(typeName), QUOTE(fieldName), NULL, 1, 0, (void*) &typeName::fieldName },

// This macro generates the sentinel value indicating the end of the list
#define GENERATE_VM_STRUCT_LAST_ENTRY() \
 { NULL, NULL, NULL, 0, 0, NULL }

// This macro checks the type of a VMStructEntry by comparing pointer types
#define CHECK_NONSTATIC_VM_STRUCT_ENTRY(typeName, fieldName, type)                 \
 {typeName *dummyObj = NULL; type* dummy = &dummyObj->fieldName; }
 
// This macro checks the type of a volatile VMStructEntry by comparing pointer types
#define CHECK_VOLATILE_NONSTATIC_VM_STRUCT_ENTRY(typeName, fieldName, type)        \
 {typedef type dummyvtype; typeName *dummyObj = NULL; volatile dummyvtype* dummy = &dummyObj->fieldName; }
 
// This macro checks the type of a VMStructEntry by comparing pointer types
#define CHECK_STATIC_VM_STRUCT_ENTRY(typeName, fieldName, type)                    \
 {type* dummy = &typeName::fieldName; }
 
// This macro ensures the type of a field and its containing type are
// present in the type table. The assertion string is shorter than
// preferable because (incredibly) of a bug in Solstice NFS client
// which seems to prevent very long lines from compiling. This assertion
// means that an entry in VMStructs::localHotSpotVMStructs[] was not
// found in VMStructs::localHotSpotVMTypes[].
#define ENSURE_FIELD_TYPE_PRESENT(typeName, fieldName, type)                       \
 { assert(findType(QUOTE(typeName)) != 0, "type \"" QUOTE(typeName) "\" not found in type table"); \
   assert(findType(QUOTE(type)) != 0, "type \"" QUOTE(type) "\" not found in type table"); }

// This is a no-op macro for unchecked fields
#define CHECK_NO_OP(a, b, c)

// This is a no-op macro for the sentinel value
#define CHECK_SENTINEL()

//
// Build-specific macros:
//

// Generate and check a nonstatic field in non-product builds
#ifndef PRODUCT
# define GENERATE_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_NONPRODUCT_FIELD_TYPE_PRESENT(a, b, c)          ENSURE_FIELD_TYPE_PRESENT(a, b, c)
#  ifndef CORE
#   define GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
#   define CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
#   define ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT(a, b, c)          ENSURE_FIELD_TYPE_PRESENT(a, b, c)
#  else
#   define GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
#   define CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
#   define ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT(a, b, c)
#  endif
#else
# define GENERATE_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_NONPRODUCT_FIELD_TYPE_PRESENT(a, b, c)
# define GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT(a, b, c)
#endif /* PRODUCT */

// Generate and check a nonstatic field in C1 builds
#ifdef COMPILER1
# define GENERATE_C1_NONSTATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_C1_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_C1_FIELD_TYPE_PRESENT(a, b, c)          ENSURE_FIELD_TYPE_PRESENT(a, b, c)
#else
# define GENERATE_C1_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_C1_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_C1_FIELD_TYPE_PRESENT(a, b, c)
#endif /* COMPILER1 */
// Generate and check a nonstatic field in C2 builds
#ifdef COMPILER2
# define GENERATE_C2_NONSTATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_C2_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_C2_FIELD_TYPE_PRESENT(a, b, c)          ENSURE_FIELD_TYPE_PRESENT(a, b, c)
#else
# define GENERATE_C2_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_C2_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_C2_FIELD_TYPE_PRESENT(a, b, c)
#endif /* COMPILER2 */

// Generate and check nonstatic and static fields in non-core builds
#ifndef CORE
# define GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define GENERATE_NONCORE_STATIC_VM_STRUCT_ENTRY(a, b, c)    GENERATE_STATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_VOLATILE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)    CHECK_VOLATILE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONCORE_STATIC_VM_STRUCT_ENTRY(a, b, c)       CHECK_STATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_NONCORE_FIELD_TYPE_PRESENT(a, b, c)          ENSURE_FIELD_TYPE_PRESENT(a, b, c)
#else
# define GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define GENERATE_NONCORE_STATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_VOLATILE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY(a, b, c)
# define CHECK_NONCORE_STATIC_VM_STRUCT_ENTRY(a, b, c)
# define ENSURE_NONCORE_FIELD_TYPE_PRESENT(a, b, c)
#endif /* CORE */

// Generate but do not check a static field in C1 builds
#ifdef COMPILER1
# define GENERATE_C1_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c)
#else
# define GENERATE_C1_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c)
#endif /* COMPILER1 */

// Generate but do not check a static field in C2 builds
#ifdef COMPILER2
# define GENERATE_C2_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c) GENERATE_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c)
#else
# define GENERATE_C2_UNCHECKED_STATIC_VM_STRUCT_ENTRY(a, b, c)
#endif /* COMPILER2 */

//--------------------------------------------------------------------------------
// VMTypeEntry macros
//

#define GENERATE_VM_TYPE_ENTRY(type, superclass) \
 { QUOTE(type), QUOTE(superclass), 0, 0, 0, sizeof(type) },

#define GENERATE_TOPLEVEL_VM_TYPE_ENTRY(type) \
 { QUOTE(type), NULL,              0, 0, 0, sizeof(type) },

#define GENERATE_OOP_VM_TYPE_ENTRY(type) \
 { QUOTE(type), NULL,              1, 0, 0, sizeof(type) },

#define GENERATE_INTEGER_VM_TYPE_ENTRY(type) \
 { QUOTE(type), NULL,              0, 1, 0, sizeof(type) },

#define GENERATE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(type) \
 { QUOTE(type), NULL,              0, 1, 1, sizeof(type) },

#define GENERATE_VM_TYPE_LAST_ENTRY() \
 { NULL, NULL, 0, 0, 0, 0 }

#define CHECK_VM_TYPE_ENTRY(type, superclass) \
 { type* dummyObj = NULL; superclass* dummySuperObj = dummyObj; }

#define CHECK_VM_TYPE_NO_OP(a)
#define CHECK_SINGLE_ARG_VM_TYPE_NO_OP(a)

//
// Build-specific macros:
//

#ifndef CORE
# define GENERATE_NONCORE_VM_TYPE_ENTRY(a, b)                GENERATE_VM_TYPE_ENTRY(a, b)
# define GENERATE_NONCORE_TOPLEVEL_VM_TYPE_ENTRY(a)          GENERATE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_INTEGER_VM_TYPE_ENTRY(a)           GENERATE_INTEGER_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(a)  GENERATE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_OOP_VM_TYPE_ENTRY(a)               GENERATE_OOP_VM_TYPE_ENTRY(a)
# define CHECK_NONCORE_VM_TYPE_ENTRY(a, b)                   CHECK_VM_TYPE_ENTRY(a, b)
# define CHECK_NONCORE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(a)
#else
# define GENERATE_NONCORE_VM_TYPE_ENTRY(a, b)
# define GENERATE_NONCORE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_INTEGER_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(a)
# define GENERATE_NONCORE_OOP_VM_TYPE_ENTRY(a)
# define CHECK_NONCORE_VM_TYPE_ENTRY(a, b)
# define CHECK_NONCORE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY(a)
#endif /* CORE */

#ifdef COMPILER1
# define GENERATE_C1_TOPLEVEL_VM_TYPE_ENTRY(a)               GENERATE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_C1_TOPLEVEL_VM_TYPE_ENTRY(a)
#else
# define GENERATE_C1_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_C1_TOPLEVEL_VM_TYPE_ENTRY(a)
#endif /* COMPILER1 */

#ifdef COMPILER2
# define GENERATE_C2_VM_TYPE_ENTRY(a, b)                     GENERATE_VM_TYPE_ENTRY(a, b)
# define CHECK_C2_VM_TYPE_ENTRY(a, b)                        CHECK_VM_TYPE_ENTRY(a, b)
# define GENERATE_C2_TOPLEVEL_VM_TYPE_ENTRY(a)               GENERATE_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_C2_TOPLEVEL_VM_TYPE_ENTRY(a)
#else
# define GENERATE_C2_VM_TYPE_ENTRY(a, b)
# define CHECK_C2_VM_TYPE_ENTRY(a, b)
# define GENERATE_C2_TOPLEVEL_VM_TYPE_ENTRY(a)
# define CHECK_C2_TOPLEVEL_VM_TYPE_ENTRY(a)
#endif /* COMPILER2 */


//--------------------------------------------------------------------------------
// VMIntConstantEntry macros
//

#define GENERATE_VM_INT_CONSTANT_ENTRY(name) \
 { QUOTE(name), (int32_t) name },

#define GENERATE_PREPROCESSOR_VM_INT_CONSTANT_ENTRY(name, value) \
 { name, (int32_t) value },

// This macro generates the sentinel value indicating the end of the list
#define GENERATE_VM_INT_CONSTANT_LAST_ENTRY() \
 { NULL, 0 }

// Generate an int constant for a noncore build
#ifndef CORE
# define GENERATE_NONCORE_VM_INT_CONSTANT_ENTRY(name)  GENERATE_VM_INT_CONSTANT_ENTRY(name)
#else
# define GENERATE_NONCORE_VM_INT_CONSTANT_ENTRY(name)
#endif /* !CORE */

// Generate an int constant for a C1 build
#ifdef COMPILER1
# define GENERATE_C1_VM_INT_CONSTANT_ENTRY(name)  GENERATE_VM_INT_CONSTANT_ENTRY(name)
#else
# define GENERATE_C1_VM_INT_CONSTANT_ENTRY(name)
#endif /* COMPILER1 */

// Generate an int constant for a C2 build
#ifdef COMPILER2
# define GENERATE_C2_VM_INT_CONSTANT_ENTRY(name)                      GENERATE_VM_INT_CONSTANT_ENTRY(name)
# define GENERATE_C2_PREPROCESSOR_VM_INT_CONSTANT_ENTRY(name, value)  GENERATE_PREPROCESSOR_VM_INT_CONSTANT_ENTRY(name, value)
#else
# define GENERATE_C2_VM_INT_CONSTANT_ENTRY(name)
# define GENERATE_C2_PREPROCESSOR_VM_INT_CONSTANT_ENTRY(name, value)
#endif /* COMPILER1 */

//--------------------------------------------------------------------------------
// VMLongConstantEntry macros
//

#define GENERATE_VM_LONG_CONSTANT_ENTRY(name) \
 { QUOTE(name), (uint64_t) name },

#define GENERATE_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY(name, value) \
 { name, (uint64_t) value },

// This macro generates the sentinel value indicating the end of the list
#define GENERATE_VM_LONG_CONSTANT_LAST_ENTRY() \
 { NULL, 0 }

// Generate a long constant for a C1 build
#ifdef COMPILER1
# define GENERATE_C1_VM_LONG_CONSTANT_ENTRY(name)  GENERATE_VM_LONG_CONSTANT_ENTRY(name)
#else
# define GENERATE_C1_VM_LONG_CONSTANT_ENTRY(name)
#endif /* COMPILER1 */

// Generate a long constant for a C2 build
#ifdef COMPILER2
# define GENERATE_C2_VM_LONG_CONSTANT_ENTRY(name)                     GENERATE_VM_LONG_CONSTANT_ENTRY(name)
# define GENERATE_C2_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY(name, value) GENERATE_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY(name, value)
#else
# define GENERATE_C2_VM_LONG_CONSTANT_ENTRY(name)
# define GENERATE_C2_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY(name, value)
#endif /* COMPILER1 */

//
// Instantiation of VMStructEntries, VMTypeEntries and VMIntConstantEntries
// 

// These initializers are allowed to access private fields in classes
// as long as class VMStructs is a friend
VMStructEntry VMStructs::localHotSpotVMStructs[] = {
  VM_STRUCTS(GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_STATIC_VM_STRUCT_ENTRY, \
             GENERATE_UNCHECKED_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_C1_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_C2_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             GENERATE_NONCORE_STATIC_VM_STRUCT_ENTRY, \
             GENERATE_C1_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
             GENERATE_C2_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
             GENERATE_VM_STRUCT_LAST_ENTRY)

  VM_STRUCTS_CPU(GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_STATIC_VM_STRUCT_ENTRY, \
                 GENERATE_UNCHECKED_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_C2_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                 GENERATE_NONCORE_STATIC_VM_STRUCT_ENTRY, \
                 GENERATE_C1_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
                 GENERATE_C2_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
                 GENERATE_VM_STRUCT_LAST_ENTRY)
  VM_STRUCTS_OS_CPU(GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_STATIC_VM_STRUCT_ENTRY, \
                    GENERATE_UNCHECKED_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_C2_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
                    GENERATE_NONCORE_STATIC_VM_STRUCT_ENTRY, \
                    GENERATE_C1_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
                    GENERATE_C2_UNCHECKED_STATIC_VM_STRUCT_ENTRY, \
                    GENERATE_VM_STRUCT_LAST_ENTRY)
};

VMTypeEntry VMStructs::localHotSpotVMTypes[] = {
  VM_TYPES(GENERATE_VM_TYPE_ENTRY, GENERATE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_OOP_VM_TYPE_ENTRY, GENERATE_INTEGER_VM_TYPE_ENTRY, GENERATE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_VM_TYPE_ENTRY, GENERATE_NONCORE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_NONCORE_OOP_VM_TYPE_ENTRY, GENERATE_NONCORE_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_C1_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_C2_VM_TYPE_ENTRY, GENERATE_C2_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_VM_TYPE_LAST_ENTRY)
  VM_TYPES_CPU(GENERATE_VM_TYPE_ENTRY, GENERATE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_OOP_VM_TYPE_ENTRY, GENERATE_INTEGER_VM_TYPE_ENTRY, GENERATE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_VM_TYPE_ENTRY, GENERATE_NONCORE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_NONCORE_OOP_VM_TYPE_ENTRY, GENERATE_NONCORE_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_C1_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_C2_VM_TYPE_ENTRY, GENERATE_C2_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_VM_TYPE_LAST_ENTRY)
  VM_TYPES_OS_CPU(GENERATE_VM_TYPE_ENTRY, GENERATE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_OOP_VM_TYPE_ENTRY, GENERATE_INTEGER_VM_TYPE_ENTRY, GENERATE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_VM_TYPE_ENTRY, GENERATE_NONCORE_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_NONCORE_OOP_VM_TYPE_ENTRY, GENERATE_NONCORE_INTEGER_VM_TYPE_ENTRY, GENERATE_NONCORE_UNSIGNED_INTEGER_VM_TYPE_ENTRY, GENERATE_C1_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_C2_VM_TYPE_ENTRY, GENERATE_C2_TOPLEVEL_VM_TYPE_ENTRY, GENERATE_VM_TYPE_LAST_ENTRY)
};

VMIntConstantEntry VMStructs::localHotSpotVMIntConstants[] = {
  VM_INT_CONSTANTS       (GENERATE_VM_INT_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_NONCORE_VM_INT_CONSTANT_ENTRY, GENERATE_C1_VM_INT_CONSTANT_ENTRY, GENERATE_C2_VM_INT_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_VM_INT_CONSTANT_LAST_ENTRY)
  VM_INT_CONSTANTS_CPU   (GENERATE_VM_INT_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_NONCORE_VM_INT_CONSTANT_ENTRY, GENERATE_C1_VM_INT_CONSTANT_ENTRY, GENERATE_C2_VM_INT_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_VM_INT_CONSTANT_LAST_ENTRY)
  VM_INT_CONSTANTS_OS_CPU(GENERATE_VM_INT_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_NONCORE_VM_INT_CONSTANT_ENTRY, GENERATE_C1_VM_INT_CONSTANT_ENTRY, GENERATE_C2_VM_INT_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_INT_CONSTANT_ENTRY, GENERATE_VM_INT_CONSTANT_LAST_ENTRY)
};

VMLongConstantEntry VMStructs::localHotSpotVMLongConstants[] = {
  VM_LONG_CONSTANTS       (GENERATE_VM_LONG_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_C1_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_VM_LONG_CONSTANT_LAST_ENTRY)
  VM_LONG_CONSTANTS_CPU   (GENERATE_VM_LONG_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_C1_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_VM_LONG_CONSTANT_LAST_ENTRY)
  VM_LONG_CONSTANTS_OS_CPU(GENERATE_VM_LONG_CONSTANT_ENTRY, GENERATE_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_C1_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_VM_LONG_CONSTANT_ENTRY, GENERATE_C2_PREPROCESSOR_VM_LONG_CONSTANT_ENTRY, GENERATE_VM_LONG_CONSTANT_LAST_ENTRY)
};

// This is used both to check the types of referenced fields and, in
// debug builds, to ensure that all of the field types are present.
void
VMStructs::init() {
  VM_STRUCTS(CHECK_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_VOLATILE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_C1_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_C2_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_VOLATILE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_NO_OP, \
             CHECK_SENTINEL);
  VM_STRUCTS_CPU(CHECK_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_VOLATILE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_C2_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_VOLATILE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_NO_OP, \
             CHECK_SENTINEL);
  VM_STRUCTS_OS_CPU(CHECK_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_VOLATILE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONPRODUCT_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_C2_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_VOLATILE_NONCORE_NONSTATIC_VM_STRUCT_ENTRY, \
             CHECK_NONCORE_STATIC_VM_STRUCT_ENTRY, \
             CHECK_NO_OP, \
             CHECK_NO_OP, \
             CHECK_SENTINEL);

  VM_TYPES(CHECK_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_NONCORE_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_C1_TOPLEVEL_VM_TYPE_ENTRY, CHECK_C2_VM_TYPE_ENTRY, CHECK_C2_TOPLEVEL_VM_TYPE_ENTRY, CHECK_SENTINEL);
  VM_TYPES_CPU(CHECK_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_NONCORE_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_C1_TOPLEVEL_VM_TYPE_ENTRY, CHECK_C2_VM_TYPE_ENTRY, CHECK_C2_TOPLEVEL_VM_TYPE_ENTRY, CHECK_SENTINEL);
  VM_TYPES_OS_CPU(CHECK_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_NONCORE_VM_TYPE_ENTRY, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_SINGLE_ARG_VM_TYPE_NO_OP, CHECK_C1_TOPLEVEL_VM_TYPE_ENTRY, CHECK_C2_VM_TYPE_ENTRY, CHECK_C2_TOPLEVEL_VM_TYPE_ENTRY, CHECK_SENTINEL);

  //
  // Split VM_STRUCTS() invocation into two parts to allow MS VC++ 6.0
  // to build with the source mounted over SNC3.2. Symptom was that
  // debug build failed with an internal compiler error. Has been seen
  // mounting sources from Solaris 2.6 and 2.7 hosts, but so far not
  // 2.8 hosts. Appears to occur because line is too long.
  //
  // If an assertion failure is triggered here it means that an entry
  // in VMStructs::localHotSpotVMStructs[] was not found in
  // VMStructs::localHotSpotVMTypes[]. (The assertion itself had to be
  // made less descriptive because of this above bug -- see the
  // definition of ENSURE_FIELD_TYPE_PRESENT.)
  //
  // NOTE: taken out because this was just not working on everyone's
  // Solstice NFS setup. If everyone switches to local workspaces on
  // Win32, we can put this back in.
#ifndef _WINDOWS
  debug_only(VM_STRUCTS(ENSURE_FIELD_TYPE_PRESENT, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_SENTINEL));
  debug_only(VM_STRUCTS(CHECK_NO_OP, \
                        ENSURE_FIELD_TYPE_PRESENT, \
                        CHECK_NO_OP, \
                        ENSURE_FIELD_TYPE_PRESENT, \
                        ENSURE_NONPRODUCT_FIELD_TYPE_PRESENT, \
                        ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT, \
                        ENSURE_C1_FIELD_TYPE_PRESENT, \
                        ENSURE_C2_FIELD_TYPE_PRESENT, \
                        ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                        ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                        ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                        CHECK_NO_OP, \
                        CHECK_NO_OP, \
                        CHECK_SENTINEL));
  debug_only(VM_STRUCTS_CPU(ENSURE_FIELD_TYPE_PRESENT, \
                            ENSURE_FIELD_TYPE_PRESENT, \
                            CHECK_NO_OP, \
                            ENSURE_FIELD_TYPE_PRESENT, \
                            ENSURE_NONPRODUCT_FIELD_TYPE_PRESENT, \
                            ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT, \
                            ENSURE_C2_FIELD_TYPE_PRESENT, \
                            ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                            ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                            ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                            CHECK_NO_OP, \
                            CHECK_NO_OP, \
                            CHECK_SENTINEL));
  debug_only(VM_STRUCTS_OS_CPU(ENSURE_FIELD_TYPE_PRESENT, \
                               ENSURE_FIELD_TYPE_PRESENT, \
                               CHECK_NO_OP, \
                               ENSURE_FIELD_TYPE_PRESENT, \
                               ENSURE_NONPRODUCT_FIELD_TYPE_PRESENT, \
                               ENSURE_NONPRODUCT_NONCORE_FIELD_TYPE_PRESENT, \
                               ENSURE_C2_FIELD_TYPE_PRESENT, \
                               ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                               ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                               ENSURE_NONCORE_FIELD_TYPE_PRESENT, \
                               CHECK_NO_OP, \
                               CHECK_NO_OP, \
                               CHECK_SENTINEL));
#endif
}

extern "C" {
JNIEXPORT VMStructEntry* gHotSpotVMStructs                 = VMStructs::localHotSpotVMStructs;
JNIEXPORT uint64_t gHotSpotVMStructEntryTypeNameOffset     = offsetof(VMStructEntry, typeName);
JNIEXPORT uint64_t gHotSpotVMStructEntryFieldNameOffset    = offsetof(VMStructEntry, fieldName);
JNIEXPORT uint64_t gHotSpotVMStructEntryTypeStringOffset   = offsetof(VMStructEntry, typeString);
JNIEXPORT uint64_t gHotSpotVMStructEntryIsStaticOffset     = offsetof(VMStructEntry, isStatic);
JNIEXPORT uint64_t gHotSpotVMStructEntryOffsetOffset       = offsetof(VMStructEntry, offset);
JNIEXPORT uint64_t gHotSpotVMStructEntryAddressOffset      = offsetof(VMStructEntry, address);
JNIEXPORT uint64_t gHotSpotVMStructEntryArrayStride        = ((uint64_t) &gHotSpotVMStructs[1]) - ((uint64_t) &gHotSpotVMStructs[0]);
JNIEXPORT VMTypeEntry*   gHotSpotVMTypes                   = VMStructs::localHotSpotVMTypes;
JNIEXPORT uint64_t gHotSpotVMTypeEntryTypeNameOffset       = offsetof(VMTypeEntry, typeName);
JNIEXPORT uint64_t gHotSpotVMTypeEntrySuperclassNameOffset = offsetof(VMTypeEntry, superclassName);
JNIEXPORT uint64_t gHotSpotVMTypeEntryIsOopTypeOffset      = offsetof(VMTypeEntry, isOopType);
JNIEXPORT uint64_t gHotSpotVMTypeEntryIsIntegerTypeOffset  = offsetof(VMTypeEntry, isIntegerType);
JNIEXPORT uint64_t gHotSpotVMTypeEntryIsUnsignedOffset     = offsetof(VMTypeEntry, isUnsigned);
JNIEXPORT uint64_t gHotSpotVMTypeEntrySizeOffset           = offsetof(VMTypeEntry, size);
JNIEXPORT uint64_t gHotSpotVMTypeEntryArrayStride          = ((uint64_t) &gHotSpotVMTypes[1]) - ((uint64_t) &gHotSpotVMTypes[0]);
JNIEXPORT VMIntConstantEntry* gHotSpotVMIntConstants       = VMStructs::localHotSpotVMIntConstants;
JNIEXPORT uint64_t gHotSpotVMIntConstantEntryNameOffset    = offsetof(VMIntConstantEntry, name);
JNIEXPORT uint64_t gHotSpotVMIntConstantEntryValueOffset   = offsetof(VMIntConstantEntry, value);
JNIEXPORT uint64_t gHotSpotVMIntConstantEntryArrayStride   = ((uint64_t) &gHotSpotVMIntConstants[1]) - ((uint64_t) &gHotSpotVMIntConstants[0]);
JNIEXPORT VMLongConstantEntry* gHotSpotVMLongConstants     = VMStructs::localHotSpotVMLongConstants;
JNIEXPORT uint64_t gHotSpotVMLongConstantEntryNameOffset   = offsetof(VMLongConstantEntry, name);
JNIEXPORT uint64_t gHotSpotVMLongConstantEntryValueOffset  = offsetof(VMLongConstantEntry, value);
JNIEXPORT uint64_t gHotSpotVMLongConstantEntryArrayStride  = ((uint64_t) &gHotSpotVMLongConstants[1]) - ((uint64_t) &gHotSpotVMLongConstants[0]);
}

#ifdef ASSERT
int
VMStructs::findType(const char* typeName) {
  VMTypeEntry* types = gHotSpotVMTypes;

  while (types->typeName != NULL) {
    if (!strcmp(typeName, types->typeName)) {
      return 1;
    }
    ++types;
  }
  return 0;
}
#endif

void vmStructs_init() {
  debug_only(VMStructs::init());
}
