/*
 * @(#)CacheSettingsDialog.java	1.29 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.JComponent;
import javax.swing.KeyStroke;
import javax.swing.JDialog;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.JComboBox;
import javax.swing.JSlider;
import javax.swing.JSeparator;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JLabel;
import javax.swing.JFileChooser;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.AbstractAction;
import javax.swing.BoxLayout;
import javax.swing.filechooser.FileFilter;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.JCheckBox;
import javax.swing.text.NumberFormatter;
import javax.swing.border.TitledBorder;

import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.Dimension;
import java.awt.event.WindowEvent;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ItemListener;
import java.awt.event.ItemEvent;

import java.io.File;
import java.io.IOException;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.ui.DialogTemplate;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.resources.ResourceManager;


public class CacheSettingsDialog extends JDialog {
    
    private final int DIALOG_WIDTH_UNIX = 510;
    private final int DIALOG_WIDTH_WIN = 470;
    private final int VERTICAL_STRUT_UNIX = 12;
    private final int VERTICAL_STRUT_WIN = 6;
    private final int CACHE_MIN_SIZE = 1; // min cache size is 1 MB
    private final int CACHE_MAX_SIZE = 1000;  // 1 GB
        
    private JTextField location;
    private JLabel locLbl, diskSpaceLbl, compressionLbl, unitsLbl;
    private JButton chooseBtn, okBtn, cancelBtn, deleteFilesBtn, restoreDefaultsBtn;
    private JComboBox compression;
    private JSlider cacheSizeSlider;
    private JSpinner cacheSizeSpinner;
    private JCheckBox cacheEnabled;
    private int dialogWidth = DIALOG_WIDTH_UNIX;
    private int verticalStrut = VERTICAL_STRUT_UNIX;

    /** Creates new form CacheSettings */
    public CacheSettingsDialog(Frame parent, boolean modal) {
        super(parent, modal);
        
        // Find out the OS we are running on.  For windows,
        // use VERTICAL_STRUT_WIN and DIALOG_WIDTH_WIN  
        if (System.getProperty("os.name").toLowerCase().indexOf("windows") != -1){
            dialogWidth = DIALOG_WIDTH_WIN;
            verticalStrut = VERTICAL_STRUT_WIN;
        }
        
        initComponents(); 
    }

    
    /** This method is called from within the constructor to
     * initialize the form.
     *
     */
    private void initComponents() {
        setTitle(getMessage("cache.settings.dialog.title"));

        JPanel chboxPanel = new JPanel(new FlowLayout(FlowLayout.LEADING, 0, 12));
        cacheEnabled = new JCheckBox(getMessage("cache.settings.dialog.cacheEnabled"));
        cacheEnabled.setMnemonic(ResourceManager.getVKCode
                ("cache.settings.dialog.cacheEnabled.mnemonic"));
        cacheEnabled.addItemListener(new ItemListener(){
            public void itemStateChanged(ItemEvent e) {
                checkboxStateChanged(e);
            }
        });
        chboxPanel.add(Box.createHorizontalStrut(12));
        chboxPanel.add(cacheEnabled);
        
        //
        // Create "Location" panel.
        //
        JPanel locationPanel = new JPanel(new BorderLayout());
        
        locationPanel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(0, 12, verticalStrut, 12),
                BorderFactory.createTitledBorder(getMessage(
                "cache.settings.dialog.cache_location"))));
        
        locLbl = new JLabel(getMessage("cache.settings.dialog.location_label"));
        
        JPanel northSubPanel = new JPanel(new FlowLayout(FlowLayout.LEADING, 0, 5));
        northSubPanel.add(Box.createHorizontalStrut(24));
        northSubPanel.add(locLbl);
        
        JPanel textFieldSubPanel = new JPanel();
        // See how this looks in windows L&F - if button and text field are too tall,
        // then set empty border with vgap=5.
        textFieldSubPanel.setLayout(new BoxLayout(textFieldSubPanel, BoxLayout.X_AXIS));
        
        location = new JTextField();
        // JTextField returns a preferred width that depends on the length of
        // the text in the field.  This means the panel will lay out differently
        // depending on how long the JAR cache directory name is.  We don't want
        // that.  Instead, we set preferred width to very small number.
        // This way, the layout manager will resize the field
        // to fit the available space in the panel, instead of resizing the
        // panel to fit the length of the text.
        location.setPreferredSize(new Dimension(10, location.getPreferredSize().height));
        
        chooseBtn = makeButton("cache.settings.dialog.change_btn");
        chooseBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                chooseButtonActionPerformed(evt);
            }
        });
        chooseBtn.setToolTipText(getMessage("cache.settings.dialog.change_btn.tooltip"));
        
        textFieldSubPanel.add(Box.createHorizontalStrut(24));
        textFieldSubPanel.add(location);
        textFieldSubPanel.add(Box.createHorizontalStrut(12));
        textFieldSubPanel.add(chooseBtn);
        textFieldSubPanel.add(Box.createHorizontalStrut(12));
        
        locationPanel.add(northSubPanel, BorderLayout.NORTH);
        locationPanel.add(textFieldSubPanel, BorderLayout.CENTER);
        locationPanel.add(Box.createVerticalStrut(verticalStrut), BorderLayout.SOUTH);
        
        
        //
        // Create panel with compression and cache size settings.
        //
        JPanel diskSpacePanel = new JPanel();
        diskSpacePanel.setLayout(new BorderLayout());
        
        diskSpacePanel.setBorder(BorderFactory.createCompoundBorder(
                        BorderFactory.createEmptyBorder(0, 12, verticalStrut, 12),
                        BorderFactory.createTitledBorder(getMessage(
                                 "cache.settings.dialog.disk_space"))));
        
        // Make sure this appears correctly when translated.
        compressionLbl = new JLabel(getMessage("cache.settings.dialog.compression"));
        
        String[] compressionElements = {getMessage("cache.settings.dialog.none"),
                                         getMessage("cache.settings.dialog.low"),
                                         getMessage("cache.settings.dialog.medium"),
                                         getMessage("cache.settings.dialog.high") };
                
        compression = new JComboBox(compressionElements);
                
        //
        // Create sub panel for compression-related items:
        //
        JPanel compressionSubPanel = new JPanel();
        compressionSubPanel.setLayout(new BoxLayout(compressionSubPanel, BoxLayout.X_AXIS));
        compressionSubPanel.setBorder(
                BorderFactory.createEmptyBorder(0, 0, verticalStrut, 0));
                
        compressionSubPanel.add(Box.createHorizontalStrut(24));
        compressionSubPanel.add(compressionLbl);
        compressionSubPanel.add(Box.createHorizontalStrut(12));
        compressionSubPanel.add(Box.createHorizontalGlue());
        compressionSubPanel.add(compression);
                
        // Make sure this appears correctly when translated.
        diskSpaceLbl = new JLabel(getMessage("cache.settings.dialog.diskSpaceLbl"));
                
        JPanel diskSpaceLblPanel = new JPanel();
        diskSpaceLblPanel.setLayout(new BoxLayout(diskSpaceLblPanel, BoxLayout.X_AXIS));
        diskSpaceLblPanel.setBorder(
                BorderFactory.createEmptyBorder(0, 0, verticalStrut, 0));
        diskSpaceLblPanel.add(Box.createHorizontalStrut(24));
        diskSpaceLblPanel.add(diskSpaceLbl);
        diskSpaceLblPanel.add(Box.createHorizontalGlue());
        diskSpaceLblPanel.add(Box.createHorizontalStrut(12));
                
        //
        // Create slider, spinner and label for disk space usage for cache.
        // We should get available disk space from System here and 
        // set CACHE_MAX_SIZE class variable to this value.
        // Currently we are using hard-coded 1 GB for unlimited disk size.
        //
        cacheSizeSlider = new JSlider(CACHE_MIN_SIZE, CACHE_MAX_SIZE);        
        
        // We want to see 20 tix all together - looks nice this way.
        // define tick spacing based on max size / number of desired ticks.
        cacheSizeSlider.setMinorTickSpacing((int)(CACHE_MAX_SIZE/20));
        cacheSizeSlider.setPaintTicks(true);
        cacheSizeSlider.setPaintLabels(false);
        cacheSizeSlider.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent evt) {
                sliderStateChanged(evt);
            }
        });
                
        // Something is wrong here with the number of columns for spinner.
        // see if there are any bugs on this in bugtraq.
        cacheSizeSpinner = new JSpinner(new SpinnerNumberModel(CACHE_MAX_SIZE,
                                                               CACHE_MIN_SIZE,
                                                               CACHE_MAX_SIZE,
                                                               CACHE_MIN_SIZE));
        JSpinner.NumberEditor spinnerEditor = 
                new JSpinner.NumberEditor(cacheSizeSpinner, "######");
        spinnerEditor.getTextField().setColumns(8);
        spinnerEditor.getTextField().setHorizontalAlignment(JTextField.TRAILING);
        
        // Set spinner's formatter to commit all valid editing.  This means that
        // when user presses numeric key, change event will be generated and
        // we'll update slider's value.
        ((NumberFormatter)spinnerEditor.getTextField().getFormatter()).
                setCommitsOnValidEdit(true);
        
        // Allow only valid input.  This means numeric, between 1 and CACHE_MAX_SIZE.
        ((NumberFormatter)spinnerEditor.getTextField().getFormatter()).
                setAllowsInvalid(false);
        cacheSizeSpinner.setEditor(spinnerEditor);
        
        // Add change listener - so that when any valid input is entered in
        // spinner, slider's value would get updated.  
        cacheSizeSpinner.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent evt) {
                updateSlider();
            }
        });
                        
        // No need to localize.
        unitsLbl = new JLabel("MB");
                
        // Align combo box for compression with spinner.
        int leftOffset = unitsLbl.getPreferredSize().width + 12 +12;
        compressionSubPanel.add(Box.createHorizontalStrut(leftOffset));
                
        JPanel sliderPanel = new JPanel();
        sliderPanel.setLayout(new BoxLayout(sliderPanel, BoxLayout.X_AXIS));
        sliderPanel.setBorder(
                BorderFactory.createEmptyBorder(0, 0, verticalStrut, 0));
                
        // This is to temporary work around the problem of spinner stretching
        // too much vertically...
        JPanel spinnerSubPanel = new JPanel();
        spinnerSubPanel.setLayout(new FlowLayout(FlowLayout.TRAILING, 0, 5));
        spinnerSubPanel.add(cacheSizeSpinner);
        
        sliderPanel.add(Box.createHorizontalStrut(24));
        sliderPanel.add(cacheSizeSlider);
        // Add 12 pix strut.
        sliderPanel.add(Box.createHorizontalStrut(12));
        sliderPanel.add(spinnerSubPanel);
        sliderPanel.add(Box.createHorizontalStrut(12));
        sliderPanel.add(unitsLbl);
        sliderPanel.add(Box.createHorizontalStrut(12));
                
                
        diskSpacePanel.add(compressionSubPanel, BorderLayout.NORTH);
        diskSpacePanel.add(diskSpaceLblPanel, BorderLayout.CENTER);
        diskSpacePanel.add(sliderPanel, BorderLayout.SOUTH);
        
        //
        // Create panel with action buttons to delete/view cached files,
        // and restore default cache settings.
        //
        JPanel buttonsPanel = new JPanel();        
        buttonsPanel.setLayout(new FlowLayout(FlowLayout.TRAILING, 6, 5));
        buttonsPanel.setBorder(
                BorderFactory.createEmptyBorder(0, 12, verticalStrut, 12));
                
        deleteFilesBtn = makeButton("cache.settings.dialog.delete_btn");
        deleteFilesBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                deleteFilesBtnActionPerformed(evt);
            }
        });
        deleteFilesBtn.setToolTipText(getMessage("temp.files.delete.btn.tooltip"));
                
        restoreDefaultsBtn= makeButton("cache.settings.dialog.restore_btn");
        restoreDefaultsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                restoreDefaultsBtnActionPerformed(evt);
            }
        });
        restoreDefaultsBtn.setToolTipText(getMessage("cache.settings.dialog.restore_btn.tooltip"));
                
        DialogTemplate.resizeButtons(new JButton[] {deleteFilesBtn, 
                                                  restoreDefaultsBtn});
                
        buttonsPanel.add(deleteFilesBtn);
        buttonsPanel.add(Box.createHorizontalGlue());
        buttonsPanel.add(Box.createHorizontalGlue());
        buttonsPanel.add(restoreDefaultsBtn);
                
        //
        // Middle panel contains location, disk size, and buttons subpanels.
        //
        JPanel middlePanel = new JPanel();
        middlePanel.setLayout(new BorderLayout());
        middlePanel.add(locationPanel, BorderLayout.NORTH);
        middlePanel.add(diskSpacePanel, BorderLayout.CENTER);
        middlePanel.add(buttonsPanel, BorderLayout.SOUTH);
                
        // assemble decision panel
        okBtn = new JButton(getMessage("common.ok_btn"));
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
        
        cancelBtn = new JButton(getMessage("common.cancel_btn"));
        cancelBtn.addActionListener(cancelAction);
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        DialogTemplate.resizeButtons(new JButton[] {okBtn, cancelBtn});
                
        JPanel decisionPanel = new JPanel();
        decisionPanel.setBorder(BorderFactory.createEmptyBorder(
                verticalStrut, 12, verticalStrut, 12));
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 0, 0));
        decisionPanel.add(Box.createHorizontalGlue());
        decisionPanel.add(okBtn);
        decisionPanel.add(Box.createHorizontalStrut(6));
        decisionPanel.add(cancelBtn);
        
        //
        // This is SOUTH panel that contains JSeparator of dialogWidth pixels, which
        // defines the width of the whole dialog and decision panel
        // with "OK" and "Cancel" buttons.
        //
        JPanel southPanel = new JPanel();        
        southPanel.setLayout(new BorderLayout());
        JSeparator sep = new JSeparator();
        sep.setPreferredSize(new Dimension(dialogWidth, 1));
        
        southPanel.add(sep, BorderLayout.NORTH);
        southPanel.add(decisionPanel, BorderLayout.CENTER);
                
        getContentPane().add(chboxPanel, BorderLayout.NORTH);
        getContentPane().add(middlePanel, BorderLayout.CENTER);
        getContentPane().add(southPanel, BorderLayout.SOUTH);                
                
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
        boolean enab = !Config.isLocked(Config.CACHEDIR_KEY) &&
                (!Config.getInstance().isPlatformWindowsVista());
        location.setEnabled(enab);
        
        // Do not allow typing in location text field.  Users can still
        // type in the file chooser...
        location.setEditable(false);
        chooseBtn.setEnabled(enab);
        
        /*
         * Set cache size.  Default setting in Config is "unlimited", which is "-1".
         * If cache size is unlimited, set maximum number on spinner to be 
         * available disk space.  This will have to be added later, once
         * RFE 4057701 is implemented.  For now CACHE_MAX_SIZE is hard
         * coded to be 100 GB.
         */
        int val;
        String cacheSize = (Config.getProperty(Config.CACHE_MAX_KEY)).trim();
        enab = !Config.isLocked(Config.CACHE_MAX_KEY);
        try {
            val = Integer.valueOf(cacheSize).intValue();
            if (val == -1) {
                /*
                 * Set spinner's value to disk's size.
                 * Set slider's position to disk's size.
                 */
               ((JSpinner.NumberEditor)cacheSizeSpinner.getEditor()).getTextField()
                                            .setValue(Integer.valueOf(CACHE_MAX_SIZE));     
               
                cacheSizeSlider.setValue(CACHE_MAX_SIZE);
            } else {
                /*
                 * Set spinner's value to <val>.
                 * Set slider's position to <val>.
                 */
                ((JSpinner.NumberEditor)cacheSizeSpinner.getEditor()).getTextField()
                                                        .setValue(Integer.valueOf(val));
                
                cacheSizeSlider.setValue(val);
            }
        } catch (NumberFormatException e) {
            val = CACHE_MAX_SIZE;
            /*
             * Set spinner's value to <val>.
             * Set slider's position to <val>.
             */
            cacheSizeSpinner.setValue(Integer.valueOf(val));
            
            cacheSizeSlider.setValue(val);
        }
        
        /*
         * Some of the components might be disabled at this point.  If
         * CACHE_MAX_KEY is not locked, and enab=true; we shouldn't re-enable
         * components that should be diabled independently of CACHE_MAX_KEY.
         */
        if ( !enab ){
            cacheSizeSlider.setEnabled(enab);
            cacheSizeSpinner.setEnabled(enab);
        }
        
        enab = !Config.isLocked(Config.CACHE_COMPRESSION_KEY);
        // Set compression value from config.  Allowed values in Config are 0-9
        // Get the number, divide it by 3 and set index to integer.  We should
        // get only 0, 3, 6, and 9 values from Config, but is user set it in
        // properties file manually...
        int compressionIdx = Config.getIntProperty(Config.CACHE_COMPRESSION_KEY);
        if (( (int)compressionIdx/3) < compression.getItemCount()){
            compression.setSelectedIndex((int)compressionIdx/3);
        }else{
            compression.setSelectedIndex(0);
        }
        compression.setEnabled(enab);
        
        enab = !Config.isLocked(Config.CACHE_ENABLED_KEY);
        // See if caching is enabled...
	boolean enabled = Config.getBooleanProperty(Config.CACHE_ENABLED_KEY);
        cacheEnabled.setSelected(enabled);
        cacheEnabled.setEnabled(enab);
        
        setAllEnabled(enabled);
    }
    
    private void restoreDefaultsBtnActionPerformed(ActionEvent evt) {
        // Restore default values for cache settings locally - in this
        // dialog only.  This is so that user could click "Cancel"
        // and revert everything to custom settings.
        
        // If cache dir is not locked, reset it to 
        // "$USER_HOME" + File.separator + "cache"
        if ( !Config.isLocked(Config.CACHEDIR_KEY) ){
            location.setText(Config.getDefaultCacheDirectory());
            location.setToolTipText(location.getText());
        }
        
        // If cache max size is not locked, reset it to maximum
        // available disk space.  Update spinner and slider.
        if ( !Config.isLocked(Config.CACHE_MAX_KEY) ){   
            ((JSpinner.NumberEditor)cacheSizeSpinner.getEditor()).getTextField()
                                        .setValue(Integer.valueOf(CACHE_MAX_SIZE));
            cacheSizeSlider.setValue(CACHE_MAX_SIZE);
        }
        
        // Reset compression to default
        if ( !Config.isLocked(Config.CACHE_COMPRESSION_KEY) ){
            int compressionIdx = Config.CACHE_COMPRESSION_DEF;
            if (( (int)compressionIdx/3) < compression.getItemCount()){
                compression.setSelectedIndex((int)compressionIdx/3);
            }else{
                compression.setSelectedIndex(0);
            }
        }
        
        if ( !Config.isLocked(Config.CACHE_ENABLED_KEY) ){
            cacheEnabled.setSelected(Config.CACHE_ENABLED_DEF);
            setAllEnabled(cacheEnabled.isSelected());
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

	    if (new File(file).isDirectory()) {
                location.setText(file);
                location.setToolTipText(file);
	    } else {
		String errorTitle = getMessage("cache.settings.dialog.chooser_title");
		String errorMasthead = getMessage("cache.settings.dialog.directory_masthead");
		String errorBody = getMessage("cache.settings.dialog.directory_body");
		String btnString = ResourceManager.getString("common.ok_btn");

		UIFactory.showErrorDialog(this, new AppInfo(), errorTitle, errorMasthead, 
					  errorBody, btnString, null, null, null, null);
					
	    }

        }
    }
    
    private void deleteFilesBtnActionPerformed(ActionEvent evt) {
        new DeleteFilesDialog(this);
    }
    
    
    private void okBtnActionPerformed(ActionEvent evt) {
        // Save values set - pass them to the Config;
        // Close this dialog        
       
        Config.setCacheDirectory(location.getText());
        
        // If current selection is set to maximum, save CACHE_MAX_SIZE
        // as unlimited, which is "-1".  Else, save selected cache limit.
        if ( cacheSizeSlider.getValue() == CACHE_MAX_SIZE){
            Config.setProperty(Config.CACHE_MAX_KEY, Config.CACHE_MAX_DEF);
        }else{
            Config.setProperty(Config.CACHE_MAX_KEY,  
                               String.valueOf(cacheSizeSlider.getValue()));
        }
        
        // Set compression to selected index*3 - to keep values we currently have
        // for compression 0-9.
        Config.setIntProperty(Config.CACHE_COMPRESSION_KEY, compression.getSelectedIndex() * 3);
        
        Config.setBooleanProperty(Config.CACHE_ENABLED_KEY, cacheEnabled.isSelected());
        
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
    
    private String getMessage(String id) {
        return ResourceManager.getMessage(id);
    }
    
    public JButton makeButton(String key) {
        JButton b = new JButton(getMessage(key));
        b.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
        return b;
    }
    
    //
    // When slider's value changes, update spinner.
    //
    private void sliderStateChanged(ChangeEvent evt) {
        cacheSizeSpinner.setValue(Integer.valueOf(cacheSizeSlider.getValue()));                       
    }
    
    //
    // When spinner's value changes, update slider.
    //
    private void updateSlider(){
        cacheSizeSlider.setValue( Integer.valueOf
                    (cacheSizeSpinner.getModel().getValue().toString()).intValue());       
    }
    
    private void checkboxStateChanged(ItemEvent ie){
        // disable cache-related components if cache is disabled 
        if (ie.getStateChange() == ItemEvent.DESELECTED) {
		setAllEnabled(false);
        }
        
        // enabled cache-related components if cache is enabled
        if (ie.getStateChange() == ItemEvent.SELECTED) {
		setAllEnabled(true);
        }
    }

    private void setAllEnabled(boolean enable){
        location.setEnabled(enable && !Config.isLocked(Config.CACHEDIR_KEY)); 
        locLbl.setEnabled(enable && !Config.isLocked(Config.CACHEDIR_KEY));
        chooseBtn.setEnabled(enable && !Config.isLocked(Config.CACHEDIR_KEY) &&
                !Config.getInstance().isPlatformWindowsVista());
        compression.setEnabled(enable && !Config.isLocked(Config.CACHE_COMPRESSION_KEY));
        compressionLbl.setEnabled(enable && !Config.isLocked(Config.CACHE_COMPRESSION_KEY));
        diskSpaceLbl.setEnabled(enable && !Config.isLocked(Config.CACHE_MAX_KEY));
        cacheSizeSlider.setEnabled(enable && !Config.isLocked(Config.CACHE_MAX_KEY));
        cacheSizeSpinner.setEnabled(enable && !Config.isLocked(Config.CACHE_MAX_KEY));
        unitsLbl.setEnabled(enable && !Config.isLocked(Config.CACHE_MAX_KEY));
    }
    
}
