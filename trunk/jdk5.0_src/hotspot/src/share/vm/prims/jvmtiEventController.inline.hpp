/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// these inline functions are in a separate file to break include cycles


///////////////////////////////////////////////////////////////
//
// JvmtiEventEnabled
//

inline jlong JvmtiEventEnabled::bit_for(jvmtiEvent event_type) {
  assert(JvmtiEventController::is_valid_event_type(event_type), "invalid event type");
  return ((jlong)1) << (event_type - TOTAL_MIN_EVENT_TYPE_VAL);  
}

inline jlong JvmtiEventEnabled::get_bits() {
  assert(_init_guard == JEE_INIT_GUARD, "enable bits uninitialized or corrupted");
  return _enabled_bits;
}

inline void JvmtiEventEnabled::set_bits(jlong bits) {
  assert(_init_guard == JEE_INIT_GUARD, "enable bits uninitialized or corrupted on set");
  _enabled_bits = bits;
}

inline bool JvmtiEventEnabled::is_enabled(jvmtiEvent event_type) {
  return (bit_for(event_type) & get_bits()) != 0;
}


///////////////////////////////////////////////////////////////
//
// JvmtiEnvThreadEventEnable
//

inline bool JvmtiEnvThreadEventEnable::is_enabled(jvmtiEvent event_type) { 
  assert(JvmtiUtil::event_threaded(event_type), "Only thread filtered events should be tested here");
  return _event_enabled.is_enabled(event_type); 
}

inline void JvmtiEnvThreadEventEnable::set_user_enabled(jvmtiEvent event_type, bool enabled) { 
  _event_user_enabled.set_enabled(event_type, enabled);  
}


///////////////////////////////////////////////////////////////
//
// JvmtiThreadEventEnable
//

inline bool JvmtiThreadEventEnable::is_enabled(jvmtiEvent event_type) { 
  assert(JvmtiUtil::event_threaded(event_type), "Only thread filtered events should be tested here");
  return _event_enabled.is_enabled(event_type); 
}


///////////////////////////////////////////////////////////////
//
// JvmtiEnvEventEnable
//

inline bool JvmtiEnvEventEnable::is_enabled(jvmtiEvent event_type) { 
  assert(!JvmtiUtil::event_threaded(event_type), "Only non thread filtered events should be tested here");
  return _event_enabled.is_enabled(event_type);
}

inline void JvmtiEnvEventEnable::set_user_enabled(jvmtiEvent event_type, bool enabled) { 
  _event_user_enabled.set_enabled(event_type, enabled);  
}

 
///////////////////////////////////////////////////////////////
//
// JvmtiEventController
//

bool JvmtiEventController::is_enabled(jvmtiEvent event_type) {
  return _universal_global_event_enabled.is_enabled(event_type);
}

