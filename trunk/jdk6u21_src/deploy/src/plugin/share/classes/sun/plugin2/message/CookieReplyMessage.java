/*
 *  @(#)CookieReplyMessage.java	1.2 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;

/** Sent from the browser back to the client in response to a
    CookieOpMessage. */

public class CookieReplyMessage extends PluginMessage {
    public static final int ID = PluginMessages.COOKIE_REPLY;
   
    // If this was a get cookie operation
    private String cookie;
    // If an exception occurred
    private String exceptionMessage;

    public CookieReplyMessage(Conversation c) {
        super(ID, c);
    }
    
    public CookieReplyMessage(Conversation c, String cookie, String exceptionMessage) {
        this(c);
        this.cookie = cookie;
        this.exceptionMessage = exceptionMessage;
    }

    public String getCookie() {
        return cookie;
    }

    public String getExceptionMessage() {
        return exceptionMessage;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeUTF(cookie);
        ser.writeUTF(exceptionMessage);
    }
    
    public void readFields(Serializer ser) throws IOException {
        cookie = ser.readUTF();
        exceptionMessage = ser.readUTF();
    }
}
