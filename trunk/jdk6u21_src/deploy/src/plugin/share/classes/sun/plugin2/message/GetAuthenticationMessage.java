/*
 * @(#)GetAuthenticationMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.net.*;

/** Requests authentication from the web browser for a
    username/password associated with a network connection, typically
    through a proxy or similar. The result is sent in a
    GetAuthenticationReplyMessage. */

public class GetAuthenticationMessage extends AppletMessage {
    public static final int ID = PluginMessages.GET_AUTHENTICATION;

    private String  protocol;
    private String  host;
    private int     port;
    private String  scheme;
    private String  realm;
    private String  requestURL;
    private boolean proxyAuthentication;

    /** For deserialization purposes */
    public GetAuthenticationMessage(Conversation c) {
        super(ID, c);
    }

    public GetAuthenticationMessage(Conversation c,
                                    int appletID,
                                    String protocol,
                                    String host,
                                    int port, 
                                    String scheme,
                                    String realm,
                                    URL requestURL,
                                    boolean proxyAuthentication) {
        super(ID, c, appletID);
        this.protocol = protocol;
        this.host = host;
        this.port = port;
        this.scheme = scheme;
        this.realm = realm;
        this.requestURL = requestURL.toExternalForm();
        this.proxyAuthentication = proxyAuthentication;
    }

    public String  getProtocol()            { return protocol;            }
    public String  getHost()                { return host;                }
    public int     getPort()                { return port;                }
    public String  getScheme()              { return scheme;              }
    public String  getRealm()               { return realm;               }
    public URL     getRequestURL() {
        try {
            return new URL(requestURL);
        } catch (MalformedURLException e) {
            e.printStackTrace();
            return null;
        }
    }
    public boolean getProxyAuthentication() { return proxyAuthentication; }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(protocol);
        ser.writeUTF(host);
        ser.writeInt(port);
        ser.writeUTF(scheme);
        ser.writeUTF(realm);
        ser.writeUTF(requestURL);
        ser.writeBoolean(proxyAuthentication);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        protocol = ser.readUTF();
        host = ser.readUTF();
        port = ser.readInt();
        scheme = ser.readUTF();
        realm = ser.readUTF();
        requestURL = ser.readUTF();
        proxyAuthentication = ser.readBoolean();
    }
}
