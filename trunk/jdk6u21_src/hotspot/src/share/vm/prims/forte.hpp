/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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

// Interface to Forte support.

class Forte : AllStatic {
 public:
   static void register_stub(const char* name, address start, address end)
                                                 KERNEL_RETURN;
                                                 // register internal VM stub
};
