#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)rewriter.hpp	1.13 03/12/23 16:40:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The Rewriter adds caches to the constant pool and rewrites bytecode indices
// pointing into the constant pool for better interpreter performance.

class Rewriter: public AllStatic {
 private:
  static void compute_index_maps(constantPoolHandle pool, intArray*& index_map, intStack*& inverse_index_map);
  static constantPoolCacheHandle new_constant_pool_cache(intArray& inverse_index_map, TRAPS);
  static methodHandle rewrite_method(methodHandle method, intArray& index_map, TRAPS);

 public:
  static void rewrite(instanceKlassHandle klass, TRAPS);
};

