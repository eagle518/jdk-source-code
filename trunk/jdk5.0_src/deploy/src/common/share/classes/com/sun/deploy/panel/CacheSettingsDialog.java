/*
 * @(#)CacheSettingsDialog.java	1.20 04/03/30
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.JComponent;
import javax.swing.KeyStroke;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.JComboBox;
import javax.swing.JSlider;
import javax.swing.JLabel;
import javax.swing.JFileChooser;
import javax.swing.ButtonGroup;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.filechooser.FileFilter;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.PlainDocument;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;

import java.awt.Toolkit;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Frame;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Dimension;
import java.awt.event.WindowEvent;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.KeyEvent;

import java.io.File;
import java.io.IOException;

import java.text.MessageFormat;
import java.text.NumberFormat;

import java.util.Hashtable;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;


public class CacheSettingsDialog extends JDialog {
    // Strings to use in launch command for plugin cache viewer.
    private String LIB = "lib";
    private String PLUGIN_JAR = "plugin.jar";
    private String DEPLOY_JAR = "deploy.jar";
    private String VIEWER = "sun.plugin.cache.JarCacheViewer";  
    private String CLASSPATH = "-classpath";

    /** Creates new form CacheSettings */
    public CacheSettingsDialog(Frame parent, boolean modal) {
        super(parent, modal);
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * 
     */
    private void initComponents() {
        setTitle(getMessage("cache.settings.dialog.title"));

        // Add checkbox to enable and disable caching
        JPanel buttonsPanel = new JPanel();
        buttonsPanel.setLayout(new BoxLayout(buttonsPanel, BoxLayout.X_AXIS));
        buttonsPanel.setBorder(BorderFactory.createEmptyBorder(6, 6, 6, 6));

        deleteFilesBtn = makeButton("cache.settings.dialog.delete_btn");
        deleteFilesBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                deleteFilesBtnActionPerformed(evt);
            }
        });
        deleteFilesBtn.setToolTipText(getMessage("temp.files.delete.btn.tooltip"));                
        viewJwsCacheBtn = makeButton("cache.settings.dialog.view_jws_btn");
        viewJwsCacheBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                viewJwsCacheBtnActionPerformed(evt);
            }
        });
        viewJwsCacheBtn.setToolTipText(getMessage("cache.settings.dialog.view_jws_btn.tooltip"));
        
        viewJpiCacheBtn = makeButton("cache.settings.dialog.view_jpi_btn");
        viewJpiCacheBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                viewJpiCacheBtnActionPerformed(evt);
            }
        });
        viewJpiCacheBtn.setToolTipText(getMessage("cache.settings.dialog.view_jpi_btn.tooltip"));

        buttonsPanel.add(deleteFilesBtn);
        buttonsPanel.add(Box.createHorizontalGlue());
        buttonsPanel.add(viewJwsCacheBtn);
        buttonsPanel.add(Box.createHorizontalGlue());
        buttonsPanel.add(viewJpiCacheBtn);
        this.getContentPane().add(buttonsPanel, BorderLayout.NORTH);


        // Create "Settings" panel and border
        JPanel settingsPanel = new JPanel();
        GridBagLayout gridBag = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        settingsPanel.setLayout(gridBag);
        Border border = BorderFactory.createEtchedBorder();
        settingsPanel.setBorder(BorderFactory.createCompoundBorder(
                                BorderFactory.createEmptyBorder(0, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, 
                                                                 getMessage("common.settings"),
                                                                 TitledBorder.DEFAULT_JUSTIFICATION, 
                                                                 TitledBorder.DEFAULT_POSITION)));

        // Add a field for setting the location of the cache
        JLabel locationLabel = new JLabel(getMessage("cache.settings.dialog.cache_location"));
        c.anchor = GridBagConstraints.WEST;
        c.weighty = 3;
        c.fill = GridBagConstraints.NONE;
        c.insets = new Insets(5, 10, 5, 10);
        gridBag.setConstraints(locationLabel, c);
        settingsPanel.add(locationLabel);
        location = new LocationField();
        location.setDocument(new LocationDocument());
        location.setEditable(false);
        
        c.weightx = 1;
        c.gridwidth = 3;
        c.insets = new Insets(5, 0, 5, 2);
        c.fill = GridBagConstraints.HORIZONTAL;
        gridBag.setConstraints(location, c);
        settingsPanel.add(location);
        choose = makeButton("cache.settings.dialog.change_btn");
        choose.setMargin(new Insets(0, 10, 0, 10));
        choose.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                chooseButtonActionPerformed(evt);
            }
        });
        choose.setToolTipText(getMessage("cache.settings.dialog.change_btn.tooltip"));
                
        c.fill = GridBagConstraints.NONE;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = new Insets(5, 2, 5, 10);
        c.weightx = 0;
        gridBag.setConstraints(choose, c);
        settingsPanel.add(choose);

        // Add radio buttons for cache size
        JSmartTextArea text = new JSmartTextArea(getMessage("cache.settings.dialog.disk_space"));        
	text.setRows(2);  // allow this to grow down not out
        c.weightx = 0;
        c.weighty = 0;
        c.gridwidth = 1;
	c.gridheight = 2;
        c.insets = new Insets(5, 5, 10, 10);
        gridBag.setConstraints(text, c);
        settingsPanel.add(text);
	c.gridheight = 1;
        
        /*
         * Unlimited/Maximum radio button group.
         */
        ButtonGroup group = new ButtonGroup();
        unlimited = new JRadioButton(getMessage("cache.settings.dialog.unlimited_btn"));
        unlimited.setToolTipText(getMessage("cache.settings.dialog.unlimited_btn.tooltip"));
        unlimited.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                radioButtonActionPerformed(evt);
            }
        });
                
        unlimited.setMargin(new Insets(0, 0, 0, 0));
        c.weightx = 1;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = new Insets(5, 0, 0, 0);
        gridBag.setConstraints(unlimited, c);
        group.add(unlimited);
        settingsPanel.add(unlimited);
        
        maximum = new JRadioButton(getMessage("cache.settings.dialog.max_btn"));
        maximum.setToolTipText(getMessage("cache.settings.dialog.max_btn.tooltip"));
        maximum.setMargin(new Insets(0, 0, 0, 0));
        maximum.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                radioButtonActionPerformed(evt);
            }
        });
                
        c.gridwidth = 1;
        c.gridx = 1;
        c.weightx = 0;
        c.insets = new Insets(0, 0, 10, 0);
        gridBag.setConstraints(maximum, c);
        group.add(maximum);
        settingsPanel.add(maximum);

        // Add a field for setting the size of the cache        
        size = new JTextField(7);
                
        size.setDocument(new SizeDocument());
        c.gridx = GridBagConstraints.RELATIVE;
        gridBag.setConstraints(size, c);
        settingsPanel.add(size);
        String[] elements = {"MB",
                             "KB",
                             "bytes"};

        units = new JComboBox(elements);
        c.gridwidth = 1;
        c.insets = new Insets(0, 2, 10, 10);
        gridBag.setConstraints(units, c);
        settingsPanel.add(units);

        // Add a slider for setting the compression level
        JLabel label = new JLabel(getMessage("cache.settings.dialog.compression"));
        label.setToolTipText(getMessage("cache.settings.dialog.compression.tooltip"));
        c.anchor = GridBagConstraints.NORTHWEST;
        c.gridy = 3;
        c.weightx = 0;
        c.weighty = 1;
        c.gridwidth = 1;
        c.insets = new Insets(5, 10, 5, 10);
        gridBag.setConstraints(label, c);
        settingsPanel.add(label);
        compression = new JSlider(0, 9, 0);
        
        Hashtable labels = new Hashtable(2);
        JLabel noneLabel = new JLabel(getMessage("cache.settings.dialog.none"));
        JLabel highLabel = new JLabel(getMessage("cache.settings.dialog.high"));
        labels.put(new Integer(0),
                   noneLabel);
        labels.put(new Integer(9),
                   highLabel);
        compression.setLabelTable(labels);
        compression.setPaintLabels(true);
        compression.setMajorTickSpacing(1);
        compression.setPaintTicks(true);
        compression.setSnapToTicks(true);
        c.weightx = 1;
        c.gridheight = 2;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(5, 0, 5, 10);
        gridBag.setConstraints(compression, c);
        settingsPanel.add(compression);

        getContentPane().add(settingsPanel, BorderLayout.CENTER);


        // assemble decision panel
        okBtn = makeButton("common.ok_btn");
        okBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okBtnActionPerformed(evt);
            }
        });

	AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelBtnActionPerformed(evt);
            }
        };
        cancelBtn = makeButton("common.cancel_btn");
        cancelBtn.addActionListener(cancelAction);
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE),"cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        JPanel decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        decisionPanel.add(okBtn);
        decisionPanel.add(cancelBtn);
        
        getContentPane().add(decisionPanel, BorderLayout.SOUTH);
        
        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(okBtn);

        pack();
        setResizable(false);
        
        initValues();
    }
    
    
    /*
     * Set up the values: cache location, size, compression, etc.
     */
    public void initValues(){
        // Get cache directory location.
        location.setText(Config.getCacheDirectory());
        location.setToolTipText(location.getText());
	location.setHorizontalAlignment(JTextField.LEFT);
	boolean enab = !Config.isLocked(Config.CACHEDIR_KEY);
	location.setEnabled(enab);
	location.setEditable(enab);
	choose.setEnabled(enab);
        
        // Set cache size.
        long val;
        String cacheSize = (Config.getProperty(Config.CACHE_MAX_KEY)).trim();
	enab = !Config.isLocked(Config.CACHE_MAX_KEY);
        try {
            int unitIndex = 0;
            if (cacheSize.endsWith("M") || cacheSize.endsWith("m")) {
                // Size is in MegaBytes
                cacheSize = cacheSize.substring(0, cacheSize.length() - 1);
                unitIndex = 0;
            } else if (cacheSize.endsWith("K") || cacheSize.endsWith("k")) {
                // Size is in KiloBytes
                cacheSize = cacheSize.substring(0, cacheSize.length() - 1);
                unitIndex = 1;
            } else {
                // Size is in bytes
                unitIndex = 2;
            }
            val = Long.valueOf(cacheSize).longValue();
            if (val == -1) {
                unlimited.setSelected(true);
                size.setEnabled(false);
                size.setEditable(false);
                units.setEnabled(false); 
            } else {
                maximum.setSelected(true);
                size.setText(cacheSize);
                units.setSelectedIndex(unitIndex);
            }
        } catch (NumberFormatException e) {
            val = 50;
            maximum.setSelected(true);
            units.setSelectedIndex(0);
        }   
        
        /*
         * Some of the components might be disabled at this point.  If
         * CACHE_MAX_KEY is not locked, and enab=true; we shouldn't re-enable
         * components that should be diabled independently of CACHE_MAX_KEY.
         */
        if ( !enab ){
            unlimited.setEnabled(enab);
            maximum.setEnabled(enab);
            size.setEnabled(enab);
            units.setEnabled(enab);
        }
        
	enab = !Config.isLocked(Config.CACHE_COMPRESSION_KEY);
        compression.setValue(Config.getIntProperty(Config.CACHE_COMPRESSION_KEY));
	compression.setEnabled(enab);

    }
    
    private void viewJpiCacheBtnActionPerformed(ActionEvent evt) {
        //  Assembling the following command:
        // "{full_path_to_jre}/bin/java" -classpath 
        // "{full_path_to_jre}/lib/plugin.jar":"{full_path_to_jre}/lib/deploy.jar"
        // sun.plugin.cache.JarCacheViewer
        // Note: quotes are needed for paths with spaces.  Like "Program Files".
        String java_home = System.getProperty("java.home");

        String profileString=System.getProperty("javaplugin.user.profile");
        if (profileString == null) { profileString=""; }
        
        if (!java_home.endsWith(File.separator)) {
            java_home = java_home + File.separator;
        }
        
        String [] cmds = new String[5];
        cmds[0] = Config.getInstance().toExecArg(Config.getJavaCommand());
        cmds[1] = CLASSPATH;
        cmds[2] = Config.getInstance().toExecArg(java_home + LIB + File.separator + PLUGIN_JAR +
                        File.pathSeparator + java_home + LIB + File.separator + DEPLOY_JAR);
	cmds[3] = Config.getInstance().toExecArg("-Djavaplugin.user.profile="+profileString);
        cmds[4] = VIEWER;
        Trace.println("Launching plug-in Cache Viewer: "+cmds[0]+" "+cmds[1]+" "+
                          cmds[2]+" "+cmds[3]+" "+cmds[4]);
        try {
            Runtime.getRuntime().exec(cmds);
        } catch (java.io.IOException ioe) {
            Trace.ignoredException(ioe);
        }

    }

    private void viewJwsCacheBtnActionPerformed(ActionEvent evt) {
	String [] cmds = new String[3];
	cmds[0] = Config.getJavawsCommand();
	cmds[1] = "-Xnosplash";
	cmds[2] = "-viewer";
	Trace.println( "Launching javaws viewer: "+cmds[0]+" "+cmds[1], 
			TraceLevel.BASIC);
        try {
            Runtime.getRuntime().exec(cmds);
        } catch (java.io.IOException ioe) {
            Trace.ignoredException(ioe);
        }
    }
    
    private void chooseButtonActionPerformed(ActionEvent evt) {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        chooser.setDialogTitle(getMessage("cache.settings.dialog.chooser_title"));
        chooser.setApproveButtonText(getMessage("cache.settings.dialog.select"));
        String tooltip = getMessage("cache.settings.dialog.select_tooltip");
        chooser.setApproveButtonToolTipText(tooltip);
        char mnemonic = getMessage("cache.settings.dialog.select_mnemonic").charAt(0);
        chooser.setApproveButtonMnemonic(mnemonic);
        File dir = new File(location.getText());
        chooser.setCurrentDirectory(dir);
        if (chooser.showDialog(this, null) == JFileChooser.APPROVE_OPTION) {
            String file;
            try {
                file = chooser.getSelectedFile().getCanonicalPath();
            } catch (IOException ioe) {
                file = chooser.getSelectedFile().getPath();
            }
            location.setText(file);
            location.setToolTipText(file);
        }      
    }    

    private void deleteFilesBtnActionPerformed(ActionEvent evt) {
        new DeleteFilesDialog(this);
    }
    
    /*
     * One of the radio buttons was clicked on.
     */
    private void radioButtonActionPerformed(ActionEvent evt){
        
        if (evt.getSource().equals(unlimited)){
            //disable the textbox and drop down list for "Maximum"
            size.setEnabled(false);
            size.setEditable(false);
            units.setEnabled(false);
        }
        else{
            // enable the textbox and drop down list for "Maximum"
            size.setEditable(true);
            size.setEnabled(true);
            units.setEnabled(true);
        }
    }

    private void okBtnActionPerformed(ActionEvent evt) {
        // Save values set - pass them to the Config;
        // Close this dialog
        /* Config.set(....); */
        Config.setCacheDirectory(location.getText());
        updateSize();
        Config.setIntProperty(Config.CACHE_COMPRESSION_KEY, compression.getValue());
        setVisible(false);
        dispose();
    }

    private void cancelBtnActionPerformed(ActionEvent evt) {
        // Clear all values to the last saved state and close dialog.
        setVisible(false);
        dispose();
    }
    
    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
        setVisible(false);
        dispose();
    }

    private void updateSize() {
        
        String str;
        if (unlimited.isSelected()){
            str = "-1";
        }else{
            // Get the size from the size field
            str = size.getText().trim();

            // If the size field is empty, set the size to zero
            if (str.equals("")) {
                str = "0";
            }

            // Make sure the size is in ASCII
            long val = Long.valueOf(str).longValue();
            str = Long.toString(val);

            // Get the units
            int index = units.getSelectedIndex();
            String[] list = {"m", "k", ""};
            str += list[index];
        }
        
        // Set the size in the model
        Config.setProperty(Config.CACHE_MAX_KEY, str);
    }
    
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    } 
    
    public JButton makeButton(String key) {
	JButton b = new JButton(getMessage(key));
	b.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
	return b;
    }    

    private JTextField location, size;
    private JButton choose, okBtn, cancelBtn, viewJwsCacheBtn, deleteFilesBtn, viewJpiCacheBtn;
    private JRadioButton unlimited, maximum;
    private JComboBox units;
    private JSlider compression;
    
    
    private class SizeDocument extends PlainDocument {
        public void insertString(int offset, String string,
                                 AttributeSet attributes)
                                 throws BadLocationException {
            if (isNumeric(string)) {
                super.insertString(offset, string, attributes);
                maximum.setSelected(true);
            } else {
                Toolkit.getDefaultToolkit().beep();
            }
        }

        public void remove(int offset, int length)
            throws BadLocationException {
            super.remove(offset, length);
            maximum.setSelected(true);
        }

        private boolean isNumeric(String s) {
            try {
                Long.valueOf(s);
            } catch (NumberFormatException e) {
                return false;
            }
            return true;
        }
    }

    // Document to update the JAR cache location
    private class LocationDocument extends PlainDocument {
        public void insertString(int offset, String string,
                                 AttributeSet attributes)
                                 throws BadLocationException {
            super.insertString(offset, string, attributes);
        }

        public void remove(int offset, int length)
            throws BadLocationException {
            super.remove(offset, length);
        }
    }
    

    // JTextField returns a preferred width that depends on the length of
    // the text in the field.  This means the panel will lay out differently
    // depending on how long the JAR cache directory name is.  We don't want
    // that.  Instead, we subclass JTextField and return a very small
    // preferred width.  This way, the layout manager will resize the field
    // to fit the available space in the panel, instead of resizing the
    // panel to fit the length of the text.
    private class LocationField extends JTextField {
        public Dimension getPreferredSize() {
            Dimension d = super.getPreferredSize();
            d.width = 10;
            return d;
        }

    }

}
