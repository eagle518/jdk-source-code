/*
 * @(#)LiveConnectSupport.java	1.37 10/05/21
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.lang.ref.*;
import java.lang.reflect.*;
import java.security.*;
import java.util.*;

// Imports for getJSProtectionDomain
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.SocketPermission;
import java.net.UnknownHostException;
import java.net.URL;
import java.text.NumberFormat;
import java.text.ParseException;
import sun.net.www.ParseUtil;
import sun.security.util.SecurityConstants;

import sun.plugin2.applet.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.SystemUtil;

// UI related imports
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;

// Security-related imports from the original plugin implementation
import sun.plugin.liveconnect.JavaScriptProtectionDomain;

// Security-related imports from common deployment code
import com.sun.deploy.security.CeilingPolicy;

import netscape.javascript.*;

// Supply the exported interfaces
import com.sun.java.browser.plugin2.liveconnect.v1.*;

import netscape.javascript.JSException;

/** Provides certain services needed for LiveConnect on the client
    side, such as registering objects we expose back to the web
    browser and reference counting browser-side JavaScript objects. */

public class LiveConnectSupport {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    private static Pipe pipe;
    private static int jvmID;

    private LiveConnectSupport() {}

    /** Initializes this class. Must be called before any of the other methods. */
    public static void initialize(Pipe pipe, int jvmID) {
        LiveConnectSupport.pipe = pipe;
        LiveConnectSupport.jvmID = jvmID;
        cleanupThread = new BrowserSideObjectCleanupThread();
        cleanupThread.start();
    }

    /** Shuts down this class -- this should ideally be called during
        termination of the surrounding environment. */
    public static void shutdown() {
        shouldStop = true;
        cleanupThread.interrupt();
    }

    /** Registers an applet as having been started. This doesn't mean
        that we've actually called start() on the applet yet; it only
        means that the applet's container has been created. */
    public static synchronized void appletStarted(int appletID, Plugin2Manager manager) {
        appletInfoMap.put(new Integer(appletID), new PerAppletInfo(appletID, manager));
    }

    /** Unregisters an applet when it has been stopped. */
    public static synchronized void appletStopped(int appletID) {
        PerAppletInfo info = (PerAppletInfo) appletInfoMap.remove(new Integer(appletID));
        if (info != null) {
            info.stop();
        }
    }

    /** Exports an object for consumption by the web browser.
        Principally this detects non-serializable Java objects and
        produces RemoteJavaObjects for them. The skipUnboxing flag
        indicates whether this object should not be unboxed even if
        the result is a String or a boxing object for a primitive
        type. The isApplet flag indicates whether or not this object
        represents the top-level object exported to the browser, i.e.,
        the applet; this is used to attach "magic" properties to only
        that object, such as the scoped Packages keyword. */
    public static Object exportObject(Object obj, int appletID, boolean skipUnboxing, boolean isApplet) {
        if (obj == null) {
            // Never export a null value as a RemoteJavaObject
            return obj;
        }
        if (ArgumentHelper.isPrimitiveOrString(obj)) {
            if (!skipUnboxing) {
                return obj;
            }
        }
        if (obj instanceof MessagePassingJSObject)
            return ((MessagePassingJSObject) obj).getBrowserSideObject();
        return exportRemoteObject(obj, appletID, isApplet);
    }

    /** Imports an object passed from the web browser for consumption
        by this VM. */
    public static Object importObject(Object obj, int appletID) {
        if (obj == null)
            return obj;
        if (ArgumentHelper.isPrimitiveOrString(obj))
            return obj;
        if (obj instanceof BrowserSideObject) {
            // Convert this to a MessagePassingJSObject and increment its reference count
            BrowserSideObject browserObject = (BrowserSideObject) obj;
            MessagePassingJSObject jsObject = new MessagePassingJSObject(browserObject, appletID, pipe);
            ref(browserObject, appletID);
            track(jsObject);
            return jsObject;
        }
        if (obj instanceof RemoteJavaObject) {
            return importRemoteObject((RemoteJavaObject) obj);
        }

        throw new IllegalArgumentException("Unsupported argument type " + obj.getClass().getName());
    }

    public static Object importOneWayJSObject(Object obj, int appletID, Plugin2Manager manager) {
        if (obj == null)
            return obj;
        if (ArgumentHelper.isPrimitiveOrString(obj))
            return obj;
        if (obj instanceof BrowserSideObject) {
            // Convert this to a MessagePassingJSObject and increment its reference count
            BrowserSideObject browserObject = (BrowserSideObject) obj;
            MessagePassingJSObject targetJSObject = new MessagePassingJSObject(browserObject, appletID, pipe, manager);
            MessagePassingOneWayJSObject jsObject = new MessagePassingOneWayJSObject(targetJSObject);
            ref(browserObject, appletID);
            track2(jsObject);
            return jsObject;
        }
        if (obj instanceof RemoteJavaObject) {
            return importRemoteObject((RemoteJavaObject) obj);
        }

        throw new IllegalArgumentException("Unsupported argument type " + obj.getClass().getName());
    }

    public static void doObjectOp(JavaObjectOpMessage msg) throws IOException {
        // If the message's Conversation is null, this means that it
        // was a JavaScript -> Java call initiated from the web
        // browser when there was no Java -> JavaScript call ongoing.
        // In this case we don't want to block the main message
        // handler thread, so we hand off this request to the
        // per-applet LiveConnect worker thread.
        RemoteJavaObject obj = msg.getObject();
        PerAppletInfo info = getInfo(obj.getAppletID());
        if (info != null) {
            if (msg.getConversation() == null) {
                info.enqueue(msg);
            } else {
		info.doObjectOp(msg);
            }
        } else {
            pipe.send(new JavaReplyMessage(msg.getConversation(),
                                           msg.getResultID(),
                                           null,
                                           false,
                                           "Applet ID " + obj.getAppletID() + " is not registered in this JVM instance"));
        }
    }

    public static synchronized void releaseRemoteObject(RemoteJavaObject object) {
        Integer key = new Integer(object.getObjectID());
        Object realObject = objectIDMap.remove(key);
        if (realObject != null) {
            exportedObjectMap.remove(realObject);
        }
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    //////////////////////////////////////////////////////
    //                                                  //
    // Methods associated with Java -> JavaScript calls //
    //                                                  //
    //////////////////////////////////////////////////////

    // Reference counting mechanism for browser-side JavaScript
    // objects exposed to this JVM. For each unique BrowserSideObject
    // we maintain a reference count, initialized to 1, incremented
    // every time we import another reference to it (and convert it to
    // a MessagePassingJSObject), and decremented when the
    // MessagePassingJSObject instances which refer to them disappear.
    private static ReferenceQueue queue = new ReferenceQueue();
    private static volatile boolean shouldStop;
    private static Thread cleanupThread;

    // We maintain a separate reference count for each applet in which
    // a given browser-side object shows up, because on the other side
    // we track in which applets we have exposed a given JavaScript object.
    private static class BrowserSideObjectKey {
        private BrowserSideObject object;
        private int appletID;

        public BrowserSideObjectKey(BrowserSideObject object,
                                    int appletID) {
            this.object = object;
            this.appletID = appletID;
        }

        public BrowserSideObject getObject()   { return object;   }
        public int               getAppletID() { return appletID; }

        public int hashCode() {
            return object.hashCode() ^ appletID;
        }

        public boolean equals(Object arg) {
            if (arg == null || arg.getClass() != getClass())
                return false;
            BrowserSideObjectKey other = (BrowserSideObjectKey) arg;
            return (object.equals(other.object) && appletID == other.appletID);
        }
    }

    private static class BrowserSideObjectCleanupThread extends Thread {
        public BrowserSideObjectCleanupThread() {
            super("Browser Side Object Cleanup Thread");
        }

        public void run() {
            while (!shouldStop) {
                try {
                    BrowserSideObjectReference obj = (BrowserSideObjectReference) queue.remove();
                    unref(obj.getObjectKey());
                } catch (IOException e) {
                    // Assume the pipe died and we should shut down
                    return;
                } catch (Exception e) {
                    // Probably an InterruptedException; assume we
                    // should continue if some other unexpected
                    // exception occurred
                    e.printStackTrace();
                }
            }
        }
    }

    private static class BrowserSideObjectReference extends PhantomReference {
        private BrowserSideObjectKey objectKey;

        public BrowserSideObjectReference(Object referent,
                                          ReferenceQueue q,
                                          BrowserSideObjectKey objectKey) {
            super(referent, q);
            this.objectKey = objectKey;
        }

        public BrowserSideObjectKey getObjectKey() {
            return objectKey;
        }
    }

    private static class ReferenceCount {
        private int count;

        ReferenceCount() {
            count = 0;
        }

        public void ref() {
            ++count;
        }

        public int unref() {
            --count;
            return count;
        }
    }

    private static Map/*<BrowserSideObjectKey, ReferenceCount>*/ refCounts = new HashMap();
    private static synchronized void ref(BrowserSideObject obj, int appletID) {
        BrowserSideObjectKey key = new BrowserSideObjectKey(obj, appletID);
        ReferenceCount refCount = (ReferenceCount) refCounts.get(key);
        if (refCount == null) {
            refCount = new ReferenceCount();
            refCounts.put(key, refCount);
        }
        refCount.ref();
    }

    private static void unref(BrowserSideObjectKey key) throws IOException {
        boolean deleted = false;
        synchronized(LiveConnectSupport.class) {
            ReferenceCount refCount = (ReferenceCount) refCounts.get(key);
            if (refCount == null) {
                // FIXME: unknown whether this can happen
                return;
            }
            if (refCount.unref() == 0) {
                refCounts.remove(key);
                deleted = true;
            }
        }

        if (deleted) {
            pipe.send(new JavaScriptReleaseObjectMessage(null, key.getObject(), key.getAppletID()));
        }
    }

    private static void track(MessagePassingJSObject obj) {
        new BrowserSideObjectReference(obj, queue,
                                       new BrowserSideObjectKey(obj.getBrowserSideObject(),
                                                                obj.getAppletID()));
    }

    private static void track2(MessagePassingOneWayJSObject obj) {
        new BrowserSideObjectReference(obj, queue,
            new BrowserSideObjectKey(obj.getBrowserSideObject(),
            obj.getAppletID()));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                       //
    // Mechanism for exporting Java objects from this JVM, used for JavaScript -> Java calls //
    //                                                                                       //
    ///////////////////////////////////////////////////////////////////////////////////////////

    private static Map/*<Object, RemoteJavaObject>*/ exportedObjectMap = new IdentityHashMap();
    private static Map/*<Integer, Object>*/ objectIDMap                = new HashMap();
    private static int nextObjectID;
    private static synchronized RemoteJavaObject exportRemoteObject(Object object, int appletID, boolean isApplet) {
        // See if this object is already in the table
        RemoteJavaObject remote = (RemoteJavaObject) exportedObjectMap.get(object);
        if (remote != null) {
            // Check this object to make sure its applet instance is still running;
            // otherwise, return a fresh reference to it.
            // This basically should only happen with access to static fields;
            // it's possible for erroneous JavaScript code to be written which
            // holds on to objects past their applet's lifetime, which we detect
            // and reject on the server side.
            if (!isAppletRunning(remote.getAppletID())) {
                releaseRemoteObject(remote);
                remote = null;
            }
        }
        if (remote == null) {
            int objectID = ++nextObjectID;
            remote = new RemoteJavaObject(jvmID, appletID, objectID, isApplet);
            exportedObjectMap.put(object, remote);
            objectIDMap.put(new Integer(objectID), object);
        }
        return remote;
    }

    private static synchronized Object importRemoteObject(RemoteJavaObject object) {
        return objectIDMap.get(new Integer(object.getObjectID()));
    }

    private static synchronized void releaseRemoteObjects(int appletID) {
        List/*<RemoteJavaObject>*/ objectsToRelease = new ArrayList();
        for (Iterator iter = exportedObjectMap.values().iterator(); iter.hasNext(); ) {
            RemoteJavaObject obj = (RemoteJavaObject) iter.next();
            if (obj.getAppletID() == appletID) {
                objectsToRelease.add(obj);
            }
        }
        for (Iterator iter = objectsToRelease.iterator(); iter.hasNext(); ) {
            releaseRemoteObject((RemoteJavaObject) iter.next());
        }
    }

    //////////////////////////////////////////////////////
    //                                                  //
    // Methods associated with JavaScript -> Java calls //
    //                                                  //
    //////////////////////////////////////////////////////

    // Maps applet IDs to PerAppletInfo instances
    private static Map/*<Integer, PerAppletInfo>*/ appletInfoMap = new HashMap();
    private static synchronized PerAppletInfo getInfo(int appletID) {
        return (PerAppletInfo) appletInfoMap.get(new Integer(appletID));
    }
    private static boolean isAppletRunning(int appletID) {
        return getInfo(appletID) != null;
    }

    // For com.sun.java.browser.plugin2.liveconnect.v1.BridgeFactory
    public static synchronized Bridge getBridge(Object applet) {
        for (Iterator iter = appletInfoMap.values().iterator(); iter.hasNext(); ) {
            PerAppletInfo info = (PerAppletInfo) iter.next();
            if (info.hostsApplet(applet)) {
                return info.getBridge();
            }
        }
        return null;
    }

    // Prevents user code from being able to induce significant memory leaks
    private static class BridgeImpl implements Bridge {
        private volatile PerAppletInfo info;

        private BridgeImpl(PerAppletInfo info) {
            this.info = info;
        }

        public void register(InvocationDelegate delegate) {
            getInfo().register(delegate);
        }

        public void unregister(InvocationDelegate delegate) {
            getInfo().unregister(delegate);
        }

        public void register(ConversionDelegate delegate) {
            getInfo().register(delegate);
        }

        public void unregister(ConversionDelegate delegate) {
            getInfo().unregister(delegate);
        }

        public int conversionCost(final Object object, final Object toType) {
            return getInfo().conversionCost(object, toType);
        }

        public Object convert(final Object object, final Object toType) throws Exception {
            return getInfo().convert(object, toType);
        }

        public void stop() {
            info = null;
        }

        //----------------------------------------------------------------------
        // Internals only below this point
        //

        private PerAppletInfo getInfo() {
            PerAppletInfo p = info;
            if (p == null) {
                throw new IllegalStateException("Applet has already terminated");
            }
            return p;
        }
    }

    private static class PerAppletInfo {
        private int appletID;
        private Plugin2Manager manager;
        private LiveConnectWorker worker;
        private BridgeImpl bridge;
        private boolean fetchedDocumentBase;
        private URL documentBase;
        private AccessControlContext context;
        private volatile boolean notifiedOfStart;
	private boolean liveconnectChecked = false;
	private boolean liveconnectPermissionGranted = false;

        // The invocation and conversion delegates that are registered
        // for this applet; this provides multi-language support to
        // the JavaScript bridge
        //
        // NOTE (FIXME) that the synchronzation here is inadequate in
        // the general case where we are iterating down the lists;
        // need an efficient mechanism -- not involving copying the
        // whole list every invocation, and not involving
        // synchronizing around all of the iterations down the list
        private List/*<InvocationDelegate>*/ invocationDelegates =
            Collections.synchronizedList(new ArrayList/*<InvocationDelegate>*/());
        private List/*<ConversionDelegate>*/ conversionDelegates =
            Collections.synchronizedList(new ArrayList/*<ConversionDelegate>*/());

        // The set of classes we've operated upon so far in the
        // context of this applet. We maintain this on a per-applet
        // basis to avoid memory leaks.
        private Map/*<Class, JavaClass>*/ classes = new HashMap();

        // The set of names known *not* to map to classes -- needed
        // for performance reasons
	private Set/*<String>*/ notJavaClasses = new HashSet();

	// HashMap for storing user's response to the liveconnect security warning dialog
	private static Map/*<String, Boolean>*/ appletLiveconnectAllowedMap = 
	    Collections.synchronizedMap(new HashMap());

        public PerAppletInfo(int appletID, Plugin2Manager manager) {
            this.appletID = appletID;
            this.manager = manager;
            bridge = new BridgeImpl(this);
            register(new DefaultInvocationDelegate());
            register(new DefaultConversionDelegate());
            worker = new LiveConnectWorker();
            manager.startWorkerThread("Applet " + appletID + " LiveConnect Worker Thread",
                                      worker);
        }

        public boolean hostsApplet(Object applet) {
            return (manager != null &&
                    manager.getApplet() == applet);
        }

        //----------------------------------------------------------------------
        // Bridge support
        //

        public Bridge getBridge() {
            return bridge;
        }

        public void register(InvocationDelegate delegate) {
            // Add at the beginning to keep the default delegate as the last one
            invocationDelegates.add(0, delegate);
        }

        public void unregister(InvocationDelegate delegate) {
            invocationDelegates.remove(delegate);
        }

        public void register(ConversionDelegate delegate) {
            // Add at the beginning to keep the default delegate as the last one
            conversionDelegates.add(0, delegate);
        }

        public void unregister(ConversionDelegate delegate) {
            conversionDelegates.remove(delegate);
        }

        // NOTE: the code in this analysis loop should match the code
        // in convert(), below; in particular, it must not report a
        // match if convert() can't actually convert one of the
        // arguments
        public int conversionCost(final Object object, final Object toType) {
            final int[] resultBox = new int[1];
            AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        for (Iterator iter = conversionDelegates.iterator(); iter.hasNext(); ) {
                            ConversionDelegate delegate = (ConversionDelegate) iter.next();
                            int cost = delegate.conversionCost(object, toType);
                            if (cost >= 0) {
                                resultBox[0] = cost;
                                return null;
                            }
                        }
                        resultBox[0] = -1;
                        return null;
                    }
                }, getContext());
            return resultBox[0];
        }

        public Object convert(final Object object, final Object toType) throws Exception {
            return AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws Exception {
                        Object[] resultBox = new Object[1];
                        for (Iterator iter = conversionDelegates.iterator(); iter.hasNext(); ) {
                            ConversionDelegate delegate = (ConversionDelegate) iter.next();
                            if (delegate.convert(object, toType, resultBox)) {
                                return resultBox[0];
                            }
                        }
                        throw inconvertible(object, toType);
                    }
                }, getContext());
        }

        //
        //----------------------------------------------------------------------


        public void enqueue(JavaObjectOpMessage msg) {
            worker.enqueue(msg);
        }

        public void stop() {
            bridge.stop();
            worker.stop();
            releaseRemoteObjects(appletID);
        }

        public void doObjectOp(final JavaObjectOpMessage msg) throws IOException {
            JavaReplyMessage reply = null;
            if (DEBUG) {
                System.out.println("LiveConnectSupport: " + getOpName(msg.getOperationKind()) + " \"" + msg.getMemberName() + "\"");
            }
	    if (!isLiveconnectCallAllowed(msg)) {
		RemoteJavaObject obj = msg.getObject();
		reply = new JavaReplyMessage(msg.getConversation(),
			                     msg.getResultID(),
			                     null,
			                     false,
			                     "Liveconnect call for Applet ID " + obj.getAppletID() + 
					         " is not allowed in this JVM instance");
		pipe.send(reply);
		return;
	    }
            try {
                waitForAppletStartOrError();
                if (manager.hasErrorOccurred()) {
                    if (manager.getErrorMessage() != null) {
                        throw new RuntimeException(manager.getErrorMessage());
                    } else if (manager.getErrorException() != null) {
                        throw (IOException) new IOException().initCause(manager.getErrorException());
                    }
                }

                final Object target = importObject(msg.getObject(), appletID);
                final Object[] args = msg.getArguments();
                if (args != null) {
                    for (int i = 0; i < args.length; i++) {
                        args[i] = importObject(args[i], appletID);
                    }
                }
                Result result = null;
                final boolean isApplet = msg.getObject().isApplet();
                boolean resultIsVoid = false;
                switch (msg.getOperationKind()) {
                    case JavaObjectOpMessage.CALL_METHOD: {
                        result = (Result) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                                public Object run() throws Exception {
                                    Result[] resultBox = new Result[1];
                                    for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                                        InvocationDelegate delegate = (InvocationDelegate) iter.next();
                                        if (delegate.invoke(msg.getMemberName(),
                                                            target,
                                                            args,
                                                            false,
                                                            isApplet,
                                                            resultBox)) {
                                            break;
                                        }
                                    }
                                    return resultBox[0];
                                }
                            }, getContext());
                        if (result.value() == Void.TYPE) {
                            resultIsVoid = true;
                            result = null;
                        }
                        break;
                    }

                    case JavaObjectOpMessage.GET_FIELD: {
                        result = (Result) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                                public Object run() throws Exception {
                                    Result[] resultBox = new Result[1];
                                    // Support for the scoped "Packages" keyword
                                    if (isApplet && "Packages".equals(msg.getMemberName())) {
                                        resultBox[0] = new Result(new JavaNameSpace(""), false);
                                    } else {
                                        for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                                            InvocationDelegate delegate = (InvocationDelegate) iter.next();
                                            if (delegate.getField(msg.getMemberName(),
                                                                  target,
                                                                  false,
                                                                  isApplet,
                                                                  resultBox)) {
                                                break;
                                            }
                                        }
                                    }
                                    return resultBox[0];
                                }
                            }, getContext());
                        break;
                    }

                    case JavaObjectOpMessage.SET_FIELD: {
                        AccessController.doPrivileged(new PrivilegedExceptionAction() {
                                public Object run() throws Exception {
                                    for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                                        InvocationDelegate delegate = (InvocationDelegate) iter.next();
                                        if (delegate.setField(msg.getMemberName(),
                                                              target,
                                                              args[0],
                                                              false,
                                                              isApplet)) {
                                            break;
                                        }
                                    }
                                    return null;
                                }
                            }, getContext());
                        resultIsVoid = true;
                        break;
                    }

                    case JavaObjectOpMessage.HAS_FIELD: {
                        result = (Result) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                                public Object run() throws Exception {
                                    boolean[] resultBox = new boolean[1];
                                    boolean handled = false;
                                    if (isApplet) {
                                        // Support for the scoped "Packages" keyword
                                        if ("Packages".equals(msg.getMemberName())) {
                                            resultBox[0] = true;
                                            handled = true;
                                        }

                                        // Deny that we have either the "width" or "height" properties
                                        // (which we actually do, through the ImageObserver interface)
                                        // so that the Firefox 3 browser will handle them and mutate
                                        // the DOM element instead of us
                                        String lowerName = msg.getMemberName().toLowerCase();
                                        if (lowerName.equals("width") ||
                                            lowerName.equals("height")) {
                                            resultBox[0] = false;
                                            handled = true;
                                        }
                                    }
                                    if (!handled) {
                                        for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                                            InvocationDelegate delegate = (InvocationDelegate) iter.next();
                                            if (delegate.hasField(msg.getMemberName(),
                                                                  target,
                                                                  false,
                                                                  isApplet,
                                                                  resultBox)) {
                                                break;
                                            }
                                        }
                                    }
                                    return (resultBox[0] ? new Result(Boolean.TRUE, false) : new Result(Boolean.FALSE, false));
                                }
                            }, getContext());
                        break;
                    }

                    case JavaObjectOpMessage.HAS_METHOD: {
                        result = (Result) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                                public Object run() throws Exception {
                                    boolean[] resultBox = new boolean[1];
                                    for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                                        InvocationDelegate delegate = (InvocationDelegate) iter.next();
                                        if (delegate.hasMethod(msg.getMemberName(),
                                                               target,
                                                               false,
                                                               isApplet,
                                                               resultBox)) {
                                            break;
                                        }
                                    }
                                    return (resultBox[0] ? new Result(Boolean.TRUE, false) : new Result(Boolean.FALSE, false));
                                }
                            }, getContext());
                        break;
                    }

		case JavaObjectOpMessage.HAS_FIELD_OR_METHOD: {
		    result = (Result) AccessController.doPrivileged(new PrivilegedExceptionAction() {
			    public Object run() throws Exception {
				boolean[] resultBox = new boolean[1];
				boolean handled = false;
				
				if (isApplet) {
				    // Support for the scoped "Packages" keyword
				    if ("Packages".equals(msg.getMemberName())) {
					resultBox[0] = true;
					handled = true;
				    }
				}

				if (!handled) {
				    for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
					InvocationDelegate delegate = (InvocationDelegate) iter.next();
					if (delegate.hasFieldOrMethod(msg.getMemberName(),
								      target,
								      false,
								      isApplet,
								      resultBox)) {
					    break;
					}
				    }
				}
				return (resultBox[0] ? new Result(Boolean.TRUE, false) : new Result(Boolean.FALSE, false));
			    }
			}, getContext());
		    break;
		} 
		    
		default:
		    // This is certainly our fault
		    throw new RuntimeException("Internal error: unknown Java object operation " + msg.getOperationKind());
                }
                // Successful operation; return any result
                Object val = null;
                boolean skipUnboxing = false;
                if (result != null) {
                    val = result.value();
                    skipUnboxing = result.skipUnboxing();
                }
                reply = new JavaReplyMessage(msg.getConversation(),
                                             msg.getResultID(),
                                             exportObject(val, appletID, skipUnboxing, false),
                                             resultIsVoid,
                                             null);
                if (DEBUG) {
                    System.out.println("LiveConnectSupport: " + getOpName(msg.getOperationKind()) + " \"" + msg.getMemberName() +
                                       "\": returning result " + val);
                }
            } catch (Throwable e) {
                if (DEBUG) {
                    System.out.println("Exception occurred during " + getOpName(msg.getOperationKind()) + " " + msg.getMemberName() + ":");
                    e.printStackTrace();
                }

                // Exception occurred: describe it to the caller
                // Unwrap exceptions to provide better error messages
                if (e instanceof PrivilegedActionException) {
                    e = ((PrivilegedActionException) e).getException();
                }

                if (e instanceof InvocationTargetException) {
                    Throwable t = ((InvocationTargetException) e).getTargetException();
                    if (t instanceof Exception) {
                        e = (Exception) t;
                    }
                }

                String exceptionMessage = e.toString();
                reply = new JavaReplyMessage(msg.getConversation(),
                                             msg.getResultID(),
                                             null,
                                             false,
                                             exceptionMessage);
            }
            pipe.send(reply);
        }


	private boolean isLiveconnectCallAllowed(JavaObjectOpMessage msg) {
	    Plugin2Manager mgr = this.manager;
	    boolean liveconnectAllowed = false;
	    if (mgr != null) {
		boolean isSecureVM = mgr.isVMSecure();
		if (!isSecureVM) {
		    Boolean userResponse = null;
		    if (this.liveconnectChecked) {
			userResponse = new Boolean(this.liveconnectPermissionGranted);
		    } else {
			String appletKey = mgr.getAppletUniqueKey();
			userResponse =
			    (Boolean) appletLiveconnectAllowedMap.get(appletKey);
			if (userResponse == null) {
			    userResponse = new Boolean(getUserPermissionForLiveconnectCall((JavaObjectOpMessage) msg));
			    appletLiveconnectAllowedMap.put(appletKey, userResponse);
			    this.liveconnectChecked = true;
			    this.liveconnectPermissionGranted = userResponse.booleanValue();
			}
		    }
		    liveconnectAllowed = userResponse.booleanValue();
		} else {
		    liveconnectAllowed = true;
		}
	    }
	    return liveconnectAllowed;
	}

	private boolean getUserPermissionForLiveconnectCall(JavaObjectOpMessage msg) {

	    String title = ResourceManager.getString("javaws.ssv.title");

	    String message = ResourceManager.getString("liveconnect.insecurejvm.warning");

	    String cont = ResourceManager.getString("common.continue_btn");

	    String cancel = ResourceManager.getString("common.cancel_btn");

	    final URL docBase = this.getDocumentBase();
	    String name = this.manager.getName();

	    AppInfo ai = new AppInfo();
	    ai.setTitle(name);
	    ai.setFrom(docBase);
        
	    int result = UIFactory.ERROR;
        
	    // the AppInfo (ai) is required so that the UIFactory won't 
	    // insert unnecessary string (such as "Name:") in the center
	    // pane of the dialog
	    result = UIFactory.showConfirmDialog(null, ai,
		message, null, title, cont, cancel, true);

	    return ((result == UIFactory.OK) ? true : false);
	}

        private AccessControlContext getContext() {
            if (context == null) {
                // NOTE: this is fundamentally what models incoming
                // (untrusted) JavaScript calls as though they were
                // coming from an unsigned applet coming from the
                // document base
                context = createContext(getDocumentBase());
            }
            return context;
        }

        private URL getDocumentBase() {
            if (!fetchedDocumentBase) {
                documentBase = manager.getDocumentBase();
                fetchedDocumentBase = true;
            }
            
            return documentBase;
        }

        private InvocationDelegate getDelegate(Object o, boolean isStatic) {
            if (o instanceof JavaNameSpace) {
                return new JavaNameSpaceDelegate();
            }

            if (isStatic) {
                return getJavaClass((Class) o);
            } else {
                return getJavaClass(o.getClass());
            }
        }

        private synchronized JavaClass getJavaClass(Class c) {
            JavaClass clazz = (JavaClass) classes.get(c);
            if (clazz == null) {
                clazz = new JavaClass(c, getBridge());
                classes.put(c, clazz);
            }
            return clazz;
        }

        private void waitForAppletStartOrError() throws IOException {
            // In similar fashion to the old Java Plug-In, this method
            // blocks JavaScript -> Java calls until the applet is
            // either successfully started, an error occurred (in
            // which case an error is reported back to the browser),
            // or a Java-to-JavaScript call is initiated, in which
            // case round-trip calls are allowed.
            if (manager.getApplet() != null ||
                manager.hasErrorOccurred() ) {
                manager.waitUntilAppletStartDone();
                return;
            }

            // So-called "dummy" applets, used only in Firefox for
            // support of the legacy "java" and other keywords, are
            // always eligible for LiveConnect calls
            if (manager.isForDummyApplet()) {
                return;
            }

            // Should not happen, since LC messages are passed _only_
            // after appletLoaded from the server side.
            // This is also the case for Java->JS,
            // since the earliest time they can be issued is
            // from the user init method.
            throw new IOException("LiveConnect operation without existing applet");
        }

        // This is the default InvocationDelegate, handling
        // invocations against Java objects and classes, and
        // delegating down to class-specific instances
        class DefaultInvocationDelegate implements InvocationDelegate {
            public boolean invoke(String methodName,
                                  Object receiver,
                                  Object[] arguments,
                                  boolean isStatic,
                                  boolean objectIsApplet,
                                  Result[] result) throws Exception {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.invoke(methodName,
                                       isStatic ? null : receiver,
                                       arguments,
                                       false,
                                       objectIsApplet,
                                       result);
            }

            public boolean getField(String fieldName,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    Result[] result) throws Exception {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.getField(fieldName,
                                         isStatic ? null : receiver,
                                         false,
                                         objectIsApplet,
                                         result);
            }

            public boolean setField(String fieldName,
                                    Object receiver,
                                    Object value,
                                    boolean isStatic,
                                    boolean objectIsApplet) throws Exception {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.setField(fieldName,
                                         isStatic ? null : receiver,
                                         value,
                                         false,
                                         objectIsApplet);
            }

            public boolean hasField(String fieldName,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    boolean[] result) {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.hasField(fieldName,
                                         isStatic ? null : receiver,
                                         false,
                                         objectIsApplet,
                                         result);
            }

            public boolean hasMethod(String methodName,
                                     Object receiver,
                                     boolean isStatic,
                                     boolean objectIsApplet,
                                     boolean[] result) {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.hasMethod(methodName,
                                          isStatic ? null : receiver,
                                          false,
                                          objectIsApplet,
                                          result);
            }

            public boolean hasFieldOrMethod(String name,
                                            Object receiver,
                                            boolean isStatic,
                                            boolean objectIsApplet,
                                            boolean[] result) {
                InvocationDelegate delegate = getDelegate(receiver, isStatic);
                return delegate.hasFieldOrMethod(name,
                                                 isStatic ? null : receiver,
                                                 false,
                                                 objectIsApplet,
                                                 result);
            }

            public Object findClass(String name) {
                try {
                    ClassLoader cl = Thread.currentThread().getContextClassLoader();
                    return Class.forName(name, false, cl);
                } catch (ClassFormatError e) {
                    // This happens if the web server returns
                    // malformed 404 replies for queries like
                    // "java.class"
                    return null;
                } catch (ClassNotFoundException e) {
                    return null;
                } catch (RuntimeException e) {
                    // This really shouldn't happen but seems to in some
                    // cases, apparently due to bugs in the deployment cache
                    throw (e);
                }
            }

            public Object newInstance(Object clazz,
                                      Object[] arguments) throws Exception {
                InvocationDelegate delegate = getJavaClass((Class) clazz);
                return delegate.newInstance(null, arguments);
            }
        }

        // This is the default conversion delegate, providing support
        // for narrowing numeric values, converting to and from
        // Strings, converting JavaScript arrays to Java arrays, and
        // other conversions
        class DefaultConversionDelegate implements ConversionDelegate {
            private Class jsObjectClass = JSObject.class;

            // Support for conversion of arbitrary Objects to Strings
            // This has a higher conversion cost than normal conversions
            // Note that this basically assumes the maximum number of incoming
            // arguments is this value; could be smarter, but little benefit
            private static final int TOSTRING_CONVERSION_PENALTY = 50;
            // Support for conversion of JSObjects into Strings and arrays
            // We prefer to do this as a last resort
            private static final int JSOBJECT_CONVERSION_PENALTY =
                TOSTRING_CONVERSION_PENALTY * TOSTRING_CONVERSION_PENALTY;

            public int conversionCost(Object arg, Object toType) {
                // Some conversion delegates may represent types in different ways
                if (!(toType instanceof Class)) {
                    return -1;
                }

                Class expectedClass = (Class) toType;
                if (arg == null) {
                    // null arguments can not undergo unboxing conversions
                    if (expectedClass.isPrimitive()) {
                        return -1;
                    }
                    return 0;
                }
                Class argClass = arg.getClass();

                // Since all JSObject instances handed back to the applet are subclasses of JSObject,
                // special-case these to not consider them to add to the number of conversions
                if ((argClass == expectedClass) ||
                    (expectedClass == jsObjectClass && expectedClass.isAssignableFrom(argClass))) {
                    return 0;  // Perfect match
                }

                if (expectedClass.isAssignableFrom(argClass)) {
                    // NOTE: should really use a better algorithm
                    // here; this has been problematic
                    return conversionDistance(argClass, expectedClass);
                }

                if (jsObjectClass.isAssignableFrom(argClass) && canConvert((JSObject) arg, expectedClass)) {
                    // JSObject conversions have higher cost than normal conversions
                    return JSOBJECT_CONVERSION_PENALTY;
                }

                // Any other conversion between JSObject and other types are forbidden
                if (jsObjectClass.isAssignableFrom(argClass) || jsObjectClass.isAssignableFrom(expectedClass)) {
                    return -1;
                }

                if (expectedClass == String.class) {
                    return TOSTRING_CONVERSION_PENALTY; // Any object can be converted to String
                }

                if (expectedClass.isPrimitive()) {
                    expectedClass = getBoxingClass(expectedClass);
                }
                if (Number.class.isAssignableFrom(expectedClass)
                    || expectedClass == Character.class || expectedClass == Boolean.class) {
                    if (expectedClass == argClass) {
                        return 0;
                    }

                    if (argClass == String.class ||
                        Number.class.isAssignableFrom(argClass) ||
                        argClass == Character.class ||
                        argClass == Boolean.class) {
                        return 1;
                    }
                }

                // Not convertible
                return -1;
            }

            // Performs the data type conversions needed for calling from
            // JavaScript to Java, including narrowing conversions for
            // primitive types and conversions of Strings to numbers
            public boolean convert(Object obj, Object toType, Object[] result) throws Exception {
                if (obj == null)
                    return true;

                if (!(toType instanceof Class)) {
                    // Some other delegate should have handled this; we can't
                    throw inconvertible(obj, toType);
                }

                Class targetClass = (Class) toType;
                Class objClass = obj.getClass();

                // If the type is compatible according to reflection, no conversion needed
                if (targetClass.isAssignableFrom(objClass)) {
                    result[0] = obj;
                    return true;
                }

                if (targetClass == java.lang.String.class) {
                    if(obj instanceof Number) {
                        NumberFormat nf = NumberFormat.getNumberInstance();
                        try {
                            result[0] = nf.parse(obj.toString()).toString();
                            return true;
                        }catch(ParseException pexc) {
                            //Ignore and return the value of toString()
                        }
                    }

                    result[0] = obj.toString();
                    return true;
                }

                // Support for converting JavaScript objects into Java arrays
                if (jsObjectClass.isAssignableFrom(objClass) && targetClass.isArray()) {
                    try {
                        JSObject jsObj = (JSObject) obj;
                        Class componentType = targetClass.getComponentType();
                        int length = ((Number) jsObj.getMember("length")).intValue();
                        Object res = Array.newInstance(componentType, length);
                        Object[] tmp = new Object[1];
                        for (int i = 0; i < length; i++) {
                            Object element = null;
                            try {
                                element = jsObj.getSlot(i);
                            } catch (JSException e) {
                                // Support sparse JavaScript arrays
                            }
                            if (element != null) {
                                convert(element, componentType, tmp);
                                Array.set(res, i, tmp[0]);
                            }
                        }
                        result[0] = res;
                        return true;
                    } catch (Exception e) {
                        throw inconvertible(objClass, targetClass, e);
                    }
                }

                // Other conversions to / from JSObject are not supported
                if (jsObjectClass.isAssignableFrom(objClass) || jsObjectClass.isAssignableFrom(targetClass))
                    throw inconvertible(objClass, targetClass);

                if (targetClass.isPrimitive() || Number.class.isAssignableFrom(targetClass)
                    || targetClass == Character.class || targetClass == Boolean.class) {
                    boolean isNumber = obj instanceof Number;
            
                    if (!isNumber && !(obj instanceof String) &&
                        !(obj instanceof Character) && !(obj instanceof Boolean)) {
                        throw inconvertible(objClass, targetClass);
                    }

                    // Conversions for primitive types
                    if (targetClass == Boolean.TYPE || targetClass == Boolean.class) {
                        if (objClass == Boolean.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            // Conversion as per Core JavaScript Guide 1.5
                            // FIXME: intermediate conversion to double may be a mistake
                            double d = ((Number) obj).doubleValue();
                            if (Double.isNaN(d) || d == 0) {
                                result[0] = Boolean.FALSE;
                            } else {
                                result[0] = Boolean.TRUE;
                            }
                            return true;
                        } else {
                            if (((String) obj).length() == 0) {
                                result[0] = Boolean.FALSE;
                            } else {
                                result[0] = Boolean.TRUE;
                            }
                            return true;
                        }
                    }

                    if (targetClass == Byte.TYPE || targetClass == Byte.class) {
                        if (objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Byte(((Number) obj).byteValue());
                        } else {
                            result[0] = Byte.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Short.TYPE || targetClass == Short.class) {
                        // Widening conversions are supported by reflection
                        if (objClass == Short.class ||
                            objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Short(((Number) obj).shortValue());
                        } else {
                            result[0] = Short.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Integer.TYPE || targetClass == Integer.class) {
                        // Widening conversions are supported by reflection
                        if (objClass == Integer.class ||
                            objClass == Character.class ||
                            objClass == Short.class ||
                            objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Integer(((Number) obj).intValue());
                        } else {
                            result[0] = Integer.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Long.TYPE || targetClass == Long.class) {
                        // Widening conversions are supported by reflection
                        if (objClass == Long.class ||
                            objClass == Integer.class ||
                            objClass == Character.class ||
                            objClass == Short.class ||
                            objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Long(((Number) obj).longValue());
                        } else {
                            result[0] = Long.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Float.TYPE || targetClass == Float.class) {
                        // Widening conversions are supported by reflection
                        if (objClass == Float.class ||
                            objClass == Long.class ||
                            objClass == Integer.class ||
                            objClass == Character.class ||
                            objClass == Short.class ||
                            objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Float(((Number) obj).floatValue());
                        } else {
                            result[0] = Float.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Double.TYPE || targetClass == Double.class) {
                        // Widening conversions are supported by reflection
                        if (objClass == Double.class ||
                            objClass == Float.class ||
                            objClass == Long.class ||
                            objClass == Integer.class ||
                            objClass == Character.class ||
                            objClass == Short.class ||
                            objClass == Byte.class) {
                            result[0] = obj;
                            return true;
                        }

                        if (isNumber) {
                            result[0] = new Double(((Number) obj).doubleValue());
                        } else {
                            result[0] = Double.valueOf((String) obj);
                        }
                        return true;
                    }

                    if (targetClass == Character.TYPE || targetClass == Character.class) {
                        if (isNumber) {
                            result[0] = new Character((char) ((Number) obj).shortValue());
                        } else {
                            result[0] = new Character((char) (Short.decode((String) obj).shortValue()));
                        }
                        return true;
                    }

                    // Should not reach here
                }

                throw inconvertible(objClass, targetClass);
            }

            // Indicates whether a JSObject can be converted to the target type
            private boolean canConvert(JSObject obj, Class targetClass) {
                if (targetClass == String.class)
                    return true;

                if (targetClass.isArray()) {
                    // See whether we have a chance of converting it; note
                    // that this is a risky conversion because it implies a
                    // lot of work that might go wrong (and we don't want to
                    // do it twice)
                    try {
                        obj.getMember("length");
                        return true;
                    } catch (JSException e) {
                        // Fall through
                    }
                }

                return false;
            }

            private int conversionDistance(Class argumentClass, Class expectedClass) {
                // Want classes closer in the hierarchy to match preferentially.
                // For example, calling:
                //   JFrame f = ...;
                //   new JDialog(f);
                // should match JDialog(Frame) instead of JDialog(Window).

                if (expectedClass.isInterface() || expectedClass.isArray()) {
                    // Should do better here
                    return 1;
                }

                int distance = 0;
                while (argumentClass != null &&
                       argumentClass != expectedClass) {
                    ++distance;
                    argumentClass = argumentClass.getSuperclass();
                }
        
                if (argumentClass != expectedClass) {
                    // Don't know what's going on
                    return 1;
                }
                return distance;
            }

            private Class getBoxingClass(Class primitiveClass) {
                if (primitiveClass == Boolean.TYPE)
                    return Boolean.class;
                if (primitiveClass == Byte.TYPE)
                    return Byte.class;
                if (primitiveClass == Short.TYPE)
                    return Short.class;
                if (primitiveClass == Character.TYPE)
                    return Character.class;
                if (primitiveClass == Integer.TYPE)
                    return Integer.class;
                if (primitiveClass == Long.TYPE)
                    return Long.class;
                if (primitiveClass == Float.TYPE)
                    return Float.class;
                if (primitiveClass == Double.TYPE)
                    return Double.class;
                throw new IllegalArgumentException("Not a primitive type class");
            }
        }

        private static IllegalArgumentException inconvertible(Object object, Object toType) {
            return new IllegalArgumentException("Object " + object +
                                                " can not be converted to " + toType);
        }

        private static IllegalArgumentException inconvertible(Class objectClass, Class targetClass) {
            return inconvertible(objectClass, targetClass, null);
        }

        private static IllegalArgumentException inconvertible(Class objectClass, Class targetClass, Exception cause) {
            IllegalArgumentException exc = 
                new IllegalArgumentException("Class " + objectClass.getName() +
                                             " can not be converted to " + targetClass.getName());
            if (cause != null) {
                exc.initCause(cause);
            }
            return exc;
        }

        // This InvocationDelegate implementation provides access to Java
        // classes: calling static methods and creation of new objects.
        // It is placed here so the class lookups can hit in the JavaClass
        // cache.
        class JavaNameSpaceDelegate implements InvocationDelegate {
            class FindClassResult {
                Object clazz;
                InvocationDelegate delegate;
                FindClassResult(Object clazz, InvocationDelegate delegate) {
                    this.clazz = clazz;
                    this.delegate = delegate;
                }
            }

            private FindClassResult findClassFromDelegates(String name) {
                if (name == null || name.equals(""))
                    return null;

                if (notJavaClasses.contains(name)) {
                    return null;
                }

                FindClassResult fcr = null;
                try {
                    Object result = null;
                    InvocationDelegate provider = null;
                    for (Iterator iter = invocationDelegates.iterator(); iter.hasNext(); ) {
                        InvocationDelegate delegate = (InvocationDelegate) iter.next();
                        try {
                            if ((result = delegate.findClass(name)) != null) {
                                provider = delegate;
                                break;
                            }
                        } catch (Exception e) {
                        }
                    }
                    if (result == null) {
                        return null;
                    }
                    fcr = new FindClassResult(result, provider);
                } finally {
                    if (fcr == null) {
                        notJavaClasses.add(name);
                    }
                }
                return fcr;
            }

            private RuntimeException fail(String nameSpaceName, String detailMessage) {
                return new RuntimeException("Delegate for \"" + nameSpaceName +
                                            "\" did not handle " + detailMessage);
            }

            public boolean invoke(String methodName,
                                  Object receiver,
                                  Object[] arguments,
                                  boolean isStatic,
                                  boolean objectIsApplet,
                                  Result[] result) throws Exception {
                // Try to look up the given class
                JavaNameSpace nameSpace = (JavaNameSpace) receiver;
                final String nameSpaceName = nameSpace.getName();
                final FindClassResult fcr = findClassFromDelegates(nameSpaceName);
                if (fcr == null) {
                    // Support a synthetic toString method coming from the JavaScript engine
                    if ("toString".equalsIgnoreCase(methodName)) {
                        result[0] = new Result("[Java Package \"" + nameSpaceName + "\"]", false);
                        return true;
                    }
                    throw new ClassNotFoundException(nameSpaceName);
                }
                // If we get this far, we actually have a class
                // We might be here to invoke a static method, or we might be here
                // to instantiate this class
                if (methodName.equals("<init>")) {
                    Object obj = fcr.delegate.newInstance(fcr.clazz, arguments);
                    if (obj == null) {
                        throw fail(nameSpaceName, "invocation of constructor");
                    }
                    result[0] = new Result(obj, true);
                } else {
                    // Assume it's a static method invocation
                    if (!fcr.delegate.invoke(methodName, fcr.clazz, arguments, true, objectIsApplet, result)) {
                        throw fail(nameSpaceName, "invocation of static method \"" + methodName + "\"");
                    }
                }
                return true;
            }

            public boolean getField(String fieldName,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    Result[] result) throws Exception {
                JavaNameSpace nameSpace = (JavaNameSpace) receiver;
                final String name = nameSpace.getName();
                // See whether this corresponds to a class we know about
                final FindClassResult fcr = findClassFromDelegates(name);
                if (fcr != null) {
                    // Assume this is a fetch of a static field
                    if (!fcr.delegate.getField(fieldName, fcr.clazz, true, objectIsApplet, result)) {
                        throw fail(name, "fetch of static field \"" + fieldName + "\"");
                    }
                } else {
                    // Otherwise, assume this is a descent into the namespace
                    JavaNameSpace ns = null;
                    if (name == null || name.equals("")) {
                        ns = new JavaNameSpace(fieldName);
                    } else {
                        ns = new JavaNameSpace(name + "." + fieldName);
                    }
                    result[0] = new Result(ns, false);
                }
                return true;
            }

            public boolean setField(String fieldName,
                                    Object receiver,
                                    Object value,
                                    boolean isStatic,
                                    boolean objectIsApplet) throws Exception {
                JavaNameSpace nameSpace = (JavaNameSpace) receiver;
                final String name = nameSpace.getName();
                // See whether this corresponds to a class we know about
                final FindClassResult fcr = findClassFromDelegates(name);
                if (fcr != null) {
                    // Assume this is a set of a static field
                    if (!fcr.delegate.setField(fieldName, fcr.clazz, value, true, objectIsApplet)) {
                        throw fail(name, "fetch of static field \"" + fieldName + "\"");
                    }
                } else {
                    throw new UnsupportedOperationException("Can not perform a setField operation on a JavaNameSpace");
                }
                return true;
            }
            
            public boolean hasField(String fieldName,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    boolean[] result) {
                JavaNameSpace nameSpace = (JavaNameSpace) receiver;
                final String name = nameSpace.getName();
                // See whether this corresponds to a class we know about
                try {
                    final FindClassResult fcr = findClassFromDelegates(name);
                    if (fcr != null) {
                        // Assume this is a fetch of a static field
                        if (!fcr.delegate.hasField(fieldName, fcr.clazz, true, objectIsApplet, result)) {
                            throw fail(name, "hasField query for static field \"" + fieldName + "\"");
                        }
                    } else {
                        if ("toString".equalsIgnoreCase(fieldName)) {
                            // Assume synthetic toString operations are method calls
                            result[0] = false;
                        } else {
                            // This is a partial package name; have to always
                            // indicate to the caller that the given field exists
                            // since we don't know where in the namespace
                            // hierarchy they're going
                            result[0] = true;
                        }
                    }
                } catch (Exception e) {
                    if (DEBUG) {
                        System.out.println("Exception occurred during JavaNameSpace hasField operation:");
                        e.printStackTrace();
                    }

                    // Should not happen; findClassFromDelegates won't throw exception
                    result[0] = true;
                }
                return true;
            }


            public boolean hasMethod(String methodName,
                                     Object receiver,
                                     boolean isStatic,
                                     boolean objectIsApplet,
                                     boolean[] result) {
                JavaNameSpace nameSpace = (JavaNameSpace) receiver;
                final String name = nameSpace.getName();
                // See whether this corresponds to a class we know about
                try {
                    final FindClassResult fcr = findClassFromDelegates(name);
                    if (fcr != null) {
                        // Assume this is a call of a static method
                        if (!fcr.delegate.hasMethod(methodName, fcr.clazz, true, objectIsApplet, result)) {
                            throw fail(name, "hasMethod query for static method \"" + methodName + "\"");
                        }
                    } else {
                        if ("toString".equalsIgnoreCase(methodName)) {
                            result[0] = true;
                        } else {
                            result[0] = false;
                        }
                    }
                } catch (Exception e) {
                    if (DEBUG) {
                        System.out.println("Exception occurred during JavaNameSpace hasMethod operation:");
                        e.printStackTrace();
                    }

                    // Should not happen; findClassFromDelegates won't throw exception
                    result[0] = false;
                }
                return true;
            }

            public boolean hasFieldOrMethod(String name,
                                            Object receiver,
                                            boolean isStatic,
                                            boolean objectIsApplet,
                                            boolean[] result) {
                hasField(name, receiver, isStatic, objectIsApplet, result);
                if (!result[0]) {
                    hasMethod(name, receiver, isStatic, objectIsApplet, result);
                }
                return true;
            }

            public Object findClass(String name) {
                throw new UnsupportedOperationException("Should not call this");
            }

            public Object newInstance(Object clazz,
                                      Object[] arguments) throws Exception {
                throw new UnsupportedOperationException("Should not call this");
            }
        }

        private class LiveConnectWorker implements Runnable {
            private volatile boolean shouldStop;
            private Object lock = new Object();
            private LinkedList/*<JavaObjectOpMessage>*/ workQueue = new LinkedList();

            public void enqueue(JavaObjectOpMessage msg) {
                synchronized(lock) {
                    workQueue.add(msg);
                    lock.notifyAll();
                }
            }

            public void stop() {
                shouldStop = true;
                synchronized(lock) {
                    lock.notifyAll();
                }
            }

            public void run() {
                // workaround the Plugin2Manager.getCurrentManager() issue
                Plugin2Manager.setCurrentManagerThreadLocal(manager);
                
                try {
                    while (!shouldStop) {
                        synchronized(lock) {
                            while (!shouldStop && workQueue.isEmpty()) {
                                try {
                                    lock.wait();
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                        while (!shouldStop && !workQueue.isEmpty()) {
                            JavaObjectOpMessage msg = null;
                            synchronized(lock) {
                                msg = (JavaObjectOpMessage) workQueue.removeFirst();
                            }
			    doObjectOp(msg);
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /** Creates an AccessControlContext providing only those
        permissions corresponding to an unsigned applet coming from
        the given codeBase. These are the only permissions we grant to
        incoming calls from JavaScript, since we can't trust this
        code. This implementation does not support the notion of
        "signed JavaScript" which was originally only supported on
        Mozilla browsers anyway. */
    private static AccessControlContext createContext(URL codeBase) {
        try {
            ProtectionDomain[] domains = new ProtectionDomain[1];
            domains[0] = getJSProtectionDomain(codeBase);
            return new AccessControlContext(domains);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Returns a protection domain that represents the default
     * permission for a given URL. This code originally came from
     * sun.plugin.com.DispatchImpl and has been modified to no longer
     * query the target object for its location -- we uniformly
     * restrict the permissions of all JavaScript invocations to those
     * of the hosting applet.
     *
     * @param url URL of the codebase of the applet
     * @return protection domain.
     */
    private static ProtectionDomain getJSProtectionDomain(URL url)
	throws MalformedURLException {
	
	// Obtain default java applet policy
	Policy policy = (Policy)AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		return Policy.getPolicy();
	    }
	});

	CodeSource cs = new CodeSource(null, (java.security.cert.Certificate[])null);
	final PermissionCollection pc = policy.getPermissions(cs);

        // Inject default set of permissions
        Plugin2ClassLoader.addDefaultPermissions(pc);

	if (url != null) {
	    Permission p;
	    String path = null;
	    try {
		p = url.openConnection().getPermission();
	    } catch (java.io.IOException ioe) {
		p = null;
	    }

	    if (p instanceof FilePermission) {
		path = p.getName();
	    } else if ((p == null) && (url.getProtocol().equals("file"))) {
		path = url.getFile().replace('/', File.separatorChar);
		path = ParseUtil.decode(path);
	    } else if (p instanceof SocketPermission) {
		// Socket permission to connect back to the host
		String host = url.getHost();
                if (host == null || host.equals("")) {
                    // Probably some sort of nested URL
                    // Try to figure out the real host, but don't worry if we fail
                    try {
                        host = new URL(url.getFile()).getHost();
                    } catch (MalformedURLException e) {
                    }
                }
                // Only grant permission to connect to the host if we have a valid one
                if (host != null && !host.equals("")) {
                    pc.add(new SocketPermission(host,  
		        SecurityConstants.SOCKET_CONNECT_ACCEPT_ACTION));
                }
	    }

	    if (path != null) {
		// We need to add an additional permission to read recursively
		if (path.endsWith(File.separator)) {
		    path += "-";
		} else {
		    int endIndex = path.lastIndexOf(File.separatorChar);
		    if (endIndex != -1)
			path = path.substring(0, endIndex+1) + "-";
		}

		pc.add(new FilePermission(path, SecurityConstants.FILE_READ_ACTION));
	    }

            // Firefox extensions expect to be able to access Java
            // from a privileged context. These extensions come from
            // URLs starting with "chrome://" and the browser displays
            // a security warning before installing such extensions.
            // Therefore for compatibility reasons we grant all
            // permissions to incoming JavaScript calls coming from
            // such URLs.
            if ("chrome".equals(url.getProtocol())) {
                CeilingPolicy.addTrustedPermissions(pc);
            }
	}

	return new JavaScriptProtectionDomain(pc);
    }

    // Debugging helper
    private static String getOpName(int objectOpKind) {
        switch (objectOpKind) {
            case JavaObjectOpMessage.CALL_METHOD:          return "CALL_METHOD";
            case JavaObjectOpMessage.GET_FIELD:            return "GET_FIELD";
            case JavaObjectOpMessage.SET_FIELD:            return "SET_FIELD";
            case JavaObjectOpMessage.HAS_FIELD:            return "HAS_FIELD";
            case JavaObjectOpMessage.HAS_METHOD:           return "HAS_METHOD";
	    case JavaObjectOpMessage.HAS_FIELD_OR_METHOD:  return "HAS_FIELD_OR_METHOD";
            default: throw new IllegalArgumentException("Invalid operation kind " + objectOpKind);
        }
    }
}
