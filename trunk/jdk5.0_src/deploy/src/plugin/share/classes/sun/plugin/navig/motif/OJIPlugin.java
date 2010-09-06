
/*
 * @(#)OJIPlugin.java	1.13 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
	   enterSpontaneousMonitor();
	}
	return pipe;
    }

    /* 
       Release the pipe associated with the current thread. First find
       the appropriate AThread in the hash table and delegate the call. 
       This is called from the native code that sends javascript calls 
    */
    public static void releasePipeForCurrentThread() {
      Thread ct = Thread.currentThread();
      if (ct != null) {
	Plugin.trace(" OJIPlugin release for:"  + ct.toString());
      } else {
	Plugin.trace(" OJIPlugin release for: null thread!?");
      }
      if (!threads.contains(ct)) {
	Plugin.trace("OJIPlugin releasePipe - exiting spont monitor");
	exitSpontaneousMonitor();
      } else {
	Plugin.trace("OJIPlugin release - non spontaneous pipe");
      }
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

    /* Acquire/Release the spontaneous monitor. The use of the spontaneous
       monitor is bracketed at the C level, but not at the Java level.
       These calls are made from AThread */
    static void enterSpontaneousMonitor() {
        Plugin.trace("Trying to enter spont monitor: " + spontcount);
	enterMonitor(SpontMonitor);
        spontcount++;
    }

    static void exitSpontaneousMonitor() {
	spontcount--;
	exitMonitor(SpontMonitor);
    }

    public final static int SpontFD = 10;

    private static int spontcount = 0;
    private static Hashtable threads = new Hashtable();
    private static Object SpontMonitor = new Object();
    private static native void enterMonitor(Object monitorobj);
    private static native void exitMonitor(Object monitorobj);
    private static DataOutputStream spontOut;
    private static DataInputStream spontIn;
}
