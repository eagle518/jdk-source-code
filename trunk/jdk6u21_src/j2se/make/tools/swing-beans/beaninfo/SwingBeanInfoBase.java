/*
 * @(#)SwingBeanInfoBase.java	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.swing;

import java.beans.*;
import java.lang.reflect.*;
import java.awt.Image;

/**
 * The superclass for all Swing BeanInfo classes.  It provides
 * default implementations of <code>getIcon</code> and 
 * <code>getDefaultPropertyIndex</code> as well as utility
 * methods, like createPropertyDescriptor, for writing BeanInfo
 * implementations.  This classes is intended to be used along
 * with <code>GenSwingBeanInfo</code> a BeanInfo class code generator.
 * 
 * @see GenSwingBeanInfo
 * @version 1.23 03/23/10
 * @author Hans Muller
 */
public class SwingBeanInfoBase extends SimpleBeanInfo
{
    /**
     * The default index is always 0.  In other words the first property
     * listed in the getPropertyDescriptors() method is the one
     * to show a (JFC builder) user in a situation where just a single
     * property will be shown.
     */
    public int getDefaultPropertyIndex() {
	return 0;
    }

    /**
     * Returns a generic Swing icon, all icon "kinds" are supported.
     * Subclasses should defer to this method when they don't have
     * a particular beans icon kind.
     */
    public Image getIcon(int kind) {
	// PENDING(hmuller) need generic swing icon images.
	return null;
    }

    /**
     * Returns the BeanInfo for the superclass of our bean, so that
     * its PropertyDescriptors will be included.
     */
    public BeanInfo[] getAdditionalBeanInfo() {
        Class superClass = getBeanDescriptor().getBeanClass().getSuperclass();
        BeanInfo superBeanInfo = null;
        try {
            superBeanInfo = Introspector.getBeanInfo(superClass);
        } catch (IntrospectionException ie) {}
        if (superBeanInfo != null) {
            BeanInfo[] ret = new BeanInfo[1];
            ret[0] = superBeanInfo;
            return ret;
        }
        return null;
    }
}

