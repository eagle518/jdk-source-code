/*
 * @(#)DeleteFilesDialog.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.Component;
import javax.swing.BoxLayout;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JCheckBox;
import javax.swing.JOptionPane;
import com.sun.deploy.config.Config;
import java.io.File;
import java.io.IOException;


public class DeleteFilesDialog {

    public DeleteFilesDialog(Component parent) {

        /*
         * Create a panel with all the checkboxes on it.
         */
        JPanel checkBoxPanel = new JPanel();
        checkBoxPanel.setLayout(new BoxLayout(checkBoxPanel, BoxLayout.Y_AXIS));
        JLabel confirmation = new JLabel (getMessage("delete.files.dlg.temp_files"));
        
        appletsCheckBox = new JCheckBox(getMessage("delete.files.dlg.applets"));
        appletsCheckBox.setSelected(true);
        applicationCheckBox = new JCheckBox(getMessage("delete.files.dlg.applications"));
        applicationCheckBox.setSelected(true);
        tempFilesCheckBox = new JCheckBox(getMessage("delete.files.dlg.other"));
        tempFilesCheckBox.setSelected(true);

        checkBoxPanel.add(confirmation);
	checkBoxPanel.add(new JLabel("   "));
        checkBoxPanel.add(appletsCheckBox);
        checkBoxPanel.add(applicationCheckBox);
        checkBoxPanel.add(tempFilesCheckBox);
        
        /*
         * Show confirmation dialog with checkboxes.
         */
        int userResponse = JOptionPane.showConfirmDialog(parent, 
                                                         checkBoxPanel, 
                                                         getMessage("delete.files.dlg.title"),
                                                         JOptionPane.OK_CANCEL_OPTION);
        if ( userResponse == JOptionPane.OK_OPTION){
            // Delete all files
            if (appletsCheckBox.isSelected()){
                // Delete all files in plug-in cache
		File pluginCacheDir = new File(Config.getPluginCacheDir());
		deleteFiles(pluginCacheDir);
            }
            if (applicationCheckBox.isSelected()){
                // Delete all files in web start cache
                String [] cmd = new String[2];
		cmd[0] = Config.getJavawsCommand();
		cmd[1] = "-uninstall";
		try {
		    Runtime.getRuntime().exec(cmd);
		} catch (IOException ioe) {
	    	    //Trace.ignoredException(ioe);
		}
            }
            if (tempFilesCheckBox.isSelected()){
                // Delete all log and trace files.
		File dir = new File(Config.getLogDirectory());
		deleteFiles(dir);

		// now need to delete ext ...
		dir = new File(Config.getUserExtensionDirectory());
		deleteFiles(dir);

		// now need to delete cache/tmp ...
		dir = new File(Config.getTempCacheDir());
		deleteFiles(dir);
            }
        }
    }

    private void deleteFilesWithExtension(File dir, String extension) {
	String end = "." + extension;
	File[] children = dir.listFiles(); 
	for (int i=0; i<children.length; i++) {
	    if (children[i].isDirectory()) {
		deleteFilesWithExtension(children[i], extension);
		children[i].delete(); 
	    } else if (children[i].getPath().endsWith(end)) {
		children[i].delete();
	    }
	}
    }

    private void deleteFiles(File dir)
    {
	if (dir.exists() && dir.isDirectory()) {
	    File[] children = dir.listFiles();
	    for (int i = 0; i < children.length; i++) {
	    	if (children[i].isDirectory()) {
		    deleteFiles(children[i]);
	        }
	        children[i].delete();
	    }
	}
    }  
    
    private String getMessage(String id)
    {
	return com.sun.deploy.resources.ResourceManager.getMessage(id);
    }    

    private JCheckBox appletsCheckBox, applicationCheckBox, tempFilesCheckBox;
}
