/*
 *  @(#)URLHelper.java	1.2 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.helper;

import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import sun.plugin2.message.Serializer;

/** This class mirrors the java.net.URL object so that the URL related
 information could be transferred between processes.
*/

public final class URLHelper {
    
    /** Writes out given URL object using the given Serializer. */
    public static void write(Serializer ser, URL url) throws IOException {
        if (url == null) {
            ser.writeBoolean(false);
        } else {
            ser.writeBoolean(true);
            ser.writeUTF(url.getProtocol());
            ser.writeUTF(url.getHost());
            ser.writeInt(url.getPort());
            ser.writeUTF(url.getFile());
        }
    }

    /** Constructs a URL object based on the information passed from 
        the given Serializer. */
    public static URL read(Serializer ser) throws IOException {
        if (!ser.readBoolean()) {
            return null;
        }
        
        URL url  = null;
        try {
            // construct a new URL object using "new URL(protocol, host, port, file)".
            url = new URL(ser.readUTF(), ser.readUTF(), ser.readInt(), ser.readUTF());
        } catch (MalformedURLException ex) {
            ex.printStackTrace();
        }
        
        return url;
    }
}
