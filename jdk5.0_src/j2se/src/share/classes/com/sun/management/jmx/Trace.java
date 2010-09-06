/*
 * @(#)Trace.java	1.7 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management.jmx;

// java import
//
import java.io.*;

// jmx import
//
import javax.management.*;

/**
 * This class does nothing. To get the traces, use the 
 * {@link java.util.logging} APIs.
 *
 * @deprecated Use {@link java.util.logging} APIs instead.
 *
 * @since 1.5
 */
@Deprecated
public class Trace {
    /**
     * Information level defined for trace level.
     * The formation will be provided to help development of JDMK applications.
     */
    public static final int LEVEL_TRACE = 1;

    /**
     * Information level defined for debug level.
     * The formation will be provided to help diagnosy. Selecting this level
     * will result to select the LEVEL_TRACE too.
     */
    public static final int LEVEL_DEBUG = 2;

    /**
     * Information type defined for MBean Server information.
     */
    public static final int INFO_MBEANSERVER = 1;

    /**
     * Information type defined for MLet service information.
     */
    public static final int INFO_MLET = 2;

    /**
     * Information type defined for Monitor information.
     */
    public static final int INFO_MONITOR = 4;

    /**
     * Information type defined for Timer information.
     */
    public static final int INFO_TIMER = 8;

    /**
     * Information type defined for HTML adaptor information.
     */
    public static final int INFO_ADAPTOR_HTML	= 16;

    /**
     * Information type defined for all other classes.
     */  
    public static final int INFO_MISC = 32;

    /**
     * Information type defined for Relation Service.
     */  
    public static final int INFO_RELATION = 64;

    /**
     * Information type defined for Model MBean
     */
    public static final int INFO_MODELMBEAN = 128;

    /**
     * Information type defined to represent all types defined.
     */
    public static int INFO_ALL = 
	INFO_MBEANSERVER |
	INFO_MLET |
	INFO_MONITOR |
	INFO_ADAPTOR_HTML |
	INFO_TIMER |
	INFO_MISC |
	INFO_RELATION |
	INFO_MODELMBEAN;

    // --------------
    // public methods
    // --------------

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     * @return false
     */
    public static boolean isSelected(int level, int type) {
	return false;
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public static void parseTraceProperties() throws IOException {
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     * @return false
     */
    public static boolean send(int level,
			       int type,
			       String className,
			       String methodName,
			       String info) {

	return false;
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     * @return false
     */
    public static boolean send(int level,
			       int type,
			       String className,
			       String methodName,
			       Throwable exception) {
	return false;
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public static void addNotificationListener(NotificationListener listener,
					       NotificationFilter f,
					       Object handback) 
	throws IllegalArgumentException {
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public static void addNotificationListener(TraceListener listener,
					       Object handback) 
	throws IllegalArgumentException {
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public static void removeNotificationListener(NotificationListener listener) {
    }

    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     */
    public static void removeAllListeners() {
    }


    /**
     * This method does nothing. To get the traces, use the 
     * {@link java.util.logging} APIs.
     * @return "Unknown type"
     */
    protected static String getRIType(int type) {
	return getType(type);
    }

    static String getType(int type) {
	return UNKOWNTYPE;
    }

    static String getLevel(int level) {
	return "Unknown level";
    }

    protected static final String UNKOWNTYPE = "Unknown type";
}
