
/*
 * @(#)OJIPlugin.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.navig.motif;

import java.io.*;
import java.util.*;
import sun.plugin.viewer.MNetscapePluginObject;


/* OJI specific plugin code. 
   @author Benedict Gomes
*/
public class OJIPlugin {

    public static void initialize() {
		/* 
		   These FDs are not really accessed from within Java, only in
		   native methods
		*/	
        spontIn = Plugin.newInput("Spont Comm", SpontFD);
		spontOut = Plugin.newOutput("Spont Comm", SpontFD);

		Object readlock = new Object();
		Object writelock = new Object();
		initializePipe(SpontFD, readlock, writelock);
	
    }


    public static void initializePipe(int pipe, Object rlock, Object wlock) {
		nativeInitializePipe(pipe, rlock, wlock);
    }

   /* 
       Called from native code. First find the AThread, and then ask
       it to return the appropriate pipe Return the pipe for the
       current thread 
    */
    public static int acquirePipeForCurrentThread() {
        AThread th = getCurrentAThread();
	int pipe;
	/* Note that this method does not need to be synchronized
	   since getCurrentAThread is synchronized. It should
	   not be synchronized, or it may block in enterSpontaneousMonitor,
	   disabling any other operations on the class */
	if (th != null) {
	    pipe = th.getPipe();
	    Plugin.trace("OJIPlugin acq thread=:" + th.toString() 
					   + " pipe=" + pipe);
	} else {
	   pipe = SpontFD;
	   Plugin.trace("OJIPlugin acq Spontaneous pipe=" + pipe);
	}
	return pipe;
    }

    /*
      Synchronized means to register and check for the presence of
       the current thread in the threads hash table. 
    */
    /* 
       Return the AThread for the current thread or null if it
       has not been registered 
    */
    private static synchronized AThread getCurrentAThread() {
      Thread ct = Thread.currentThread();
      Plugin.trace("Current thread:" + ct.toString());
      if (threads.contains(ct)) {
	Plugin.trace("OJIPlugin: Retrieve the AThread\n");
	return (AThread) threads.get(ct);
      } else {
	Plugin.trace("OJIPlugin: No AThread\n");
	return null;
      }
    }


    public static synchronized void registerThread(AThread th) {
        Thread ct = Thread.currentThread();
        Plugin.trace("Registering thread: " + ct.toString() 
			   + " with AThread " + th.toString());
	threads.put(ct, th);
    }

    public final static int SpontFD = 10;

    private static native void nativeInitializePipe(int pipe, Object rlock, Object wlock);

    private static Hashtable threads = new Hashtable();
    private static DataOutputStream spontOut;
    private static DataInputStream spontIn;
}
