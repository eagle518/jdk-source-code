/*
 * Copyright (c) 2006, 2007, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_dtraceAttacher.cpp.incl"

#ifdef SOLARIS

class VM_DeoptimizeTheWorld : public VM_Operation {
 public:
  VMOp_Type type() const {
    return VMOp_DeoptimizeTheWorld;
  }
  void doit() {
    CodeCache::mark_all_nmethods_for_deoptimization();
    ResourceMark rm;
    DeoptimizationMarker dm;
    // Deoptimize all activations depending on marked methods
    Deoptimization::deoptimize_dependents();

    // Mark the dependent methods non entrant
    CodeCache::make_marked_nmethods_not_entrant();
  }
};

static void set_bool_flag(const char* flag, bool value) {
  CommandLineFlags::boolAtPut((char*)flag, strlen(flag), &value,
                              ATTACH_ON_DEMAND);
}

// Enable only the "fine grained" flags. Do *not* touch
// the overall "ExtendedDTraceProbes" flag.
void DTrace::enable_dprobes(int probes) {
  bool changed = false;
  if (!DTraceAllocProbes && (probes & DTRACE_ALLOC_PROBES)) {
    set_bool_flag("DTraceAllocProbes", true);
    changed = true;
  }
  if (!DTraceMethodProbes && (probes & DTRACE_METHOD_PROBES)) {
    set_bool_flag("DTraceMethodProbes", true);
    changed = true;
  }
  if (!DTraceMonitorProbes && (probes & DTRACE_MONITOR_PROBES)) {
    set_bool_flag("DTraceMonitorProbes", true);
    changed = true;
  }

  if (changed) {
    // one or more flags changed, need to deoptimize
    VM_DeoptimizeTheWorld op;
    VMThread::execute(&op);
  }
}

// Disable only the "fine grained" flags. Do *not* touch
// the overall "ExtendedDTraceProbes" flag.
void DTrace::disable_dprobes(int probes) {
  bool changed = false;
  if (DTraceAllocProbes && (probes & DTRACE_ALLOC_PROBES)) {
    set_bool_flag("DTraceAllocProbes", false);
    changed = true;
  }
  if (DTraceMethodProbes && (probes & DTRACE_METHOD_PROBES)) {
    set_bool_flag("DTraceMethodProbes", false);
    changed = true;
  }
  if (DTraceMonitorProbes && (probes & DTRACE_MONITOR_PROBES)) {
    set_bool_flag("DTraceMonitorProbes", false);
    changed = true;
  }
  if (changed) {
    // one or more flags changed, need to deoptimize
    VM_DeoptimizeTheWorld op;
    VMThread::execute(&op);
  }
}

// Do clean-up on "all door clients detached" event.
void DTrace::detach_all_clients() {
  /*
   * We restore the state of the fine grained flags
   * to be consistent with overall ExtendedDTraceProbes.
   * This way, we will honour command line setting or the
   * last explicit modification of ExtendedDTraceProbes by
   * a call to set_extended_dprobes.
   */
  if (ExtendedDTraceProbes) {
    enable_dprobes(DTRACE_ALL_PROBES);
  } else {
    disable_dprobes(DTRACE_ALL_PROBES);
  }
}

void DTrace::set_extended_dprobes(bool flag) {
  // explicit setting of ExtendedDTraceProbes flag
  set_bool_flag("ExtendedDTraceProbes", flag);

  // make sure that the fine grained flags reflect the change.
  if (flag) {
    enable_dprobes(DTRACE_ALL_PROBES);
  } else {
    /*
     * FIXME: Revisit this: currently all-client-detach detection
     * does not work and hence disabled. The following scheme does
     * not work. So, we have to disable fine-grained flags here.
     *
     * disable_dprobes call has to be delayed till next "detach all "event.
     * This is to be  done so that concurrent DTrace clients that may
     * have enabled one or more fine grained dprobes and may be running
     * still. On "detach all" clients event, we would sync ExtendedDTraceProbes
     * with  fine grained flags which would take care of disabling fine grained flags.
     */
    disable_dprobes(DTRACE_ALL_PROBES);
  }
}

#endif /* SOLARIS */
