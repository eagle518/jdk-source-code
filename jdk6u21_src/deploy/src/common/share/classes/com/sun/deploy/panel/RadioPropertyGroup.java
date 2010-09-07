/*
 * @(#)RadioPropertyGroup.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import com.sun.deploy.config.Config;

/*
 * This class takes care of tracking which element in the group
 * of radio button nodes is currently selected.
 *
 * The update of the tree is done by registering mouse listener on
 * the tree in TreeBuilder.  So, every time user clicks on a new 
 * radio button node, the selectionKey in RadioPropertyGroup gets
 * updated, and repain of the tree is triggered in the mouse click
 * handler, which causes the new selection to be painted.
 *
 */


public class RadioPropertyGroup {

    /** Creates new RadioPropertyGroup */
    public RadioPropertyGroup(String propName, String selection) {
        propertyName = propName;        
        selectionKey = selection;
        
        /*
         * Check if this property is set in Config.  If it is,
         * then overwrite default with value from Config.
         */
        String configValue = Config.getProperty(propertyName);
        if ( configValue != null ){
            selectionKey = configValue;
        }
    }
    
    public void setCurrentSelection(String selection){
        if (selection.trim().equals(""))
            return;
        
        selectionKey = selection;        
        Config.setProperty(propertyName, selectionKey);
    }
    
    public String getCurrentSelection(){
        return selectionKey;
    }
    
    public String getPropertyName(){
        return propertyName;
    }    

    private String propertyName, selectionKey;
}
