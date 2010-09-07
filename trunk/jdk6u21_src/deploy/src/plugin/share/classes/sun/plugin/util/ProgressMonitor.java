/*
 * @(#)ProgressMonitor.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.lang.ref.SoftReference;
import java.lang.reflect.*;
import java.net.URL;
import java.security.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;
import sun.net.ProgressEvent;
import sun.net.ProgressSource;
import sun.net.ProgressListener;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 * ProgressMonitor is a class for monitoring progress in network input stream
 * in Java Plug-in.
 *
 * @author Stanley Man-Kit Ho
 */
public class ProgressMonitor extends sun.net.ProgressMonitor
{
    /** 
     * Return a snapshot of the ProgressSource list
     */
    public ArrayList getProgressSources()    {
	ArrayList snapshot = new ArrayList();
	
	try {
	    synchronized(progressSourceList)	{
		for (Iterator iter = progressSourceList.iterator(); iter.hasNext();)	{
		    ProgressSource pi = (ProgressSource) iter.next();
    	
		    // Clone ProgressSource and add to snapshot	
		    snapshot.add((ProgressSource)pi.clone());
		}	
	    }
	}
	catch(CloneNotSupportedException e) {
	    e.printStackTrace();
	}
		
	return snapshot;    
    }

    /**
     * Return update notification threshold  
     */
    public int getProgressUpdateThreshold()    {
	// Default is 64K: a number large enough to reduce the 
	// number of listener callbacks but still small enough to show 
	// progress bar update frequently.
	return 65536;
    }
     
    /**
     * Return true if metering should be turned on 
     * for a particular URL input stream. 
     */ 
    public boolean shouldMeterInput(URL url, String method) {
    
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	// Check if there is any listener interested in metering
	synchronized(threadGroupListenerMap)
	{
	    listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));
	}
	
	// Metering is disabled by default if there is no progress listener
	if (listener == null)
	    return false;
    
	// We are only interested in monitoring HTTP/HTTPS/FILE GET request
	String protocol = url.getProtocol();
	
	if ((protocol.equalsIgnoreCase("http") 
	    || protocol.equalsIgnoreCase("https")
	    || protocol.equalsIgnoreCase("file"))
	    && method.equalsIgnoreCase("GET"))
	    return true;
	else
	    return false;
    }
    
    /** 
     * Register progress source when progress is began.
     */
    public void registerSource(ProgressSource pi) {

	synchronized(progressSourceList)    
	{
	    if (progressSourceList.contains(pi))
		return;

	    progressSourceList.add(pi);
	}
	
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	//
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroupListenerMap)
	{
	    listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    ProgressEvent pe = newProgressEvent(pi);
	    listener.progressStart(pe);
	}
    }

    /** 
     * Unregister progress source when progress is finished.
     */
    public void unregisterSource(ProgressSource pi) {
	    
	synchronized(progressSourceList) 
	{
	    // Return if ProgressEvent does not exist
	    if (progressSourceList.contains(pi) == false)
		return;
		
	    // Close entry and remove from map	
	    pi.close();
	    progressSourceList.remove(pi);
	}
		    
	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroupListenerMap)
	{
	    listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    ProgressEvent pe = newProgressEvent(pi);
	    listener.progressFinish(pe);
	}
    }

    /** 
     * Progress source is updated.
     */
    public void updateProgress(ProgressSource pi)   {

	synchronized (progressSourceList)   {
	    if (progressSourceList.contains(pi) == false)
		return;
	} 

	// Find the proper progress listener from the registered list 
	// accordingly to the caller thread group.
	// 
	// It is VERY VERY important that the event dispatch 
	// doesn't occur until we get out of the synchronization
	// block. Otherwise, multiple dispatches from different
	// threads in the same thread group may result in
	// deadlock in case the callee is blocked
	//
	Thread t = Thread.currentThread();
	ThreadGroup tg = t.getThreadGroup();
	ProgressListener listener = null;

	synchronized(threadGroupListenerMap)
	{
	    listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));
	}

	// We MUST not be in synchronization block 
	// when we do dispatch
	//
	if (listener != null)
	{
	    ProgressEvent pe = newProgressEvent(pi);
	    listener.progressUpdate(pe);
	}
    }
    
    /**
     * Add progress listener in progress monitor.
     */
    public void addProgressListener(ThreadGroup tg, ProgressListener l)	{

	Trace.msgPrintln("progress.listener.added", new Object[] {l}, TraceLevel.BASIC);
	
	// Soft reference is used to ensure that ThreadGroup and
	// ProgressListener will not be held even if the caller 
	// fails to remove the listener for some reasons.
	//
	synchronized(threadGroupListenerMap)
	{
	    // First check if we have this ThreadGroup in the HashMap table.
	    ProgressListener listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));

	    // Use event multicaster to broadcast events
	    listener = EventMulticaster.add(listener, l);

	    // Add back to HashMap
	    threadGroupListenerMap.put(new Integer(tg.hashCode()), listener);
	}
    }
    
    /**
     * Remove progress listener from progress monitor.
     */
    public void removeProgressListener(ThreadGroup tg, ProgressListener l) {
    
	Trace.msgPrintln("progress.listener.removed", new Object[] {l}, TraceLevel.BASIC);
	
	synchronized(threadGroupListenerMap)
	{
	    // First check if we have this ThreadGroup in the HashMap table.
	    ProgressListener listener = (ProgressListener) threadGroupListenerMap.get(new Integer(tg.hashCode()));

	    // Use event multicaster to broadcast events
	    listener = EventMulticaster.remove(listener, l);

	    // Add back to HashMap
	    if (listener != null)
		threadGroupListenerMap.put(new Integer(tg.hashCode()), listener);
	    else
		threadGroupListenerMap.remove(new Integer(tg.hashCode()));
	}		
    }

    // ArrayList for outstanding progress sources
    private ArrayList progressSourceList = new ArrayList();   
    
    // HashMap for ThreadGroups of progress listeners
    private HashMap threadGroupListenerMap = new HashMap();    

    //----------------------------------------------------------------------
    // Helper methods
    //

    public static long getProgress(ProgressEvent event) {
        if (progressEventGetProgressMethod == null) {
            progressEventGetProgressMethod = getProgressEventMethod("getProgress");
        }
        try {
            return ((Number) progressEventGetProgressMethod.invoke(event, null)).longValue();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // We promote to long internally even if using the int versions of the methods
    public static long getExpected(ProgressEvent event) {
        if (progressEventGetExpectedMethod == null) {
            progressEventGetExpectedMethod = getProgressEventMethod("getExpected");
        }
        try {
            return ((Number) progressEventGetExpectedMethod.invoke(event, null)).longValue();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // Because ProgressSource and ProgressEvent changed in JDK 7 to
    // use longs instead of ints, we have to make certain method calls
    // via reflection
    private static volatile Method      progressSourceGetProgressMethod;
    private static volatile Method      progressSourceGetExpectedMethod;
    private static volatile Method      progressEventGetProgressMethod;
    private static volatile Method      progressEventGetExpectedMethod;
    private static volatile Constructor progressEventCtor;
    private static volatile boolean     usingLongs;  // JDK 7 semantics?

    private static Method getProgressSourceMethod(final String method) {
        return (Method) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    try {
                        Method m = ProgressSource.class.getDeclaredMethod(method, null);
                        m.setAccessible(true);
                        return m;
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                }
            });
    }

    // We promote to long internally even if using the int versions of the methods
    private static long getProgress(ProgressSource source) {
        if (progressSourceGetProgressMethod == null) {
            progressSourceGetProgressMethod = getProgressSourceMethod("getProgress");
        }
        try {
            return ((Number) progressSourceGetProgressMethod.invoke(source, null)).longValue();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // We promote to long internally even if using the int versions of the methods
    private static long getExpected(ProgressSource source) {
        if (progressSourceGetExpectedMethod == null) {
            progressSourceGetExpectedMethod = getProgressSourceMethod("getExpected");
        }
        try {
            return ((Number) progressSourceGetExpectedMethod.invoke(source, null)).longValue();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static Method getProgressEventMethod(final String method) {
        return (Method) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    try {
                        Method m = ProgressEvent.class.getDeclaredMethod(method, null);
                        m.setAccessible(true);
                        return m;
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                }
            });
    }

    private static Constructor getProgressEventConstructor(boolean useLongs) {
        try {
            return ProgressEvent.class.getDeclaredConstructor(new Class[] {
                ProgressSource.class,
                URL.class,
                String.class,
                String.class,
                ProgressSource.State.class,
                useLongs ? Long.TYPE : Integer.TYPE,
                useLongs ? Long.TYPE : Integer.TYPE
            });
        } catch (NoSuchMethodException e) {
            return null;
        }
    }

    private static ProgressEvent newProgressEvent(ProgressSource source) {
        if (progressEventCtor == null) {
            progressEventCtor = (Constructor) AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        try {
                            // NOTE: trying JDK 7 semantics first
                            Constructor c = getProgressEventConstructor(true);
                            if (c != null) {
                                usingLongs = true;
                            } else {
                                c = getProgressEventConstructor(false);
                            }
                            c.setAccessible(true);
                            return c;
                        } catch (Exception e) {
                            throw new RuntimeException(e);
                        }
                    }
                });
        }
        long progress = getProgress(source);
        long expected = getExpected(source);
        Object progressBox = null;
        Object expectedBox = null;
        if (usingLongs) {
            progressBox = new Long(progress);
            expectedBox = new Long(expected);
        } else {
            progressBox = new Integer((int) progress);
            expectedBox = new Integer((int) expected);
        }
        try {
            return (ProgressEvent) progressEventCtor.newInstance(new Object[] {
                source,
                source.getURL(),
                source.getMethod(),
                source.getContentType(),
                source.getState(),
                progressBox,
                expectedBox
            });
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // This method exists only to provide some static type checking
    // for our dependent methods in the ProgressSource and
    // ProgressEvent classes. If these classes change in an
    // incompatible way, we will have to update the reflection code
    // above.
    static ProgressEvent unusedCreateProgressEvent(ProgressSource source) {
        return new ProgressEvent(source,
                                 source.getURL(),
                                 source.getMethod(),
                                 source.getContentType(),
                                 source.getState(),
                                 source.getProgress(),
                                 source.getExpected());
    }
}
