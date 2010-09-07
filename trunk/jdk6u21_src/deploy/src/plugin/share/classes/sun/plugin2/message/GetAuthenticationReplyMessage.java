/*1
 * @(#)GetAuthenticationReplyMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.net.PasswordAuthentication;

/** Passes back authentication information from the web browser to the
    applet which requested it. */

public class GetAuthenticationReplyMessage extends PluginMessage {
    public static final int ID = PluginMessages.GET_AUTHENTICATION_REPLY;

    private String  username;
    private char[]  password;
    private String  errorMessage;

    /** For deserialization purposes */
    public GetAuthenticationReplyMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a new GetAuthenticationReplyMessage. If "auth" is
        non-null, then the errorMessage must be null and
        vice-versa. */
    public GetAuthenticationReplyMessage(Conversation c,
                                         PasswordAuthentication auth,
                                         String errorMessage) {
        this(c);
        if (auth != null) {
            this.username = auth.getUserName();
            this.password = auth.getPassword();
        }
        this.errorMessage = errorMessage;
    }

    public PasswordAuthentication getAuthentication() {
        if (username == null)
            return null;
        return new PasswordAuthentication(username, password);
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeUTF(username);
        ser.writeCharArray(password);
        ser.writeUTF(errorMessage);
    }

    public void readFields(Serializer ser) throws IOException {
        username = ser.readUTF();
        password = ser.readCharArray();
        errorMessage = ser.readUTF();
    }
}
