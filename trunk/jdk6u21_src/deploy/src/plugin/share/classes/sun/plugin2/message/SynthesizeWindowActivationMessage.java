/*
 * @(#)SynthesizeWindowActivationMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent to force activation or deactivation of the
    EmbeddedFrame, to enable focus in the applet. */

public class SynthesizeWindowActivationMessage extends AppletMessage {
    public static final int ID = PluginMessages.SYNTHESIZE_WINDOW_ACTIVATION;

    private boolean active;

    /** For deserialization purposes */
    public SynthesizeWindowActivationMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a new SynthesizeWindowActivationMessage. The
        "active" argument indicates whether the applet's EmbeddedFrame
        is supposed to be activated or deactivated. */
    public SynthesizeWindowActivationMessage(Conversation c,
                                             int appletID,
                                             boolean active) {
        super(ID, c, appletID);
        this.active = active;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeBoolean(active);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        active = ser.readBoolean();
    }

    /** Indicates whether the applet's EmbeddedFrame is supposed to be
        activated or deactivated. */
    public boolean getActive() {
        return active;
    }
}
