/*
 * @(#)MethodExitEvent.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.event;

import com.sun.jdi.*;

/**
 * Notification of a method return in the target VM. This event
 * is generated after all code in the method has executed, but the 
 * location of this event is the last executed location in the method. 
 * Method exit events are generated for both native and non-native 
 * methods. Method exit events are not generated if the method terminates
 * with a thrown exception. 
 *
 * @see EventQueue
 *
 * @author Robert Field
 * @since  1.3
 */
public interface MethodExitEvent extends LocatableEvent {

    /**
     * Returns the method that was exited.
     *
     * @return a {@link Method} which mirrors the method that was exited.
     * @throws ObjectCollectedException may be thrown if class 
     * has been garbage collected.
     */
    public Method method();
}

