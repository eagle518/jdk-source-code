/*
 * @(#)JMProperties.java	1.6 04/05/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management.jmx;

// java import
//
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 * This class reads a file containing the property list defined for Java DMK
 * and adds all the read properties to the list of system properties.
 *
 * @deprecated This class is obsolete. 
 *
 * @since 1.5
 */
@Deprecated
public class JMProperties {

    // private constructor defined to "hide" the default public constructor
	    private JMProperties() {

    }

    // PUBLIC STATIC METHODS
    //----------------------

    /**
     * Reads the JMX RI property list from a file and
     * adds the read properties as system properties.
     * @deprecated This class is obsolete. 
     */
    @Deprecated
    public static void load(String file) throws IOException {
        Properties props = new Properties(System.getProperties());
        InputStream is = new FileInputStream(file);
        props.load(is);
        is.close();
        System.setProperties(props);
    }

    // PUBLIC STATIC VARIABLES
    //------------------------

    /**
     * References the property that specifies the directory where
     * the native libraries will be stored before the MLet Service
     * loads them into memory.
     * <p>
     * Property Name: <B>jmx.mlet.library.dir</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
    public static final String MLET_LIB_DIR = 
	com.sun.jmx.defaults.JmxProperties.MLET_LIB_DIR;


    /**
     * References the property that specifies the full name of the JMX
     * specification implemented by this product.
     * <p>
     * Property Name: <B>jmx.specification.name</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
     public static final String JMX_SPEC_NAME =  
	 com.sun.jmx.defaults.JmxProperties.JMX_SPEC_NAME;
    
    /**
     * References the property that specifies the version of the JMX
     * specification implemented by this product.
     * <p>
     * Property Name: <B>jmx.specification.version</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
     public static final String JMX_SPEC_VERSION =   
	 com.sun.jmx.defaults.JmxProperties.JMX_SPEC_VERSION;
    
    /**
     * References the property that specifies the vendor of the JMX
     * specification implemented by this product.
     * <p>
     * Property Name: <B>jmx.specification.vendor</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
     public static final String JMX_SPEC_VENDOR =    
	 com.sun.jmx.defaults.JmxProperties.JMX_SPEC_VENDOR;
    
    /**
     * References the property that specifies the full name of this product
     * implementing the  JMX specification.
     * <p>
     * Property Name: <B>jmx.implementation.name</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
    public static final String JMX_IMPL_NAME =     
	com.sun.jmx.defaults.JmxProperties.JMX_IMPL_NAME;
    
    /**
     * References the property that specifies the name of the vendor of 
     * this product implementing the  JMX specification.
     * <p>
     * Property Name: <B>jmx.implementation.vendor</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
    public static final String JMX_IMPL_VENDOR =     
	com.sun.jmx.defaults.JmxProperties.JMX_IMPL_VENDOR;
    
    /**
     * References the property that specifies the version of this product
     * implementing the  JMX specification.
     * <p>
     * Property Name: <B>jmx.implementation.version</B>
     * @deprecated This class is obsolete. 
     */
    @Deprecated
    public static final String JMX_IMPL_VERSION =     
	com.sun.jmx.defaults.JmxProperties.JMX_IMPL_VERSION;
}
