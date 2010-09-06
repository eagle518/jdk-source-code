#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdiEventFromJvmtiEvent.hpp	1.5 03/12/23 16:43:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMTITOJVMDIEVENT_H_
#define _JAVA_JVMTITOJVMDIEVENT_H_

///////////////////////////////////////////////////////////////
// 
// JvmdiEventFromJvmtiEvent
//
// Convert a JVMDI event hook into JVMTI callback structure, with
// callbacks set for each JVMDI event.  When these callbacks are
// invoked they will generate a JVMDI event structure and call 
// the hook. 
// As a result, outside of this class, the rest of the JVMTI 
// implementation need not worry about event hooks or JVMDI event
// structures.
//

class JvmdiEventFromJvmtiEvent : public AllStatic {
private:
  static jvmtiEventMode _jvmdi_class_unload_enabled_mode;
public:
  static jvmtiEventCallbacks *set_jvmdi_event_hook(JVMDI_EventHook new_hook);
  static void set_jvmdi_class_unload_enabled_mode(jvmtiEventMode mode);
private:
  static void set_class_unload_callback_for_jvmdi();
};


#endif   /* _JAVA_JVMTITOJVMDIEVENT_H_ */
