/*
 * @(#)LocatableEvent.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.event;

import com.sun.jdi.*;

import java.util.List;

/**
 * Abstract superinterface of events which have both location
 * and thread.
 *
 * @author Robert Field
 * @since  1.3
 */
public interface LocatableEvent extends Event, Locatable {

    /**
     * Returns the thread in which this event has occurred. 
     *
     * @return a {@link ThreadReference} which mirrors the event's thread in 
     * the target VM.
     */
    public ThreadReference thread();
}
    

