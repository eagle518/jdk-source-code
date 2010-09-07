/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
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

  static int pd_instruction_alignment() {
    return sizeof(int);
  }

  static const char* pd_cpu_opts() {
    return (VM_Version::v9_instructions_work()?
            (VM_Version::v8_instructions_work()? "" : "v9only") : "v8only");
  }
