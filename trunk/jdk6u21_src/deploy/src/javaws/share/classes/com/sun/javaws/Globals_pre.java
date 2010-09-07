/*
 * @(#)Globals_pre.java	1.115 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Locale;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.lang.NoSuchFieldException;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.File;
import java.net.URL;
import com.sun.deploy.util.Trace;
import com.sun.deploy.Environment;
import java.awt.GraphicsEnvironment;

public class Globals {

    /**
     * The name of the Java Web Start Component, this is cast
     * in stone, if this changes then the whole updating schema
     * will have to change.
     */
    private static final String JAVAWS_NAME = "javaws-##RELEASE##";

    public static final String JAVAWS_VERSION = "##RELEASE##";

    // This needs to be updated whenever there is new release of the JNLP
    // specification
    static final String JNLP_VERSION = "6.0";


    // Used to identify the windows platform.
    private static final String WIN_ID = "Windows";

    // State of Client:
    private static boolean _isSilentMode = false;
    private static boolean _isQuietMode = false;
   
    private static boolean _isSecureMode = false;

    private static boolean _isReverseMode = false;
  
    private static String[] _applicationArgs = null;
    private static boolean _createShortcut = false;
    private static boolean _createAssoc = false;
    private static boolean _showPrompts = false;
   
    private static boolean _isIconImageUpdated = false;
    
    //Default Host and port to log to
    private static final String DEFAULT_LOGHOST = "localhost:8205";

    // List of testing flags
    //
    static public String BootClassPath   = "NONE"; // Allows for -Xbootclasspath
    static public String JCOV            = "NONE"; // Allows for -Xrunjcov

    // Tracing flags for various subsystems
    //
    // Warning: Flags that begins with 'Trace' or 'x' can be set from JNLP file as
    // a debugging aid. Don't make any flags that compromises security start with 'x'
    // or 'Trace'. This feature should probably be disabled for FCS.
    static public boolean TraceDefault            = true;
    static public boolean TraceBasic              = false;
    static public boolean TraceNetwork            = false;
    static public boolean TraceSecurity           = false;
    static public boolean TraceCache              = false;
    static public boolean TraceExtensions         = false;
    static public boolean TraceTemp               = false;

    static public String  LogToHost               = null;  //Format host:port or [ipaddress]:port for IPv6 literal address

    // General
    static public boolean SupportJREinstallation  = true;  // Wether automatic JRE installation is turned on or not
    // Flags that can be set from the JNLP file as added debugging help
    static public boolean OverrideSystemClassLoader  = true; // Sets system classloader to the JNLP ClassLoader (1.4 and above only)

    // TCK Flag which sets RedirectErrors and RedirectOutput
    static public boolean TCKHarnessRun           = false;

    // the workaroud for TCP flushing problem on windows
    //  bug #4396040
    //  used to check that TCK receive a message
    static public boolean TCKResponse     = false;

    //Enumerations for the TCK States reported only to the harness
    static public final String JAVA_STARTED             = "Java Started";
    static public final String JNLP_LAUNCHING           = "JNLP Launching";
    static public final String NEW_VM_STARTING          = "JVM Starting";
    static public final String JAVA_SHUTDOWN            = "JVM Shutdown";
    static public final String CACHE_CLEAR_OK           = "Cache Clear Success";
    static public final String CACHE_CLEAR_FAILED       = "Cache Clear Failed";



    static private final Locale defaultLocale = Locale.getDefault();
    static private final String defaultLocaleString = getDefaultLocale().toString();

    static public String getDefaultLocaleString() { return defaultLocaleString; }
    static public Locale getDefaultLocale() { return defaultLocale; }

    // Global helper methods

   
    static public boolean isShortcutMode() { return _createShortcut; }
    static public boolean createShortcut() { return _createShortcut; }
    static public boolean createAssoc() { return _createAssoc; }
    static public boolean showPrompts() { return _showPrompts; }
    
    // run quietly if either quiet or silent and either import or uninstall
    static public boolean isSilentMode() {
	return (_isQuietMode || 
                (_isSilentMode && (Environment.isImportMode() || 
                Environment.isInstallMode())));
    }

    static public boolean isQuietMode() {
        return _isQuietMode;
    }

    // only enable reverse mode if import
    static public boolean isReverseMode() {
        return _isReverseMode && Environment.isImportMode();
    }
    
    static public boolean isIconImageUpdated() {
        return _isIconImageUpdated;
    }
    
    static public void setIconImageUpdated(boolean b) {
        _isIconImageUpdated = b;
    }
    
    static public boolean isSecureMode() { return _isSecureMode; }
    
    static public String[] getApplicationArgs() { return _applicationArgs; }
    
   
    static public void setCreateShortcut(boolean s) { _createShortcut = s; }
    static public void setCreateAssoc(boolean s) { _createAssoc = s; }
    static public void setShowPrompts(boolean s) { _showPrompts = s; }
    
    
    static public void setSilentMode(boolean s) { _isSilentMode = s; }
    static public void setQuietMode(boolean s) { _isQuietMode = s; }

    static public void setReverseMode(boolean s) { _isReverseMode = s; }
    
    static public void setSecureMode(boolean s) { _isSecureMode = s; }
    
    static public void setApplicationArgs(String[] s) { _applicationArgs = s; }


    static public boolean isHeadless() {
	if (!isJavaVersionAtLeast14()) {
	    // no headless mode before 1.4
	    return false;
	} else {
	    return GraphicsEnvironment.isHeadless();
	}
    }

    static public boolean havePack200() {
	return isJavaVersionAtLeast15();
    }

    private static final String _javaVersionProperty =
	System.getProperty("java.version");

    private static final boolean _atLeast14 = (
	!_javaVersionProperty.startsWith("1.2") &&
	!_javaVersionProperty.startsWith("1.3"));

    private static final boolean _atLeast15 = (_atLeast14 &&
	!_javaVersionProperty.startsWith("1.4"));

    public static boolean isJavaVersionAtLeast15() {
        return _atLeast15;
    }

    public static boolean isJavaVersionAtLeast14() {
        return _atLeast14;
    }

    /*
     *  Returns the build-id. This is stored in the jar file
     */
    static public String getBuildID() {
	String build = null;
        InputStream s = Globals.class.getResourceAsStream("/build.id");
        if (s != null) {
	    BufferedReader br = new BufferedReader(new InputStreamReader(s));
	    try {
		build = br.readLine();
	    } catch(IOException e) { /* ignore */ }
        }
        return (build == null || build.length() == 0) ? "<internal>" : build;
    }

    /*
     * Returns the java.version
     */
    static public String getJavaVersion() {
	return _javaVersionProperty;
    }

    /**
     * Returns the name of the JavaWeb Component name
     */
    static public String getComponentName() { return JAVAWS_NAME ; }

    /*
     * Returns the user-agent string
     */
    static public String getUserAgent() {
	// fix for 4676386: better user-agent value in http request header
	return "JNLP/" + JNLP_VERSION + " javaws/" + JAVAWS_VERSION + " (" + getBuildID() + ")" + " Java/" + System.getProperty("java.version");
    }

    /** Gets called with a list of arguments. It weeds out all options starting with -XX:yyy=zzz
     *  an sets the corresponding gloabal argument accordingly. It scans until it finds a non
     *  -option, i.e., a string not starting with a hypen.
     */
    static public String[] parseOptions(String[] argsArray) {
        readOptionFile();

        ArrayList args = new ArrayList();

        int pos = 0;
        boolean done = false;
        while(pos < argsArray.length) {
	    String option = argsArray[pos++];
	    if (option.startsWith("-XX:") && !done) {
		// Remove element for the arguments
		parseOption(option.substring(4), false);
	    } else {
		args.add(option);
	    }

	    // Stop scanning after the first non-option argument
	    if (!option.startsWith("-")) done = true;
        }
	//Set the options which are triggered by one.
        setTCKOptions();
        String[] newargs = new String[args.size()];
        return (String[])args.toArray(newargs);
    }

    static public void getDebugOptionsFromProperties(Properties props) {
        int i = 0;
        while(true) {
	    String option = props.getProperty("javaws.debug." + i);
	    if (option == null) return;
	    // As a security check, only the options starting with 'x' or 'Trace' are allowed from
	    // a JNLP file
	    parseOption(option, true);
	    i++;
        }
    }

    // Helper methods
    static private void setTCKOptions() {
	if (Globals.TCKHarnessRun==true) {
	    if (LogToHost == null) {
	    	Trace.println("Warning: LogHost = null");
	    }
	}
    }
    static private void parseOption(String option, boolean restricted) {
        String key = null;
        String value = null;

        int i = option.indexOf('=');
        if (i == -1) {
	    key = option;
	    value = null;
        } else {
	    key = option.substring(0, i);
	    value = option.substring(i+1);
        }

        // Check for the shorthand form for boolean arguments
        if (key.length() > 0 && (key.startsWith("-") || key.startsWith("+"))) {
	    key   = key.substring(1);
	    value = option.startsWith("+") ? "true" : "false";
        }

        // Restrict options that can be set from JNLP file
        if (restricted && !(key.startsWith("x") || key.startsWith("Trace"))) {
	    key = null;
        }

	if (key != null) {
	    setOption(key, value);
	}

    }

    static private boolean setOption(String key, String value) {
        Class stringType  = new String().getClass();
        boolean res = true;

        // Use reflection to lookup key
        try {
	    Field f = new Globals().getClass().getDeclaredField(key);
	    if ((f.getModifiers() & Modifier.STATIC) == 0) return false;

	    Class type = f.getType();
	    if (type ==  stringType)  {
		f.set(null, value);
	    } else if (type == Boolean.TYPE) {
		f.setBoolean(null, Boolean.valueOf(value).booleanValue());
	    } else if (type == Integer.TYPE) {
		f.setInt(null, Integer.parseInt(value));
	    } else if (type == Float.TYPE) {
		f.setFloat(null, Float.parseFloat(value));
	    } else if (type == Double.TYPE) {
		f.setDouble(null, Double.parseDouble(value));
	    } else if (type == Long.TYPE) {
		f.setLong(null, Long.parseLong(value));
	    } else {
		// Unsupported type
		return false;
	    }
        } catch(IllegalAccessException iae) {
	    return false;
        } catch(NoSuchFieldException nsfe) {
	    return false;
        }
        return res;
    }

    /** Try to read the '.javawsrc' property file from current directory */
    static private void readOptionFile() {
	FileInputStream fis = null;
        try {
	    fis = new FileInputStream(".javawsrc");
	} catch (FileNotFoundException fnfe) {
	    try {
	    	fis = new FileInputStream(System.getProperty("user.home") +
			File.separator + ".javawsrc");
	    } catch (FileNotFoundException fnfe1) {
		return;
	    }
	}
	try {
	    Properties props = new Properties();
	    props.load(fis);

	    // Iterate through properyies
	    Enumeration enums = props.propertyNames();
	    while(enums.hasMoreElements()) {
		String key   = (String)enums.nextElement();
		String value = props.getProperty(key);
		parseOption(key + "=" + value, false);
	    }
	} catch(IOException ioe) {  }            /* ignore */
    }

    public static String getJavawsVersion() {
	int dashIndex = JAVAWS_VERSION.indexOf("-");
	if (dashIndex > 0) {
	    return JAVAWS_VERSION.substring(0, dashIndex);
	}
	return JAVAWS_VERSION;
    }
}






