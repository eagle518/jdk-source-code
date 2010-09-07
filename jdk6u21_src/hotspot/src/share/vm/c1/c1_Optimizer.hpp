/*
 * Copyright (c) 1999, 2001, Oracle and/or its affiliates. All rights reserved.
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
