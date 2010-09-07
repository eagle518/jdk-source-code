/*
 * @(#)InformationDesc.java	1.29 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;
import java.net.URL;
import java.util.Locale;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.AssociationDesc;


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
    
    /** Information */
    public String  getTitle() { return _title; }
    public String  getVendor() { return _vendor; }
    public URL     getHome() { return _home; }
    public boolean supportsOfflineOperation() { return _supportOfflineOperation; }
    public IconDesc[] getIcons() { return _icons; }
    public ShortcutDesc getShortcut() { return _shortcutHints; }
    public AssociationDesc[] getAssociations() { return _associations; }

    /**
     * Sets new shortcut hints. This is used in IntegrationService API (programmatic
     * shortcut management).
     *
     * @param shortcutDesc the new shortcut hints to set
     */
    public void setShortcut(ShortcutDesc shortcut) {
        _shortcutHints = shortcut;
    }

    /**
     * Sets new associations. This is used in IntegrationService API
     * (programmatic association management). It only takes on association
     * as argument, because this API can only create one association per
     * call. It replaces all existing associations that might have already been
     * set.
     *
     * @param assoc the association to set
     */
    public void setAssociation(AssociationDesc assoc) {
        _associations = new AssociationDesc[] { assoc };
    }

    public RContentDesc[] getRelatedContent() { return _relatedContent; }
    
    /** Returns the description of the given kind.
     *  will return null if none there
     */
    public String  getDescription(int kind) {
        return _descriptions[kind];
    }
 
    /** Looks up a best match on icon of the given size and kind */
    public IconDesc getIconLocation(int size, int kind) {

        IconDesc bestID = null;
        long bestMatch = 0;

        for(int i = 0; i < _icons.length; i++) {
            IconDesc id = _icons[i];

            // any file extension can be used as a shortcut, 
            // but for other uses the file can't be a ".ico" extension
            boolean typeOK = (kind == IconDesc.ICON_KIND_SHORTCUT) ||
                !(id.getSuffix().equalsIgnoreCase(".ico"));

            if ((id.getKind() == kind) && typeOK) {
                // Check for exact match
                if (id.getHeight() == size && id.getWidth() == size) {
                    return id;
                }
                // Has no area information?
                if (id.getHeight() == 0 && id.getWidth() == 0) {
                    // Use as default if no-one has been found
                    if (bestID == null) {
                        bestID = id;
                    }
                } else {
                    // Check how close it is.
                    // do not use area (overly favors smaller one)
                    // instead use sum of diffs
                    int diff = id.getHeight() + id.getWidth() 
                        - (2 * size);

                    long delta = Math.abs(diff);
                 
                    if (bestMatch == 0 || delta < bestMatch) {
                        bestMatch = delta;
                        bestID = id;
                    } else if (delta == bestMatch && diff > 0) {
                        // also prefer larger when diff is same
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

}


