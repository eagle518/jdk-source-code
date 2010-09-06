/*
 * @(#)JavawsTableModel.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.HashSet;
import java.io.File;
import java.lang.Runtime;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.*;
import com.sun.deploy.util.Trace;


/**
 *
 * @author  mfisher
 * @version 
 */
public class JavawsTableModel extends JreTableModel {
    private boolean _system;
    private HashSet _hidden;
    
    /** Creates new JavawsTableModel */
    public JavawsTableModel(boolean bSystem) {
        super( new String[]{
                    ResourceManager.getMessage(
			"controlpanel.jre.platformTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.productTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.locationTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.pathTableColumnTitle"),
                    ResourceManager.getMessage(
			"controlpanel.jre.enabledTableColumnTitle")
                } );
                
        _system = bSystem;
        
        _hidden = new HashSet(); 
        refresh();
    }

    public void refresh() {
	String [] cmds = new String [2];
	cmds[0] = Config.getJavawsCommand();
	cmds[1] = "-quick";
	try {
	    Runtime.getRuntime().exec(cmds).waitFor();
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	if (Config.isDiskNewer()) {
	    Config.refreshUnchangedProps();
	}
        JREInfo [] alljres = JREInfo.get();
        
	_jres.clear();
        _validPaths.clear();
        _hidden.clear();
        
        Trace.println("refresh for system = "+_system);
        for( int i = 0; i < alljres.length; i++ ) {
            JREInfo jre = alljres[i];
            
            if (jre.getOSName() == null ||
                jre.getOSArch() == null ||
                (jre.getOSName().equals(Config.getOSName()) &&
                jre.getOSArch().equals(Config.getOSArch()))) {	
		if (jre.getPath() != null && isValidJREPath(jre.getPath())) {
		    add(new JREInfo(jre), false, false);
		} else {
		    _hidden.add(new JREInfo(jre));
		}
            } else {
		_hidden.add(new JREInfo(jre));
            }
        }
        fireTableDataChanged();
    }
    
    public HashSet getHiddenJREs(){
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
            default:
                return new Boolean(getJRE(row).isEnabled());
        }
    }    
    
    public boolean isCellEditable(int rowIndex, int columnIndex) {
        // can't edit system ones at all
        return (!getJRE(rowIndex).isSystemJRE());
    }    
    
    public Class getColumnClass(int c) {
        if (c < 4) return String.class;
        else return Boolean.class;
    }    
    
    public void add(JREInfo jre, boolean isValid, boolean notify) {
	if(jre.isSystemJRE() == _system) {
            super.add(jre, isValid, notify);
        }
    }
    
    
    public boolean isValidJREPath(String path) {
        if (path != null) {
            File f = new File(path);
            return (f.exists() && !f.isDirectory());
        }
        return false;
    }            
    
    public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
        if (rowIndex >= _jres.size()) {
            // bug in JTable that this is messaged
            return;
        }
            
        JREInfo jre = getJRE(rowIndex);
            
        switch (columnIndex) {
            case 0:
                _jres.set(rowIndex, new JREInfo( 
                	  (String)aValue,
                          jre.getProduct(),
                          jre.getLocation(),
                          jre.getPath(),
                          jre.getOSName(), 
			  jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
                break;
            case 1:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
			  (String)aValue,
                          jre.getLocation(),
                          jre.getPath(),
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
			  jre.getOSName(), 
			  jre.getOSArch(),
                          jre.isEnabled(),
                          jre.isRegistered()));
                // Force a recheck.
                _validPaths.set(rowIndex, null);
                break;
            default:
                _jres.set(rowIndex, new JREInfo(
                          jre.getPlatform(),
                          jre.getProduct(),
                          jre.getLocation(),
                          jre.getPath(),
			  jre.getOSName(), 
			  jre.getOSArch(),
                          ((Boolean)aValue).booleanValue(),
                          jre.isRegistered()));
                break;
        }
        fireTableRowsUpdated(rowIndex, rowIndex);
    }
}
