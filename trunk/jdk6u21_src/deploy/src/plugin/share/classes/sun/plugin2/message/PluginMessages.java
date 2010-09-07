/*
 * @(#)PluginMessages.java	1.20 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import sun.plugin2.message.transport.*;

/** Enumerates the message IDs the Java Plug-In sends. Note that the
    reason ints are used instead of Enums, for example, is that this
    code needs to be able to run on the JDK 1.4 platform. */

public class PluginMessages {
    // Note that we keep these constants package-private to simplify
    // the external interface outside this package to the ID fields of
    // the individual message classes

    // Basic functionality
    static final int SET_JVM_ID       = 1;
    static final int JVM_STARTED_ID   = 2;
    static final int START_APPLET     = 3;
    static final int START_APPLET_ACK = 4;
    static final int SET_APPLET_SIZE  = 5;
    static final int SET_CHILD_WINDOW_HANDLE = 6;
    static final int SYNTHESIZE_WINDOW_ACTIVATION = 7;
    static final int PRINT_APPLET     = 8;
    static final int PRINT_APPLET_REPLY = 9;
    static final int PRINT_BAND       = 10;
    static final int PRINT_BAND_REPLY = 11;
    static final int STOP_APPLET      = 12;
    static final int STOP_APPLET_ACK  = 13;
    static final int SHUTDOWN_JVM     = 14;
    static final int HEARTBEAT        = 15;
    static final int MARK_JVM_TAINTED  = 16;
    static final int BEST_JRE_AVAILABLE = 17;

    // LiveConnect functionality (Java -> JavaScript)
    static final int JAVASCRIPT_GET_WINDOW = 21;
    static final int JAVASCRIPT_CALL       = 22;
    static final int JAVASCRIPT_EVAL       = 23;
    static final int JAVASCRIPT_MEMBER_OP  = 24;
    static final int JAVASCRIPT_SLOT_OP    = 25;
    static final int JAVASCRIPT_TO_STRING  = 26;
    static final int JAVASCRIPT_REPLY      = 27;
    static final int JAVASCRIPT_RELEASE_OBJECT = 28;

    // LiveConnect functionality (JavaScript -> Java)
    static final int GET_APPLET            = 31;
    static final int GET_NAMESPACE         = 32;
    static final int JAVA_OBJECT_OP        = 33;
    static final int JAVA_REPLY            = 34;
    static final int RELEASE_REMOTE_OBJECT = 35;

    // Proxy, Cookie, and browser authentication related messages
    static final int GET_PROXY                = 41;
    static final int PROXY_REPLY              = 42;
    static final int GET_AUTHENTICATION       = 43;
    static final int GET_AUTHENTICATION_REPLY = 44;
    static final int COOKIE_OP                = 45;
    static final int COOKIE_REPLY             = 46;

    // AppletContext related messages
    static final int SHOW_DOCUMENT         = 51;
    static final int SHOW_STATUS           = 52;

    // Modality related messages
    static final int MODALITY_CHANGE       = 61;

    private PluginMessages() {}

    public static void register(SerializingTransport transport) {
        // Basic messages
        transport.registerMessageID(SetJVMIDMessage.ID,                SetJVMIDMessage.class);
        transport.registerMessageID(JVMStartedMessage.ID,              JVMStartedMessage.class);
        transport.registerMessageID(StartAppletMessage.ID,             StartAppletMessage.class);
        transport.registerMessageID(StartAppletAckMessage.ID,          StartAppletAckMessage.class);
        transport.registerMessageID(SetAppletSizeMessage.ID,           SetAppletSizeMessage.class);
        transport.registerMessageID(SetChildWindowHandleMessage.ID,    SetChildWindowHandleMessage.class);
        transport.registerMessageID(SynthesizeWindowActivationMessage.ID, SynthesizeWindowActivationMessage.class);
        transport.registerMessageID(PrintAppletMessage.ID,             PrintAppletMessage.class);
        transport.registerMessageID(PrintAppletReplyMessage.ID,        PrintAppletReplyMessage.class);
        transport.registerMessageID(PrintBandMessage.ID,               PrintBandMessage.class);
        transport.registerMessageID(PrintBandReplyMessage.ID,          PrintBandReplyMessage.class);
        transport.registerMessageID(StopAppletMessage.ID,              StopAppletMessage.class);
        transport.registerMessageID(StopAppletAckMessage.ID,           StopAppletAckMessage.class);
        transport.registerMessageID(ShutdownJVMMessage.ID,             ShutdownJVMMessage.class);
        transport.registerMessageID(HeartbeatMessage.ID,               HeartbeatMessage.class);
        transport.registerMessageID(MarkTaintedMessage.ID,             MarkTaintedMessage.class);
	transport.registerMessageID(BestJREAvailableMessage.ID,	       BestJREAvailableMessage.class);

        // LiveConnect messages (Java -> JavaScript)
        transport.registerMessageID(JavaScriptGetWindowMessage.ID,     JavaScriptGetWindowMessage.class);
        transport.registerMessageID(JavaScriptCallMessage.ID,          JavaScriptCallMessage.class);
        transport.registerMessageID(JavaScriptEvalMessage.ID,          JavaScriptEvalMessage.class);
        transport.registerMessageID(JavaScriptMemberOpMessage.ID,      JavaScriptMemberOpMessage.class);
        transport.registerMessageID(JavaScriptSlotOpMessage.ID,        JavaScriptSlotOpMessage.class);
        transport.registerMessageID(JavaScriptToStringMessage.ID,      JavaScriptToStringMessage.class);
        transport.registerMessageID(JavaScriptReplyMessage.ID,         JavaScriptReplyMessage.class);
        transport.registerMessageID(JavaScriptReleaseObjectMessage.ID, JavaScriptReleaseObjectMessage.class);

        // LiveConnect messages (JavaScript -> Java)
        transport.registerMessageID(GetAppletMessage.ID,           GetAppletMessage.class);
        transport.registerMessageID(GetNameSpaceMessage.ID,        GetNameSpaceMessage.class);
        transport.registerMessageID(JavaObjectOpMessage.ID,        JavaObjectOpMessage.class);
        transport.registerMessageID(JavaReplyMessage.ID,           JavaReplyMessage.class);
        transport.registerMessageID(ReleaseRemoteObjectMessage.ID, ReleaseRemoteObjectMessage.class);

        // Browser service related messages
        transport.registerMessageID(GetProxyMessage.ID,   GetProxyMessage.class);
        transport.registerMessageID(ProxyReplyMessage.ID, ProxyReplyMessage.class);
        transport.registerMessageID(GetAuthenticationMessage.ID,      GetAuthenticationMessage.class);
        transport.registerMessageID(GetAuthenticationReplyMessage.ID, GetAuthenticationReplyMessage.class);
        transport.registerMessageID(CookieOpMessage.ID,    CookieOpMessage.class);
        transport.registerMessageID(CookieReplyMessage.ID, CookieReplyMessage.class);

        // AppletContext related messages
        transport.registerMessageID(ShowDocumentMessage.ID, ShowDocumentMessage.class);
        transport.registerMessageID(ShowStatusMessage.ID, ShowStatusMessage.class);

        // Modality related messages
        transport.registerMessageID(ModalityChangeMessage.ID, ModalityChangeMessage.class);
    }
}
