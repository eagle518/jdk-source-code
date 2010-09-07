/*
 * @(#)TextFieldProperty.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import com.sun.deploy.config.Config;


/*
 * This property contains a command to launch the browser.  
 * 
 */

public class TextFieldProperty extends BasicProperty {

    /** Creates new TextFieldProperty */
    public TextFieldProperty(String propertyName, String sValue) {
        super(propertyName, 
              sValue );
        
        String currentValue = Config.getProperty(propertyName);
        if ( currentValue != null ){
            setValue(currentValue);
        }
    }
    
    /*
     * There is no components in this property that could be selected,
     * so return "false".
     */
    public boolean isSelected(){
        return false;
    }    
}
