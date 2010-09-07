/*
 * @(#)DeployPerfUtil.java	1.6 09/04/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.lang.IllegalStateException;
import java.lang.Object;
import java.lang.String;
import com.sun.deploy.config.Config;
import com.sun.deploy.perf.DefaultPerfHelper;
import com.sun.deploy.perf.PerfHelper;
import com.sun.deploy.perf.PerfLabel;

public class DeployPerfUtil extends Object {

    // only for JDK 5 or above
    private static final boolean deployFirstframePerfEnabled;

    private static volatile boolean getenvSupported = true;

    // must be after the static variable definition above
    static {
        deployFirstframePerfEnabled =
                (getenv("DEPLOY_FIRSTFRAME_PERF_ENABLED") != null);
    }

    /** Wrapper for System.getenv(), which doesn't work on platforms
    earlier than JDK 5 */
    private static String getenv(String variableName) {
        if (getenvSupported) {
            try {
                return System.getenv(variableName);
            } catch (Error e) {
                getenvSupported = false;
            }
        }
        return null;
    }

    /**
     * Checks if perf. monitoring is enabled for first frame displayed.
     * If this is true, the performance log will be created only when the 
     * Plugin2BasicService.isWebBrowserSupported() is called. Otherwise,
     * the performance log will be created in the Plugin2Manager before the 
     * applet's start method is called.
     */
    public static boolean isDeployFirstframePerfEnabled() {
        return deployFirstframePerfEnabled;
    }


    /***************************************************************************
     * Checks to see if the user has enabled Plug-in metric collection by
     * setting the <code>DEPLOY_PERF_ENABLED</code> environment variable.  All instrumented
     * code in the Plug-in should be encapsulated in an <code>if-block</code>
     * that checks this method before executing.
     *
     * @return <code>true</code> if logging is enabled; <code>flase</code>
     *         otherwise.
     */
    public static boolean isEnabled() {
        return (perfLog != null);
    }

    /***************************************************************************
     * Writes a short message to the native message storage.  A time stamp is
     * automatically generated for the message.
     *
     * @param  message  the message to write.
     */
    public static void put(String label) {	
	if ((isEnabled() == true) && (helper != null)) {
	    helper.put(label);
	}	
    }

    /***************************************************************************
     * Puts a time stamped label entry into storage,
     * where the relative time stamp uses deltaStart as it's datum reference
     *
     * Optional extended time reference functionality
     *
     * @param  deltaStart  the start of the delta, which is to be printed
     * @param  label  the label to store.
     * @return the current time, relative to init time
     * @see setInitTime
     */
    public static long put(long deltaStart, String label) {
        if ((isEnabled() == true) && (helper != null)) {
            return helper.put(deltaStart, label);
        }
        return 0;
    }

    public static void setInitTime(long t0) {
        if ((isEnabled() == true) && (helper != null)) {
            helper.setInitTime(t0);
        }
    }

    /**
     * Write the performance information to the log file.  All performance
     * labels are written to the file in the format:
     *
     * <blockquote><pre>
     *   <time-stamp>, <elapsed>, <delta>, <message>
     * </pre></blockquote>
     */
    public static void write() throws IOException {
        write(null, false);
    }

    /**
     * Write the performance information to the log file.  This method will first
     * write the default output, followed by a rollup summary using the specified
     * <code>PerfRollup</code>.
     *
     * The <code>rollup</code> object can be used to perform more advanced
     * analysis of the label data.  For example, the labels array can be put
     * into a map that uses the label for a key.  This map can then be easily
     * searched for specific events to provide a summary of the most interesting
     * data.  If <code>rollup</code> is <code>null</code> then the only default
     * output is genearted.
     *
     * @param rollup  an instance of a class that implements the
     *                <code>PerfRollup</code> interface.
     */
    public synchronized static void write(PerfRollup rollup, boolean isNewPlugin) 
							throws IOException {

	if ((isEnabled() == true) && (helper != null)) {
	    // make sure perfLog is not a directory
	    if (perfLog.isDirectory() == true) {
		throw (new IllegalStateException(INVALID_DEPLOY_PERF_LOG));
	    }

	    // make sure the parent directories for the file exist
	    if ((perfLog.getParentFile().exists() == false) &&
		(perfLog.getParentFile().mkdirs() == false)) {
		throw (new IllegalStateException(MISSING_PARENTS));
	    }
	    PrintStream out = null;
	    try {
		out = new PrintStream(new FileOutputStream(perfLog, false));

		// if everything is still good, then write the data
		PerfLabel [] labels  = helper.toArray();

		if ((labels != null) && (labels.length > 0)) {

		    long         start   = 0;
		    long         last    = 0;
		    long         elapsed = 0;
		    long         delta   = 0;

		    for (int i = 0; i < labels.length; i++) {
			long time = labels[i].getTime();

			if (start == 0) {
			    start = time;
			}
			else {
			    elapsed = time - start;
			    delta   = time - last;
			}

			last = time;


			StringBuffer buffer = new StringBuffer(256);

			buffer.append(pad(labels[i].getLabel(), 65, true));
			buffer.append(" , ").append(pad(elapsed, 7, true));
			buffer.append(" , ").append(pad(delta, 7, true));

			out.println(buffer.toString());

		    }
		}
		else {
		    out.println("The perf label event array is empty.");
		}

		out.flush();

		if ((rollup != null) && !isNewPlugin) {
		    rollup.doRollup(labels, out);
		    out.flush();
		}
	    } catch (IOException ioe) {
		throw ioe;
	    } finally {
		if (out != null) {
		    out.close();
		}
	    }
	}

    }

    /**
     * Initialize the static instance.
     */
    public static synchronized void initialize(PerfHelper helper) {
        if (DeployPerfUtil.helper == null) {
            DeployPerfUtil.helper = helper;
        }
    }
    public static synchronized PerfHelper getPerfHelper() {
        return DeployPerfUtil.helper;
    }

    /***************************************************************************
     * The name of the environment variable used to enable performance metric
     * collection.
     */
    private static final String DEPLOY_PERF_ENABLED = "DEPLOY_PERF_ENABLED";

    /***************************************************************************
     * The name of the environment variable used to override the default logfile
     * name.
     */
    private static final String DEPLOY_PERF_LOG = "DEPLOY_PERF_LOG";

    /***************************************************************************
     * The default logfile name.
     */
    private static final String DEFAULT_LOGNAME = "deploy_perf.log";

    /***************************************************************************
     * The default logfile name.  The default logfile is written to the directory
     * specified by Config.getLogDirectory().
     */
    private static final String DEFAULT_LOGFILE = "deploy_perf.log";

    private static final String INVALID_DEPLOY_PERF_LOG =
        "The DEPLOY_PERF_LOG variable must point to a file path, not a directory!";

    private static final String MISSING_PARENTS =
        "Failed to create the parent directories for the file specified by " +
        "DEPLOY_PERF_LOG!";

    /***************************************************************************
     * Get the <code>File</code> object to use when writing the performance log.
     *
     * By default, the log file is written to the <code>deploy_perf.log</code>
     * file in the Deployment trace directory.  The user can override this by
     * setting <code>DEPLOY_PERF_LOG</code>.
     */
    private static File getPerfLog() {
        File result = null;
        try {
            String perfEnabled = System.getenv(DEPLOY_PERF_ENABLED);

            if ((perfEnabled != null) &&
                (perfEnabled.equalsIgnoreCase("false") == false)) {
                String perfLogName = System.getenv(DEPLOY_PERF_LOG);

                if (perfLogName != null) {
                    result = new File(perfLogName);
                }
                else {
                    result = new File(Config.getLogDirectory(), DEFAULT_LOGNAME);
                }
            }
        } catch (Error e) {
            // This code needs to fail gracefully on JDK 1.4.2
        }
        return (result);
    }

    /**
     * Pad the given number to the given width.
     *
     * @param value        the value pad.
     * @param width        the maximum amount of padding to add.
     * @param leftJustify  if <code>true</code> then left justify the output.
     *
     * @return a <code>String</code> containing the padded <code>value</code>.
     */
    private static String pad(long value, int width, boolean leftJustify) {
        String       text   = Long.toString(value);
	
	return (pad(text, width, leftJustify));

    }

    /**
     * Pad the given string to the given width.
     * This is similar to the above method but accepts String as the first argument.
     */
    private static String pad(String label, int width, boolean leftJustify) {
	int          space  = width - label.length();
	StringBuffer buffer = new StringBuffer(width);

	if (leftJustify == false) {
	    for (int i = 0; i < space; i++) {
		buffer.append(' ');
	    }
	}

	buffer.append(label);

	if (leftJustify == true) {
	    for (int i = 0; i < space; i++) {
		buffer.append(' ');
	    }
	}

	return (buffer.toString());
    }

    private static final File perfLog = getPerfLog();
    private static PerfHelper helper  = null;
}
