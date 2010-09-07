/*
 * @(#)JreTableModel.java	1.8 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.panel;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.HashSet;
import javax.swing.table.AbstractTableModel;
import javax.swing.event.*;

import com.sun.deploy.config.JREInfo;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.NativePlatform;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 * Table model for jre entries in the java control panel
 */
public class JreTableModel extends AbstractTableModel {
    
    private ArrayList _jres;
    private ArrayList _validPaths;
    private String[] _columnNames;
    private boolean _system;
    private HashSet _hidden;

    private boolean dirty = false;
    
    /** Creates new JreTableModel */
    public JreTableModel(boolean bSystem) {
        this( new String[]{
                    ResourceManager.getMessage(
			"controlpanel.jre.platformTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.productTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.locationTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.pathTableColumnTitle"),
		    ResourceManager.getMessage(
	        	"controlpanel.jre.vmargsTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.enabledTableColumnTitle")
                }, bSystem );
    }

    private JreTableModel(String[] cols, boolean bSystem) {
        _jres = new ArrayList();
        _validPaths = new ArrayList();
        _columnNames = cols;
        _system = bSystem;
        _hidden = new HashSet(); 
        refresh();

	if (bSystem == false) {
	    addTableModelListener(new TableModelListener() {
		    public void tableChanged(TableModelEvent e) {
			dirty = true;
		    }
		});
	}
    }

    private void refresh() {
	// JREInfo is updated when ControlPanel is constructed
	// We can call Config.refreshProps() here to get last
	// minute changes each time jre dialog is opened.
	Config.refreshProps();
        JREInfo [] alljres = JREInfo.getAll();
        
	_jres.clear();
        _validPaths.clear();
        _hidden.clear();
        
        Trace.println("refresh for "+ (_system? "system":"user")+" JREs", TraceLevel.BASIC);
        for(int i = 0; i < alljres.length; i++ ) {
            JREInfo jre = alljres[i];
	    if (jre.isSystemJRE() != _system) {
		continue;
	    }
	    
            if (jre.getOSName() == null || // JREInfo always sets it to Config.getOSName
		jre.getOSArch() == null || // JREInfo always sets it to Config.getOSArch
		jre.isOsInfoMatch()) {	
		if (jre.getPath() != null && JREInfo.isValidJREPath(jre.getPath())) { 
		    // FIXME: the above check may be duplicate since JREInfo has already checked it
		    add(new JREInfo(jre), true, false);
		} else {
		    // FIXME: unlikely since JREInfo validated path
		    _hidden.add(new JREInfo(jre));
		}
	    } else {
		// hide entries for other native platforms. e.g. x86 jre while 
		// on a sparc machine.
		_hidden.add(new JREInfo(jre));
	    }
	}
        fireTableDataChanged();
    }
    
    HashSet getHiddenJREs(){
        return _hidden;
    }
    
    public Object getValueAt(int row, int column) {
        switch (column) {
	case 0:
	    return getJRE(row).getPlatform();
	case 1:
	    return getJRE(row).getProduct();
	case 2:
	    return getJRE(row).getLocation();
	case 3:
	    return getJRE(row).getPath();
	case 4:
	    return getJRE(row).getVmArgs();
	default:
	    return new Boolean(getJRE(row).isEnabled());
        }
    }    
    
    public boolean isCellEditable(int rowIndex, int columnIndex) {
        // can't edit system ones at all
        return (!getJRE(rowIndex).isSystemJRE());
    }    
    
    public Class getColumnClass(int c) {
        if (c < 5) return String.class;
        else return Boolean.class;
    }    

    void add(JREInfo jre, boolean isValid, boolean notify) {
	if(jre.isSystemJRE() == _system) {
            if (Trace.isTraceLevelEnabled(TraceLevel.TEMP)) {
	        Trace.println("Table model adding jre: "+jre, TraceLevel.TEMP);
            }
	    _jres.add(jre);
	    if (isValid) {
		_validPaths.add(Boolean.TRUE);
	    } else {
		_validPaths.add(null);
	    }
	    if (notify) {
		fireTableRowsInserted(_jres.size() - 1, _jres.size() - 1);
	    }
	}
    }    
    
    public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
        if (rowIndex >= _jres.size()) {
            // bug in JTable that this is messaged
            return;
        }
            
        JREInfo jre = getJRE(rowIndex);
            
        switch (columnIndex) {
            case 0:

		// FIXME: set platform null or empty string will
		// remove this entry from JREInfo
		String platform = (String)aValue;
		if (platform != null && !platform.equals("")) {
		    _jres.set(rowIndex, new JREInfo( 
				  (String)aValue,
				  jre.getProduct(),
				  jre.getLocation(),
				  jre.getPath(),
				  jre.getVmArgs(),
				  jre.getOSName(), 
				  jre.getOSArch(),
				  jre.isEnabled(),
				  jre.isRegistered()));
		}
                break;
            case 1:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          (String)aValue,
                          jre.getLocation(),
                          jre.getPath(),
                          jre.getVmArgs(),
                          jre.getOSName(), 
                          jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
                break;
            case 2:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          jre.getProduct(),
                          (String)aValue,
                          jre.getPath(),
                          jre.getVmArgs(),
                          jre.getOSName(), 
                          jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
                break;
            case 3:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          jre.getProduct(),
                          jre.getLocation(),
                          (String)aValue,
                          jre.getVmArgs(),
                          jre.getOSName(), 
                          jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
                // Force a recheck.
                _validPaths.set(rowIndex, null);
                break;
	    case 4:
		_jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          jre.getProduct(),
                          jre.getLocation(),
                          jre.getPath(),
                          (String)aValue,
                          jre.getOSName(), 
                          jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
		break;
            default:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          jre.getProduct(),
                          jre.getLocation(),
                          jre.getPath(),
                          jre.getVmArgs(),
                          jre.getOSName(), 
                          jre.getOSArch(),
                          ((Boolean)aValue).booleanValue(),
                          jre.isRegistered()));
                break;
        }
        fireTableRowsUpdated(rowIndex, rowIndex);
    }
    
    /*
     * Get number of columns.
     */
    public int getColumnCount() {
        return _columnNames.length;
    }    
    
    /*
     * Get number of rows.
     */
    public int getRowCount() {
        return _jres.size();
    }  
    
    /*
     * return JREInfo at specified index.
     */
    JREInfo getJRE(int index) {
        return (JREInfo)_jres.get(index);
    }
    
    /*
     * Return column name at specified index.
     */
    public String getColumnName(int column) {
        return _columnNames[column];
    }  

    // This is to validate and save JRE table to JREInfo
    void validateAndSave() {
	// only user table need validation
	if (_system == false && dirty) {
	    if (getRowCount() > 0 ) {
		JREInfo.clear();
		for (int n = 0; n < getRowCount(); n++) { 
		    if (isPathValid(n)) {
			JREInfo.addJRE(validatePlatform(n));
		    }
		}

		// add back any hidden jres 
		for (Iterator n = getHiddenJREs().iterator(); n.hasNext(); ) {
		    JREInfo jre = (JREInfo)n.next();
		    JREInfo.addJRE(jre);
		}	
	    }
	} else if (_system == true) {
	    if (getRowCount() > 0 ) {
		for (int n = 0; n < getRowCount(); n++) { 
		    JREInfo.addJRE(getJRE(n));
		}
	    }
	    
	    // add back any hidden jres 
	    for (Iterator n = getHiddenJREs().iterator(); n.hasNext(); ) {
		JREInfo jre = (JREInfo)n.next();
		JREInfo.addJRE(jre);
	    }	    
	}   
    }
    
    /*
     * Check if path for jre in a <row> is valid.
     */
    boolean isPathValid(int row) {
        Boolean b = (Boolean)_validPaths.get(row);
	
        if (b == null) {
            // Haven't checked it yet, check it out.
            if (JREInfo.isValidJREPath(getJRE(row).getPath())) {
                b = Boolean.TRUE;
            }
            else {
                b = Boolean.FALSE;
            }
            _validPaths.set(row, b);
        }
        return Boolean.TRUE.equals(b);
    }  

    /**
     * Helper method to add platform and/or product information to a
     * JREInfo if they are missing.
     */
    private JREInfo validatePlatform(int row) {
	// jre's platform need be non-null and >= 1.3 in order to be added 
	// to JREInfo's _jres

	JREInfo jre = (JREInfo) getJRE(row);
	if (jre.getPlatform() == null) {
	    String platform = JREInfo.getPlatformByProduct(jre.getProduct());
	    if (platform != null) {
		jre.setPlatform(platform);
	    } else {
		// either product is null or bad product 
		// try to get them from the path
		JREInfo jreinfo = null;
		try {
		    jreinfo = JreLocator.getVersion(new File(jre.getPath()));
		} catch (Exception e) {
		    Trace.ignored(e);
		}
		if (jreinfo != null) {
		    jre.setProduct(jreinfo.getProduct());
		    jre.setPlatform(jreinfo.getPlatform());
		}
	    }
	}
	return jre;
    } 
    
    void remove(int[] rows) {
        if (rows != null) {
            int count = getRowCount();
            for (int counter = rows.length - 1; counter >= 0;
                 counter--) {
                if (rows[counter] != -1 && rows[counter] < count) {
                    _jres.remove(rows[counter]);
                    _validPaths.remove(rows[counter]);
                }
            }
        }
        fireTableDataChanged();
    }  

    boolean isSystem() {
	return _system;
    }

}
