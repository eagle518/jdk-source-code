/*
 * @(#)PlatformService.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import com.sun.deploy.util.Trace;

/** 
 * PlatformService is an interface that encapsulates the platform service.
 *
 * @since 1.4.2
 */
public abstract class PlatformService
{

    public int createEvent() {
	return 0;
    }

    public void deleteEvent(int event) {
    }

    /**
     * Signal an event, the default behavior is doing nothing
     */
    public void signalEvent(int handle) {
    }

    /**
     * Wait on an event, the default behavior is doing nothing
     */
    public void waitEvent(int handle) {
    }

    public void waitEvent(long winHandle, int handle) {
	waitEvent(handle);
    }

    public void waitEvent(long winHandle, int handle, long timeout) {
        waitEvent(handle);
    }

    /**
     * Dispatch platform native event, the default behavior is doing nothing
     */
    public void dispatchNativeEvent() {
    }

    /**
     * Return platform service.
     */
    public static synchronized PlatformService getService()
    {
	if (ps == null)
	{
	    String osName = (String) java.security.AccessController.doPrivileged(
	 	    	    new sun.security.action.GetPropertyAction("os.name"));
	    
	    try 
	    {
		String platformClassName = null;

		if (osName.indexOf("Windows") != -1)
		    platformClassName = "sun.plugin.services.WPlatformService";
		else
		    platformClassName = "sun.plugin.services.MPlatformService";

		Class platformClass = Class.forName(platformClassName);

		if (platformClass != null) 
		{
		    Object obj = platformClass.newInstance();

		    if (obj instanceof PlatformService) 
			ps = (PlatformService) obj;
		}
	    } 
	    catch(Exception e) 
	    {
		Trace.printException(e);
	    }
	}
	
	return ps;
    }


    // Platform service instance
    private static PlatformService ps = null;
}


