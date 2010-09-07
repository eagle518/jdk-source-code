/*
 * @(#)JVMLauncher.java	1.15 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.jvm;

import java.io.*;
import java.util.*;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.JVMParameters;
import com.sun.deploy.util.SystemUtils;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.perf.DefaultPerfHelper;
import sun.plugin2.util.*;

/** Launches a subordinate JVM with the specified parameters. */

public class JVMLauncher {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    private long startTimeUserClick;
    private String javaHome;
    private JVMParameters params;

    private Process process;
    private volatile boolean exited;
    private int exitCode = -1;
    private Exception error;

    private List/*<JVMEventListener>*/ listeners = new ArrayList/*<JVMEventListener>*/();

    /** Creates a JVMLauncher object which will launch Java from the
        given javaHome directory with the specified JVM arguments.
        (Currently it is not well specified exactly what the
        JVMParameters contains -- see the class javadoc -- but at the
        moment it encapsulates any JVM command-line arguments as well
        as the classpath, class name, program arguments, etc.) */
    public JVMLauncher(long startTimeUserClick, String javaHome,
                       JVMParameters params) {
        this.startTimeUserClick = startTimeUserClick;
        this.javaHome = javaHome;
        this.params = params;
    }

    /** Adds an event listener to this JVM launcher which receives
        events of the untimely exiting of the subordinate JVM. */
    public synchronized void addJVMEventListener(JVMEventListener listener) {
        listeners.add(listener);
    }

    /** Starts this JVMLauncher. */
    public void start() {
        long launchTime = SystemUtils.microTime();
        if(DEBUG) {
            System.out.println("JVMLauncher.start(): now - user.startApplet(): "+(launchTime-startTimeUserClick)+" us");
        }
        if ( DeployPerfUtil.isEnabled() ) {
            DeployPerfUtil.initialize(new DefaultPerfHelper(startTimeUserClick));
        }
        long t0 = DeployPerfUtil.put(0, "JVMLauncher.start() - BEGIN");

        String launchTimeProperty = "-D"+ParameterNames.JVM_LAUNCH_TIME+"="+launchTime;
        int custArgsMaxLen  = Config.getMaxCommandLineLength();
        // Fetch the JVM command line arguments
        List/*<String>*/ args;
        // First we need to find Java in the java.home location
        String java = findJava();
        if (java == null) {
            throw new RuntimeException("Unable to locate the java launcher in java.home \"" + javaHome + "\"");
        }
        exited = false;
        error = null;
        if (SystemUtil.isWindowsVista()) {
            // To avoid opening security holes by allowing arbitrary
            // low integrity ActiveX controls running inside of IE to
            // execute Java, we use a custom launcher which
            // trampolines over to java.exe inside of our desired
            // java.home and performs argument validation. We request
            // silent elevation to medium integrity for this launcher.
            String launcher = findPlugin2VistaLauncher();
            if (launcher == null) {
                throw new RuntimeException("Unable to locate the Java Plug-In's custom launcher for Windows Vista in java.home \"" +
                                           System.getProperty("java.home") + "\"");
            }
            custArgsMaxLen -= launcher.length()+1; // incl EOS
            custArgsMaxLen -= javaHome.length()+1; // incl EOS
            custArgsMaxLen -= launchTimeProperty.length()+1; // incl EOS

            // seperator, internals, os-conform, secure, .
            args = params.getCommandLineArguments(true, true, true, true, custArgsMaxLen);
            args.add(0, launcher);
            args.add(1, javaHome);
            args.add(2, launchTimeProperty);
        } else {
            custArgsMaxLen -= java.length()+1; // incl EOS
            custArgsMaxLen -= launchTimeProperty.length()+1; // incl EOS

            // !seperator, internals, os-conform, secure, .
            args = params.getCommandLineArguments(false, true, true, true, custArgsMaxLen);

            // For non-vista platforms
            // Go through the args list and validate command line arguments
            // This is similar to what we do in jp2launcher.exe
            int i, n;
            String arg = null;
            Iterator itr=args.iterator();
            for (i=0, n=args.size(); i < n; i++){
                arg = (String) itr.next();
                if (!JVMParameters.isJVMCommandLineArgument(arg))
                    break;        
            }
            
            if (i == n) {
                exited = true;
                // This can occur because of invalid JVM command line arguments
                error = new RuntimeException("Invalid arguments: no main class found");
                fireJVMExited();
                return;
            }

            if (arg!= null && !arg.equals("sun.plugin2.main.client.PluginMain")) {
                exited = true;
                // This can occur because of invalid JVM command line arguments
                error = new RuntimeException("Invalid arguments: PluginMain main class not found");
                fireJVMExited();
                return;
            }

            args.add(0, java);
            args.add(1, launchTimeProperty);
        }

        DeployPerfUtil.put("JVMLauncher.start() - post param parsing");

        if(DEBUG) {
            String arg = null;
            int argsLen=0, i, n;
            Iterator itr=args.iterator();
            for (i=0, n=args.size(); i < n; i++){
                arg = (String) itr.next();
                argsLen+=arg.length()+1;
                System.out.println("JVMLauncher.processArg["+i+"]: "+arg); 
            }
            System.out.println("JVMLauncher.processArgs total len: "+argsLen+", custArgsMaxLen: "+custArgsMaxLen);
        }
        
        DeployPerfUtil.put("JVMLauncher.start() - pre ProcessBuilder cstr");

        ProcessBuilder builder = new ProcessBuilder(args);
        DeployPerfUtil.put("JVMLauncher.start() - post ProcessBuilder cstr");

        Map/*<String,String>*/ env = builder.environment();
        DeployPerfUtil.put("JVMLauncher.start() - post ProcessBuilder env mapping");
        // Remove LD_LIBRARY_PATH entries corresponding to this JVM
        // instance, if any, as they can cause problems with the
        // subordinate JVM
        // NOTE: this is only from recollection; believe this is needed on Unix
        String ldLibraryPath = (String) env.get("LD_LIBRARY_PATH");
        if (ldLibraryPath != null) {
            String[] entries = ldLibraryPath.split(File.pathSeparator);
            String javaHome = Config.getJREHome();
            for (int i = 0; i < entries.length; i++) {
                if (entries[i].startsWith(javaHome))
                    entries[i] = null;
            }
            String newPath = null;
            for (int i = 0; i < entries.length; i++) {
                if (entries[i] != null) {
                    if (newPath == null) {
                        newPath = entries[i];
                    } else {
                        newPath = newPath + File.pathSeparator + entries[i];
                    }
                }
            }
            env.put("LD_LIBRARY_PATH", newPath);
        }
        DeployPerfUtil.put("JVMLauncher.start() - post ProcessBuilder env LD_LIBRARY_PATH");
        // Remove any CLASSPATH from the environment as we don't (and
        // shouldn't) pay attention to this in the Java Plug-In
        env.remove("CLASSPATH");
        // Note that we leave _JAVA_OPTIONS and JAVA_TOOL_OPTIONS in
        // the environment for compatibility reasons
        DeployPerfUtil.put("JVMLauncher.start() - post ProcessBuilder env cleanup ");
        try {
            DeployPerfUtil.put("JVMLauncher.start() - pre process start ");
            process = builder.start();
            DeployPerfUtil.put("JVMLauncher.start() - post process start ");
            new Thread(new JVMWatcher()).start();

            // FIXME: need to consider starting threads to drain
            // stdout/stderr from the child process; however, the
            // applet initialization setup should redirect these very
            // early in the startup process so maybe doing so isn't
            // necessary

        } catch (Exception e) {
            exited = true;
            error = e;
        }
        DeployPerfUtil.put(t0, "JVMLauncher.start() - END");
    }

    /** Returns the parameters that were used to construct this
        launcher. */
    public JVMParameters getParameters() {
        return params;
    }

    /** Clears out the user-specified command-line arguments from the
        JVMParameters. This is used to support restarting a JVM
        instance. */
    public void clearUserArguments() {
        params.clearUserArguments();
    }

    /** Indicates whether the child process exited, including whether
        it exited during startup. */
    public boolean exited() {
        return exited;
    }

    /** Returns the exit code of the process if it exited, or -1 if it
        either has not exited yet or exited during startup. */
    public int getExitCode() {
        return exitCode;
    }

    /** If an error occurred during startup of the JVM process, this
        method returns the associated Exception. */
    public Exception getErrorDuringStartup() {
        return error;
    }

    /** Forcibly destroys the process associated with the target JVM. */
    public void destroy() {
        process.destroy();
    }

    /** Returns the InputStream associated with stdout of the sub-process. */
    public InputStream getInputStream() {
        return process.getInputStream();
    }

    /** Returns the InputStream associated with stderr of the sub-process. */
    public InputStream getErrorStream() {
        return process.getErrorStream();
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private String findJava() {
        String javaName = SystemUtil.formatExecutableName("java");
        // Find this inside java.home
        String path = javaHome + File.separator + "bin" + File.separator + javaName;
        if (new File(path).exists())
            return path;
        // Try again with "jre" in the path
        path = javaHome + File.separator + "jre" + File.separator + "bin" + File.separator + javaName;
        if (new File(path).exists())
            return path;
        // No better guesses at this point
        return null;
    }

    private String findPlugin2VistaLauncher() {
        // We find this inside of our java.home
        String myJavaHome = System.getProperty("java.home");
        String launcherName = "jp2launcher.exe";
        String path = myJavaHome + File.separator + "bin" + File.separator + launcherName;
        if (new File(path).exists())
            return path;
        // Try again with "jre" in the path
        path = myJavaHome + File.separator + "jre" + File.separator + "bin" + File.separator + launcherName;
        if (new File(path).exists())
            return path;
        // No better guesses at this point
        return null;
    }

    private synchronized List/*<JVMEventListener>*/ copyListeners() {
        return (List) ((ArrayList) listeners).clone();
    }

    private void fireJVMExited() {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            JVMEventListener listener = (JVMEventListener) iter.next();
            listener.jvmExited(this);
        }
    }

    class JVMWatcher implements Runnable {
        public void run() {
            boolean done = false;
            while (!done) {
                try {
                    exitCode = process.waitFor();
                    done = true;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            exited = true;
            fireJVMExited();
        }
    }
}
