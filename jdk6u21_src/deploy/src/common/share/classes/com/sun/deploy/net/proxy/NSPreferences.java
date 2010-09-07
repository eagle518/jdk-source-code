/*
 * @(#)NSPreferences.java	1.27 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.FileReader;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import com.sun.deploy.util.Trace;


/**
 * Configuration information about Netscape 4.x, notably the proxy addresses,
 * is stored in a JavaScript format file on both Windows and Unix.  Each
 * line in the file is a call to the user_pref function, here's an
 * example:
 * <pre>
 * user_pref("network.proxy.http", "webcache-cup.eng");
 * </pre>
 * This class supports extracting the proxy related entries from this file,
 * and initializing an InternetProxyInfo object.  It could be used like this
 * to print the current NS proxy settings on Unix:
 * <pre>
 * InternetProxyInfo info = new InternetProxyInfo();
 * NSPreferences.parse(new File("/home/foo/.netscape/preferences.js"), info);
 * System.out.println(info);
 *
 * @version 1.5, 02/09/01
 */
public class NSPreferences
{
    /**
     * Each line in the preferences file should look like this:
     * user_pref("foo.bar.baz", <value>);
     * We extract the value part of the line here; assuming that
     * the "," isn't legal as part of the (foo.bar.baz) path.
     */
    private static String parseValue(String line) {
        int i1 = line.indexOf(",");
        if (i1 != -1) {
            int i2 = line.lastIndexOf(")");
            if ((i2 != -1) && ((i1 + 1) < i2)) {
                return line.substring(i1 + 1, i2).trim();
            }
        }
        return null;
    }
    
    
    /**
     * Return the value part of the line with the leading and trailing
     * quotes removed.
     */
    private static String parseString(String line) {
        String value = parseValue(line);
        if ((value != null) && (value.length() > 1) && value.startsWith("\"") && value.endsWith("\"")) {
            return value.substring(1, value.length() - 1);
        }
        else {
            return null;
        }
    }
   
    
    /**
     * The value part of the line is assumed to be an unadorned base 10
     * integer. Return it or return -1 if the value can't be parsed.
     */
    private static int parseInt(String line) {
        String value = parseValue(line);
        if (value != null) {
            try {
                return Integer.parseInt(value);
            }
            catch (NumberFormatException e) {
            }
        }
        return -1;
    }
    
    
    /**
     * The value part of the line is assumed to be a string that contains
     * a comma separated list off tokens.
     */
    private static List parseList(String line) {
        StringTokenizer st = new StringTokenizer(parseString(line), ", ");
        ArrayList list = new ArrayList();
        while (st.hasMoreTokens()) 
	{
	    // Notice that there is no wildcard specified in the list,
	    // so we add it manually.
	    //	    
            list.add("*" + st.nextToken());
        }
        return list;
    }
    
    
    /**
     * Each line in the preferences file should look like this:
     * user_pref("keyword", <value>);
     * Return true if keyword matches the keyword on this line.  We're
     * assuming that there's no space between the open paren and the
     * open quote on the left side of the keyword.
     */
    private static boolean isKeyword(String line, String keyword) {
        int i = line.indexOf("(");
        return (i != -1) && line.substring(i+1, line.length()).startsWith("\"" + keyword + "\"");
    }
    
    
    
    private static boolean getDeaultValue(File file) {
    	boolean _isSystemChecked = false;
    	try {
    		if(file.exists()) {
        		BufferedReader allIn = new BufferedReader(new InputStreamReader(new FileInputStream(file), "ISO-8859-1"));           
                String aline;
                while((aline = allIn.readLine()) != null)
        	    {
                        if (!aline.startsWith("pref")) {
                            continue;
                        } else { 
                        	if (isKeyword(aline, "config.use_system_prefs")) {
                        		if(parseValue(aline).equals("true")) {
                            		_isSystemChecked = true;
                            		break;
                            	}else {
                            		_isSystemChecked = false;
                            		break;
                            	}
                        	}
                        }
        	    }
                allIn.close();
        	}
    	}catch (IOException e) {
    		Trace.ignoredException(e);
    	}
    	return _isSystemChecked;
    }
    
    /**
     * get the default value of "config.use_system_prefs"
     * for JDS mozilla the defaule value is true,for community mozilla default value is false
     * JDS for solaris mozilla install in /usr/sfw/lib/mozilla ,JDS for linux mozilla install in /usr/lib/mozilla
     * this method check the config file all.js in /usr/sfw/lib/mozilla/greprefs/all.js and /usr/lib/mozilla/greprefs/all.js
     * if one of them exist,parse the file get the value for "config.use_system_prefs"
     * if both of them not exist, it means that the browse in this UNIX is community mozilla,so the default value is false.
     */
    private static boolean getDefaultValueOfUseSystemPrefs() {
    	boolean _isSystemChecked = false;
    	
	//parse all.js 
    	File solarisAll = new File("/usr/sfw/lib/mozilla/greprefs/all.js");
    	File linuxAll = new File("/usr/lib/mozilla/greprefs/all.js");

    	//if this file exist, that is to say JDS mozilla in use.
    	//the JDS for solaris mozilla is constantly in /usr/sfw/lib/mozilla
    	//the JDS for linux mozilla is in /usr/lib/mozilla
    	_isSystemChecked = getDeaultValue(solarisAll) || getDeaultValue(linuxAll);

    	return _isSystemChecked;
    }
    
    /**
     * Extract the proxy information from the specified "prefs.js" JavaScript
     * file ("preferences.js" on Unix) and return an BrowserProxyInfo object
     * that contains whatever information was found.
     */
    protected static void parseFile(File file, BrowserProxyInfo info, float version,
                                    boolean checkSystemPrefs) 
    {
        BufferedReader in = null;
        try 
	{
	    in = new BufferedReader(new InputStreamReader(new FileInputStream(file), "ISO-8859-1"));           
            String line;
	    String httpHost=null;
	    String autoConfigURL = null;
	    int proxyType = -1;
	    int httpPort = -1;
	    boolean isSystemChecked = getDefaultValueOfUseSystemPrefs();
	    
	    
            while((line = in.readLine()) != null)
	    {
                if (!line.startsWith("user_pref")) {
                    continue;
                }
                else if (isKeyword(line, "network.proxy.type")) {
		    proxyType = parseInt(line);
		    //Netscape 4.79 and Solaris, proxyType 3 means NO PROXY
		    if (proxyType == 3 && System.getProperty("os.name").equals("SunOS") && version >= 4) {
			info.setType(ProxyType.NONE);
			proxyType = 0;
		    } else {
			info.setType(proxyType);	
		    }
    		    Trace.msgNetPrintln("net.proxy.browser.proxyEnable", new Object[] {new Integer(proxyType)});
                    
                }
                else if (isKeyword(line, "network.proxy.http")) {
		    httpHost = parseString(line);
		    try 
		    {
			URL u = new URL(httpHost);
			httpHost = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.http=" + httpHost);
		    info.setHttpHost(httpHost);
                }
                else if (isKeyword(line, "network.proxy.http_port")) 
		{
		    httpPort = parseInt(line);
    		    Trace.netPrintln("    network.proxy.http_port=" + httpPort);
                    info.setHttpPort(httpPort);
                }
                else if (isKeyword(line, "network.proxy.ssl")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.ssl=" + host);
		    info.setHttpsHost(host);
                }
                else if (isKeyword(line, "network.proxy.ssl_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.ssl_port=" + port);
                    info.setHttpsPort(port);
                }
                else if (isKeyword(line, "network.proxy.ftp")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.ftp=" + host);
		    info.setFtpHost(host);
                }
                else if (isKeyword(line, "network.proxy.ftp_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.ftp_port=" + port);
                    info.setFtpPort(port);
                }
                else if (isKeyword(line, "network.proxy.gopher")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.gopher=" + host);
		    info.setGopherHost(host);
                }
                else if (isKeyword(line, "network.proxy.gopher_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.gopher_port=" + port);
                    info.setGopherPort(port);
                }
                else if (isKeyword(line, "network.proxy.socks")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.socks=" + host);
		    info.setSocksHost(host);
                }
                else if (isKeyword(line, "network.proxy.socks_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.socks_port=" + port);
                    info.setSocksPort(port);
                }
                else if (isKeyword(line, "network.proxy.no_proxies_on")) 
		{
		    Trace.msgNetPrintln("net.proxy.browser.proxyOverride", new Object[] {parseString(line)});
                    info.setOverrides(parseList(line));
                }
                else if (isKeyword(line, "network.proxy.autoconfig_url")) 
		{
		    autoConfigURL = parseString(line);
		    Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL});
                    info.setAutoConfigURL(autoConfigURL);
                }
                //the default value of "config.use_system_prefs" in JDS mozilla is true and false in community version
                //config in pref.js take higer priority than all.js 
                else if (isKeyword(line, "config.use_system_prefs")) {
                	if(parseValue(line).equals("true")) {
                		isSystemChecked = true;
                	}else {
                		isSystemChecked = false;
                	}
                } 
            }
            
            in.close();

            // if parsing a "prefs.js" file that was written by Firefox (on Unix)
            // don't do this check
            if (checkSystemPrefs) {
                if(isSystemChecked) {
                    info.setType(ProxyType.SYSTEM);
                    return;
                }
            }


	    // No special cases here for Solaris because we can't tell
	    // the difference between 4.76 and 4.79 and the proxyType
	    // settings are going to be different between those.
	    // NS 6 & 7 proxy detection should behave correctly on
	    // Solaris but 4.76 and 4.79 are going to show the proxy
	    // dialog even if NONE is selected in the browser.

	    // Solaris and netscape 7
	    // proxy type can be 0 (none), 1 (manual) and -1 (auto)
	    if (proxyType == -1 && System.getProperty("os.name").equals("SunOS") && version >= 6) {
		info.setType(ProxyType.AUTO);
		return;
	    }

	    // Linux and netscape 4 and netscape 6.2
	    // proxy type can be -1 (None), 1 (manual) and 2 (auto)
	    // on linux, proxyType == -1 implies NONE
	    if (proxyType == -1 && System.getProperty("os.name").equals("Linux") && version >= 4) {
		info.setType(ProxyType.NONE);
		return;
	    }
	    
	    // Windows and netscape 4.x
	    // type can be -1 (none), 1 (manual), 2 (auto)
	    if (proxyType == -1 && System.getProperty("os.name").indexOf("Windows") != -1 && version >= 4 && version < 5) {
		info.setType(ProxyType.NONE);
		return;
	    }
	    
	    // Windows and netscape 6.2 & 7
	    // type can be 0 (none), -1 (manual), 2 (auto)
	    if (httpHost != null && httpPort == -1 && System.getProperty("os.name").indexOf("Windows") != -1 && version >= 6) {
		// netscape 6.2 (windows) defaults to port 8080 if nothing specified
		httpPort = 8080;
		info.setHttpPort(8080);
	    }
	 
	    // in netscape 6.2 (windows),netscape 7.0, Mozilla and Firefox browser, 
	    // if it did NOT specify proxy type, it means DIRECT 
	    if (System.getProperty("os.name").indexOf("Windows") != -1 
		&& version >= 6 && proxyType == -1) { 
		proxyType = 0; 
		info.setType(ProxyType.NONE); 
	    }

	    // work around a bug in Netscape 4.79, 6, 7 and who knows
	    // what other versions of netscape.  If we stil have not figured
	    // out what the proxy is supposed to be, and the autoconfig file
	    // exists, make the type autoconfig.  This way, the proxy dialog
	    // will popup with hints from the autoconfig file.
	    if (proxyType == -1 && autoConfigURL != null) {
		info.setType(ProxyType.AUTO);
	    }
            
            // If Proxy type is 4 (Auto-Detect proxy settings) and version
            // is at least equal to the version of Mozilla(firefox) Auto-Detect
            // was introduced in, set the proxyType to auto indicate
            // AutoProxy detection is enabled
            if( proxyType == 4 && version >= 6 ) {
                info.setType(ProxyType.AUTO);
                info.setAutoProxyDetectionEnabled(true);
            }

        }
        catch (IOException exc1) {
            if (in != null) {
                try {
                    in.close();
                }
                catch (IOException exc2) {
                }
            }
        }
    }

    /**
     * Return the location of the "prefs.js" user profile file in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 6 of Navigator.
     */
    public static File getNS6PrefsFile(File registryFile) throws IOException
    {
	return new File(getNS6UserProfileDirectory(registryFile), "prefs.js");
    }

    /**
     * Return the location of the user profile directory in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 6 of Navigator.
     */
    public static String getNS6UserProfileDirectory(File registryFile) throws IOException
    {
        NSRegistry reg = new NSRegistry().open(registryFile);
        String path = null;
        String currProfileName = null;

        // Get current user profile directory
        if (reg != null)
        {
            currProfileName = reg.get("Common/Profiles/CurrentProfile");
            if (currProfileName != null)
            {
                path = reg.get("Common/Profiles/" + currProfileName + "/directory");
            }
            reg.close();
        }

        if (path == null)
        {
            throw new IOException();
        }
        else
            return path;
    }

    /**
     * Return the location of the user profile directory in the
     * FireFox profile.ini file or null if we can't figure that out.
     */
    public static String getFireFoxUserProfileDirectory(File iniFile) throws IOException
    {
        String currProfileName = null;
        String line = null;
        String path = null;
	boolean isRelative = true;
	boolean readingProfile0 = false;
	final String IS_RELATIVE = "isrelative=";
        final String PATH_PROFILES = "path=";

        // Get FireFox current user profile directory
        if (iniFile.exists()) {
           BufferedReader in = new BufferedReader(new FileReader(iniFile));
           while ( (line = in.readLine()) != null) {

	     // Since we can't figure out which profile is really in use
	     // we will use Profile0 data by default
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

             if (readingProfile0 && line.toLowerCase().startsWith(PATH_PROFILES) ) {
                currProfileName = line.substring(PATH_PROFILES.length());
		break;
	     }
           }
        }

        if (currProfileName != null) {
	   if (isRelative) {
              int x = (iniFile.toString().length()-12);
              path = iniFile.toString().substring(0, x) + currProfileName;
	   }
	   else {
	      path = currProfileName;
	   }
        }

        if (path == null) {
            throw new IOException();
        }
        else
            return path;
    }
}

