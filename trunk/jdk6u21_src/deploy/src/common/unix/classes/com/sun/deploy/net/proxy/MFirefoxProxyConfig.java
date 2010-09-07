/*
 * @(#)MFirefoxProxyConfig.java	1.3 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import java.io.*;
import com.sun.deploy.util.Trace;


public final class MFirefoxProxyConfig implements BrowserProxyConfig 
{
    public BrowserProxyInfo getBrowserProxyInfo()
    {
        final String IS_RELATIVE = "isRelative=";
        final String PATH = "Path=";

        Trace.msgNetPrintln("net.proxy.loading.ns");

        File preferences = null;        
        BrowserProxyInfo info = new BrowserProxyInfo();
        info.setType(ProxyType.UNKNOWN);

        try {
            String homeDir = System.getProperty("user.home");
            File profiles = new File(homeDir + "/.mozilla/firefox/profiles.ini");

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
                    if (line.trim().equals("[Profile0]")) {
                        readingProfile0 = true;
                        continue;
                    }

                    // we take 0 as false, any other value as true
                    if (readingProfile0 && line.startsWith(IS_RELATIVE)) {
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

                    if (readingProfile0 && line.startsWith(PATH)) {
                        if (isRelative) {
                            preferences = new File(homeDir + "/.mozilla/firefox/" + 
                                                   line.substring(PATH.length()) +
                                                   "/prefs.js");
                        }

                        else {
                            preferences = new File(line.substring(PATH.length()) + "/prefs.js");
                        }                        

                        break;
                    }
                }
               
                in.close();

                if (preferences.exists()) {
                    NSPreferences.parseFile(preferences, info, 6, false);
                
                    // If User has selected Auto Proxy detection determine 
                    // what the correct WPAD URL should be.  
                    if(info.isAutoProxyDetectionEnabled()) {
                        // Set the Auto Config URL to the WPAD based URL
                        info.setAutoConfigURL( WebProxyAutoDetection.getWPADURL() );
                    }
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
