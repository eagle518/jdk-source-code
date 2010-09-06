/*
 * @(#)JarCacheTableColumnModel.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.util.*;
import javax.swing.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;

public class JarCacheTableColumnModel extends DefaultTableColumnModel {

    public final static boolean VISIBLE = true;
    public final static boolean HIDDEN = false;
    public final static boolean LEFT_ALIGNED = true;
    public final static boolean RIGHT_ALIGNED = false;
    public final static boolean ASCENDING = true;
    public final static boolean DESCENDING = false;

    private JarCacheTableColumn nameColumn      = 
	new JarCacheTableColumn(JarCacheEntry.NAME, JarCacheEntry.NAME_HELP, true);
    private JarCacheTableColumn typeColumn      = 
	new JarCacheTableColumn(JarCacheEntry.TYPE, JarCacheEntry.TYPE_HELP, true);
    private JarCacheTableColumn sizeColumn      = 
	new JarCacheTableColumn(JarCacheEntry.SIZE, JarCacheEntry.SIZE_HELP, true);
    private JarCacheTableColumn expiryColumn    = 
	new JarCacheTableColumn(JarCacheEntry.EXPIRY_DATE, JarCacheEntry.EXPIRY_DATE_HELP, true);
    private JarCacheTableColumn modifyColumn    = 
	new JarCacheTableColumn(JarCacheEntry.MODIFY_DATE, JarCacheEntry.MODIFY_DATE_HELP, true);
    private JarCacheTableColumn versionColumn   = 
	new JarCacheTableColumn(JarCacheEntry.VERSION, JarCacheEntry.VERSION_HELP, true);
    private JarCacheTableColumn urlColumn       = 
	new JarCacheTableColumn(JarCacheEntry.URL, JarCacheEntry.URL_HELP, false);

    private JarCacheTableColumn [] tableColumns =   {   
	nameColumn,
	typeColumn,
	sizeColumn,
	expiryColumn,
	modifyColumn,
	versionColumn,
	urlColumn
    };

    public JarCacheTableColumnModel() {
	for (int i=0; i<tableColumns.length; i++ ) {
	    addColumn(tableColumns[i]);
	}
    }

    public JarCacheTableColumn [] getColumnArray() {
	return tableColumns;
    }

    public String [] getColumnNames() {
	String [] names = new String[tableColumns.length];
	for(int i=0; i<names.length; i++) {
	    names[i] = tableColumns[i].getHeaderValue().toString();
	}
	return names;
    }
}



