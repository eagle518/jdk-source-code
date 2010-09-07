/*
 * @(#)JarCacheUtil.java	1.27 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.cache;

import java.io.*;
import java.lang.ref.*;
import java.net.*;
import java.security.*;
import java.util.*;
import sun.plugin.util.*;
import java.text.DateFormat;
import java.util.regex.Pattern; 
import com.sun.deploy.cache.Cache;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

import com.sun.deploy.resources.ResourceManager;

public class JarCacheUtil{
    public static HashMap getJarsWithVersion(String cacheArchive, 
            String cacheVersion, String cacheArchiveEx) {

	String jarFileName, jarFileVersion;
	HashMap jarFileVersionMap = new HashMap();

	if(cacheArchive!= null && cacheVersion != null) {
	    StringTokenizer fileTok = new StringTokenizer(cacheArchive, ",", false);
	    int fileCount = fileTok.countTokens();

	    StringTokenizer verTok = new StringTokenizer(cacheVersion, ",", false);
	    int versionCount = verTok.countTokens();

	    if(fileCount != versionCount){
                // alert developer the number of jar version specified does not
                // match number of jar file specified
                if (Trace.isTraceLevelEnabled(TraceLevel.BASIC))
                    Trace.println(ResourceManager.getMessage(
                        "cache.version_attrib_error"), TraceLevel.BASIC);
	    }

            // maps available jar filename to jar version
	    while(fileTok.hasMoreTokens() && verTok.hasMoreTokens()){
		jarFileName = fileTok.nextToken().trim();
		jarFileVersion = verTok.nextToken().trim();
		jarFileVersionMap.put(jarFileName, jarFileVersion);
	    }
	}

	if(cacheArchiveEx != null) {
	    //Figure out which JAR files still need to be loaded.
	    StringTokenizer jarTok = new StringTokenizer(cacheArchiveEx, ",", false);
	    while (jarTok.hasMoreTokens() ) {
		String elem = jarTok.nextToken().trim();
		StringTokenizer fieldTok = new StringTokenizer(elem, ";", false);
		jarFileName = fieldTok.nextToken().trim();

		//Find out whether the version is specified	    		
		while (fieldTok.hasMoreTokens() ) {
		    jarFileVersion = fieldTok.nextToken().trim();
		    if( Pattern.matches(FileVersion.regEx, jarFileVersion) ) {
			jarFileVersionMap.put(jarFileName, jarFileVersion);
			break;
		    }
		}
	    }
	}
		
	return jarFileVersionMap;
    }

    public synchronized static void preload(URL codeBase, HashMap preloadMap) 
    throws IOException {
        
        Iterator keyItr = preloadMap.keySet().iterator();
        while(keyItr.hasNext()) {
            String jarFileName = (String)keyItr.next();
            String jarFileVersion = (String)preloadMap.get(jarFileName);
            URL url = new URL(codeBase, jarFileName);
            URL jarUrl = null;
            if (jarFileVersion != null) {
                jarUrl = new URL("jar:" + url.toString() + "?version-id=" +
                        jarFileVersion + "!/");
            } else {
                jarUrl = new URL("jar:" + url.toString() + "!/");
            }
            JarURLConnection jarConnection = (JarURLConnection)jarUrl.openConnection();
            // this will trigger loading of the jar resource
            jarConnection.getContentLength();
        }
       
    }
}

