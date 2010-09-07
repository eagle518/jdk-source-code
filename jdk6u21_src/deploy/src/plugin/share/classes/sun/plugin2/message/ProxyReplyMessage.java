/*
 *  @(#)ProxyReplyMessage.java	1.3 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.util.*;
import java.net.Proxy;

import sun.plugin2.message.Serializer;
import sun.plugin2.message.helper.ProxyHelper;

/** Message sent from the browser back to the client in reply to the
    GetBrowserProxyInfoMessage request. 
*/

public class ProxyReplyMessage extends PluginMessage {
    public static final int ID = PluginMessages.PROXY_REPLY;

    /* result should be type of List<java.net.Proxy> */
    private List      proxyList;
    private int       listSize;

    public ProxyReplyMessage(Conversation c) {
        super(ID, c);
    }

    public ProxyReplyMessage(Conversation c, List proxyList) {
        this(c);
        
        if (proxyList == null) {
            // If the passed in proxyList is null in case, construct
            // an ArrayList and add Proxy.NO_PROXY to it.
            listSize = 1;
            this.proxyList = new ArrayList();
            this.proxyList.add(Proxy.NO_PROXY);
        } else {
            this.proxyList = proxyList;
            this.listSize  = proxyList.size();
        }
    }

    // The returned result should NOT be null. 
    public List getProxyList() {
        return proxyList;
    }
    
    // The browser side calls this method to write the List<Proxy>.
    public void writeFields(Serializer ser) throws IOException {
        ser.writeInt(listSize);
        Iterator/*<Proxy>*/ iter = proxyList.iterator();
        while (iter.hasNext()) {
            Proxy p = (Proxy)iter.next();
            ProxyHelper.write(ser, p);
        }
    }

    // The applet side calls this method to retrieve the proxy list.
    // Notes:
    public void readFields(Serializer ser) throws IOException {
        listSize = ser.readInt();
        
        proxyList = new ArrayList(listSize);
        for (int i = 0; i < listSize; i++) {
            proxyList.add(ProxyHelper.read(ser));
        }
    }
}
