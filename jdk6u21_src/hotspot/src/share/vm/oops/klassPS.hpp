/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
#ifndef KLASS_PS_H
#define KLASS_PS_H

  // Expands to Parallel Scavenge and Parallel Old declarations

#ifndef SERIALGC
#define PARALLEL_GC_DECLS \
  virtual void oop_copy_contents(PSPromotionManager* pm, oop obj);          \
  virtual void oop_push_contents(PSPromotionManager* pm, oop obj);          \
  /* Parallel Old GC support                                                \
                                                                            \
   The 2-arg version of oop_update_pointers is for objects that are         \
   known not to cross chunk boundaries.  The 4-arg version is for           \
   objects that do (or may) cross chunk boundaries; it updates only those   \
   oops that are in the region [beg_addr, end_addr).  */                    \
  virtual void oop_follow_contents(ParCompactionManager* cm, oop obj);      \
  virtual int  oop_update_pointers(ParCompactionManager* cm, oop obj);      \
  virtual int  oop_update_pointers(ParCompactionManager* cm, oop obj,       \
                                   HeapWord* beg_addr, HeapWord* end_addr);

// Pure virtual version for klass.hpp
#define PARALLEL_GC_DECLS_PV \
  virtual void oop_copy_contents(PSPromotionManager* pm, oop obj) = 0;      \
  virtual void oop_push_contents(PSPromotionManager* pm, oop obj) = 0;      \
  virtual void oop_follow_contents(ParCompactionManager* cm, oop obj) = 0;  \
  virtual int  oop_update_pointers(ParCompactionManager* cm, oop obj) = 0;  \
  virtual int  oop_update_pointers(ParCompactionManager* cm, oop obj,       \
                                   HeapWord* beg_addr, HeapWord* end_addr) = 0;
#else  // SERIALGC
#define PARALLEL_GC_DECLS
#define PARALLEL_GC_DECLS_PV
#endif // SERIALGC

#endif // KLASS_PS_H
