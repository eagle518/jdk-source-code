/*
 * @(#)NeverOfflineHandler.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.offline;

import java.net.URL;

/**
 * System offline handler that never offline.
 *
 * @author Stanley Man-Kit Ho
 */
class NeverOfflineHandler implements OfflineHandler 
{
    /**
     * Returns true if the system is globally offline.
     */
    public boolean isGlobalOffline()	{
	return false;
    }
	
    /**
     * Change the global offline state of the system. 
     *  
     * @param offline offline state.
     * @return true if succeeded.
     */
    public boolean setGlobalOffline(boolean offline)	{
 	return (!offline);
    }
	
    /**
     * Ask user permission to go online.
     * 
     * @param url URL to go online.
     * @return true if user gives permission.
     */
    public boolean askUserGoOnline(URL url)	{
	return true;
    }
}



