/*
 * @(#)ConsoleHelper.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.util.TreeSet;
import java.util.Collection;
import java.util.Iterator;
import java.util.Properties;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


/*
 * ConsoleHelper is used for displaying various system messages
 * in Java Console.
 */

public final class ConsoleHelper
{
    // ConsoleWindowStrategy object controls the behavior of the console
    // during user interaction.
    //
    private static ConsoleController controller = null;

    public static void setConsoleController(ConsoleController c)
    {
	controller = c;
    }

    public static String dumpAllStacks() {
        
        /* 
         * This calls into jvm.dll _JVM_DumpAllStacks function to get the
         * stack trace.
         *
         * The native code will redirect the file descriptor 1 (stdout) to a 
         * temp file, call _JVM_DumpAllStacks, get the output from the temp 
         * file and return it.
         *
         * Starting in 6.0 on Windows, Java is compiled with VC7.  Older 
         * releases is compiled with VC6.  So when doing the stdout redirection,
         * we must load the correct C-Runtime (CRT) because Java Web Start can 
         * run with different versions of Java; and different versions of CRT 
         * has its own stdout state.
         *
         * When we are running in mustang or above, we will be using the
         * msvcr71.dll, which is the CRT that we bundled in mustang or above.
         *
         * However, when Java Web Start 6.0 tries to run with any JRE before
         * mustang, we must do the stdout redirection with the CRT from VC6,
         * msvcrt.dll, otherwise the output from _JVM_DumpAllStacks from Java
         * 5.0 or earlier will not be seen.
         *
         * For Solaris or Linux, the problem does not exists and
         * dumpAllStacksImpl and preMustangDumpAllStacksImpl is the same.
         *
         */
        if (Config.isJavaVersionAtLeast16()) {
            return dumpAllStacksImpl();
        } else {
            return preMustangDumpAllStacksImpl();
        }      
    }
    
    private static native String dumpAllStacksImpl();
    
    private static native String preMustangDumpAllStacksImpl();

    /**
     * Dump threads in threadgroup recursively.
     *
     * @param tg Root of thread group tree.
     * @param buffer String buffer to append output
     */
    static void dumpThreadGroup(ThreadGroup tg, StringBuffer sb)
    {
	if (tg != null)
	{
	    // Clean up thread group if necessary.
	    try
	    {
		if (tg.activeCount() == 0 && tg.activeGroupCount() == 0 && tg.isDestroyed() == false)
		    tg.destroy();
	    }
	    catch (Throwable e)
	    {
	    }

	    // Dump thread group
	    sb.append("Group " + tg.getName());
	    sb.append(",ac=" + tg.activeCount());
	    sb.append(",agc=" + tg.activeGroupCount());
	    sb.append(",pri=" + tg.getMaxPriority());
	    if (tg.isDestroyed())
		sb.append(",destoyed");
	    if (tg.isDaemon())
		sb.append(",daemon");
	    sb.append("\n");

	    Thread[] tt = new Thread[1000];
	    tg.enumerate(tt, false);

	    // Dump threads within thread group
	    for (int i=0; i < tt.length; i++)
	    {
		if (tt[i] != null)
		{
		    sb.append("    ");
		    sb.append(tt[i].getName());
		    sb.append(",");
		    sb.append(tt[i].getPriority());

		    if (tt[i].isAlive())
			sb.append(",alive");
		    else
			sb.append(",not alive");

		    if (tt[i].isDaemon())
			sb.append(",daemon");

		    if (tt[i].isInterrupted())
			sb.append(",interrupted");

		    sb.append("\n");
		}
	    }

	    // Obtain sub-thread group
	    ThreadGroup[] tgArray = new ThreadGroup[1000];
	    tg.enumerate(tgArray, false);

	    // Dump sub-thread group
	    for (int i=0; i < tgArray.length; i++)
	    {
		if (tgArray[i] != null)
		    dumpThreadGroup(tgArray[i], sb);
	    }
	}
    }


    /**
     * Display help information.
     */
    public static String displayHelp()
    {
	StringBuffer buffer = new StringBuffer();

	buffer.append(ResourceManager.getMessage("console.menu.text.top"));
	buffer.append(ResourceManager.getMessage("console.menu.text.c"));
	buffer.append(ResourceManager.getMessage("console.menu.text.f"));
	buffer.append(ResourceManager.getMessage("console.menu.text.g"));
	buffer.append(ResourceManager.getMessage("console.menu.text.h"));

	if (controller.isJCovSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.j"));
	}

	if (controller.isDumpClassLoaderSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.l"));
	}

	buffer.append(ResourceManager.getMessage("console.menu.text.m"));

	if (controller.isLoggingSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.o"));
	}

	if (controller.isProxyConfigReloadSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.p"));
	}

	buffer.append(ResourceManager.getMessage("console.menu.text.q"));

	if (controller.isSecurityPolicyReloadSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.r"));
	}

	buffer.append(ResourceManager.getMessage("console.menu.text.s"));
	buffer.append(ResourceManager.getMessage("console.menu.text.t"));

	if (controller.isDumpStackSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.v"));
	}

	if (controller.isClearClassLoaderSupported())
	{
	    buffer.append(ResourceManager.getMessage("console.menu.text.x"));
	}

	buffer.append(ResourceManager.getMessage("console.menu.text.0"));
	buffer.append(ResourceManager.getMessage("console.menu.text.tail"));

	return (buffer.toString());
    }

    public static String displayVersion()
    {
	// Tell people a little about ourselves.  This is partly so we can
	// get better information when people report problems.

	StringBuffer buffer = new StringBuffer();

	buffer.append(controller.getProductName());
	buffer.append("\n");
	buffer.append(ResourceManager.getMessage("console.using_jre_version"));
	buffer.append(" ");
	buffer.append(System.getProperty("java.runtime.version"));
	buffer.append(" ");
	buffer.append(System.getProperty("java.vm.name"));
	buffer.append("\n");
	buffer.append(ResourceManager.getMessage("console.user_home"));
	buffer.append(" = ");
	buffer.append(System.getProperty("user.home"));

	return (buffer.toString());
    }


    public static String displaySystemProperties()
    {
	// Obtain all properties name and sort them first
	//
	TreeSet treeSet = new TreeSet();

	Properties config = Config.getProperties();

	for (java.util.Enumeration e = System.getProperties().propertyNames(); e.hasMoreElements();)
	{
	    treeSet.add(e.nextElement());
	}

	StringBuffer buffer = new StringBuffer();

	buffer.append(ResourceManager.getMessage("console.dump.system.properties"));
	buffer.append(ResourceManager.getMessage("console.menu.text.top"));

	// Iterate the sorted properties
	//
	for (Iterator iter = treeSet.iterator(); iter.hasNext();)
	{
	    String key = (String) iter.next();
	    if (!config.containsKey(key)) {
	        String value = System.getProperty(key);

	        if (value != null)
    	        {
		    if (value.equals("\n"))
		        value = "\\n";
		    else if (value.equals("\r"))
		        value = "\\r";
		    else if (value.equals("\r\n"))
		        value = "\\r\\n";
		    else if (value.equals("\n\r"))
		        value = "\\n\\r";
		    else if (value.equals("\n\n"))
		        value = "\\n\\n";
		    else if (value.equals("\r\r"))
		        value = "\\r\\r";
	        }

	        buffer.append(key + " = " + value + "\n");
	    }
	}

	buffer.append(ResourceManager.getMessage("console.menu.text.tail"));

	// now add deployment properties:
	treeSet.clear();
        for (java.util.Enumeration e = config.propertyNames(); e.hasMoreElements();)
        {
            treeSet.add(e.nextElement());
        }
 

        buffer.append(ResourceManager.getMessage("console.dump.deployment.properties"));
        buffer.append(ResourceManager.getMessage("console.menu.text.top"));
 
        // Iterate the sorted properties
        //
        for (Iterator iter = treeSet.iterator(); iter.hasNext();)
        {
            String key = (String) iter.next();
            String value = config.getProperty(key);
 
            if (value != null)
            {  
                if (value.equals("\n"))
                    value = "\\n";
                else if (value.equals("\r"))
                    value = "\\r";
                else if (value.equals("\r\n"))
                    value = "\\r\\n";
                else if (value.equals("\n\r"))
                    value = "\\n\\r";
                else if (value.equals("\n\n"))
                    value = "\\n\\n";
                else if (value.equals("\r\r"))
                    value = "\\r\\r";
            }
 
            buffer.append(key + " = " + value + "\n");
        }
 
        buffer.append(ResourceManager.getMessage("console.menu.text.tail"));
        buffer.append(ResourceManager.getMessage("console.done"));

	return (buffer.toString());
    }
}
