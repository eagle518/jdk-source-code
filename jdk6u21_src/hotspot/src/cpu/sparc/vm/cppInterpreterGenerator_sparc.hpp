/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

  static address frame_manager_return;
  static address frame_manager_sync_return;


  void generate_more_monitors();
  void generate_deopt_handling();
  void adjust_callers_stack(Register args);
  void generate_compute_interpreter_state(const Register state,
                                          const Register prev_state,
                                          bool native);
