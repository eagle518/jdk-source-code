/*
 * @(#)AdvancedDialog.java	1.13 01/12/03
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import sun.plugin.converter.engine.PluginConverter;
import javax.swing.*;
import sun.plugin.converter.ResourceHandler;

/**
 * Dialog to display advanced options.
 *
 */
public class AdvancedDialog extends JDialog 
			    implements ActionListener, ItemListener
{
    private JTextField cabTextField, nsTextField, smartUpdateTextField, mimeTypeTextField, logTextField;
    private JButton resetButton, logButton, okButton, cancelButton;
    private JCheckBox logCheckBox;

    private String logFileNameStr;

    private PluginConverter converter;


    public AdvancedDialog(Frame parent, PluginConverter converter)
    {
	this(parent, ResourceHandler.getMessage("advanced_dialog.caption"), false, converter);
    }

    public AdvancedDialog(Frame parent, boolean modal, PluginConverter converter)
    {
	this(parent, ResourceHandler.getMessage("advanced_dialog.caption"), modal, converter);
    }

    public AdvancedDialog(Frame parent, String title, PluginConverter converter)
    {
	this(parent, title, false, converter);
    }

    public AdvancedDialog(Frame parent, String title, boolean modal, PluginConverter converter)
    {
        super(parent, title, modal);

	this.converter = converter;
        majorLayout();

	setup();
    }


    /** 
     * <P> Layout all the elements of the dialog.
     * </P>
     */
    public void majorLayout()
    {
        setResizable(true);

        JLabel cabLabel  = new JLabel(ResourceHandler.getMessage("advanced_dialog.cab"));
        cabTextField = new JTextField();


        JLabel nsLabel   = new JLabel(ResourceHandler.getMessage("advanced_dialog.plugin"));
        nsTextField = new JTextField();

        JLabel smartUpdateLabel   = new JLabel(ResourceHandler.getMessage("advanced_dialog.smartupdate"));
        smartUpdateTextField = new JTextField();

        JLabel mimeTypeLabel   = new JLabel(ResourceHandler.getMessage("advanced_dialog.mimetype"));
        mimeTypeTextField = new JTextField();


        JLabel logLabel  = new JLabel(ResourceHandler.getMessage("advanced_dialog.log"));
	logTextField = new JTextField();

	logButton = new JButton(ResourceHandler.getMessage("button.browse.dir"));
        logButton.setMnemonic(ResourceHandler.getAcceleratorKey("button.browse.dir"));
	logButton.addActionListener(this);

	logCheckBox = new JCheckBox(ResourceHandler.getMessage("advanced_dialog.generate"));
        logCheckBox.setMnemonic(ResourceHandler.getAcceleratorKey("advanced_dialog.generate"));
        
	logCheckBox.addItemListener(this);


	resetButton = new JButton(ResourceHandler.getMessage("button.reset"));
        resetButton.setMnemonic(ResourceHandler.getAcceleratorKey("button.reset"));
	resetButton.addActionListener(this);

        okButton = new JButton(ResourceHandler.getMessage("button.okay"));
        okButton.setMnemonic(ResourceHandler.getAcceleratorKey("button.okay"));
	okButton.addActionListener(this);

        cancelButton = new JButton(ResourceHandler.getMessage("button.cancel"));
        cancelButton.setMnemonic(ResourceHandler.getAcceleratorKey("button.cancel"));
	cancelButton.addActionListener(this);

	Box box = Box.createHorizontalBox();
	box.add(resetButton);
	box.add(Box.createHorizontalGlue());
	box.add(cancelButton);
	box.add(Box.createHorizontalStrut(5));
	box.add(okButton);

	GridBagLayout layout = new GridBagLayout();
	getContentPane().setLayout(layout);
	    
	layout.setConstraints(cabLabel, 
			      new GridBagConstraints(0, 0, GridBagConstraints.REMAINDER, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,10,0,0), 0, 0));
	layout.setConstraints(cabTextField, 
			      new GridBagConstraints(0, 1, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(nsLabel, 
			      new GridBagConstraints(0, 2, GridBagConstraints.REMAINDER, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,10,0,0), 0, 0));
	layout.setConstraints(nsTextField, 
			      new GridBagConstraints(0, 3, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(smartUpdateLabel, 
			      new GridBagConstraints(0, 4, GridBagConstraints.REMAINDER, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,10,0,0), 0, 0));
	layout.setConstraints(smartUpdateTextField, 
			      new GridBagConstraints(0, 5, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(mimeTypeLabel, 
			      new GridBagConstraints(0, 6, GridBagConstraints.REMAINDER, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,10,0,0), 0, 0));
	layout.setConstraints(mimeTypeTextField, 
			      new GridBagConstraints(0, 7, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(logCheckBox, 
			      new GridBagConstraints(0, 8, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(20,10,0,0), 0, 0));
	layout.setConstraints(logLabel, 
			      new GridBagConstraints(0, 9, GridBagConstraints.REMAINDER, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(5,10,0,0), 0, 0));
	layout.setConstraints(logTextField, 
			      new GridBagConstraints(0, 10, 2, 1, 
						     2, 0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(logButton, 
			      new GridBagConstraints(2, 10, 1, 1, 
						     0, 0, GridBagConstraints.EAST, GridBagConstraints.NONE, 
						     new Insets(0,10,0,10), 0, 0));
	layout.setConstraints(box, 
			      new GridBagConstraints(0, 11, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(20,10,10,10), 0, 0));


	getContentPane().add(cabLabel);
	getContentPane().add(cabTextField);
	getContentPane().add(nsLabel);
	getContentPane().add(nsTextField);
	getContentPane().add(smartUpdateLabel);
	getContentPane().add(smartUpdateTextField);
	getContentPane().add(mimeTypeLabel);
	getContentPane().add(mimeTypeTextField);
	getContentPane().add(logCheckBox);
	getContentPane().add(logLabel);
	getContentPane().add(logTextField);
	getContentPane().add(logButton);
	getContentPane().add(box);

	pack();
    }

    private void setup()
    {
	cabTextField.setText(converter.getCabFileLocation());
	nsTextField.setText(converter.getNSFileLocation());
	smartUpdateTextField.setText(converter.getSmartUpdateLocation());
	mimeTypeTextField.setText(converter.getMimeType());

	logCheckBox.setSelected(converter.isCreateLog());
	enableLogTextField(logCheckBox.isSelected());

	File logFile = converter.getLogFile();
	if ( logFile != null ) {
	    logFileNameStr = logFile.getName();
	    logTextField.setText(logFile.getPath());
	}
    }

    public void resetDefaults()
    {
	cabTextField.setText(converter.getDefaultCabFileLocation());
	nsTextField.setText(converter.getDefaultNSFileLocation());
	smartUpdateTextField.setText(converter.getDefaultSmartUpdateLocation());
	mimeTypeTextField.setText(converter.getDefaultMimeType());
	logTextField.setText(converter.getDefaultLogFile().getPath());
    }

    public void actionPerformed(ActionEvent e)
    {
        Component target = (Component) e.getSource();

        if (target == resetButton)
	    {
		resetDefaults();
	    }
        else if (target == okButton)
	    {
		converter.setCabFileLocation(cabTextField.getText());
		converter.setNSFileLocation(nsTextField.getText());
		converter.setSmartUpdateLocation(smartUpdateTextField.getText());
		converter.setMimeType(mimeTypeTextField.getText());
		converter.setLogFile(new File(logTextField.getText()));
		converter.setCreateLog(logCheckBox.isSelected());

		setVisible(false);
		dispose();
	    }
        else if (target == cancelButton)
	    {
		setVisible(false);
		dispose();
	    }
	else if (target == logButton)
	{
	    JFileChooser chooser = new JFileChooser(); 
	    
	    try  {
		chooser.setCurrentDirectory(new File(logTextField.getText()));
	    } catch (Exception ex)  {
	    }

	    int returnVal = chooser.showOpenDialog(this); 
	    if(returnVal == JFileChooser.APPROVE_OPTION) 
	    { 
		logTextField.setText(chooser.getSelectedFile().getAbsolutePath());
	    }
	}
    }

    /**
     * Overrides java.awt.Container.setVisible() so that we are
     * sure that the Frame's size is set before displaying.
     */
    public void setVisible(boolean state)
    {
	if (state)
	{
            Dimension bounds = getToolkit().getScreenSize();
            setLocation((bounds.width - getSize().width)/2,
        	        (bounds.height - getSize().height)/2);
        }

        //Gabe Boys: Added pack to make internationaliztion work correctly
	if ( state == true ) { 
	   super.pack();	
	}
        super.setVisible(state);
    }

    /**
     * Enable/disable logTextField.
     */
    private void enableLogTextField(boolean state)
    {
	logTextField.setEditable(state);
	logButton.setEnabled(state);
    }

    /**
     * Handle ItemEvents.
     */
    public void itemStateChanged(ItemEvent e)
    {
	Component target = (Component) e.getSource();
	
	if (target == logCheckBox)
	{
	    enableLogTextField(logCheckBox.isSelected());
	}
    }
    
    
}
