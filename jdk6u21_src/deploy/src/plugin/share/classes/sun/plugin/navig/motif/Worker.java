/*
 * @(#)Worker.java	1.64 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.navig.motif;

import java.io.*;
import java.net.URL;
import java.util.Hashtable;

/* 
 * This class is used to manage the "worker pipe" that allow the java
 * child process to send supplications back to the parental plugin.
 *
 * The Worker class establishes the connection from the plugin to the
 * browser. Immediately after creation, it sends an acknowledgement
 * back to the browser side (the browser waits for this
 * acknowledgement before proceeding after creating the plugin). See
 * JavaVM::StartJavaVM 
 */
public class Worker {


    Worker(DataInputStream data_in, DataOutputStream data_out) {
	this.data_out = data_out;
	this.data_in = data_in;
    }

    /* 
     * Indicate that the request currently in process was
     * abruptly terminated due to some bad condition
     * (usually the plugin instance going away before the
     * request is done).
     */
    static synchronized void terminateRequestAbruptly() {
	terminateAbruptly();
    }

    private static synchronized void pushRequest(String mesg) 
	throws IOException {
        data_out.flush();
	Plugin.trace("Worker pushRequest:" + mesg);
    }

    private static synchronized void writeString(String mess) 	
	throws IOException {
        data_out.writeInt(mess.length());
	for (int i = 0; i < mess.length(); i++) {
	    char ch = mess.charAt(i);
	    data_out.writeByte((byte)ch);
	}
    }


    private static synchronized void writeByteArr(byte[] mess)
	throws IOException {
        data_out.writeInt(mess.length);
	/* This will be horribly slow. Add a buffer or redo in native code? */
	for (int i = 0; i < mess.length; i++) 
	    data_out.writeByte(mess[i]);
    }


    /**
     * Request the plugin to move navigator to a different page.
     * Called by the MotifAppletContext to show a document at the head
     * 
     */
    public static synchronized void showDocument(int pluginIndex, java.net.URL url,
						 String target) {
	try {
	    data_out.writeInt(JAVA_PLUGIN_SHOW_DOCUMENT);
 	    data_out.writeShort(pluginIndex);
	    writeString("" + url);
	    writeString(target);
 	    pushRequest("showDocument");
	} catch (IOException ix) {
	    Plugin.error("java process showDocument: write to parent failed"
			 + "\nException: " + ix.toString());
	}
    }

    /*
     * Show a status messgae for a given plugin instance.
     */
    public static synchronized void showStatus(int pluginIndex, String mess) 
    {
      //	if (!Plugin.isOldNavigator()) {
      //    Plugin.error("Show status not working with Mozilla 5");
      //    return;
      //}
	try {
	    // We often get a flurry of status messages after the plugin
	    // has been deleted.  Avoid queueing these.
	    if (! Plugin.parentAlive()) {
		return;
	    }

            // AppletContext.showStatus states that a null parameter
            // clears the status bar.
            if (mess == null) {
                mess = "";
            }

	    // Send all the status messages to the console as well.
	    data_out.writeInt(JAVA_PLUGIN_SHOW_STATUS);
 	    data_out.writeShort(pluginIndex);
	    writeString(mess);
 	    pushRequest("showstatus:" + mess);
	} catch (IOException ix) {
	    Plugin.error("java process show status: write to parent failed"
			 + "\nException: " + ix.toString());
	}
    }

	/* notify the status change of applet loading */
	public static synchronized void notifyStatusChange(int pluginIndex, int status)
	{
	  try {
	    data_out.writeInt(JAVA_PLUGIN_STATUS_CHANGE);
	    data_out.writeShort(pluginIndex);
	    data_out.writeShort(status);
	    pushRequest("notify status change:" + status);
	  }catch(IOException ix) {
	    Plugin.error("java process fails to notify the browser about the applet status change");
	  }
	}
    
    /*
     * Find out which proxy to use for a given URL. This call
     * must wait until the proxy mapping table has been initialized
     * by the browser. It then calls into the browser and the browser
     * is queried for the proxy mapping. This querying works differently
     * in Netscape 4.0 (and earlier), where a complex Javascript is
     * evaluated to find the proxy, and in Mozilla, where there
     * is a direct interface to find the proxy.
     */
    public static synchronized String getProxySettings(String rawURL) {
	try {
	    URL url = new URL(rawURL);

	    String key = url.getProtocol() + ":" + 
		url.getHost() + ":" + url.getPort();

	    Plugin.trace("getProxySettings. Using key:" + key);

	    // If the result is already cached, don't ask again.
	    String result = (String) proxmap.get(key);

	    if (result != null) {
		Plugin.trace("Retrieving cached proxy:" + result);
		return result;
	    } 
	    // Wait to be alone to make a request
	    enterRequest("Proxy");

	    // Send a request to our parent plugin.
	    data_out.writeInt(JAVA_PLUGIN_FIND_PROXY);
 	    data_out.writeShort(-1);
	    writeString(rawURL);
	    writeString(url.getHost());
	    
 	    pushRequest("FindProxy");

	    // Wait for the result to be put in our proxmap table.
	    waitForResponse("Proxy");
	    
	    result = (String) proxmap.get(key);
	    clearRequest();
	    
	    return result;
	} catch (java.net.MalformedURLException ex) {
	    System.err.println("Bad URL in getProxySettings: " + rawURL);
	} catch (IOException ex) {
	    System.err.println("getProxySettings: IO error on plugin");
	}
	// We only get here if we failed.
	clearRequest();
	return null;
    }
  
    /*
     * Upon a request from the plugin, add a URL to the proxy mapping
     * This sets up a mapping betweeen rawURL and the name by which it
     * can be obtained, "proxy"
     */
    static synchronized void addProxyMapping(String rawURL, String proxy) {
	try {
	    URL url = new URL(rawURL);
	    String key = url.getProtocol() + ":" + url.getHost() 
		+ ":" + url.getPort();
	    proxmap.put(key, proxy);
	    responseIsReady("Proxy");
	} catch (java.net.MalformedURLException ex) {
	    System.err.println("Bad URL in getting proxy: " + rawURL);
	}
    }

     /*
     *  Send a request to the plugin to eval a javascript string
     *  'expr' in the context of the plugin instance 'id'.
     *  May not execute if the plugin is already shut down.
     */
    public static synchronized void sendJSRequest(int id, String expr) 
    {
	try {
	    data_out.writeInt(JAVA_PLUGIN_JAVASCRIPT_REQUEST);
	    data_out.writeShort((short) id);
            // This next integer is ment to indicate if we are using a 
            // temporary file.   Due to a change in the pipe handling on 
            // the browser side this file is obsolete.  We maintain its 
            // protocal spot just in case though. 
	    data_out.writeShort((short) 0);
	    writeString(expr);
	    pushRequest("JS Request");
	} catch (IOException e) {
	    System.err.println("sendJSRequest: io error in Plugin " + e);
	}
    }
		
    /**
     * Find out which cookie to use for a given URL. Called
     * from native code from the ActivatorCookieHandler
     * Shared code keeps a cache and only calls us on cache misses.
     * This is done by querying for all cookies, since there is no direct
     * browser interface for finding the cookies for a URL.
     */
    public static synchronized String findCookieForURL(String rawURL) {
	Plugin.trace("Worker.findCookieForURL: " + rawURL);
	try {
	    // Wait till all other requests are done
	    enterRequest("Cookie");
	    
	    cookieString = null;

	    // Make and send the cookie message
	    data_out.writeInt(JAVA_PLUGIN_FIND_COOKIE);
	    data_out.writeShort(-1);
	    writeString(rawURL);
	    pushRequest("FindCookie");

	    // Wait for a reply
	    waitForResponse("Cookie");

	    String returnString = cookieString; 
	    clearRequest();

	    Plugin.trace(" Got cookie string:" + returnString);
	    return returnString;
	} catch (IOException e) {
	    System.err.println("IOException in findCookieURL");
	} 
	// Even if there is a runtime exception, clear all flags. We
	// may be in a bad state, but limp along if possible
	System.err.println("Bad termination of cookie request!");
	clearRequest();
	return null;
    }


    /**
     * Set cookie for a given URL. Called from ActivatorCookieHandler
     * when the server sends us a "Set-Cookie" HTTP header.
     */
    public static synchronized String setCookieForURL(String rawURL, String cookie) 
    {
	Plugin.trace("Worker.setCookieForURL: " + rawURL + "=" + cookie);

	try {
	    data_out.writeInt(JAVA_PLUGIN_SET_COOKIE);
 	    data_out.writeShort(-1);
	    writeString("" + rawURL);
	    writeString("" + cookie);
 	    pushRequest("setCookie");
	} catch (IOException ix) {
	    Plugin.error("java process setCookie: write to parent failed"
			 + "\nException: " + ix.toString());
	}
	return null;
    }


    /**
     * Receive a response for a cooking request. Pairs up with
     * findCookieForURL and called by Plugin when a cookie appears.
     */
    static synchronized void setCookieString(String cookie) {
	cookieString = cookie;
	responseIsReady("Cookie");
    }

    private static DataInputStream data_in;
    private static DataOutputStream data_out;

     // Cache and state for proxy configuration.
    private static Hashtable proxmap = new Hashtable();

    // Cookie response is stored here
    private static String cookieString = null;

    // Codes for messages from java process to plugin,
    public final static int JAVA_PLUGIN_SHOW_STATUS	   = 0xF60001;
    public final static int JAVA_PLUGIN_SHOW_DOCUMENT	   = 0xF60002;
    public final static int JAVA_PLUGIN_FIND_PROXY	   = 0xF60003;
    public final static int JAVA_PLUGIN_FIND_COOKIE        = 0xF60004;
    public final static int JAVA_PLUGIN_JAVASCRIPT_REQUEST = 0xF60006;
    public final static int JAVA_PLUGIN_SET_COOKIE         = 0xF60009;
	public final static int JAVA_PLUGIN_STATUS_CHANGE      = 0xF6000A;


    /* Handler to serialize requests from the plugin and to handle
     * abrupt termination. Multiple threads can make requests on
     * a request handler and only one will be let through at a time
     * The sequence of calls is:
     *   enterRequest -> waitForResponse -> clearRequest
     *                -> responseIsReady
     *                -> terminateAbruptly
     */
    /*
     * Wait until we can enter a request
     */
    synchronized static void enterRequest(String reqName) {
	try {
	    while(requestStatus != REQUEST_IDLE) 
		Worker.class.wait();
	    requestName = reqName;
	    requestStatus = REQUEST_IN_PROGRESS;
	    Plugin.trace("Entering request for:" + reqName);
	} catch (InterruptedException e) {
	    Plugin.trace("Request was interrupted when entering");
	}
    }

    /*
     * Wait until a response to the cookie, proxie of cache response
     * arrives. 
     * @arg reqName   Informational description of the type of request 
     * @return true if a response was received, false if abruptly
     *             terminated.
     */
    synchronized static boolean waitForResponse(String reqName) {
	Plugin.trace("Waiting for response: " + reqName);
	try {
	    while (requestStatus != RESPONSE_IS_READY) {
		Worker.class.wait();
		Plugin.trace("Woke up in request for:" + reqName);
		if (requestStatus == REQUEST_ABRUPTLY_TERMINATED) {
		    clearRequest();
		    return false;
		}
	    }
	    Plugin.trace("Got response for request:" + reqName);
	    return true;
	} catch(InterruptedException e) {
 	    Plugin.trace("Request was interrupted before response");
	}
	return false;
    }

    /* 
     * Change the request status to indicate that the response
     * is available and wake up all waiting threads
     */
    synchronized static void responseIsReady(String reqName) {
	Plugin.trace("Response is ready:" + reqName);
	requestStatus = RESPONSE_IS_READY;
	Worker.class.notifyAll();
    }

    /*
     * An abrupt termination request with the request incomplete.
     * This does not clear the request.
     */
    synchronized static void terminateAbruptly() {
	Plugin.trace("Request was abruptly terminated");
	if (requestStatus != REQUEST_IDLE) {
	    requestStatus = REQUEST_ABRUPTLY_TERMINATED;
	}
	Worker.class.notifyAll();
    }


    /* 
     * End the current request to permit a new request
     */
    synchronized static void clearRequest() {
	Plugin.trace("Request was cleared");
	requestStatus = REQUEST_IDLE;
        Worker.class.notifyAll();
    }

    /* Name of the current request */
    private static String requestName;

    // Codes for the status of the request. 
    private final static int REQUEST_IDLE = 1;
    private final static int REQUEST_IN_PROGRESS = 2;
    private final static int RESPONSE_IS_READY = 3;
    private final static int REQUEST_ABRUPTLY_TERMINATED = 4;

    // Current request status
    private static int requestStatus = REQUEST_IDLE;

}




