/*
 * @(#)InvocationDelegate.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2.liveconnect.v1;

/** Maps JavaScript field accesses, method calls and object creations
    to a language hosted on the Java platform. <P>

    When the web browser's JavaScript engine performs an operation
    against an object owned by the Java Virtual Machine, the Bridge
    iterates down the registered InvocationDelegates looking for a
    delegate which will handle the operation. Delegates are called in
    reverse order of registration, so the delegate most recently
    registered will be called first. The Java Plug-In registers a
    default InvocationDelegate supporting invocations against Java
    objects, which will be called last if none of the other registered
    delegates handles the request. <P>

    Most of the operations ({@link #invoke invoke}, {@link #getField
    getField}, {@link #setField setField}, {@link #hasField hasField},
    {@link #hasMethod hasMethod}, and {@link #hasFieldOrMethod
    hasFieldOrMethod}) return {@code true} to indicate that the
    operation has been handled. The {@link #findClass findClass}
    operation returns a non-null value to indicate that it has been
    handled. {@link #newInstance newInstance} will only be called
    against a given delegate if it has previously returned a non-null
    value from the {@link #findClass findClass} operation. A non-null
    value must be produced by the delegate, or an exception will be
    thrown back to the JavaScript engine. <P>

    Using an InvocationDelegate, a language implementor can expose not
    only the accessible fields and methods of objects, but also expose
    synthetic fields and methods to the JavaScript engine which have
    special semantics. For example, the default InvocationDelegate
    attaches a synthetic {@code Packages} field to the applet object,
    which provides the ability to call static methods, access static
    fields, and create new instances of objects, even if the target
    classes are in a user-defined namespace or represent non-Java
    types. This functionality is easily customizable by language
    implementors via the {@link #findClass findClass} and {@link
    #newInstance newInstance} hooks.
*/

public interface InvocationDelegate {
    /** Performs the specified method invocation. <P>

        The delegate may either choose to perform the given invocation
        or defer it to another registered delegate. Returning {@code
        true} from this method indicates the delegate has performed
        the invocation; returning {@code false} indicates the
        invocation has not been handled and that the invocation should
        be passed on to other delegates. Throwing an exception causes
        that exception to be reported (in the closest form possible)
        to the JavaScript engine. <P>

        At the time this method is called, the incoming arguments have
        been translated from the web browser's JavaScript engine
        according to the following rules: <P>

        <UL>

        <LI> Numeric values from the JavaScript engine have been boxed
        in the analogous Java boxing types: Integer, Double, etc.
        (Different JavaScript engines represent numeric values
        differently, so implementations of this interface should be
        robust with regard to the precise boxing class used in various
        situations.)

        <LI> JavaScript strings have been converted to Java Strings.

        <LI> JavaScript objects and arrays have been mirrored as
        instances of {@code netscape.javascript.JSObject}.

        <LI> Java (and non-Java) objects of other types which were
        previously returned to the JavaScript engine are passed
        through unchanged.

        </UL>
        
        <P>

        Implementors of this interface are responsible for overloaded
        method resolution, further conversion of arguments to other
        data types, and ultimately method invocation and production of
        a result. <P>

        The default InvocationDelegate resolves overloaded methods by
        computing a conversion cost of the incoming arguments to the
        types in matching methods' signatures. Only if there is an
        unambiguous resolution is the invocation performed; otherwise,
        an exception is thrown. Implementors can write similar
        cost-based overload resolution using {@link
        Bridge#conversionCost Bridge.conversionCost}, {@link
        Bridge#convert Bridge.convert}, and the {@link
        ConversionDelegate ConversionDelegate} interface. <P>

        Invocations that come directly from the web browser will
        always have a non-null receiver. The {@code isStatic} flag
        indicates whether the invocation is conceptually a static
        method invocation. It is set to true when the browser is
        performing an invocation against a {@link JavaNameSpace
        JavaNameSpace} object, and that object has just been resolved
        by the bridge to a class type via a call to {@link #findClass
        findClass}. Using this flag, implementors can distinguish
        between static method calls against a class, and instance
        method calls against a class object. <P>

        The {@code objectIsApplet} flag indicates whether the receiver
        is the root applet object which is exposed to the web
        browser. It supports attaching synthetic fields and methods to
        just the applet object: for example, the per-applet {@code
        Packages} keyword. <P>

        The {@code result} is a 1-length array into which a newly
        allocated {@link Result Result} should be placed. Null return
        values may be represented either by returning a null Result or
        wrapping the null value. Wrap the value {@link Void#TYPE
        Void.TYPE} in a Result to indicate to the Bridge that the
        method has a void return type, to distinguish between that and
        a null value on some web browsers.

        @param methodName the method name to invoke
        @param receiver the object against which to invoke the method
        @param arguments the incoming arguments to the method invocation
        @param isStatic a hint whether this corresponds to a static method invocation
        @param objectIsApplet whether the receiver is the applet object
        @param result 1-length array into which the result of the invocation should be placed
        @throws Exception if an exception occurs during the method invocation
        @return true if this InvocationDelegate handled the
                invocation, false if it should be passed on to the
                next delegate
     */
    public boolean invoke(String methodName,
                          Object receiver,
                          Object[] arguments,
                          boolean isStatic,
                          boolean objectIsApplet,
                          Result[] result) throws Exception;

    /** Fetches the specified field. <P>

        See {@link #invoke invoke} for a discussion of the semantics
        of the arguments and return values.

        @param fieldName the field name to fetch
        @param receiver the object against which to fetch the field
        @param isStatic a hint whether this corresponds to a static field fetch
        @param objectIsApplet whether the receiver is the applet object
        @param result 1-length array into which the result of the fetch should be placed
        @throws Exception if an exception occurs during the field fetch
        @return true if this InvocationDelegate handled the
                fetch, false if it should be passed on to the
                next delegate
        @see #invoke
     */
    public boolean getField(String fieldName,
                            Object receiver,
                            boolean isStatic,
                            boolean objectIsApplet,
                            Result[] result) throws Exception;

    /** Sets the specified field to the given value. <P>

        See {@link #invoke invoke} for a discussion of the semantics
        of the arguments and return value. The incoming {@code value}
        argument has been processed according to the same rules for
        the incoming method arguments in {@link #invoke invoke}. The
        value should be converted to the field's type in similar
        fashion to how arguments are converted during method
        invocation.

        @param fieldName the field name to set
        @param receiver the object against which to set the field
        @param isStatic a hint whether this corresponds to a static field assignment
        @param objectIsApplet whether the receiver is the applet object
        @throws Exception if an exception occurs during the field assignment
        @return true if this InvocationDelegate handled the
                assignment, false if it should be passed on to the
                next delegate
        @see #invoke
    */
    public boolean setField(String fieldName,
                            Object receiver,
                            Object value,
                            boolean isStatic,
                            boolean objectIsApplet) throws Exception;

    /** Queries whether the given object has a field of the specified name. <P>

        See {@link #invoke invoke} for a discussion of the semantics
        of the arguments. The {@code result} is a 1-length boolean
        array into which the result of the query should be placed if
        the delegate returns {@code true}. <P>

        <B>NOTE:</B> some JavaScript engines may not handle well the
        case where a delegate returns a {@code true} result from both
        the {@link #hasField hasField} and {@link #hasMethod
        hasMethod} queries. Implementations should consider shadowing
        one definition or the other if the language has different
        namespaces for fields and methods, like the Java language. <P>

        @param fieldName the field name to query
        @param receiver the object against which to query the field
        @param isStatic a hint whether this corresponds to a static field query
        @param objectIsApplet whether the receiver is the applet object
        @param result whether or not the given object has the specified field
        @return true if this InvocationDelegate handled the query,
                false if it should be passed on to the next delegate
        @see #invoke
    */        

    public boolean hasField(String fieldName,
                            Object receiver,
                            boolean isStatic,
                            boolean objectIsApplet,
                            boolean[] result);

    /** Queries whether the given object has a method of the specified name. <P>

        See {@link #hasField hasField} for a discussion of the
        semantics of the arguments and results.

        @param methodName the method name to query
        @param receiver the object against which to query the method
        @param isStatic a hint whether this corresponds to a static method query
        @param objectIsApplet whether the receiver is the applet object
        @param result whether or not the given object has the specified method
        @return true if this InvocationDelegate handled the query,
                false if it should be passed on to the next delegate
        @see #hasField
    */        
    public boolean hasMethod(String methodName,
                             Object receiver,
                             boolean isStatic,
                             boolean objectIsApplet,
                             boolean[] result);

    /** Queries whether the given object has a field or method of the
        specified name. Some JavaScript engines phrase their queries
        in this form rather than with separate field and method
        queries. <P>

        See {@link #hasField hasField} for a discussion of the
        semantics of the arguments and results.

        @param name the field or method name to query
        @param receiver the object against which to query
        @param isStatic a hint whether this corresponds to a static field or method query
        @param objectIsApplet whether the receiver is the applet object
        @param result whether or not the given object has the specified field or method
        @return true if this InvocationDelegate handled the query,
                false if it should be passed on to the next delegate
        @see #hasField
    */        
    public boolean hasFieldOrMethod(String name,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    boolean[] result);

    /** Looks up the class of the given name. <P>

        This method supports the {@link JavaNameSpace JavaNameSpace}
        functionality built in to the default InvocationDelegate. It
        may be called with partial package and class names (like
        "java.lang"), in which case the InvocationDelegate should
        return null. It is recommended that implementations optimize
        the case of repeated queries of the same partial name, and not
        simply delegate down to the JVM to attempt and fail to load
        the given class. <P>

        Implementations may choose to represent class types with
        objects other than {@code java.lang.Class} instances. <P>

        The default InvocationDelegate uses {@link
        Class#forName(String, boolean, ClassLoader) Class.forName} to
        resolve the given name. If this implementation suffices for
        another language hosted on the Java platform, it can return
        null from this method. However, it should be noted that
        another delegate might attempt to satisfy the {@code
        findClass} query, leading to unexpected results.

        @param name the class name to look up
        @return the object representing that class, or null if this
        delegate chooses to pass the lookup to the next delegate, or
        the class does not exist
    */
    public Object findClass(String name);

    /** Creates a new instance of the given class. This method is
        called in response to evaluation of the "new" keyword in
        JavaScript. The target of the "new" expression is resolved and
        is passed as the {@code clazz} argument. <P>

        This method will only be called by the Bridge if this
        InvocationDelegate has just returned a non-null value from
        {@link #findClass findClass}. It works only in conjunction
        with the {@link JavaNameSpace JavaNameSpace} support built in
        to the Bridge. <P>

        See {@link #invoke invoke} for a discussion of the semantics
        of the arguments. <P>

        @param clazz the class to instantiate
        @param arguments the incoming arguments to the instantiation
        @return the newly-created object instance
        @throws Exception if an exception occurred during instantiation
    */
    public Object newInstance(Object clazz,
                              Object[] arguments) throws Exception;
}
