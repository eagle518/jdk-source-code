/*
 * @(#)MXBeanSupport.java	1.4 04/06/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import javax.management.*;
import java.util.*;
import javax.management.openmbean.OpenType;
import javax.management.openmbean.OpenDataException;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.InvocationTargetException;
import java.security.AccessController;
import java.security.PrivilegedAction;

/*
 * Support class for platform MXBeans with type mapping.
 *
 * - build MBeanInfo for a given MXBean interface 
 * - provide support for getAttribute, getAttributes, setAttribute,
 *   setAttributes, and invoke methods for DynamicMBean interface
 * - provide support for proxy MXBean to forward a method call to
 *   MBeanServerConnection
 */
public class MXBeanSupport implements DynamicMBean {
    private String className;
    private Class interfaceClass;
    private Map<String, AttributeMethod> attrMethods; 
    private Map<String, List<Method>> opMethods;
    private MBeanInfo minfo;
    private Map<Method, ProxyMethod> proxyMethods;
    private int numAttributes;
    private int numOperations;
    private boolean notificationEmitter;

    protected MXBeanSupport(Class mxbeanInterface) {
        initialize(mxbeanInterface, false);
    }

    protected MXBeanSupport(Class mxbeanInterface, boolean notifEmitter) {
        initialize(mxbeanInterface, notifEmitter);
    }

    void initialize(Class mxbeanInterface, boolean notifEmitter) {
        this.className = this.getClass().getName();
        // FIXME: need to validate if mxbeanInterface 
        this.interfaceClass = mxbeanInterface;
        this.attrMethods = null;
        this.opMethods = null;
        this.minfo = null;
        this.proxyMethods = null;
        this.numAttributes = 0;
        this.numOperations = 0;
        this.notificationEmitter = notifEmitter;
    }


    //
    // Implementation of the DynamicMBean Interface
    //

    public Object getAttribute(String attributeName)
        throws AttributeNotFoundException,
               MBeanException,
               ReflectionException {
        return getAttribute(this, attributeName);
    }

    public AttributeList getAttributes(String[] attributeNames) {
        return getAttributes(this, attributeNames);
    }

    public void setAttribute(Attribute attribute)
        throws AttributeNotFoundException,
               InvalidAttributeValueException,
               MBeanException,
               ReflectionException {
        setAttribute(this, attribute);
    }


    public AttributeList setAttributes(AttributeList attributes) {
        return setAttributes(this, attributes);
    }

    public Object invoke(String operationName, Object params[],
                         String signatures[])
        throws MBeanException, ReflectionException {
        return invoke(this, operationName, params, signatures);
    }

    public synchronized MBeanInfo getMBeanInfo() {
        if (minfo != null) {
            return minfo;
        }

        // build MBeanInfo with the given notifications with no constructor
        MBeanNotificationInfo[] notifications;
        if (notificationEmitter) {
            NotificationEmitter emitter = (NotificationEmitter) this;
            notifications = emitter.getNotificationInfo();
        } else {
            // No notification
            notifications = new MBeanNotificationInfo[0];
        }
 
        // parse the mxbean interface
        parse();

        MBeanConstructorInfo[] constructors = new MBeanConstructorInfo[0];
        MBeanAttributeInfo[] attributes = buildAttributeInfoArray(attrMethods);
        MBeanOperationInfo[] operations = buildOperationInfoArray(opMethods, numOperations);

        minfo = new MBeanInfo(className,
                              className,
                              attributes,
                              constructors,
                              operations,
                              notifications);
        return minfo;
    }

    private synchronized void parse() {
        if (attrMethods != null || opMethods != null) {
            return; 
        }

        final Method[] methods = (Method[])
            AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return interfaceClass.getMethods();
                }
            });
        attrMethods = new HashMap<String, AttributeMethod>();
        opMethods = new HashMap<String, List<Method>>();

        // FIXME: covariant returns are not supported
        for (Method m : methods) {
            if (isGetter(m)) {
                String attName = getAttributeName(m);
                AttributeMethod am = attrMethods.get(attName);
                if (am == null) {
                    am = new AttributeMethod(attName);
                    attrMethods.put(attName, am);
                }
                am.setGetterMethod(m);
            } else if (isSetter(m)) {
                String attName = getAttributeName(m);
                AttributeMethod am = attrMethods.get(attName);
                if (am == null) {
                    am = new AttributeMethod(attName);
                    attrMethods.put(attName, am);
                }
                am.setSetterMethod(m);
            } else {
                // An operation
                String opName = m.getName();
                List<Method> ops = opMethods.get(opName);
                if (ops == null) {
                    ops = new ArrayList<Method>();
                    opMethods.put(opName, ops);
                }
                ops.add(m);
                numOperations++;
            }
        }

    }

    // Is a getter if 
    //  - the method starts with "get" or "is"; and
    //  - it's not named "getClass"; and
    //  - it has no input parameter; and
    //  - the return type is not Class 
    //  - the return type of a "is" method is boolean
    public static boolean isGetter(Method m) {
        final String name = m.getName();
        final Class type = m.getReturnType(); 
        String attName = "";
        if (name.startsWith("get")) {
            attName = name.substring(3);
        } else if (name.startsWith("is") && type == boolean.class) {
            attName = name.substring(2);
        } else {
            return false;
        }

        if (! attName.equals("") && ! attName.equals("Class") &&
            m.getParameterTypes().length == 0 &&
            type != void.class) {
            return true;
        } else {
            return false;
        }
    }

    // Is a setter if 
    //  - the method starts with "set";  and
    //  - it has only 1 input parameter; and
    //  - the return type is void
    public static boolean isSetter(Method m) {
        final String name = m.getName();
        final Class type = m.getReturnType(); 
        String attName = "";
        if (name.startsWith("set")) {
            attName = name.substring(3);
        } else {
            return false;
        } 

        if (! attName.equals("") &&
            m.getParameterTypes().length == 1 &&
            type == void.class) {
            return true;
        } else {
            return false;
        }
    }

    public static String getAttributeName(Method m) {
        String name = m.getName();
        String attName = null;
        if (name.startsWith("get")) {
            attName = name.substring(3);
        } else if (name.startsWith("is")) {
            attName = name.substring(2);
        } else if (name.startsWith("set")) {
            attName = name.substring(3);
        }
        return attName;
    }


    static class AttributeMethod {
        String attributeName;
        Method getter;
        Method setter;
        AttributeMethod(String name) {
            this.attributeName = name; 
            this.getter = null; 
            this.setter = null; 
        }    
        void setGetterMethod(Method m) {
            if (getter != null) {
                throw new AssertionError("Getter already exists: " +
                    getter.getName() + " of type " + 
                    getter.getReturnType().getName());
            }
            if (setter != null) {
                Class setterType = setter.getParameterTypes()[0];
                Class getterType = m.getReturnType();
                if (!setterType.getName().equals(getterType.getName())) {
                    throw new AssertionError("Mismatched attribute type " +
                        m.getName() + "()" + getterType.getName() +
                        setter.getName() + "(" + setterType.getName() + ")");
                }
            }
            getter = m;
        }
        void setSetterMethod(Method m) {
            if (setter != null) {
                throw new AssertionError("Setter already exists: " +
                    setter.getName());
            }
            if (getter != null) {
                Class getterType = getter.getReturnType();
                Class setterType = m.getParameterTypes()[0];
                if (!setterType.getName().equals(getterType.getName())) {
                    throw new AssertionError("Mismatched attribute type " +
                        m.getName() + "()" + getterType.getName() +
                        setter.getName() + "(" + setterType.getName() + ")");
                }
            }
            setter = m;
        }
        String getName() {
            return attributeName;
        }
        Type getType() {
            if (getter != null) {
                return getter.getGenericReturnType();
            } else {
                return setter.getGenericParameterTypes()[0];
            }
        }
        boolean isReadable() {
            return getter != null;
        }
        boolean isWriteable() {
            return setter != null;
        }
        boolean isIs() {
            return getter != null && getter.getName().startsWith("is");
        }
    }

    // Support for MXBean proxy
    private static Map<Class, MXBeanSupport> proxyMXBeans = 
        new HashMap<Class, MXBeanSupport>();
    static synchronized MXBeanSupport newProxy(Class mxbeanInterface) {
        MXBeanSupport ms = proxyMXBeans.get(mxbeanInterface);
        if (ms == null) {
             ms = new MXBeanSupport(mxbeanInterface);
             proxyMXBeans.put(mxbeanInterface, ms);
             ms.buildProxyMethods();
        }
        return ms;
    }
    
    private void buildProxyMethods() {
        final Method[] methods = (Method[])
            AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return interfaceClass.getMethods();
                }
            });

        proxyMethods = new HashMap<Method, ProxyMethod>(); 
        for (Method m : methods) {
            ProxyMethod pm = null;
            try {
                pm = new ProxyMethod(m);
            } catch (OpenDataException e) {
                final IllegalArgumentException iae =
                    new IllegalArgumentException(interfaceClass.getName() +
                        " is not a valid MXBean interface.");
                iae.initCause(e);
                throw iae;
            }
            proxyMethods.put(m, pm);
        }
    }

    public Object forward(MBeanServerConnection mbsc,
                          ObjectName objName,
                          Method method, 
                          Object[] args) 
            throws Throwable {
        ProxyMethod pm = proxyMethods.get(method);
        if (pm == null) {
            pm = new ProxyMethod(method);
            proxyMethods.put(method, pm);
        }

        return pm.invoke(mbsc, objName, args);
    }

    private static final Type[] NO_PARAMS = new Type[0];
    static class ProxyMethod {
        Method   method;
        String   name;
        boolean  isSetter;
        boolean  isGetter;
        Type     returnType;
        Type[]   paramTypes;
        String[] openParamTypeSignatures;
        ProxyMethod(Method m) throws OpenDataException {
            this.method = m;
            this.isGetter = isGetter(m);
            this.isSetter = isSetter(m);
            if (isGetter) {
                this.name = getAttributeName(m);
                this.returnType = m.getGenericReturnType();
                this.paramTypes = NO_PARAMS;
            } else if (isSetter) {
                this.name = getAttributeName(m);
                this.paramTypes = m.getGenericParameterTypes();
            } else {
                // an operation
                this.name = method.getName();
                this.paramTypes = m.getGenericParameterTypes();
                this.returnType = m.getGenericReturnType();
            }
            openParamTypeSignatures = new String[paramTypes.length];
            for (int i = 0; i < paramTypes.length; i++) {
                MappedMXBeanType mappedType = 
                    MappedMXBeanType.getMappedType(paramTypes[i]);
                openParamTypeSignatures[i] = 
                    mappedType.getMappedTypeClass().getName();
            }
        } 
        
        // forward this method call to the MBeanServerConnection
        // - input parameters are converted from a declared Java type
        //   to its mapped open type before passing to MBSC
        // - return value is converted from a mapped open type to
        //   the declared Java type before returning to the proxy 
        Object invoke(MBeanServerConnection mbsc,
                      ObjectName objName,
                      Object[] args) throws Throwable {

            try {
                if (isGetter) { 
                    Object result = mbsc.getAttribute(objName, 
                                                      name);
                    return MappedMXBeanType.toJavaTypeData(result, returnType);
                } else if (isSetter) {
                    Object op = MappedMXBeanType.toOpenTypeData(args[0], 
                                                                paramTypes[0]);
                    Attribute attr = new Attribute(name, op);
                    mbsc.setAttribute(objName, attr);
                    return null;
                } else {
                    final int nargs = (args == null) ? 0 : args.length;
    
                    Object[] openArgs = new Object[nargs];
                    for (int i = 0; i < nargs; i++) {
                        openArgs[i] = 
                            MappedMXBeanType.toOpenTypeData(args[i], 
                                                            paramTypes[i]);
                    }
                    // Let the MBeanServer to check if the input parameters
                    // are of the right types. Expect no error since
                    // this method is only called from the proxy.
                    Object result = mbsc.invoke(objName, 
                                                name, 
                                                openArgs, 
                                                openParamTypeSignatures);
                    return MappedMXBeanType.toJavaTypeData(result, 
                                                           returnType);
                }
            } catch (MBeanException e) {
                // unwrap MBeanException
                throw e.getTargetException();
            } catch (JMRuntimeException e) {
                // unwrap JMRuntimeException
                Throwable t = e.getCause();
                while (t instanceof JMRuntimeException) {
                    // For some reason, RuntimeException was wrapped 
                    // multiple times
                    t = t.getCause();
                }
                throw t;
            }
        }
    }

    // Support for the DynamicMBean interface implementation
    // for a given MXBean
    public Object getAttribute(Object mxbean, String attributeName)
        throws AttributeNotFoundException,
               MBeanException,
               ReflectionException {
        if (attributeName == null) {
            throw new RuntimeOperationsException(
                new IllegalArgumentException("attributeName cannot be null"),
                "Exception occured trying to invoke the getter on the MBean");
        }

        // make sure the MXBean interface has been parsed
        synchronized (this) {
            if (attrMethods == null) {
                parse();
            }
        }

        AttributeMethod am = attrMethods.get(attributeName);
        if (am != null && am.getter != null) {
            try {
                Object result = am.getter.invoke(mxbean);
                return MappedMXBeanType.toOpenTypeData(result, am.getType());
            } catch (InvocationTargetException e) {
                throw unwrapException(e, 
                    "when getting attribute " + attributeName);
            } catch (IllegalAccessException e) {
                throw new ReflectionException(e, "IllegalAccessException " +
                    "when getting attribute " + attributeName);
            } catch (Exception e) {
                throw new MBeanException(e, "Exception thrown when " +
                    " getting attribute " + attributeName);
            }
        } else {
            throw new AttributeNotFoundException("Attribute " + attributeName +
                " does not exist.");
        }
    }


    public void setAttribute(Object mxbean, Attribute attribute)
        throws AttributeNotFoundException,
               InvalidAttributeValueException,
               MBeanException,
               ReflectionException {
        if (attribute == null) {
            throw new RuntimeOperationsException(
                new IllegalArgumentException("attribute cannot be null"),
                "Exception occured trying to invoke the setter on the MBean");
        }

        // make sure the MXBean interface has been parsed
        synchronized (this) {
            if (attrMethods == null) {
                parse();
            }
        }

        AttributeMethod am = attrMethods.get(attribute.getName());
        if (am != null && am.setter != null) {
            try {
                Object param = MappedMXBeanType.toJavaTypeData(attribute.getValue(), 
                                                               am.getType());
                am.setter.invoke(mxbean, param);
            } catch (InvocationTargetException e) {
                throw unwrapException(e, 
                    "when setting attribute " + attribute.getName());
            } catch (IllegalAccessException e) {
                throw new ReflectionException(e, "IllegalAccessException " +
                    "when setting attribute " + attribute.getName());
            } catch (Exception e) {
                throw new MBeanException(e, "Exception thrown when " +
                    " setting attribute " + attribute.getName());
            }
        } else {
            throw new AttributeNotFoundException("Attribute " + 
                attribute.getName() + " does not exist.");
        }
    }

    public AttributeList getAttributes(Object mxbean, String[] attributes) {
        if (attributes == null) {
            throw new RuntimeOperationsException(
                new IllegalArgumentException("attributes cannot be null"),
                "Exception occured trying to invoke the getter on the MBean");

        }
        AttributeList resultList = new AttributeList();
        if (attributes.length == 0) {
            return resultList;
        }

        for (int i = 0; i < attributes.length ; i++){
            try {
                Object value = getAttribute(mxbean, attributes[i]);
                resultList.add(new Attribute(attributes[i],value));
            } catch (Exception e) {
                // getAttributes does not throw any exception
                // TODO: log the exception
            }
        }
        return(resultList);
    }

    public AttributeList setAttributes(Object mxbean, AttributeList attributes) {
        if (attributes == null) {
            throw new RuntimeOperationsException(
                new IllegalArgumentException("Attributes cannot be null"),
                "Exception occured trying to invoke the setter on the MBean");
        }

        AttributeList resultList = new AttributeList();

        // if attributeNames is empty, nothing more to do
        if (attributes.isEmpty()) {
            return resultList;
        }

        // make sure the MXBean interface has been parsed
        synchronized (this) {
            if (attrMethods == null) {
                parse();
            }
        }

        // for each attribute, try to set it and add to the result list
        // if successful
        for (Iterator i = attributes.iterator(); i.hasNext();) {
            Attribute attr = (Attribute) i.next();
            try {
                setAttribute(mxbean, attr);
                String name = attr.getName();
                Object value = getAttribute(mxbean, name);
                resultList.add(new Attribute(name,value));
            } catch(Exception e) {
                // setAttributes does not throw any exception
                // TODO: log the exception
            }
        }
        return(resultList);
    }

    public Object invoke(Object mxbean, String operationName, 
                         Object openParams[], String signature[])
        throws MBeanException, ReflectionException { 
        if (operationName == null) {
            throw new RuntimeOperationsException(
              new IllegalArgumentException("Operation name cannot be null"),
              "Exception occured trying to invoke the operation on the MBean");
        }

        // make sure the MXBean interface has been parsed
        synchronized (this) {
            if (opMethods == null) {
                parse();
            }
        }

        List<Method> methods = opMethods.get(operationName);
        if (methods != null) {
            for (Method m : methods) { 
                try {
                    // : validate the parameter types and signature
                    final Type[] paramTypes = m.getGenericParameterTypes();
                    final int nargs = (openParams == null ? 0 
                                                          : openParams.length);
                    final int nsigs = (signature == null ? 0 
                                                          : signature.length);
                    Object[] params = new Object[nargs];
                    if (paramTypes.length != nargs ||
                        paramTypes.length != nsigs) {
                        // unmatched signature
                        continue;
                    }
    
                    boolean sigMatched = true;
                    for (int i = 0; i < nargs ; i++) {
                        final MappedMXBeanType mappedType = 
                            MappedMXBeanType.getMappedType(paramTypes[i]);
                        String typeName = mappedType.getMappedTypeClass().getName();
                        if (!signature[i].equals(typeName)) {
                            // unmatched signature 
                            sigMatched = false;
                            break;
                        }
                        params[i] = mappedType.toJavaTypeData(openParams[i]);
                    } 
                    if (sigMatched) {
                        Object result = m.invoke(mxbean, params);
                        return MappedMXBeanType.toOpenTypeData(result,
                                                               m.getGenericReturnType());
                    } 
                } catch (InvocationTargetException e) {
                    throw unwrapException(e, 
                        "when invoking operation " + operationName);
                } catch (IllegalAccessException e) {
                    throw new ReflectionException(e, "IllegalAccessException " +
                        "when invoking operation " + operationName);
                } catch (Exception e) {
                    // wrap the exception with MBeanException
                    throw new MBeanException(e, "Exception thrown when " +
                        " invoking operation " + operationName);
                }
            }
        }
        throw new ReflectionException(
            new NoSuchMethodException("Operation " + operationName + 
                                      " does not exist."));
    }
 
    private static MBeanAttributeInfo[] 
        buildAttributeInfoArray(Map<String, AttributeMethod> attrMethods) {

        MBeanAttributeInfo[] ai = 
            new MBeanAttributeInfo[attrMethods.size()];

        int i = 0;
        for (AttributeMethod am : attrMethods.values()) {
            final MappedMXBeanType mappedType;
            try {
                mappedType = MappedMXBeanType.getMappedType(am.getType());
            } catch (OpenDataException e) {
                throw Util.newInternalError(e);
            }
            ai[i++] = new MBeanAttributeInfo(am.getName(),
                                             mappedType.getTypeName(),
                                             am.getName(),
                                             am.isReadable(),
                                             am.isWriteable(),
                                             am.isIs());
        }
        return ai;
    }

    private static MBeanOperationInfo[] 
        buildOperationInfoArray(Map<String, List<Method>> opMethods,
                                int numOperations) {

        MBeanOperationInfo[] oi = 
            new MBeanOperationInfo[numOperations];

        // Assume all operations has read-write impact
        final int impact = MBeanOperationInfo.ACTION_INFO;

        int i = 0;
        for (List<Method> methods : opMethods.values()) {
            for (Method m : methods) {
                final MappedMXBeanType mappedReturnType; 
                try {
                    mappedReturnType = 
                        MappedMXBeanType.getMappedType(m.getGenericReturnType());
                } catch (OpenDataException e) {
                    throw Util.newInternalError(e);
                }
        
                final Type[] paramTypes = m.getGenericParameterTypes();
                final MBeanParameterInfo[] pi =
                    new MBeanParameterInfo[paramTypes.length];
                for (int j = 0; j < paramTypes.length; j++) {
                    final String paramName = "p" + j;
                    final String paramDesc = paramName;
                       
                    final MappedMXBeanType mappedParamType;
                    try {
                        mappedParamType =
                            MappedMXBeanType.getMappedType(paramTypes[j]);
                    } catch (OpenDataException e) {
                        throw Util.newInternalError(e);
                    }
                    pi[j] = new MBeanParameterInfo(paramName,
                                                   mappedParamType.getTypeName(),
                                                   paramDesc);
                }
                oi[i++] = new MBeanOperationInfo(m.getName(),
                                                 m.getName(),
                                                 pi,
                                                 mappedReturnType.getTypeName(),
                                                 impact);

            }
        }
        assert(i == numOperations);
        return oi;
    }

    private static MBeanException unwrapException(InvocationTargetException e,
                                                  String message) {
        // unwrap the exception.
        Throwable t = e.getTargetException();
        if (t instanceof RuntimeException) {
            throw (RuntimeException) t;
        } else if (t instanceof Error) {
            throw (Error) t;
        } else {
            return new MBeanException((Exception) t,
               "Exception thrown " + message);
        }

    }

}
