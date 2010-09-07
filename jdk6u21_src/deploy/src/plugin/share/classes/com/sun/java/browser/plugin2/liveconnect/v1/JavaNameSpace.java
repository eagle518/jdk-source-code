/*
 * @(#)JavaNameSpace.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

/** Represents a Java name space exposed to the web browser's
    JavaScript engine. This may be a package name (possibly a partial
    package name), such as "java" or "java.lang", or a fully-qualified
    class name such as "java.lang.String". <P>

    Instances of this class which are returned to the JavaScript
    engine are handled specially by the Bridge. Invocations, field
    fetches, and instantiations against them are assumed to represent
    operations on the class name encapsulated in the {@code
    JavaNameSpace} object. The Bridge looks up the target class using
    the registered InvocationDelegates' {@link
    InvocationDelegate#findClass findClass} method, and passes {@code
    true} for the {@code isStatic} argument to the various operations
    on the InvocationDelegate. <P>

    InvocationDelegate implementations may use this class to provide
    specialized behavior for a scripting language. For example, a
    scripting language runtime might use a generic Applet subclass to
    launch programs written in that language. The class name of the
    script might be passed to the applet as a parameter. It might be
    desirable to easily and generically refer to the script's class,
    and static fields and methods it contains. The language runtime
    might then use an InvocationDelegate to attach a synthetic
    "script" field to the applet, and when the JavaScript engine in
    the browser fetches that field, return a JavaNameSpace object
    pointing to the class name of the script. The InvocationDelegate
    would implement the {@code findClass} hook to handle the lookup of
    the script's class, and handle static method and field accesses
    appropriately to operate within the namespace of the script, but
    interacting at the level of the scripting language.
*/
public final class JavaNameSpace {
    private String name;

    /** Constructs a new JavaNameSpace object representing the given
        class name, package name or partial package name.

        @param name the point in the Java namespace to expose to the JavaScript engine
    */
    public JavaNameSpace(String name) {
        this.name = name;
    }

    /** Returns the point in the Java namespace this object represents.

        @return the point in the Java namespace this object represents
    */
    public String getName() {
        return name;
    }
}
