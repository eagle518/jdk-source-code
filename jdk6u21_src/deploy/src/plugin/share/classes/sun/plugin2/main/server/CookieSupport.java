/*
 * @(#)CookieSupport.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import sun.plugin2.message.*;

import com.sun.deploy.net.cookie.CookieUnavailableException;

public class CookieSupport {
    public static CookieReplyMessage getCookieReply(Plugin plugin, CookieOpMessage cookieMsg) {
        try {
            switch (cookieMsg.getOperationKind()) {
                case CookieOpMessage.GET_COOKIE: {
                    return new CookieReplyMessage(cookieMsg.getConversation(),
                                                  plugin.getCookie(cookieMsg.getURL()),
                                                  null);
                }

                case CookieOpMessage.SET_COOKIE: {
                    plugin.setCookie(cookieMsg.getURL(), cookieMsg.getCookie());
                    return new CookieReplyMessage(cookieMsg.getConversation(),
                                                  null,
                                                  null);
                }

                default: {
                    return new CookieReplyMessage(cookieMsg.getConversation(),
                                                  null,
                                                  "Error: unknown cookie operation kind " + cookieMsg.getOperationKind());
                }
            }
        } catch (CookieUnavailableException e) {
            return new CookieReplyMessage(cookieMsg.getConversation(),
                                          null,
                                          e.toString());
        }
    }
}
