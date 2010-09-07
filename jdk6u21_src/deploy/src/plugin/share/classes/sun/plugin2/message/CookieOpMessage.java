/*
 *  @(#)CookieOpMessage.java	1.3 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;
import java.net.URL;
import sun.plugin2.message.helper.URLHelper;

/** Used by the client JVM instances to get or set cookies in the browser. */

public class CookieOpMessage extends AppletMessage {
    public static final int ID = PluginMessages.COOKIE_OP;
   
    public static final int GET_COOKIE = 1;
    public static final int SET_COOKIE = 2;

    private int operationKind;
    private URL url;
    private String cookie;
    
    public CookieOpMessage(Conversation c) {
        super(ID, c);
    }
    
    public CookieOpMessage(Conversation c, int appletID, int operationKind, URL url, String cookie) 
        throws IllegalArgumentException {

        super(ID, c, appletID);
        if (operationKind != GET_COOKIE && operationKind != SET_COOKIE) {
            throw new IllegalArgumentException("Illegal operationKind");
        }

        if (url == null) {
            throw new IllegalArgumentException("Null URL");
        }
        
        this.operationKind = operationKind;
        this.url = url;
        this.cookie = cookie;
    }

    public int getOperationKind() {
        return operationKind;
    }

    public URL getURL() {
        return url;
    }

    public String getCookie() {
        return cookie;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeInt(operationKind);
        URLHelper.write(ser, url);
        ser.writeUTF(cookie);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        operationKind = ser.readInt();
        url = URLHelper.read(ser);
        cookie = ser.readUTF();
    }
}
