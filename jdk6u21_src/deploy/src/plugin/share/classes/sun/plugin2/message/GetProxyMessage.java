/*
 *  @(#)GetProxyMessage.java	1.4 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.IOException;
import java.net.URL;
import sun.plugin2.message.helper.URLHelper;

/** Used by the client JVM instances to request the browser proxy info. */

public class GetProxyMessage extends AppletMessage {
    public static final int ID = PluginMessages.GET_PROXY;
   
    private URL url;
    private boolean isSocketURI;
    
    public GetProxyMessage(Conversation c) {
        super(ID, c);
    }
    
    public GetProxyMessage(Conversation c, int appletID, URL url, boolean isSocketURI) 
        throws IllegalArgumentException {

        super(ID, c, appletID);
        if (url == null) {
            throw new IllegalArgumentException();
        }
        
        this.url = url;
        this.isSocketURI = isSocketURI;
    }

    public URL getURL() {
        return url;
    }

    public boolean isSocketURI() {
        return isSocketURI;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        URLHelper.write(ser, url);
        ser.writeBoolean(isSocketURI);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        url = URLHelper.read(ser);
        isSocketURI = ser.readBoolean();
    }
}

