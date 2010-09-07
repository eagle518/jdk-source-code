/*
 * @(#)IExplorerPlugin.java	1.56 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.awt.Rectangle;
import java.io.*;
import java.net.PasswordAuthentication;
import java.net.URL;
import java.util.*;

import com.sun.deploy.net.cookie.CookieUnavailableException;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import netscape.javascript.*;
import sun.plugin2.ipc.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.*;

/** This class corresponds to the underlying ActiveX control in the
    IE-specific native code, which calls up into this class to do most
    of its work. AxControl.cpp knows about this class's name and its
    methods. <P>

    NOTE that the native methods for this class are pulled in from the
    jp2iexp.dll which is already loaded by the IE process by the time
    we get into this code. We use the NativeLibLoader to System.load()
    this library to make the native methods visible to the boot class
    loader, so we don't have to use RegisterNatives to hook up the
    native methods (which is both high-maintenance and error-prone).
    Using System.load() instead of RegisterNatives also improves the
    ability to test this class in isolation of the web browser.
*/

public class IExplorerPlugin extends AbstractPlugin {
    // Pointer to the underlying CAxControl instance; unclear if we're
    // going to need this
    private long cAxControl;
    // The parent window of our control window (unclear whether we need this)
    private long hWndParent;
    // The HWND corresponding to the control window
    private long hWndControlWindow;
    // The initial location of the control window
    private Rectangle initialLocation;

    // Small state machine to ensure we get a rectangle from the
    // browser before attempting to start the applet
    private boolean activated;
    private boolean gotInitialSize;
    private int     initialWidth;
    private int     initialHeight;

    // The applet parameters, extracted from the underlying IPersistPropertyBag
    private Map/*<String,String>*/ params = new HashMap/*<String,String>*/();

    // An indication of whether we've started the applet yet
    private boolean started;
    private AppletID appletID;

    // An Event object used for signalling the browser main thread during JavaScript -> Java calls
    private Event mainThreadEvent;

    // Java version string based on the clsid.
    // By design, this could be null in the case if 
    // the dynamic clsid is being used
    private String javaVersion;

    // Let's see whether we can pull in the natives by loading
    // jp2iexp.dll with the boot loader even though it's already
    // loaded in the process
    static {
        sun.plugin2.util.NativeLibLoader.load(new String[] {"jp2iexp", "deploy"});
        try {
            // Install browser services such as proxy, cookie and etc.
            ServiceManager.
                setService((Service)Class.forName("sun.plugin.services.WIExplorerBrowserService").newInstance());
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        JVMManager.setBrowserType(BrowserType.INTERNET_EXPLORER);
    }

    public IExplorerPlugin(long cAxControl, String javaVersion) {
        this.cAxControl = cAxControl;
	this.javaVersion = javaVersion;
        if (DEBUG) {
            System.out.println("IExplorerPlugin.IExplorerPlugin(" + toHexString(cAxControl) + ")");
	    System.out.println("IExplorerPlugin.IExplorerPlugin(" + javaVersion + ")");
        }
        mainThreadEvent = IPCFactory.getFactory().createEvent(null);
    }

    public void FinalRelease() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.FinalRelease(cAxControl = " + toHexString(cAxControl) + ")");
        }

        mainThreadEvent.dispose();
    }

    public void OnSetFocus() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.OnSetFocus(cAxControl = " + toHexString(cAxControl) + ")");
        }
        if (appletID != null) {
            JVMManager.getManager().synthesizeWindowActivation(appletID, true);
        }
    }

    public void OnDestroy() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.OnDestroy(cAxControl = " + toHexString(cAxControl) + ")");
        }
        // FIXME: not sure whether we need to do anything here or in
        // fact receive this notification at all
    }

    public void CreateControlWindow(long hWndParent,
                                    long hWndControlWindow,
                                    int rectLeft,
                                    int rectRight,
                                    int rectTop,
                                    int rectBottom) {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.CreateControlWindow(cAxControl = " + toHexString(cAxControl) +
                               ", hWndParent = " + toHexString(hWndParent) + ", hWndControlWindow = " + toHexString(hWndControlWindow) + ")");
        }
        this.hWndParent = hWndParent;
        this.hWndControlWindow = hWndControlWindow;
        initialLocation = new Rectangle(rectLeft, rectTop,
                                        rectRight - rectLeft,
                                        rectBottom - rectTop);
        // FIXME: send this information over to the other side to
        // create and size the embedded frame (actually, defer this
        // until we have the applet information all ready to go)
    }

    public boolean OnDraw(long hdc) {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.OnDraw(cAxControl = " + toHexString(cAxControl) + ")");
        }
        if (appletID != null) { 
            return JVMManager.getManager().printApplet(appletID, hdc, 0, 0, 0, 0); 
        }
        return true;
    }

    public void InPlaceActivate() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.InPlaceActivate(cAxControl = " + toHexString(cAxControl) + ") entered");
        }

        activated = true;
        startBackgroundStarter();
        // FIXME: note that this is done differently than in the native plugin
        if (DEBUG) {
            System.out.println("IExplorerPlugin.InPlaceActivate(cAxControl = " + toHexString(cAxControl) + ") exited");
        }
    }

    public void InPlaceDeactivate() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.InPlaceDeactivate(cAxControl = " + toHexString(cAxControl) + ")");
        }
        // Used to do this in OnDestroy; this seems to be a better place to do it
        // (i.e., the control window is definitely still alive -- this is important for clean shutdown)
        cancelBackgroundStarter();
        activated = false;
        gotInitialSize = false;
    }

    public void SetObjectRects(int rectLeft,
                               int rectRight,
                               int rectTop,
                               int rectBottom) {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.SetObjectRects(cAxControl = " + toHexString(cAxControl) +
                               ", left = " + rectLeft + ", right = " + rectRight + ", top = " + rectTop +
                               ", bottom = " + rectBottom + ")");
        }
        // FIXME: consider passing up the clip region as well and doing something with it

        int width  = rectRight - rectLeft;
        int height = rectBottom - rectTop;
        if (width >= 0 && height >= 0) {
            // Ignore bogus inputs as we'll automatically correct for
            // this on the other side by parsing the width and height
            // out of the applet parameters; see comment in old
            // plugin's CAxControl::SetObjectRects about the "pogo
            // applets bug"
            if (appletID != null) {
                JVMManager.getManager().setAppletSize(appletID, width, height);
            } else {
                gotInitialSize = true;
                initialWidth   = width;
                initialHeight  = height;
                startBackgroundStarter();
            }
        }

        // IE sends one or more bogus SetObjectRects when the plugin
        // is first instantiated for fully visible applets, with left
        // = right = top = bottom = 0. We want to filter these out as
        // much as possible since they cause race conditions; some
        // applets expect that their size is set completely correctly
        // when they are first started. However, some applets might
        // not be initially visible but for correctness and
        // compatibility reasons we need to start them even though
        // they have a zero width and height. We have found that it is
        // impossible to write a set of heuristics which works in all
        // cases so the best we can do if we get a bogus value is
        // delay the applet's start a little bit and hope that we get
        // a more correct width and height a little further in the
        // future. If this heuristic fails, we eventually start the
        // applet regardless, even if this means giving it an
        // incorrect initial width and height.

        if (rectLeft == 0 && rectRight == 0 && rectTop == 0 && rectBottom == 0 &&
            getIntParameter("width") > 1 && getIntParameter("height") > 1) {
            if (DEBUG) {
                System.out.println("   (detected bogus SetObjectRects; requested width = " +
                                   params.get("width") + ", requested height = " + params.get("height") + ")");
            }
            delayBackgroundStarter();
        } else {
            undelayBackgroundStarter();
        }
    }

    public boolean SetClientSite(long oleClientSite,
                                 long scriptDispatch) {
        try {
            if (DEBUG) {
                System.out.println("IExplorerPlugin.SetClientSite(cAxControl = " + toHexString(cAxControl) +
                                   ", oleClientSite = " + toHexString(oleClientSite) + ")");
            }

	    clientSite = oleClientSite;

            if (javaScriptWindow != 0) {
                iUnknownRelease(javaScriptWindow);
                javaScriptWindow = 0;
            }

            // The incoming IDispatch* is the Script property out of the
            // IHTMLDocument we live in. It also happens to be an
            // IHTMLWindow (at least in the current IE browsers). We need
            // this fact in order to implement JSObject.eval().
            if (scriptDispatch != 0 && iDispatchIsIHTMLWindow2(scriptDispatch)) {
                javaScriptWindow = scriptDispatch;
                // Manually increment the reference count of this one
                // since we hold on to it in addition to any clients that
                // might
                iUnknownAddRef(javaScriptWindow);

                // Initialize Java namespace support -- support for formerly Mozilla-specific
                // functionality: the Packages keyword, calling static methods, and allocating
                // Java objects from JavaScript.
                BrowserSideObject window = javaScriptGetWindowInternal(false);
                if (window != null) {
                    try {
                        defineNameSpaceVariable(window, "Packages", "");
                        defineNameSpaceVariable(window, "java",     "java");
                        defineNameSpaceVariable(window, "netscape", "netscape");
                        if (DEBUG) {
                            System.out.println("IExplorerPlugin.SetClientSite: successfully defined namespace variables");
                        }
                    } catch (JSException e) {
                        e.printStackTrace();
                    }
                }
            }

            // FIXME: start to shut things down if the oleClientSite is null?
            return true;
        } catch (RuntimeException e) {
            e.printStackTrace();
            throw(e);
        } catch (Error e) {
            e.printStackTrace();
            throw(e);
        }
    }

    public void OnFrameWindowActivate(boolean activate) {
        if (DEBUG) {
            System.out.println("IExplorerPlugin.OnFrameWindowActivate(cAxControl = " + toHexString(cAxControl) + ", " + activate + ")");
        }
        // Sending synthetic window deactivation messages causes
        // problems in at least some situations
        if (appletID != null && activate) {
            JVMManager.getManager().synthesizeWindowActivation(appletID, activate);
        }
    }

    public void addParameters(String[] keys, String[] values) {
        for (int i = 0; i < keys.length; i++) {
            addParameter(keys[i], values[i]);
        }
    }

    public void addParameter(String key, String value) {
        // Some of the parameters coming up from native code might be
        // null; filter these out for cleanliness
        if (key != null) {
            key = key.trim().toLowerCase(java.util.Locale.ENGLISH);
            // The result of GetAmbientDisplayName should only be used
            // if the name was not specified in the applet parameters;
            // the applet parameters are passed up first in the
            // initialization sequence
            if (key.equals("name")) {
                String tmp = (String) params.get(key);
                if (tmp == null || tmp.equals("")) {
                    // Only in this case should we consider the new value
                    if (value != null && !value.equals("")) {
                        params.put(key, value);
                    }
                }
                return;
            }
            params.put(key, value);
        }
    }

    // This is supposed to be called after addParameters
    public int getBgColor() {
	String colorStr = (String) params.get(ParameterNames.BOX_BG_COLOR);
	if (colorStr != null) {
	    return ColorUtil.createColorRGB(ParameterNames.BOX_BG_COLOR, colorStr);
	} else {
	    return 0;
	}
    }

    //----------------------------------------------------------------------
    // Background starting of applet

    // This is a heuristic to work around poor behavior of IE. When
    // the applet tag is dynamically inserted into the document via
    // JavaScript, IE activates and deactivates the ActiveX control
    // more than once before finally settling down. We ideally want
    // the applet to be started only once, because stopping and
    // re-starting it causes JavaScript errors and therefore problems
    // for some Java detection mechanisms.

    protected long getScriptingObjectForApplet(long exceptionInfo) {
        waitForBackgroundStarter();
        return super.getScriptingObjectForApplet(exceptionInfo);
    }

    protected Object getJavaNameSpace(String nameSpace) {
        waitForBackgroundStarter();
        return super.getJavaNameSpace(nameSpace);
    }

    private synchronized void startBackgroundStarter() {
        if (backgroundStarter == null) {
            if (hWndControlWindow != 0 &&
                activated &&
                gotInitialSize &&
                params != null) {
                backgroundStarter = new BackgroundStarter();
                backgroundStarter.start();
            }
        }
    }

    private synchronized void delayBackgroundStarter() {
        if (backgroundStarter != null) {
            backgroundStarter.delay();
        } else {
            _delayed = true;
        }
    }

    private synchronized void undelayBackgroundStarter() {
        _delayed = false;
        if (backgroundStarter != null) {
            backgroundStarter.undelay();
        }
    }

    private synchronized void cancelBackgroundStarter() {
        if (backgroundStarter != null) {
            backgroundStarter.cancel();
            backgroundStarter = null;
        } else {
            stopApplet();
        }
    }

    private void waitForBackgroundStarter() {
        boolean shouldWait = false;
        do {
            synchronized(this) {
                shouldWait = (backgroundStarter != null);
            }
            if (shouldWait) {
                runMessagePump();
            }
        } while (shouldWait);
    }

    private volatile BackgroundStarter backgroundStarter;
    private boolean _delayed;
    class BackgroundStarter extends Thread {
        private boolean cancelled;
        private boolean delayed;

        BackgroundStarter() {
            super("Background Applet Starter");
            this.delayed = _delayed;
        }

        public void cancel() {
            synchronized(IExplorerPlugin.this) {
                cancelled = true;
            }
        }

        public void delay() {
            synchronized(IExplorerPlugin.this) {
                delayed = true;
            }
        }

        public void undelay() {
            synchronized(IExplorerPlugin.this) {
                delayed = false;
                IExplorerPlugin.this.notifyAll();
            }
        }

        public void run() {
            synchronized(IExplorerPlugin.this) {
                do {
                    boolean wasDelayed = delayed;
                    delayed = false;
                    try {
                        // The 100 ms is a completely heuristic value
                        // but appears to be sufficient; the browser
                        // very quickly deactivates and reactivates us.
                        // The 500 ms is also a completely heuristic
                        // value but appears to be sufficient to
                        // achieve page layout without delaying too
                        // badly the start of applets which don't meet
                        // our heuristics.
                        IExplorerPlugin.this.wait(wasDelayed ? 500 : 100);
                    } catch (InterruptedException e) {
                    }
                } while (delayed);
                if (!cancelled) {
                    // Really start the applet
                    invokeLater(new Runnable() {
                            public void run() {
                                maybeStartApplet();
                                // It is crucial that this be set to null here
                                // to avoid race conditions in waitForBackgroundStarter()
                                backgroundStarter = null;
                                notifyMainThread();
                            }
                        });
                } else {
                    notifyMainThread();
                }
            }
        }
    }

    //----------------------------------------------------------------------
    // Implementation of the Plugin interface
    //

    private String documentBase = null;

    // NOTE that this apparently only works once CreateControlWindow
    // has completely returned (including at the C++ level), as that
    // seems to be when the WndProc and therefore the message map is
    // set up
    public void invokeLater(Runnable r) {
        if (hWndControlWindow != 0) {
            invokeLater0(hWndControlWindow, r);
        }
    }

    private native void invokeLater0(long hWnd, Runnable runnable);
        
    public void notifyMainThread() {
        mainThreadEvent.signal();
    }

    public String getDocumentBase() {
        if (documentBase == null) {
            if (javaScriptWindow == 0) {
                throw new IllegalStateException("Can not fetch the document base before SetClientSite is called");
            }

            // Unfortunately even our internal JavaScript support
            // isn't bootstrapped at the time we need to make these
            // calls (we don't yet have a valid applet ID), so we need
            // to do this in native code at least for the time being
            documentBase = getDocumentBase0(javaScriptWindow);
        }                

        return documentBase;
    }

    private static native String getDocumentBase0(long iHTMLWindow2);

    private static String convertTarget(String target) {
	// The target name MUST be either alphanumeric or 
	// underscore character, or the "open" call won't
	// work. So it is VERY important to do the proper
	// conversion.
	//
	StringBuffer buffer = new StringBuffer();
	for (int i=0; i < target.length(); i++)	{
	    char c = target.charAt(i);

	    if (Character.isLetterOrDigit(c) || c == '_')
		buffer.append(c);
	    else
		buffer.append('_');
        }
        return buffer.toString();
    }

    public void showDocument(String url, String target) {
        BrowserSideObject browserObj = javaScriptGetWindowInternal(false);
        if (browserObj != null) {           
            /*
              JavaScript URL can only be evaluated by using javaScriptEval after
              MSFT hotfix #867801, therefore the actual JavaScript URL is evaluated 
              by embedding it in an outer JavaScript URL(javascript:window.open()) 
              to support the evaluation in the specified target. 
            */
	    if (url.startsWith("javascript:")) {
		url = escapeURL(url);
		javaScriptEval(browserObj, "javascript:window.open('" + url + 
		    " ','" + target + "')");
	    } else {
		String modUrl = url;
		//See 6396591, convert "file:/c:/foo" to "file:///c:/foo"
		if (url.startsWith("file:/") && url.length() > 6 
		    && url.charAt(6) != '/') {
		    modUrl = "file:///" + url.substring(6);
		}
		// The iWebBrowser2->Navigate method doesn't work well
		// when the "TargetFrameName" is specified.
		// A workaround fix is to use iWebBrowser2->Navigate for the
		// well-known targets, otherwise fallback to calling into 
		// javascript.
                long browserWindowHandle = getBrowserWindowHandle(clientSite);
                if (target != null && 
                    (target.equalsIgnoreCase("_blank") || target.equalsIgnoreCase("_parent") ||
                    target.equalsIgnoreCase("_self") || target.equalsIgnoreCase("_top")) && 
                    isBrowserWindowInForeground(browserWindowHandle)) {
                        iWebBrowser2Navigate(modUrl, target, clientSite);
		} else {
                    if (!isBrowserWindowInForeground(browserWindowHandle) &&
                        target!=null && target.equalsIgnoreCase("_blank") ) {
                        if (!showDocument0(url)) {
                            if (DEBUG) {
                                System.out.println("showDocument failed URL: " +
                                    url + " target: " + target);
                            }
                        }
                    } else {
                        Object [] args = new Object[2];
                        args[0] = modUrl;
                        args[1] = target;
                        try {
                            javaScriptCall(browserObj, "open", args);
                        } catch (JSException e1) {
                            // Try work-around for IE's failure to handle frame targets with hyphens
                            try {
                                BrowserSideObject res =
                                    (BrowserSideObject) javaScriptEval(browserObj, "top.frames['" + target + "']");
                                if (res != null) {
                                    javaScriptSetMember(res, "location", url);
                                    return;
                                }
                            } catch (JSException e2) {
                            }
                            // Fall back to old plug-in's behavior
                            args[1] = convertTarget(target);
                            javaScriptCall(browserObj, "open", args);
                        }
                    }
		}
	    }
        }
    }

    private static native void iWebBrowser2Navigate(String url, String target, long clientSite);
    private static native void iWebBrowser2PutStatusText(String status, long clientSite);
    private static native boolean isBrowserWindowInForeground(long browserWindow);
    private static native long getBrowserWindowHandle(long clientSite);
    private static native boolean showDocument0(String url);

    public void showStatus(String status) {
	if (status == null) {
	    return;
	}

	iWebBrowser2PutStatusText(status, clientSite);
    }

    public String getCookie(URL url) throws CookieUnavailableException {
        // Delegate back to the WIExplorerBrowserService for the time being
        return ServiceManager.
            getService().
            getCookieHandler().
            getCookieInfo(url);
    }

    public void setCookie(URL url, String value) throws CookieUnavailableException {
        // Delegate back to the WIExplorerBrowserService for the time being
        ServiceManager.
            getService().
            getCookieHandler().
            setCookieInfo(url, value);
    }

    public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
                                                    String scheme, String realm, URL requestURL,
                                                    boolean proxyAuthentication) {
        // Delegate back to the WIExplorerBrowserService for the time being
        return ServiceManager.
            getService().
            getBrowserAuthenticator().
            getAuthentication(protocol, host, port,
                              scheme, realm, requestURL,
                              proxyAuthentication);
    }

    public void setChildWindowHandle(long windowHandle) {
        // Unneeded on this platform
    }

    //----------------------------------------------------------------------
    // LiveConnect support routines
    //

    // This is the IDispatch* passed up during SetClientSite and is
    // the reference to the root scripting object of the document
    // (which we can invoke methods against). It is (at least in all
    // existing IE browsers) also an IHTMLWindow2 and we can use
    // automation to call its execScript() method. We can not
    // immediately wrap it in a BrowserSideObject because we don't
    // have a valid applet ID
    private long javaScriptWindow;

    // native pointer to the client site
    private long clientSite;

    ///////////////////////////////////////////////////////////////////////
    //                                                                   //
    // Implementation of the LiveConnect methods in the Plugin interface //
    //                                                                   //
    ///////////////////////////////////////////////////////////////////////

    // The reason of having an internal version of javaScriptGetWindow is to support liveconnect
    // functionality inside the browser process only. This is useful to support showDocument
    // method in AppletContext.
    private BrowserSideObject javaScriptGetWindowInternal(boolean registerWithLiveConnectSupport) {
        if (javaScriptWindow == 0)
            return null;

        return newBrowserSideObject(javaScriptWindow, registerWithLiveConnectSupport);
    }

    public BrowserSideObject javaScriptGetWindow() {
        return javaScriptGetWindowInternal(true);
    }

    public void javaScriptRetainObject(BrowserSideObject obj) {
        iUnknownAddRef(obj.getNativeObjectReference());
    }

    public void javaScriptReleaseObject(BrowserSideObject obj) {
        iUnknownRelease(obj.getNativeObjectReference());
    }

    public Object javaScriptCall(BrowserSideObject obj, String methodName, Object[] args) throws JSException {
        return javaScriptCallImpl(obj, methodName, args, DISPATCH_METHOD);
    }

    public Object javaScriptEval(BrowserSideObject obj, String code) throws JSException {
        BrowserSideObject target = obj;
        if (!iDispatchIsIHTMLWindow2(obj.getNativeObjectReference())) {
            target = javaScriptGetWindow();
        }

        // FIXME: would like to see if there's a cleaner way to do this invocation at some point
        Object args[] = new Object[1];

        // Escape all the '\'' and '\\'s in the string s
        StringBuffer buffer = new StringBuffer();

        for (int i=0; i < code.length(); i++) {
            char c = code.charAt(i);

            if (c == '\'' || c == '\"' || c == '\\')
                buffer.append('\\');

            buffer.append(c);
        }


        // Fixed #4324382   
        args[0]="evalIntermediateValueToReturn=0;evalIntermediateValueToReturn=eval('" + buffer.toString() + "');";

        try {
            javaScriptCall(target, "execScript", args);
        } catch (JSException ex0) {
            throw new JSException("Failure to evaluate " + code);
        }

        try {
            // Exception may be thrown if there is no return value.
            return javaScriptGetMember(target, "evalIntermediateValueToReturn");
        } catch (JSException ex1) {
            return null;
        }
    }

    public Object javaScriptGetMember(BrowserSideObject obj, String name) throws JSException {
        return javaScriptCallImpl(obj, name, null, DISPATCH_PROPERTYGET);
    }

    public void javaScriptSetMember(BrowserSideObject obj, String name, Object value) throws JSException {
        javaScriptCallImpl(obj, name, new Object[] { value }, DISPATCH_PROPERTYPUT);
    }

    public void javaScriptRemoveMember(BrowserSideObject obj, String name) throws JSException {
        iDispatchExDeleteMember(obj.getNativeObjectReference(), name);
    }

    public Object javaScriptGetSlot(BrowserSideObject obj, int index) throws JSException {
        // This differs from the old code but is more correct (i.e., actually works)
        return javaScriptGetMember(obj, Integer.toString(index));
    }

    public void javaScriptSetSlot(BrowserSideObject obj, int index, Object value) throws JSException {
        // This differs from the old code but is more correct (i.e., actually works)
        javaScriptSetMember(obj, Integer.toString(index), value);
    }

    public String javaScriptToString(BrowserSideObject obj) {
        // FIXME: this is different than the original code (no map
        // of all of the dispatch types to Strings -- just calling
        // toString() on the JavaScript object) but vastly simpler
        Object retVal = null;
        String strVal = null;

        try {
            retVal = javaScriptCall(obj, "toString", null);
        } catch (JSException e) {
            retVal = "[object JSObject]";
        }

        if (retVal != null)
            strVal = retVal.toString();

        return strVal;
    }

    private Object javaScriptCallImpl(BrowserSideObject obj, String methodName, Object args[], int invokeCode) throws JSException {
        long argArray = 0;
        try {
            if (args != null && args.length > 0) {
                argArray = allocateVariantArray(args.length);
                // Arguments are passed in the COM DISPPARAMS structure in reverse order (not well documented)
                for (int i = 0; i < args.length; i++) {
                    objectToVariantArrayElement(args[i], argArray, args.length - i - 1);
                }
            }

            Object res = iDispatchInvoke(obj.getNativeObjectReference(), methodName, invokeCode, argArray, (args == null ? 0 : args.length));
            return res;
        } finally {
            if (argArray != 0) {
                freeVariantArray(argArray, args.length);
            }
        }
    }

    ////////////////////////////////////////////
    //                                        //
    // Methods associated with COM invocation //
    //                                        //
    ////////////////////////////////////////////

    // Invocation codes for COM
    private static final int DISPATCH_METHOD = 0x1;
    private static final int DISPATCH_PROPERTYGET = 0x2;
    private static final int DISPATCH_PROPERTYPUT = 0x4;

    // Exposure of IUnknown::AddRef() and Release()
    private static native void iUnknownAddRef (long iUnknown);
    private static native void iUnknownRelease(long iUnknown);
    // Queries to see whether an IDispatch* is also an IHTMLWindow2
    // (so that we can call execScript against it)
    private static native boolean iDispatchIsIHTMLWindow2(long iDispatch);

    // Actually invokes a method against an IDispatch (corresponding to a view of a JavaScript object in the JScript engine).
    // The boolean "useIDispatchEx" indicates whether to try using the functionality of the IDispatchEx interface.
    // This provides the ability to dynamically create new properties in JSObject.setMember() and setSlot() via
    // the use of the GetDispID method with the fdexNameEnsure flag. We prefer to only try to use this functionality
    // if requested -- as a fallback -- so that the common case can not require the presence of the IDispatchEx
    // interface.
    // We strongly prefer to construct exceptions up in Java to keep all internationalization in one place.
    // Return codes:
    //   0  OK
    //  -1  No such method / property name (GetIDsOfNames failed)
    //  -2  Method invocation failed (res[0] is set to exception message)
    //  -3  Object does not implement the IDispatchEx interface
    private native int iDispatchInvoke0(long cAxControl, long iDispatch, String methodName, int flags, boolean useIDispatchEx,
                                        long variantArray, int numArgs, Object[] res);
    private Object iDispatchInvoke(long iDispatch, String methodName, int flags, long variantArray, int numArgs) throws JSException {
        // Always prepare box in case of exceptional return value
        Object[] box = new Object[1];
        int res = iDispatchInvoke0(cAxControl, iDispatch, methodName, flags, false, variantArray, numArgs, box);
        if (flags == DISPATCH_PROPERTYPUT && res == -1) {
            // Try again using the IDispatchEx functionality
            res = iDispatchInvoke0(cAxControl, iDispatch, methodName, flags, true, variantArray, numArgs, box);
        }
        if (res < 0) {
            switch (res) {
                case -1:
                case -3:
                    if ((flags & DISPATCH_METHOD) != 0) {
                        throw JSExceptions.noSuchMethod(methodName);
                    } else {
                        throw JSExceptions.noSuchProperty(methodName);
                    }
                case -2: {
                    String detailMessage = "";
                    if (box[0] != null)
                        detailMessage = (String) box[0];
                    else
                        detailMessage = "Unspecified error invoking method or accessing property \"" + methodName + "\"";
                    throw new JSException(detailMessage);
                }
                default:
                    throw new JSException("Unexpected error code " + res);
            }
        }
        return box[0];
    }
    // This method supports JSObject.removeMember via IDispatchEx::DeleteMemberByName().
    // We again prefer to construct all exceptions up in Java to ease internationalization.
    // Return codes:
    //   0  OK
    //  -1  Object was not an IDispatchEx
    //  -2  Member does not exist, or exists and can not be deleted
    private static native int iDispatchExDeleteMember0(long iDispatch, String memberName);
    private static void iDispatchExDeleteMember(long iDispatch, String memberName) throws JSException {
        int res = iDispatchExDeleteMember0(iDispatch, memberName);
        if (res < 0) {
            switch (res) {
                case -1:
                    throw new JSException("Object does not support removeMember");
                case -2:
                    throw JSExceptions.canNotRemoveMember(memberName);
                default:
                    throw new JSException("Unexpected error code " + res);
            }
        }
    }

    //----------------------------------------------------------------------
    // Routines supporting JavaScript -> Java calls
    //

    ///////////////////////////////////////////////////////////////////
    //                                                               //
    // Implementation of the LiveConnect methods from AbstractPlugin //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    protected boolean scriptingObjectArgumentListsAreReversed() {
        return true;
    }

    protected native long allocateVariantArray(int size);
    protected native void freeVariantArray(long array, int size);
    protected native void setVariantArrayElement(long variantArray, int index, boolean value);
    protected native void setVariantArrayElement(long variantArray, int index, byte value);
    protected native void setVariantArrayElement(long variantArray, int index, char value);
    protected native void setVariantArrayElement(long variantArray, int index, short value);
    protected native void setVariantArrayElement(long variantArray, int index, int value);
    protected native void setVariantArrayElement(long variantArray, int index, long value);
    protected native void setVariantArrayElement(long variantArray, int index, float value);
    protected native void setVariantArrayElement(long variantArray, int index, double value);
    protected native void setVariantArrayElement(long variantArray, int index, String value);
    protected native void setVariantArrayElementToScriptingObject(long variantArray, int index, long value);
    // Reinitializes the given element
    protected native void setVariantArrayElementToVoid(long variantArray, int index);

    //
    // This method helps convert VARIANTs to Java objects for method invocation results
    //
    protected Object variantArrayElementToObject(long variantArray, int index) {
        return variantArrayElementToObject0(cAxControl, variantArray, index);
    }

    private static native Object variantArrayElementToObject0(long cAxControl, long variantArray, int index);

    //
    // Methods for mapping RemoteJavaObject instances to IDispatch objects
    //
    private static Map/*<RemoteJavaObject, Long>*/ javaDispatchMap = new HashMap();
    // Need to preserve a reverse mapping as well for the case where the browser passes
    // us back a CJavaDispatch* we previously gave it
    private static Map/*<Long, RemoteJavaObject>*/ remoteJavaObjectMap = new HashMap();

    private static native long allocateIDispatch(long cAxControl,
                                                 RemoteJavaObject object,
                                                 boolean objectIsApplet);

    // This allocates a CJavaDispatch whose corresponding Java object is initialized lazily
    private static native long allocateIDispatchForJavaNameSpace(long cAxControl,
                                                                 String nameSpace);

    // This implements the mapping from a remote Java object to the IDispatch wrapper we have created for it on our side
    protected long lookupScriptingObject(RemoteJavaObject object, boolean objectIsApplet) {
        synchronized(javaDispatchMap) {
            Long val = (Long) javaDispatchMap.get(object);
            if (val != null)
                return val.longValue();
            long disp = allocateIDispatch(cAxControl, object, objectIsApplet);
            if (disp != 0) {
                val = new Long(disp);
                javaDispatchMap.put(object, val);
                remoteJavaObjectMap.put(val, object);
                return val.longValue();
            }
            return 0;
        }
    }

    // This implements the mapping from a CJavaDispatch* we handed back to the browser to the corresponding
    // RemoteJavaObject the other side gave us. Returns null if this IDispatch* isn't one we created.
    private RemoteJavaObject lookupRemoteJavaObject(long iDispatchPointer) {
        Long key = new Long(iDispatchPointer);
        synchronized (javaDispatchMap) {
            return (RemoteJavaObject) remoteJavaObjectMap.get(key);
        }
    }

    protected Object wrapOrUnwrapScriptingObject(long iDispatchPointer) {
        Object res = lookupRemoteJavaObject(iDispatchPointer);
        if (res != null)
            return res;
        return newBrowserSideObject(iDispatchPointer);
    }

    protected String identifierToString(long dispId) {
        synchronized(IExplorerPlugin.class) {
            // Special case for synthetic "toString" DISPID
            if (dispId == 0) {
                return "toString";
            }

            return (String) dispIdMap.get(new Integer((int) dispId));
        }
    }

    protected void fillInExceptionInfo(long exceptionInfo,
                                       Exception exc) {
        fillInExceptionInfo(exceptionInfo, exc.getMessage());
    }

    protected void fillInExceptionInfo(long exceptionInfo,
                                       String description) {
        if (exceptionInfo != 0) {
            fillInExceptionInfo0(exceptionInfo,
                                 (short) 1001,
                                 "Java",
                                 description);
        }
    }

    private static native void fillInExceptionInfo0(long exceptionInfo,
                                                    short errorCode,
                                                    String source,
                                                    String description);

    protected boolean doJavaObjectOp(RemoteJavaObject object,
                                     boolean objectIsApplet,
                                     int operationKind,
                                     String name,
                                     long nameIdentifier,
                                     long variantArgs,
                                     int  argCount,
                                     long variantResult,
                                     long exceptionInfo) {
        if (objectIsApplet && name != null && name.equals("unselectable")) {
            if (DEBUG) {
                System.out.println("IExplorerPlugin short-circuiting fetch of \"unselectable\" property");
            }
            // This is a synthetic property queried by Internet Explorer.
            // We would like to simply return "false" rather than make a
            // round-trip call to the target JVM which will result in a
            // LiveConnect exception being thrown. Among other things this
            // seems to reduce flickering of the applet upon being selected
            // for some reason.
            setVariantArrayElement(variantResult, 0, false);
            return true;
        }

        if (nameIdentifier == 0 && "toString".equals(name) && operationKind == JavaObjectOpMessage.GET_FIELD) {
            // Synthetic fetch of the "toString" property -- convert to method call
            operationKind = JavaObjectOpMessage.CALL_METHOD;
        }

        return super.doJavaObjectOp(object, objectIsApplet, operationKind, name, nameIdentifier,
                                    variantArgs, argCount, variantResult, exceptionInfo);
    }

    protected void releaseRemoteJavaObject(RemoteJavaObject object) {
        super.releaseRemoteJavaObject(object);
        synchronized(javaDispatchMap) {
            Long iDispatch = (Long) javaDispatchMap.remove(object);
            if (iDispatch != null) {
                remoteJavaObjectMap.remove(iDispatch);
            }
        }
    }

    // We need to maintain a stable mapping from COM's "dispatch IDs"
    // to the names of the methods and properties they describe. We
    // want to take the simplest possible approach not involving a
    // round-trip communication with the client JVM instance and not
    // involving building a full COM implementation. It's somewhat
    // unclear how to do this and also support passing returned Java
    // objects (exposed to the JavaScript engine by wrapping them in
    // IDispatchEx instances) between multiple Java plugin instances
    // on the same page without inducing a memory leak. Essentially we
    // need a static table mapping dispatch IDs to the names we
    // assigned to them, but would like to be able to release these
    // names at some point. We could consider something like having a
    // static WeakHashMap mapping interned Strings to their dispatch
    // IDs and per-plugin Sets holding on to the interned String
    // instances it has returned for queries to GetIDOfName to keep
    // the instances in the static WeakHashMap alive. However this
    // scheme gets complicated and it seems doesn't really work for
    // some scenarios where one plugin returns an object for which
    // GetIDOfName has been called, the object is passed into another
    // plugin instance for invocation, and the first plugin instance
    // goes away.
    //
    // For the time being, despite memory leak issues, we're going to
    // maintain a static table for these IDs.
    //
    // NOTE also that we only really support 2^31 - 1 unique names.
    // The moment the high bit is set it looks like a negative DISPID
    // and we filter these out at the native level to avoid having to
    // deal with them. See OleCtl.h and definitions of DISPID_AUTOSIZE
    // and similar.

    private static Map/*<Integer, String>*/ dispIdMap =
        new HashMap/*<Integer, String>*/();
    private static Map/*<String, Integer>*/ nameMap =
        new HashMap/*<String, Integer>*/();
    private static int nextDispId;

    // This method is called from the native code
    private int GetIDOfName(String name, RemoteJavaObject targetObject) {
        if (name == null) {
            return 0;
        }

        // Try to reduce memory usage by having a certain amount of
        // canonicalization of these objects
        name = name.intern();
        Integer id = null;
        synchronized(IExplorerPlugin.class) {
            id = (Integer) nameMap.get(name);
            if (id == null) {
                id = new Integer(++nextDispId);
                nameMap.put(name, id);
                dispIdMap.put(id, name);
            }
        }
	
	// Check if the target object has the field or the method
	// Return -1 if it has no. The caller will return DISP_E_UNKNOWNNAME
	if ( !javaObjectHasFieldOrMethod(targetObject, id.intValue())) {
	    return -1;
	}
	return id.intValue();
    }

    //----------------------------------------------------------------------
    // Support for blocking of the browser window by modal dialogs
    //

    public void waitForSignalWithModalBlocking() {
        if (DEBUG) {
            System.out.println("IExplorerPlugin entering waitForSignalWithModalBlocking for " + appletID);
        }
        boolean installedHooks = WindowsHelper.registerModalDialogHooks(this.hWndControlWindow, appletID.getID());
        runMessagePump();
        if (installedHooks) {
            WindowsHelper.unregisterModalDialogHooks(this.hWndControlWindow);
        }
        if (DEBUG) {
            System.out.println("IExplorerPlugin exiting waitForSignalWithModalBlocking for " + appletID);
        }
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private int getIntParameter(String param) {
        String val = (String) params.get(param);
        if (val == null) {
            return 0;
        }
        try {
            return Integer.parseInt(val);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private void maybeStartApplet() {
        if (!started) {
            if (hWndControlWindow != 0 &&
                activated &&
                gotInitialSize &&
                params != null) {
                if (DEBUG) {
                    System.out.println("  Attempting to start applet");
                }
                started = true;
                // Try to behave more like the old Java Plug-In for
                // initially hidden applets, for which we receive a
                // size of (0, 0)
                if (initialWidth == 0 && initialHeight == 0) {
                    int newInitialWidth  = getIntParameter("width");
                    int newInitialHeight = getIntParameter("height");
                    if (newInitialWidth > 0 && newInitialHeight > 0) {
                        initialWidth  = newInitialWidth;
                        initialHeight = newInitialHeight;
                    }
                }
                params.put("width", Integer.toString(initialWidth));
                params.put("height", Integer.toString(initialHeight));
                // set java_version if it hasn't been set due to not being
                // specified as part of the html parameters and javaVersion isn't null
                if ((params.get("java_version") == null) && (javaVersion != null)) {
                    params.put("java_version", javaVersion);
                }
                // Since we're not running on X11 or Mac OS X the last two arguments don't matter
                appletID = JVMManager.getManager().startApplet(params, this, hWndControlWindow, 0, false);
                appletStarted(appletID, getResultHandler());
                if (DEBUG) {
                    System.out.println("  Received applet ID " + appletID);
                }
            } else {
                if (DEBUG) {
                    System.out.println("  Skipped starting applet: hWndControlWindow = " + hWndControlWindow + ", params = " + params +
                                       ", activated = " + activated + ", gotInitialSize = " + gotInitialSize);
                }
            }
        }
    }

    private void stopApplet() {
	if (appletID == null) return;
	if (DEBUG) {
	    System.out.println("  Stopping applet ID " + appletID);
	}

	boolean runMessagePump = true;
	Object stopMark = (Object) appletStopMark.get();
	if (stopMark != null) {
	    // recursive stopApplet() detected, don't wait for ack
	    runMessagePump = false;
	} else {
	    appletStopMark.set(this);
	}

	JVMManager.getManager().sendStopApplet(appletID);
	try {
	    long startTime = System.currentTimeMillis();
	    boolean timedOut = false;
	    long endTime = startTime + STOP_ACK_DELAY;
	    while (!timedOut &&
		   !JVMManager.getManager().appletExited(appletID) &&
		   !JVMManager.getManager().receivedStopAcknowledgment(appletID)) {
		long curTime = System.currentTimeMillis();
		if (endTime - curTime > 0) {
		    if (runMessagePump) {
			runMessagePump(endTime - curTime);
		    } else {
			// reduce wait time to ~200 ms. We just want
			// to exit the recursion ASAP
			endTime =- STOP_ACK_DELAY_REDUCTION;
			Thread.yield();
		    }
		}
		if (!JVMManager.getManager().receivedStopAcknowledgment(appletID)) {
		    curTime = System.currentTimeMillis();
		    timedOut = (curTime >= endTime);
		}
	    }
	} finally {
	    try {
		JVMManager.getManager().recycleAppletID(appletID);
	    } finally {
		appletStopped();
		appletID = null;
		// Under some circumstances (like the Information Bar popping up and blocking us) IE
		// reuses the same plugin instance to display the applet. Resetting the "started" flag
		// supports this.
		started = false;

		// reset the appletStopMark if this stopApplet() call sets it
		appletStopMark.set(stopMark);
	    }
	}
    }

    private void runMessagePump() {
        runMessagePump(-1);
    }

    private void runMessagePump(long timeout) {
        WindowsHelper.runMessagePump(mainThreadEvent, timeout,
                                     ModalitySupport.appletShouldBlockBrowser(appletID));
    }

    private ResultHandler getResultHandler() {
        return new ResultHandler() {
                public void waitForSignal() {
                    runMessagePump();
                }

                public void waitForSignal(long millis) {
                    runMessagePump(millis);
                }
            };
    }

    // Helper routine for Java namespace support
    private void defineNameSpaceVariable(BrowserSideObject jsObject,
                                         String variableName,
                                         String nameSpaceName) {
        Object tmp = null;
        try {
            // See whether we've already defined these variables in this namespace (unlikely);
            // might happen if we have two applets on the same web page
            tmp = javaScriptGetMember(jsObject, variableName);
        } catch (JSException e) {
        }
        if (tmp == null) {
            long iDispatch = allocateIDispatchForJavaNameSpace(cAxControl, nameSpaceName);
            // Don't need to register this with the LiveConnectSupport; it's only temporary
            BrowserSideObject obj = newBrowserSideObject(iDispatch, false);
            try {
                javaScriptSetMember(jsObject, variableName, obj);
            } catch (JSException e) {
                if (DEBUG) {
                    System.out.println("IExplorerPlugin.defineNameSpaceVariable: error setting up namespace variable \"" +
                                       variableName + "\"");
                    e.printStackTrace();
                }
            }
        }
    }

    private static String escapeURL(String url) {
        //Escape the characters '\'', '\"' and '\\'in the JavaScript URL
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < url.length(); i++) {
            char c = url.charAt(i);
            if (c == '\'' || c == '\"' || c == '\\') {
                buf.append("\\");
            }
            buf.append(c);
        }
        return buf.toString();
    }
    
    private static String toHexString(long val) {
        return "0x" + Long.toHexString(val);
    }
}
