/*
 * @(#)MBeanServerImpl.java	1.7 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.management.jmx;

// java import
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.lang.reflect.InvocationTargetException; 
import java.lang.reflect.Method; 
import java.lang.reflect.Constructor;
import java.io.OptionalDataException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException ;

// RI import
import javax.management.*; 
import javax.management.loading.ClassLoaderRepository; 


/**
 * This class should not be used directly, 
 * use {@link javax.management.MBeanServerFactory} instead.
 *
 * @deprecated Get a MBeanServer from 
 *     {@link javax.management.MBeanServerFactory} instead.
 *
 * @since 1.5
 */
@Deprecated
public class MBeanServerImpl implements MBeanServer { 

    private final MBeanServer server; 

    public MBeanServerImpl()  { 
        this(null);
    } 

    public MBeanServerImpl(String domain)  { 
	MBeanServerBuilder  builder  = new MBeanServerBuilder();
	MBeanServerDelegate delegate = builder.newMBeanServerDelegate();
	server = builder.newMBeanServer(domain,null,delegate);
    } 

    public Object instantiate(String className) 
	throws ReflectionException, MBeanException { 
        return server.instantiate(className);
    }

    public Object instantiate(String className, ObjectName loaderName) 
        throws ReflectionException, MBeanException, 
	InstanceNotFoundException { 
        return server.instantiate(className,loaderName);

    }

    public Object instantiate(String className, Object params[], 
			      String signature[]) 
        throws ReflectionException, MBeanException { 

        return server.instantiate(className, params, signature);
    }

    public Object instantiate(String className, ObjectName loaderName, 
			      Object params[], String signature[]) 
        throws ReflectionException, MBeanException, 
	InstanceNotFoundException { 
        return server.instantiate(className, loaderName, params, signature);
    }

    public ObjectInstance createMBean(String className, ObjectName name)
        throws ReflectionException, InstanceAlreadyExistsException, 
	MBeanRegistrationException, MBeanException, 
	NotCompliantMBeanException { 

        return server.createMBean(className,name);
    }

    public ObjectInstance createMBean(String className, ObjectName name, 
				      ObjectName loaderName) 
        throws ReflectionException, InstanceAlreadyExistsException, 
	MBeanRegistrationException, MBeanException, NotCompliantMBeanException,
	InstanceNotFoundException { 
	return server.createMBean(className,name,loaderName);
    }

    public ObjectInstance createMBean(String className, ObjectName name, 
				      Object params[], String signature[]) 
        throws ReflectionException, InstanceAlreadyExistsException, 
	MBeanRegistrationException, MBeanException, 
	NotCompliantMBeanException  { 
	return server.createMBean(className,name,params,signature);
    }

    public ObjectInstance createMBean(String className, ObjectName name, 
				      ObjectName loaderName, 
				      Object params[], String signature[]) 
        throws ReflectionException, InstanceAlreadyExistsException, 
	MBeanRegistrationException, MBeanException, 
	NotCompliantMBeanException, InstanceNotFoundException { 
        return server.createMBean(className,name,loaderName,params,signature);
    }

    public ObjectInstance registerMBean(Object object, ObjectName name) 
	throws InstanceAlreadyExistsException, MBeanRegistrationException,
	NotCompliantMBeanException  { 
	return server.registerMBean(object,name);
    } 

    public void unregisterMBean(ObjectName name) 
	throws InstanceNotFoundException, MBeanRegistrationException  {    
	server.unregisterMBean(name);
    } 

    public ObjectInstance getObjectInstance(ObjectName name) 
	throws InstanceNotFoundException {
	return server.getObjectInstance(name);
    }

    public Set queryMBeans(ObjectName name, QueryExp query) {
	return server.queryMBeans(name,query);
    } 

    public Set queryNames(ObjectName name, QueryExp query) {
	return server.queryNames(name,query);
    } 

    public boolean isRegistered(ObjectName name)  { 
	return server.isRegistered(name);
    } 

    public Integer getMBeanCount()  { 
        return (server.getMBeanCount());
    } 

    public Object getAttribute(ObjectName name, String attribute) 
	throws
	MBeanException, AttributeNotFoundException, 
	InstanceNotFoundException, ReflectionException { 

        return server.getAttribute(name, attribute);
    } 

    public AttributeList getAttributes(ObjectName name, String[] attributes)
        throws InstanceNotFoundException, ReflectionException  { 
	return server.getAttributes(name,attributes);
    } 

    public void setAttribute(ObjectName name, Attribute attribute) 
	throws InstanceNotFoundException, AttributeNotFoundException, 
	InvalidAttributeValueException, MBeanException, 
	ReflectionException  { 
	server.setAttribute(name,attribute);
    }

    public AttributeList setAttributes(ObjectName name, 
				       AttributeList attributes)
        throws InstanceNotFoundException, ReflectionException  { 
	return server.setAttributes(name,attributes);
    }

    public Object invoke(ObjectName name, String operationName, 
			 Object params[], String signature[]) 
	throws InstanceNotFoundException, MBeanException, 
	ReflectionException { 
        return server.invoke(name, operationName, params, signature);
    }
    
    public String getDefaultDomain()  { 
        return server.getDefaultDomain();
    } 

    public String[] getDomains()  { 
        return server.getDomains();
    } 

    public void addNotificationListener(ObjectName name, 
					NotificationListener listener, 
					NotificationFilter filter, 
					Object handback)
        throws InstanceNotFoundException{
	server.addNotificationListener(name,listener,filter,handback);
    }

    public void addNotificationListener(ObjectName name, ObjectName listener,
					NotificationFilter filter, 
					Object handback)
        throws InstanceNotFoundException{
	server.addNotificationListener(name,listener,filter,handback);
    }

    public void removeNotificationListener(ObjectName name, 
					   NotificationListener listener )
        throws InstanceNotFoundException, ListenerNotFoundException {
	server.removeNotificationListener(name,listener);
    }

    public void removeNotificationListener(ObjectName name, 
					   ObjectName listener) 
        throws InstanceNotFoundException, ListenerNotFoundException {

        server.removeNotificationListener(name,listener) ;
    }

    public void removeNotificationListener(ObjectName name,
					   ObjectName listener,
					   NotificationFilter filter,
					   Object handback)
	throws InstanceNotFoundException, ListenerNotFoundException {
	server.removeNotificationListener(name,listener,filter,handback);
    }

    public void removeNotificationListener(ObjectName name,
					   NotificationListener listener,
					   NotificationFilter filter,
					   Object handback)
	throws InstanceNotFoundException, ListenerNotFoundException {
	server.removeNotificationListener(name,listener,filter,handback);
    }

    public MBeanInfo getMBeanInfo(ObjectName name) throws
	InstanceNotFoundException, IntrospectionException, 
	ReflectionException { 
	return server.getMBeanInfo(name);
    }

    public boolean isInstanceOf(ObjectName name, String className) 
	throws InstanceNotFoundException {
	return server.isInstanceOf(name,className);
    }

    public ObjectInputStream deserialize(ObjectName name, byte[] data)
        throws InstanceNotFoundException, OperationsException {
        return server.deserialize(name,data);
    }


    public ObjectInputStream deserialize(String className, byte[] data)
        throws OperationsException, ReflectionException {
        return server.deserialize(className,data);
    }
   
    
    public ObjectInputStream deserialize(String className, 
					 ObjectName loaderName, byte[] data)
        throws InstanceNotFoundException, OperationsException, 
	ReflectionException {
	return server.deserialize(className,loaderName,data);
    }
    
    public ClassLoader getClassLoaderFor(ObjectName mbeanName) 
	throws InstanceNotFoundException {
	return server.getClassLoaderFor(mbeanName);
    }

    public ClassLoader getClassLoader(ObjectName loaderName)
	throws InstanceNotFoundException {
	return server.getClassLoader(loaderName);	
    }

    public ClassLoaderRepository getClassLoaderRepository() {
	return server.getClassLoaderRepository();
    }
  
}
