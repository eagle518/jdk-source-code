/*
 * @(#)JavaScriptSlotOpMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to perform an
    operation on a slot of a JavaScript object (get or set). The
    return value is sent in a JavaScriptReplyMessage.  Note that even
    for operations not having a return value (set and remove) we still
    need to wait for a reply in case we need to raise an exception. <P>

    Implementation note: some browsers can treat these messages
    identically to member operations, but some can not, so we have to
    distinguish between the two on this side.
*/

public class JavaScriptSlotOpMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_SLOT_OP;

    // Kinds of member operations
    public static final int GET    = 1;
    public static final int SET    = 2;

    private int slot;
    private int operationKind;
    // Optional argument
    private Object arg;

    /** For deserialization purposes */
    public JavaScriptSlotOpMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptSlotOpMessage with the given
        parameters. The argument (if non-null) may not be an arbitrary
        object, but must follow the conventions described in the
        {@link sun.plugin2.liveconnect.ArgumentHelper
        ArgumentHelper}. */
    public JavaScriptSlotOpMessage(Conversation c,
                                   BrowserSideObject object,
                                   int appletID,
                                   int slot,
                                   int operationKind,
                                   Object arg) {
        super(ID, c, object, appletID);
        this.slot = slot;
        this.operationKind = operationKind;
        this.arg = arg;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeInt(slot);
        ser.writeByte((byte) operationKind);
        ArgumentHelper.writeObject(ser, arg);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        slot = ser.readInt();
        operationKind = ser.readByte() & 0xFF;
        arg = ArgumentHelper.readObject(ser);
    }

    public int getSlot() {
        return slot;
    }

    /** Returns the kind of operation this message implies -- GET or
        SET. */
    public int getOperationKind() {
        return operationKind;
    }

    public Object getArgument() {
        return arg;
    }
}
