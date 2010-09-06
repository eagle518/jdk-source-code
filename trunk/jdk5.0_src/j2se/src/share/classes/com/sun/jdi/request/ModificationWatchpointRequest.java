/*
 * @(#)ModificationWatchpointRequest.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.request;

import com.sun.jdi.*;

/**
 * Request for notification when a field is set.
 * This event will be triggered when a value is assigned to the specified
 * field with a Java<SUP><FONT SIZE="-2">TM</FONT></SUP> programming
 * language statement (assignment, increment, etc) or by a
 * Java Native Interface (JNI) set function (<code>Set&lt;Type&gt;Field,
 * SetStatic&lt;Type&gt;Field</code>).
 * Setting a field to a value which is the same as the previous value
 * still triggers this event.
 * Modification by JDI does not trigger this event.
 * When an enabled 
 * ModificationWatchpointRequest is satisfied, an
 * {@link com.sun.jdi.event.EventSet event set} containing a
 * {@link com.sun.jdi.event.ModificationWatchpointEvent ModificationWatchpointEvent}
 * will be placed on 
 * the {@link com.sun.jdi.event.EventQueue EventQueue}.
 * The collection of existing
 * watchpoints is 
 * managed by the {@link EventRequestManager}.  
 *
 * @see com.sun.jdi.event.ModificationWatchpointEvent
 * @see AccessWatchpointRequest
 * @see com.sun.jdi.event.EventQueue
 * @see EventRequestManager
 *
 * @author Robert Field
 * @since  1.3
 */
public interface ModificationWatchpointRequest extends WatchpointRequest {
}


			
