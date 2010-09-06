/*
 * @(#)IProperty.java	1.5 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

public interface IProperty 
{
    /**
     * Return the description of the property.
     */
    public String getDescription();
    
    /*
     * Returns the name of the property.
     */
    public String getPropertyName();
    
    /**
     * Return string for the tooltip.
     */
    public String getTooltip();

    /**
     * Returns value.
     */
    public String getValue();

    /*
     * Sets the value.
     */
    public void setValue( String sValue );

    /*
     * Check if property is selected.
     */
    public boolean isSelected();
}
