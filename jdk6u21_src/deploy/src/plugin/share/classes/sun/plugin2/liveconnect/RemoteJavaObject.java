/*
 * @(#)RemoteJavaObject.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.liveconnect;

import java.io.*;

import sun.plugin2.message.*;

/** Represents a Java object in a remote (attached) JVM instance. */

public class RemoteJavaObject {
    // The ID of the JVM instance in whose context this object exists.
    // We prefer to use a JVM ID rather than an applet ID because it's
    // legal usage to pass Java objects exposed to JavaScript between
    // applets, and we don't want the complexity associated with
    // associating an object with one applet, having that applet shut
    // down, and then passing the same object into another applet.
    // Note that this implementation requires that Java objects
    // exposed to JavaScript are only passed back to the same JVM
    // instance in which they are hosted..
    private int jvmID;

    // The ID of the applet which exposed this object. This is used to
    // determine the protection domain of future invocations against
    // the object.
    private int appletID;

    // The ID of the object within the target JVM. Note that the fact
    // that this is an int restricts the number of exported objects to
    // 2^32. This is not expected to be a limitation in practice, but
    // if it is, this could trivially be expanded out to a long on
    // both sides.
    private int objectID;

    // An indication of whether this RemoteJavaObject represents the
    // top-level object exported to the browser, i.e., the applet. We
    // can use this flag to attach "magic" properties to only this
    // object, for example the scoped Packages keyword.
    private boolean isApplet;

    public RemoteJavaObject(int jvmID,
                            int appletID,
                            int objectID,
                            boolean isApplet) {
        this.jvmID = jvmID;
        this.appletID = appletID;
        this.objectID = objectID;
        this.isApplet = isApplet;
    }

    public boolean equals(Object o) {
        if ((o == null) || (getClass() != o.getClass()))
            return false;

        RemoteJavaObject other = (RemoteJavaObject) o;
        return (jvmID == other.jvmID &&
                appletID == other.appletID &&
                objectID == other.objectID &&
                isApplet == other.isApplet);
    }

    public int hashCode() {
        return jvmID ^ appletID ^ objectID;
    }

    /** Returns the JVM ID in whose context this object was created. */
    public int getJVMID() {
        return jvmID;
    }

    /** Returns the applet ID in whose context this object was created. */
    public int getAppletID() {
        return appletID;
    }

    /** Returns the ID of the object in the remote JVM instance. */
    public int getObjectID() {
        return objectID;
    }

    /** Indicates whether this RemoteJavaObject represents the
        top-level object exported to the browser, i.e., the applet. */
    public boolean isApplet() {
        return isApplet;
    }

    /** Writes the given RemoteJavaObject to the given Serializer. */
    public static void write(Serializer ser, RemoteJavaObject obj) throws IOException {
        if (obj == null) {
            ser.writeBoolean(false);
        } else {
            ser.writeBoolean(true);
            ser.writeInt(obj.getJVMID());
            ser.writeInt(obj.getAppletID());
            ser.writeInt(obj.getObjectID());
            ser.writeBoolean(obj.isApplet());
        }
    }

    /** Reads a RemoteJavaObject from the given Serializer. */
    public static RemoteJavaObject read(Serializer ser) throws IOException {
        if (!ser.readBoolean())
            return null;
        return new RemoteJavaObject(ser.readInt(), ser.readInt(), ser.readInt(), ser.readBoolean());
    }

    public String toString() {
        return "[RemoteJavaObject jvmID=" + jvmID + " appletID=" + appletID +
            " objectID=" + objectID + " isApplet=" + isApplet + "]";
    }
}
