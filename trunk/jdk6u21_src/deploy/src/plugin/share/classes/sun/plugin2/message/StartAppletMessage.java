/*
 * @(#)StartAppletMessage.java	1.7 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.util.*;

/** A request to start an applet with the specified parameters, applet
    ID, and parent containing window. */

public class StartAppletMessage extends PluginMessage {
    public static final int ID = PluginMessages.START_APPLET;

    // The applet parameters, specified as separate key / value arrays
    private String[] keys;
    private String[] values;
    // The integer ID that should be associated with this applet in the target JVM
    private int appletID;
    // The native window handle / widget into which we embed the applet
    private long parentNativeWindowHandle;
    // The parent connection is a Mac OS X-specific construct
    private long parentConnection;
    // If we're on X11 platforms, this indicates whether we should
    // attempt to use XEmbed support
    private boolean useXEmbed;
    // plugin document base
    private String docbase;
    // Indicates whether or not this is a "dummy applet" used only for
    // scripting purposes in the Firefox browser
    private boolean isForDummyApplet;

    /** For deserialization purposes */
    public StartAppletMessage(Conversation c) {
        super(ID, c);
    }

    /** Creates a new message with the given parameters. A copy of the
        parameters is made internally, so subsequent changes will not
        be visible. */
    public StartAppletMessage(Conversation c,
                              Map/*<String,String>*/ parameters,
                              long parentNativeWindowHandle,
                              long parentConnection,
                              boolean useXEmbedOnX11Platforms,
                              int appletID, String docbase,
                              boolean isForDummyApplet) {
        this(c);
        setParameters(parameters);
        this.parentNativeWindowHandle = parentNativeWindowHandle;
        this.parentConnection = parentConnection;
        this.useXEmbed = useXEmbedOnX11Platforms;
        this.appletID = appletID;
        this.docbase = docbase;
        this.isForDummyApplet = isForDummyApplet;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeUTFArray(keys);
        ser.writeUTFArray(values);
        ser.writeLong(parentNativeWindowHandle);
        ser.writeLong(parentConnection);
        ser.writeBoolean(useXEmbed);
        ser.writeInt(appletID);
        ser.writeUTF(docbase);
        ser.writeBoolean(isForDummyApplet);
    }

    public void readFields(Serializer ser) throws IOException {
        keys = ser.readUTFArray();
        values = ser.readUTFArray();
        parentNativeWindowHandle = ser.readLong();
        parentConnection = ser.readLong();
        useXEmbed = ser.readBoolean();
        appletID = ser.readInt();
        docbase = ser.readUTF();
        isForDummyApplet = ser.readBoolean();
    }

    /** Reset applet parameters.
      */
    public void setParameters(Map/*<String,String>*/ parameters) {
        List/*<String>*/ keyList = new ArrayList/*<String>*/();
        List/*<String>*/ valueList = new ArrayList/*<String>*/();
        keyList.addAll(parameters.keySet());
        for (Iterator iter = keyList.iterator(); iter.hasNext(); ) {
            valueList.add(parameters.get(iter.next()));
        }
        keys = (String[]) keyList.toArray(new String[0]);
        values = (String[]) valueList.toArray(new String[0]);
    }

    /** Returns a newly-constructed Map of Strings to Strings
        representing the applet parameters. The caller should cache
        this map rather than calling this method over and over
        again. */
    public Map/*<String,String>*/ getParameters() {
        Map/*<String,String>*/ parameters = new HashMap/*<String,String>*/();
        for (int i = 0; i < keys.length; i++) {
            parameters.put(keys[i], values[i]);
        }
        return parameters;
    }

    /** Returns the parent's native window handle into which we should
        embed the applet we create. */
    public long getParentNativeWindowHandle() {
        return parentNativeWindowHandle;
    }

    /** Returns the parent connection -- this is a Mac OS X-specific
        construct needed in conjunction with the window handle. */
    public long getParentConnection() {
        return parentConnection;
    }

    /** On X11 platforms, indicates whether we should use XEmbed
        support. */
    public boolean useXEmbed() {
        return useXEmbed;
    }

    /** Returns the integer ID associated with this applet. */
    public int getAppletID() {
        return appletID;
    }

    /** Returns the document base associated with this applet. */
    public String getDocumentBase() {
      return docbase;
    }

    /** Indicates whether this message represents a start of a
        so-called "dummy applet" -- see {@link
        sun.plugin2.applet.Applet2Manager#setForDummyApplet}. */
    public boolean isForDummyApplet() {
        return isForDummyApplet;
    }
 }
