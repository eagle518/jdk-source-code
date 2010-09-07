/*
 * @(#)ProgressGUI.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.gui;

import java.awt.*;
import java.awt.event.*;
import java.io.*;

import sun.plugin.converter.ResourceHandler;
import sun.plugin.converter.util.*;
import sun.plugin.converter.engine.*;
import javax.swing.*;


/**
 * Frame which contains the progress report GUI of <B><I>Cuckoo</I></B>.
 *
 */
public class ProgressGUI extends JFrame implements ActionListener, ConverterProgressListener
{
    private JButton cancelBttn;
    private JLabel processingLbl, folderLbl, fileLbl,
                  totalFilesLbl, totalAppletsLbl, totalErrorsLbl;
    private JLabel processing2Lbl, folder2Lbl, file2Lbl,
                  totalFiles2Lbl, totalApplets2Lbl, totalErrors2Lbl;
    private PluginConverter converter;

    /**
     * Creates new ProgressGUI Frame.
     */
    public ProgressGUI(PluginConverter converter)
    {
	super(ResourceHandler.getMessage("progress_dialog.caption"));

	this.converter = converter;

	setResizable(true);

	majorLayout();
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

	super.setVisible(state);

	if (state)
	{
	    converter.startConversion();
	    converter.addConverterProgressListener(this);
        }
    }

    /**
     * Creates the GUI.
     */
    public void majorLayout()
    {
	processingLbl    = new JLabel(ResourceHandler.getMessage("progress_dialog.processing"));
	processing2Lbl   = new JLabel();
	folderLbl        = new JLabel(ResourceHandler.getMessage("progress_dialog.folder"));
	folder2Lbl       = new JLabel();
	fileLbl          = new JLabel(ResourceHandler.getMessage("progress_dialog.file"));
	file2Lbl         = new JLabel();
	totalFilesLbl    = new JLabel(ResourceHandler.getMessage("progress_dialog.totalfile"));
	totalFiles2Lbl   = new JLabel();
	totalAppletsLbl  = new JLabel(ResourceHandler.getMessage("progress_dialog.totalapplet"));
	totalApplets2Lbl = new JLabel();
	totalErrorsLbl   = new JLabel(ResourceHandler.getMessage("progress_dialog.totalerror"));
	totalErrors2Lbl  = new JLabel();
	cancelBttn       = new JButton(ResourceHandler.getMessage("button.cancel"));
        cancelBttn.setMnemonic(ResourceHandler.getAcceleratorKey("button.cancel"));


	addListeners();

	final int buf=10,           // Buffer (between components and form)
		  vsp=5,            // Vertical space
		  indent=30;        // Indent between form (left edge) and component

	GridBagLayout layout = new GridBagLayout();
	getContentPane().setLayout(layout);
	    
	layout.setConstraints(processingLbl, 
			      new GridBagConstraints(0, 0, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(buf,buf,0,0), 0, 0));
	layout.setConstraints(processing2Lbl, 
			      new GridBagConstraints(1, 0, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(buf,0,0,buf), 0, 0));
	layout.setConstraints(folderLbl, 
			      new GridBagConstraints(0, 1, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,buf,0,0), 0, 0));
	layout.setConstraints(folder2Lbl, 
			      new GridBagConstraints(1, 1, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(10,0,0,buf), 0, 0));
	layout.setConstraints(fileLbl, 
			      new GridBagConstraints(0, 2, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(vsp,buf,0,0), 0, 0));
	layout.setConstraints(file2Lbl, 
			      new GridBagConstraints(1, 2, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, 
						     new Insets(vsp,0,0,buf), 0, 0));
	layout.setConstraints(totalFilesLbl, 
			      new GridBagConstraints(1, 3, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,0,0,0), 0, 0));
	layout.setConstraints(totalFiles2Lbl, 
			      new GridBagConstraints(2, 3, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(10,0,0,buf), 0, 0));
	layout.setConstraints(totalAppletsLbl, 
			      new GridBagConstraints(1, 4, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(vsp,0,0,0), 0, 0));
	layout.setConstraints(totalApplets2Lbl, 
			      new GridBagConstraints(2, 4, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, 
						     new Insets(vsp,0,0,buf), 0, 0));
	layout.setConstraints(totalErrorsLbl, 
			      new GridBagConstraints(1, 5, 1, 1, 
						     0, 0, GridBagConstraints.WEST, GridBagConstraints.NONE, 
						     new Insets(vsp,0,0,0), 0, 0));
	layout.setConstraints(totalErrors2Lbl, 
			      new GridBagConstraints(2, 5, GridBagConstraints.REMAINDER, 1, 
						     1, 0, GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, 
						     new Insets(vsp,0,0,buf), 0, 0));
	layout.setConstraints(cancelBttn, 
			      new GridBagConstraints(0, 6, GridBagConstraints.REMAINDER, 1, 
						     1, 1, GridBagConstraints.CENTER, GridBagConstraints.NONE, 
						     new Insets(0,0,buf,buf), 0, 0));

	getContentPane().add(processingLbl);
	getContentPane().add(processing2Lbl);
	getContentPane().add(folderLbl);
	getContentPane().add(folder2Lbl);
	getContentPane().add(fileLbl);
	getContentPane().add(file2Lbl);
	getContentPane().add(totalFilesLbl);
	getContentPane().add(totalFiles2Lbl);
	getContentPane().add(totalAppletsLbl);
	getContentPane().add(totalApplets2Lbl);
	getContentPane().add(totalErrorsLbl);
	getContentPane().add(totalErrors2Lbl);
	getContentPane().add(cancelBttn);

	pack();
    }


    /**
     * Add listeners to components.
     */
    private void addListeners()
    {
	cancelBttn.addActionListener(this);

        addWindowListener(new WindowAdapter()
	{
	    public void windowClosing(WindowEvent e)
	    {
	        close();
	    }
	});
    }

    /**
     * Handle ActionEvents.
     */
    public void actionPerformed(ActionEvent e)
    {
	Component target = (Component) e.getSource();

	if (target == cancelBttn)
	{
	    /*TMP*/ close();
	}
    }

    /**
     * Stop the conversion Thread, hide the Frame, reclaim resources, and exit.
     */
    protected void close()
    {
	converter.stopConversion();

	setVisible(false);
	dispose();
    }

    public void converterProgressUpdate(ConverterProgressEvent e)
    {
	processing2Lbl.setText(e.getStatusText());
	folder2Lbl.setText(e.getSourcePath());
	file2Lbl.setText(e.getCurrentFile());
	totalFiles2Lbl.setText(String.valueOf(e.getFilesProcessed()));
	totalApplets2Lbl.setText(String.valueOf(e.getAppletsFound()));
	totalErrors2Lbl.setText(String.valueOf(e.getErrorsFound()));

	//Gabe Boys: Added this so that localization can handle big strings.
	pack();
	
	if (e.getStatus() == ConverterProgressEvent.ALL_DONE)
	{
	    folder2Lbl.setText("");
    	    file2Lbl.setText("");

    	    cancelBttn.setText(ResourceHandler.getMessage("button.done"));
            cancelBttn.setMnemonic(ResourceHandler.getAcceleratorKey("button.done"));
	}
	else if (e.getStatus() == ConverterProgressEvent.DEST_DIR_NOT_CREATED)
	{
	    // TO-DO:  Dialog("Could Not Create The Destination Directory");
	    //         display the dir that it tried to create.
	}
    }
}
