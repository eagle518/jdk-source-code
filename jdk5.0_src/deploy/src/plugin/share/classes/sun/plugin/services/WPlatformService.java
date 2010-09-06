/*
 * @(#)WPlatformService.java	1.3 02/09/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    public void waitEvent(int handle) {
	waitEvent(0, handle);
    }

    public native void signalEvent(int handle);
    public native void waitEvent(int winHandle, int handle);
    public native void dispatchNativeEvent();
}


