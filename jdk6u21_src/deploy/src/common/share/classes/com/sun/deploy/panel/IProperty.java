/*
 * @(#)IProperty.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
