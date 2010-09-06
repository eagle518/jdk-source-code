/*
 * @(#)JreTableModel.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.panel;

import java.util.ArrayList;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.config.Config;
import javax.swing.table.AbstractTableModel;
import java.io.File;
import com.sun.deploy.util.Trace;

/**
 *
 * @author  mfisher
 * @version 
 */
public abstract class JreTableModel extends AbstractTableModel {
    
    ArrayList _jres;
    ArrayList _validPaths;
    String[] _columnNames;

    /** Creates new JreTableModel */
    public JreTableModel(String[] columnNames) {
        _jres = new ArrayList();
        _validPaths = new ArrayList();
        _columnNames = columnNames;
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
    public JREInfo getJRE(int index) {
        return (JREInfo)_jres.get(index);
    }
    
    /*
     * Return column name at specified index.
     */
    public String getColumnName(int column) {
        return _columnNames[column];
    }   
    
    
    /*
     * Check if path for jre in a <row> is valid.
     */
    public boolean isPathValid(int row) {
        Boolean b = (Boolean)_validPaths.get(row);

        if (b == null) {
            // Haven't checked it yet, check it out.
            if (isValidJREPath(getJRE(row).getPath())) {
                b = Boolean.TRUE;
            }
            else {
                b = Boolean.FALSE;
            }
            _validPaths.set(row, b);
        }
        return Boolean.TRUE.equals(b);
    }   
    
    public abstract boolean isValidJREPath(String path);
    

    public void add(JREInfo jre, boolean isValid, boolean notify) {
        Trace.println("adding jre: "+jre);
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
    
    
    public void remove(int[] rows) {
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

}
