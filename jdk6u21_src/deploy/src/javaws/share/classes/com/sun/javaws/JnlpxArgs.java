/*
 * @(#)JnlpxArgs.java	1.77 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.util.Vector;
import java.util.List;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Iterator;
import java.util.Enumeration;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;

import com.sun.deploy.util.StringQuoteUtil;
import com.sun.deploy.util.JVMParameters;
import com.sun.deploy.util.Property;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.progress.ProgressListener;
import com.sun.deploy.cache.LocalApplicationProperties;

import com.sun.javaws.jnl.JREDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDescFactory;
import com.sun.javaws.jnl.LaunchSelection;
import com.sun.javaws.util.GeneralUtil;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.exceptions.LaunchDescException;
import com.sun.javaws.exceptions.JNLPException;

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
    static final public String ARG_JVM = "jnlpx.jvm";
    static final public String ARG_SPLASHPORT = "jnlpx.splashport";
    static final public  String ARG_REMOVE = "jnlpx.remove";
    static final public  String ARG_OFFLINE = "jnlpx.offline";
    static final public String ARG_HEAPSIZE = "jnlpx.heapsize";
    static final public String ARG_VMARGS = "jnlpx.vmargs";
    static final public String ARG_HOME = "jnlpx.home";

    static final public String ARG_RELAUNCH = "jnlpx.relaunch";

    static private File _currentJVMCommand = null;

    static final private String JAVAWS_JAR = "javaws.jar";

    static final private String DEPLOY_JAR = "deploy.jar";
    
    static final private String PLUGIN_JAR = "plugin.jar";

    static final private Vector fileReadWriteList = new Vector();

    /** Returns the splash port */
    static public int getSplashPort() {
        try {
            return Integer.parseInt(System.getProperty(ARG_SPLASHPORT, "-1"));
        } catch(NumberFormatException nfe) {
            return -1;
        }
    }

    static public String getVMArgs() {
        return StringQuoteUtil.unquoteIfEnclosedInQuotes(System.getProperty(ARG_VMARGS));
    }

    static public boolean getIsRelaunch() {
        return getBooleanProperty(ARG_RELAUNCH);
    }

    /** Returns the path for the current running JVM */
    static public File getJVMCommand() {
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

    private static boolean heapSizesValid(long minHeap, long maxHeap) {

        return !(minHeap == -1 && maxHeap == -1);

    }

    /** Returns a set of arguments that can be supplied to the java command in order to
     *  reproduce the current settings
     */
    static private List/*String*/ getArgumentList(String jvmCommandStd, 
                long reqMinHeap, long reqMaxHeap, JVMParameters jvmParams, 
                boolean secure, int custArgsMaxLen) {
        String heapSizeArg = "-D" + ARG_HEAPSIZE + "=NULL,NULL";

        if (heapSizesValid(reqMinHeap, reqMaxHeap)) {
            heapSizeArg = "-D" + ARG_HEAPSIZE + "=" + reqMinHeap + "," + reqMaxHeap;
        }

        String s = getVMArgs();
        if (s != null) {
            // don't modify the given JVMParameters
            jvmParams = jvmParams.copy();
            // add the getVMArgs()
            jvmParams.parse(s);
        }

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
                    File.separator + DEPLOY_JAR +File.pathSeparator +
                    Config.getJavaHome() + File.separator + "lib" +
                    File.separator + PLUGIN_JAR ,

                "-classpath",
                    Config.getJavaHome() + File.separator + "lib" +
                    File.separator + DEPLOY_JAR,

                null, // earmark vmArgsProperty ..
                "-D" + ARG_JVM + "=" + jvmCommandStd  ,
                "-D" + ARG_SPLASHPORT + "=" + getSplashPort(),
                "-D" + ARG_HOME + "=" + Config.getJavaHome() +
                                        File.separator + "bin",
                "-D" + ARG_REMOVE + "=" +
                                (shouldRemoveArgumentFile() ? "true" : "false"),
                "-D" + ARG_OFFLINE + "=" + (isOffline() ? "true" : "false"),
                "-D" + ARG_RELAUNCH + "=true",
                heapSizeArg,
                "-Djava.security.policy=" + getPolicyURLString(),
                "-DtrustProxy=true",
                "-Xverify:remote",
                useJCOV(),
                useBootClassPath(),
                useJpiProfile(),
                useDebugMode(),
                useDebugVMMode(),
                "-Dsun.awt.warmup=true",
                "com.sun.javaws.Main",
                (secure ? "-secure" : ""),
                setTCKHarnessOption(),
                useLogToHost()
        };

        for(int j = 0; j < args.length; j++) {
            if(args[j]!=null) {
                custArgsMaxLen -= args[j].length()+1; // incl. seperator
            }
        }

        // no sepereator, no internals, os-conform, include-secure:=<secure>
        // half of custArgsMaxLen, for vmArgsProperty, -20 for property and quoting
        List/*String*/ returnArgs = jvmParams.getCommandLineArguments(false, false, true, secure, (custArgsMaxLen/2)-20);

        String vmArgs = StringQuoteUtil.getStringByCommandList(returnArgs);
        custArgsMaxLen -= vmArgs.length();

        Property vmArgsProperty = new Property(ARG_VMARGS, vmArgs);
        String vmArgsPropertyStr = vmArgsProperty.toString(true); // os dependent representation
        if(custArgsMaxLen<vmArgsPropertyStr.length()) {
            // Ooops .. drop it, remainding commandline too short
            vmArgsPropertyStr=null; 
            Trace.println("JnlpxArgs.getArgumentList: Internal Error: remaining custArgsMaxLen: "+custArgsMaxLen+
                          " < vmArgsPropertyStr.length: "+vmArgsPropertyStr.length()+
                          " dropping vmArgsPropertyStr" );
        }

        for(int j = 0; j < args.length; j++) {
            if(args[j]==null) {
                if(vmArgsPropertyStr!=null) {
                    returnArgs.add(vmArgsPropertyStr);
                    vmArgsPropertyStr=null;
                }
            } else {
                if (args[j].length()>0) {
                    returnArgs.add(args[j]);
                }
            }
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
    static public void removeArgumentFile(String arg) {
        // Remove argument, if we are suppose to
        if (JnlpxArgs.shouldRemoveArgumentFile() && arg != null) {
            new File(arg).delete();
        }
    }

    /** Method to verify that all properties are setup correctly */
    public static void verify() {
        // Check that all properties are setup correctly

        if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
            Trace.println("Java part started", TraceLevel.BASIC);
            Trace.println(ARG_JVM        + ": " + getJVMCommand(), TraceLevel.BASIC);
            Trace.println(ARG_SPLASHPORT + ": " + getSplashPort(), TraceLevel.BASIC);
            Trace.println(ARG_REMOVE     + ": " + shouldRemoveArgumentFile(), TraceLevel.BASIC);
            Trace.println(ARG_HEAPSIZE   + ": " + getHeapSize(), TraceLevel.BASIC);
        }

    }

    static private boolean getBooleanProperty(String key) {
        String s = System.getProperty(key, "false");
        return s != null && s.equals("true");
    }

    static public Vector getFileReadWriteList() {
        return fileReadWriteList;
    }

    static protected Process execProgram(JREInfo jreInfo, String[] args,
        long minHeap, long maxHeap, JVMParameters jvmParams, boolean secure) throws IOException {
        String javacmd = null;
        String stdjavacmd = null;
        stdjavacmd = jreInfo.getPath();
        if (Config.isDebugMode() && Config.isDebugVMMode()) {
            javacmd = jreInfo.getDebugJavaPath();
        } else {
            javacmd = jreInfo.getPath();
        }
        if ((javacmd.length() == 0) || (stdjavacmd.length() == 0)) {
            throw new IllegalArgumentException("must exist");
        }

        int custArgsMaxLen = Config.getMaxCommandLineLength();
        for(int i = 0; i < args.length; i++) {
            if(args[i]!=null) {
                custArgsMaxLen -= args[i].length()+1; // incl. seperator
            }
        }
        custArgsMaxLen -= javacmd.length()+1; // incl. seperator

        List /*String*/ jnlpxs = getArgumentList( stdjavacmd, 
            minHeap, maxHeap, jvmParams, secure, custArgsMaxLen);
        int cmdssize = 1 + jnlpxs.size() + args.length;
        String[] cmds = new String[cmdssize];
        int pos = 0;
        cmds[pos++] = javacmd;

        for(int i = 0; i < jnlpxs.size(); i++) {
            cmds[pos++] = (String)jnlpxs.get(i);
        }

        for(int i = 0; i < args.length; i++) {
            cmds[pos++] = args[i];
        }

        if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
            Trace.println("Launching new JRE version: " + jreInfo, TraceLevel.BASIC);
            Trace.println("\t jvmParams: "+jvmParams, TraceLevel.BASIC);
            for(int i = 0; i < cmds.length; i++) {
                Trace.println("cmd " + i + " : " + cmds[i], TraceLevel.BASIC);
            }
        }

        if (Globals.TCKHarnessRun) {
            Main.tckprintln(Globals.NEW_VM_STARTING);
        }
        Trace.flush();
        return Runtime.getRuntime().exec(cmds);
    }

    static public void executeInstallers(ArrayList files, 
                                         final ProgressListener progress) 
        throws ExitException 
    {

        // Let progress window show launching behavior
        if (progress.getOwner() != null) {
            String title = 
                ResourceManager.getString("progress.title.installer");


            progress.showLaunchingApplication(title);
            // We will just show this window a short while, and then remove
            // it. The installer is going to pop up pretty quickly.
            new Thread(new Runnable() {
                        public void run() {
                            try {
                                Thread.sleep(2500);
                            } catch(Exception ioe) { /* ignore */ };
                            progress.setVisible(false);
                        }}).start();
        }

        for(int i = 0; i < files.size(); i++) {
            File jnlpFile = (File)files.get(i);

            try {
                LaunchDesc ld = LaunchDescFactory.buildDescriptor(jnlpFile, null, null, null);
                LocalApplicationProperties lap = 
                        Cache.getLocalApplicationProperties(jnlpFile.getPath());
                lap.setLocallyInstalled(false);
                lap.store();

                // proceed with normal installation

                Trace.println("Installing extension: " + jnlpFile, TraceLevel.EXTENSIONS);

                String[] args = new String[]{ "-installer",
                                              jnlpFile.getAbsolutePath() };
                // Determine JRE to run on
                JREInfo jreInfo = ld.selectJRE();
                if (jreInfo == null) {
                    progress.setVisible(false);
                    // No JRE to run application (FIXIT: Wrong exception)
                    LaunchDescException lde = new LaunchDescException(ld,
                        ResourceManager.getString("launch.error.missingjreversion"), null);
                    throw new ExitException(lde, ExitException.LAUNCH_ERROR);
                }

                // remeber whether to removeArgumentFile for the application
                boolean removeArgumentFile = JnlpxArgs.shouldRemoveArgumentFile();

                // Exec installer and wait for it to complete
                // should not remove installer JNLP file in cache
                JnlpxArgs.setShouldRemoveArgumentFile("false");

                LaunchSelection.MatchJREIf jreMatcher = ld.getJREMatcher();
                JVMParameters jvmParams = jreMatcher.getSelectedJVMParameters();
                JREDesc jreDesc = jreMatcher.getSelectedJREDesc();
                long minHeap = jreDesc.getMinHeap();
                long maxHeap = jreDesc.getMaxHeap();

                Process p = JnlpxArgs.execProgram(jreInfo, args, minHeap, maxHeap, jvmParams, false);
                EatInput.eatInput(p.getErrorStream());
                EatInput.eatInput(p.getInputStream());
                p.waitFor();

                // reset removeArgumentFile flag for this application
                JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(removeArgumentFile));

                // Validate that installation succeded
                lap.refresh();
                if (lap.isRebootNeeded()) {
                    boolean doboot = false;
                    ExtensionInstallHandler eih =
                        ExtensionInstallHandler.getInstance();
                    if (eih != null &&
                        eih.doPreRebootActions(progress.getOwner())) {
                            doboot = true;
                    }
                    // set locally installed to be true
                    lap.setLocallyInstalled(true);
                    lap.setRebootNeeded(false);
                    lap.store();
                    if (doboot && eih.doReboot()) {
                        throw new ExitException(null, ExitException.REBOOT);
                    }
                }
                if (!lap.isLocallyInstalled()) {
                    progress.setVisible(false);
                    // Installation failed
                    throw new ExitException(new LaunchDescException(ld,
                        ResourceManager.getString("Launch.error.installfailed"), null),
                        ExitException.LAUNCH_ERROR);
                }
            } catch(JNLPException je) {
                progress.setVisible(false);
                throw new ExitException(je, ExitException.LAUNCH_ERROR);
            } catch(IOException io) {
                progress.setVisible(false);
                throw new ExitException(io, ExitException.LAUNCH_ERROR);
            } catch(InterruptedException iro) {
                progress.setVisible(false);
                throw new ExitException(iro, ExitException.LAUNCH_ERROR);
            }
        }
    }

    static public void executeUninstallers(ArrayList files)
        throws ExitException {

        for(int i = 0; i < files.size(); i++) {
            File jnlpFile = (File)files.get(i);

            try {
                LaunchDesc ld = LaunchDescFactory.buildDescriptor(jnlpFile, null, null, null);
                LocalApplicationProperties lap = 
                       Cache.getLocalApplicationProperties(jnlpFile.getPath());
                // proceed with normal installation

                Trace.println("uninstalling extension: " + jnlpFile,
			      TraceLevel.EXTENSIONS);
		// we should only do uninstall if the ld is an installer
		if (ld.isInstaller() == false) {
		    throw new ExitException(null, ExitException.LAUNCH_ERROR);
		}
 
		// only apply -secure if installer jnlp request all permission
		String[] args = new String[]{ "-silent",
					      "-installer",
					      jnlpFile.getAbsolutePath() };
              
                // Determine JRE to run on
                JREInfo jreInfo = ld.selectJRE();
                if (jreInfo == null) {
                    // No JRE to run application (FIXIT: Wrong exception)
                    LaunchDescException lde = new LaunchDescException(ld,
                        ResourceManager.getString(
                        "launch.error.missingjreversion"), null);
                    throw new ExitException(lde, ExitException.LAUNCH_ERROR);
                }

                LaunchSelection.MatchJREIf jreMatcher = ld.getJREMatcher();
                JVMParameters jvmParams = jreMatcher.getSelectedJVMParameters();
                JREDesc jreDesc = jreMatcher.getSelectedJREDesc();
                long minHeap = jreDesc.getMinHeap();
                long maxHeap = jreDesc.getMaxHeap();

                Process p = JnlpxArgs.execProgram(jreInfo, args, minHeap, maxHeap, jvmParams, false);
                EatInput.eatInput(p.getErrorStream());
                EatInput.eatInput(p.getInputStream());
                p.waitFor();

                lap.refresh();
                if (lap.isRebootNeeded()) {
                    boolean doboot = false;
                    ExtensionInstallHandler eih =
                        ExtensionInstallHandler.getInstance();
                    if (eih != null && eih.doPreRebootActions(null)) {
                            doboot = true;
                    }
                    lap.setRebootNeeded(false);
                    lap.setLocallyInstalled(false);
                    lap.store();
                    if (doboot && eih.doReboot()) {
                        throw new ExitException(null, ExitException.REBOOT);
                    }
                }
            } catch(JNLPException je) {
                throw new ExitException(je, ExitException.LAUNCH_ERROR);
            } catch(IOException io) {
                throw new ExitException(io, ExitException.LAUNCH_ERROR);
            } catch(InterruptedException iro) {
                throw new ExitException(iro, ExitException.LAUNCH_ERROR);
            }
        }
    }

    private static String sizeString(long size) {
        if (size > 1024 * 1024) {
            return "" + size/(1024*1024) + "Mb";
        }
        return "" + size + "bytes";
    }

    private static class EatInput implements Runnable {
        private InputStream _is;

        EatInput(InputStream is) {
            _is = is;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            try {
                int n=0;
                while(n != -1) {
                    n = _is.read(buffer);
                }
            } catch(IOException ioe) { /* just ignore */ }
        }

        private static void eatInput(InputStream is) {
            EatInput eater = new EatInput(is);
            new Thread(eater).start();
        }
    }


}



