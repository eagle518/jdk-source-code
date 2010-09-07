/*
 * @(#)JavaRunTime.java	1.81 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A loaded and running Java Virtual machine is initialized and
 * released in the JavaRunTime.
 * This is the opportunity for startup code
 *
 * @version         1.2.1
 * @author        Jerome Dochez
 *
 *
 * Modified by: * Rita Fisher      Apr. 04, 2001       Bug fix 4421334 - made Java Console truly disable-able (there are now
 *                                      three possible states for Java Console - full size, hidden (icon/system
 *                                      tray) or no console).
 * 
 */
import java.io.File;
import java.lang.reflect.Method;
import java.util.logging.Level;
import java.util.logging.Logger;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.perf.NativePerfHelper;
import com.sun.deploy.util.ConsoleHelper;
import com.sun.deploy.util.ConsoleWindow;
import com.sun.deploy.util.ConsoleController14;
import com.sun.deploy.util.ConsoleTraceListener;
import com.sun.deploy.util.FileTraceListener;
import com.sun.deploy.util.LoggerTraceListener;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.DeploySysRun;
import sun.plugin.util.UserProfile;
import sun.plugin.util.PluginSysUtil;
import com.sun.deploy.config.Config;

public class JavaRunTime 
{
    /**
     * <p>
     * Method called once when the javaplugin is loaded. This is the opportunity
     * to add some initialization function. For convenience the java home is 
     * passed to be able to know where the java environement resides
     *
     * @param javaHome                java home directory
     * @param bridgeLibPath        Activator path
     * @param userHome                user home dir
     */
    public static void initEnvironment(String javaHome, String bridgeLibPath, String userHome) {
        initEnvironment(javaHome, bridgeLibPath, userHome, true);
    }

    /**
     * <p>
     * Method called once when the javaplugin is loaded. This is the opportunity
     * to add some initialization function. For convenience the java home is 
     * passed to be able to know where the java environement resides
     *
     * @param javaHome                java home directory
     * @param bridgeLibPath        Activator path
     * @param userHome                user home dir
     * @param redirectStdio        whether to redirect stdout and stderr
     */
    public static void initEnvironment(String javaHome, String bridgeLibPath, String userHome, boolean redirectStdio) 
    {
        if (DeployPerfUtil.isEnabled() == true) {
            DeployPerfUtil.initialize(new NativePerfHelper());
        }

        DeployPerfUtil.put("START - Java   - JVM - " +
                           "JavaRunTime.initEnvironment");

        DeployPerfUtil.put("START - Java   - JVM - " +
                           "JavaRunTime.initEnvironment - instantiate PluginSysUtil");
        DeploySysRun.setOverride(new PluginSysUtil());
        DeployPerfUtil.put("END   - Java   - JVM - " + 
                           "JavaRunTime.initEnvironment - instantiate PluginSysUtil");

        DeployPerfUtil.put("START - Java   - JVM - " +
                           "JavaRunTime.initEnvironment - set user.home property");
        java.util.Properties systemProps = System.getProperties();
        systemProps.put("java.home", javaHome );  
        if (userHome==null) {
            systemProps.put("user.home", javaHome);
        }
        else {
            systemProps.put("user.home", userHome);
        }
        DeployPerfUtil.put("END   - Java   - JVM - " + 
                           "JavaRunTime.initEnvironment - set user.home property");

	// Load the properties
        // Need to do this reflectively to break dependence on AppletViewer
        try {
            Class c = Class.forName("sun.plugin.AppletViewer");
            Method m = c.getMethod("loadPropertiesFiles", null);
            m.invoke(null, null);
        } catch (Throwable t) {
        }

        // Redirect System.out/System.err
        DeployPerfUtil.put("START - Java   - JVM - " +
                           "JavaRunTime.initEnvironment - setup trace redirect");
        if (redirectStdio) {
            com.sun.deploy.util.Trace.redirectStdioStderr();
        }
        DeployPerfUtil.put("END   - Java   - JVM - " + 
                           "JavaRunTime.initEnvironment - setup trace redirect");

        DeployPerfUtil.put("END   - Java   - JVM - " + 
                           "JavaRunTime.initEnvironment");
    }     

    /* pending actions (until console window is created */
    private static StringBuffer consoleLog = new StringBuffer();
    private static boolean pendingTraceListener = false;

    /**
     * <p>
     * Return the console window for this running Java Virtual Machine.
     * If the console is not created yet, create a new Java Console.
     * 
     * Note that we return "Object" instead of "ConsoleWindow" from this method
     * to avoid needing to load dependent classes referenced by the return type,
     * in particular Swing classes like javax.swing.JFrame.
     *
     * @return console window object
     * </p>
     */
    private synchronized static Object getJavaConsole()
    {
        // Initialize tracing environment
        initTraceEnvironment();

        if (console == null)
        {
            // Create console
            console = ConsoleWindow.create(controller);

            // Display version information in console
            appendStringToConsole(ConsoleHelper.displayVersion());

            // Flush pending log
            appendStringToConsole(consoleLog.toString());
            consoleLog = null;  //console is never deleted
                                //and therefore we will never need
                                //buffer again

            // Install pending trace listener
            if (pendingTraceListener) {
                installConsoleTraceListener();
            }
        }

        return console;
    }

    /**
     * <p>
     * Initialize the trace file facilities.
     * </p>
     */
    public synchronized static void initTraceEnvironment() {
        initTraceEnvironment(null);
    }

    /**
     * <p>
     * Initialize the trace file facilities. The optional
     * ConsoleController14 argument allows overriding of the default
     * PluginConsoleController.
     * </p>
     */
    public synchronized static void initTraceEnvironment(ConsoleController14 cctrl)
    {
        // Return if environment has been initialized
        if (traceInit) 
            return;

        traceInit = true;
        
        File logDir = new File(UserProfile.getLogDirectory());
        if(!logDir.exists()) {
            logDir.mkdirs();
        }

        // Create Java Console
        if (cctrl != null) {
            controller = cctrl;
        } else {
            // Need to instantiate this reflectively to break dependence on PluginConsoleController
            try {
                controller = (ConsoleController14)
                    Class.forName("sun.plugin.util.PluginConsoleController").newInstance();
            } catch (Throwable t) {
            }
        }

        // Add console tracing
        ctl = new ConsoleTraceListener(controller);
        Trace.addTraceListener(ctl);                        
        
        boolean pluginTrace, deployTrace;
        pluginTrace = Config.getBooleanProperty("javaplugin.trace");
        deployTrace = Config.getBooleanProperty(Config.TRACE_MODE_KEY);

        String initTraceLevel = null;

        if (pluginTrace) {
            initTraceLevel = Config.getProperty("javaplugin.trace.option");
        } else if (deployTrace) {
            initTraceLevel = Config.getProperty(Config.TRACE_LEVEL_KEY);
        }

        if (pluginTrace || deployTrace) {
            try {
                if (initTraceLevel == null || initTraceLevel.equals("")) {
                    // set trace level to MAX
                    Trace.setBasicTrace(true);
                    Trace.setNetTrace(true);
                    Trace.setCacheTrace(true);
                    Trace.setTempTrace(true);
                    Trace.setSecurityTrace(true);
                    Trace.setExtTrace(true);
                    Trace.setLiveConnectTrace(true);
                } else {
                    Trace.setInitialTraceLevel(initTraceLevel);
                }
                
                // Add file tracing
                File traceFile = null;
                String traceFilename = 
                        Config.getProperty(Config.JPI_TRACE_FILE_KEY);
                if (traceFilename != null && traceFilename != "") {                    
                    traceFile = new File(traceFilename);
                    File parent = traceFile.getParentFile();
                    if (parent != null) {
                        parent.mkdirs();
                    }
                    traceFile.createNewFile();
                    if (traceFile.exists() == false) {
                        traceFile = null;
                    }
                }
                
                if (traceFile == null) {
                    // Check whether user wants to overwrite trace file.
                    Boolean overWrite =
                            (Boolean) java.security.AccessController.doPrivileged(
                            new sun.security.action.GetBooleanAction(
                            "javaplugin.outputfiles.overwrite"));
                    
                    if (overWrite.equals(Boolean.TRUE)){
                        // Assemble full file name, including directory.
                        StringBuffer nameBuffer = new StringBuffer();
                        nameBuffer.append(logDir);
                        nameBuffer.append(File.separator);
                        nameBuffer.append(Config.PLUGIN_OUTPUTFILE_PREFIX);
                        String noDotVersion =
                                (String) java.security.AccessController.doPrivileged(
                                new sun.security.action.GetPropertyAction(
                                "javaplugin.nodotversion"));
                        
                        nameBuffer.append(noDotVersion);
                        nameBuffer.append(Config.OUTPUTFILE_TRACE_SUFFIX);
                        traceFile = new File(nameBuffer.toString());
                    }else{
                        // Create temp file with new name.
                        traceFile= Trace.createTempFile(
                                Config.PLUGIN_OUTPUTFILE_PREFIX,
                                Config.OUTPUTFILE_TRACE_SUFFIX, logDir);
                    }
                }
                
                if (traceFile.canWrite() || (!traceFile.exists() &&
                        logDir.canWrite())) {
                    Trace.addTraceListener(new FileTraceListener(traceFile, false));
                } else {
                    Trace.println("can not write to trace file: "+traceFile,
                            TraceLevel.BASIC);
                }
            } catch (Exception e) {
                Trace.println("can not write to trace file", TraceLevel.BASIC);
                Trace.ignored(e);
            }
        }
        
        if (Config.getBooleanProperty(Config.LOG_MODE_KEY)) {
            try {
                File logFile = null;
                String logFilename = 
                        Config.getProperty(Config.JPI_LOG_FILE_KEY);
                
                if (logFilename != null && logFilename != "") {
                    logFile = new File(logFilename);
                    File parent = logFile.getParentFile();
                    if (parent != null) {
                        parent.mkdirs();
                    }
                    logFile.createNewFile();
                    if (logFile.exists() == false) {
                        logFile = null;
                    }
                }
                if (logFile == null) {
                    // Create log trace listener
                    logFile = Trace.createTempFile(Config.PLUGIN_OUTPUTFILE_PREFIX,
                            Config.OUTPUTFILE_LOG_SUFFIX, logDir);
                }
                logDir.mkdirs();
                if (logFile.canWrite() || (!logFile.exists() &&
                        logDir.canWrite())) {
                    LoggerTraceListener ltl = new LoggerTraceListener("sun.plugin",
                            logFile.getPath());
                    ltl.getLogger().setLevel(Level.ALL);
                    controller.setLogger(ltl.getLogger());
                    // Add log tracing
                    Trace.addTraceListener(ltl);
                } else {
                    Trace.println("can not write to log file: " + logFile,
                            TraceLevel.BASIC);
                }
            } catch (Exception e) {
                Trace.println("can not write to log file", TraceLevel.BASIC);
                Trace.ignored(e);
            }
            
        }
    }
    

    /**
     * <p>
     * Checks if the Java Console Window is visible.
     *
     * @return true if visible.
     * </p>
     */
    public static boolean isJavaConsoleVisible()  
    {
        if (console == null)
            return false;
        else
            return console.isConsoleVisible();
    }


    /**
     * <p>
     * Show or hide Java Console Window.
     *
     * @param visible true if to show, else to hide.
     * </p>
     */
    public static void showJavaConsole(final boolean visible)  
    {      
            // Notice that setVisible MUST be called from
            // the event dispatch thread because the Swing
            // component may have been realized. Otherwise,
            // it may cause deadlock.
            //
            try
            {
                PluginSysUtil.invokeAndWait(new Runnable()
                {
                    public void run()
                    {
                        ConsoleWindow cw = (ConsoleWindow) getJavaConsole();
                        if(cw != null)
                            cw.showConsole(visible);
                    }
                });
            }
            catch (Throwable e)
            {
                e.printStackTrace();
            }
    }


    /**
     * <p>
     * Show or hide Java Console Window later.
     *
     * @param visible true if to show, else to hide.
     * </p>
     */
    public static void showJavaConsoleLater(final boolean visible)  
    {

        // Notice that setVisible MUST be called from
        // the event dispatch thread because the Swing
        // component may have been realized. Otherwise,
        // it may cause deadlock.
        //
        try
        {
            PluginSysUtil.invokeLater(new Runnable()
            {
                public void run()
                {
                ConsoleWindow cw = (ConsoleWindow) getJavaConsole();
                if(cw != null)
                        cw.showConsole(visible);
                }
            });
        }
        catch (Throwable e)
        {
            e.printStackTrace();
        }
    }

     /**
     * <p>
     * Print a message to Java Console Window.
     *
     * @param msg Message to be printed.
     * </p>
     */
    public static void printToJavaConsole(String msg)   
    {
        ctl.print(msg + "\n");
    }

    /**
     * Appends a string to the console window.
     */
    public static void appendStringToConsole(String str) {
        if (console != null) {
            ConsoleWindow win = (ConsoleWindow) getJavaConsole();
            win.append(str);
            win.append("\n");
        } else {
            //avoid creation of console until it is needed
            consoleLog.append(str);
            consoleLog.append("\n");
        }
    }

    private static boolean installed = false;
    /**
     * Install the console trace listener into the Java Console
     * window. This causes any buffered output to be flushed to the
     * console and should be called at the beginning of time after the
     * console window has been displayed.
     */
    public static synchronized void installConsoleTraceListener() {
        if (!installed) {
            if (console != null) {
                ctl.setConsole((ConsoleWindow) getJavaConsole());
                installed = true;
                pendingTraceListener = false;
            } else {
                pendingTraceListener = true;
            }
        }
    }

    /*
     * <p>
     * the Java Console to display useful error and trace messages
     * </p>
     *
     */
    private static boolean traceInit = false;
    private static ConsoleWindow console = null;
    private static ConsoleTraceListener ctl = null;
    private static ConsoleController14 controller = null;
}
