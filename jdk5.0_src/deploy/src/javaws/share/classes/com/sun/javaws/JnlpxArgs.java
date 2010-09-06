/*
 * @(#)JnlpxArgs.java	1.53 04/02/02
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import com.sun.javaws.util.GeneralUtil;
import java.util.Vector;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Iterator;
import java.io.File;
import java.net.URL;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;

/**
 *  Utility class that provides easy access to
 *  all the properterty arguments that are passed in by
 *  the native launcher.
 *
 *  The options are:
 *
 *   jnlpx.cmd  : What command to execute
 *   jnlpx.jvm  : What JVM key was used
 *   jnlpx.splashport: Port the splash screen is listning to
 *   jnlpx.home : Directory where this app. is installed
 *
 *   This file must correspond with the launcher.c file
 */
public class JnlpxArgs {
    static final private String ARG_JVM = "jnlpx.jvm";
    static final private String ARG_SPLASHPORT = "jnlpx.splashport";
    static final private  String ARG_REMOVE = "jnlpx.remove";
    static final private  String ARG_OFFLINE = "jnlpx.offline";
    static final private String ARG_HEAPSIZE = "jnlpx.heapsize";
    static final private String ARG_VMARGS = "jnlpx.vmargs";
    static final private String ARG_HOME = "jnlpx.home";

    
    static private File _currentJVMCommand = null;

    static final private String JAVAWS_JAR = (Config.isDebugMode() ? "javaws_g.jar" : "javaws.jar");

    static final private String DEPLOY_JAR = (Config.isDebugMode() ? "deploy_g.jar" : "deploy.jar");
    
    /** Returns the splash port */
    static public int getSplashPort() {
	try {
	    return Integer.parseInt(System.getProperty(ARG_SPLASHPORT, "-1"));
	} catch(NumberFormatException nfe) {
	    return -1;
	}
    }

    static public String getVMArgs() {
	return System.getProperty(ARG_VMARGS);
    }
    
    /** Returns the path for the current running JVM */
    static File getJVMCommand() {
	if (_currentJVMCommand == null) {
            String prop = System.getProperty(ARG_JVM, "").trim();
	    if (prop.startsWith("X")) {
		prop = JREInfo.getDefaultJavaPath();
	    } 
            if (prop.startsWith("\"")) prop = prop.substring(1);
            if (prop.endsWith("\"")) prop = prop.substring(0, prop.length() - 1);
	    _currentJVMCommand = new File(prop);
	}
	return _currentJVMCommand;
    }
    
    /** Returns true if the argument file should be removed */
    static public boolean shouldRemoveArgumentFile() { return getBooleanProperty(ARG_REMOVE); }
    /** Returns true if the argument file should be removed */
    
    static public void setShouldRemoveArgumentFile(String value) { System.setProperty(ARG_REMOVE, value); }
    
    /** Returns true if the application is invoked in offline mode */
    static public boolean isOffline() { return getBooleanProperty(ARG_OFFLINE); }
    
    /** Set isOffline property */
    static public void SetIsOffline() { System.setProperty(ARG_OFFLINE, "true"); }
    
    static public String getHeapSize() { return System.getProperty(ARG_HEAPSIZE); }
    
    static public long getInitialHeapSize() {
	String s = getHeapSize();
	if (s == null) return -1;
	String val = s.substring(s.lastIndexOf('=') + 1);
	String initialHeap = val.substring(0, val.lastIndexOf(','));
	return GeneralUtil.heapValToLong(initialHeap);
    }
    
    static public long getMaxHeapSize() {
	String s = getHeapSize();
	if (s == null) return -1;
	String val = s.substring(s.lastIndexOf('=') + 1);
	String maxHeap = val.substring(val.lastIndexOf(',') + 1, val.length());
	return GeneralUtil.heapValToLong(maxHeap);
    }
    
    public static boolean isCurrentRunningJREHeap(long reqMinHeap, long reqMaxHeap) {

	long currMinHeap = getInitialHeapSize();
	long currMaxHeap = getMaxHeapSize();

	Trace.println("isCurrentRunningJREHeap: passed args: " + reqMinHeap + ", " + reqMaxHeap, TraceLevel.BASIC);
	Trace.println("JnlpxArgs is " + currMinHeap + ", " + currMaxHeap, TraceLevel.BASIC);
	
	return ((currMinHeap == reqMinHeap) && (currMaxHeap == reqMaxHeap));
    }
        
    public static boolean isAuxArgsMatch(Properties props, String jnlp_vmargs) {
	String [] secureProps = Config.getSecureProperties();
	for (int i=0; i<secureProps.length; i++) {
	    String key = secureProps[i];
	    if (props.containsKey(key)) {
	        Object value = props.get(key);
		if ((value != null) && !value.equals(System.getProperty(key))) {
		    return false;
		}
	    }
	} 
	if ((jnlp_vmargs != null) && (getVMArgs() == null)) {
	    StringTokenizer st = new StringTokenizer(jnlp_vmargs);
	    while (st.hasMoreTokens()) {
		String arg = st.nextToken();
		if (Config.isSecureVmArg(arg)) {
		    return false;	
		}
	    }
	}
	return true;
    }

    private static boolean heapSizesValid(long minHeap, long maxHeap) {
	
	return !(minHeap == -1 && maxHeap == -1);

    }
    
    /** Returns a set of arguments that can be supplied to the java command in order to
     *  reproduce the current settings
     */
    static public String[] getArgumentList(String jvmCommand, long reqMinHeap, 
		long reqMaxHeap, Properties props, String jnlpArgs) {
	String heapSizeArg = "-D" + ARG_HEAPSIZE + "=NULL,NULL";
	String minHeapSizeVMArg = "";
	String maxHeapSizeVMArg = "";

	if (heapSizesValid(reqMinHeap, reqMaxHeap)) {

	    heapSizeArg = "-D" + ARG_HEAPSIZE + "=" + reqMinHeap + "," + reqMaxHeap;
	    if (reqMinHeap > 0) minHeapSizeVMArg = "-Xms" + reqMinHeap;
	    if (reqMaxHeap > 0) maxHeapSizeVMArg = "-Xmx" + reqMaxHeap;
	  
	}
	String vmArgs = getDesiredVMArgs(getVMArgs(), jnlpArgs);

		/** 
		 * Note care must be taken to ensure the proper arrangement
		 * of the args, properties and VM options should be before
		 * the Main and all javaws options must be after the Main.
		 */
	String[] args = {

		"-Xbootclasspath/a:" +
		    Config.getJavaHome() + File.separator + "lib" +
   		    File.separator + JAVAWS_JAR + File.pathSeparator +
		    Config.getJavaHome() + File.separator + "lib" +
		    File.separator + DEPLOY_JAR,

		"-classpath", 
		    File.pathSeparator +
		    Config.getJavaHome() + File.separator + "lib" +
		    File.separator + DEPLOY_JAR,

	    	minHeapSizeVMArg,
		maxHeapSizeVMArg,
		(vmArgs != null ? ("-D" + ARG_VMARGS + "=" + vmArgs) : ""),
	    	"-D" + ARG_JVM + "=" + jvmCommand  ,
		"-D" + ARG_SPLASHPORT + "=" + getSplashPort(),
		"-D" + ARG_HOME + "=" + Config.getJavaHome() + 
					File.separator + "bin",
		"-D" + ARG_REMOVE + "=" + 
				(shouldRemoveArgumentFile() ? "true" : "false"),
		"-D" + ARG_OFFLINE + "=" + (isOffline() ? "true" : "false"),
		heapSizeArg,
		"-Djava.security.policy=" + getPolicyURLString(),
		"-DtrustProxy=true",
		"-Xverify:remote",
		useJCOV(),
		useBootClassPath(),
		useJpiProfile(),
		useDebugMode(),
		useDebugVMMode(),
		"com.sun.javaws.Main",
		setTCKHarnessOption(),
		useLogToHost()
	};
	
	/** a "" argument is not allowed, so they get stripped here */
	int count = 0;
	for(int i = 0; i < args.length; i++) {
	    if (! args[i].equals("")) count++;
	}

	String [] vmargs = getVMArgList(props, jnlpArgs);
        int vmcount = vmargs.length;
	
	String returnArgs[] = new String[count+vmcount];
	int i=0;
	for (i=0; i<vmcount; i++) {
	    returnArgs[i] = vmargs[i];
	}

	for(int j = 0; j < args.length; j++) {
	    if (! args[j].equals("")) returnArgs[i++] = args[j];
	}

	return returnArgs;
    }

    static String getPolicyURLString() {
	String path = Config.getJavaHome() + File.separator + "lib" + 
		File.separator + "security" + File.separator + "javaws.policy";
	String path1 = path;
	try {
	    URL url = new URL("file", "", path);
	    path1 = url.toString();
	} catch (Exception e) {
	}
	return path1;
/*
        Trace.println("path is: "+path);
        Trace.println("path1 is: "+path1);
	String path2 = "file:" + path;
        Trace.println("path2 is: "+path2);
        Trace.println ("path1.compareTo(path2) = " + path1.compareTo(path2));
	return path2;
*/
    }

    static private String getDesiredVMArgs(String oldVMArgs, 
					 String jnlpVmArgs) {
	StringTokenizer st;
	if (oldVMArgs == null) { 
	    if (jnlpVmArgs != null) {
		String ret = "";
	        st = new StringTokenizer(jnlpVmArgs, " \t\n\r\f\"");
	        while(st.hasMoreTokens()) {
		    String arg = st.nextToken();
                    if (Config.isSecureVmArg(arg)) {
			if (ret.length() == 0) {
			    ret = arg;
			} else {
                            ret += " " + arg;
			}
		    }
                }
		if (ret.length() > 0) {
		    return ret;
		}
            }
        }
	return oldVMArgs;
    }	


    static private String[] getVMArgList(Properties props, String jnlpVmArgs) {
	Vector v = new Vector();
	String s = null;

	if ((s = getVMArgs()) != null) {
	    StringTokenizer st = new StringTokenizer(s, " \t\n\r\f\"");
	    while(st.hasMoreTokens()) {
		v.add(st.nextToken());
	    }
	}

	if (jnlpVmArgs != null) {
	    StringTokenizer st = new StringTokenizer(jnlpVmArgs, " \t\n\r\f\"");
	    while(st.hasMoreTokens()) {
		String arg = st.nextToken();
		if (Config.isSecureVmArg(arg)) {
		    if (!v.contains(arg)) {
		        v.add(arg);
		    }
		}
	    }
	}

	String [] secureProps = Config.getSecureProperties();
	for (int index=0; index<secureProps.length; index++) {
	    String key = secureProps[index];
            if (props.containsKey(key)) {
		String arg = "-D"+key+"="+props.get(key);
		if (!v.contains(arg)) {
	            v.add(arg);
		}
	    }
        }

	String [] args = new String[v.size()];
	for (int i = 0; i < v.size(); i++) {
	    args[i] = new String((String)v.elementAt(i));
	}
	return args;
    }						     

    /** set the logHost */
    static public String useLogToHost() {
        if (Globals.LogToHost != null) {
            return "-XX:LogToHost="+Globals.LogToHost;
        }
        return "";
    }
    
    /** Set the TCK option */
    static public String setTCKHarnessOption() {
        if (Globals.TCKHarnessRun == true) {
            return "-XX:TCKHarnessRun=true";
        }
        return "";
    }
    
    /** for setting BootClassPath, helpful with Silk testing */
    static public String useBootClassPath() {
	if (Globals.BootClassPath.equals("NONE")) {
	    return "";
	} else {
	    return "-Xbootclasspath" + Globals.BootClassPath;
	}
    }

    static public String useJpiProfile() {
        String profileString=System.getProperty("javaplugin.user.profile");
	if (profileString != null) {
	    return ("-Djavaplugin.user.profile="+profileString);
	}
	return "";
    }
    
    /** for JCOV tool, used in coverage testing */
    static public String useJCOV() {
	if (Globals.JCOV.equals("NONE")) {
	    return "";
	} else {
	    return "-Xrunjcov:file=" + Globals.JCOV;
	}
    }

    static public String useDebugMode() {
	if (Config.isDebugMode()) {
	    return "-Ddeploy.debugMode=true";
	} else {
	    return "";
	}
    }

    static public String useDebugVMMode() {
	if (Config.isDebugVMMode()) {
	    return "-Ddeploy.useDebugJavaVM=true";
	} else {
	    return "";
	}
    }


    /** Removes the argument if neccesary */
    static public void removeArgumentFile(String args[]) {	
	// Remove argument, if we are suppose to
	if (JnlpxArgs.shouldRemoveArgumentFile() && args != null && args.length > 0) {	   
	    new File(args[0]).delete();
	}
    }
    
    /** Method to verify that all properties are setup correctly */
    static void verify() {
	// Check that all properties are setup correctly

	Trace.println("Java part started", TraceLevel.BASIC);
	Trace.println(ARG_JVM        + ": " + getJVMCommand(), TraceLevel.BASIC);
	Trace.println(ARG_SPLASHPORT + ": " + getSplashPort(), TraceLevel.BASIC);
	Trace.println(ARG_REMOVE     + ": " + shouldRemoveArgumentFile(), TraceLevel.BASIC);
	Trace.println(ARG_HEAPSIZE   + ": " + getHeapSize(), TraceLevel.BASIC);
	
    }
    
    static private boolean getBooleanProperty(String key) {
	String s = System.getProperty(key, "false");
	return s != null && s.equals("true");
    }
}



