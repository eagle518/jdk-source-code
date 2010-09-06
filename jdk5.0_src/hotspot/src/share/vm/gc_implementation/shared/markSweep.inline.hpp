#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)markSweep.inline.hpp	1.8 03/12/23 16:40:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void MarkSweep::_adjust_pointer(oop* p, bool isroot) {
  oop obj = *p;
  VALIDATE_MARK_SWEEP_ONLY(oop saved_new_pointer = NULL);
  if (obj != NULL) {
    oop new_pointer = oop(obj->mark()->decode_pointer());
    assert(new_pointer != NULL ||                     // is forwarding ptr?
           obj->mark() == markOopDesc::prototype() || // not gc marked?
           obj->is_shared(),                          // never forwarded?
           "should contain a forwarding pointer");
    if (new_pointer != NULL) {
      *p = new_pointer;
      assert(Universe::heap()->is_in_reserved(new_pointer),
	     "should be in object space");
      VALIDATE_MARK_SWEEP_ONLY(saved_new_pointer = new_pointer);
    }
  }
  VALIDATE_MARK_SWEEP_ONLY(track_adjusted_pointer(p, saved_new_pointer, isroot));
}

inline void MarkSweep::mark_object(oop obj) {
  // some marks may contain information we need to preserve so we store them away
  // and overwrite the mark.  We'll restore it at the end of markSweep.
  markOop mark = obj->mark();
  obj->set_mark(markOopDesc::prototype()->set_marked());

  if (mark != markOopDesc::prototype()) {
    // we may need to preserve this mark
    if (mark->must_be_preserved()) {
      // the only marks we don't have to preserve are unlocked ones with no hash.
      // Anything else needs to be preserved
      preserve_mark(obj, mark);
    }
  }
}
