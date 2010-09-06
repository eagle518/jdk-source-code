/*
 * @(#)JavaRunTime.java	1.64 04/04/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A loaded and running Java Virtual machine is initialized and
 * released in the JavaRunTime.
 * This is the opportunity for startup code
 *
 * @version 	1.2.1
 * @author	Jerome Dochez
 *
 *
 * Modified by:
 * Rita Fisher      Apr. 04, 2001       Bug fix 4421334 - made Java Console truly disable-able (there are now
 *                                      three possible states for Java Console - full size, hidden (icon/system
 *                                      tray) or no console).
 * 
 */
import java.io.File;
import java.util.logging.Level;
import java.util.logging.Logger;
import com.sun.deploy.util.ConsoleHelper;
import com.sun.deploy.util.ConsoleWindow;
import com.sun.deploy.util.ConsoleController;
import com.sun.deploy.util.ConsoleTraceListener;
import com.sun.deploy.util.FileTraceListener;
import com.sun.deploy.util.LoggerTraceListener;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.DeploySysRun;
import sun.plugin.util.UserProfile;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.PluginConsoleController;
import com.sun.deploy.config.Config;

public class JavaRunTime 
{
    /**
     * <p>
     * Method called once when the javaplugin is loaded. This is the opportunity
     * to add some initialization function. For convenience the java home is 
     * passed to be able to know where the java environement resides
     *
     * @param javaHome		java home directory
     * @param bridgeLibPath	Activator path
     * @param userHome		user home dir
     */
    protected static void initEnvironment(String javaHome, String bridgeLibPath, String userHome) 
    {
	DeploySysRun.setOverride(new PluginSysUtil());

	java.util.Properties systemProps = System.getProperties();
	systemProps.put("java.home", javaHome );  
	if (userHome==null)
	    systemProps.put("user.home", javaHome);
	else
	    systemProps.put("user.home", userHome);

	// Load the properties
	AppletViewer.loadPropertiesFiles();

	// Redirect System.out/System.err
	com.sun.deploy.util.Trace.redirectStdioStderr();
    }     

    /**
     * <p>
     * Return the console window for this running Java Virtual Machine.
     * If the console is not created yet, create a new Java Console.
     *
     * @return console window object
     * </p>
     */
    public synchronized static ConsoleWindow getJavaConsole()
    {
	// Initialize tracing environment
	initTraceEnvironment();

        if (console == null)
	{
	    // Create console
	    console = ConsoleWindow.create(controller);

	    // Hook up console trace listener
	    ctl.setConsole(console);
	}

	return console;
    }

    /**
     * <p>
     * Initialize the trace file facilities.
     * </p>
     */
    public synchronized static void initTraceEnvironment()
    {
	// Return if environment has been initialized
	if (traceInit) 
	    return;

	traceInit = true;

	if (Config.getBooleanProperty(Config.TRACE_MODE_KEY)) {

	    String initTraceLevel = Config.getProperty(Config.TRACE_LEVEL_KEY);

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
		Trace.setInitialTraceLevel();			
	    }
	}

	// Create Java Console
	controller = new PluginConsoleController();

	// Add console tracing
	ctl = new ConsoleTraceListener(controller);
	Trace.addTraceListener(ctl);
	
	// Add file tracing
	File traceFile = new File(UserProfile.getTraceFile());
	traceFile.getParentFile().mkdirs();
	if (traceFile.canWrite() || (!traceFile.exists() &&
	    traceFile.getParentFile().canWrite())) {
	    Trace.addTraceListener(new FileTraceListener(traceFile, false));
	} else {
	    Trace.println("can not write to trace file: "+traceFile,
		TraceLevel.BASIC);
	}

	if (Config.getBooleanProperty(Config.LOG_MODE_KEY)) {
	
	    // Create log trace listener
	    String logFilename = UserProfile.getLogFile();
	    File logFile = new File(logFilename);
	    logFile.getParentFile().mkdirs(); 
	    if (logFile.canWrite() || (!logFile.exists() &&
		logFile.getParentFile().canWrite())) {
	        LoggerTraceListener ltl = new LoggerTraceListener("sun.plugin",
		    logFilename);
	        ltl.getLogger().setLevel(Level.ALL);
	        ((PluginConsoleController)controller).setLogger(
		    ltl.getLogger());
	        // Add log tracing
	        Trace.addTraceListener(ltl);
	    } else {
		Trace.println("can not write to log file: " + logFilename, 
		    TraceLevel.BASIC);
	    }
	}

	// Display version information in console
	System.out.print(ConsoleHelper.displayVersion());
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
					ConsoleWindow cw = getJavaConsole();
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
				ConsoleWindow cw = getJavaConsole();
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

    /*
     * <p>
     * the Java Console to display useful error and trace messages
     * </p>
     *
     */
    private static boolean traceInit = false;
    public static native String dumpAllStacks();  
    private static ConsoleWindow console = null;
    private static ConsoleTraceListener ctl = null;
    private static ConsoleController controller = null;
}



