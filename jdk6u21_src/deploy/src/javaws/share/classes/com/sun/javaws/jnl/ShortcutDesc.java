/*
 * @(#)ShortcutDesc.java	1.5 10/03/24* 
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class ShortcutDesc implements XMLable {

    private boolean _online;
    private boolean _desktop;
    private boolean _menu;
    private String  _submenu;
        
    public ShortcutDesc(boolean online, boolean desktop, 
			boolean menu, String submenu) { 

	_online = online;
	_desktop = desktop;
	_menu = menu;
	_submenu = submenu;
    }

    public boolean getOnline()  { return _online; }
    public boolean getDesktop() { return _desktop; }
    public boolean getMenu()    { return _menu; }
    public String getSubmenu()  { return _submenu; }
    
         
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("online", _online);
	XMLNodeBuilder nb = new XMLNodeBuilder("shortcut", 
				ab.getAttributeList());

	if (_desktop) {
            nb.add(new XMLNode("desktop", null));
	}
	if (_menu) {
	    if (_submenu == null) {
		nb.add(new XMLNode("menu", null));
	    } else {
		nb.add(new XMLNode("menu", new XMLAttribute("submenu", _submenu)));
	    }
	}
	return nb.getNode();
    }
}




