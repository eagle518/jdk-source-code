/*
 * @(#)DeployAWTUtil.java	1.1 03/03/08
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.awt.Component;
import java.awt.EventQueue;
import java.awt.Toolkit;
import java.awt.AWTEvent;
import java.awt.event.InvocationEvent;
import java.lang.reflect.InvocationTargetException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.awt.SunToolkit;
import sun.awt.AppContext;

/**
 * <code>DeployAWTUtil</code> is a platform-independent class
 * that dispatch events to the event queue in the proper 
 * AppContext.
 * <p>
 *
 * @author Stanley Man-Kit Ho
 */
public class DeployAWTUtil 
{
    /**
     * Causes AWTEvent to be posted on the event thread that belongs to
     * the AppContext of the component.
     *
     * @param component Component
     * @param event AWTEvent
     */
    public static void postEvent(Component component, AWTEvent event)
    {
	SunToolkit.postEvent(SunToolkit.targetToAppContext(component), event);
    }

    /**
     * Causes <code>runnable</code> to have its <code>run</code>
     * method called in the dispatch thread of the <code>EventQueue</code>.
     * This will happen after all pending events are processed.
     *
     * @param runnable  the <code>Runnable</code> whose <code>run</code>
     *                  method should be executed
     *                  synchronously on the <code>EventQueue</code>
     * @see             #invokeAndWait
     * @since           1.2
     */
    public static void invokeLater(Component component, Runnable runnable) {
	SunToolkit.postEvent(SunToolkit.targetToAppContext(component),
		  	     new InvocationEvent(Toolkit.getDefaultToolkit(), runnable));
    }

    /**
     * Causes <code>runnable</code> to have its <code>run</code>
     * method called in the dispatch thread of the
     * <code>EventQueue</code> associated with the supplied
     * <code>AppContext</code>.
     * This will happen after all pending events are processed.
     *
     * @param runnable  the <code>Runnable</code> whose <code>run</code>
     *                  method should be executed
     *                  synchronously on the <code>EventQueue</code>
     * @see             #invokeAndWait
     * @since           1.2
     */
    public static void invokeLater(AppContext context, Runnable runnable)     
    {
	SunToolkit.postEvent(context,
	new InvocationEvent(Toolkit.getDefaultToolkit(), runnable));
    }

    /**
     * Causes <code>runnable</code> to have its <code>run</code>
     * method called in the dispatch thread of the <code>EventQueue</code>.
     * This will happen after all pending events are processed.
     * The call blocks until this has happened.  This method
     * will throw an Error if called from the event dispatcher thread.
     *
     * @param runnable  the <code>Runnable</code> whose <code>run</code>
     *                  method should be executed
     *                  synchronously on the <code>EventQueue</code>
     * @exception       InterruptedException  if another thread has
     *                  interrupted this thread
     * @exception       InvocationTargetException  if an exception is thrown
     *                  when running <code>runnable</code>
     * @see             #invokeLater
     * @since           1.2
     */
    public static void invokeAndWait(Component component, Runnable runnable)
             throws InterruptedException, InvocationTargetException {

        if (EventQueue.isDispatchThread()) {
            throw new Error("Cannot call invokeAndWait from the event dispatcher thread");
        }

	class AWTInvocationLock {}
        Object lock = new AWTInvocationLock();

        InvocationEvent event = 
            new InvocationEvent(Toolkit.getDefaultToolkit(), runnable, lock,
				true);

        synchronized (lock) {
	    SunToolkit.postEvent(SunToolkit.targetToAppContext(component), event);
            lock.wait();
        }

        Exception eventException = event.getException();
        if (eventException != null) {
            throw new InvocationTargetException(eventException);
        }
    }

    /**
     * Causes <code>runnable</code> to have its <code>run</code>
     * method called in the dispatch thread of the <code>EventQueue</code>.
     * This will happen after all pending events are processed.
     * The call blocks until this has happened.  This method
     * will throw an Error if called from the event dispatcher thread.
     *
     * @param runnable  the <code>Runnable</code> whose <code>run</code>
     *                  method should be executed
     *                  synchronously on the <code>EventQueue</code>
     * @exception       InterruptedException  if another thread has
     *                  interrupted this thread
     * @exception       InvocationTargetException  if an exception is thrown
     *                  when running <code>runnable</code>
     * @see             #invokeLater
     * @since           1.2
     */
    public static void invokeAndWait(AppContext appContext, Runnable runnable)
             throws InterruptedException, InvocationTargetException {

        if (EventQueue.isDispatchThread()) {
            throw new Error("Cannot call invokeAndWait from the event dispatcher thread");
        }

	class AWTInvocationLock {}
        Object lock = new AWTInvocationLock();

        InvocationEvent event = 
            new InvocationEvent(Toolkit.getDefaultToolkit(), runnable, lock,
				true);

        synchronized (lock) {
	    SunToolkit.postEvent(appContext, event);
            lock.wait();
        }

        Exception eventException = event.getException();
        if (eventException != null) {
            throw new InvocationTargetException(eventException);
        }
    }
}

