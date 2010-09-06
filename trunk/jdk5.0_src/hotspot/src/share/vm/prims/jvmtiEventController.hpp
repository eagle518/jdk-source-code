#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiEventController.hpp	1.13 04/02/15 12:35:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMTI_EVENT_CONTROLLER_H_
#define _JAVA_JVMTI_EVENT_CONTROLLER_H_

// forward declaration
class JvmtiEventControllerPrivate;
class JvmtiEventController;
class JvmtiEnvThreadState;
class JvmtiFramePop;
class JvmtiEnvBase;


// Extension event support
//
// jvmtiExtEvent is the extensions equivalent of jvmtiEvent
// jvmtiExtCallbacks is the extensions equivalent of jvmtiEventCallbacks

// Extension events start JVMTI_MIN_EVENT_TYPE_VAL-1 and work towards 0.
typedef enum {
  EXT_EVENT_CLASS_UNLOAD = JVMTI_MIN_EVENT_TYPE_VAL-1,
  EXT_MIN_EVENT_TYPE_VAL = EXT_EVENT_CLASS_UNLOAD,
  EXT_MAX_EVENT_TYPE_VAL = EXT_EVENT_CLASS_UNLOAD
} jvmtiExtEvent;

typedef struct {
  jvmtiExtensionEvent ClassUnload;
} jvmtiExtEventCallbacks;


// The complete range of events is EXT_MIN_EVENT_TYPE_VAL to 
// JVMTI_MAX_EVENT_TYPE_VAL (inclusive and contiguous).
enum {
  TOTAL_MIN_EVENT_TYPE_VAL = EXT_MIN_EVENT_TYPE_VAL,
  TOTAL_MAX_EVENT_TYPE_VAL = JVMTI_MAX_EVENT_TYPE_VAL
};


///////////////////////////////////////////////////////////////
//
// JvmtiEventEnabled
//
// Utility class
//
// A boolean array indexed by event_type, used as an internal
// data structure to track what JVMTI event types are enabled.
// Used for user set enabling and disabling (globally and on a 
// per thread basis), and for computed merges across environments,
// threads and the VM as a whole.
//
// for inlines see jvmtiEventController_inline.hpp
//

class JvmtiEventEnabled VALUE_OBJ_CLASS_SPEC {
private:
  friend class JvmtiEventControllerPrivate;
  jlong _enabled_bits;
#ifndef PRODUCT
  enum {
    JEE_INIT_GUARD = 0xEAD0
  } _init_guard;
#endif
  static inline jlong bit_for(jvmtiEvent event_type);
  inline jlong get_bits();
  inline void set_bits(jlong bits);
public:
  JvmtiEventEnabled();
  void clear();
  inline bool is_enabled(jvmtiEvent event_type);
  void set_enabled(jvmtiEvent event_type, bool enabled);
};


///////////////////////////////////////////////////////////////
//
// JvmtiEnvThreadEventEnable
//
// JvmtiEventController data specific to a particular environment and thread.
//
// for inlines see jvmtiEventController_inline.hpp
//

class JvmtiEnvThreadEventEnable VALUE_OBJ_CLASS_SPEC {
private:
  friend class JvmtiEventControllerPrivate;
  JvmtiEventEnabled _event_user_enabled;
  JvmtiEventEnabled _event_enabled;

public:
  JvmtiEnvThreadEventEnable();
  ~JvmtiEnvThreadEventEnable();
  inline bool is_enabled(jvmtiEvent event_type);
  inline void set_user_enabled(jvmtiEvent event_type, bool enabled);
};


///////////////////////////////////////////////////////////////
//
// JvmtiThreadEventEnable
//
// JvmtiEventController data specific to a particular thread.
//
// for inlines see jvmtiEventController_inline.hpp
//

class JvmtiThreadEventEnable VALUE_OBJ_CLASS_SPEC {
private:
  friend class JvmtiEventControllerPrivate;
  JvmtiEventEnabled _event_enabled;

public:
  JvmtiThreadEventEnable();
  ~JvmtiThreadEventEnable();
  inline bool is_enabled(jvmtiEvent event_type);
};


///////////////////////////////////////////////////////////////
//
// JvmtiEnvEventEnable
//
// JvmtiEventController data specific to a particular environment.
//
// for inlines see jvmtiEventController_inline.hpp
//

class JvmtiEnvEventEnable VALUE_OBJ_CLASS_SPEC {
private:
  friend class JvmtiEventControllerPrivate;

  // user set global event enablement indexed by jvmtiEvent  
  JvmtiEventEnabled _event_user_enabled;

  // this flag indicates the presence (true) or absence (false) of event callbacks
  // it is indexed by jvmtiEvent  
  JvmtiEventEnabled _event_callback_enabled;

  // indexed by jvmtiEvent true if enabled globally or on any thread.
  // True only if there is a callback for it.
  JvmtiEventEnabled _event_enabled;

public:
  JvmtiEnvEventEnable();
  ~JvmtiEnvEventEnable();
  inline bool is_enabled(jvmtiEvent event_type);
  inline void set_user_enabled(jvmtiEvent event_type, bool enabled);
};


///////////////////////////////////////////////////////////////
//
// JvmtiEventController
//
// The class is the access point for all actions that change 
// which events are active, this include:
//      enabling and disabling events
//      changing the callbacks/eventhook (they may be null)
//      setting and clearing field watchpoints
//      setting frame pops
//      encountering frame pops
//
// for inlines see jvmtiEventController_inline.hpp
//

class JvmtiEventController : AllStatic {
private:
  friend class JvmtiEventControllerPrivate;

  // for all environments, global array indexed by jvmtiEvent 
  static JvmtiEventEnabled _universal_global_event_enabled;

public:
  static inline bool is_enabled(jvmtiEvent event_type);

  // events that can ONLY be enabled/disabled globally (can't toggle on individual threads).
  static bool is_global_event(jvmtiEvent event_type);

  // is the event_type valid?
  // to do: check against valid event array
  static inline bool is_valid_event_type(jvmtiEvent event_type) {
    return (event_type >= TOTAL_MIN_EVENT_TYPE_VAL) && (event_type <= TOTAL_MAX_EVENT_TYPE_VAL);
  }

  // Use (thread == NULL) to enable/disable an event globally.
  // Use (thread != NULL) to enable/disable an event for a particular thread.
  // thread is ignored for events that can only be specified globally
  static void set_user_enabled(JvmtiEnvBase *env, JavaThread *thread, 
                               jvmtiEvent event_type, bool enabled);

  // Setting callbacks changes computed enablement and must be done
  // at a safepoint otherwise a NULL callback could be attempted
  static void set_event_callbacks(JvmtiEnvBase *env,  
                                  const jvmtiEventCallbacks* callbacks, 
                                  jint size_of_callbacks);

  // Sets the callback function for a single extension event and enables 
  // (or disables it).
  static void set_extension_event_callback(JvmtiEnvBase* env,
					   jint extension_event_index, 
					   jvmtiExtensionEvent callback);

  static void set_frame_pop(JvmtiEnvThreadState *env_thread, JvmtiFramePop fpop);
  static void clear_frame_pop(JvmtiEnvThreadState *env_thread, JvmtiFramePop fpop);
  static void clear_to_frame_pop(JvmtiEnvThreadState *env_thread, JvmtiFramePop fpop);

  static void change_field_watch(jvmtiEvent event_type, bool added);

  static void thread_started(JavaThread *thread);
  static void thread_ended(JavaThread *thread);

  static void env_initialize(JvmtiEnvBase *env);
  static void env_dispose(JvmtiEnvBase *env);

  static void vm_start();
  static void vm_init();
  static void vm_death();
};

#endif   /* _JAVA_JVMTI_EVENT_CONTROLLER_H_ */
