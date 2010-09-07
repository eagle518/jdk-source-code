/*
 * @(#)WPlatformService.java	1.3 02/09/05
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.io.File;


/** 
 * WPlatformService is a class that encapsulates the platform service
 * in Windows.
 *
 * @since 1.4.2
 */
public final class WPlatformService extends PlatformService
{
    public void waitEvent(int event) {
	waitEvent(0, event, (long)-1);
    }

    public void waitEvent(long handle, int event) {
        waitEvent(handle, event, -1);
    }

    public native int createEvent();
    public native void deleteEvent(int event);
    public native void signalEvent(int handle);
    public native void waitEvent(long winHandle, int handle, long timeout);
    public native void dispatchNativeEvent();
}


