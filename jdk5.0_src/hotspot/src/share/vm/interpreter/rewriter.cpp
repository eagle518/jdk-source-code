#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)rewriter.cpp	1.37 03/12/23 16:40:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_rewriter.cpp.incl"


// Computes an index_map (new_index -> original_index) for contant pool entries
// that are referred to by the interpreter at runtime via the constant pool cache.
void Rewriter::compute_index_maps(constantPoolHandle pool, intArray*& index_map, intStack*& inverse_index_map) {
  const int length  = pool->length();
  index_map         = new intArray(length, -1);
  // Choose an initial value large enough that we don't get frequent
  // calls to grow().
  inverse_index_map = new intStack(length / 2);
  for (int i = 0; i < length; i++) {
    switch (pool->tag_at(i).value()) {
      case JVM_CONSTANT_Fieldref          : // fall through
      case JVM_CONSTANT_Methodref         : // fall through
      case JVM_CONSTANT_InterfaceMethodref: {
        index_map->at_put(i, inverse_index_map->length());
        inverse_index_map->append(i);
      }
    }
  }
}


// Creates a constant pool cache given an inverse_index_map
constantPoolCacheHandle Rewriter::new_constant_pool_cache(intArray& inverse_index_map, TRAPS) {
  const int length = inverse_index_map.length();
  constantPoolCacheOop cache = oopFactory::new_constantPoolCache(length, CHECK_(constantPoolCacheHandle()));
  cache->initialize(inverse_index_map);
  return constantPoolCacheHandle(THREAD, cache);
}


// Rewrites a method given the index_map information
methodHandle Rewriter::rewrite_method(methodHandle method, intArray& index_map, TRAPS) {  

  int nof_jsrs = 0;
  bool has_monitor_bytecodes = false;

  {
    // We cannot tolerate a GC in this block, because we've
    // cached the bytecodes in 'code_base'. If the methodOop
    // moves, the bytecodes will also move.
    No_Safepoint_Verifier nsv;
    Bytecodes::Code c;

    // Bytecodes and their length
    const address code_base = method->code_base();
    const int code_length = method->code_size();
  
    int bc_length;
    for (int bci = 0; bci < code_length; bci += bc_length) {
      address bcp = code_base + bci;
      c = (Bytecodes::Code)(*bcp);
      
      // Since we have the code, see if we can get the length
      // directly. Some more complicated bytecodes will report
      // a length of zero, meaning we need to make another method
      // call to calculate the length.
      bc_length = Bytecodes::length_for(c);
      if (bc_length == 0) {
        bc_length = Bytecodes::length_at(bcp);

        // length_at will put us at the bytecode after the one modified
        // by 'wide'. We don't currently examine any of the bytecodes
        // modified by wide, but in case we do in the future...
        if (c == Bytecodes::_wide) {
          c = (Bytecodes::Code)bcp[1];
        }
      }

      assert(bc_length != 0, "impossible bytecode length");

      switch (c) {
        case Bytecodes::_lookupswitch   : {
#ifndef CC_INTERP
          Bytecode_lookupswitch* bc = Bytecode_lookupswitch_at(bcp);        
          bc->set_code(
            bc->number_of_pairs() < BinarySwitchThreshold
            ? Bytecodes::_fast_linearswitch
            : Bytecodes::_fast_binaryswitch
          );        
#endif
          break;
        }
        case Bytecodes::_getstatic      : // fall through
        case Bytecodes::_putstatic      : // fall through
        case Bytecodes::_getfield       : // fall through
        case Bytecodes::_putfield       : // fall through
        case Bytecodes::_invokevirtual  : // fall through
        case Bytecodes::_invokespecial  : // fall through
        case Bytecodes::_invokestatic   : // fall through
        case Bytecodes::_invokeinterface: {
          address p = bcp + 1;
          Bytes::put_native_u2(p, index_map[Bytes::get_Java_u2(p)]);
          break;
        }
        case Bytecodes::_jsr            : // fall through
        case Bytecodes::_jsr_w          : nof_jsrs++;                   break;      
        case Bytecodes::_monitorenter   : // fall through
        case Bytecodes::_monitorexit    : has_monitor_bytecodes = true; break;
      }  
    }
  }
  
  // Update access flags
  if (has_monitor_bytecodes) {
    method->set_has_monitor_bytecodes();
  }

  // The present of a jsr bytecode implies that the method might potentially
  // have to be rewritten, so we run the oopMapGenerator on the method
  if (nof_jsrs > 0) {
    method->set_has_jsrs();
    ResolveOopMapConflicts romc(method);    
    methodHandle original_method = method;
    method = romc.do_potential_rewrite(CHECK_(methodHandle()));
    if (method() != original_method()) {
      // Insert invalid bytecode into original methodOop and set
      // interpreter entrypoint, so that a executing this method
      // will manifest itself in an easy recognizable form.
      address bcp = original_method->bcp_from(0);
      *bcp = (u1)Bytecodes::_shouldnotreachhere;
      int kind = AbstractInterpreter::method_kind(original_method);
      original_method->set_interpreter_kind(kind);
    }
       
    // Update monitor matching info.
    if (romc.monitor_safe()) {
      method->set_guaranteed_monitor_matching();
    }
  }  

  // Setup method entrypoints for compiler and interpreter
  methodOopDesc::link_method(method);
      
  return method;
}


void Rewriter::rewrite(instanceKlassHandle klass, TRAPS) {
  // gather starting points  
  ResourceMark rm(THREAD);
  constantPoolHandle pool (THREAD, klass->constants());
  objArrayHandle methods  (THREAD, klass->methods());
  assert(pool->cache() == NULL, "constant pool cache must not be set yet");

  // determine index maps for methodOop rewriting
  intArray* index_map         = NULL;
  intStack* inverse_index_map = NULL;
  compute_index_maps(pool, index_map, inverse_index_map);

  // allocate constant pool cache
  constantPoolCacheHandle cache = new_constant_pool_cache(*inverse_index_map, CHECK);
  pool->set_cache(cache());
  cache->set_constant_pool(pool());
  
  // rewrite methods
  { int i = methods->length();
    while (i-- > 0) {       
      methodHandle m(THREAD, (methodOop)methods->obj_at(i));
      m = rewrite_method(m, *index_map, CHECK);
      // Method might have gotten rewritten.
      methods->obj_at_put(i, m());      
    }
  }
}
