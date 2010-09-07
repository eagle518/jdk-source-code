/*
 * @(#)JVMInstance.java	1.66 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.io.*;
import java.net.PasswordAuthentication;
import java.util.*;

import com.sun.deploy.config.*;
import com.sun.deploy.util.*;
import netscape.javascript.*;
import sun.plugin2.jvm.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.message.transport.*;
import sun.plugin2.util.ParameterNames;
import sun.plugin2.util.SystemUtil;

/** Represents a currently running attached JVM. */

public class JVMInstance {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    private static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);

     private static final boolean NO_HEARTBEAT = (SystemUtil.getenv("JPI_PLUGIN2_NO_HEARTBEAT") != null);

    // The ID of this JVM instance
    private int jvmID;

    // The information about the JRE we're launching
    private JREInfo javaInfo;

    // The TransportFactory which creates the Transport for us;
    // keep this around to terminate it cleanly
    private TransportFactory transportFactory;

    // The pipe we use to communicate with our sub-process
    private volatile Pipe pipe;

    // Some state
    private boolean started;

    // indicate the jvm is tainted and should not be used to run additional applets
    private boolean tainted = false;

    // indication whether the jvm is to be used by a specific applet and not to be
    // shared by other applets
    private boolean exclusive = false;

    // The original JVMParameters we were constructed with
    // (we need a copy since we mutate them irrevocably while starting the JVMLauncher)
    private JVMParameters originalParams;

    // The real JVMParameters we used to start the JVMLauncher with
    private JVMParameters realParams;

    // The launcher for the sub-process
    private JVMLauncher launcher;

    // Some state to terminate the worker thread
    private volatile boolean shouldStop;

    // A map from the applet IDs hosted by this JVM instance to the
    // corresponding Plugin objects
    private Map/*<Integer, Plugin>*/ appletToPluginMap = new HashMap/*<Integer, Plugin>*/();

    // Which applets we have received stop acknowledgment messages for
    private Set/*<Integer>*/ stopAckSet = new HashSet/*<Integer>*/();

    // Support for detection of illegal command-line arguments.
    // In the case the user specified an illegal command-line argument
    // either via the Java Control Panel or in the HTML, the JVM will
    // fail to reach PluginMain.main() and will not send us a
    // JVMStartedMessage. We can detect this, clear out the
    // user-specified command-line arguments and attempt to restart
    // the JVM with no intervention from higher levels. In order to do
    // this we need to temporarily queue up any messages we send over
    // to the target JVM and replay them if the JVM exited abruptly.

    // We can only restart the JVM once
    private boolean restartable = true;
    private boolean gotJVMStartedMessage = false;
    private List/*<Message>*/ queuedMessages = new ArrayList/*<Message>*/();

    // Applet printing state
    private volatile boolean busyPrinting;

    /** Creates a JVMInstance object. Does not launch the JVM yet;
        call {@link #start start} to do so. The incoming JVMParameters
        should not specify a main class or arguments yet. They may be
        modified (currently, will be modified). (FIXME: consider
        factoring out the main class and program arguments into a
        separate class than JVMParameters.) */
    public JVMInstance(long startTimeUserClick, int id,
                       JREInfo javaInfo,
                       JVMParameters params,
                       boolean exclusive) 
   {
        this.jvmID = id;
        this.javaInfo = javaInfo;

        originalParams = params.copy();
        realParams = params.copy();
        // Create the JVM launcher but don't start it yet
        launcher = new JVMLauncher(startTimeUserClick, javaInfo.getJREPath(), params);
        launcher.addJVMEventListener(new Listener());
        this.busyPrinting = false;
        this.exclusive = exclusive;
    }

    /** Returns the integer ID of this JVM instance, used to identify
        Java objects exposed from it back to the web browser. */
    public int getID() {
        return jvmID;
    }

    /** Returns the platform information for the target JVM. */
    public JREInfo getJavaInfo() {
        return javaInfo;
    }

    /** Convenience method to return the JREInfo's VersionID. */
    public VersionID getProductVersion() {
        return getJavaInfo().getProductVersion();
    }

    /** Returns the JVMParameters used to create the target JVM. NOTE
        that mutating the returned object is NOT supported and will
        produce incorrect and unexpected results. */
    public JVMParameters getParameters() {
        return originalParams;
    }

    /** Indicates whether the given applet is running. */
    public synchronized boolean appletRunning(AppletID appletID) {
        return (appletToPluginMap.get(new Integer(appletID.getID())) != null);
    }

    public synchronized boolean isTainted() {
        return tainted;
    }

    synchronized void markTainted() {
        tainted = true;
        if (DEBUG) {
            System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") marked tainted");
        }
    }

    public synchronized boolean isExclusive() {
        return exclusive;
    }

    /** Starts the underlying JVM. */
    public void start() throws IOException {
        startImpl(false);
    }

    private void startImpl(boolean isRestart) throws IOException {
        if (started) {
            throw new IllegalStateException("Already started");
        }
        started = true;

        // We defer the initialization of the IPC mechanism to this point
        // largely to be able to independently test the JVM selection algorithm

        // Set up the transport and pipe
        transportFactory = TransportFactory.createForCurrentOS();
        SerializingTransport transport = transportFactory.getTransport();
        // Register the messages we send to the other side
        PluginMessages.register(transport);
        // Create the pipe
        pipe = new Pipe(transport, true);

        JVMParameters params = launcher.getParameters();

        // Add the outgoing arguments to the main class on the other side
        // There is an implicit contract between this code and sun.plugin2.main.client.PluginMain
        params.addArgument("sun.plugin2.main.client.PluginMain");
        String[] additionalArgs = transportFactory.getChildProcessParameters();
        for (int i = 0; i < additionalArgs.length; i++) {
            params.addArgument(additionalArgs[i]);
        }
        if (DEBUG && VERBOSE) {
            System.out.println("JVMInstance.start: launcher params:");
            List/*<String>*/ subordinateArgs = params.getCommandLineArguments(true, false);
            for (Iterator iter = subordinateArgs.iterator(); iter.hasNext(); ) {
                System.out.println("\t<"+(String) iter.next()+">");
            }
        }
        // Start the sub-process
        launcher.start();
        Exception exc = launcher.getErrorDuringStartup();
        if (exc != null) {
            exc.printStackTrace();
            throw (IOException) new IOException().initCause(exc);
        }
        new StreamMonitor(launcher.getInputStream());
        new StreamMonitor(launcher.getErrorStream());
        new WorkerThread().start();

        String[][] paramsArray = realParams.copyToStringArrays();
        // get the user's home setting based on the USERPROFILE env. variable on windows.
        // On unix, getUserHomeOverride() returns null because we don't need to override
        // the setting of the user.home property set by the java launcher
        String userHome = Config.getInstance().getUserHomeOverride();
        SetJVMIDMessage jvmIDMsg = new SetJVMIDMessage(null, jvmID, JVMManager.getBrowserType(), exclusive, userHome, paramsArray);

        if (DEBUG && VERBOSE) {
            System.out.println("JVMInstance.start: JVMID original params array:");
            for (int i=0; i<paramsArray.length; i++) {
                for (int j=0; j<paramsArray[i].length; j++) {
                    System.out.println("\t["+i+"]["+j+"]: <"+paramsArray[i][j]+">");
                }
            }
        }

        if (!isRestart) {
            try {
                // Start things off by sending the JVM ID down the wire
                sendMessage(jvmIDMsg);
            } catch (IOException e) {
                // We have to give it one more try (!isRestart),
                // Don't bail out with a RuntimeException yet.
                if (DEBUG) {
                    e.printStackTrace();
                }
            }
        } else {
            // Replay all previously queued messages.
            // Note that we first need to replace the old
            // SetJVMIDMessage with a new one reflecting the new
            // (empty) command-line arguments
            for (int i = 0; i < queuedMessages.size(); i++) {
                Message msg = (Message) queuedMessages.get(i);
                if (msg.getID() == SetJVMIDMessage.ID) {
                    queuedMessages.set(i, jvmIDMsg);
                    break;
                }
            }
            try {
                for (Iterator iter = queuedMessages.iterator(); iter.hasNext(); ) {
                    sendMessage((Message) iter.next());
                }
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }

    /** Forcibly shuts down the process associated with the target JVM. */
    public void destroy() {
        launcher.destroy();
    }

    /** Indicates whether an error occurred during startup of the target JVM. */
    public boolean errorOccurred() {
        // FIXME: consider reporting these errors in more descriptive
        // form to higher levels
        return (launcher.getErrorDuringStartup() != null);
    }

    /** Indicates whether the target JVM has exited. */
    public synchronized boolean exited() {
        return (launcher.exited() && (!restartable || gotJVMStartedMessage));
    }

    /** Returns the exit code of the target JVM, if it has exited. */
    public int exitCode() {
        return launcher.getExitCode();
    }

    //----------------------------------------------------------------------
    // Applet starting, stopping and resizing functionality
    //

    /** Starts an applet with the given applet parameters, Plugin
        instance, parent native window, indication of whether to use
        the XEmbed protocol on X11 platforms, and applet ID. (See
        {@link sun.plugin2.applet.Applet2Manager#setForDummyApplet}
        for a description of the "isForDummyApplet" argument, which is
        a concession to the Firefox browser.) Returns true if the
        (asynchronous) starting of the applet appeared to be
        successful, or false if the communications channel with the
        target JVM seems to be shut down. */
    public boolean startApplet(Map/*<String,String>*/ parameters,
                               Plugin plugin,
                               long parentNativeWindow,
                               long parentConnection,
                               boolean useXEmbedOnX11Platforms,
                               int appletID,
                               boolean isForDummyApplet, boolean isRelaunch) {
        try {
            // This is a first cut at some semblance of error handling
            // FIXME: consider more detailed error reporting to higher levels
            StartAppletMessage message = new StartAppletMessage(null,
                                                                parameters,
                                                                parentNativeWindow,
                                                                parentConnection,
                                                                useXEmbedOnX11Platforms,
                                                                appletID,
                                                                plugin.getDocumentBase(),
                                                                isForDummyApplet);
            if (DEBUG) {
                System.out.println("JVMInstance for " + javaInfo.getProductVersion() + " sending start applet message");
                System.out.println("  isRelaunch: "+isRelaunch);
                System.out.println("  Parameters:");
                for (Iterator iter = parameters.keySet().iterator(); iter.hasNext(); ) {
                    String key = (String) iter.next();
                    System.out.println("    " + key + "=" + (String) parameters.get(key));
                }
            }
            //Check if the vm is tainted. 
            synchronized(this) {
                if (isTainted()) {
                    if (DEBUG) {
                        System.out.println("JVMInstance for " + javaInfo.getProductVersion() + " is tainted. Don't start applet");
                    }
                    return false;
                }

                sendMessage(message);

                registerApplet(appletID, plugin);

                return true;
            }
        } catch (IOException e) {
            if (DEBUG) {
                e.printStackTrace();
            }
            return false;
        }
    }

    /** Resizes the applet with the given ID. */
    public void setAppletSize(int appletID,
                              int width,
                              int height) {
        try {
            SetAppletSizeMessage message =
                new SetAppletSizeMessage(null, appletID, width, height);
            sendMessage(message);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /** Sends a message to stop the applet with the specified ID. This
        method returns immediately; use {@link
        receivedStopAcknowledgment receivedStopAcknowledgment} (with
        care) to determine whether the applet shutdown appeared to be
        successful. */
    public void sendStopApplet(int appletID) {
        Pipe localPipe = pipe;
        if (localPipe != null) {
            try {
                StopAppletMessage message = new StopAppletMessage(null, appletID);
                if (DEBUG) 
                {
                    System.out.println("JVMInstance for " + javaInfo.getProductVersion() +
                                       " sending stop applet message for applet ID " + appletID);
                }
                localPipe.send(message);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /** Indicates whether we received an acknowledgment of a request
        to shut down the applet with the specified ID. */
    public synchronized boolean receivedStopAcknowledgment(int appletID) {
        return stopAckSet.contains(new Integer(appletID));
    }

    /** Recycles the given applet ID for later reuse. */
    public synchronized void recycleAppletID(int appletID) {
        unregisterApplet(appletID);
    }

    // Records an acknowledgment of the given StopAppletAckMessage --
    // only if the applet is still considered alive by this side.
    private synchronized void recordStopAck(int appletID) {
        Integer key = new Integer(appletID);
        if (appletToPluginMap.get(key) != null) {
            stopAckSet.add(key);
        }
        // Wake up this plugin's main thread in case it's waiting
        Plugin plugin = getPluginForApplet(appletID);
        if (plugin != null) {
            plugin.notifyMainThread();
        }
    }

    public void synthesizeWindowActivation(int appletID, boolean active) {
        try {
            SynthesizeWindowActivationMessage message =
                new SynthesizeWindowActivationMessage(null, appletID, active);
            sendMessage(message);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /** Sends a message to print the applet with the specified ID. */
    public boolean printApplet(int appletID, long hdc, int x, int y, int width, int height) {

        boolean isPrinterDC = ServerPrintHelper.isPrinterDC(hdc);
        // do nothing if the device context isn't of printer type
        if (!isPrinterDC) {
            return true;
        }

        PrintAppletReplyMessage reply = null;
        Pipe localPipe = pipe;
        if (localPipe != null) {
            Conversation c = pipe.beginConversation();
            try {                
                PrintAppletMessage message = new PrintAppletMessage(c, appletID, hdc, isPrinterDC,
                                                                    x, y, width, height);
                if (DEBUG) {
                    System.out.println("JVMInstance for " + javaInfo.getProductVersion() +
                                       " sending print applet message for applet ID " + appletID +
                                       ", HDC = " + hdc + ", isPrinterDC = " + isPrinterDC);
                }
                busyPrinting = true;
                localPipe.send(message);
                reply = (PrintAppletReplyMessage) localPipe.receive(0, c);
                busyPrinting = false;
                if ( reply != null) {
                    if ( reply.getAppletID() != appletID) {
                        return false;
                    }
                    return reply.getRes();
                }
            } catch (InterruptedException e) {
		if (DEBUG) {
		    e.printStackTrace();
		}
            } catch (IOException e) {
		if (DEBUG) {
		    e.printStackTrace();
		}
            } finally {
                busyPrinting = false;
                localPipe.endConversation(c);
            }
        }
        return false;

    }


    //----------------------------------------------------------------------
    // LiveConnect support
    //

    public void sendGetApplet(int appletID, int resultID) throws IOException {
        // NOTE that we allow IOException to be propagated out of here.
        // This particular operation is very sensitive, as the browser's /
        // plugin's main thread will wait nearly indefinitely for a reply.
        // We don't want to hide any errors that might occur, since if one
        // happens then we might wait for a reply message that never comes
        // (though likely the target JVM instance would eventually exit
        // and we would notice this).
        if (DEBUG) {
            System.out.println("JVMInstance sending request for applet ID " + appletID +
                               " with result ID " + resultID);
        }
        GetAppletMessage msg = new GetAppletMessage(null, appletID, resultID);
        sendMessage(msg);
    }

    public void sendGetNameSpace(int appletID, String nameSpace, int resultID) throws IOException {
        // NOTE that we allow IOException to be propagated out of here.
        // This particular operation is very sensitive, as the browser's /
        // plugin's main thread will wait nearly indefinitely for a reply.
        // We don't want to hide any errors that might occur, since if one
        // happens then we might wait for a reply message that never comes
        // (though likely the target JVM instance would eventually exit
        // and we would notice this).
        if (DEBUG) {
            System.out.println("JVMInstance sending request for namespace \"" + nameSpace + "\" in applet ID " + appletID +
                               " with result ID " + resultID);
        }
        GetNameSpaceMessage msg = new GetNameSpaceMessage(null, appletID, nameSpace, resultID);
        sendMessage(msg);
    }

    public void releaseRemoteJavaObject(int remoteObjectID) {
        try {
            ReleaseRemoteObjectMessage message =
                new ReleaseRemoteObjectMessage(null, remoteObjectID);
            if (DEBUG) {
                System.out.println("JVMInstance for " + javaInfo.getProductVersion() +
                                   " sending release remote object message for ID " + remoteObjectID);
            }
            sendMessage(message);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void sendRemoteJavaObjectOp(Conversation conversation,
                                       RemoteJavaObject object,
                                       String memberName,
                                       int operationKind,
                                       Object[] args,
                                       int resultID) throws IOException {
        // NOTE that we allow IOException to be propagated out of here.
        // This particular operation is very sensitive, as the browser's /
        // plugin's main thread will wait nearly indefinitely for a reply.
        // We don't want to hide any errors that might occur, since if one
        // happens then we might wait for a reply message that never comes
        // (though likely the target JVM instance would eventually exit
        // and we would notice this).
        JavaObjectOpMessage msg = new JavaObjectOpMessage(conversation,
                                                          object,
                                                          memberName,
                                                          operationKind,
                                                          args,
                                                          resultID);
        sendMessage(msg);
    }

    //----------------------------------------------------------------------
    // Worker thread processing messages back and forth
    //

    private class WorkerThread extends Thread {
        public WorkerThread() {
            super("JRE " + javaInfo.getProductVersion() + " Worker Thread");
        }

        public void run() {
            try {
                while (!shouldStop) {
                    // Wake up periodically even if no data comes in
                    Message msg = null;
                    Pipe localPipe = pipe;
                    if (localPipe == null) {
                        // Target process died; terminate worker thread
                        return;
                    }

                    if (localPipe != null) {
                        msg = localPipe.receive(2000);
                    }
                    
                    if (msg != null) {
                        switch (msg.getID()) {

                        case JVMStartedMessage.ID: {
                            synchronized(JVMInstance.this) {
                                gotJVMStartedMessage = true;
                                queuedMessages.clear();
                                // If a debugger is going to be attached, don't start our
                                // heartbeat thread and inadvertently kill off the debuggee
                                if (!realParams.contains("-Xdebug") && !NO_HEARTBEAT) {
                                    new HeartbeatThread().start();
                                }
                            }
                            break;
                        }

                        case StopAppletAckMessage.ID: {
                            StopAppletAckMessage ackMsg = (StopAppletAckMessage) msg;
                            int id = ackMsg.getAppletID();
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing StopAppletAckMessage for applet ID " + id);
                            }
                            recordStopAck(id);
                            break;
                        }

                        case HeartbeatMessage.ID: {
                            long hbT0 = SystemUtils.microTime();
                            HeartbeatMessage heartbeatMessage = (HeartbeatMessage) msg;
                            // Just send this back
                            sendMessage(msg);
                            if (DEBUG && VERBOSE) {
                                long hbT1 = SystemUtils.microTime();
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing HeartbeatMessage, recv ts: "+hbT0+", send ts: "+hbT1+", dT: "+(hbT1-hbT0));
                            }
                            break;
                        }
                            
                        case GetProxyMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing GetProxyMessage");
                            }
                            GetProxyMessage getProxyMsg = (GetProxyMessage) msg;
                            processProxyRequest(getProxyMsg);
                            break;
                        }
                            
                        case JavaScriptGetWindowMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptGetWindowMessage");
                            }
                            final JavaScriptGetWindowMessage jsMsg = (JavaScriptGetWindowMessage) msg;
                            JVMManager.getManager().drainAppletMessages(new AppletID(jsMsg.getAppletID()));
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       return LiveConnectSupport.javaScriptGetWindow(jsMsg.getConversation(),
                                                                                                     jsMsg.getAppletID());
                                                   }
                                               });
                            break;
                        }
                            
                        case JavaScriptCallMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptCallMessage");
                            }
                            final JavaScriptCallMessage jsMsg = (JavaScriptCallMessage) msg;
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       return LiveConnectSupport.javaScriptCall(jsMsg.getConversation(),
                                                                                                jsMsg.getAppletID(),
                                                                                                jsMsg.getObject(),
                                                                                                jsMsg.getMethodName(),
                                                                                                jsMsg.getArguments());
                                                   }
                                               });
                            break;
                        }
                            
                        case JavaScriptEvalMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptEvalMessage");
                            }
                            final JavaScriptEvalMessage jsMsg = (JavaScriptEvalMessage) msg;
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       return LiveConnectSupport.javaScriptEval(jsMsg.getConversation(),
                                                                                                jsMsg.getAppletID(),
                                                                                                jsMsg.getObject(),
                                                                                                jsMsg.getCode());
                                                   }
                                               });
                            break;
                        }
                            
                        case JavaScriptMemberOpMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptMemberOpMessage");
                            }
                            final JavaScriptMemberOpMessage jsMsg = (JavaScriptMemberOpMessage) msg;
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       switch (jsMsg.getOperationKind()) {
                                                       case JavaScriptMemberOpMessage.GET:
                                                           return LiveConnectSupport.javaScriptGetMember(jsMsg.getConversation(),
                                                                                                         jsMsg.getAppletID(),
                                                                                                         jsMsg.getObject(),
                                                                                                         jsMsg.getMemberName());
                                                       case JavaScriptMemberOpMessage.SET:
                                                           LiveConnectSupport.javaScriptSetMember(jsMsg.getConversation(),
                                                                                                  jsMsg.getAppletID(),
                                                                                                  jsMsg.getObject(),
                                                                                                  jsMsg.getMemberName(),
                                                                                                  jsMsg.getArgument());
                                                           return null;
                                                       case JavaScriptMemberOpMessage.REMOVE:
                                                           LiveConnectSupport.javaScriptRemoveMember(jsMsg.getConversation(),
                                                                                                     jsMsg.getAppletID(),
                                                                                                     jsMsg.getObject(),
                                                                                                     jsMsg.getMemberName());
                                                           return null;
                                                       default:
                                                           throw new JSException("Unexpected JavaScript member operation " +
                                                                                         jsMsg.getOperationKind());
                                                       }
                                                   }
                                               });
                            break;
                        }
                            
                        case JavaScriptSlotOpMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptSlotOpMessage");
                            }
                            final JavaScriptSlotOpMessage jsMsg = (JavaScriptSlotOpMessage) msg;
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       switch (jsMsg.getOperationKind()) {
                                                       case JavaScriptSlotOpMessage.GET:
                                                           return LiveConnectSupport.javaScriptGetSlot(jsMsg.getConversation(),
                                                                                                       jsMsg.getAppletID(),
                                                                                                       jsMsg.getObject(),
                                                                                                       jsMsg.getSlot());
                                                       case JavaScriptSlotOpMessage.SET:
                                                           LiveConnectSupport.javaScriptSetSlot(jsMsg.getConversation(),
                                                                                                jsMsg.getAppletID(),
                                                                                                jsMsg.getObject(),
                                                                                                jsMsg.getSlot(),
                                                                                                jsMsg.getArgument());
                                                           return null;
                                                       default:
                                                           throw new JSException("Unexpected JavaScript slot operation " +
                                                                                 jsMsg.getOperationKind());
                                                       }
                                                   }
                                               });
                            break;
                        }
                            
                        case JavaScriptToStringMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptToStringMessage");
                            }
                            final JavaScriptToStringMessage jsMsg = (JavaScriptToStringMessage) msg;
                            doJavaToJavaScript(jsMsg.getAppletID(), jsMsg.getConversation(),
                                               new LiveConnectHelper() {
                                                   public Object doWork() throws JSException{
                                                       return LiveConnectSupport.javaScriptToString(jsMsg.getConversation(),
                                                                                                    jsMsg.getAppletID(),
                                                                                                    jsMsg.getObject());
                                                   }
                                                   });
                            break;
                        }
                            
                        case JavaScriptReleaseObjectMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaScriptReleaseObjectMessage");
                            }
                            // This message doesn't expect a reply
                            JavaScriptReleaseObjectMessage jsMsg = (JavaScriptReleaseObjectMessage) msg;
                            try {
                                LiveConnectSupport.releaseObject(jsMsg.getAppletID(), jsMsg.getObject());
                            } catch (JSException e) {
                                if (DEBUG)
                                    e.printStackTrace();
                            }
                            break;
                        }
                            
                        case JavaReplyMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing JavaReplyMessage");
                            }
                            JavaReplyMessage jMsg = (JavaReplyMessage) msg;
                            if (DEBUG) {
                                System.out.println("JVMInstance received JavaReplyMessage with result ID " + jMsg.getResultID() +
                                                   ((jMsg.getExceptionMessage() != null) ? " (exception)" : ""));
                            }
                            // The browser's / plugin's main thread is waiting for this;
                            // hand it over to the LiveConnectSupport class 
                            Object result = null;
                            if (jMsg.getExceptionMessage() != null) {
                                result = new RuntimeException(jMsg.getExceptionMessage());
                            } else {
                                result = jMsg.getResult();
                            }
                            if (jMsg.isResultVoid()) {
                                result = Void.TYPE;
                            }
                            LiveConnectSupport.recordResult(new ResultID(jMsg.getResultID()), result);
                            break;
                        }

                        case ShowDocumentMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing ShowDocumentMessage");
                            }
                            processShowDocRequest((ShowDocumentMessage)msg);
                            break;
                        }
            
                        case ShowStatusMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing ShowStatusMessage");
                            }
                            processShowStatusRequest((ShowStatusMessage)msg);
                            break;
                        }

                        case GetAuthenticationMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing GetAuthenticationMessage");
                            }
                            processGetAuthenticationRequest((GetAuthenticationMessage) msg);
                            break;
                        }

                        case CookieOpMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing CookieOpMessage");
                            }
                            processCookieRequest((CookieOpMessage) msg);
                            break;
                        }

                        case ModalityChangeMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing ModalityChangeMessage");
                            }
                            ModalityChangeMessage modalityMsg = (ModalityChangeMessage) msg;
                            ModalitySupport.modalityChanged(modalityMsg.getAppletID(),
                                                            modalityMsg.getModalityPushed());
                            break;
                        }

                        case MarkTaintedMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing MarkTaintedMessage");
                            }
                            markTainted();

                            boolean isEmpty;
                            synchronized (JVMInstance.this) {
                                isEmpty = appletToPluginMap.isEmpty();
                            }
                            // If there is no applet running, send Shutdown message to client VM
                            if (isEmpty) {
                                try {
                                    sendMessage(new ShutdownJVMMessage(null));
                                } catch (IOException e) {
                                    if (DEBUG) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                            break;
                        }

                        // this is sent back to indicate that the
                        // applet will execute (with or without error
                        // conditions) in this JVM instance, so we
                        // drain the spooled up messages
                        case StartAppletAckMessage.ID: {
                            StartAppletAckMessage startAppletAckMsg = (StartAppletAckMessage) msg;
                            int appletID = startAppletAckMsg.getAppletID();

                            if (DEBUG && VERBOSE) {
                                System.out.print("JVMInstance (" + javaInfo.getProductVersion() + 
                                                   ") processing StartAppletAckMessage with:"+
                                                   "\n\tappletID: "+appletID);
                            }

                            JVMManager.getManager().drainAppletMessages(new AppletID(appletID));
                            break;
                        }

                        // this is for relaunching an applet. 
                        // reusing the StartAppletMessage since the content of the message is the same.
                        case StartAppletMessage.ID: {
                            StartAppletMessage relaunchMsg = (StartAppletMessage) msg;
                            Map appletParams = relaunchMsg.getParameters();

                            // get and remove "__jre_installed" from the applet parameters map
			    String javaVersion = null;
                            boolean newJREInstalled = false;
                            if (appletParams != null) {
                                String tmp = (String) appletParams.remove(ParameterNames.JRE_INSTALLED);
                                if(null!=tmp) {
                                    newJREInstalled = Boolean.valueOf(tmp).booleanValue();
                                }
				javaVersion = (String) appletParams.get(ParameterNames.SSV_VERSION);
                            }
               
                            int appletID = relaunchMsg.getAppletID();
                            Integer key = new Integer(appletID);

                            Plugin pluginObj = null;
                            synchronized(JVMInstance.this) {
                                // save the plugin instance and remove it from the
                                // appletToPluginMap based on the appletID obtained
                                // from the relaunchMsg
                                pluginObj = (Plugin) appletToPluginMap.remove(key);
                            }

                            if (DEBUG && VERBOSE) {
                                String java_args = null;
                                if (appletParams != null) {
                                    java_args = (String) appletParams.get("java_arguments");
                                }
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + 
                                                   ") processing StartAppletMessage with:"+
                                                   "\n\tappletID: "+appletID +
                                                   "\n\targs: "+java_args +
                                                   "\n\t__applet_ssv_version: "+javaVersion+
                                                   "\n\tnewJREInstalled: "+newJREInstalled+
                                                   "\n\tplugin: "+pluginObj);
                            }
                            if(null==pluginObj) {
                                // This is an error on our part, and
                                // due to apparent race conditions is
                                // happening when it shouldn't.
                                // However, throwing an exception here
                                // causes all future applets launched
                                // in the same JVM to hang so we
                                // simply print a warning and continue.
                                System.out.println("ERROR: JVMInstance (" + javaInfo.getProductVersion() +
                                                   ") failed to relaunch applet ID " + appletID +
                                                   " because of null plugin instance");
                                break;
                            }

                            // relaunch the applet
                            AppletID res = JVMManager.getManager().relaunchApplet(
                                appletParams,
                                pluginObj,
                                relaunchMsg.getParentNativeWindowHandle(),
                                relaunchMsg.getParentConnection(),
                                relaunchMsg.useXEmbed(),
                                javaVersion,
                                appletID, newJREInstalled);

                            // If this JVM instance was created for a
                            // specific applet, then shut down the attached JVM
                            if (exclusive) {
                                sendMessage(new ShutdownJVMMessage(null));
                            }
                            break;
                        }

                        // This is only needed on Mac OS X
                        case SetChildWindowHandleMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing SetChildWindowHandleMessage");
                            }
                            final SetChildWindowHandleMessage scwhMsg = (SetChildWindowHandleMessage) msg;
                            final Plugin plugin = getPluginForApplet(scwhMsg.getAppletID()); 
                            if (plugin != null) {
                                plugin.invokeLater(new Runnable() {
                                        public void run() {
                                            plugin.setChildWindowHandle(scwhMsg.getWindowHandle());
                                        }
                                    });
                            }
                            break;
                        }

                        case PrintBandMessage.ID: {
                            if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing PrintBandMessage");
                            }

                            final PrintBandMessage pbMsg = (PrintBandMessage) msg;
                            Plugin plugin = getPluginForApplet(pbMsg.getAppletID()); 
                            if (plugin != null) {
                                try {
                                    boolean res = ServerPrintHelper.printBand(
                                        pbMsg.getHDC(), pbMsg.getDataAsByteBuffer(), pbMsg.getOffset(),
                                        pbMsg.getSrcX(), pbMsg.getSrcY(), pbMsg.getSrcWidth(), pbMsg.getSrcHeight(),
                                        pbMsg.getDestX(), pbMsg.getDestY(), pbMsg.getDestWidth(), pbMsg.getDestHeight());

                                    sendMessage(new PrintBandReplyMessage(pbMsg.getConversation(),
                                                                          pbMsg.getAppletID(),
                                                                          pbMsg.getDestY(), res));
                                } catch (IOException e) {
                                    if (DEBUG) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                            break;
                        }
 
			case BestJREAvailableMessage.ID: {
			    if (DEBUG && VERBOSE) {
                                System.out.println("JVMInstance (" + javaInfo.getProductVersion() + ") processing BestJREAvailableMessage");
                            }

			    final BestJREAvailableMessage bestJREMsg = (BestJREAvailableMessage) msg;
			    String bestJREVersion = JVMManager.getManager().getBestJREInfo(
				new VersionString(bestJREMsg.getJavaVersion())).getProduct();
			    sendMessage(new BestJREAvailableMessage(bestJREMsg.getConversation(), 
								    BestJREAvailableMessage.REPLY, bestJREVersion));
			    break;
			}

                        default:
                            // FIXME: need more messages
                            // Consider throwing an InternalError here since this would be entirely our fault
                            // However may want to soft-fail, as we can envision wanting to handle certain failure modes by
                            // dropping an ongoing conversation
                            if (DEBUG) {
                                System.err.println("sun.plugin2.main.server.JVMInstance.WorkerThread: unexpected message ID " + msg.getID() + " from client JVM instance");
                            }
                            break;
                        }
                    }
                }
            } catch (Exception e) {
		if (DEBUG) {
		    e.printStackTrace();
		}
                // Terminate the worker thread; this will cause
                // things to shut down eventually
            }
        }
    }

    private void processProxyRequest(final GetProxyMessage proxyRequest) {
        final Plugin plugin = getPluginForApplet(proxyRequest.getAppletID()); 
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        try {
                            sendMessage(ProxySupport.getProxyReply(plugin, proxyRequest));
                        } catch (IOException ex) {
                            ex.printStackTrace();
                            shutdown();
                        }
                    }
                });
        } else {
            // FIXME: should be able to signal an error in this case
            try {
                sendMessage(new ProxyReplyMessage(proxyRequest.getConversation(), null));
            } catch (IOException ex) {
                ex.printStackTrace();
                shutdown();
            }
        }
    }

    private void processShowDocRequest(final ShowDocumentMessage showDocRequest) {
        final Plugin plugin = getPluginForApplet(showDocRequest.getAppletID()); 
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        plugin.showDocument(showDocRequest.getURL(),
                                            showDocRequest.getTarget());
                    }
                });
        }
    }

    private void processShowStatusRequest(final ShowStatusMessage showStatusRequest) {
        final Plugin plugin = getPluginForApplet(showStatusRequest.getAppletID()); 
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        plugin.showStatus(showStatusRequest.getStatus());
                    }
                });
        }
    }

    private void processGetAuthenticationRequest(final GetAuthenticationMessage message) {
        final Plugin plugin = getPluginForApplet(message.getAppletID());
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        PasswordAuthentication auth = plugin.getAuthentication(message.getProtocol(),
                                                                               message.getHost(),
                                                                               message.getPort(),
                                                                               message.getScheme(),
                                                                               message.getRealm(),
                                                                               message.getRequestURL(),
                                                                               message.getProxyAuthentication());
                        GetAuthenticationReplyMessage reply =
                            new GetAuthenticationReplyMessage(message.getConversation(),
                                                              auth,
                                                              null);
                        try {
                            sendMessage(reply);
                        } catch (IOException e) {
                            e.printStackTrace();
                            shutdown();
                        }
                    }
                });
        } else {
            GetAuthenticationReplyMessage reply =
                new GetAuthenticationReplyMessage(message.getConversation(),
                                                  null,
                                                  "No registered plugin for applet ID " +
                                                  message.getAppletID());
            try {
                sendMessage(reply);
            } catch (IOException e) {
                e.printStackTrace();
                shutdown();
            }
        }
    }

    private void processCookieRequest(final CookieOpMessage request) {
        final Plugin plugin = getPluginForApplet(request.getAppletID()); 
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        try {
                            sendMessage(CookieSupport.getCookieReply(plugin, request));
                        } catch (IOException ex) {
                            ex.printStackTrace();
                            shutdown();
                        }
                    }
                });
        } else {
            try {
                sendMessage(new CookieReplyMessage(request.getConversation(), null,
                                                   "No registered plugin for applet ID " +
                                                   request.getAppletID()));
            } catch (IOException ex) {
                ex.printStackTrace();
                shutdown();
            }
        }
    }

    // LiveConnect helper method and class to allow better code refactoring above
    private static abstract class LiveConnectHelper {
        public abstract Object doWork() throws JSException;
    }
    private void doJavaToJavaScript(int appletID, final Conversation conversation, final LiveConnectHelper helper) {
        final Plugin plugin = getPluginForApplet(appletID);
        if (plugin != null) {
            plugin.invokeLater(new Runnable() {
                    public void run() {
                        Object result = null;
                        String exceptionMessage = null;
                        try {
                            result = helper.doWork();
                        } catch (JSException e) {
                            exceptionMessage = e.getMessage();
                            if (exceptionMessage == null)
                                exceptionMessage = e.toString();
                        }
                        JavaScriptReplyMessage reply = new JavaScriptReplyMessage(conversation,
                                                                                  result,
                                                                                  exceptionMessage);
                        try {
                            sendMessage(reply);
                        } catch (IOException e) {
                            e.printStackTrace();
                            shutdown();
                        }
                    }
                });
        } else {
            JavaScriptReplyMessage reply = new JavaScriptReplyMessage(conversation,
                                                                      null,
                                                                      "No registered plugin for applet ID " +
                                                                      appletID);
            try {
                sendMessage(reply);
            } catch (IOException e) {
                e.printStackTrace();
                shutdown();
            }
        }
    }

    //----------------------------------------------------------------------
    // Heartbeat detection for remote JVM
    //

    // Use a longer heartbeat timeout here because we really don't
    // want to unexpectedly kill attached JVMs unless absolutely
    // necessary
    private static final int HEARTBEAT_TIMEOUT = 10000; // ms
    private class HeartbeatThread extends Thread {
        public HeartbeatThread() {
            super("JRE " + javaInfo.getProductVersion() + " Heartbeat Thread");
        }

        public void run() {
            Pipe localPipe = pipe;
            Conversation c = localPipe.beginConversation();
            try {
                while (!exited()) {
                    // Don't ping the JVM if it's busy printing
                    while (busyPrinting) {
                        Thread.sleep(500);
                    }

                    sendMessage(new HeartbeatMessage(c));
                    Message m = localPipe.receive(HEARTBEAT_TIMEOUT, c);
                    if (m == null && !launcher.exited() && !busyPrinting) {
                        // The remote JVM is not responding; kill the process,
                        // which will cause us to detect the exit and launch a new
                        // one later
                        if (DEBUG) {
                            System.out.println("JVMInstance for " + javaInfo.getProductVersion() +
                                               " killing sub-process because of no heartbeat reply");
                        }
                        launcher.destroy();
                        break;
                    }
                    Thread.sleep(HEARTBEAT_TIMEOUT);
                }
            } catch (Exception e) {
                if (DEBUG) {
                    e.printStackTrace();
                }
            } finally {
                localPipe.endConversation(c);
            }
        }
    }

    //----------------------------------------------------------------------
    // JVM instance termination
    //

    // NOTE: we use this for implementing tainting and certain other
    // error conditions. We don't get shutdown hooks run when the web
    // browser terminates because at least on Windows they are
    // predicated on receiving Ctrl-C or a similar console event; they
    // aren't registered with e.g. atexit(). This means that the JVMs
    // attached to the browser need to send heartbeat events back to
    // this JVM to see whether they need to shut down.

    /** Forcibly shuts down this JVM instance. */
    public synchronized void shutdown() {
        shouldStop = true;

        if (pipe != null) {
            // Other process didn't terminate already
            try {
                sendMessage(new ShutdownJVMMessage(null));
            } catch (IOException e) {
                // FIXME: is forcibly destroying the target process always
                // the right thing to do?
                launcher.destroy();
            }
        }
        // Let's see whether this is a robust enough mechanism before
        // adding anything more like an acknowledgment; may want to
        // call dispose() directly from here
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private void sendMessage(Message message) throws IOException {
        if (message instanceof AppletMessage) {
            if(JVMManager.getManager().spoolAppletMessage((AppletMessage)message)) {
                // message is an AppletMessage, i.e. needs a started applet to be received
                // and we are still waiting until it has started

                // Allow selected messages such as the
                // SetAppletSizeMessage through to allow the gray box
                // painter to resize itself properly before the applet
                // is loaded
                if (!(message instanceof SetAppletSizeMessage)) {
                    return ;
                }
            }
        }
        sendMessageDirect(message);
    }

    protected void sendMessageDirect(Message message) throws IOException {
        Pipe localPipe = null;
        synchronized (this) {
            localPipe = pipe;
            if (localPipe != null) {
                if (restartable && !gotJVMStartedMessage) {
                    queuedMessages.add(message);
                }
            }
        }

        // Need to do this out of the cover of the lock to avoid deadlocks
        if (localPipe != null) {
            localPipe.send(message);
        }
    }

    private synchronized void registerApplet(int appletID, Plugin plugin) {
        if (DEBUG) {
            System.out.println("JVMInstance.registerApplet for applet ID " + appletID + ", plugin " + plugin);
        }

        appletToPluginMap.put(new Integer(appletID), plugin);
        LiveConnectSupport.initialize(appletID, plugin);
        ModalitySupport.initialize(appletID, plugin);
    }

    private synchronized void unregisterApplet(int appletID) {
        if (DEBUG) {
            System.out.println("JVMInstance.unregisterApplet for applet ID " + appletID);
        }

        Integer key = new Integer(appletID);
        appletToPluginMap.remove(key);
        stopAckSet.remove(key);
        LiveConnectSupport.shutdown(appletID);
        ModalitySupport.shutdown(appletID);

        //Check if there is no more active applet
        //If so, send ShutdownJVMMessage to the client 
        if (isTainted()) {
            if (appletToPluginMap.isEmpty()) {
                if (DEBUG) {
                    System.out.println("JVM instance for " + javaInfo.getProductVersion() + " shutting down due to tainting");
                }
                shutdown();
            } else {
                if (DEBUG) {
                    System.out.println("JVM instance for " + javaInfo.getProductVersion() + " tainted, but still has " +
                                       appletToPluginMap.size() + " running applets");
                }
            }
        }
    }

    int mostRecentAppletID = -1;
    private synchronized Plugin getPluginForApplet(int appletID) {
        if (appletID < 0) {
            // Assume this request is coming from the "default" applet
            // execution context over on the client side. In general
            // this shouldn't happen, but we try to support it both
            // for compatibility and robustness.

            // Generally, see whether there's a most recent applet ID,
            // and if there isn't one, choose a random plugin
            // instance. The latter is somewhat risky (it might be
            // being torn down at the time we're trying to service the
            // request) but the consequences on the client side
            // (threads hanging waiting for a browser reply) should be
            // minimal, and eventually recoverable.
            Plugin res = null;
            if (mostRecentAppletID >= 0) {
                appletID = mostRecentAppletID;
                res = (Plugin) appletToPluginMap.get(new Integer(appletID));
            }

            if (res != null) {
                return res;
            }

            // Try to walk the map finding a non-null plugin instance
            for (Iterator iter = appletToPluginMap.keySet().iterator();
                 iter.hasNext(); ) {
                Integer key = (Integer) iter.next();
                res = (Plugin) appletToPluginMap.get(key);
                if (res != null) {
                    mostRecentAppletID = key.intValue();
                    return res;
                }
            }

            // Fall through at this point (will end up returning null)
        } else {
            mostRecentAppletID = appletID;
        }

        return (Plugin) appletToPluginMap.get(new Integer(appletID));
    }

    private class Listener implements JVMEventListener {
        public void jvmExited(JVMLauncher launcher) {
            synchronized(JVMInstance.this) {
                if (restartable && !gotJVMStartedMessage) {
                    restart();
                } else {
                    if (DEBUG) {
                        System.out.println("JVM instance for " + javaInfo.getProductVersion() + " exited");
                    }

                    dispose();
                }
            }
        }
    }

    private void restart() {
        started = false;
        // Only try restarting the target JVM one time
        restartable = false;
        disposePipe();
        realParams.clearUserArguments();
        launcher.clearUserArguments();
        try {
            startImpl(true);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void dispose() {
        // Unregister all running applets and wake up their plugins' main threads
        // in case they're waiting for a LiveConnect call to complete
        List runningAppletIDs = new ArrayList/*<Integer>*/();
        synchronized(this) {
            runningAppletIDs.addAll(appletToPluginMap.keySet());
        }
        for (Iterator iter = runningAppletIDs.iterator(); iter.hasNext(); ) {
            unregisterApplet(((Integer) iter.next()).intValue());
        }
        disposePipe();
    }

    private void disposePipe() {
        if (pipe != null) {
            pipe.shutdown();
            try {
                transportFactory.dispose();
            } catch (IOException e) {
                e.printStackTrace();
            }
            pipe = null;
            transportFactory = null;
        }
    }

    private class StreamMonitor implements Runnable {
        private String versionString;
        private InputStream istream;
        public StreamMonitor(InputStream stream) {
            istream = stream;
            versionString = javaInfo.getProductVersion().toString();
            new Thread(this, "JRE " + versionString + " Output Reader Thread").start();
        }

        public void run()
        {
            byte[] buffer = new byte[4096];
            try {
                int numRead = 0;
                do {
                    numRead = istream.read(buffer);
                    if (DEBUG) {
                        if (numRead > 0) {
                            System.out.print("JRE " + versionString + ": ");
                            System.out.write(buffer, 0, numRead);
                            System.out.flush();
                        }
                    }
                } while (numRead >= 0);
            }
            catch (IOException e) {
                try {
                    istream.close();
                } catch (IOException e2) {
                }
                // Should allow clean exit when process shuts down
            }
        }
    }
}
