/*
 * @(#)PluginTableModel.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.util.HashSet;
import com.sun.deploy.config.*;
import com.sun.deploy.resources.ResourceManager;
import java.io.File;


/**
 *
 * TableModel to be used in PluginJreDialog.
 */
public class PluginTableModel extends JreTableModel {
    HashSet _vmArgs;

    /** Creates new PluginTableModel */
    public PluginTableModel() {  
        super( new String[]{
                ResourceManager.getMessage("jpi.jres.dialog.column1"), 
                ResourceManager.getMessage("jpi.jres.dialog.column2"), 
                ResourceManager.getMessage("jpi.jres.dialog.column3"), 
                ResourceManager.getMessage("jpi.jres.dialog.column4")
        } );  
        _vmArgs = new HashSet();
        refresh();
    }  
        
    /*
     * Clear all arrays, then add all jres from JREInfo
     */
    public void refresh(){
        PluginJavaInfo [] alljres = PluginJavaInfo.getAll();
        
        _jres.clear();
        _validPaths.clear();
        _vmArgs.clear();

        for(int i = 0; i < alljres.length; i++) {
            PluginJavaInfo jre = alljres[i];
	    if (jre.getPath() != null ) {
		add(new PluginJavaInfo(jre), false, false);
		addVmArgs(jre);
	    }
        }
        fireTableDataChanged();
    }             

    /*
     * Get value from cell.
     */
    public Object getValueAt(int row, int col) {
        switch (col) {
            case 0:
                if (getJRE(row) instanceof PluginJavaInfo){
                    return ((PluginJavaInfo)getJRE(row)).getProductName();
                }else {
                    return "";
                }
            case 1:
                return getJRE(row).getProduct();
            case 2:
                return getJRE(row).getPath();
            default:
                String retVal;
                if (getJRE(row) instanceof PluginJavaInfo){
                    retVal = ((PluginJavaInfo)getJRE(row)).getVmArgs();
                }else{
                    // We should not get here, since there are only Plugin jres in this table.
                    retVal = "";
                }
                return retVal;
        }
    }
    
    /*
     *If JRE is Public and we are editing anything else but "vm args"
     * column - return "false".   Or we are editing "Product Name" column -
     * return false.  For anything else - return true.    
     **/
    public boolean isCellEditable(int rowIndex, int columnIndex) {
        if ( (getJRE(rowIndex).isSystemJRE() && columnIndex < 3)
              || columnIndex < 1 ){
            return false;
        }else{
            return true;                
        }
    }   
    
    public void add(PluginJavaInfo jre, boolean isValid, boolean notify) {
        super.add(jre, isValid, notify);
    }
        

    /*
     * Don't need to implement this method unless table's
     * data can change.
     */
    public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
        if (rowIndex >= _jres.size()) {
            // bug in JTable that this is messaged
            return;
        }
        
        switch (columnIndex) {
            case 0:
                ((PluginJavaInfo)getJRE(rowIndex)).setProductName((String) aValue);
                break;
            case 1:
                getJRE(rowIndex).setProduct((String) aValue);
                break;
            case 2:
                getJRE(rowIndex).setPath((String) aValue);
                // Force a recheck.
                _validPaths.set(rowIndex, null);
                break;
            case 3:
                ((PluginJavaInfo)getJRE(rowIndex)).setVmArgs((String) aValue);
                break;
        }

        fireTableCellUpdated(rowIndex, columnIndex);
    }  
          

    public void addVmArgs(PluginJavaInfo addme){
        _vmArgs.add(addme);
    }
    
    /*
     * Example of a valid jre path:
     * /home/mfisher/j2re1.4.1
     * To check if the path valid, add to the current path "/bin/java" and check 
     * if file exists.
     * On Windows, add "bin/java.exe" and check if file exists.
     */
    public boolean isValidJREPath(String path) {
        String java = "java";
        if ( System.getProperty("os.name").indexOf("Windows") != -1 ){
            java = java + ".exe";
        }
        
        if (path != null) {
            if (path.endsWith(File.separator)){
                path = path + "bin" + File.separator + java;
            }else{
                path = path + File.separator + "bin" + File.separator + java;
            }
            
            // Check if path exists.  
            File f = new File(path);
            return (f.exists());
        }
        return false;
    }    
}
