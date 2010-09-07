/*
 * @(#)ToggleProperty.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.Properties;
import com.sun.deploy.config.Config;


/*
 * This is how ToggleProperty instance would be stored:
 * sName = "Logging Enabled"
 * propertyName = "deployment.log"
 * sValue = "true" or "false"
 * tooltipStr = "Create log file to capture errors."
 */
final class ToggleProperty extends BasicProperty {

    public ToggleProperty(String propertyName, String sValue ) {
        /*
         * Since this property is for the node with a checkbox, there are only
         * two possible values - true, or false.
         */
        super(propertyName, sValue);
        
        /*
         * Get the current value for the property from deployment.properties
         */
        String currentValue = Config.getProperty(propertyName);
        if ( currentValue != null ){
            setValue(currentValue);
        }
    }
    
    /*
     * Return true is this property is currently selected,
     * else return false.
     */
    public boolean isSelected(){
        if ( "true".equalsIgnoreCase( getValue() ) )
            return true;
        else
            return false;
    }

}
