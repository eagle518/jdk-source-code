/*
 * @(#)InformationDesc.java	1.19 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;
import java.net.URL;
import java.util.Locale;

/**
 *  Contains information about an applciation. This information
 *  might be be locale , os, and architecure specific
 */
public class InformationDesc implements XMLable {
    private String _title;
    private String _vendor;
    private URL  _home;
    private String[] _descriptions;
    private IconDesc[] _icons;
    private ShortcutDesc _shortcutHints;
    private AssociationDesc[] _associations;
    private RContentDesc[] _relatedContent; 
    private boolean _supportOfflineOperation;
    
    public InformationDesc(String title, String vendor, URL home,
			   String[] descriptions,
			   IconDesc[] icons,
			   ShortcutDesc shortcutHints, 
			   RContentDesc[] relatedContent,
			   AssociationDesc[] associations,
			   boolean supportOfflineOperation) {
        _title = title;
        _vendor = vendor;
        _home = home;
        if (descriptions == null) descriptions = new String[NOF_DESC];
        _descriptions = descriptions;
        _icons = icons;
	_shortcutHints = shortcutHints;
	_associations = associations;
	_relatedContent = relatedContent;
        _supportOfflineOperation = supportOfflineOperation;
    }
    
    /** Constants for the getInfoDescription */
    final public static int DESC_DEFAULT = 0;
    final public static int DESC_SHORT   = 1;
    final public static int DESC_ONELINE = 2;
    final public static int DESC_TOOLTIP = 3;
    final public static int NOF_DESC     = 4;
    
    /** Constants for icon sizes */
    final public static int ICON_SIZE_SMALL   = 0; /* 16 x 16 */
    final public static int ICON_SIZE_MEDIUM  = 1; /* 32 x 32 */
    final public static int ICON_SIZE_LARGE   = 2; /* 64 x 64 */
    
    /** Information */
    public String  getTitle() { return _title; }
    public String  getVendor() { return _vendor; }
    public URL     getHome() { return _home; }
    public boolean supportsOfflineOperation() { return _supportOfflineOperation; }
    public IconDesc[] getIcons() { return _icons; }
    public ShortcutDesc getShortcut() { return _shortcutHints; }
    public AssociationDesc[] getAssociations() { return _associations; }
    public RContentDesc[] getRelatedContent() { return _relatedContent; }
    
    /** Returns the description of the given kind.
     *  will return null if none there
     */
    public String  getDescription(int kind) {
        return _descriptions[kind];
    }
 
    /** Looks up a best match on icon of the given kind */
    public IconDesc getIconLocation(int size, int kind) {
        int height = 0;
        int width = 0;
        switch(size) {
	    case ICON_SIZE_SMALL: height = width = 16; break;
	    case ICON_SIZE_MEDIUM: height = width = 32; break;
	    case ICON_SIZE_LARGE: height = width = 64; break;
        }
        
        IconDesc bestID = null;
        long bestMatch = 0;
        for(int i = 0; i < _icons.length; i++) {
	    IconDesc id = _icons[i];
	    if (id.getKind() == kind) {
		// Check for exact match
		if (id.getHeight() == height && id.getWidth() == width) {
		    return id;
		}
		// Has no area information?
		if (id.getHeight() == 0 && id.getWidth() == 0) {
		    // Use as default if no-one has been found
		    if (bestID == null) {
			bestID = id;
		    }
		} else {
		    // Check how close area is?
		    long delta = Math.abs( (id.getHeight() * id.getWidth()) - (height * width));
		    if (bestMatch == 0 || delta < bestMatch) {
			bestMatch = delta;
			bestID = id;
		    }
		}
	    }
        }
        return bestID;
    }
    
    /** Creates XML structure for document */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        
        XMLNodeBuilder nb = new XMLNodeBuilder("information", ab.getAttributeList());
        nb.add("title", _title);
        nb.add("vendor", _vendor);
        nb.add(new XMLNode("homepage",
			   new XMLAttribute("href", (_home != null) ? _home.toString() : null),
			   null,
			   null));
        nb.add(getDescriptionNode(DESC_DEFAULT, ""));
        nb.add(getDescriptionNode(DESC_SHORT, "short"));
        nb.add(getDescriptionNode(DESC_ONELINE, "one-line"));
        nb.add(getDescriptionNode(DESC_TOOLTIP, "tooltip"));
	if (_icons != null) {
	    for(int i = 0; i < _icons.length; i++) {
		nb.add(_icons[i]);
	    }
	}

	if (_shortcutHints != null) {
	    nb.add(_shortcutHints);
	}

	if (_associations != null) {
	    for (int i=0; i<_associations.length; i++) {
		nb.add(_associations[i]);
	    }
	}

	if (_relatedContent != null) {
	    for (int i=0; i<_relatedContent.length; i++) {
		nb.add(_relatedContent[i]);
	    }
	}

        if (_supportOfflineOperation) {
	    nb.add(new XMLNode("offline-allowed", null));
        }
        return nb.getNode();
    }
    
    private XMLNode getDescriptionNode(int kind, String kindStr) {
        String value = _descriptions[kind];
        if (value == null) return null;
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("kind", kindStr);
        return new XMLNode("description", ab.getAttributeList(), new XMLNode(value), null);
    }
};


