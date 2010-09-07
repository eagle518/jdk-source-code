/*
 * @(#)ModalityChangeMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Represents a change in the level of modality in a given applet. */

public class ModalityChangeMessage extends AppletMessage {
    public static final int ID = PluginMessages.MODALITY_CHANGE;

    private boolean modalityPushed;

    /** For deserialization purposes */
    public ModalityChangeMessage(Conversation c) {
        super(ID, c);
    }

    public ModalityChangeMessage(Conversation c,
                                 int appletID,
                                 boolean modalityPushed) {
        super(ID, c, appletID);
        this.modalityPushed = modalityPushed;
    }

    public boolean getModalityPushed() {
        return modalityPushed;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeBoolean(modalityPushed);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        modalityPushed = ser.readBoolean();
    }
}
