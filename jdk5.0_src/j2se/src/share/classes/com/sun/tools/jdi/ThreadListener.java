/*
 * @(#)ThreadListener.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.tools.jdi;

import com.sun.jdi.*;
import java.util.EventListener;

interface ThreadListener extends EventListener {
    boolean threadResumable(ThreadAction action);
    /* 
     * Not needed for current implemenation, and hard to implement
     * correctly. (See TargetVM.handleEventCmdSet)
     *   void threadSuspended(ThreadAction action);
     */
}


