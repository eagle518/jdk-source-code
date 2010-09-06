#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)serialize.cpp	1.2 03/12/23 16:41:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_serialize.cpp.incl"


// Serialize out the block offset shared array for the shared spaces.

void CompactingPermGenGen::serialize_bts(SerializeOopClosure* soc) {
  _ro_bts->serialize(soc, readonly_bottom, readonly_end);
  _rw_bts->serialize(soc, readwrite_bottom, readwrite_end);
}


// Read/write a data stream for restoring/preserving oop pointers and
// miscellaneous data from/to the shared archive file.

void CompactingPermGenGen::serialize_oops(SerializeOopClosure* soc) {
  int tag = 0;
  soc->do_tag(--tag);

  // Verify the sizes of various oops in the system.
  soc->do_tag(sizeof(oopDesc));
  soc->do_tag(sizeof(instanceOopDesc));
  soc->do_tag(sizeof(methodOopDesc));
  soc->do_tag(sizeof(constMethodOopDesc));
#ifdef CORE
  soc->do_tag(-1);
#else
  soc->do_tag(sizeof(methodDataOopDesc));
#endif // !CORE
  soc->do_tag(sizeof(arrayOopDesc));
  soc->do_tag(sizeof(constantPoolOopDesc));
  soc->do_tag(sizeof(constantPoolCacheOopDesc));
  soc->do_tag(sizeof(objArrayOopDesc));
  soc->do_tag(sizeof(typeArrayOopDesc));
  soc->do_tag(sizeof(symbolOopDesc));
  soc->do_tag(sizeof(klassOopDesc));
  soc->do_tag(sizeof(markOopDesc));
  NOT_CORE(soc->do_tag(sizeof(compiledICHolderOopDesc));)

  // Dump the block offset table entries.
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  CompactingPermGenGen* pg = (CompactingPermGenGen*)gch->perm_gen();
  pg->serialize_bts(soc);
  soc->do_tag(--tag);
  pg->ro_space()->serialize_block_offset_array_offsets(soc);
  soc->do_tag(--tag);
  pg->rw_space()->serialize_block_offset_array_offsets(soc);
  soc->do_tag(--tag);

  // Special case - this oop needed in oop->is_oop() assertions.
  soc->do_ptr((void**)&Universe::_klassKlassObj);
  soc->do_tag(--tag);

  // Dump/restore miscellaneous oops.
  Universe::oops_do(soc, true);
  soc->do_tag(--tag);

  vmSymbols::oops_do(soc, true);               soc->do_tag(--tag);
  NOT_CORE(CodeCache::oops_do(soc));           soc->do_tag(--tag);
  soc->do_tag(666);
}
