/*
 * @(#)JavaScriptBaseMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Abstract base class for most JavaScript requests going from the
    client JVM instances over to the browser. */

public abstract class JavaScriptBaseMessage extends AppletMessage {
    private BrowserSideObject object;

    /** For deserialization purposes */
    protected JavaScriptBaseMessage(int id, Conversation conversation) {
        super(id, conversation);
    }

    /** For construction purposes */
    protected JavaScriptBaseMessage(int id,
                                    Conversation conversation,
                                    BrowserSideObject object,
                                    int appletID) {
        super(id, conversation, appletID);
        this.object = object;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        BrowserSideObject.write(ser, object);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        object = BrowserSideObject.read(ser);
    }
    
    public BrowserSideObject getObject() {
        return object;
    }
}
