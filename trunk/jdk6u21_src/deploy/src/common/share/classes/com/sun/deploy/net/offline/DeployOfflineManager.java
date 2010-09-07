/*
 * @(#)DeployOfflineManager.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.offline;

import java.net.URL;
import com.sun.deploy.services.Service;

/**
 * Offline manager that is used to control system global offline
 * state.
 *
 * @author Stanley Man-Kit Ho
 */
public class DeployOfflineManager 
{
    // Offline handler
    private static OfflineHandler handler = new NeverOfflineHandler();
    
    // by default deploy is always online
    private static boolean forcedOffline = false;
    
    public static void setForcedOffline(boolean offline) {
        forcedOffline = offline;
    }
    
    public static boolean promptUserGoOnline(URL u) {
        if (isGlobalOffline() && isForcedOffline() == false) {
            if (askUserGoOnline(u) == false) {
                setForcedOffline(true);
                return false;
            }
        }
        return true;
    }

    public static boolean isForcedOffline() {
        return forcedOffline;
    }
	
    /**
     * Reset offline manager.
     */
    public static void reset()    {
	Service service = com.sun.deploy.services.ServiceManager.getService();
	handler = service.getOfflineHandler();
	
	if (handler == null)
	    handler = new NeverOfflineHandler();		
    }
	
    /**
     * Returns true if the system is globally offline.
     */
    public static boolean isGlobalOffline()	{
        // if forced offline, return true
        if (forcedOffline) {
            return true;
        }
	return handler.isGlobalOffline();
    }
	
    /**
     * Change the global offline state of the system.
     *  
     * @param offline offline state.
     * @return true if succeeded.
     */
    public static boolean setGlobalOffline(boolean offline)	{
	return handler.setGlobalOffline(offline);
    }
	
    /**
     * Ask user permission to go online.
     * 
     * @param url URL to go online.     
     * @return true if user gives permission.
     */
    public static boolean askUserGoOnline(URL url)	{
        // if forced offline, no need to ask user and return false
        if (forcedOffline) {
            return false;
        }
	return handler.askUserGoOnline(url);
    }
}



