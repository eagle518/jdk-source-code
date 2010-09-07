/*
 * @(#)Bridge.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

/** The base of the inter-language bridge between JavaScript in the
    web browser and arbitrary languages hosted on the Java virtual
    machine implementation. <P>

    The Bridge provides the following functionality:

    <UL>

    <LI> Registration of {@link InvocationDelegate invocation
         delegates}, which affect how JavaScript field accesses,
         method invocations, and the like are translated into
         operations against objects in the target language;

    <LI> Registration of {@link ConversionDelegate conversion
         delegates}, which convert values passed in from the
         JavaScript engine to appropriate values in the target
         language;

    <LI> Calls to convert values between types, and to provide a
         measure of the conversion cost, to help with overloaded
         method resolution.

    </UL>

    A Bridge instance is obtained by calling {@link
    BridgeFactory#getBridge(Applet)}. The Bridge instance is
    security-sensitive and should not be passed to untrusted code.

    <P>

    A new Bridge instance is created by the Java Plug-In for each new
    applet instance. Therefore, new delegates must be registered for
    each new applet instance which hosts content written in a non-Java
    language. Typically this will be done by the language run-time
    system during {@link java.applet.Applet#init applet.init()}.
*/

public interface Bridge {

    /** Registers an {@link InvocationDelegate} with the Bridge. The
        InvocationDelegate will be called in response to field
        accesses, method calls, and creation of new objects from the
        JavaScript engine. */
    public void register(InvocationDelegate delegate);

    /** Unregisters an {@link InvocationDelegate} from the
        Bridge. Calling this method is optional since the bridge
        instance will be destroyed once the applet is terminated. */
    public void unregister(InvocationDelegate delegate);

    /** Registers a {@link ConversionDelegate} with the Bridge. The
        ConversionDelegate will be called to convert values between
        data types and and compute the cost of converting a value to a
        given data type. */
    public void register(ConversionDelegate delegate);

    /** Unregisters a {@link ConversionDelegate} from the Bridge.
        Calling this method is optional since the bridge instance will
        be destroyed once the applet is terminated. */
    public void unregister(ConversionDelegate delegate);

    /** Uses the registered ConversionDelegates to compute the cost of
        converting the given object to the given type. This method
        will typically be called by an {@link InvocationDelegate
        InvocationDelegate} during method lookup. If any registered
        ConversionDelegate returns a value greater than or equal to
        zero, the bridge will immediately return that value. Returns a
        value less than zero if the given value is not convertible to
        the given type. <P>

        Note that the target type is represented with an {@code
        Object} rather than a {@code Class}. When dealing with Java
        objects, {@code toType} will always be a {@code Class}
        instance. However, non-Java languages may, and often do,
        represent types with objects that are not instances of {@code
        java.lang.Class}. For this reason the target type is
        represented by a generic {@code Object} so that the delegate
        may decide whether or not to handle the conversion based on
        the type of {@code toType}, or other factors. <P>

        @param object the object to convert
        @param toType the target type
        @return a value >= 0 if the object is convertible to the given
                type, or a value < 0 if the object is not convertible
                to the given type
    */
    public int conversionCost(Object object, Object toType);

    /** Uses the registered ConversionDelegates to convert the given
        object to the given type. This method will generally be called
        by an InvocationDelegate during method lookup and setting of
        fields. If one of the registered ConversionDelegates returns
        <CODE>true</CODE> from its {@link ConversionDelegate#convert
        convert} method, the bridge will immediately return the result
        it has produced. If none of the ConversionDelegates accepts
        responsibility for the conversion, this method throws an
        unspecified exception. <P>

        See {@link #conversionCost conversionCost} for an explanation
        of why {@code toType} is an {@code Object} rather than a
        {@code Class}.

        @param object the object to convert
        @param toType the target type
        @return the object converted to the appropriate type
        @throws Exception if an exception occurs during conversion
        @see #conversionCost
    */
    public Object convert(Object object, Object toType) throws Exception;
}
