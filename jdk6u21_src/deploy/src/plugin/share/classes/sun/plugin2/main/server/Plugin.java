/*
 *  @(#)Plugin.java	1.12 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import netscape.javascript.*;
import sun.plugin2.liveconnect.*;
import com.sun.deploy.net.cookie.CookieUnavailableException;
import java.net.PasswordAuthentication;
import java.net.URL;

/** Defines services which a plugin implementation has to provide to
    the rest of the server-side infrastructure, such as execution of
    code on the browser's main thread. */

public interface Plugin {
    /** Runs the given Runnable on the browser's (or plugin's) main
        thread. */
    public void invokeLater(Runnable runnable);

    /** Signals the main thread that it should wake up if it is
        blocked on a certain operation. This is mainly used to make
        the implementation of JavaScript-to-Java calls portable. */
    public void notifyMainThread();

    /** Returns the document base of the plugin instance. Note that it
        is possible to fetch this value in a portable way using
        LiveConnect, but in order to bootstrap the legacy applet
        lifecycle, we need the document base to be present in all
        situations at the beginning of time, before the applet's
        execution environment is fully set up. Therefore proper
        support for this method is required in all ports. If you
        return null, you will get failures if an applet requests the
        legacy applet lifecycle. */
    public String getDocumentBase();

    /** Show document specified by URL in the specified target. */
    public void showDocument(String url, String target);

    /** Show applet status in the browser */
    public void showStatus(String status);

    /** Requests the cookie for the given URL from the browser. */
    public String getCookie(URL url) throws CookieUnavailableException;

    /** Sets the cookie for the given URL in the browser. */
    public void setCookie(URL url, String value) throws CookieUnavailableException;

    /** Requests authentication via the browser. */
    public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
                                                    String scheme, String realm, URL requestURL,
                                                    boolean proxyAuthentication);

    /** Sets the child window handle associated with the plugin. This
        is currently only needed on Mac OS X. */
    public void setChildWindowHandle(long windowHandle);

    //----------------------------------------------------------------------
    // LiveConnect functionality
    //

    // NOTE that when implementing these methods it is essential to
    // properly manage the BrowserSideObjects exposed to the outside
    // world. We basically assume a reference counting scheme is used
    // on the browser side. Every BrowserSideObject that is passed to
    // the outside world must have its reference count incremented
    // exactly once for each applet instance in which it is exposed.
    // The LiveConnectSupport class in this package takes care of
    // this. Implementations of these methods and in general of
    // LiveConnect should delegate responsibility for this reference
    // count tracking to the LiveConnectSupport class, which will turn
    // around and call javaScriptRetainObject as necessary. When the
    // plugin receives the javaScriptReleaseObject notification for a
    // given BrowserSideObject, that is the indication that the
    // reference count of the object should be decremented and that it
    // should be removed from this internal table. Higher-level code
    // in the JVMManager and JVMInstance classes, as well as in the
    // client JVMs attached to this one, take care of tracking which
    // JVMs have received references to which JavaScript objects and
    // properly calling javaScriptReleaseObject when all references
    // have been released.

    /** Fetches the BrowserSideObject corresponding to the JavaScript
        window object for this plugin instance. An easy way for a
        plugin implementor to skip implementing LiveConnect support
        initially is to return null from this method. The plugin
        should not release the returned object until a
        javaScriptReleaseObject call referring to an equivalent
        BrowserSideObject is received.  Note that it is guaranteed
        that this will be called on the browser's (or plugin's) main
        thread. */
    public BrowserSideObject javaScriptGetWindow();

    /** Increments the reference count of the given JavaScript object.
        This is called from the LiveConnectSupport class in this
        package, which assumes the responsibility for reference count
        tracking and validity checking of JavaScript objects. Note
        that it is guaranteed that this method will be called from the
        browser's (or plugin's) main thread. */
    public void javaScriptRetainObject(BrowserSideObject obj);

    /** Releases the given JavaScript object. This will not be called
        until the object is no longer reachable from any client JVM
        instance attached to this one. Note that it is guaranteed that
        this method will be called from the browser's (or plugin's)
        main thread. */
    public void javaScriptReleaseObject(BrowserSideObject obj);

    /** Calls a JavaScript method. Note that it is guaranteed that
        this method will be called from the browser's (or plugin's)
        main thread. */
    public Object javaScriptCall(BrowserSideObject obj, String methodName, Object[] args) throws JSException;

    /** Evaluates a JavaScript expression. Note that it is guaranteed
        that this method will be called from the browser's (or
        plugin's) main thread. */
    public Object javaScriptEval(BrowserSideObject obj, String code) throws JSException;

    /** Retrieves a named member of a JavaScript object. Note that it
        is guaranteed that this method will be called from the
        browser's (or plugin's) main thread. */
    public Object javaScriptGetMember(BrowserSideObject obj, String name) throws JSException;

    /** Sets a named member of a JavaScript object. Note that it is
        guaranteed that this method will be called from the browser's
        (or plugin's) main thread. */
    public void javaScriptSetMember(BrowserSideObject obj, String name, Object value) throws JSException;

    /** Removes a named member of a JavaScript object. Note that it is
        guaranteed that this method will be called from the browser's
        (or plugin's) main thread. */
    public void javaScriptRemoveMember(BrowserSideObject obj, String name) throws JSException;

    /** Retrieves an indexed member of a JavaScript object. Note that
        it is guaranteed that this method will be called from the
        browser's (or plugin's) main thread. */
    public Object javaScriptGetSlot(BrowserSideObject obj, int index) throws JSException;

    /** Sets an indexed member of a JavaScript object. Note that it is
        guaranteed that this method will be called from the browser's
        (or plugin's) main thread. */
    public void javaScriptSetSlot(BrowserSideObject obj, int index, Object value) throws JSException;

    /** Converts this JavaScript object into a String representation. */
    public String javaScriptToString(BrowserSideObject obj);
    
    /** Returns a number of Java Script calls actualy on the stack. */
    public int getActiveJSCounter();
       
    /** Increments a number of active Java Script calls on the stack. */
    public void incrementActiveJSCounter();

    /** Decrements a number of active Java Script calls on the stack. */
    public void decrementActiveJSCounter();
    
    
    //----------------------------------------------------------------------
    // Modal dialog support
    //

    /** Wait for a signal via {@link #notifyMainThread
        notifyMainThread}, blocking input events to the web browser in
        the interim because a Java modal dialog is up. FIXME: may need
        to rethink the structure of this API for best portability. */
    public void waitForSignalWithModalBlocking();
}
