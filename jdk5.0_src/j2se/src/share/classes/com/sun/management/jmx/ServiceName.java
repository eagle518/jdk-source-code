/*
 * @(#)ServiceName.java	1.7 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management.jmx;

/**
 * This class is used for storing the names of core services.
 *
 * @deprecated  Call MBeanServerDelegateMBean instead.
 *
 * @since 1.5
 */
@Deprecated
public class ServiceName  {

    private ServiceName() {
	
    }

    /**
     * The object name of the MBeanServer delegate object
     * <BR>
     * The value is <CODE>JMImplementation:type=MBeanServerDelegate</CODE>.
     */
    public static final String DELEGATE =
	com.sun.jmx.defaults.ServiceName.DELEGATE;
    
    
    /**
     * The default key properties for registering the class loader of the 
     * MLet service.
     * <BR>
     * The value is <CODE>type=MLet</CODE>.
     */
    public static final String MLET = 
	com.sun.jmx.defaults.ServiceName.MLET;
    
    /**
     * The default domain.
     * <BR>
     * The value is <CODE>DefaultDomain</CODE>.
     */
    public static final String DOMAIN = 
	com.sun.jmx.defaults.ServiceName.DOMAIN;

    /**
     * The default port for the HTML adaptor.
     * <BR>
     * The value is <CODE>8082</CODE>.
     */
    public static final int HTML_ADAPTOR_PORT = 8082 ;    

   /**
     * The default key properties for the HTML protocol adaptor.
     * <BR>
     * The value is <CODE>name=HtmlAdaptorServer</CODE>.
     */
    public static final String HTML_ADAPTOR_SERVER = "name=HtmlAdaptorServer" ;
   
    /**
     * The name of the JMX specification implemented by this product.    
     * <BR>
     * The value is <CODE>Java Management Extensions</CODE>.
     */
     public static final String JMX_SPEC_NAME = 
	 com.sun.jmx.defaults.ServiceName.JMX_SPEC_NAME;

    /**
     * The version of the JMX specification implemented by this product.
     * <BR>
     * The value is <CODE>1.0 Final Release</CODE>.
     */
     public static final String JMX_SPEC_VERSION = 
	 com.sun.jmx.defaults.ServiceName.JMX_SPEC_VERSION;
    
    /**
     * The vendor of the JMX specification implemented by this product.     
     * <BR>
     * The value is <CODE>Sun Microsystems</CODE>.
     */
     public static final String JMX_SPEC_VENDOR = 
	 com.sun.jmx.defaults.ServiceName.JMX_SPEC_VENDOR;
    
    /**
     * The name of this product implementing the  JMX specification.
     * <BR>
     * The value is <CODE>JMX RI</CODE>.
     */
    public static final String JMX_IMPL_NAME = 
	com.sun.jmx.defaults.ServiceName.JMX_IMPL_NAME;
    
    /**
     * The name of the vendor of this product implementing the  
     * JMX specification.  
     * <BR>
     * The value is <CODE>Sun Microsystems</CODE>.
     */
    public static final String JMX_IMPL_VENDOR = 
	com.sun.jmx.defaults.ServiceName.JMX_IMPL_VENDOR;
    
    /**
     * The version of this product implementing the  JMX specification.  
     * <BR>
     * The value is <CODE>1.0</CODE>.
     */
    public static final String JMX_IMPL_VERSION = 
	com.sun.jmx.defaults.ServiceName.JMX_IMPL_VERSION;

    /**
     * The build number of the current product version 
     */
    public static final String BUILD_NUMBER = "unknown";
	
    
}

