/*
 *  @(#)ShowDocumentMessage.java	1.3 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;
import java.net.URL;
import sun.plugin2.message.helper.URLHelper;


/**
 * This class encapsulate the request message from client side to 
 * show the document specified by a URL and a target (e.g. _self or
 * parent etc). 
 * 
 * Notes:
 *   The "url" field is a String type instead of URL type due to the fact
 * that some URL schemes can only be recognized by java.net.URL when an appropriate
 * stream handler has been installed. As part of the applet start up environment
 * setup, the "com.sun.deploy.net.protocol" is set as the value of property
 * "java.protocol.handler.pkgs" and as a result, any URLStreamHandler inside 
 * that package will be considered to handle a specific scheme. A typical example
 * will be "javascript:" scheme. Since the server side process, i.e. browser process, 
 * won't add anything like that, it would throw MalformedURLException if URL type
 * is used for the "url" field when "readFields" is called. For that matter,
 * a String type is simply used for that field.
 */
  
public class ShowDocumentMessage extends AppletMessage {

    public static final int ID = PluginMessages.SHOW_DOCUMENT;

    private String url;
    private String target;

    public ShowDocumentMessage(Conversation c) {
        super(ID, c);
    }

    public ShowDocumentMessage(Conversation c, int appletID, 
                               String url, String target) {

        super(ID, c, appletID);
        this.url = url;
        this.target = target;
    }

    public String getURL() {
        return url;
    }

    public String getTarget() {
        return target;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(url);
        ser.writeUTF(target);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        url      = ser.readUTF();
        target   = ser.readUTF();
    }
}
