/*
 * @(#)AccessWatchpointEvent.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.event;

import com.sun.jdi.*;

/**
 * Notification of a field access in the target VM. Field modifications
 * are not considered field accesses. 
 *
 * @see EventQueue
 * @see VirtualMachine
 *
 * @author Robert Field
 * @since  1.3
 */
public interface AccessWatchpointEvent extends WatchpointEvent {
}
    

