/*
 * @(#)MessagePassingJSObject.java	1.13 10/05/21
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.security.*;

import netscape.javascript.*;

import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.*;
import sun.plugin2.applet.Plugin2Manager;

// FIXME: need to support swapping in of the NoopExecutionContext to
// disallow LiveConnect for a particular applet

/** Implementation of JSObject which delegates back to the web browser
    via our message passing system. */

public class MessagePassingJSObject extends JSObject {
    private BrowserSideObject object;
    // The ID of the applet which originally received this object
    private int appletID;
    private Pipe pipe;
    private Plugin2Manager manager;
    private static final boolean DEBUG;

    static {
        DEBUG = (AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return SystemUtil.getenv("JPI_PLUGIN2_DEBUG");
                }
            }) != null);
    }

    public MessagePassingJSObject(BrowserSideObject object, int appletID, Pipe pipe) {
        this.object = object;
        this.appletID = appletID;
        this.pipe = pipe;
        this.manager = null;
    }

    public MessagePassingJSObject(BrowserSideObject object, int appletID, Pipe pipe, Plugin2Manager manager) {
        this.object = object;
        this.appletID = appletID;
        this.pipe = pipe;
        this.manager = manager;
    }

    public BrowserSideObject getBrowserSideObject() {
        return object;
    }

    public int getAppletID() {
        return appletID;
    }

    public Plugin2Manager getManager() {
        return manager;
    }

    public Object call(String methodName, Object args[]) throws JSException {

        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        if ("eval".equals(methodName) && concatenatable(args)) {
            return eval(concat(args));
        }

        Conversation c = pipe.beginConversation();
        try {
            JavaScriptCallMessage msg = new JavaScriptCallMessage(c,
                                                                  object,
                                                                  appletID,
                                                                  methodName,
                                                                  convert(args));
            pipe.send(msg);
            return waitForReply(c);
        } catch (IOException e) {
            throw newJSException(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    public Object eval(String s) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        Conversation c = pipe.beginConversation();
        try {
            JavaScriptEvalMessage msg = new JavaScriptEvalMessage(c,
                                                                  object,
                                                                  appletID,
                                                                  s);
            pipe.send(msg);
            return waitForReply(c);
        } catch (IOException e) {
            throw newJSException(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    public Object getMember(String name) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        return doMemberOp(name, JavaScriptMemberOpMessage.GET, null);
    }

    public void setMember(String name, Object value) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        doMemberOp(name, JavaScriptMemberOpMessage.SET, value);
    }

    public void removeMember(String name) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        doMemberOp(name, JavaScriptMemberOpMessage.REMOVE, null);
    }

    public Object getSlot(int index) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        return doSlotOp(index, JavaScriptSlotOpMessage.GET, null);
    }

    public void setSlot(int index, Object value) throws JSException {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        doSlotOp(index, JavaScriptSlotOpMessage.SET, value);
    }

    public String toString() {
        if (manager != null) {
            manager.decreaseJava2JSCounter();
        }

        Conversation c = pipe.beginConversation();
        try {
            JavaScriptToStringMessage msg = new JavaScriptToStringMessage(c,
                                                                          object,
                                                                          appletID);
            pipe.send(msg);
            return (String) waitForReply(c);
        } catch (IOException e) {
            throw newJSException(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    // Waits for a reply message from the browser-side JVM. Note that
    // round-trip Java-to-JavaScript calls may occur at this time, and
    // we need to handle them.
    private Object waitForReply(Conversation c) throws JSException {
        while (true) {
            try {
                Message msg = pipe.receive(0, c);
                switch (msg.getID()) {
                    case JavaScriptReplyMessage.ID: {
                        JavaScriptReplyMessage reply = (JavaScriptReplyMessage) msg;
                        if (reply.getExceptionMessage() != null) {
                            throw newJSException(reply.getExceptionMessage());
                        }
                        return LiveConnectSupport.importObject(reply.getResult(), appletID);
                    }

                    case JavaObjectOpMessage.ID: {
                        try {
                            LiveConnectSupport.doObjectOp((JavaObjectOpMessage) msg);
                            break;
                        } catch (IOException e) {
                            throw newJSException(e);
                        }
                    }

                    default:
                        throw newJSException("Unexpected reply message ID " + msg.getID() + " from web browser");
                }
            } catch (IOException e) {
                // Probably shouldn't see this happen unless something goes wrong
                throw newJSException(e);
            } catch (InterruptedException e) {
                // Probably shouldn't see this happen unless something goes wrong
                throw newJSException(e);
            }
        }
    }


    // Unwraps MessagePassingJSObjects, wraps Java objects into RemoteJavaObjects, etc.
    private Object[] convert(Object[] args) {
        // We always copy the incoming argument array since call() shouldn't mutate its arguments
        if (args == null)
            return null;

        Object[] newArgs = new Object[args.length];
        for (int i = 0; i < args.length; i++) {
            newArgs[i] = LiveConnectSupport.exportObject(args[i], appletID, false, false);
        }
        return newArgs;
    }

    private Object doMemberOp(String memberName, int kind, Object arg) throws JSException {
        Conversation c = pipe.beginConversation();
        try {
            JavaScriptMemberOpMessage msg = new JavaScriptMemberOpMessage(c,
                                                                          object,
                                                                          appletID,
                                                                          memberName,
                                                                          kind,
                                                                          LiveConnectSupport.exportObject(arg, appletID, false, false));
            pipe.send(msg);
            return waitForReply(c);
        } catch (IOException e) {
            throw newJSException(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    private Object doSlotOp(int slot, int kind, Object arg) throws JSException {
        Conversation c = pipe.beginConversation();
        try {
            JavaScriptSlotOpMessage msg = new JavaScriptSlotOpMessage(c,
                                                                      object,
                                                                      appletID,
                                                                      slot,
                                                                      kind,
                                                                      LiveConnectSupport.exportObject(arg, appletID, false, false));
            pipe.send(msg);
            return waitForReply(c);
        } catch (IOException e) {
            throw newJSException(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    private JSException newJSException(Exception cause) {
        JSException res = (JSException) new JSException().initCause(cause);
        if (DEBUG) {
            res.printStackTrace();
        }
        return res;
    }

    private JSException newJSException(String message) {
        JSException res = new JSException(message);
        if (DEBUG) {
            res.printStackTrace();
        }
        return res;
    }

    private boolean concatenatable(Object[] args) {
        if (args == null) {
            return true;
        }

        for (int i = 0; i < args.length; i++) {
            if (!(args[i] instanceof String)) {
                return false;
            }
        }

        return true;
    }

    private String concat(Object[] args) {
        StringBuffer buf = new StringBuffer();
        if (args != null) {
            for (int i = 0; i < args.length; i++) {
                if (args[i] != null) {
                    buf.append(args[i]);
                    if (i < args.length - 1) {
                        buf.append(" ");
                    }
                }
            }
        }
        return buf.toString();
    }
}
