/*
 * @(#)ComponentBeanInfo.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.infos;

import java.beans.*;

/**
 * BeanInfo descriptor for a standard AWT component.
 */

public class ComponentBeanInfo extends SimpleBeanInfo {
    private static final Class beanClass = java.awt.Component.class;

    public PropertyDescriptor[] getPropertyDescriptors() {
        try {
            PropertyDescriptor 
		      name = new PropertyDescriptor("name",       beanClass),
		background = new PropertyDescriptor("background", beanClass),
	        foreground = new PropertyDescriptor("foreground", beanClass),
		      font = new PropertyDescriptor("font",       beanClass),
		   enabled = new PropertyDescriptor("enabled",    beanClass),
	           visible = new PropertyDescriptor("visible",    beanClass),
		 focusable = new PropertyDescriptor("focusable",  beanClass);

	    enabled.setExpert(true);
            visible.setHidden(true);

            background.setBound(true);
            foreground.setBound(true);
            font.setBound(true);
            focusable.setBound(true);
      
            PropertyDescriptor[] rv = {name, background, foreground, font, enabled, visible, focusable };
            return rv;
        } catch (IntrospectionException e) {
            throw new Error(e.toString());
        }
    }
}


