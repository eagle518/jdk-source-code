/*
 * @(#)JavaClass.java	1.19 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.liveconnect;

import java.lang.reflect.*;
import java.util.*;

import com.sun.java.browser.plugin2.liveconnect.v1.*;

// Import of JSClassLoader for security purposes
import sun.plugin.javascript.JSClassLoader;
// Import of ReflectUtil because it seems to provide what we need
import sun.plugin.javascript.ReflectUtil;
// Import of utility class
import sun.plugin2.util.SystemUtil;

/** A JavaClass wraps a java.lang.Class and assists in method
    invocations and field get and set operations initiated from
    JavaScript. It implements all of the rules that are needed when
    attempting to pass arguments from JavaScript to Java, including
    narrowing conversions for numerical types (not supported in core
    reflection), conversions from Strings to primitive types, and
    exposure of Java arrays to JavaScript (portably) where the
    elements are exposed as JavaScript properties of name, for
    example, "0", "1", and so on. */

public class JavaClass implements InvocationDelegate {
    private Class clazz;
    private Bridge bridge;

    // Overload resolution: map of the names of visible methods to the
    // MemberBundles they map to
    private Map/*<String,MemberBundle>*/ methodMap;
    // Map of the names of visible fields
    private Map/*<String,Field>*/ fieldMap;
    // Bundle of the visible constructors
    private MemberBundle constructors;

    // Lower-case versions of these maps which we only use when the
    // initial lookups fail; the intent here is to support
    // case-insensitivity but to also support discrimination of
    // differing mixed-case method names if the user is careful about
    // their capitalization on the JavaScript side
    private Map/*<String,MemberBundle>*/ lowerCaseMethodMap;
    private Map/*<String,Field>*/ lowerCaseFieldMap;

    // We cache this information for array types for performance reasons
    private boolean isArray;
    private Class   componentType; // if array

    public JavaClass(Class clazz, Bridge bridge) {
        this.clazz = clazz;
        this.bridge = bridge;
        isArray = clazz.isArray();
        if (isArray) {
            componentType = clazz.getComponentType();
        }
    }

    private static abstract class MemberInfo {
        private Member member;
        protected Class[] parameterTypes;

        protected MemberInfo(Member member) {
            this.member = member;
        }

        protected Member  getMember()         { return member;                }
        public    String  getName()           { return getMember().getName(); }
        public    Class[] getParameterTypes() { return parameterTypes;        }

        // This might do either an invoke() or a newInstance() operation;
        // in the case of newInstance(), the target is ignored and may be null
        public abstract Object invoke(Object target, Object[] args) throws Exception;

        // This returns the return type for a Method or the declaring
        // class for a Constructor
        public abstract Class getReturnType();

        // This indicates whether the method is a "bridge method" on
        // 1.5, which causes the invocation loop below some confusion
        // because it appears there are multiple methods matching the
        // interface
        public abstract boolean isBridge();

        public String toString() {
            return member.toString();
        }
    }

    private static class MethodInfo extends MemberInfo {
        protected MethodInfo(Method method) {
            super(method);
            parameterTypes = method.getParameterTypes();
        }

        // We override equals() to filter out duplicate methods that
        // come to us through different points of the inheritance
        // hierarchy (i.e., an abstract base class as well as an
        // interface)
        public boolean equals(Object o) {
            if (o == null || (o.getClass() != getClass())) {
                return false;
            }
            Method self = getMethod();
            Method other = ((MethodInfo) o).getMethod();
            // We consider ourselves equal if the name, return type
            // and parameters match, ignoring the declaring class
            return (self.getName().equals(other.getName()) &&
                    self.getReturnType() == other.getReturnType() &&
                    arraysEq(self.getParameterTypes(),
                             other.getParameterTypes()));
        }

        public Method getMethod() { return (Method) getMember(); }

        public Object invoke(Object target, Object[] args) throws Exception {
            Object res = JSClassLoader.invoke(getMethod(), target, args);
            // Return Void.TYPE for methods returning void to
            // disambiguate null and void return values to the caller
            if (res == null && getMethod().getReturnType() == Void.TYPE)
                return Void.TYPE;
            return res;
        }

        public Class getReturnType() {
            return getMethod().getReturnType();
        }

        public boolean isBridge() {
            try {
                return getMethod().isBridge();
            } catch (Error e) {
                // isBridge() was introduced in 5.0; handle
                // NoSuchMethodErrors on 1.4.x and return false
                return false;
            }
        }

        private boolean arraysEq(Class[] params1,
                                 Class[] params2) {
            if ((params1 == null) != (params2 == null)) {
                return false;
            }
            if (params1 == null) {
                return true;
            }
            if (params1.length != params2.length) {
                return false;
            }
            for (int i = 0; i < params1.length; i++) {
                if (params1[i] != params2[i]) {
                    return false;
                }
            }
            return true;
        }
    }

    private static class ConstructorInfo extends MemberInfo {
        protected ConstructorInfo(Constructor constructor) {
            super(constructor);
            parameterTypes = constructor.getParameterTypes();
        }
        
        public Constructor getConstructor() { return (Constructor) getMember(); }
        
        public Object invoke(Object target, Object[] args) throws Exception {
            return JSClassLoader.newInstance(getConstructor(), args);
        }

        public Class getReturnType() {
            return getConstructor().getDeclaringClass();
        }

        public boolean isBridge() {
            // No such thing as a bridge constructor
            return false;
        }
    }

    // Represents a set of overloaded methods or constructors
    // Note that "member" is a bit of a misnomer -- according to the JLS,
    // Constructors aren't members
    private class MemberBundle {
        protected List/*<MemberInfo>*/ members = new ArrayList/*<MemberInfo>*/();

        public void add(Method method) {
            MethodInfo info = new MethodInfo(method);
            // Filter out bridge and duplicate methods early
            if (!info.isBridge() &&
                !members.contains(info)) {
                members.add(info);
            }
        }

        public void add(Constructor constructor) {
            members.add(new ConstructorInfo(constructor));
        }

        // This might end up doing an invoke() or a newInstance() call internally
        public Result invoke(Object target, Object[] arguments) throws Exception {
            MemberInfo chosenInfo = null;
            MemberInfo ambiguousInfo = null;
            Class[] chosenParameterTypes = null;
            int minNumConversions = 0;
            boolean ambiguous = false;

            for (Iterator iter = members.iterator(); iter.hasNext(); ) {
                MemberInfo info = (MemberInfo) iter.next();
                Class[] parameterTypes = info.getParameterTypes();
                if (arguments == null) {
                    if (parameterTypes.length != 0)
                        continue;
                } else if (parameterTypes.length != arguments.length)
                    continue;

                // NOTE: the code in this analysis loop should match
                // the code in convert(), below; in particular, it
                // shouldn't report a match if convert() can't
                // actually convert one of the arguments

                // If this contains a negative number after analysis,
                // the argument lists aren't compatible
                int numConversions = 0;
                for (int i = 0; i < parameterTypes.length; i++) {
                    Object arg = arguments[i];
                    Class expectedClass = parameterTypes[i];
                    int cost = bridge.conversionCost(arg, expectedClass);
                    if (cost < 0) {
                        numConversions = -1;
                        break;
                    }
                    numConversions += cost;
                }

                if (numConversions >= 0) {
                    if (chosenInfo == null ||
                        (numConversions < minNumConversions)) {
                        chosenInfo = info;
                        chosenParameterTypes = info.getParameterTypes();
                        minNumConversions = numConversions;
                        ambiguous = false;
                    } else if (numConversions == minNumConversions) {
                        ambiguous = true;
                        ambiguousInfo = info;
                    }
                }
            }

            if (chosenInfo == null) {
                throw new IllegalArgumentException("No method found matching name " +
                                                   ((MemberInfo) members.get(0)).getName() +
                                                   " and arguments " + argsToString(arguments));
            }

            if (ambiguous) {
                throw new IllegalArgumentException("More than one method matching name " +
                                                   ((MemberInfo) members.get(0)).getName() +
                                                   " and arguments " + argsToString(arguments) +
                                                   "\n  Method 1: " + chosenInfo.getMember().toString() +
                                                   "\n  Method 2: " + ambiguousInfo.getMember().toString());
            }
            
            // Convert all arguments
            Object[] newArgs = null;
            if (arguments != null) {
                newArgs = new Object[arguments.length];
                for (int i = 0; i < arguments.length; i++) {
                    newArgs[i] = bridge.convert(arguments[i], chosenParameterTypes[i]);
                }
            }
            Object ret = chosenInfo.invoke(target, newArgs);
            return new Result(ret, isBoxingClass(chosenInfo.getReturnType()));
        }
    }

    private static String argsToString(Object[] arguments) {
        StringBuffer buf = new StringBuffer("[");
        if (arguments != null) {
            for (int i = 0; i < arguments.length; i++) {
                if (i > 0)
                    buf.append(", ");
                Object arg = arguments[i];
                String className = null;
                if (arg != null)
                    className = arg.getClass().getName();
                buf.append(className);
            }
        }
        buf.append("]");
        return buf.toString();
    }

    /** Indicates to the caller whether this class has the given
        field. This is a concession to the Mozilla JavaScript engine,
        which asks objects whether they have a given method or
        property before deciding how to invoke against them, and which
        requires accurate answers or it fails in unexpected ways
        (reporting that a given object has no properties, for
        example). This method accepts the receiver as argument to
        provide more precise answers in the case of arrays. */
    public boolean hasField(String fieldName,
                            Object receiver,
                            boolean isStatic,
                            boolean objectIsApplet,
                            boolean[] result) {
        result[0] = hasField0(fieldName, receiver, objectIsApplet);
        return true;
    }

    private boolean hasField0(String fieldName,
                              Object receiver,
                              boolean objectIsApplet) {
        if (isArray) {
            if ("length".equals(fieldName)) {
                return true;
            } else {
                try {
                    int index = Integer.parseInt(fieldName);
                    return (index >= 0 && index < Array.getLength(receiver));
                } catch (Exception e) {
                    return false;
                }
            }
        } else {
            if (fieldMap == null)
                collectFields();
            Field field = (Field) fieldMap.get(fieldName);
            if (field != null)
                return true;
            // Try again with the lower-case / case-insensitive version
            field = (Field) lowerCaseFieldMap.get(fieldName.toLowerCase());
            return (field != null);
        }
    }

    /** Indicates to the caller whether this class has the given
        method. This is a concession to the Mozilla JavaScript engine,
        which asks objects whether they have a given method or
        property before deciding how to invoke against them, and which
        requires accurate answers or it fails in unexpected ways
        (reporting that a given object has no properties, for
        example). */
    public boolean hasMethod(String methodName,
                             Object receiver,
                             boolean isStatic,
                             boolean objectIsApplet,
                             boolean[] result) {
        result[0] = hasMethod0(methodName, receiver, objectIsApplet);
        return true;
    }

    private boolean hasMethod0(String methodName, Object receiver, boolean objectIsApplet) {
        if (methodMap == null)
            collectMethods();

        MemberBundle bundle = (MemberBundle) methodMap.get(methodName);
        if (bundle != null)
            return true;
        // Try again with the lower-case / case-insensitive version
        bundle = (MemberBundle) lowerCaseMethodMap.get(methodName.toLowerCase());
        return (bundle != null);
    }

    /** Indicates to the caller whether this class has the given field
        or method. This is  used to provide more precise answers to 
        Internet Explorer when it calls GetIDsOfNames on the CJavaDispatch
        objects corresponding to Java objects (in particular, being able 
        to return DISP_E_UNKNOWNNAME). */
    public boolean hasFieldOrMethod(String name,
                                    Object receiver,
                                    boolean isStatic,
                                    boolean objectIsApplet,
                                    boolean[] result) {
        boolean res = (hasField0(name, receiver, objectIsApplet) ||
                       hasMethod0(name, receiver, objectIsApplet));
        result[0] = res;
        return true;
    }

    /** Invokes the named method against the given receiver object
        (which is allowed to be null in the case of invoking static
        methods) with the given parameter list, resolving overloading
        dynamically using JavaScript to Java conversion rules. <P>

        The "skipUnboxing" argument should be a 1-length boolean array
        whose contents are initialized to false. If skipUnboxing[0] is
        true after the invocation, then if the return value is a box
        for a primitive value, it should not be unboxed on the other
        side, but returned as a Java object to the JavaScript
        engine. <P>

        <B>NOTE</B> that if the target method has a void return type,
        this method will return the value Void.TYPE rather than null!
        This is necessary in order to disambiguate void and null
        return types for some JavaScript engines.
    */
    public boolean invoke(String methodName,
                          Object receiver,
                          Object[] arguments,
                          boolean isStatic,
                          boolean objectIsApplet,
                          Result[] result) throws Exception {
        result[0] = invoke0(methodName, receiver, arguments, objectIsApplet);
        return true;
    }

    private Result invoke0(String methodName,
                           Object receiver,
                           Object[] arguments,
                           boolean objectIsApplet) throws Exception {
        if (methodMap == null)
            collectMethods();

        MemberBundle bundle = (MemberBundle) methodMap.get(methodName);
        if (bundle == null) {
            // Try again with the lower-case / case-insensitive version
            bundle = (MemberBundle) lowerCaseMethodMap.get(methodName.toLowerCase());
        }
        if (bundle == null) {
            throw new NoSuchMethodException(methodName + " in class: " + clazz.getName());
        }
        return bundle.invoke(receiver, arguments);
    }

    public Object findClass(String name) {
        throw new UnsupportedOperationException("Should not call this");
    }

    /** Creates a new instance of the given class by calling the
        constructor most closely matching the set of incoming
        arguments. */
    public Object newInstance(Object clazz,
                              Object[] arguments) throws Exception {
        if (constructors == null)
            collectConstructors();

        Result res = constructors.invoke(null, arguments);
        return res.value();
    }

    /** Gets the named field of the given object (which may be null in
        the case of static fields). If the given object is an array,
        exposes the length of the array as the virtual field "length",
        and the elements of the array as the virtual fields "0", "1",
        etc. <P>

        The "skipUnboxing" argument should be a 1-length boolean array
        whose contents are initialized to false. If skipUnboxing[0] is
        true after the invocation, then if the return value is a box
        for a primitive value, it should not be unboxed on the other
        side, but returned as a Java object to the JavaScript
        engine. <P>
    */
    public boolean getField(String fieldName,
                            Object receiver,
                            boolean isStatic,
                            boolean objectIsApplet,
                            Result[] result) throws Exception {
        result[0] = getField0(fieldName, receiver, objectIsApplet);
        return true;
    }

    private Result getField0(String fieldName, Object receiver, boolean objectIsApplet) throws Exception {
        if (isArray) {
            if ("length".equals(fieldName)) {
                return new Result(new Integer(Array.getLength(receiver)), false);
            } else {
                int index = Integer.parseInt(fieldName);
                return new Result(Array.get(receiver, index),
                                  isBoxingClass(receiver.getClass().getComponentType()));
            }
        } else {
            if (fieldMap == null)
                collectFields();
            Field field = (Field) fieldMap.get(fieldName);
            if (field == null) {
                // Try again with the lower-case / case-insensitive version
                field = (Field) lowerCaseFieldMap.get(fieldName.toLowerCase());
            }
            if (field == null) {
                throw new NoSuchFieldException(fieldName + " in class: " + clazz.getName());
            }
            return new Result(field.get(receiver), isBoxingClass(field.getType()));
        }
    }

    /** Sets the named field of the given object (which may be null in
        the case of static fields). If the given object is an array,
        exposes the elements of the array as the virtual fields "0",
        "1", etc. */
    public boolean setField(String fieldName,
                            Object receiver,
                            Object value,
                            boolean isStatic,
                            boolean objectIsApplet) throws Exception {
        if (isArray) {
            int index = Integer.parseInt(fieldName);
            Array.set(receiver, index, bridge.convert(value, componentType));
        } else {
            if (fieldMap == null)
                collectFields();
            Field field = (Field) fieldMap.get(fieldName);
            if (field == null) {
                // Try again with the lower-case / case-insensitive version
                field = (Field) lowerCaseFieldMap.get(fieldName.toLowerCase());
            }
            if (field == null) {
                throw new NoSuchFieldException(fieldName);
            }
            Class expectedType = field.getType();
            field.set(receiver, bridge.convert(value, expectedType));
        }
        return true;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private static boolean isBoxingClass(Class clazz) {
        // NOTE that we convert Java Strings to native strings in the
        // JavaScript engine except in the case where the user
        // explicitly calls "new java.lang.String(...)" from
        // JavaScript. This is the behavior of the old plug-in in IE
        // and is by almost all measures the "right thing to do".
        // However, this does break some old LiveConnect test cases
        // from Mozilla, and is different behavior than the old OJI
        // plug-in for Firefox / Mozilla.
        return (clazz == Boolean.class ||
                clazz == Byte.class ||
                clazz == Short.class ||
                clazz == Character.class ||
                clazz == Integer.class ||
                clazz == Long.class ||
                clazz == Float.class ||
                clazz == Double.class);
    }

    private void collectMethods() {
        // NOTE: need this call to ReflectUtil because the type we
        // represent might be inaccessible -- for example, a
        // package-private implementation of an interface. Must get
        // the right declaring class for the public Methods we expose.
        Method[] methods = ReflectUtil.getJScriptMethods(clazz);
        Map/*<String,MemberBundle>*/ methodMap = new HashMap/*<String,MemberBundle>*/();
        Map/*<String,MemberBundle>*/ lowerCaseMethodMap = new HashMap/*<String,MemberBundle>*/();
        for (int i = 0; i < methods.length; i++) {
            Method method = methods[i];
            MemberBundle bundle = (MemberBundle) methodMap.get(method.getName());
            if (bundle == null) {
                bundle = new MemberBundle();
                methodMap.put(method.getName(), bundle);
            }
            bundle.add(method);

            // Lower-case / case-insensitive version as well (note
            // that the MemberBundle might contain different or more
            // entries than the case-sensitive one)
            String lowerCaseName = method.getName().toLowerCase();
            bundle = (MemberBundle) lowerCaseMethodMap.get(lowerCaseName);
            if (bundle == null) {
                bundle = new MemberBundle();
                lowerCaseMethodMap.put(lowerCaseName, bundle);
            }
            bundle.add(method);
        }
        this.methodMap = methodMap;
        this.lowerCaseMethodMap = lowerCaseMethodMap;
    }

    private void collectConstructors() {
        Constructor[] ctors = clazz.getConstructors();
        MemberBundle ctorBundle = new MemberBundle();
        for (int i = 0; i < ctors.length; i++) {
            ctorBundle.add(ctors[i]);
        }
        constructors = ctorBundle;
    }

    private void collectFields() {
        Field[] fields = ReflectUtil.getJScriptFields(clazz);
        Map/*<String,Field>*/ fieldMap = new HashMap/*<String,Field>*/();
        Map/*<String,Field>*/ lowerCaseFieldMap = new HashMap/*<String,Field>*/();
        for (int i = 0; i < fields.length; i++) {
            fieldMap.put(fields[i].getName(), fields[i]);

            // Lower-case / case-insensitive version as well
            lowerCaseFieldMap.put(fields[i].getName().toLowerCase(), fields[i]);
        }
        this.fieldMap = fieldMap;
        this.lowerCaseFieldMap = lowerCaseFieldMap;
    }
}
