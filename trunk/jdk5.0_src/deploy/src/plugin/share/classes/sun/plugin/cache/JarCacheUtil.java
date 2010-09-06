/*
 * @(#)JarCacheUtil.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.cache;

import java.io.*;
import java.lang.ref.*;
import java.net.*;
import java.security.*;
import java.util.*;
import sun.plugin.util.*;
import java.text.DateFormat;
import sun.plugin.resources.ResourceHandler;
import java.util.regex.Pattern; 
import sun.plugin.ClassLoaderInfo;


public class JarCacheUtil{
    private static String PRELOAD = "preload";

    public synchronized static void verifyJarVersions(URL codeBase, String key, HashMap jarFileVersionMap)
		    throws IOException, JarCacheVersionException {
    	boolean markClassLoader = false;

	//If versions are specified for all the jar files, try to mark
	//the cached files 
	Iterator iter = jarFileVersionMap.keySet().iterator();
	while(iter.hasNext()) {
	    String jarFileName = (String)iter.next();
	    String jarFileVersion = (String)jarFileVersionMap.get(jarFileName);
	    URL url = new URL(codeBase, jarFileName);
	    //System.out.println("File name is: " + jarFileName);
	    //System.out.println("File Version is: " + jarFileVersion);
            Trace.msgNetPrintln("cache.version_checking", new Object[] {jarFileName, jarFileVersion});
	    boolean result = JarCache.handleVersion( url, new FileVersion(jarFileVersion) );
	    if(result == true) markClassLoader = true;			    
	}

	//If one of the jar file has been modified, clear the classloader since 
	//it is no longer correct to use it.
	if(markClassLoader == true) {
	    ClassLoaderInfo.markNotCachable(codeBase, key);
	}
    }	

    public static HashMap getJarsWithVersion(String cacheArchive, String cacheVersion, String cacheArchiveEx) 
	throws JarCacheVersionException {

	String jarFileName, jarFileVersion;
	HashMap jarFileVersionMap = new HashMap();

	if(cacheArchive!= null && cacheVersion != null) {
	    StringTokenizer fileTok = new StringTokenizer(cacheArchive, ",", false);
	    int fileCount = fileTok.countTokens();

	    StringTokenizer verTok = new StringTokenizer(cacheVersion, ",", false);
	    int versionCount = verTok.countTokens();

	    if(versionCount != versionCount){
		throw new JarCacheVersionException( ResourceHandler.getMessage("cache.version_attrib_error") );
	    }

	    while(fileTok.hasMoreTokens()){
		jarFileName = fileTok.nextToken().trim();
		jarFileVersion = verTok.nextToken().trim();
		//System.out.println("Added: " + jarFileName + jarFileVersion);
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
			//System.out.println("Added: " + jarFileName + jarFileVersion);
			jarFileVersionMap.put(jarFileName, jarFileVersion);
			break;
		    }
		}
	    }
	}
		
	return jarFileVersionMap;
    }

    public synchronized static String getJarsInCacheArchiveEx(String cacheArchiveEx){
	if(cacheArchiveEx != null) {
	    String jarFiles = "";
	    StringTokenizer jarTok = new StringTokenizer(cacheArchiveEx, ",", false);
	    int jarCount = jarTok.countTokens();

	    for(int i=0;i<jarCount;i++) {
		String elem = jarTok.nextToken().trim();
		int index = elem.indexOf(";");
		if(index != -1) {
		    String jarFileName = elem.substring(0, index);
		    jarFiles += jarFileName;
		    jarFiles += (i != jarCount-1)?",":"";
		}
	    }
    
	    return jarFiles;	    
	}

	return null;
    }


    public synchronized static void preload(URL codeBase, String cacheArchiveEx) throws IOException {
	
	ArrayList jarFiles = new ArrayList();
	// Figure out which JAR files still need to be loaded.
	StringTokenizer jarTok = new StringTokenizer(cacheArchiveEx, ",", false);
	while (jarTok.hasMoreTokens()) {
	    String elem = jarTok.nextToken().trim();
	    int optionIndex = elem.indexOf(';');
	    if(optionIndex != -1) {
		String optionString = elem.substring(optionIndex); 
		if(optionString.toLowerCase().indexOf(PRELOAD) != -1) {
		    String jarFileName = elem.substring(0, optionIndex);
		    jarFiles.add(jarFileName);
		}
	    }
	}

	//pre-load the jar files
	for(int i = 0; i < jarFiles.size(); i++){
	    URL url = new URL(codeBase, (String)jarFiles.get(i));
	    Trace.msgNetPrintln("cache.preloading", new Object[]{(String)jarFiles.get(i)});
	    JarCache.get(url);
	}
    }
}

