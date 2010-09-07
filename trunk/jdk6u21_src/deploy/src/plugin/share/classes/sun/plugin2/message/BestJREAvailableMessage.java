/*
 * @(#)BestJREAvailableMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Message sent from client to server to query the best
    available jre version for request java version string. */

public class BestJREAvailableMessage extends PluginMessage {
    public static final int ID = PluginMessages.BEST_JRE_AVAILABLE;

    public static final int ASK = 1;
    public static final int REPLY = 2;

    private int kind;
    private String javaVersion;

    /** For deserialization purposes */
    public BestJREAvailableMessage(Conversation c) {
        super(ID, c);
    }

    public BestJREAvailableMessage(Conversation c, int kind, String javaVersion) {
	this(c);
	this.kind = kind;
	this.javaVersion = javaVersion;
    }

    public boolean isReply() {
	return (kind == REPLY);
    }

    public String getJavaVersion() {
	return javaVersion;
    }

    public void writeFields(Serializer ser) throws IOException {
	ser.writeInt(kind);
	ser.writeUTF(javaVersion);
    }

    public void readFields(Serializer ser) throws IOException {
	kind = ser.readInt();
	javaVersion = ser.readUTF();
    }

}
