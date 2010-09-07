/*
 * @(#)ExtensionUtils.java	1.18 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.extension;

import java.io.IOException;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.PrivilegedAction;
import java.security.AccessController;
import sun.plugin.util.UserProfile;
import com.sun.deploy.util.Trace;

/**
 * Provide facilities to install extensions in the standard extension
 * directory of the running JDK
 *
 * @author Stanley Man-Kit Ho
 */
public class ExtensionUtils 
{
    // Version table filename
    private static final String _tempDir;

    static
    {
        String userHome = (String) java.security.AccessController.doPrivileged(
               new sun.security.action.GetPropertyAction("user.home"));

        String pluginVersion = (String) java.security.AccessController.doPrivileged(
               new sun.security.action.GetPropertyAction("javaplugin.version"));

	// Determine filename of the version table
	_tempDir = UserProfile.getTempDirectory();

	// Make sure the directory is created.
	AccessController.doPrivileged(
	    new PrivilegedAction() 
	    {	
		public Object run()
		{
		    try
		    {
			File tempDir = new File(_tempDir);
			tempDir.mkdirs();
		    }
		    catch (Throwable e)
		    {
			Trace.extPrintException(e);
		    }

		    return null;
		}
	    });
    }

    /** 
     * @return the name of the temp directory.
     */
    static String getTempDir()
    {
	return _tempDir;
    }


    /**
     * @return the name of the jar file from an Jar URL
     */
    static String extractJarFileName(String url) 
    {
	int lastIndex;
	int fromIndex = url.indexOf('#');
	if (fromIndex==-1) {
	    lastIndex=url.lastIndexOf('/');
	    if (lastIndex<url.lastIndexOf('\\'))
		lastIndex=url.lastIndexOf('\\');
	} else {
	    lastIndex=url.lastIndexOf('/', fromIndex);
	    if (lastIndex<url.lastIndexOf('\\', fromIndex));
		lastIndex = url.lastIndexOf('\\', fromIndex);
	}

	if (lastIndex==-1)
	    return null;
	else 
	    return url.substring(lastIndex+1);
    }

    /*
     * <p>
     * Blindly copy the inputstream into the outputstream until no more 
     * data is available
     * </p>
     *
     * @param is is the inputstream to read and copy from
     * @param os is the OutputStream to copy to
     */
    static void copy(InputStream is, OutputStream os) 
	throws IOException
    {
	byte[] buffer = new byte[10240];
	int nRead;
	do {
	    nRead = is.read(buffer);
	    if (nRead != -1)
		os.write(buffer, 0, nRead);
	} while (nRead!=-1);
	is.close();
	os.close();
    }

    /*
     * <p>
     * URL poiting to an extension installation jar may be system dependent
     * The Plug-in is responsible for determining depending on the value of 
     * the "os.name" system property for making this particular url platform
     * dependent.
     * Replace the system dependent portion of an url with the value of the
     * sytem property. 
     * </p>
     *
     * @param source is the url eventually containing the key ("os.name")
     * 
     * @return the system dependent url where the key has been replaced with
     * the value of the system propery key.
     */
    static String makePlatformDependent(String source) 
    {
	String result = makePlatformDependentOsName0(makePlatformDependentOsName1(source));
	result = makePlatformDependentName("os.arch", result);
	result = makePlatformDependentName("os.version", result);
	return result;	
    }

    /*
     * <p>
     * URL poiting to an extension installation jar may be system dependent
     * The Plug-in is responsible for determining depending on the value of 
     * the "os.name" system property for making this particular url platform
     * dependent.
     * Replace the system dependent portion of an url with the value of the
     * sytem property. 
     * </p>
     *
     * @param source is the url eventually containing the key ("os.name")
     * 
     * @return the system dependent url where the key has been replaced with
     * the value of the system propery key.
     */
    static String makePlatformDependentOsName0(String source) 
    {
	final String property = "os.name";
	String key = "$(os-name)$";
	int platformIndex = source.indexOf(key);
	String target = source;
	if (platformIndex != -1) {
	    String osName = System.getProperty(property);
	    String platformString = osName.replace(' ', '-');

	    // Replace "$(os.name)$" (the key) with the value of
	    // the system property (same key).
	    target = source.substring(0, platformIndex) + 
		platformString + 
		source.substring(platformIndex + key.length(), 
				 source.length());
	}
	return target;
    }

    /*
     * <p>
     * URL poiting to an extension installation jar may be system dependent
     * The Plug-in is responsible for determining depending on the value of 
     * the "os.name" system property for making this particular url platform
     * dependent.
     * Replace the system dependent portion of an url with the value of the
     * sytem property. 
     * </p>
     *
     * @param source is the url eventually containing the key ("os.name")
     * 
     * @return the system dependent url where the key has been replaced with
     * the value of the system propery key.
     */
    static String makePlatformDependentOsName1(String source) 
    {
	final String property = "os.name";
	String key = "$(os.name)$";
	int platformIndex = source.indexOf(key);
	String target = source;
	if (platformIndex != -1) {
	    String osName = System.getProperty(property);
	    String platformString = osName.replace(' ', '-');

	    // Replace "$(os.name)$" (the key) with the value of
	    // the system property (same key).
	    target = source.substring(0, platformIndex) + 
		platformString + 
		source.substring(platformIndex + key.length(), 
				 source.length());
	}
	return target;
    }

    /*
     * <p>
     * URL poiting to an extension installation jar may be system dependent
     * The Plug-in is responsible for determining depending on the value of 
     * the system property for making this particular url platform
     * dependent.
     * Replace the system dependent portion of an url with the value of the
     * sytem property. 
     * </p>
     *
     * @param property is the system property name
     * @param source is the url eventually containing the key
     * 
     * @return the system dependent url where the key has been replaced with
     * the value of the system propery key.
     */
    static String makePlatformDependentName(String property, String source) 
    {
	String key = "$(" + property + ")$";
	int platformIndex = source.indexOf(key);
	String target = source;
	if (platformIndex != -1) {
	    String value = System.getProperty(property);
	    String platformString = value.replace(' ', '-');

	    // Replace "$(property)$" (the key) with the value of
	    // the system property (same key).
	    target = source.substring(0, platformIndex) + 
		platformString + 
		source.substring(platformIndex + key.length(), 
				 source.length());
	}
	return target;
    }
}
