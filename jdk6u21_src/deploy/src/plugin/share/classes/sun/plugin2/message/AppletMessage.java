/*
 * @(#)AppletMessage.java	1.1 07/06/18
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;

/** This marker class only serves to clearly delineate the messages
    the Java Plug-In sends to a valid started applet. 
    The underlying transport mechanism
    supports transmission of any arbitrary Message subclass and is not
    tied to the rest of the Java Plug-In. */

public abstract class AppletMessage extends PluginMessage {
    private int appletID;

    public AppletMessage(int id, Conversation conversation) {
        super(id, conversation);
    }

    public AppletMessage(int id, Conversation conversation, int appletID) {
        super(id, conversation);
        this.appletID = appletID;
    }

    public int getAppletID() {
        return appletID;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeInt(appletID);
    }
    
    public void readFields(Serializer ser) throws IOException {
        appletID = ser.readInt();
    }
}
