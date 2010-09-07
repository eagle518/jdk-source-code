/*
 * @(#)WIExplorerOfflineHandler.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.offline;

import java.net.URL;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.resources.ResourceManager;
/**
 * System offline handler for Internet Explorer on Windows.
 *
 * @author Stanley Man-Kit Ho
 */
public class WIExplorerOfflineHandler implements OfflineHandler 
{
     /**
      * Returns true if the system is globally offline.
      */
     public native boolean isGlobalOffline();
	
     /**
      * Change the global offline state of the system.
      *  
      * @param offline offline state.
      * @return true if succeeded.
      */
     public native boolean setGlobalOffline(boolean offline);
	
     /**
      * Ask user permission to go online.
      * 
      * @return true if user gives permission.
      */
     public boolean askUserGoOnline(URL url)	{
        // do not display any AppInfo
        AppInfo appInfo = new AppInfo(AppInfo.TYPE_UNKNOWN, null, null, 
                   url, null, null, false, false, null, null);
	int ret = UIFactory.showConfirmDialog(null, appInfo,
                ResourceManager.getMessage("deployOfflineManager.askUserGoOnline.message"),
                ResourceManager.getMessage("deployOfflineManager.askUserGoOnline.title"));
        if (ret == UIFactory.OK) {
            // user agree to go online
            setGlobalOffline(false);
            return true;
        }
        return false;
     }

    /**
     * Ask user permission to go online.
     * 
     * @return true if user gives permission.
     */
    private native boolean askUserGoOnline(String urlString);
}



