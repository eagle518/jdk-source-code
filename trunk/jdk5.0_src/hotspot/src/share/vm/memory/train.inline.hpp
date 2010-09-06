#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)train.inline.hpp	1.7 03/12/23 16:41:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

julong Train::train_number() const {
  return first_car()->train_number();
}

// Length computations
bool Train::has_single_car() const {
  return first_car()->next_car() == NULL;
}

HeapWord* Train::allocate_within(size_t size) {
  HeapWord* result = last_car()->allocate(size);
  if (result != NULL) {
    _tg->inc_used_counter(size);
  }
  return result;
}
