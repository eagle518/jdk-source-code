/*
 * @(#)WFirefoxProxyConfig.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import java.io.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.WinRegistry;

public final class WFirefoxProxyConfig implements BrowserProxyConfig 
{
    public BrowserProxyInfo getBrowserProxyInfo()
    {
        final String IS_RELATIVE = "isrelative=";
        final String PATH_PROFILES = "path=";

        Trace.msgNetPrintln("net.proxy.loading.ns");

        File preferences = null;        
        BrowserProxyInfo info = new BrowserProxyInfo();
        info.setType(ProxyType.UNKNOWN);

        try {
            String appDataDir = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER, 
		          "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                          "AppData");

            File profiles = new File(appDataDir + "\\mozilla\\firefox\\profiles.ini");

            if (profiles.exists()) {
                FileInputStream fis = new FileInputStream(profiles);
                InputStreamReader isr = new InputStreamReader(fis, "ISO-8859-1");
                BufferedReader in = new BufferedReader(isr);

                boolean readingProfile0 = false;
                boolean isRelative = true;
                String line;
                while((line = in.readLine()) != null)
                {
                    // since we can not figure out which profile is really in use, we
                    // use the Profile0 data
                    if (line.trim().toLowerCase().equals("[profile0]")) {
                        readingProfile0 = true;
                        continue;
                    }

                    // we take 0 as false, any other value as true
                    if (readingProfile0 && line.toLowerCase().startsWith(IS_RELATIVE)) {
                        try {
                            int val = Integer.parseInt(line.substring(IS_RELATIVE.length()));
                            isRelative = (val != 0) ? true : false;
                        }
                        catch (NumberFormatException e) 
                        {
                            isRelative = true;
                        }
                        continue;
                    }

                    if (readingProfile0 && line.toLowerCase().startsWith(PATH_PROFILES)) {
                        if (isRelative) {
                            preferences = new File(appDataDir + "\\mozilla\\firefox\\" + 
                                                   line.substring(PATH_PROFILES.length()) +
                                                   "\\prefs.js");
                        }
                        else {
                            preferences = new File(line.substring(PATH_PROFILES.length()) + "\\prefs.js");
                        }                        

                        break;
                    }
                }
               
                in.close();

                if ( (preferences != null) && preferences.exists()) {
                    NSPreferences.parseFile(preferences, info, 6, true);
                }
            }
        }
            
        catch (IOException e)
        {
            info.setType(ProxyType.UNKNOWN);
        }
        catch (SecurityException e) 
        {
            Trace.netPrintException(e);
            info.setType(ProxyType.UNKNOWN);
        }
        
        // This is a workaroud for NS6 because of the LiveConnect bug
        //
        if (java.security.AccessController.doPrivileged(
              new sun.security.action.GetPropertyAction("javaplugin.version")) != null)
        {
            info.setType(ProxyType.BROWSER);
        }
        Trace.msgNetPrintln("net.proxy.loading.done");
        
        return info;
    }
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }
    
}
