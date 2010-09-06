#ifdef USE_PRAGMA_IDENT_SCR
#pragma ident "@(#)jvmtiManageCapabilities.cpp	1.22 04/03/23 23:31:14 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */
# include "incls/_precompiled.incl"
# include "incls/_jvmtiManageCapabilities.cpp.incl"

static const jint CAPA_SIZE = (JVMTI_INTERNAL_CAPABILITY_COUNT + 7) / 8; 

  // capabilities which are always potentially available 
jvmtiCapabilities JvmtiManageCapabilities::always_capabilities;

  // capabilities which are potentially available during OnLoad
jvmtiCapabilities JvmtiManageCapabilities::onload_capabilities;

  // capabilities which are always potentially available 
  // but to only one environment
jvmtiCapabilities JvmtiManageCapabilities::always_solo_capabilities;

  // capabilities which are potentially available during OnLoad
  // but to only one environment
jvmtiCapabilities JvmtiManageCapabilities::onload_solo_capabilities;

  // remaining capabilities which are always potentially available 
  // but to only one environment
jvmtiCapabilities JvmtiManageCapabilities::always_solo_remaining_capabilities;

  // remaining capabilities which are potentially available during OnLoad
  // but to only one environment
jvmtiCapabilities JvmtiManageCapabilities::onload_solo_remaining_capabilities;

  // all capabilities ever acquired
jvmtiCapabilities JvmtiManageCapabilities::acquired_capabilities;

void JvmtiManageCapabilities::initialize() {
  always_capabilities = init_always_capabilities();
  onload_capabilities = init_onload_capabilities();
  always_solo_capabilities = init_always_solo_capabilities();
  onload_solo_capabilities = init_onload_solo_capabilities();
  always_solo_remaining_capabilities = init_always_solo_capabilities();
  onload_solo_remaining_capabilities = init_onload_solo_capabilities();
  memset(&acquired_capabilities, 0, sizeof(acquired_capabilities));
}


// corresponding init functions
jvmtiCapabilities JvmtiManageCapabilities::init_always_capabilities() { 
  jvmtiCapabilities jc;
  
  memset(&jc, 0, sizeof(jc));
  jc.can_get_bytecodes = 1;
  jc.can_signal_thread = 1;
  jc.can_get_source_file_name = 1;
  jc.can_get_line_numbers = 1;
  jc.can_get_synthetic_attribute = 1;
  jc.can_get_monitor_info = 1;
  jc.can_generate_monitor_events = 1;
  jc.can_generate_garbage_collection_events = 1;
  jc.can_generate_compiled_method_load_events = 1;
  jc.can_generate_native_method_bind_events = 1;
  jc.can_generate_vm_object_alloc_events = 1;
  if (os::is_thread_cpu_time_supported()) {
    jc.can_get_current_thread_cpu_time = 1;
    jc.can_get_thread_cpu_time = 1;
  }
  return jc;
} 

jvmtiCapabilities JvmtiManageCapabilities::init_onload_capabilities() {
  jvmtiCapabilities jc;
  
  memset(&jc, 0, sizeof(jc));
  jc.can_pop_frame = 1;
  jc.can_redefine_classes = 1;
  jc.can_get_source_debug_extension = 1;
  jc.can_access_local_variables = 1;
  jc.can_maintain_original_method_order = 1;
  jc.can_redefine_any_class = 1;
  jc.can_generate_all_class_hook_events = 1;
  jc.can_generate_single_step_events = 1;
  jc.can_generate_exception_events = 1;
  jc.can_generate_frame_pop_events = 1;
  jc.can_generate_method_entry_events = 1;
  jc.can_generate_method_exit_events = 1;
  jc.can_get_owned_monitor_info = 1;
  jc.can_get_current_contended_monitor = 1;
  // jc.can_get_monitor_info = 1;

  if (!UseConcMarkSweepGC) {
    jc.can_tag_objects = 1;
    jc.can_generate_object_free_events = 1;
  }
  return jc;
}


jvmtiCapabilities JvmtiManageCapabilities::init_always_solo_capabilities() {
  jvmtiCapabilities jc;
  
  memset(&jc, 0, sizeof(jc));
  jc.can_suspend = 1;
  return jc;
}


jvmtiCapabilities JvmtiManageCapabilities::init_onload_solo_capabilities() {
  jvmtiCapabilities jc;
  
  memset(&jc, 0, sizeof(jc));
  jc.can_generate_field_modification_events = 1;
  jc.can_generate_field_access_events = 1;
  jc.can_generate_breakpoint_events = 1;
  return jc;
}


jvmtiCapabilities *JvmtiManageCapabilities::either(const jvmtiCapabilities *a, const jvmtiCapabilities *b,
                                                   jvmtiCapabilities *result) {
  char *ap = (char *)a;
  char *bp = (char *)b;
  char *resultp = (char *)result;

  for (int i = 0; i < CAPA_SIZE; ++i) {
    *resultp++ = *ap++ | *bp++;
  }

  return result;
}


jvmtiCapabilities *JvmtiManageCapabilities::both(const jvmtiCapabilities *a, const jvmtiCapabilities *b, 
                                                    jvmtiCapabilities *result) {
  char *ap = (char *)a;
  char *bp = (char *)b;
  char *resultp = (char *)result;

  for (int i = 0; i < CAPA_SIZE; ++i) {
    *resultp++ = *ap++ & *bp++;
  }

  return result;
}


jvmtiCapabilities *JvmtiManageCapabilities::exclude(const jvmtiCapabilities *a, const jvmtiCapabilities *b, 
                                                    jvmtiCapabilities *result) {
  char *ap = (char *)a;
  char *bp = (char *)b;
  char *resultp = (char *)result;

  for (int i = 0; i < CAPA_SIZE; ++i) {
    *resultp++ = *ap++ & ~*bp++;
  }

  return result;
}


bool JvmtiManageCapabilities::has_some(const jvmtiCapabilities *a) {
  char *ap = (char *)a;

  for (int i = 0; i < CAPA_SIZE; ++i) {
    if (*ap++ != 0) {
      return true;
    }
  }

  return false;
}


void JvmtiManageCapabilities::copy_capabilities(const jvmtiCapabilities *from, jvmtiCapabilities *to) {
  char *ap = (char *)from;
  char *resultp = (char *)to;

  for (int i = 0; i < CAPA_SIZE; ++i) {
    *resultp++ = *ap++;
  }
}


void JvmtiManageCapabilities::get_potential_capabilities(const jvmtiCapabilities *current, jvmtiCapabilities *result) {
  // must include current since it may possess solo capabilities
  either(current, &always_capabilities, result);

  // add other remaining 
  either(result, &always_solo_remaining_capabilities, result);

  // if this is during OnLoad more capabilities are available
  if (JvmtiEnv::get_phase() == JVMTI_PHASE_ONLOAD) {
    either(result, &onload_capabilities, result);
    either(result, &onload_solo_remaining_capabilities, result);
  }
}

jvmtiError JvmtiManageCapabilities::add_capabilities(const jvmtiCapabilities *current, 
                                                     const jvmtiCapabilities *desired, 
                                                     jvmtiCapabilities *result) {
  // check that the capabilities being added are potential capabilities
  jvmtiCapabilities temp;
  get_potential_capabilities(current, &temp);
  if (has_some(exclude(desired, &temp, &temp))) {
    return JVMTI_ERROR_NOT_AVAILABLE;
  }

  // add to the set of ever acquired capabilities
  either(&acquired_capabilities, desired, &acquired_capabilities);

  // onload capabilities that got added are now permanent - so, also remove from onload
  both(&onload_capabilities, desired, &temp);
  either(&always_capabilities, &temp, &always_capabilities);
  exclude(&onload_capabilities, &temp, &onload_capabilities);

  // same for solo capabilities (transferred capabilities in the remaining sets handled as part of standard grab - below)
  both(&onload_solo_capabilities, desired, &temp);
  either(&always_solo_capabilities, &temp, &always_solo_capabilities);
  exclude(&onload_solo_capabilities, &temp, &onload_solo_capabilities);

  // remove solo capabilities that are now taken
  exclude(&always_solo_remaining_capabilities, desired, &always_solo_remaining_capabilities);
  exclude(&onload_solo_remaining_capabilities, desired, &onload_solo_remaining_capabilities);

  // return the result
  either(current, desired, result);

  update();

  return JVMTI_ERROR_NONE;
}


void JvmtiManageCapabilities::relinquish_capabilities(const jvmtiCapabilities *current,
                                                      const jvmtiCapabilities *unwanted, 
                                                      jvmtiCapabilities *result) {
  jvmtiCapabilities to_trash;
  jvmtiCapabilities temp;

  // can't give up what you don't have
  both(current, unwanted, &to_trash);

  // restore solo capabilities but only those that belong
  either(&always_solo_remaining_capabilities, both(&always_solo_capabilities, &to_trash, &temp), 
         &always_solo_remaining_capabilities);
  either(&onload_solo_remaining_capabilities, both(&onload_solo_capabilities, &to_trash, &temp),
         &onload_solo_remaining_capabilities);

  update();

  // return the result
  exclude(current, unwanted, result);
}


jvmtiError JvmtiManageCapabilities::estimate_cost_of_capabilities(const jvmtiCapabilities *current,
                                                                  const jvmtiCapabilities *desired,
                                                                  jint* time_impact_ptr, 
                                                                  jint* space_impact_ptr) {
  // check that the capabilities being estimated are potential capabilities
  jvmtiCapabilities temp;
  get_potential_capabilities(current, &temp);
  if (has_some(exclude(desired, &temp, &temp))) {
    return JVMTI_ERROR_NOT_AVAILABLE;
  }

  // FIXLATER - just punt for now
  *time_impact_ptr = 0;
  *space_impact_ptr = 0;
  return JVMTI_ERROR_NONE;
}

void JvmtiManageCapabilities::update() {
  jvmtiCapabilities avail;

  // all capabilities
  either(&always_capabilities, &always_solo_capabilities, &avail);

  bool interp_events = 
    avail.can_generate_field_access_events ||
    avail.can_generate_field_modification_events ||
    avail.can_generate_single_step_events ||
    avail.can_generate_frame_pop_events ||
    avail.can_generate_method_entry_events ||
    avail.can_generate_method_exit_events;
  bool enter_all_methods = 
    interp_events ||
    avail.can_generate_breakpoint_events;
  UseFastEmptyMethods = !enter_all_methods;
  UseFastAccessorMethods = !enter_all_methods;

  if (avail.can_generate_breakpoint_events) {
    RewriteFrequentPairs = false;
  }

  JvmtiExport::set_can_get_source_debug_extension(avail.can_get_source_debug_extension);
  JvmtiExport::set_can_examine_or_deopt_anywhere(
    avail.can_generate_breakpoint_events ||
    interp_events || 
    avail.can_redefine_classes ||
    avail.can_access_local_variables ||
    avail.can_get_owned_monitor_info ||
    avail.can_get_current_contended_monitor ||
    avail.can_get_monitor_info);
  JvmtiExport::set_can_maintain_original_method_order(avail.can_maintain_original_method_order);
  JvmtiExport::set_can_post_interpreter_events(interp_events);
  JvmtiExport::set_can_hotswap_or_post_breakpoint(
    avail.can_generate_breakpoint_events ||
    avail.can_redefine_classes);
  JvmtiExport::set_can_modify_any_class(
    avail.can_generate_breakpoint_events ||
    avail.can_redefine_classes ||  // FIXME: remove when there is support for redefine with class sharing
    avail.can_redefine_any_class ||
    avail.can_generate_all_class_hook_events);
  JvmtiExport::set_can_walk_any_space(
    avail.can_tag_objects);  // FIXME: remove when IterateOverReachableObjects supports class sharing
  JvmtiExport::set_can_access_local_variables(
    avail.can_access_local_variables  ||
    avail.can_redefine_classes);
  JvmtiExport::set_can_post_exceptions(
    avail.can_generate_exception_events ||
    avail.can_generate_frame_pop_events ||
    avail.can_generate_method_exit_events);
  JvmtiExport::set_can_post_breakpoint(avail.can_generate_breakpoint_events);
  JvmtiExport::set_can_post_field_access(avail.can_generate_field_access_events);
  JvmtiExport::set_can_post_field_modification(avail.can_generate_field_modification_events);
  JvmtiExport::set_can_post_method_entry(avail.can_generate_method_entry_events);
  JvmtiExport::set_can_post_method_exit(avail.can_generate_method_exit_events ||
                                        avail.can_generate_frame_pop_events);
  JvmtiExport::set_can_pop_frame(avail.can_pop_frame);
  JvmtiExport::set_should_clean_up_heap_objects(avail.can_generate_breakpoint_events);
}

