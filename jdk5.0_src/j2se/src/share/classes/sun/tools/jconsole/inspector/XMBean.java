/*
 * @(#)XMBean.java	1.7 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
//

// jmx import
import javax.management.*;
import javax.management.openmbean.OpenMBeanInfo;
import javax.management.modelmbean.ModelMBeanInfo;
//

// swing import
import javax.swing.*;
import javax.swing.border.*;
//
import sun.tools.jconsole.MBeansTab;

public class XMBean extends Object {	
    private ObjectName objectName;
    private Icon icon;
    private String text;
    private boolean broadcaster;
    private MBeansTab mbeansTab;
    
    public XMBean(ObjectName objectName, 
		  MBeansTab mbeansTab) {
	this.mbeansTab = mbeansTab;
	setObjectName(objectName);
	try {
	    icon = selectIcon();
	}catch(Exception e) {
	    System.out.println("Error creating XMBean :"+ 
			       e.getMessage());
	}
    }
    
    private Icon selectIcon() {
	String className;
	try {
	    className =getClassName().toLowerCase();
	}
	catch (Exception e) {
	    className = getClass().getName().toLowerCase();
	}
	try {
	    if (className.equals("javax.management.mbeanserverdelegate"))
		return IconManager.MBEANSERVERDELEGATE;
	    
	    MBeanInfo info = getMBeanInfo();
	    
	    if(info instanceof OpenMBeanInfo)
		return IconManager.OPENMBEAN;
	    
	    if(info instanceof ModelMBeanInfo)
		return IconManager.MODELMBEAN;
	    
	    return IconManager.STANDARDMBEAN;
	}
	catch (Throwable e) {
	    return IconManager.STANDARDMBEAN;
	}
    }	
    
    MBeanServerConnection getMBeanServerConnection() {
	return mbeansTab.getMBeanServerConnection();
    }

    //For XObjectMBeanWrapper 
    protected XMBean(Object object) {
	try {
	    ObjectName objectName = 
		new ObjectName("DefaultDomain",
			       "name",
			       Utils.generateName(object.getClass().toString()));
	    setObjectName(objectName);
	}
	catch(Exception e) {}
    }
    
    public boolean isBroadcaster() {
	try {
	    return
		getMBeanServerConnection().isInstanceOf(getObjectName(), 
							"javax.management.NotificationBroadcaster");
	}catch(Exception e) {
	     System.out.println("Error calling isBroadcaster :"+ 
				e.getMessage());
	}
	return false;
    }
	
    public String getClassName() 
	throws InstanceNotFoundException, IOException {
	return getObjectInstance().getClassName();
    }
	
    public Object invoke(String operationName) throws Exception {
	Object result = getMBeanServerConnection().invoke(getObjectName(),
							  operationName,
							  new Object[0],
							  new String[0]);
	return result;
    }
	
    public Object invoke(String operationName, Object params[], String sig[]) 
	throws Exception {
	Object result = getMBeanServerConnection().invoke(getObjectName(),
							  operationName,
							  params,
							  sig);
	return result;
    }
	
    public void setAttribute(Attribute attribute)
	throws InstanceNotFoundException,
	       AttributeNotFoundException,
	       InvalidAttributeValueException,
	       MBeanException,
	       ReflectionException,
	       IOException {
	getMBeanServerConnection().setAttribute(getObjectName(),attribute);
    }
	
    public Object getAttribute(String attributeName)
	throws MBeanException,
	       AttributeNotFoundException,
	       InstanceNotFoundException,
	       ReflectionException,
	       IOException  {
	return getMBeanServerConnection().getAttribute(getObjectName(),
						       attributeName);
    }

    public AttributeList getAttributes(String attributeNames[]) 
	throws MBeanException,
	       AttributeNotFoundException,
	       InstanceNotFoundException,
	       ReflectionException,
	       IOException  {
	return getMBeanServerConnection().getAttributes(getObjectName(),
							attributeNames);
    }
	
    public AttributeList getAttributes(MBeanAttributeInfo attributeNames[]) 
	throws MBeanException,
	       AttributeNotFoundException,
	       InstanceNotFoundException,
	       ReflectionException,
	       IOException  {
	String attributeString[] = new String[attributeNames.length];
	for (int i = 0; i < attributeNames.length; i++) {
	    attributeString[i] = attributeNames[i].getName();
	}
	return getAttributes(attributeString);
    }
	
    public ObjectInstance getObjectInstance() 
	throws javax.management.InstanceNotFoundException,
	       IOException  {
	return getMBeanServerConnection().getObjectInstance(getObjectName());
    }
	
    public ObjectName getObjectName() {
	return objectName;
    }
	
    private void setObjectName(ObjectName objectName) {
	this.objectName = objectName;
	// generate a readable name now
	String name = getObjectName().getKeyProperty("name");
	if (name==null)
	    setText(getObjectName().getDomain());
	else
	    setText(name);			
    }
	
    public MBeanInfo getMBeanInfo() 
	throws InstanceNotFoundException,
	       IntrospectionException,
	       ReflectionException,
	       IOException  {
	return getMBeanServerConnection().getMBeanInfo(getObjectName());
    }	
	
    public boolean equals(Object o) {
	if (o instanceof XMBean) {
	    XMBean mbean = (XMBean) o;
	    return getObjectName().equals((mbean).getObjectName());
	}
	return false;
    }
    
    public String getText() {
	return text;
    }
	
    public void setText(String text) {
	this.text = text;
    }
	
	
    public Icon getIcon() {
	return icon;
    }
	
    public void setIcon(Icon icon) {
	this.icon = icon;
    }
	
    public String toString() {
	return getText();
    }
}
