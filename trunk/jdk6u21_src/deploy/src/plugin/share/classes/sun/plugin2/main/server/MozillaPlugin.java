/*
 * @(#)MozillaPlugin.java	1.35 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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
import sun.plugin2.util.*;

/** This class corresponds to the underlying MozPluginInstance in the
    Mozilla-specific native code, which calls up into this class to do
    most of its work. MozPluginInstance.cpp knows about this class's
    name and its methods. <P>

    NOTE that the native methods for this class are pulled in from the
    npjp2.dll which is already loaded by the Mozilla / Firefox process
    by the time we get into this code. We use the NativeLibLoader to
    System.load() this library to make the native methods visible to
    the boot class loader, so we don't have to use RegisterNatives to
    hook up the native methods (which is both high-maintenance and
    error-prone). Using System.load() instead of RegisterNatives also
    improves the ability to test this class in isolation of the web
    browser.
*/

public class MozillaPlugin extends AbstractPlugin {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null); 

    // Note that we use the XEmbed protocol on X11 platforms by default

    // Note: due to bootstrapping issues (we don't currently initialize the JVM
    // as early as would be needed), the native code queries the same environment
    // variable without calling up to Java; see MozPluginExports.cpp
    private static final boolean USE_XEMBED = (SystemUtil.getenv("JPI_PLUGIN2_NO_XEMBED") == null); 

    // A way to disable the workarounds for Mozilla bug 406251
    private static final boolean DISABLE_406251_WORKAROUND =
        (SystemUtil.getenv("JPI_PLUGIN2_DISABLE_406251_WORKAROUND") != null);

    private static final boolean isMacOSX =
        System.getProperty("os.name").toLowerCase().startsWith("mac os x");

    // The applet parameters, provided by the underlying browser plugin
    private Map/*<String,String>*/ params = new HashMap/*<String,String>*/();

    // The underlying MozPluginInstance*
    private long mozPluginInstance = 0;
    // The opaque identifier on the browser side corresponding to this plugin instance
    private long npp               = 0;

    private String docbase = null;

    private String mimeType = null;

    private String requestedVersion = null;

    private final String _versionPattern = ";version=";

    private final String validChars = "0123456789._";

    private long hWndControlWindow = 0;
  
    private boolean started = false;
    private AppletID appletID;

    // Keep track of whether NPP_Destroy has already been called, and
    // forbid operations on NPObjects the browser has exported to us
    private boolean destroyed;

    // Abstraction of platform-specific portions of this code
    private PlatformDependentHandler handler;

    // We assume that the MozillaPlugin is instantiated on the "plugin main thread"
    private Thread pluginMainThread;

    static {
        if (isMacOSX) {
            // Load the bundle directly as the native code for this class
            sun.plugin2.util.NativeLibLoader.load(new String[] {"deploy", "JavaPlugIn2"});
        } else {
            sun.plugin2.util.NativeLibLoader.load(new String[] {"deploy", "npjp2"});
        }
        // Install browser services such as proxy, cookie and etc.
        //
        // FIXME: it is essential that we work with Mozilla to remove
        // the remaining XPCOM dependencies in this native code as
        // soon as possible; non-Firefox browsers that use this code
        // will not be able to perform the associated operations. It
        // is necessary to add entry points to the NPAPI to get and
        // set cookies, query proxy information, and fetch browser
        // authentication information.
        int osType = SystemUtil.getOSType();
        if (osType == SystemUtil.WINDOWS || osType == SystemUtil.UNIX  || osType == SystemUtil.MACOSX) {
            ServiceManager.setService(new MozillaBrowserService());
        } else {
            throw new RuntimeException("Must port BrowserService portion of MozillaPlugin to your platform");
        }
        JVMManager.setBrowserType(BrowserType.MOZILLA);
        initServiceManager();
    }

    public MozillaPlugin(long mozPluginInstance, long npp, String docbase, String mimeType) {
        if (DEBUG) {
            System.out.println("MozillaPlugin.MozillaPlugin");
        }
        this.mozPluginInstance = mozPluginInstance;
        this.npp               = npp;
        this.docbase           = docbase;
        this.mimeType          = mimeType;
        this.requestedVersion  = getRequestedVersion(mimeType);
        handler                = getPlatformDependentHandler();
        pluginMainThread       = Thread.currentThread();
    }
  

    public void addParameters(String[] keys, String[] values) {
        for (int i = 0; i < keys.length; i++) {
            addParameter(keys[i], values[i]);
        }
    }
  
    private void addParameter(String key, String value) {
        // Some of the parameters coming up from native code might be
        // null; filter these out for cleanliness

	// IE plugin does not allow key starts with '_'
	// To be consistent, we filter out them too
        if (key != null && key.charAt(0) != '_' && !key.equals("PARAM")) {
            key = key.trim().toLowerCase(java.util.Locale.ENGLISH);
            params.put(key, value);
        }
    }

    public void setWindow(long hWndControlWindow, 
                          int x, int y, 
                          int width, int height,
                          int clipTop, int clipLeft,
                          int clipBottom, int clipRight) {
    
        if (DEBUG) {
            System.out.println("MozillaPlugin.setWindow " + this + 
                               " hWndControlWindow = " + hWndControlWindow);
        }

        boolean mustRelink = false;
        if (isMacOSX) {
            if (this.hWndControlWindow != 0 &&
                this.hWndControlWindow != hWndControlWindow &&
                sharedWindow != null) {
                // A tab was just dragged out of Safari into its own
                // window; hook up to the new window
                unlinkSharedWindow();
                mustRelink = true;
            }
        }

        this.hWndControlWindow = hWndControlWindow;

        if (mustRelink) {
            setChildWindowHandle(childWindowHandle);
        }

        if (appletID != null) {
            if (DEBUG) {
                System.out.println("MozillaPlugin.setWindow setting applet " + appletID +
                                   " size to " + width + ", " + height);
            }
            JVMManager.getManager().setAppletSize(appletID, width, height);
            if (isMacOSX) {
                // We also need to reposition and clip the shared window
                sharedWindowX = x;
                sharedWindowY = y;
                sharedWindowClipX = clipLeft;
                sharedWindowClipY = clipTop;
                sharedWindowClipWidth = clipRight - clipLeft;
                sharedWindowClipHeight = clipBottom - clipTop;
                updateLocationAndClip();
            }
        } else {
            // Need to feed down the initial width and height to the applet
            // in order to properly handle constructs like 'width="100%"'
            params.put("width", Integer.toString(width));
            params.put("height", Integer.toString(height));
            maybeStartApplet(false);
        }
    }
 
    public boolean print(long hdc, int x, int y, int width, int height) {

        if (DEBUG) {
            System.out.println("MozillaPlugin.printApplet " + this + 
                               " hdc = " + hdc +
                               ", x = " + x + ", y = " + y + 
                               ", width = " + width + ", height = " + height);
        }

        if (appletID != null) {
            if (DEBUG) {
                System.out.println("MozillaPlugin.print() applet " + appletID);
            }
            return JVMManager.getManager().printApplet(appletID, hdc, x, y, width, height);
        } 
        return false;
    } 

    public void destroy() {
        if (DEBUG) {
            System.out.println("MozillaPlugin.destroy");
        }
        stopApplet();

        if (javaScriptWindow != 0) {
            npnReleaseObject(javaScriptWindow);
            javaScriptWindow = 0;
        }

        destroyed = true;
        disposeMainThreadEvent();
    }
  
    // An indication of whether destroy() has already been called on this plugin
    public boolean isDestroyed() {
        return destroyed;
    }

    private void maybeStartApplet(boolean isForDummyApplet) {
        if (!started) {
            if ((hWndControlWindow != 0 || isForDummyApplet) &&
                params != null) {
                if (DEBUG) {
                    System.out.println("  Attempting to start " +
                                       (isForDummyApplet ? "dummy " : "") + "applet");
                }
                started = true;
                // set java_version if it hasn't been set due to not being
                // specified as part of the html parameters and requestedVersion isn't null
                if ((params.get("java_version") == null) && (requestedVersion != null)) {
                    params.put("java_version", requestedVersion);
                }
                if (!isForDummyApplet) {
                    appletID = JVMManager.getManager().startApplet(params, this, hWndControlWindow, getConnectionHandle(), USE_XEMBED);
                } else {
                    // FIXME: the modification of the document base is a hack to work
                    // around the case where an expression like "javascript:alert(window.java);"
                    // is typed into the browser's URL field without a page being viewed,
                    // in which case there is no document base
                    if (docbase == null) {
                        docbase = "file:///";
                    }
                    appletID = JVMManager.getManager().startDummyApplet(params, this);
                }
                appletStarted(appletID, handler);
                if (DEBUG) {
                    System.out.println("  Received applet ID " + appletID);
                }
            } else {
                if (DEBUG) {
                    System.out.println("  Skipped starting applet: hWndControlWindow = " + hWndControlWindow + ", params = " + params);
                }
            }
        }
    }

    private void stopApplet() {
	if (appletID == null) return;
	if (DEBUG) {
	    System.out.println("  Stopping applet ID " + appletID);
	}
	
	boolean waitForSignal = true;
	Object stopMark = (Object) appletStopMark.get();
	if (stopMark != null) {
	    // recursive stopApplet() detected, don't wait for ack
	    waitForSignal = false;
	} else {
	    appletStopMark.set(this);
	}
	
	if (isMacOSX) {
	    // First unlink the shared window
	    unlinkSharedWindow();
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
		    if (waitForSignal) {
			handler.waitForSignal(endTime - curTime);
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
	    
		// reset the appletStopMark to null if this call sets it
		appletStopMark.set(stopMark);
	    }
	}
    }
    
    //----------------------------------------------------------------------
    // Implementation of the Plugin interface
    //

    public void invokeLater(Runnable callback) {
        // Calls to invokeLater0 don't preempt our wait loop (at least
        // on the Unix platfom) and we don't yet have the required
        // NPN_PluginThreadFlushAsyncCalls() in the NPAPI. Therefore
        // we need to avoid race conditions by always maintaining the
        // runnable queue ourselves; otherwise we could find ourselves
        // in a situation where the browser gives us control but has
        // some pending async calls it needs to process in order for
        // us to make progress.
        getRunnableQueue(pluginMainThread).add(new RunnableWrapper(callback, this));
        invokeLater0(npp, drainer);
        // Always signal main thread in case it's waiting
        notifyMainThread();
    }

    public void notifyMainThread() {
        getMainThreadEvent(pluginMainThread).signal();
    }
  
    public String getDocumentBase() {
        return docbase;
    }

    public void showDocument(String url, String target) {
        if (npp != 0) {
            showDocument0(npp, url, target);
        }
    }

    public void showStatus(String status) {
	if ( npp != 0 && status != null ){
	    showStatus0(npp, status);
	}
    }

    public String getCookie(URL url) throws CookieUnavailableException {
        try {
            return getCookie0(npp, url.toString());
        } catch (Exception e) {
            throw (CookieUnavailableException)
                new CookieUnavailableException().initCause(e);
        }
    }

    public void setCookie(URL url, String value) throws CookieUnavailableException {
        try {
            setCookie0(npp, url.toString(), value);
        } catch (Exception e) {
            throw (CookieUnavailableException)
                new CookieUnavailableException().initCause(e);
        }
    }

    public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
                                                    String scheme, String realm, URL requestURL,
                                                    boolean proxyAuthentication) {
        return getPAFromCharArray(getAuthentication0(npp,
                                                     protocol,
                                                     host,
                                                     port,
                                                     scheme,
                                                     realm));
    }

    private static PasswordAuthentication getPAFromCharArray(char[] credential) {
        if(credential == null)
            return null;

        int index = 0;
        while(index < credential.length && ':' != credential[index])
            index ++;
                
        PasswordAuthentication pa = null;
        if(index < credential.length) {
            String userName = new String(credential, 0, index);
            char[] password = extractArray(credential, index + 1);
            pa = new PasswordAuthentication(userName, password);
            resetArray(password);
        }
        resetArray(credential);

        return pa;
    }

    private static void resetArray(char[] arr) {
        java.util.Arrays.fill(arr, ' ');
    }


    private static char[] extractArray(char[] src, int start) {
        char[] dest = new char[src.length - start];
        for(int index = 0; index < dest.length; index ++)
            dest[index] = src[index + start];

        return dest;
    }

    //----------------------------------------------------------------------
    // Implementation of proxy lookups
    // This should be unified with the IE implementation
    //

    public String getProxy(String url) {
        return getProxy0(npp, url);
    }

    //---------------------------------------
    // These are used only on Mac OS X
    private long parentWindowHandle;
    private long childWindowHandle;
    private SharedWindow sharedWindow;
    private double sharedWindowX, sharedWindowY;
    private double sharedWindowClipX, sharedWindowClipY, sharedWindowClipWidth, sharedWindowClipHeight;
    private static long connectionHandle;
    private static long getConnectionHandle() {
        if (SystemUtil.getOSType() == SystemUtil.MACOSX) {
            if (connectionHandle == 0) {
                connectionHandle = SharedWindowHost.getConnectionHandle();
            }
        }
        return connectionHandle;
    }
    public void setChildWindowHandle(long windowHandle) {
        if (isMacOSX) {
            if (hWndControlWindow == 0) {
                if (DEBUG) {
                    System.out.println("MozillaPlugin.setChildWindowHandle: hWndControlWindow == 0");
                }
                return;
            }
            
            // NOTE: right now we always force the drawing model to
            // one which will cause Carbon to be used; later, we will
            // need to switch to one which will cause Cocoa to be used
            // for the event model, in which case how we get the
            // window reference will change
            parentWindowHandle = getNativeWindowFromCarbonWindowRef(hWndControlWindow);
            this.childWindowHandle = windowHandle;
            if (DEBUG) {
                System.out.println("MozillaPlugin: linking 0x" + Long.toHexString(parentWindowHandle) +
                                   " to 0x" + Long.toHexString(childWindowHandle));
            }
            sharedWindow = SharedWindowHost.linkSharedWindowTo(parentWindowHandle, childWindowHandle);
            // FIXME: this is a major hack needed only because the
            // call to JFrame.setVisible(true) isn't completely
            // synchronous on the other side, so we can't control the
            // position of the window immediately after it has been
            // shown
            new Thread(new Runnable() {
                    public void run() {
                        for (int i = 0; i < 20; i++) {
                            try {
                                Thread.sleep(100);
                            } catch (InterruptedException e) {
                            }
                            invokeLater(new Runnable() {
                                    public void run() {
                                        updateLocationAndClip();
                                    }
                                });
                        }
                    }
                }).start();
        }
    }
    private native long getNativeWindowFromCarbonWindowRef(long windowRef);
    // Output bounds are top, left, bottom, right
    private native void getCarbonWindowBounds(long windowRef, int which, int[] bounds);
    private static final int kWindowTitleBarRgn = 0;
    private static final int kWindowStructureRgn = 32;
    private static final int kWindowContentRgn  = 33;
    private static final int kWindowGlobalPortRgn = 40;
    private static final int kWindowToolbarButtonRgn = 41;

    private boolean useFirefoxClipWorkaround;

    private void updateLocationAndClip() {
        if (sharedWindow != null) {
            // Through experimentation it turns out that the structure
            // region is the right one to use along with the offset
            // from NPP_SetWindow to properly place the applet on the page
            int[] structureBounds = new int[4];
            getCarbonWindowBounds(hWndControlWindow, kWindowStructureRgn, structureBounds);
            
            if (DEBUG) {
                System.out.println("MozillaPlugin: Setting location: " + sharedWindowX + ", " + sharedWindowY);
                System.out.println("               clip: " + sharedWindowClipX + ", " + sharedWindowClipY + " -- " + sharedWindowClipWidth + ", " + sharedWindowClipHeight);
                System.out.println("               structureBounds[0] = " +  structureBounds[0] + "  structureBounds[1] = " +  structureBounds[1] + "  structureBounds[2] = " +  structureBounds[2] + "  structureBounds[3] = " + structureBounds[3]);
            }
            int locY = (int) sharedWindowY + structureBounds[0];
            sharedWindow.setLocation(sharedWindowX + structureBounds[1], locY);
            int clipY = (int) (sharedWindowClipY - sharedWindowY);
            // If we ever see a bogus clip region come in, turn on
            // this workaround, which is a concession to apparently
            // incorrect clip coordinates coming in from Firefox
            // (off by +22)
            if (clipY < 0) {
                useFirefoxClipWorkaround = true;
            }
            if (useFirefoxClipWorkaround) {
                clipY += 22;
            }
            sharedWindow.setClip(sharedWindowClipX - sharedWindowX,
                                 clipY,
                                 sharedWindowClipWidth,
                                 sharedWindowClipHeight);
            // On Mac OS X this also brings the PluginEmbeddedFrame to
            // the front, which is the desired side-effect; might
            // consider adding another message for this to avoid focus
            // changes
            JVMManager.getManager().synthesizeWindowActivation(appletID, true);
        }
    }

    private void unlinkSharedWindow() {
        if (sharedWindow != null) {
            SharedWindowHost.unlinkSharedWindowFrame(parentWindowHandle, sharedWindow);
            sharedWindow = null;
        }
    }

    //
    //---------------------------------------


    private static native void initServiceManager();

    private native void invokeLater0(long npp, Runnable callback);

    private native void showDocument0(long npp, String url, String target);

    private native void showStatus0(long npp, String status);
  
    private native String getProxy0(long npp, String url);
    private native String getCookie0(long npp, String url) throws RuntimeException;
    private native void   setCookie0(long npp, String url, String value) throws RuntimeException;
    private native char[] getAuthentication0(long npp,
                                             String protocol,
                                             String hostName,
                                             int port,
                                             String scheme,
                                             String realm);

    //----------------------------------------------------------------------
    // LiveConnect support routines
    //

    // This is the NPObject* associated with the browser window
    // containing this plugin instance. We need to keep track of it
    // because its reference count is automatically incremented when
    // we fetch it using NPN_GetValue and we need to decrement it when
    // we 
    private long javaScriptWindow;

    ///////////////////////////////////////////////////////////////////////
    //                                                                   //
    // Implementation of the LiveConnect methods in the Plugin interface //
    //                                                                   //
    ///////////////////////////////////////////////////////////////////////

    private void checkValidity() {
        if (isDestroyed()) {
            JSException exc = new JSException("Plugin instance was already destroyed! Should not reach here!");
            if (DEBUG) {
                exc.printStackTrace();
            }
            throw exc;
        }
    }

    // The reason of having an internal version of javaScriptGetWindow is to support liveconnect
    // functionality inside the browser process only. This is useful to support showDocument
    // method in AppletContext.
    private BrowserSideObject javaScriptGetWindowInternal(boolean registerWithLiveConnectSupport) {
        if (javaScriptWindow == 0) {
            if (npp == 0)
                return null;

            javaScriptWindow = javaScriptGetWindow0(npp);
        }

        if (javaScriptWindow == 0) {
            // Probably shouldn't happen; should we throw an exception?
            return null;
        }

        return newBrowserSideObject(javaScriptWindow, registerWithLiveConnectSupport);
    }

    public BrowserSideObject javaScriptGetWindow() {
        checkValidity();

        return javaScriptGetWindowInternal(true);
    }

    private native long javaScriptGetWindow0(long npp);

    public void javaScriptRetainObject(BrowserSideObject obj) {
        checkValidity();
        npnRetainObject(obj.getNativeObjectReference());
    }

    public void javaScriptReleaseObject(BrowserSideObject obj) {
        checkValidity();
        npnReleaseObject(obj.getNativeObjectReference());
    }

    public Object javaScriptCall(BrowserSideObject obj,
                                 String methodName,
                                 Object[] args) throws JSException {
        checkValidity();
        if (npp == 0)
            return null;

        long argArray = 0;
        long variantRes = 0;
        try {
            if (args != null && args.length > 0) {
                argArray = allocateVariantArray(args.length);
                for (int i = 0; i < args.length; i++) {
                    objectToVariantArrayElement(args[i], argArray, i);
                }
            }

            long identifier = npnGetStringIdentifier(methodName);
            variantRes = allocateVariantArray(1);
            boolean ret = npnInvoke(npp,
                                    obj.getNativeObjectReference(),
                                    identifier,
                                    argArray,
                                    (args == null ? 0 : args.length),
                                    variantRes);
            if (!ret) {
                // See whether it was because the method didn't exist
                if (!npnHasMethod(npp,
                                  obj.getNativeObjectReference(),
                                  identifier)) {
                    throw JSExceptions.noSuchMethod(methodName);
                }

                // Unfortunately we can't get a more descriptive error from the JavaScript engine
                throw new JSException("JavaScript error while calling \"" + methodName + "\"");
            }

            return variantArrayElementToObject(variantRes, 0);
        } finally {
            if (argArray != 0) {
                freeVariantArray(argArray, args.length);
            }
            if (variantRes != 0) {
                freeVariantArray(variantRes, 1);
            }
        }
    }

    public Object javaScriptEval(BrowserSideObject obj, String code) throws JSException {
        checkValidity();
        long variantRes = allocateVariantArray(1);
        try {
            boolean ret = npnEvaluate(npp,
                                      obj.getNativeObjectReference(),
                                      code,
                                      variantRes);
            if (!ret) {
                throw new JSException("JavaScript error evaluating code \"" + code + "\"");
            }

            return variantArrayElementToObject(variantRes, 0);
        } finally {
            if (variantRes != 0) {
                freeVariantArray(variantRes, 1);
            }
        }
    }


    public Object javaScriptGetMember(BrowserSideObject obj, String name) throws JSException {
        checkValidity();
        return javaScriptGetMemberImpl(obj.getNativeObjectReference(),
                                       npnGetStringIdentifier(name),
                                       name, 0);
    }

    public void javaScriptSetMember(BrowserSideObject obj, String name, 
                                    Object value) throws JSException {
        checkValidity();
        javaScriptSetMemberImpl(obj.getNativeObjectReference(),
                                npnGetStringIdentifier(name),
                                value,
                                name, 0);
    }

    public void javaScriptRemoveMember(BrowserSideObject obj, String name) throws JSException {
        checkValidity();
        if (npp == 0)
            return;

        if (!npnRemoveProperty(npp, obj.getNativeObjectReference(),
                               npnGetStringIdentifier(name))) {
            throw new JSException("JavaScript error removing member \"" + name + "\"");
        }
    }
  
    public Object javaScriptGetSlot(BrowserSideObject obj, int index) throws JSException {
        checkValidity();
        return javaScriptGetMemberImpl(obj.getNativeObjectReference(),
                                       npnGetIntIdentifier(index),
                                       null, index);
    }

    public void javaScriptSetSlot(BrowserSideObject obj, int index, Object value) throws JSException {
        checkValidity();
        javaScriptSetMemberImpl(obj.getNativeObjectReference(),
                                npnGetIntIdentifier(index),
                                value,
                                null, index);
    }

    public String javaScriptToString(BrowserSideObject obj) {
        checkValidity();
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

    private Object javaScriptGetMemberImpl(long npObject, long npIdentifier, String name, int index) throws JSException {
        if (npp == 0)
            return null;

        long variantRes = allocateVariantArray(1);
        try {
            boolean ret = npnHasProperty(npp, npObject, npIdentifier);
            if (!ret) {
                if (name != null) {
                    throw JSExceptions.noSuchProperty(name);
                } else {
                    throw JSExceptions.noSuchSlot(index);
                }
            }
            ret = npnGetProperty(npp, npObject, npIdentifier, variantRes);
            if (!ret) {
                if (name != null) {
                    throw new JSException("JavaScript error while getting property \"" + name + "\"");
                } else {
                    throw new JSException("JavaScript error while getting index " + index);
                }
            }

            return variantArrayElementToObject(variantRes, 0);
        } finally {
            if (variantRes != 0) {
                freeVariantArray(variantRes, 1);
            }
        }
    }

    private void javaScriptSetMemberImpl(long npObject, long npIdentifier, Object value, String name, int index) throws JSException {
        if (npp == 0)
            return;

        long variantArg = allocateVariantArray(1);
        try {
            objectToVariantArrayElement(value, variantArg, 0);
            boolean ret = npnSetProperty(npp, npObject, npIdentifier, variantArg);
            if (!ret) {
                if (name != null) {
                    throw new JSException("JavaScript error while setting property \"" + name + "\"");
                } else {
                    throw new JSException("JavaScript error while setting index " + index);
                }
            }
        } finally {
            if (variantArg != 0) {
                freeVariantArray(variantArg, 1);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //                                                                   //
    // NPN_ functions from browser for interacting with scripting engine //
    //                                                                   //
    ///////////////////////////////////////////////////////////////////////

    private static native long npnGetStringIdentifier(String name);
    private static native long npnGetIntIdentifier(int id);
    private static native boolean npnIdentifierIsString(long npIdentifier);
    private static native String npnUTF8FromIdentifier(long npIdentifier);
    private static native int  npnIntFromIdentifier(long npIdentifier);
    private static native void npnRetainObject(long npObject);
    private static native void npnReleaseObject(long npObject);
    private static native boolean npnInvoke(long npp,
                                            long npObject,
                                            long npIdentifier,
                                            long argArray,
                                            int  numArgs,
                                            long variantRes);
    private static native boolean npnEvaluate(long npp,
                                              long npObject,
                                              String code,
                                              long variantRes);
    private static native boolean npnGetProperty(long npp,
                                                 long npObject,
                                                 long npIdentifier,
                                                 long variantRes);
    private static native boolean npnSetProperty(long npp,
                                                 long npObject,
                                                 long npIdentifier,
                                                 long variantValue);
    private static native boolean npnRemoveProperty(long npp,
                                                    long npObject,
                                                    long npIdentifier);
    private static native boolean npnHasProperty(long npp,
                                                 long npObject,
                                                 long npIdentifier);
    private static native boolean npnHasMethod(long npp,
                                               long npObject,
                                               long npIdentifier);
    private static native void npnSetException(long npObject,
                                               String message);

    //----------------------------------------------------------------------
    // Routines supporting JavaScript -> Java calls
    //

    ///////////////////////////////////////////////////////////////
    //                                                           //
    // Implementation of LiveConnect methods from AbstractPlugin //
    //                                                           //
    ///////////////////////////////////////////////////////////////

    protected boolean scriptingObjectArgumentListsAreReversed() {
        return false;
    }

    protected native long allocateVariantArray(int size);
    protected native void freeVariantArray(long array, int size);
    // Provide Java-level interposition points for all of these methods in case we need them in the future
    protected native void setVariantArrayElement0(long variantArray, int index, boolean value);
    protected native void setVariantArrayElement0(long variantArray, int index, byte value);
    protected native void setVariantArrayElement0(long variantArray, int index, char value);
    protected native void setVariantArrayElement0(long variantArray, int index, short value);
    protected native void setVariantArrayElement0(long variantArray, int index, int value);
    protected native void setVariantArrayElement0(long variantArray, int index, long value);
    protected native void setVariantArrayElement0(long variantArray, int index, float value);
    protected native void setVariantArrayElement0(long variantArray, int index, double value);
    protected native void setVariantArrayElement0(long variantArray, int index, String value);
    protected native void setVariantArrayElementToScriptingObject0(long variantArray, int index, long value);
    protected native void setVariantArrayElementToVoid0(long variantArray, int index);

    protected void setVariantArrayElement(long variantArray, int index, boolean value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    protected void setVariantArrayElement(long variantArray, int index, byte value) {
        if (DISABLE_406251_WORKAROUND) {
            setVariantArrayElement0(variantArray, index, value);
        } else {
            setVariantArrayElement(variantArray, index, (int) value);
        }
    }
    protected void setVariantArrayElement(long variantArray, int index, char value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    protected void setVariantArrayElement(long variantArray, int index, short value) {
        if (DISABLE_406251_WORKAROUND) {
            setVariantArrayElement0(variantArray, index, value);
        } else {
            setVariantArrayElement(variantArray, index, (int) value);
        }
    }
    protected void setVariantArrayElement(long variantArray, int index, int value) {
        if (DISABLE_406251_WORKAROUND) {
            setVariantArrayElement0(variantArray, index, value);
        } else {
            // It appears that Firefox uses a type tagging system where
            // they lose the high two bits of 32-bit integer values;
            // promote these to wider JavaScript types

            if ((value & 0xC0000000) != 0) {
                setVariantArrayElement(variantArray, index, (long) value);
            } else {
                setVariantArrayElement0(variantArray, index, value);
            }
        }
    }
    protected void setVariantArrayElement(long variantArray, int index, long value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    protected void setVariantArrayElement(long variantArray, int index, float value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    protected void setVariantArrayElement(long variantArray, int index, double value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    protected void setVariantArrayElement(long variantArray, int index, String value) {
        setVariantArrayElement0(variantArray, index, value);
    }
    // Sets the given element either to the given non-null NPObject or NPVariantType_Null
    protected void setVariantArrayElementToScriptingObject(long variantArray, int index, long value) {
        setVariantArrayElementToScriptingObject0(variantArray, index, value);
    }
    // Sets the given element to NPVariantType_Void
    protected void setVariantArrayElementToVoid(long variantArray, int index) {
        setVariantArrayElementToVoid0(variantArray, index);
    }

    //
    // This method helps convert NPVariants to Java objects for method invocation results
    //
    protected Object variantArrayElementToObject(long variantArray, int index) {
        return variantArrayElementToObject0(mozPluginInstance, variantArray, index);
    }

    protected static native Object variantArrayElementToObject0(long mozPluginInstance, long variantArray, int index);

    // Maps RemoteJavaObject instances to C/C++ JavaObject (i.e., NPObject) objects
    private static Map/*<RemoteJavaObject, Long>*/ npObjectMap = new HashMap();

    private static native long allocateNPObject(long mozPluginInstance,
                                                RemoteJavaObject object);

    private static native long allocateNPObjectForJavaNameSpace(long mozPluginInstance,
                                                                String nameSpace);

    protected long lookupScriptingObject(RemoteJavaObject object, boolean objectIsApplet) {
        synchronized(npObjectMap) {
            Long val = (Long) npObjectMap.get(object);
            if (val != null)
                return val.longValue();
            long npObject = allocateNPObject(mozPluginInstance, object);
            if (npObject != 0) {
                val = new Long(npObject);
                npObjectMap.put(object, val);
                return val.longValue();
            }
            return 0;
        }
    }

    protected Object wrapOrUnwrapScriptingObject(long scriptingObject) {
        // We make the determination of whether NPObjects are ours or
        // not down in our native code; see MozPluginInstance::variantToJObject()
        return newBrowserSideObject(scriptingObject);
    }

    protected String identifierToString(long npIdentifier) {
        // We make no distinction between string and integer
        // identifiers in the pure Java code in our implementation
        if (npnIdentifierIsString(npIdentifier)) {
            return npnUTF8FromIdentifier(npIdentifier);
        } else {
            return Integer.toString(npnIntFromIdentifier(npIdentifier));
        }
    }

    protected void fillInExceptionInfo(long exceptionInfo, String message) {
        if (exceptionInfo == 0) {
            if (DEBUG) {
                System.out.println("MozillaPlugin: JavaScript error: " + message);
            }
        } else {
            npnSetException(exceptionInfo, message);
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

    protected void releaseRemoteJavaObject(RemoteJavaObject object) {
        super.releaseRemoteJavaObject(object);
        synchronized(npObjectMap) {
            npObjectMap.remove(object);
        }
    }

    // In order to support the Packages.* and java.* keywords in the
    // Mozilla JavaScript engine when using the NPRuntime-based Java
    // Plug-In, we have a private contract with the Mozilla code. When
    // they want to add support for the Packages.* keyword, they
    // create an instance of our plugin, but don't call NPP_SetWindow.
    // Then when they query that instance our plugin for its
    // scriptable object, they expect to get back an NPObject
    // corresponding to the root of the Java name space. Overriding
    // getScriptingObjectForApplet appropriately allows us to provide
    // these semantics.
    protected long getScriptingObjectForApplet(long exceptionInfo) {
        if (hWndControlWindow == 0) {
            maybeStartApplet(true);
            return allocateNPObjectForJavaNameSpace(mozPluginInstance, "");
        }

        return super.getScriptingObjectForApplet(exceptionInfo);
    }

    //----------------------------------------------------------------------
    // Support for blocking of the browser window by modal dialogs
    //

    public void waitForSignalWithModalBlocking() {
        if (DEBUG) {
            System.out.println("MozillaPlugin entering waitForSignalWithModalBlocking for " + appletID);
        }
        handler.waitForSignalWithModalBlocking(this.hWndControlWindow, appletID.getID());
        if (DEBUG) {
            System.out.println("MozillaPlugin exiting waitForSignalWithModalBlocking for " + appletID);
        }
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    // We need the runnable queue to be per-plugin-main-thread rather
    // than per-MozillaPlugin because from a semantic standpoint we
    // need to be able to execute Runnables associated with other
    // MozillaPlugin instances on the same web page while we have the
    // flow of control and not the browser. Right now this coalesces
    // to a single plugin main thread, but this might change in the
    // future if Firefox is changed to have a thread per open window.
    private static final Map/*<Thread, List<Runnable>>*/ runnableQueueMap =
        new HashMap/*<Thread, List<Runnable>>*/();
    private static final Map/*<Thread, Event>*/ mainThreadEventMap =
        new HashMap/*<Thread, Event>*/();

    // We also need to be absolutely sure that we do not execute
    // Runnables associated with previously-destroyed plugin
    // instances.
    static class RunnableWrapper implements Runnable {
        private Runnable runnable;
        private MozillaPlugin plugin;

        public RunnableWrapper(Runnable runnable, MozillaPlugin plugin) {
            this.runnable = runnable;
            this.plugin = plugin;
        }

        public void run() {
            runnable.run();
        }

        public MozillaPlugin getPlugin() {
            return plugin;
        }
    }

    private static synchronized List/*<Runnable>*/ getRunnableQueue(Thread pluginMainThread) {
        List/*<Runnable>*/ queue = (List) runnableQueueMap.get(pluginMainThread);
        if (queue == null) {
            queue = Collections.synchronizedList(new LinkedList());
            runnableQueueMap.put(pluginMainThread, queue);
        }
        return queue;
    }

    private boolean iCreatedMainThreadEvent = false;
    private Event getMainThreadEvent(Thread pluginMainThread) {
        synchronized (mainThreadEventMap) {
            Event event = (Event) mainThreadEventMap.get(pluginMainThread);
            if (event == null) {
                event = handler.createEvent();
                mainThreadEventMap.put(pluginMainThread, event);
            }
            return event;
        }
    }

    private void disposeMainThreadEvent() {
        if (iCreatedMainThreadEvent) {
            synchronized (mainThreadEventMap) {
                Event event = (Event) mainThreadEventMap.remove(pluginMainThread);
                if (event != null) {
                    event.dispose();
                }
            }
            iCreatedMainThreadEvent = false;
        }
    }

    private void drainRunnableQueue() {
        List/*<Runnable>*/ queue = getRunnableQueue(pluginMainThread);
        while (!queue.isEmpty()) {
            RunnableWrapper r = (RunnableWrapper) queue.remove(0);
            if (!r.getPlugin().isDestroyed()) {
                r.run();
            }
        }
    }

    class DrainRunnableQueue implements Runnable {
        public void run() {
            drainRunnableQueue();
        }
    }
    private Runnable drainer = new DrainRunnableQueue();

    // This abstracts away the platform-dependent portions of this class.
    // 
    static abstract class PlatformDependentHandler extends ResultHandler {
        public abstract Event createEvent();
        public abstract void waitForSignalWithModalBlocking(long hWndControlWindow, int appletID);
    }

    class WindowsHandler extends PlatformDependentHandler {
        public Event createEvent() {
            return IPCFactory.getFactory().createEvent(null);
        }

        public void waitForSignal() {
            waitForSignal(-1);
        }

        public void waitForSignal(long timeout) {
            WindowsHelper.runMessagePump(getMainThreadEvent(pluginMainThread), timeout,
                                         ModalitySupport.appletShouldBlockBrowser(appletID));
            drainRunnableQueue();
        }

        public void waitForSignalWithModalBlocking(long hWndControlWindow, int appletID) {
            boolean installedHooks = WindowsHelper.registerModalDialogHooks(hWndControlWindow, appletID);
            waitForSignal();
            if (installedHooks) {
                WindowsHelper.unregisterModalDialogHooks(hWndControlWindow);
            }
        }
    }

    class UnixHandler extends PlatformDependentHandler {
        public Event createEvent() {
            return new InProcEvent();
        }

        public void waitForSignal() {
            waitForSignal(0);
        }

        public void waitForSignal(long millis) {
            getMainThreadEvent(pluginMainThread).waitForSignal(millis);
            drainRunnableQueue();
        }

        public void waitForSignalWithModalBlocking(long hWndControlWindow, int appletID) {
            waitForSignal();
        }
    }

    private PlatformDependentHandler getPlatformDependentHandler() {
        if (SystemUtil.getOSType() == SystemUtil.WINDOWS) {
            return new WindowsHandler();
        } else if (SystemUtil.getOSType() == SystemUtil.UNIX ||
                   SystemUtil.getOSType() == SystemUtil.MACOSX) {
            // FIXME: could probably do better on Mac OS X; need to
            // figure out whether it's possible to run a nested
            // message pump
            return new UnixHandler();
        } else {
            throw new RuntimeException("Need to port platform-specific portion of MozillaPlugin to your platform");
        }
    }

    private String getRequestedVersion(String mimeType) {
        if (mimeType != null && mimeType.indexOf(_versionPattern) != -1) {
            String[] splittedStrings = mimeType.split(_versionPattern);
            if ((splittedStrings.length == 2) &&
                isValidVersionString(splittedStrings[1])) {
                // Append the wildcard character.
                // Note that we treat the version selection in Firefox
                // as a request for a minimum version, which is the
                // original semantic meaning. Note also that because
                // we are already running on at least a version 6 JRE,
                // this version attribute is not really that useful.
                return splittedStrings[1] + "+";
            }
        }
        return null;
    }

    private boolean isValidVersionString(String versionString) {
        if (versionString.length() < 3)
            return false;
        for (int i = 0; i < versionString.length(); i++) {
            char c = versionString.charAt(i);
            if (validChars.indexOf(c) == -1) {
                return false;
            }
        }
        if (versionString.compareTo("1.4") < 0) {
            // We only support running on top of 1.4 at the earliest
            return false;
        }
        return true;
    }
}
