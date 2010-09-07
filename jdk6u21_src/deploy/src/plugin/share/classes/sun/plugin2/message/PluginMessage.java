/*
 * @(#)PluginMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

/** This marker class only serves to clearly delineate the messages
    the Java Plug-In sends. The underlying transport mechanism
    supports transmission of any arbitrary Message subclass and is not
    tied to the rest of the Java Plug-In. */

public abstract class PluginMessage extends Message {
    public PluginMessage(int id, Conversation conversation) {
        super(id, conversation);
    }
}
