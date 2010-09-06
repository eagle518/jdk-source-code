/*
 * @(#)ProgressMonitor.java	1.1 04/02/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.lang.ref.SoftReference;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;
import sun.net.ProgressEvent;
import sun.net.ProgressSource;
import sun.net.ProgressListener;

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
	    ProgressEvent pe = new ProgressEvent(pi, pi.getURL(), pi.getMethod(), pi.getContentType(), pi.getState(), pi.getProgress(), pi.getExpected());
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
	    ProgressEvent pe = new ProgressEvent(pi, pi.getURL(), pi.getMethod(), pi.getContentType(), pi.getState(), pi.getProgress(), pi.getExpected());
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
	    ProgressEvent pe = new ProgressEvent(pi, pi.getURL(), pi.getMethod(), pi.getContentType(), pi.getState(), pi.getProgress(), pi.getExpected());
	    listener.progressUpdate(pe);
	}
    }
    
    /**
     * Add progress listener in progress monitor.
     */
    public void addProgressListener(ThreadGroup tg, ProgressListener l)	{

	Trace.msgPrintln("progress.listener.added", new Object[] {l});
	
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
    
	Trace.msgPrintln("progress.listener.removed", new Object[] {l});
	
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
}
