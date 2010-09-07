/*
 * @(#)RadioProperty.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import java.util.Locale;


/* 
 * RadioProperty is similar to the ToggleProperty.  The difference
 * is that RadioProperty always belongs to some RadioPropertyGroup,
 * which tracks which radio node out of the group is currently 
 * selected.
 *
 * Current selection is retrieved by RadioPropertyGroup constructor
 * from Config.
 *
 */
final class RadioProperty extends BasicProperty {

    public RadioProperty(String propertyName, String sValue ) {
        super(propertyName + "." + sValue, sValue);
    }
    
    /*
     * Set RadioPropertyGroup for this button.
     */
    public void setGroup(RadioPropertyGroup g){
        group = g;
    }
    
    /*
     * Check if this radio button is selected.
     */
    public boolean isSelected(){
        if ( group.getCurrentSelection().equalsIgnoreCase(getValue()) )
            return true;
        else 
            return false;
    }
    
    private RadioPropertyGroup group;
    
    /*
     * Set current selection in radio group.
     */
    public void setValue(String sValue){        
        group.setCurrentSelection(getValue());        
    }    

    public String getGroupName() {
	return group.getPropertyName();
    }
}
