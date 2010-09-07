/*
 * @(#)SetJVMIDMessage.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Initial message which provides the JVM ID to the subordinate JVM
    process. Used to identify RemoteJavaObjects passed back to the
    browser. Also used to carry other one-time initialization
    information. */
public class SetJVMIDMessage extends PluginMessage {
    public static final int ID = PluginMessages.SET_JVM_ID;

    private int jvmID;

    // The type of browser we're attached to; see sun.plugin2.util.BrowserType
    private int browserType;

    // Indicates whether this JVM was started on behalf of a single
    // applet specifying the separate_jvm parameter
    private boolean separateJVM;

    // The jvm command line properties
    private String[][] properties;

    // user home directory - relevant to windows platform only
    private String userHome;

    /** For deserialization purposes */
    public SetJVMIDMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a SetJVMIDMessage with the given parameters. */
    public SetJVMIDMessage(Conversation c,
                           int jvmID,
                           int browserType,
                           boolean separateJVM,
                           String userHome,
			   String [][] params) {
        super(ID, c);
        this.jvmID = jvmID;
        this.browserType = browserType;
        this.separateJVM = separateJVM;
        this.userHome = userHome;

	// Copy to the internal 2D array of properties
        properties = new String[params.length][];
        for (int i = 0; i < params.length; i++) {
            properties[i] = (String[]) params[i].clone();
        } 
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeInt(jvmID);
        ser.writeInt(browserType);
        ser.writeBoolean(separateJVM);
        ser.writeUTF(userHome);
	// write the number of properties lists
	// read side need this number to allocate array memory
	ser.writeInt(properties.length);
	for (int i = 0; i < properties.length; i++) {
	    ser.writeUTFArray(properties[i]);
	}
    }
    
    public void readFields(Serializer ser) throws IOException {
        jvmID = ser.readInt();
        browserType = ser.readInt();
        separateJVM = ser.readBoolean();
        userHome = ser.readUTF();
	int numberOfArray = ser.readInt();
	properties = new String[numberOfArray][];
	for (int i = 0; i < properties.length; i++) {
	    properties[i] = ser.readUTFArray();
	}
    }

    public int getJVMID() {
        return jvmID;
    }

    public int getBrowserType() {
        return browserType;
    }

    public boolean isSeparateJVM() {
        return separateJVM;
    }

    public String getUserHome() {
        return userHome;
    }

    public String[][] getParameters() {
	return properties;
    }
}
