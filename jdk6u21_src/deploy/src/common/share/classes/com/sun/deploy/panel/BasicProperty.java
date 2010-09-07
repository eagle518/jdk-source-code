/*
 * @(#)BasicProperty.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeCellEditor;
import java.util.HashMap;
import java.util.Locale;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;

/**
 * @author  mfisher
 */

/*
 * This class described all the common methods for all the nodes.  Basically,
 * all nodes will differ only in their renderers and editors.
 *
 * - sName is the string to be displayed at the node 
 * - propertyName is the name of the property as we find it in the properties file
 * - sValue is the string that represents currently selected value.
 * - tooltipStr is the string to be used in the tooltip for the node.
 */
public abstract class BasicProperty implements IProperty {

    /** Creates new BasicProperty */
    public BasicProperty(String propertyName, String sValue ) {
        this.sName = ResourceManager.getMessage(propertyName);
        this.propertyName = propertyName;        
        this.sValue = sValue;
        
        String tooltip = ResourceManager.getMessage(propertyName + ".tooltip");
        // Make sure we did not get back the key instead of description...
        if (! tooltip.equalsIgnoreCase(propertyName + ".tooltip") ){
            tooltipStr = tooltip;
        } else {
	    tooltipStr = sName;
	}
    }

    /*
     * Returns the name of this node.
     */
    public String getDescription(){
        return sName;
    }
    
    /*
     * Returns the name of the property.
     */
    public String getPropertyName(){
        return propertyName;
    }
    
    /*
     * Return tooltip string for the node.
     */
    public String getTooltip(){
        return tooltipStr;
    }

    /**
     * Returns value.
     */
    public String getValue(){
        return sValue;
    }

    /*
     * Sets the value.
     */
    public void setValue( String sValue ){
        this.sValue = sValue;
        
        /*
         * Set new value for the property in Config.
         */
        Config.setProperty( propertyName, sValue );
    }

    private String sName, sValue, propertyName, tooltipStr;
}
