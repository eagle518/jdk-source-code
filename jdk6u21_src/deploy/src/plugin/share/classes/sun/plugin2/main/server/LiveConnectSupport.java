/*
 * @(#)LiveConnectSupport.java	1.22 10/03/30
 *
 * Copyright (c) 2007,2010 Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.io.*;
import java.util.*;

import netscape.javascript.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.SystemUtil;

/** This class assists in the maintenance of reference counts and
    validation of incoming JavaScript objects, as well as other
    functionality. It could have been implemented as a helper class
    which each Plugin implementation used internally, but that would
    complicate the semantics of some of the Plugin methods. Instead we
    completely factor it out. */

public class LiveConnectSupport {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    private static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);

    /** Initializes LiveConnect support for the given applet. */
    public static synchronized void initialize(int appletID, Plugin plugin) {
        AppletID aid = new AppletID(appletID);
        pluginInfoMap.put(aid, new PerPluginInfo(plugin, appletID));
    }

    /** Shuts down LiveConnect support for the given applet. This must
        be called from the browser's main thread. */
    public static synchronized void shutdown(int appletID) {
        if (DEBUG) {
            System.out.println("  LiveConnectSupport.shutdown(" + appletID + ")");
        }
        PerPluginInfo info = getInfo(appletID);
        info.releaseAllObjects();
        AppletID aid = new AppletID(appletID);
        pluginInfoMap.remove(aid);
        // Clean up any remaining current conversation for this plugin
        currentConversationMap.remove(info.getPlugin());
        // Finally, notify the plugin's main thread in case we were
        // waiting for LiveConnect responses that are never going to
        // come
        info.getPlugin().notifyMainThread();
    }

    /** Registers this BrowserSideObject in the context of the given
        applet ID. This must be called from the browser's / plugin's
        main thread as it may turn around and call
        Plugin.javaScriptRetainObject. */
    public static void registerObject(int appletID, BrowserSideObject object) throws JSException {
        getInfo(appletID).registerObject(object);
    }

    /** Releases this BrowserSideObject in the context of the given
        applet. This does not need to be called from the browser's
        main thread but may be called from an arbitrary thread. */
    public static void releaseObject(int appletID, BrowserSideObject object) {
        try {
            getInfo(appletID).releaseObject(object);
        } catch (JSException e) {
            // This happens when we terminate an applet before it gets the chance
            // to release its JavaScript objects
        }
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static BrowserSideObject javaScriptGetWindow(Conversation conversation, int appletID) throws JSException {
        return getInfo(appletID).javaScriptGetWindow(conversation);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static Object javaScriptCall(Conversation conversation,
                                        int appletID,
                                        BrowserSideObject obj, String methodName, Object args[]) throws JSException {
        
        Object object = null;
        PerPluginInfo info = getInfo(appletID);
        Plugin plugin = info.getPlugin();
        
        plugin.incrementActiveJSCounter();
        try {
            object = info.javaScriptCall(conversation, obj, methodName, args);
        } finally {
            plugin.decrementActiveJSCounter();
        }
        
        return object;
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static Object javaScriptEval(Conversation conversation,
                                        int appletID,
                                        BrowserSideObject obj, String code) throws JSException {
        
        Object object = null;
        PerPluginInfo info = getInfo(appletID);
        Plugin plugin = info.getPlugin();
        
        plugin.incrementActiveJSCounter();
        try {
            object = info.javaScriptEval(conversation, obj, code);
        } finally {
            plugin.decrementActiveJSCounter();
        }
        
        return object;

    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static Object javaScriptGetMember(Conversation conversation,
                                             int appletID,
                                             BrowserSideObject obj, String name) throws JSException {
        return getInfo(appletID).javaScriptGetMember(conversation, obj, name);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static void javaScriptSetMember(Conversation conversation,
                                           int appletID,
                                           BrowserSideObject obj, String name, Object value) throws JSException {
        getInfo(appletID).javaScriptSetMember(conversation, obj, name, value);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static void javaScriptRemoveMember(Conversation conversation,
                                              int appletID,
                                              BrowserSideObject obj, String name) throws JSException {
        getInfo(appletID).javaScriptRemoveMember(conversation, obj, name);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static Object javaScriptGetSlot(Conversation conversation,
                                           int appletID,
                                           BrowserSideObject obj, int index) throws JSException {
        return getInfo(appletID).javaScriptGetSlot(conversation, obj, index);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static void javaScriptSetSlot(Conversation conversation,
                                         int appletID,
                                         BrowserSideObject obj, int index, Object value) throws JSException {
        getInfo(appletID).javaScriptSetSlot(conversation, obj, index, value);
    }

    /** Performs the given JavaScript operation, doing some argument validation.
        This must be called on the browser's / plugin's main thread. */
    public static String javaScriptToString(Conversation conversation,
                                            int appletID,
                                            BrowserSideObject obj) {
        return getInfo(appletID).javaScriptToString(conversation, obj);
    }

    //----------------------------------------------------------------------
    // Routines supporting JavaScript -> Java operations
    //

    /** Sends a request for the RemoteJavaObject representing a given
        applet. See {@link #sendRemoteJavaObjectOp
        sendRemoteJavaObjectOp} below for an explanation of the
        ResultID. We split this operation up into two so that if
        JavaScript fetches the object corresponding to a large applet
        that takes a long time to load, we don't hang the browser
        while we wait for the applet to finish loading. */
    public static ResultID sendGetApplet(Plugin source, AppletID appletID) throws IOException {
        // Note that we use no preexisting conversation for this
        // transaction because we don't want this request to go to any
        // thread but the worker thread on the other side, and we
        // don't start a new conversation because we want the reply
        // message to go to our worker thread so this thread can pump
        // messages if necessary while waiting for the reply

        // Create a ResultID for the result
        int id = nextResultID();
        ResultID resultID = new ResultID(id);
        // Register interest in this result
        synchronized (LiveConnectSupport.class) {
            resultIDInterestMap.put(resultID, source);
        }

        resendGetApplet(appletID, resultID);

        // Return the token the user can use to get the result later, if any
        return resultID;
    }

    public static void resendGetApplet(AppletID appletID, ResultID resultID) throws IOException {
        // Send the message
        JVMManager.getManager().sendGetApplet(appletID, resultID.getID());
    }

    /** Sends a request for a RemoteJavaObject representing a portion
        of the Java namespace (in the form of a JavaNameSpace object).
        See {@link #sendRemoteJavaObjectOp sendRemoteJavaObjectOp}
        below for an explanation of the ResultID. We split this
        operation up into two so that if JavaScript for example forces
        initialization of a class that takes a long time to load, we
        don't hang the browser while we wait for the operation to
        complete. */
    public static ResultID sendGetNameSpace(Plugin source, AppletID appletID, String nameSpace) throws IOException {
        // Note that we use no preexisting conversation for this
        // transaction because we don't want this request to go to any
        // thread but the worker thread on the other side, and we
        // don't start a new conversation because we want the reply
        // message to go to our worker thread so this thread can pump
        // messages if necessary while waiting for the reply

        // Create a ResultID for the result
        int id = nextResultID();
        ResultID resultID = new ResultID(id);
        // Register interest in this result
        synchronized (LiveConnectSupport.class) {
            resultIDInterestMap.put(resultID, source);
        }
        // Send the message
        JVMManager.getManager().sendGetNameSpace(appletID, nameSpace, id);
        // Return the token the user can use to get the result later, if any
        return resultID;
    }

    /** Sends a Java object operation over the wire. The operationKind
        must be one of those described in {@link
        sun.plugin2.message.JavaObjectOpMessage
        JavaObjectOpMessage}. Returns a ResultID which may be polled
        later by the browser's / plugin's main thread. When the result
        comes in, the plugin's main thread will be notified via {@link
        sun.plugin2.main.server.Plugin#signal Plugin.signal()}. <P>

        The ResultID is necessary for the following reasons. We can
        have multiple ongoing round-trip Java <-> JavaScript calls
        simultaneously because Java is multi-threaded. The moment a
        JavaScript -> Java call is initiated, this opens up the
        possibility that a different thread in the Java process might
        initiate a Java -> JavaScript call. We consider this legal
        usage. One specific scenario we are concerned with is:

        <UL>
        <LI> The browser initiates a JavaScript -> Java call handled
             by Java thread A.

        <LI> Java thread B jumps in and initiates a Java -> JavaScript
             call, which begins another round-trip JavaScript -> Java
             call handled by thread B (since it is the most recent
             thread to initiate a Java -> JavaScript call and
             therefore the round-trip JavaScript -> Java call is a
             consequence of B's work).

        <LI> While thread B is doing its work, thread A completes its
             work and returns its result back to the web browser.

        <LI> At this point on the browser side there are two frames on
             the plugin thread's stack associated with waiting for
             results from the target Java process: one waiting for a
             result from thread B (the topmost frame), and one waiting
             for a result from thread A. If we consider the return
             result from thread A to satisfy the topmost frame, we
             will return the wrong result to the browser.
        </UL>

        The Conversation paradigm at the Pipe level is useful, but we
        don't want to try to overload it to handle this case. It is
        already being used to shuttle the round-trip JavaScript ->
        Java calls back to the threads which initiated the Java ->
        JavaScript calls. For this reason we queue up the returned
        results from the JavaScript -> Java calls and poll them
        explicitly from the browser's main thread.
    */
    public static ResultID sendRemoteJavaObjectOp(Plugin source,
                                                  RemoteJavaObject object,
                                                  String memberName,
                                                  int operationKind,
                                                  Object[] args) throws IOException, JSException {
        // Need to verify that the object and any RemoteJavaObjects in
        // the argument array all have the same JVM ID
        int requiredJVMID = object.getJVMID();
        if (args != null) {
            for (int i = 0; i < args.length; i++) {
                Object arg = args[i];
                if (arg != null && (arg instanceof RemoteJavaObject)) {
                    RemoteJavaObject remote = (RemoteJavaObject) arg;
                    if (remote.getJVMID() != requiredJVMID) {
                        throw new JSException("Can not pass objects between JVMs (arg " + i +
                                              " JVM ID = " + remote.getJVMID() + ", required " + requiredJVMID + ")");
                    }
                }
            }
        }

        // See whether there's a current ongoing Java to JavaScript call for this plugin
        Conversation cur = getCurrentConversation(source);
        // Create a ResultID for the result
        int id = nextResultID();
        ResultID resultID = new ResultID(id);
        // Register interest in this result
        synchronized (LiveConnectSupport.class) {
            resultIDInterestMap.put(resultID, source);
        }
        if (DEBUG) {
            System.out.println("LiveConnectSupport.sendRemoteJavaObjectOp: " +
                               getOpName(operationKind) + " \"" + memberName + "\"");
        }
        // Send the message
        JVMManager.getManager().sendRemoteJavaObjectOp(cur,
                                                       object,
                                                       memberName,
                                                       operationKind,
                                                       args,
                                                       id);
        // Return the token the user can use to get the result later, if any
        return resultID;
    }

    /** Records a result (from a JavaScript -> Java call) for the
        given ResultID. If an exception occurred during the call, the
        caller should register a RuntimeException object, which will
        be propagated back to the caller. */
    public static synchronized void recordResult(ResultID id, Object result) {
        results.put(id, result);
        Plugin waiter = (Plugin) resultIDInterestMap.remove(id);
        // The waiter should really never be null
        if (DEBUG) {
            if (waiter == null) {
                System.out.println("*** WARNING: no plugin was waiting for result " + id);
            }
        }
        if (waiter != null) {
            waiter.notifyMainThread();
        }
    }

    /** Indicates whether a result has come in for the given ResultID. */
    public static synchronized boolean resultAvailable(ResultID id) {
        return results.containsKey(id);
    }

    /** Fetches the result for the given ResultID. If an exception
        occurred during the JavaScript -> Java operation, or if a
        result has not yet been produced, this method will throw a
        RuntimeException. <P>

        <B>NOTE</B> that for method invocations and other operations
        (like field sets) that have no return value, this method will
        NOT return null, but instead the value Void.TYPE! See
        JVMInstance.java and the processing of JavaReplyMessages.
        This is necessary in order to disambiguate null and void
        return values to some JavaScript engines.
    */
    public static synchronized Object getResult(ResultID id) throws RuntimeException {
        if (!resultAvailable(id))
            throw new RuntimeException("Result has not yet been produced for " + id);
        Object result = results.remove(id);
        if (result != null && (result instanceof RuntimeException)) {
            if (DEBUG) {
                System.out.println("LiveConnectSupport: exception thrown during JavaScript -> Java call:");
                ((RuntimeException) result).printStackTrace();
            }
            throw (RuntimeException) result;
        }
        if (DEBUG) {
            System.out.println("LiveConnectSupport: result " + id + " = " + result);
        }
        return result;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private LiveConnectSupport() {}

    // Map from applet ID to PerPluginInfo, used for Java to JavaScript calls
    private static Map/*<AppletID, PerPluginInfo>*/ pluginInfoMap = new HashMap();

    // Map from Plugin to the most recent LiveConnect-related
    // Conversation associated with it, used for JavaScript to Java calls
    private static Map/*<Plugin, Conversation>*/ currentConversationMap = new HashMap();

    // The next integer result ID that needs to be passed back and
    // forth between the browser and client
    private static int curResultID;

    // Map from ResultID to Plugin indicating interest in incoming result
    private static Map/*<ResultID, Plugin>*/ resultIDInterestMap = new HashMap();

    // Map of JavaScript -> Java results that have come back in
    private static Map/*<ResultID, Object>*/ results = new HashMap();

    private static synchronized PerPluginInfo getInfo(int appletID) throws JSException {
        PerPluginInfo info = (PerPluginInfo) pluginInfoMap.get(new AppletID(appletID));
        if (info == null) {
            throw new JSException("Plugin instance for applet ID " + appletID + " was already released");
        }
        return info;
    }

    private static synchronized Conversation replaceCurrentConversation(Plugin plugin, Conversation conversation) {
        return (Conversation) currentConversationMap.put(plugin, conversation);
    }

    private static synchronized Conversation getCurrentConversation(Plugin plugin) {
        return (Conversation) currentConversationMap.get(plugin);
    }

    private static synchronized int nextResultID() {
        return ++curResultID;
    }

    // This is where all of the work is done
    static class PerPluginInfo {
        private Plugin plugin;

        // For debugging only
        private int appletID;

        // Note that because this set is currently on a per-applet
        // basis, and because we decide the validity and reference
        // counting of BrowserSideObjects on the same per-applet
        // basis, this implies that the user can't pass JSObjects
        // between applets on the client side. From a conceptual
        // standpoint this is correct, but there may be existing
        // applications out there which rely on this functionality. If
        // so, we'll have to extend this tracking to a two-level
        // scheme, so that we have a global map of BrowserSideObjects
        // to their reference counts, as well as per-applet sets of
        // BrowserSideObjects. Validity checks would then be extended
        // to decide validity based on presence of the
        // BrowserSideObject in the global map.
        private Set/*<BrowserSideObject>*/ registeredBrowserSideObjects =
            new HashSet/*<BrowserSideObject>*/();

        public PerPluginInfo(Plugin plugin, int appletID) {
            this.plugin = plugin;
            this.appletID = appletID;
        }

        public Plugin getPlugin() {
            return plugin;
        }

        public synchronized void registerObject(BrowserSideObject object) {
            if (registeredBrowserSideObjects.add(object)) {
                plugin.javaScriptRetainObject(object);
                if (DEBUG && VERBOSE) {
                    System.out.println("  LiveConnectSupport: retained " + object + " for applet " + appletID);
                }
            }
        }

        public synchronized void releaseObject(final BrowserSideObject object) {
            if (registeredBrowserSideObjects.remove(object)) {
                plugin.invokeLater(new Runnable() {
                        public void run() {
                            plugin.javaScriptReleaseObject(object);
                        }
                    });
            } else {
                if (DEBUG) {
                    System.out.println("!!! WARNING: LiveConnectSupport.releaseObject called for already released / untracked object 0x" +
                                       Long.toHexString(object.getNativeObjectReference()));
                }
            }
        }

        public synchronized void releaseAllObjects() {
            for (Iterator iter = registeredBrowserSideObjects.iterator(); iter.hasNext(); ) {
                BrowserSideObject obj = (BrowserSideObject) iter.next();
                plugin.javaScriptReleaseObject(obj);
                if (DEBUG && VERBOSE) {
                    System.out.println("  LiveConnectSupport: released " + obj + " for applet " + appletID);
                }
            }
            registeredBrowserSideObjects.clear();
        }

        public BrowserSideObject javaScriptGetWindow(Conversation conversation) {
            return plugin.javaScriptGetWindow();
        }

        public Object javaScriptCall(Conversation conversation,
                                     BrowserSideObject obj, String methodName, Object[] args) throws JSException {
            // Check to see whether all incoming BrowserSideObjects are known
            validateObject(obj);
            validateObjectArray(args);
            if (DEBUG) {
                System.out.println("LiveConnectSupport.PerPluginInfo.javaScriptCall:");
                System.out.println("  methodName: " + methodName);
                System.out.print  ("  args: ");
                if (args == null) {
                    System.out.println("null");
                } else {
                    System.out.print("[");
                    for (int i = 0; i < args.length; i++) {
                        if (i > 0) {
                            System.out.print(", ");
                        }
                        System.out.print(args[i]);
                    }
                    System.out.println("]");
                }
            }
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                Object res = plugin.javaScriptCall(obj, methodName, args);
                if (DEBUG) {
                    System.out.println("  result of call to \"" + methodName + "\": " + res);
                }
                return res;
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public Object javaScriptEval(Conversation conversation,
                                     BrowserSideObject obj, String code) throws JSException {
            // Check to see whether all incoming BrowserSideObjects are known
            validateObject(obj);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                return plugin.javaScriptEval(obj, code);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public Object javaScriptGetMember(Conversation conversation,
                                          BrowserSideObject obj, String name) throws JSException {
            validateObject(obj);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                return plugin.javaScriptGetMember(obj, name);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public void javaScriptSetMember(Conversation conversation,
                                        BrowserSideObject obj, String name, Object value) throws JSException {
            validateObject(obj);
            validateObject(value);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                plugin.javaScriptSetMember(obj, name, value);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public void javaScriptRemoveMember(Conversation conversation,
                                           BrowserSideObject obj, String name) throws JSException {
            validateObject(obj);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                plugin.javaScriptRemoveMember(obj, name);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }
        
        public Object javaScriptGetSlot(Conversation conversation,
                                        BrowserSideObject obj, int index) throws JSException {
            validateObject(obj);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                return plugin.javaScriptGetSlot(obj, index);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public void javaScriptSetSlot(Conversation conversation,
                                      BrowserSideObject obj, int index, Object value) throws JSException {
            validateObject(obj);
            validateObject(value);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                plugin.javaScriptSetSlot(obj, index, value);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        public String javaScriptToString(Conversation conversation,
                                         BrowserSideObject obj) {
            validateObject(obj);
            Conversation old = replaceCurrentConversation(plugin, conversation);
            try {
                return plugin.javaScriptToString(obj);
            } finally {
                replaceCurrentConversation(plugin, old);
            }
        }

        //----------------------------------------------------------------------
        // Internals only below this point
        //

        private void validateObjectArray(Object[] args) throws JSException {
            if (args != null) {
                for (int i = 0; i < args.length; i++) {
                    validateObject(args[i]);
                }
            }
        }

        private void validateObject(Object obj) throws JSException {
            if (obj == null)
                return;

            if (obj instanceof BrowserSideObject) {
                if (!registeredBrowserSideObjects.contains(obj)) {
                    throw new JSException("Attempt to reference unknown or already-released JavaScript object; may not pass JSObjects between applets directly");
                }
            }
        }
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
