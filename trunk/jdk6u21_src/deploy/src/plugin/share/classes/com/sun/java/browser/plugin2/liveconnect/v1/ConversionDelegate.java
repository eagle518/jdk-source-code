/*
 * @(#)ConversionDelegate.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

/** Converts values between types, and provides a measure of the cost
    of performing such conversion. <P>

    Incoming values from the JavaScript engine in the web browser
    usually require conversion to be able to pass them to Java
    objects, or to objects of non-Java languages hosted on the JVM.
    The reason is that the JavaScript language is untyped, and
    different JavaScript engines represent entities like numeric
    values differently. <P>

    When handling an incoming method call, field access, etc. from
    JavaScript via the {@link InvocationDelegate InvocationDelegate}
    interface, the Bridge represents incoming values according to the
    following rules: <P>

    <UL>

    <LI> Numeric values from the JavaScript engine are boxed in the
    analogous Java boxing types: Integer, Double, etc. (Different
    JavaScript engines represent numeric values differently, so
    implementations should be robust with regard to the precise boxing
    class used in various situations.)

    <LI> JavaScript strings are converted to Java Strings.

    <LI> JavaScript objects and arrays are mirrored as instances of
    {@code netscape.javascript.JSObject}.

    <LI> Java and non-Java objects of other types which were
    previously returned to the JavaScript engine are passed through
    unchanged.

    </UL>

    <P>

    The InvocationDelegate will typically call {@link Bridge#convert
    Bridge.convert} to coerce values to certain types in preparation
    for invoking a method or setting a field; for example, to perform
    narrowing conversions on primitive types. The Bridge delegates
    this work to the registered ConversionDelegates. <P>

    A ConversionDelegate may choose to compute a cost for a particular
    conversion and/or perform the conversion, or pass this work on to
    the next delegate in the chain. A default ConversionDelegate is
    registered and is called last if no other delegate handles the
    operation. The default delegate represents types with {@code
    java.lang.Class} instances, and implements the following
    conversions: <P>

    <UL>

    <LI> Conversions among all primitive types, including narrowing
    conversions and conversions to {@code boolean}

    <LI> Conversion of any value to {@code java.lang.String}

    <LI> Conversion of {@code java.lang.String} to a numeric value or
    boolean; an empty String corresponds to the value {@code false}

    <LI> Conversion of JavaScript arrays to Java arrays, including
    multi-dimensional arrays

    </UL>

    To assist in overloaded method resolution, this package assigns
    costs to conversions. (It is unclear whether this scheme will
    stand the test of time, which is why "v1" (version 1) is included
    in the package name.) ConversionDelegate implementations should
    try to assign costs to conversions in a way that minimizes the
    probability that two methods with the same number of arguments,
    but materially different signatures, will report an ambiguity for
    a certain argument list when it is obvious which method should be
    invoked.

    <P>

    To avoid creating overdependencies, the exact algorithm and values
    currently used by the default ConversionDelegate are left
    unspecified. However, as a hint to other implementors, it uses the
    following set of rules when computing costs:

    <UL>

    <LI> The following conversions have the lowest cost:

    <UL>
    <LI> Numeric type to the analogous Java primitive type
    <LI> Null to any non-primitive type
    <LI> JSObject to JSObject
    <LI> Class type to Class type where the types are equal
    </UL>

    <LI> The following conversions report higher cost:

    <UL>
    <LI> Numeric type to a different primitive type
    <LI> String to numeric type
    <LI> Class type to superclass type; this assists in overload
    resolution where one method takes a subclass as argument and
    another method takes a superclass as argument, and an instance of
    the subclass is passed in. The default delegate computes a measure
    of the "distance" in the class hierarchy between a value and the
    desired type.
    </UL>

    <LI> The following conversion reports still higher cost:

    <UL>
    <LI> Any Java value to String
    </UL>

    <LI> The following conversions report even higher cost:

    <UL>
    <LI> JSObject to String
    <LI> JSObject to Java array
    </UL>

    <LI> The following are reported as not convertible (negative
    conversion cost):

    <UL>
    <LI> Any value to a type not represented by {@code java.lang.Class}
    <LI> Null to primitive type
    <LI> JSObject to any type not described above
    <LI> Any other conversion not described above
    </UL>

    </UL>
*/

public interface ConversionDelegate {
    /** Computes the cost of converting the given object to the given
        type. Both are represented as opaque entities ({@code
        java.lang.Object}) since different delegates may represent
        classes and objects in different ways. A zero or positive
        value indicates this delegate can convert the value to the
        type. Lower numbers represent better matches. A negative
        return value indicates this delegate can not perform the
        conversion.

        @param object the object to convert
        @param toType the type to which to convert the object
        @return the cost of converting the given object to the type,
        or a negative value if this delegate can not convert the
        object to the given type
    */
    public int conversionCost(Object object, Object toType);

    /** Converts the given object to the given type. This should
        produce a valid result if {@link #conversionCost
        conversionCost} returns a nonnegative value, although this may
        not be possible in all cases; for example, during conversion
        of a JavaScript array to a Java primitive array when the JS
        array contains an inconvertible value. Should return true if
        this delegate handles the conversion; false if it wants to
        pass through to the next delegate; and should throw an
        exception if it both handles the conversion and the conversion
        is illegal.

        @param object the object to convert
        @param toType the type to which to convert the object
        @param result 1-length array into which to store the result of
        the conversion, or the original object if it does not require
        conversion
        @return true if this ConversionDelegate handled the
        conversion, false if it should be passed to the next delegate
        @throws Exception if an exception occurred during conversion
    */
    public boolean convert(Object object, Object toType, Object[] result) throws Exception;
}
