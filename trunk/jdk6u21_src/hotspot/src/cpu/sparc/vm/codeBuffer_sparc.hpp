/*
 * Copyright (c) 2002, 2005, Oracle and/or its affiliates. All rights reserved.
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

private:
  void pd_initialize() {}

public:
  void flush_bundle(bool start_new_bundle) {}

  // Heuristic for pre-packing the pt/pn bit of a predicted branch.
  bool is_backward_branch(Label& L) {
    return L.is_bound() && code_end() <= locator_address(L.loc());
  }
