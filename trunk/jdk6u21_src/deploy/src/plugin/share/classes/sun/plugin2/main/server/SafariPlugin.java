/*
 * @(#)SafariPlugin.java	1.4 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.net.PasswordAuthentication;
import java.net.URL;
import java.security.SecureRandom;
import java.util.*;
import netscape.javascript.*;

import com.sun.deploy.net.cookie.CookieUnavailableException;
import com.sun.deploy.net.proxy.BrowserProxyInfo;
import com.sun.deploy.net.proxy.ProxyConfigException;
import com.sun.deploy.net.proxy.ProxyInfo;
import com.sun.deploy.net.proxy.ProxyType;
import com.sun.deploy.net.proxy.ProxyUnavailableException;
import com.sun.deploy.services.ServiceManager;

import sun.plugin2.ipc.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.util.*;

public class SafariPlugin extends AbstractPlugin {
    // Pointer to underlying SafariJavaPlugin2 instance, which is also an NSView
    private long safariPluginInstance;

    // Pointer to our WebPlugInContainer
    private long webPlugInContainer;

    // Pointer to the JSObjectRef corresponding to the window containing us
    private long windowScriptObject;

    // The base URL of the document containing the applet
    private String baseURL;

    // The applet parameters, provided by the underlying browser plugin
    private Map/*<String,String>*/ params = new HashMap/*<String,String>*/();

    private AppletID appletID;
    private boolean gotInitialSize;
    private boolean destroyed;

    // Event which is signalled to wake up the main thread
    private Event mainThreadEvent = new InProcEvent();

    private SafariResultHandler handler = new SafariResultHandler();

    // Pull in the native code for this class
    // I don't know how to get this into the core Java Plug-In bundle (or if there is
    // any way to reasonably do so) -- for the time being, keep it in a separate jnilib
    static {
        sun.plugin2.util.NativeLibLoader.load(new String[] {"deploy", "jp2safari"});

        try {
            // Install browser services such as proxy, cookie and etc.
            // FIXME: the implementation of all of this needs to be filled out
            ServiceManager.setService(new SafariBrowserService());
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        JVMManager.setBrowserType(BrowserType.SAFARI_MACOSX);
    }

    public SafariPlugin(long safariPluginInstance,
                        long webPlugInContainer,
                        long windowScriptObject) {
        this.safariPluginInstance = safariPluginInstance;
        nsObjectRetain(webPlugInContainer);
        this.webPlugInContainer = webPlugInContainer;
        jsValueProtect(webPlugInContainer, windowScriptObject);
        this.windowScriptObject = windowScriptObject;
    }

    // Called by the native code
    public void addParameters(String[] keys, String[] values) {
        for (int i = 0; i < keys.length; i++) {
            addParameter(keys[i], values[i]);
        }
    }
  
    private void addParameter(String key, String value) {
        // Some of the parameters coming up from native code might be
        // null; filter these out for cleanliness
        if (key != null) {
            key = key.trim().toLowerCase(java.util.Locale.ENGLISH);
            params.put(key, value);
        }
    }
    
    // Called by the native code
    public void setBaseURL(String baseURL) {
        if (DEBUG) {
            System.out.println("SafariPlugin.setBaseURL(" + baseURL + ")");
        }
        this.baseURL = baseURL;
    }

    // Called by the native code
    public void setFrame(float x, float y, float w, float h) {
        // setFrame is the notification we really need to pay attention to
        setSizeImpl(x, y, w, h);
        if (DEBUG && VERBOSE) {
            System.out.println("SafariPlugin.setFrame(" + x + ", " + y + ", " + w + ", " + h + ")");
        }
    }

    // Called by the native code
    public void setBounds(float x, float y, float w, float h) {
        // This notification doesn't currently seem to be sent to us
        setSizeImpl(x, y, w, h);
        if (DEBUG && VERBOSE) {
            System.out.println("SafariPlugin.setBounds(" + x + ", " + y + ", " + w + ", " + h + ")");
        }
    }

    private void setSizeImpl(float x, float y, float w, float h) {
        if (appletID != null) {
            JVMManager.getManager().setAppletSize(appletID, (int) w, (int) h);
        } else {
            params.put("width", Integer.toString((int) w));
            params.put("height", Integer.toString((int) h));
            gotInitialSize = true;
            maybeStartApplet();
        }
    }

    // Called by the native code
    public void webPlugInInitialize() {
        if (DEBUG) {
            System.out.println("SafariPlugin.webPlugInInitialize()");
        }

        synchronized (SafariPlugin.class) {
            if (!initialized) {
                initialized = true;
                webPlugInInitialize0();
            }
        }

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
                    System.out.println("SafariPlugin: successfully defined namespace variables");
                }
            } catch (JSException e) {
                e.printStackTrace();
            }
        }

        maybeStartApplet();
    }


    private static boolean initialized = false;
    private static native void webPlugInInitialize0();

    // Called by the native code
    public void webPlugInStart() {
        if (DEBUG) {
            System.out.println("SafariPlugin.webPlugInStart");
        }
    }

    // Called by the native code
    public void webPlugInStop() {
        if (DEBUG) {
            System.out.println("SafariPlugin.webPlugInStop");
        }
    }

    // The amount of time we wait for an acknowledgment of a request
    // to stop an applet, in milliseconds
    private static final long STOP_ACK_DELAY = 1100; // ms

    // Called by the native code
    public void webPlugInDestroy() {
        if (DEBUG) {
            System.out.println("SafariPlugin.webPlugInDestroy");
        }

        // First unlink the shared window
        if (sharedWindow != null) {
            SharedWindowHost.unlinkSharedWindowFrame(parentWindowHandle, sharedWindow);
            sharedWindow = null;
        }

        if (appletID != null) {
            if (DEBUG) {
                System.out.println("  Stopping applet ID " + appletID);
            }
            JVMManager.getManager().sendStopApplet(appletID);
            try {
                // Should consider running a nested message pump of some sort here
                mainThreadEvent.waitForSignal(STOP_ACK_DELAY);
            } finally {
                JVMManager.getManager().recycleAppletID(appletID);
                appletStopped();
                appletID = null;
            }
        }

        if (windowScriptObject != 0 && webPlugInContainer != 0) {
            jsValueUnprotect(webPlugInContainer, windowScriptObject);
            windowScriptObject = 0;
        }
        if (webPlugInContainer != 0) {
            nsObjectRelease(webPlugInContainer);
            webPlugInContainer = 0;
        }
        destroyed = true;
    }

    // Called by the native code
    public void webPlugInSetIsSelected(boolean selected) {
        if (DEBUG) {
            System.out.println("SafariPlugin.webPlugInSetIsSelected(" + selected + ")");
        }
    }

    //----------------------------------------------------------------------
    // A little state machine in case the order of initialization
    // changes in the future.
    //
    // Currently the order of operations appears to be:
    //   - webPlugInInitialize
    //   - webPlugInStart
    //   - setFrame
    //
    // We want to wait until we receive the first frame size from the
    // browser before starting the applet.

    private void maybeStartApplet() {
        if (appletID == null && gotInitialSize) {
            appletID = JVMManager.getManager().startApplet(params, this,
                                                           parentWindowHandle,
                                                           getConnectionHandle(),
                                                           false);
            appletStarted(appletID, handler);
        }
    }

    //----------------------------------------------------------------------
    // Parent / child window linking
    //

    private static long connectionHandle;
    private static long getConnectionHandle() {
        if (connectionHandle == 0) {
            connectionHandle = SharedWindowHost.getConnectionHandle();
        }
        return connectionHandle;
    }

    // The native window handle into which we put our content
    private long parentWindowHandle;
    // Called by the native code
    private void setWindowHandle(long windowHandle) {
        parentWindowHandle = windowHandle;
    }

    private long childWindowHandle;
    private SharedWindow sharedWindow;
    // This is declared in the Plugin interface
    public void setChildWindowHandle(long windowHandle) {
        this.childWindowHandle = windowHandle;
        if (DEBUG) {
            System.out.println("SafariPlugin: linking 0x" + Long.toHexString(parentWindowHandle) +
                               " to 0x" + Long.toHexString(childWindowHandle));
        }
        sharedWindow = SharedWindowHost.linkSharedWindowTo(parentWindowHandle, childWindowHandle);
        sharedWindow.setVisible(false);
        // Update the location and clip the first time
        // FIXME: this is a hack -- need a reliable way of knowing
        // when the window is actually on screen and can be reliably
        // controlled by the SharedWindow mechanism -- right now the
        // window display is asynchronous
        new Thread(new Runnable() {
                public void run() {
                    for (int i = 0; i < 5; i++) {
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                        }
                        invokeLater(new Runnable() {
                                public void run() {
                                    updateLocationAndClip(safariPluginInstance);
                                    sharedWindow.setVisible(true);
                                }
                            });
                    }
                }
            }).start();
    }

    private native void updateLocationAndClip(long safariPluginInstance);

    // Called by the native code
    private void setLocationAndClip(double x, double y,
                                    double clipX, double clipY, double clipWidth, double clipHeight) {
        if (sharedWindow != null) {
            if (DEBUG) {
                System.out.println("SafariPlugin: Setting location: " + x + ", " + y);
                System.out.println("            clip: " + clipX + ", " + clipY + " -- " + clipWidth + ", " + clipHeight);
            }
            sharedWindow.setLocation(x, y);
            sharedWindow.setClip(clipX, clipY, clipWidth, clipHeight);
        }
    }

    //----------------------------------------------------------------------
    // Helpers for dealing with NSObjects
    //

    private static native void nsObjectRetain(long nsObject);
    private static native void nsObjectRelease(long nsObject);

    //----------------------------------------------------------------------
    // Helpers for dealing with JSValueRefs
    //

    private static native void jsValueProtect(long webPlugInContainer, long jsValueRef);
    private static native void jsValueUnprotect(long webPlugInContainer, long jsValueRef);

    //----------------------------------------------------------------------
    // Implementation of the Plugin interface
    //

    // We know that Safari currently has one main thread for all
    // windows, and that this is unlikely to change, so we eliminate
    // the complexity in the MozillaPlugin and use one static runnable
    // queue
    private static List/*<Runnable>*/ runnableQueue =
        Collections.synchronizedList(new LinkedList());

    // We also need to be absolutely sure that we do not execute
    // Runnables associated with previously-destroyed plugin
    // instances.
    static class RunnableWrapper implements Runnable {
        private Runnable runnable;
        private SafariPlugin plugin;

        public RunnableWrapper(Runnable runnable, SafariPlugin plugin) {
            this.runnable = runnable;
            this.plugin = plugin;
        }

        public void run() {
            runnable.run();
        }

        public SafariPlugin getPlugin() {
            return plugin;
        }
    }

    private boolean isDestroyed() {
        return destroyed;
    }

    public void invokeLater(Runnable runnable) {
        // Add the runnable to the queue
        runnableQueue.add(new RunnableWrapper(runnable, this));
        // Kick the browser's main thread
        invokeLater0(safariPluginInstance);
    }

    private native void invokeLater0(long safariPluginInstance);

    // Called from native code
    private static void drainRunnableQueue() {
        while (!runnableQueue.isEmpty()) {
            RunnableWrapper r = (RunnableWrapper) runnableQueue.remove(0);
            if (!r.getPlugin().isDestroyed()) {
                r.run();
            }
        }
    }

    public void notifyMainThread() {
        mainThreadEvent.signal();
    }

    public String getDocumentBase() {
        return baseURL;
    }

    public void showDocument(String url, String target) {
        if (webPlugInContainer != 0) {
            showDocument0(webPlugInContainer, url, target);
        }
    }

    private native void showDocument0(long webPlugInContainer, String url, String target);

    public void showStatus(String status) {
        if (webPlugInContainer != 0) {
            showStatus0(webPlugInContainer, status);
        }
    }

    private native void showStatus0(long webPlugInContainer, String status);

    public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
                                                    String scheme, String realm, URL requestURL,
                                                    boolean proxyAuthentication) { throw new RuntimeException("unimplemented"); }

    public void waitForSignalWithModalBlocking() {
        if (DEBUG) {
            System.out.println("SafariPlugin entering waitForSignalWithModalBlocking for " + appletID);
        }
        handler.waitForSignalWithModalBlocking();
        if (DEBUG) {
            System.out.println("SafariPlugin exiting waitForSignalWithModalBlocking for " + appletID);
        }
    }

    class SafariResultHandler extends ResultHandler {
        public void waitForSignal() {
            waitForSignal(0);
        }

        public void waitForSignal(long millis) {
            mainThreadEvent.waitForSignal(millis);
            drainRunnableQueue();
        }

        public void waitForSignalWithModalBlocking() {
            waitForSignal();
        }
    }

    //----------------------------------------------------------------------
    // Java-to-JavaScript and JavaScript-to-Java calls
    //

    private BrowserSideObject javaScriptGetWindowInternal(boolean registerWithLiveConnectSupport) {
        if (windowScriptObject == 0)
            return null;

        return newBrowserSideObject(windowScriptObject, registerWithLiveConnectSupport);
    }

    public BrowserSideObject javaScriptGetWindow() {
        return javaScriptGetWindowInternal(true);
    }

    public void javaScriptRetainObject(BrowserSideObject obj) {
        jsValueProtect(webPlugInContainer, obj.getNativeObjectReference());
    }

    public void javaScriptReleaseObject(BrowserSideObject obj) {
        jsValueUnprotect(webPlugInContainer, obj.getNativeObjectReference());
    }

    public Object javaScriptCall(BrowserSideObject obj, String methodName, Object[] args) throws JSException {
        long argArray = 0;
        int  argArrayLen = 0;
        long resArray = 0;

        try {
            if (args != null && args.length > 0) {
                argArrayLen = args.length;
                argArray = allocateVariantArray(argArrayLen);
                for (int i = 0; i < argArrayLen; i++) {
                    objectToVariantArrayElement(args[i], argArray, i);
                }
            }
            resArray = allocateVariantArray(1);
            Object[] exceptionString = new Object[1];

            boolean res = javaScriptCall0(webPlugInContainer, obj.getNativeObjectReference(), methodName,
                                          argArray, argArrayLen, resArray, exceptionString);
            if (!res) {
                String detail = "JavaScript error while calling \"" + methodName + "\"";
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }

            return variantArrayElementToObject(resArray, 0);
        } finally {
            if (argArray != 0) {
                freeVariantArray(argArray, argArrayLen);
            }

            if (resArray != 0) {
                freeVariantArray(resArray, 1);
            }
        }
    }

    private native boolean javaScriptCall0(long webPlugInContainer, long receiver, String name,
                                           long argArray, int argArrayLen, long resArray, Object[] exceptionString);

    public Object javaScriptEval(BrowserSideObject obj, String code) throws JSException {
        long resArray = allocateVariantArray(1);
        try {
            Object[] exceptionString = new Object[1];
            if (!javaScriptEval0(webPlugInContainer, obj.getNativeObjectReference(), code, resArray, exceptionString)) {
                String detail = "JavaScript error evaluating code \"" + code + "\"";
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }
            return variantArrayElementToObject(resArray, 0);
        } finally {
            freeVariantArray(resArray, 1);
        }
    }

    private native boolean javaScriptEval0(long webPlugInContainer, long receiver, String code, long resArray, Object[] exceptionString);

    public Object javaScriptGetMember(BrowserSideObject obj, String name) throws JSException {
        long resArray = allocateVariantArray(1);
        try {
            Object[] exceptionString = new Object[1];
            if (!javaScriptGetMember0(webPlugInContainer, obj.getNativeObjectReference(), name, resArray, exceptionString)) {
                String detail = "JavaScript error while getting property \"" + name + "\"";
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }
            return variantArrayElementToObject(resArray, 0);
        } finally {
            freeVariantArray(resArray, 1);
        }
    }

    private native boolean javaScriptGetMember0(long webPlugInContainer, long receiver, String name, long resArray, Object[] exceptionString);

    public void javaScriptSetMember(BrowserSideObject obj, String name, Object value) throws JSException {
        long argArray = allocateVariantArray(1);
        try {
            objectToVariantArrayElement(value, argArray, 0);
            Object[] exceptionString = new Object[1];
            if (!javaScriptSetMember0(webPlugInContainer, obj.getNativeObjectReference(), name, argArray, exceptionString)) {
                String detail = "JavaScript error while setting property \"" + name + "\"";
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }
        } finally {
            freeVariantArray(argArray, 1);
        }
    }

    private native boolean javaScriptSetMember0(long webPlugInContainer, long receiver, String name, long argArray, Object[] exceptionString);

    public void javaScriptRemoveMember(BrowserSideObject obj, String name) throws JSException {
        Object[] exceptionString = new Object[1];
        if (!javaScriptRemoveMember0(webPlugInContainer, obj.getNativeObjectReference(), name, exceptionString)) {
            String detail = "JavaScript error while removing property \"" + name + "\"";
            if (exceptionString[0] != null) {
                detail = detail + ": " + exceptionString[0];
            }
            throw new JSException(detail);
        }
    }

    private native boolean javaScriptRemoveMember0(long webPlugInContainer, long receiver, String name, Object[] exceptionString);

    public Object javaScriptGetSlot(BrowserSideObject obj, int index) throws JSException {
        long resArray = allocateVariantArray(1);
        try {
            Object[] exceptionString = new Object[1];
            if (!javaScriptGetSlot0(webPlugInContainer, obj.getNativeObjectReference(), index, resArray, exceptionString)) {
                String detail = "JavaScript error while getting index " + index;
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }
            return variantArrayElementToObject(resArray, 0);
        } finally {
            freeVariantArray(resArray, 1);
        }
    }

    private native boolean javaScriptGetSlot0(long webPlugInContainer, long receiver, int index, long resArray, Object[] exceptionString);

    public void javaScriptSetSlot(BrowserSideObject obj, int index, Object value) throws JSException {
        long argArray = allocateVariantArray(1);
        try {
            Object[] exceptionString = new Object[1];
            objectToVariantArrayElement(value, argArray, 0);
            if (!javaScriptSetSlot0(webPlugInContainer, obj.getNativeObjectReference(), index, argArray, exceptionString)) {
                String detail = "JavaScript error while setting index " + index;
                if (exceptionString[0] != null) {
                    detail = detail + ": " + exceptionString[0];
                }
                throw new JSException(detail);
            }
        } finally {
            freeVariantArray(argArray, 1);
        }
    }

    private native boolean javaScriptSetSlot0(long webPlugInContainer, long receiver, int index, long argArray, Object[] exceptionString);

    public String javaScriptToString(BrowserSideObject obj) throws JSException {
        Object[] exceptionString = new Object[1];
        String res = javaScriptToString0(webPlugInContainer, obj.getNativeObjectReference(), exceptionString);
        if (res == null) {
            String detail = "JavaScript error converting object to string";
            if (exceptionString[0] != null) {
                detail = detail + ": " + exceptionString[0];
            }
            throw new JSException(detail);
        }
        return res;
    }

    private native String javaScriptToString0(long webPlugInContainer, long receiver, Object[] exceptionString);

    protected boolean scriptingObjectArgumentListsAreReversed() {
        return false;
    }

    protected        long allocateVariantArray(int size) {
        return allocateVariantArray0(webPlugInContainer, size);
    }
    protected native long allocateVariantArray0(long webPlugInContainer, int size);
    protected native void freeVariantArray(long array, int size);
    protected        void setVariantArrayElement(long variantArray, int index, boolean value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, boolean value);
    protected        void setVariantArrayElement(long variantArray, int index, byte value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, byte value);
    protected        void setVariantArrayElement(long variantArray, int index, char value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, char value);
    protected        void setVariantArrayElement(long variantArray, int index, short value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, short value);
    protected        void setVariantArrayElement(long variantArray, int index, int value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, int value);
    protected        void setVariantArrayElement(long variantArray, int index, long value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, long value);
    protected        void setVariantArrayElement(long variantArray, int index, float value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, float value);
    protected        void setVariantArrayElement(long variantArray, int index, double value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, double value);
    protected        void setVariantArrayElement(long variantArray, int index, String value) {
        setVariantArrayElement0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElement0(long webPlugInContainer, long variantArray, int index, String value);
    protected        void setVariantArrayElementToScriptingObject(long variantArray, int index, long value) {
        setVariantArrayElementToScriptingObject0(webPlugInContainer, variantArray, index, value);
    }
    protected native void setVariantArrayElementToScriptingObject0(long webPlugInContainer, long variantArray, int index, long value);
    protected        void setVariantArrayElementToVoid(long variantArray, int index) {
        setVariantArrayElementToVoid0(webPlugInContainer, variantArray, index);
    }
    protected native void setVariantArrayElementToVoid0(long webPlugInContainer, long variantArray, int index);
    protected        Object variantArrayElementToObject(long variantArray, int index) {
        return variantArrayElementToObject0(webPlugInContainer, variantArray, index);
    }
    protected native Object variantArrayElementToObject0(long webPlugInContainer, long variantArray, int index);

    //----------------------------------------------------------------------
    // The following methods are strictly for JavaScript-to-Java calls
    //

    // The AbstractPlugin's infrastructure is built around identifiers
    // rather than Strings, so we do the mapping up in Java
    //
    // See IExplorerPlugin for a discussion of the complexities of
    // this mapping and why we use a static map with strong references
    // despite potential memory leak issues
    private static Map/*<Integer, String>*/ idToNameMap =
        new HashMap/*<Integer, String>*/();
    private static Map/*<String, Integer>*/ nameToIdMap =
        new HashMap/*<String, Integer>*/();
    private static int nextId;

    // Called by native code
    // NOTE that we reserve the 0 identifier for the toString operation
    private long stringToIdentifier(String name) {
        synchronized (SafariPlugin.class) {
            Integer res = (Integer) nameToIdMap.get(name);
            if (res == null) {
                res = new Integer(++nextId);
                nameToIdMap.put(name, res);
                idToNameMap.put(res, name);
            }
            return res.longValue();
        }
    }

    private long slotToIdentifier(int slot) {
        return stringToIdentifier(Integer.toString(slot));
    }

    protected String identifierToString(long id) {
        if (id == 0) {
            // Special case for toString operations
            return "toString";
        }

        synchronized(SafariPlugin.class) {
            String res = (String) idToNameMap.get(new Integer((int) id));
            return res;
        }
    }

    // Maps RemoteJavaObject instances to Objective-C JavaObject instances
    private static Map/*<RemoteJavaObject, Long>*/ javaObjectMap = new HashMap();

    private native long allocateJavaObject(long webPlugInContainer, RemoteJavaObject object);
    private native long allocateJavaObjectForNameSpace(long webPlugInContainer, String nameSpace);

    protected long lookupScriptingObject(RemoteJavaObject object,
                                         boolean objectIsApplet)  {
        synchronized(javaObjectMap) {
            Long val = (Long) javaObjectMap.get(object);
            if (val != null)
                return val.longValue();
            long javaObject = allocateJavaObject(webPlugInContainer, object);
            if (javaObject != 0) {
                val = new Long(javaObject);
                javaObjectMap.put(object, val);
                return val.longValue();
            }
            return 0;
        }
    }

    protected Object wrapOrUnwrapScriptingObject(long scriptingObject)  {
        // We make the determination of whether JavaObjects are ours or
        // not down in our native code; see variantArrayElementToObject()
        return newBrowserSideObject(scriptingObject);
    }

    protected void fillInExceptionInfo(long exceptionInfo, String message) {
        if (exceptionInfo == 0) {
            if (DEBUG) {
                System.out.println("SafariPlugin: JavaScript error: " + message);
            }
        } else {
            fillInExceptionInfo0(webPlugInContainer, exceptionInfo, message);
        }
    }

    protected void fillInExceptionInfo(long exceptionInfo, Exception exc) {
        if (exceptionInfo == 0) {
            if (DEBUG) {
                exc.printStackTrace();
            }
        } else {
            fillInExceptionInfo(exceptionInfo, exc.getMessage());
        }
    }

    protected native void fillInExceptionInfo0(long webPlugInContainer, long exceptionInfo, String message);

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
            long jsobj = allocateJavaObjectForNameSpace(webPlugInContainer, nameSpaceName);
            // Don't need to register this with the LiveConnectSupport; it's only temporary
            BrowserSideObject obj = newBrowserSideObject(jsobj, false);
            try {
                javaScriptSetMember(jsObject, variableName, obj);
            } catch (JSException e) {
                if (DEBUG) {
                    System.out.println("SafariPlugin.defineNameSpaceVariable: error setting up namespace variable \"" +
                                       variableName + "\"");
                    e.printStackTrace();
                }
            }
        }
    }

    //----------------------------------------------------------------------
    // No-op implementations of some of the services we need to support
    //
    // FIXME: need to provide real implementations of these

    static class SafariCookieHandler implements com.sun.deploy.net.cookie.CookieHandler {
        private Map/*<URL, String>*/ cookieMap = new HashMap();
        
        public String getCookieInfo(URL url) throws CookieUnavailableException {
            return (String) cookieMap.get(url);
        }

        public void setCookieInfo(URL url, String value) throws CookieUnavailableException {
            cookieMap.put(url, value);
        }
    }

    static class SafariProxyConfig implements com.sun.deploy.net.proxy.BrowserProxyConfig {
        public BrowserProxyInfo getBrowserProxyInfo() {
            // The default parameters should be sufficient to indicate
            // that we are using a direct internet connection
            return new BrowserProxyInfo();
        }

        public void getSystemProxy(BrowserProxyInfo bpi) {
        }
    }

    static class SafariProxyHandler implements com.sun.deploy.net.proxy.ProxyHandler {
        public boolean isSupported(int proxyType) {
            return (proxyType == ProxyType.NONE);
        }

        public boolean isProxyCacheSupported() {
            return true;
        }

        public void init(BrowserProxyInfo info) throws ProxyConfigException {
        }

        public ProxyInfo[] getProxyInfo(URL u) throws ProxyUnavailableException {
            return new ProxyInfo[] { new ProxyInfo(null) };
        }
    }

    static class SafariBrowserService implements sun.plugin.services.BrowserService {
        public com.sun.deploy.net.cookie.CookieHandler getCookieHandler() {
            return new SafariCookieHandler();
        }

        public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig() {
            return new SafariProxyConfig();
        }

        public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler() {
            return new SafariProxyHandler();
        }

        public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
            return null;
        }

        public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() {
            return null;
        }

        public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore() {
            return null;
        }

        public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore() {
            return null;
        }

        public com.sun.deploy.security.CertStore getBrowserTrustedCertStore() {
            return null;
        }

        public java.security.KeyStore getBrowserClientAuthKeyStore() {
            return null;
        }

        public Object getAppletContext() {
            return null;
        }

        public Object getBeansContext() {
            return null;
        }

        public SecureRandom getSecureRandom() {
            return new SecureRandom();
        }

        public boolean isIExplorer() {
            return false;
        }

        public boolean isNetscape() {
            return false;
        }

        public float getBrowserVersion() {
            return 1.0f;
        }

        public boolean isConsoleIconifiedOnClose() {
            return false;
        }

        public boolean installBrowserEventListener() {
            throw new RuntimeException("Unimplemented");
        }

        public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
            return null;
        }

        public com.sun.deploy.security.CredentialManager getCredentialManager() {
            return null;
        }

        public String mapBrowserElement(String rawName) {
            throw new RuntimeException("No longer used");
        }

        public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler() {
            return null;
        }
    }
}
