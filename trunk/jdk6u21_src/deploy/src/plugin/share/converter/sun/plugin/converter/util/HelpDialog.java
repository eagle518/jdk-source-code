/*
 * @(#)HelpDialog.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.net.URL;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.html.*;

import sun.plugin.converter.ResourceHandler;


/**
 * Dialog to display the help.
 *
 */
public class HelpDialog extends JDialog implements ActionListener
{
    JButton okayButton;

    private ResourceBundle rb = null;
    String helpFile ="";

    public HelpDialog(Frame parent)
    {
	this(parent, ResourceHandler.getMessage("help_dialog.caption"), false);
    }

    public HelpDialog(Frame parent, boolean modal)
    {
	this(parent, ResourceHandler.getMessage("help_dialog.caption"), modal);
    }

    public HelpDialog(Frame parent, String title)
    {
	this(parent, title, false);
    }

    public HelpDialog(Frame parent, String title, boolean modal)
    {
        super(parent, title, modal);

        majorLayout();
    }


    /** 
     * <P> Layout all the elements of the help dialog.
     * </P>
     */
    public void majorLayout()
    {
        final JEditorPane editorPane = new JEditorPane();
         
        editorPane.setEditable(false);
        setResizable(true);

        try {
           rb = ResourceBundle.getBundle("sun.plugin.converter.resources.ConverterHelp");
        } catch(Exception ex) {
           ex.printStackTrace();
        }

        try {
           if (rb != null)
               helpFile = rb.getString("conhelp.file");
        }
        catch (Exception ex) {
           ex.printStackTrace();
        }
	
	//Gabe Boys: Set the preffered size of the scroll pane
	//so that when the pack() happens the text are is not
	//to big */

	// Setup scroll pane for editorPane 
        JScrollPane editorScrollPane = new JScrollPane(editorPane);
	editorScrollPane.setVerticalScrollBarPolicy(
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        editorScrollPane.setHorizontalScrollBarPolicy(
                JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        Dimension screenSize = getToolkit().getScreenSize();
	editorScrollPane.setPreferredSize(new Dimension((screenSize.width/2),(screenSize.height/2))); 

        editorPane.setText(helpFile);
        editorPane.setCaretPosition(0);
         
	// Setup okay button	
        okayButton    = new JButton(ResourceHandler.getMessage("button.okay"));
        okayButton.setMnemonic(ResourceHandler.getAcceleratorKey("button.okay"));
	okayButton.addActionListener(this);

	JPanel buttonsPanel = new JPanel();
	buttonsPanel.setLayout(new FlowLayout(FlowLayout.CENTER));
        buttonsPanel.add(okayButton);

	getContentPane().setLayout(new BorderLayout());
	getContentPane().add(editorScrollPane, BorderLayout.CENTER);
	getContentPane().add(buttonsPanel, BorderLayout.SOUTH);

	pack();
    }


    public void actionPerformed(ActionEvent e)
    {
        Component target = (Component) e.getSource();

        if (target == okayButton)
	{
	    setVisible(false);
	    dispose();
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
	
	//Gabe Boys:  Added the pack so that localization
	// would appear correctly(i.e. Strings don't run off 
	//of the edge of the Dialog box anymore)
  	if( state == true ) {	
	   super.pack();
	}
        super.setVisible(state);
    } 
}
