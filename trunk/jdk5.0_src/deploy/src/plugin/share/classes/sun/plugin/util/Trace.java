/*
 * @(#)Trace.java	1.47 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Component;
import java.util.Set;
import java.util.StringTokenizer;
import java.text.MessageFormat;
import sun.plugin.resources.ResourceHandler;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.resources.ResourceManager;


/**
 * Trace is a class that provides basic tracing capability in Java Plug-in.
 * It is enabled through two properties:
 *
 * 1) javaplugin.trace
 * 2) javaplugin.trace.option
 *
 * These two properties control how fine-grained the Java Plug-in tracing
 * should be.
 */
public class Trace 
{
    static
    {
	// Determine if tracing is enabled;
	reset();

	// Determine if the tracing is enabled at all.
	//
	boolean bAutomationOn = ((Boolean) java.security.AccessController.doPrivileged(
    			       new sun.security.action.GetBooleanAction("javaplugin.automation"))).booleanValue();

	//com.sun.deploy.util.Trace.enableAutomation(bAutomationOn);
    }

 
    /** 
     * Initialize the Trace facilities
     */
    public static void reset()
    {
	// Determine if the tracing is enabled at all.
	//
	if (((Boolean) java.security.AccessController.doPrivileged(
    		new sun.security.action.GetBooleanAction("javaplugin.trace"))).booleanValue())
	{
	    // Determine how fine-grained tracing should be.
	    //
            String traceOptions = (String) java.security.AccessController.doPrivileged(
    				  new sun.security.action.GetPropertyAction("javaplugin.trace.option"));

	    if (traceOptions == null)
	    {
		com.sun.deploy.util.Trace.setBasicTrace(true);
		com.sun.deploy.util.Trace.setCacheTrace(true);
		com.sun.deploy.util.Trace.setNetTrace(true);
		com.sun.deploy.util.Trace.setSecurityTrace(true);
		com.sun.deploy.util.Trace.setExtTrace(true);
		com.sun.deploy.util.Trace.setLiveConnectTrace(true);
	    }
	    else
	    {
    		StringTokenizer st = new StringTokenizer(traceOptions, "|");
     
		while (st.hasMoreTokens()) 
		{
		    String option = (String) st.nextToken();

		    if (option == null || option.equalsIgnoreCase("all"))
		    {
			com.sun.deploy.util.Trace.setBasicTrace(true);
			com.sun.deploy.util.Trace.setCacheTrace(true);
			com.sun.deploy.util.Trace.setNetTrace(true);
			com.sun.deploy.util.Trace.setSecurityTrace(true);
			com.sun.deploy.util.Trace.setExtTrace(true);
			com.sun.deploy.util.Trace.setLiveConnectTrace(true);

			// If "all", then there is no need to check the rest
			break;
		    }
		    else if (option.equalsIgnoreCase("basic"))
		    {
			com.sun.deploy.util.Trace.setBasicTrace(true);
		    }
		    else if (option.equalsIgnoreCase("net"))
		    {
			com.sun.deploy.util.Trace.setCacheTrace(true);
			com.sun.deploy.util.Trace.setNetTrace(true);
		    }
		    else if (option.equalsIgnoreCase("security"))
		    {
			com.sun.deploy.util.Trace.setSecurityTrace(true);
		    }
		    else if (option.equalsIgnoreCase("ext"))
		    {
			com.sun.deploy.util.Trace.setExtTrace(true);
		    }
		    else if (option.equalsIgnoreCase("liveconnect"))
		    {
			com.sun.deploy.util.Trace.setLiveConnectTrace(true);
		    }
		}
	    }
	}
    }

    
    /**
     * Determine if tracing is enabled.
     *
     * @return true if tracing is enabled
     */
    public static boolean isEnabled()
    {
	return com.sun.deploy.util.Trace.isEnabled();
    }

    /**
     * Determine if automation is enabled.
     *
     * @return true if automation is enabled
     */
    public static boolean isAutomationEnabled()
    {
	return com.sun.deploy.util.Trace.isAutomationEnabled();
    }


    /** 
     * Output message to System.out if tracing is enabled. 
     *
     * @param msg Message to be printed
     */
    public static void println(String msg)
    {
	println(msg, TraceFilter.DEFAULT);
    }


    /** 
     * Output message to System.out if tracing is enabled. 
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void println(String msg, int filter)
    {
	// Check if tracing is enabled at certain levels
	//
	if ((filter & TraceFilter.JAVA_CONSOLE_ONLY) == TraceFilter.JAVA_CONSOLE_ONLY)
	{
	    // If no level is specified, default to basic level
	    //
	    if ((filter & TraceFilter.LEVEL_MASK) == 0)
		filter |= TraceFilter.LEVEL_BASIC;    

	    switch (filter & TraceFilter.LEVEL_MASK)
	    {
		case TraceFilter.LEVEL_BASIC:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.BASIC);
		    break;
		}
		case TraceFilter.LEVEL_NET:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.NETWORK);
		    break;
		}
		case TraceFilter.LEVEL_SECURITY:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.SECURITY);
		    break;
		}
		case TraceFilter.LEVEL_EXT:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.EXTENSIONS);
		    break;
		}
		case TraceFilter.LEVEL_LIVECONNECT:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.LIVECONNECT);
		    break;
		}
		default:
		{
		    break;
		}
	    }
	}
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     */
    public static void msgPrintln(String resource)
    {
	msgPrintln(resource, null, TraceFilter.DEFAULT);
    }


    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgPrintln(String resource, int filter)
    {
    	msgPrintln(resource, null, filter);
    }

    /** 
     * Output resource to System.out if tracing is enabled. 
     *
     * @param resource Resource message to be printed
     * @param params for the resource string
     */
    public static void msgPrintln(String resource, Object[] params)
    {
    	msgPrintln(resource, params, TraceFilter.DEFAULT);
    }

    /*
     * Output resource message to System.out if basic tracing is enabled.
     *
     * @param resource to be printed
     * @param params for the resource string
     * @param fileter Trace filter
     *
     * Check if tracing is on or if message is to be printed in status bar
     * or gray box - then get message from ResourceHandler and call Println;
     * else - don't spend any time getting message.
     */
    public static void msgPrintln(String resource, Object[] params, int filter) 
    {
	// Check if tracing is enabled at certain levels
	//
	if ((filter & TraceFilter.JAVA_CONSOLE_ONLY) == TraceFilter.JAVA_CONSOLE_ONLY)
	{
	    String msg = null;

	    if (params == null)
	    {
		msg = ResourceHandler.getMessage(resource);
	    }
	    else
	    {
		MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage(resource));            
		msg = formatter.format(params);
	    }

	    // If no level is specified, default to basic level
	    //
	    if ((filter & TraceFilter.LEVEL_MASK) == 0)
		filter |= TraceFilter.LEVEL_BASIC;    

	    switch (filter & TraceFilter.LEVEL_MASK)
	    {
		case TraceFilter.LEVEL_BASIC:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.BASIC);
		    break;
		}
		case TraceFilter.LEVEL_NET:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.NETWORK);
		    break;
		}
		case TraceFilter.LEVEL_SECURITY:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.SECURITY);
		    break;
		}
		case TraceFilter.LEVEL_EXT:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.EXTENSIONS);
		    break;
		}
		case TraceFilter.LEVEL_LIVECONNECT:
		{
		    com.sun.deploy.util.Trace.println(msg, com.sun.deploy.util.TraceLevel.LIVECONNECT);
		    break;
		}
		default:
		{
		    break;
		}
	    }
	}
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void printException(Throwable e)
    {
	printException(null, e);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     */
    public static void printException(Component parentComponent, Throwable e)
    {
	printException(parentComponent, e, 
		       null,
		       null);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, String desc, String caption)
    {
	printException(parentComponent, e, desc, caption, true);
    }

    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param parentComponent Parent Component
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     * @param show Show exception dialog
     */
    public static void printException(Component parentComponent, Throwable e, 
				      String desc, String caption, boolean show)
    {
	// Exception should be shown all the time
	e.printStackTrace();

	// Show exception only if automation is disabled
	if (show && isAutomationEnabled() == false)
	{
	    if (desc == null)
		desc = ResourceManager.getMessage("dialogfactory.general_error");

	    // Show Exception dialog
	    DialogFactory.showExceptionDialog(parentComponent, e, desc, caption);
	}
    }

    /** 
     * Output message to System.out if network tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void netPrintln(String msg)
    {
	println(msg, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }


    /** 
     * Output message to System.out if network tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void netPrintln(String msg, int filter)
    {
	println(msg, filter | TraceFilter.LEVEL_NET);
    }

    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgNetPrintln(String resource)
    {
	msgPrintln(resource, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }


    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgNetPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, filter | TraceFilter.LEVEL_NET);
    }
    
    /** 
     * Output resource to System.out if network tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgNetPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceFilter.DEFAULT | TraceFilter.LEVEL_NET);
    }

    /** 
     * Output exception to System.out if net tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void netPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.net_error"), null, false);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void netPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, false);
    }


    /** 
     * Output message to System.out if security tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void securityPrintln(String msg)
    {
    	 println(msg, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     */
    public static void msgSecurityPrintln(String resource)
    {
    	 msgPrintln(resource, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }


    /** 
     * Output resource message to System.out if security tracing is enabled.
     *
     * @param resource Resource Message to be printed
     * @param params for the resource string
     */
    public static void msgSecurityPrintln(String resource, Object[] params)
    {
    	 msgPrintln(resource, params, TraceFilter.DEFAULT | TraceFilter.LEVEL_SECURITY);
    }

    /** 
     * Output exception to System.out if security tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void securityPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.security_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void securityPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if extension tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void extPrintln(String msg)
    {
	println(msg, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }


    /** 
     * Output message to System.out if extension tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void extPrintln(String msg, int filter)
    {
	println(msg, filter | TraceFilter.LEVEL_EXT);
    }

   
    /** 
     * Output resource message to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgExtPrintln(String resource)
    {
	msgPrintln(resource, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }


    /** 
     * Output resource message to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgExtPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, filter | TraceFilter.LEVEL_EXT);
    }

    /** 
     * Output resource to System.out if extension tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgExtPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceFilter.DEFAULT | TraceFilter.LEVEL_EXT);
    }

    /** 
     * Output exception to System.out if extension tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void extPrintException(Throwable e)
    {
	printException(null, e, ResourceManager.getMessage("dialogfactory.ext_error"), null, true);
    }


    /** 
     * Output exception to System.out if tracing is enabled.
     *
     * @param e exception to be printed
     * @param desc Description about the exception
     * @param caption Caption for the exception dialog
     */
    public static void extPrintException(Throwable e, String desc, String caption)
    {
	printException(null, e, desc, caption, true);
    }


    /** 
     * Output message to System.out if LiveConnect tracing is enabled.
     *
     * @param msg Message to be printed
     */
    public static void liveConnectPrintln(String msg)
    {
	println(msg, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }


    /** 
     * Output message to System.out if LiveConnect tracing is enabled.
     *
     * @param msg Message to be printed
     * @param filter Trace filter
     */
    public static void liveConnectPrintln(String msg, int filter)
    {
	println(msg, filter | TraceFilter.LEVEL_LIVECONNECT);
    }

    
    /** 
     * Output resource message to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     */
    public static void msgLiveConnectPrintln(String resource)
    {
	msgPrintln(resource, null, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }


    /** 
     * Output resource message to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param filter Trace filter
     */
    public static void msgLiveConnectPrintln(String resource, int filter)
    {
	msgPrintln(resource, null, filter | TraceFilter.LEVEL_LIVECONNECT);
    }

    /** 
     * Output resource to System.out if LiveConnect tracing is enabled.
     *
     * @param resource Resource message to be printed
     * @param params parameters for the message string
     */
    public static void msgLiveConnectPrintln(String resource, Object[] params)
    {
	msgPrintln(resource, params, TraceFilter.DEFAULT | TraceFilter.LEVEL_LIVECONNECT);
    }

    /** 
     * Output exception to System.out if LiveConnect tracing is enabled.
     *
     * @param e exception to be printed
     */
    public static void liveConnectPrintException(Throwable e)
    {
	// Notice that we should never popup dialog in LiveConnect because
	// it may be called from the main thread, and we may end up deadlock
	// ourselves.

	printException(null, e, null, null, false);
    }
}


