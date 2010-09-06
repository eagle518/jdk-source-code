/*
 * @(#)TextFieldProperty.java	1.5 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
