#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmStructs_ia64.hpp	1.15 03/12/23 16:36:52 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// These are the CPU-specific fields, types and integer
// constants required by the Serviceability Agent. This file is
// referenced by vmStructs.cpp.

#define VM_STRUCTS_CPU(nonstatic_field, static_field, unchecked_nonstatic_field, volatile_nonstatic_field, nonproduct_nonstatic_field, nonproduct_noncore_nonstatic_field, c2_nonstatic_field, noncore_nonstatic_field, volatile_noncore_nonstatic_field, noncore_static_field, unchecked_c1_static_field, unchecked_c2_static_field, last_entry)            \
                                                                   \
  /******************************/                                 \
  /* JavaCallWrapper            */                                 \
  /******************************/                                 \
                                                                   \
  /******************************/                                 \
  /* JavaThread                 */                                 \
  /******************************/                                 \
  /* cInterpreter            */                                                                \
  /******************************/                                                             \
  nonstatic_field(cInterpreter,             _thread,                  JavaThread*)             \
  nonstatic_field(cInterpreter,             _bcp,                     address)                 \
  nonstatic_field(cInterpreter,             _locals,                  intptr_t*)               \
  nonstatic_field(cInterpreter,             _constants,               constantPoolCacheOop)    \
  nonstatic_field(cInterpreter,             _method,                  methodOop)               \
  nonstatic_field(cInterpreter,             _stack,                   intptr_t*)               \
  nonstatic_field(cInterpreter,             _msg,                     cInterpreter::messages)  \
  nonstatic_field(cInterpreter,             _stack_base,              intptr_t*)               \
  nonstatic_field(cInterpreter,             _stack_limit,             intptr_t*)               \
  nonstatic_field(cInterpreter,             _monitor_base,            BasicObjectLock*)        \
  nonstatic_field(cInterpreter,             _prev_link,               cInterpreter*)           

  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_STRUCTS_OS_CPU macro (and must  */
  /* be present there)                                                */


#define VM_TYPES_CPU(declare_type, declare_toplevel_type, declare_oop_type, declare_integer_type, declare_unsigned_integer_type, declare_noncore_type, declare_noncore_toplevel_type, declare_noncore_oop_type, declare_noncore_integer_type, declare_noncore_unsigned_integer_type, declare_c1_toplevel_type, declare_c2_type, declare_c2_toplevel_type, last_entry)                               \
        declare_toplevel_type(cInterpreter)                         \
        declare_toplevel_type(cInterpreter*)                        \
        declare_toplevel_type(BasicObjectLock*)                     \
        declare_toplevel_type(JavaCallWrapper*)                     \
        declare_toplevel_type(cInterpreter::messages)                         \
        declare_toplevel_type(interpreterState)
                                                                                                                                     
  /* NOTE that we do not use the last_entry() macro here; it is used  */
  /* in vmStructs_<os>_<cpu>.hpp's VM_TYPES_OS_CPU macro (and must    */
  /* be present there)                                                */


#define VM_INT_CONSTANTS_CPU(declare_constant, declare_preprocessor_constant, declare_noncore_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry)                                                              \
                                                                        
  /* NOTE that we do not use the last_entry() macro here; it is used        */
  /* in vmStructs_<os>_<cpu>.hpp's VM_INT_CONSTANTS_OS_CPU macro (and must  */
  /* be present there)                                                      */

#define VM_LONG_CONSTANTS_CPU(declare_constant, declare_preprocessor_constant, declare_c1_constant, declare_c2_constant, declare_c2_preprocessor_constant, last_entry)                                                              \
  declare_constant(cInterpreter::no_request)                                 \
  declare_constant(cInterpreter::initialize)                                 \
  declare_constant(cInterpreter::method_entry)                               \
  declare_constant(cInterpreter::method_resume)                              \
  declare_constant(cInterpreter::got_monitors)                               \
  declare_constant(cInterpreter::rethrow_exception)                          \
  declare_constant(cInterpreter::call_method)                                \
  declare_constant(cInterpreter::return_from_method)                         \
  declare_constant(cInterpreter::retry_method)                               \
  declare_constant(cInterpreter::more_monitors)                              \
  declare_constant(cInterpreter::throwing_exception)                         \
  declare_constant(cInterpreter::popping_frame)

  /* NOTE that we do not use the last_entry() macro here; it is used         */
  /* in vmStructs_<os>_<cpu>.hpp's VM_LONG_CONSTANTS_OS_CPU macro (and must  */
  /* be present there)                                                       */
