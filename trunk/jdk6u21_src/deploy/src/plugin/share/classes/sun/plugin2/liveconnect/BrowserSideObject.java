/*
 * @(#)BrowserSideObject.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.liveconnect;

import java.io.*;
import sun.plugin2.message.*;

/** This "marker" class represents a JavaScript object in the web
    browser. Because applets are executing in a different process in
    this model, when we want to make a Java to JavaScript call, we
    can't just call directly into the web browser; we have to send a
    message between processes. This object represents the data which
    needs to be sent back to the browser to effect the call. We use a
    marker class so that we can distinguish between these and other
    objects that are passed back and forth during Java-to-JavaScript
    and JavaScript-to-Java calls.<P>

    This class does not contain any functionality, only the reference
    to the remote object, to reduce the number of classes requiring
    porting to a new browser. The Plugin class in
    sun.plugin2.main.server references these objects and there are
    several methods there which need to be implemented in order to
    implement Java to JavaScript calls. <P>

    There are some assumptions about how the browser exposes its
    JavaScript objects to plugins. Namely, they are expected to be
    exposed as C structs, C++ objects, or other "stable" pointer-sized
    values. This is true at least on IE (where JavaScript objects are
    represented as IDispatch pointers managed through COM reference
    counting) and the Mozilla browser family (where, with the use of
    NPRuntime, JavaScript objects are represented as reference counted
    NPObject instances). <P>

    Note that the issue of maintaining references to browser-side
    objects is a bit tricky. All of the browsers out there today
    create a new plugin instance for each applet being viewed. From a
    conceptual standpoint, each JavaScript object is associated with
    the plugin instance that it's passed into. If each plugin (applet)
    on a web page were completely independent, we would maintain a
    per-applet table of the BrowserSideObjects it has received either
    from calls to getWindow(), return values from JSObject methods, or
    upcalls from JavaScript to Java. Each JSObject on the client side
    would contain the applet ID with which it is associated. <P>

    The Java-side inter-applet communication APIs
    (AppletContext.getApplets()) break this model. In theory it would
    be possible for one applet to get a handle to a JSObject and pass
    it directly to another applet on the same page, where the second
    applet would operate upon it. <P>

    It is probably the case that doing this would actually work on
    today's browsers. The old in-process Java Plug-In may
    inadvertently support this. This issue has been carefully thought
    through in the context of this multiple JRE plugin. For now, to
    keep things somewhat sane, we basically assume a reference counted
    system on the browser side (see {@link
    sun.plugin2.main.server.Plugin#javaScriptRetainObject
    Plugin.javaScriptRetainObject} and {@link
    sun.plugin2.main.server.Plugin#javaScriptReleaseObject
    Plugin.javaScriptReleaseObject}) and require that a reference to a
    given browser-side JavaScript object be passed directly to an
    applet wishing to operate upon it; attempts to pass JSObjects
    between applets will be detected and result in a JSException. <P>

    In order to reduce reference-counting traffic between the browser
    and attached client JVM instances, a distributed reference
    counting scheme is used.

    <UL>

    <LI> On the browser side, the first time a JavaScript object is
         passed up from the browser to a given applet, its reference
         count is incremented. This policy is implemented in the
         server-side {@link sun.plugin2.main.server.LiveConnectSupport
         LiveConnectSupport} class. This means that if a given
         JavaScript object (for example, representing an element in
         the DOM) is passed up from the web browser to two different
         applets, it will have its reference count incremented twice.

    <LI> On the client side (in the JVM instance executing the
         applet), we maintain a per-applet table from
         BrowserSideObjects to reference counts. Each time we
         deserialize a BrowserSideObject from the browser process
         targeted for a given applet and create a JSObject (actually,
         a MessagePassingJSObject) for it, we make sure the
         (BrowserSideObject, applet ID) pair is in this table and
         increment its reference count. We use phantom references in
         this process to track when the JSObject instances are
         released, and each time one is, we decrement the reference
         count of the BrowserSideObject. When the reference count goes
         to zero, we send a message back to the browser process
         indicating that the object can be released in the context of
         the specified applet. This scheme reduces traffic between the
         processes; we don't need to send a message each time an
         object is referenced or unreferenced in the client process.

    <LI> Back on the browser side, the reference counts for JavaScript
         objects are decremented in two situations. The first is when
         the applet is shut down; all JavaScript objects that were
         passed up to it are released. The second is when all
         references to a given JavaScript object from that applet are
         released on the client side and a message is received to
         decrement the reference count of that object.

    </UL>

    Note to the implementor: this policy and validation mechanism is
    implemented in sun.plugin2.main.server.LiveConnectSupport. Take
    care if attempting to extend it to support passing JSObjects
    between applets as some browsers may not support referencing a
    JavaScript object from a different plugin instance than it was
    originally passed to. In particular, the question of which plugin
    instance (and therefore main thread) to use to decrement a given
    object's reference count is an outstanding question, as is passing
    multiple JSObjects as arguments where some were "hosted" by one
    applet and some were hosted by another.
*/

public final class BrowserSideObject {
    // This is an IDispatch* in IE and an NPObject* on Firefox
    private long nativeObjectReference;

    /** Constructs a wrapper for the given native object reference (an
        IDispatch* on IE and an NPObject* on Firefox). */
    public BrowserSideObject(long nativeObjectReference) {
        this.nativeObjectReference = nativeObjectReference;
    }

    /** Returns the native object reference. */
    public long getNativeObjectReference() {
        return nativeObjectReference;
    }

    public int hashCode() {
        return (int) nativeObjectReference;
    }

    public boolean equals(Object arg) {
        if (arg == null || arg.getClass() != getClass())
            return false;
        BrowserSideObject other = (BrowserSideObject) arg;
        return (nativeObjectReference == other.nativeObjectReference);
    }

    public String toString() {
        return "[BrowserSideObject 0x" + Long.toHexString(nativeObjectReference) + "]";
    }

    /** Writes the given BrowserSideObject to the given Serializer. */
    public static void write(Serializer ser, BrowserSideObject obj) throws IOException {
        if (obj == null) {
            ser.writeBoolean(false);
        } else {
            ser.writeBoolean(true);
            ser.writeLong(obj.getNativeObjectReference());
        }
    }

    /** Reads a BrowserSideObject from the given Serializer. */
    public static BrowserSideObject read(Serializer ser) throws IOException {
        if (!ser.readBoolean())
            return null;
        return new BrowserSideObject(ser.readLong());
    }
}
