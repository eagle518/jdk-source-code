#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CacheLocals_sparc.cpp	1.44 03/12/23 16:36:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_CacheLocals_sparc.cpp.incl"


//------------------caching locals---------------------

LocalMapping* LIR_LocalCaching::preferred_locals(const ciMethod* method) {
  CallingConvention* cc = FrameMap::calling_convention(method);
  RInfoCollection* mapping = new RInfoCollection();
  ciSignature* sig = method->signature();

  int index = 0;
  if (!method->is_static()) {
    mapping->at_put(index, cc->arg_at(index).incoming_reg_location());
    index++;
  }

  for (int i = 0; i < sig->count(); i++) {
    ArgumentLocation loc = cc->arg_at(index);
    BasicType type = sig->type_at(i)->basic_type();
    bool is_two_word = type == T_LONG || type == T_DOUBLE;

    if (loc.is_register_arg() && !is_two_word) {
      mapping->at_put(index, loc.incoming_reg_location());
    }
    index += (is_two_word) ? 2 : 1;
  }
  return new LocalMapping(ir()->local_name_to_offset_map(), mapping);
}
