/*
 * @(#)IExplorerAppletContext.java	1.23 10/05/20
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.lang.ref.SoftReference;
import sun.plugin.javascript.ocx.JSObject;
import sun.applet.AppletPanel;
import sun.plugin.AppletViewer;

/**
 * This class corresponds to an applet's environment: the
 * document containing the applet and the other applets in the same
 * document in Internet Explorer.
 * <p>
 * The methods in this interface can be used by an applet to obtain
 * information about its environment.
 *
 */
public class IExplorerAppletContext extends DefaultPluginAppletContext
{
    /**
     * Create a IExplorerAppletContext object.
     */
    public IExplorerAppletContext()
    {
	this.handle = 0;
    }

    /**
     * Set the underlying handle of the Applet context
     * 
     * @param handle Handle
     */
    public void setAppletContextHandle(int handle)
    {
	this.handle = handle;
    }

    /**
     * Get the underlying handle of the Applet context
     * 
     * @return handle Handle
     */
    public int getAppletContextHandle()
    {
	return handle;
    }

    /*
     * Requests that the browser or applet viewer show the Web page
     * indicated by the url argument. 
     *
     * @param url the address to transfer to
     * @param target One of the value
     *	"_self"  show in the current frame
     *  "_parent"show in the parent frame
     *  "_top"   show in the topmost frame
     *  "_blank" show in a new unnamed top-level windownameshow in a 
     *           new top-level window named name
     */
    public void doShowDocument(URL url, String target)
    {
	// Call native method
	if (handle > 0) {
	    /*
             JavaScript URL can only be evaluated by using execScript after the
             MSFT hotfix #867801, therefore the actual JavaScript URL is evaluated 
             by embedding it in an outer JavaScript URL(javascript:window.open()) 
             to support the evaluation in the specified target. 
            */
            if(url.getProtocol().equals("javascript")) {
                //Escape the characters '\'', '\"' and '\\'in the JavaScript URL
                StringBuffer buffer = new StringBuffer();
                String urlStr = url.toString();
                for (int i=0; i < urlStr.length(); i++) {
                    char c = urlStr.charAt(i);
                    if (c == '\'' || c == '\"' || c == '\\')
                        buffer.append('\\');
                    buffer.append(c);
                }
                //Embed the JavaScript URL
                String arg = "javascript:window.open('" + buffer + "', '" + target + "')";
                nativeInvokeScript(handle, JSObject.DISPATCH_METHOD, "execScript", new Object[] {arg});
            } else {
	        Object [] args = new Object[2];
	        String str = url.toString();
	        //See 6396591, convert "file:/c:/foo" to "file:///c:/foo"
	        if(str.startsWith("file:/") 	
	            && str.length() > 6
	            && str.charAt(6) != '/') {
	                str = "file:///" + str.substring(6);
	        }
	        args[0] = str;
	        args[1] = target;
	        nativeInvokeScript(handle, JSObject.DISPATCH_METHOD, "open", args);
	    }
	}
    }

    /*
     * Show status.
     * 
     * @param status status message
     */ 
    public void doShowStatus(String status)
    {
	//During applet shutdown, AxControl window goes away before showing 
	//the satus message and releasing the resources. Therefore we avoid
	//status messages during shutdown
	boolean stopStatus = ((AppletViewer)appletPanel).isStopped();
	if( handle > 0 && !stopStatus){
	    Object [] args = new Object[1];
	    args[0] = status;
	    // Call native method
	    nativeInvokeScript(handle, JSObject.DISPATCH_PROPERTYPUT, "status", args);
	}
    }

    // Native methods
    private native void nativeInvokeScript(int handle, int flags, String name, Object [] args);

    public void onClose() {
	super.onClose();
	handle=0;
    }


    public synchronized netscape.javascript.JSObject getJSObject()
    {
	JSObject jsObj = getJSObject(handle);
	jsObj.setIExplorerAppletContext(this);
	return jsObj;
    }

    // this method isn't used in the old plugin
    public synchronized netscape.javascript.JSObject getOneWayJSObject() {
        return null;
    }


    /* 
     * <p>
     * JSContext implementation
     * @return the JSObject implementation for the current browser 
     * <p>
     */
    private native JSObject getJSObject(int handle);

    public void addJSObjectToLockedList(netscape.javascript.JSObject jsObject) {
	synchronized(locked) {
	    locked.add(jsObject);
	}
    }

    /*
     * <p>
     * Native object reference
     * </p>
     */
    private int handle;

    /*
     * <p>
     * List of exported JSObject to be finalized upon closing the applet
     * </p>
     */
    private java.util.ArrayList locked = new ArrayList();
}


