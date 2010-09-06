#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Optimizer.hpp	1.10 03/12/23 16:39:16 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class Optimizer VALUE_OBJ_CLASS_SPEC {
 private:
  IR* _ir;
  
 public:
  Optimizer(IR* ir);
  IR* ir() const                                 { return _ir; }

  // optimizations
  void eliminate_conditional_expressions();
  void eliminate_blocks();
  void eliminate_null_checks();
};

