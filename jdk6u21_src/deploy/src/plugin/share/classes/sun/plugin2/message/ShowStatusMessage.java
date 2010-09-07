/*
 * @(#)ShowStatusMessage.java	1.4 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;

/**
 * This class encapsulate the request message from client side to 
 * show the applet status messages.
 */

public class ShowStatusMessage extends AppletMessage {
    
    public static final int ID = PluginMessages.SHOW_STATUS;
    
    private String status;
    
    public ShowStatusMessage(Conversation c) {
        super(ID, c);
    }
    
    public ShowStatusMessage(Conversation c, int appletID, 
			     String status) {
        super(ID, c, appletID);
        this.status = status;
    }
    
    public String getStatus() {
        return status;
    }
    
    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(status);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        status   = ser.readUTF();
    }
}
