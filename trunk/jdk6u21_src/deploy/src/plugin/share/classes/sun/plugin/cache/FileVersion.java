/*
 * @(#)FileVersion.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.cache;

import java.text.MessageFormat;
import java.util.regex.Pattern; 
import java.util.StringTokenizer;
import com.sun.deploy.resources.ResourceManager;

public class FileVersion{

    private String strVersion;
    private long   longVersion;
    final private static int VERSION_DIGITS = 4;
    final private static int VERSION_DIGITS_BITSIZE = 16;
    final private static int VERSION_DIGITS_BYTESIZE = 4;
    final private static int VERSION_DIGITS_RADIX = 16;
    public final static String defStrVersion = "x.x.x.x";
    public final static int defIntVersion = 0;
    public final static String regEx = "\\p{XDigit}{1,4}\\.\\p{XDigit}{1,4}\\.\\p{XDigit}{1,4}\\.\\p{XDigit}{1,4}";

    public FileVersion(){
	strVersion = defStrVersion;
	longVersion = defIntVersion;
    }

    public FileVersion(String ver)throws JarCacheVersionException{
	strVersion = ver;
	longVersion = convertToLong(ver);
    }

    public FileVersion(long ver){
	strVersion = convertToString(ver);
	longVersion = ver;
    }

    public void setVersion(long ver)
    {
	if(ver > 0) {
	    longVersion = ver;
	    strVersion = convertToString(ver);
	}
    }

    public void setVersion(String ver) throws JarCacheVersionException
    {
	if(ver != null) {
	    strVersion = ver;
	    longVersion = convertToLong(ver);
	}
    }

    public long getVersionAsLong()
    {
	return longVersion;
    }

    public String getVersionAsString()
    {
	return strVersion;
    }

    public boolean isUpToDate(FileVersion inVer)
    {
	//return false if the file version is the default version
	if(!strVersion.equals(defStrVersion) && longVersion >= inVer.longVersion)
	    return true;
	else
	    return false;
    }

    public static long convertToLong(String inVersion) throws JarCacheVersionException {
	long ver = 0;

	if( !Pattern.matches(regEx, inVersion) ) 
	{
	    MessageFormat formatter = new MessageFormat(ResourceManager.getMessage("cache.version_format_error"));

	    throw new JarCacheVersionException(formatter.format(new Object[] { inVersion }));
	}

	//Calculate the version
        StringTokenizer st = new StringTokenizer(inVersion, ".", false);
	while(st.hasMoreTokens()){
	    String digit = st.nextToken().trim();
	    ver = ver << VERSION_DIGITS_BITSIZE;
	    ver = ver + Integer.parseInt(digit, VERSION_DIGITS_RADIX);
	}

	return ver;
    }

    public static String convertToString(long version)
    {
	String ver = "";
	long temp = version;
	for(int i=0;i<VERSION_DIGITS;i++){
	    long digit = (temp >> ((VERSION_DIGITS-1) * VERSION_DIGITS_BITSIZE)) & 0xffff;
	    temp = temp << VERSION_DIGITS_BITSIZE;
	    ver += Long.toString(digit, VERSION_DIGITS_RADIX);
	    ver += (i != VERSION_DIGITS -1)?".":"";
	}	
	return ver;
    }

    public static String getMessage(String key) 
    {
	return ResourceManager.getMessage(key);
    }
}
