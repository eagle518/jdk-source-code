#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)forte.hpp	1.1 04/04/05 13:15:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface to Forte support.

class Forte : AllStatic {
 public:
   static void register_stub(const char* name, address start, address end);    
                                                 // register internal VM stub
};
