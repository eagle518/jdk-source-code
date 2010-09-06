#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC_sparc.hpp	1.7 03/12/23 16:37:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  address _npc;

 public:
  address npc() { return _npc; }
  ExtendedPC(address pc, address npc) {
    _pc  = pc;
    _npc = npc;
  }
  ExtendedPC() {
    _pc  = NULL;
    _npc = NULL;
  }
  static bool is_contained_in(address pc, address begin, address end);
