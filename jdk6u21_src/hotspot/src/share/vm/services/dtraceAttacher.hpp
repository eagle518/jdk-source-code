/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
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

#define DTRACE_ALLOC_PROBES    0x1
#define DTRACE_METHOD_PROBES   0x2
#define DTRACE_MONITOR_PROBES  0x4
#define DTRACE_ALL_PROBES      (DTRACE_ALLOC_PROBES  | \
                                DTRACE_METHOD_PROBES | \
                                DTRACE_MONITOR_PROBES)

class DTrace : public AllStatic {
 private:
  // disable one or more probes - OR above constants
  static void disable_dprobes(int probe_types);

 public:
  // enable one or more probes - OR above constants
  static void enable_dprobes(int probe_types);
  // all clients detached, do any clean-up
  static void detach_all_clients();
  // set ExtendedDTraceProbes flag
  static void set_extended_dprobes(bool value);
};
