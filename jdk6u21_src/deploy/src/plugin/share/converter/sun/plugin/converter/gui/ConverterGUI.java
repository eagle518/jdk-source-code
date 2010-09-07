/*
 * @(#)ConverterGUI.java	1.30 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.gui;

import java.awt.*;
import java.awt.event.*;
import java.awt.event.ItemEvent.*;
import java.util.*;
import java.io.*;

import sun.plugin.converter.ResourceHandler;
import sun.plugin.converter.util.*;
import sun.plugin.converter.engine.*;
import javax.swing.*;
import javax.swing.border.*;
import java.text.MessageFormat;

/**
 * Frame which contains the major component of the GUI of <B><I>Cuckoo</I></B>.
 *
 */
public class ConverterGUI extends JFrame 
			  implements ActionListener, ItemListener
{
    private JButton dirBttn, backupBttn, runBttn, invisibleBttn;
    private JRadioButton staticVersioningRadioButton,dynamicVersioningRadioButton;
    private JCheckBox recursiveCheckBox;
    private JTextField dirTF, matchingTF, backupTF;
    private JPanel staticVersioningPanel = new JPanel();
    private JPanel dynamicVersioningPanel = new JPanel();
    private JTextArea staticVersioningTextArea;
    private JTextArea dynamicVersioningTextArea;
    private TemplateFileChoice templateCh;
    private JLabel dirLabel, matchingLabel, backupLabel, templateLabel,
                   staticVersioningLabel;
    private JMenuItem exitMenuItem, optionMenuItem, helpMenuItem, aboutMenuItem;
    private JSeparator sep1 = new JSeparator();
    private JSeparator sep2 = new JSeparator();
    private JSeparator sep3 = new JSeparator();

    private String[] descriptors;

    private String defaultDirBackupPath, defaultOneFileBackupPath;

    private boolean firstTime = true;

    private static PluginConverter converter;
    private ConvertSet convertSet;

    /**
     * Creates new ConverterGUI Frame.
     */
    public ConverterGUI()
    {
	super(ResourceHandler.getMessage("product_name"));   // UniCode (tm) == \u2122

	try
	{
converter = new PluginConverter();
	    converter.setShowProgressStdOut(false);
	}
	catch (FileNotFoundException e)
	{
	    showNoTemplateDialog();
	}
	converter.setConvertSet(convertSet = new DefaultSet());

	setResizable(true);

	majorLayout();
	setup();
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
    }

    /**
     * Creates the GUI.
     */
    public void majorLayout()
    {
	//
	// Setup Menu 
	//
	JMenuBar menuBar = new JMenuBar();
	JMenu file = new JMenu(ResourceHandler.getMessage("menu.file"));
        file.setMnemonic(ResourceHandler.getAcceleratorKey("menu.file"));
        
	file.add(exitMenuItem = new JMenuItem(ResourceHandler.getMessage("menu.exit")));
        exitMenuItem.setMnemonic(ResourceHandler.getAcceleratorKey("menu.exit"));
	exitMenuItem.addActionListener(this);
	menuBar.add(file);

	JMenu edit = new JMenu(ResourceHandler.getMessage("menu.edit"));
        edit.setMnemonic(ResourceHandler.getAcceleratorKey("menu.edit"));
        
	edit.add(optionMenuItem = new JMenuItem(ResourceHandler.getMessage("menu.option")));
        optionMenuItem.setMnemonic(ResourceHandler.getAcceleratorKey("menu.option"));
	optionMenuItem.addActionListener(this);
	menuBar.add(edit);

	JMenu help = new JMenu(ResourceHandler.getMessage("menu.help"));
        help.setMnemonic(ResourceHandler.getAcceleratorKey("menu.help"));
        
	help.add(helpMenuItem = new JMenuItem(ResourceHandler.getMessage("menu.help")));
        helpMenuItem.setMnemonic(ResourceHandler.getAcceleratorKey("menu.help"));
        
	help.add(new JSeparator());
	help.add(aboutMenuItem = new JMenuItem(ResourceHandler.getMessage("menu.about")));
        aboutMenuItem.setMnemonic(ResourceHandler.getAcceleratorKey("menu.about"));
	helpMenuItem.addActionListener(this);
	aboutMenuItem.addActionListener(this);
	menuBar.add(help);

	setJMenuBar(menuBar);


	//
	// Setup main GUI
	//

	dirLabel = new JLabel(ResourceHandler.getMessage("converter_gui.lablel0"));
	dirTF = new JTextField();
	dirBttn = new JButton(ResourceHandler.getMessage("button.browse.dir"));
        dirBttn.setMnemonic(ResourceHandler.getAcceleratorKey("button.browse.dir"));
        
	matchingLabel = new JLabel(ResourceHandler.getMessage("converter_gui.lablel1"));
	matchingTF = new JTextField(ResourceHandler.getMessage("converter_gui.lablel2"));
	recursiveCheckBox = new JCheckBox(ResourceHandler.getMessage("converter_gui.lablel3"));
        recursiveCheckBox.setMnemonic(ResourceHandler.getAcceleratorKey("converter_gui.lablel3"));
        
	backupLabel = new JLabel(ResourceHandler.getMessage("converter_gui.lablel5"));
	backupTF = new JTextField();
	backupBttn = new JButton(ResourceHandler.getMessage("button.browse.backup"));
        backupBttn.setMnemonic(ResourceHandler.getAcceleratorKey("button.browse.backup"));

	templateLabel = new JLabel(ResourceHandler.getMessage("converter_gui.lablel7"));
	templateCh = new TemplateFileChoice();

        staticVersioningLabel = new JLabel(ResourceHandler.getMessage("static.versioning.label"));
        String version = System.getProperty("java.version");
        if ( version.indexOf("-") > 0 ) {
           version = version.substring(0,version.indexOf("-"));
        }
	int dotIndex = version.indexOf(".");
	dotIndex = version.indexOf(".",dotIndex+1);
	String familyVersion = version.substring(0,dotIndex);

        MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("static.versioning.radio.button"));
        staticVersioningRadioButton = new JRadioButton(formatter.format( new Object[] {version}));
        staticVersioningRadioButton.setMnemonic(ResourceHandler.getAcceleratorKey("static.versioning.radio.button"));
        
        formatter = new MessageFormat(ResourceHandler.getMessage("dynamic.versioning.radio.button"));
        dynamicVersioningRadioButton = new JRadioButton(formatter.format(new Object[] { familyVersion }));
        dynamicVersioningRadioButton.setMnemonic(ResourceHandler.getAcceleratorKey("dynamic.versioning.radio.button"));

	staticVersioningTextArea = new JTextArea(ResourceHandler.getMessage("static.versioning.text"));
        
        formatter = new MessageFormat(ResourceHandler.getMessage("dynamic.versioning.text"));
        dynamicVersioningTextArea = new JTextArea(formatter.format( new Object [] { familyVersion} ));
        
	ButtonGroup versioningButtonGroup = new ButtonGroup();
	versioningButtonGroup.add(staticVersioningRadioButton);
	versioningButtonGroup.add(dynamicVersioningRadioButton);

	runBttn = new JButton(ResourceHandler.getMessage("button.convert"));
        runBttn.setMnemonic(ResourceHandler.getAcceleratorKey("button.convert"));

	recursiveCheckBox.setOpaque(false);
	staticVersioningRadioButton.setOpaque(false);
	dynamicVersioningRadioButton.setOpaque(false);

        staticVersioningTextArea.setEditable(false);
        staticVersioningTextArea.setLineWrap(true);
        staticVersioningTextArea.setWrapStyleWord(true);
        dynamicVersioningTextArea.setEditable(false);
        dynamicVersioningTextArea.setLineWrap(true);
        dynamicVersioningTextArea.setWrapStyleWord(true);

        staticVersioningPanel.setLayout(new BorderLayout());
        staticVersioningPanel.add(staticVersioningTextArea, "Center");
        staticVersioningPanel.setBorder(new LineBorder(Color.black));

        dynamicVersioningPanel.setLayout(new BorderLayout());
        dynamicVersioningPanel.add(dynamicVersioningTextArea, "Center");
        dynamicVersioningPanel.setBorder(new LineBorder(Color.black));

	if ( converter.isStaticVersioning() ) {
	    staticVersioningRadioButton.setSelected(true);
	}
	else {
	    dynamicVersioningRadioButton.setSelected(true);
	}

	addListeners();

	    final int buf=10,           // Buffer (between components and form)
		      sp=10,            // Space between components
		      vsp=5,            // Vertical space
		      indent=20;        // Indent between form (left edge) and component


	GridBagConstraints gbc = new GridBagConstraints();
	GridBagLayout gbl = new GridBagLayout();
	getContentPane().setLayout(gbl);

	//
	// Setup top panel
	//
	GridBagLayout topLayout = new GridBagLayout();
	JPanel topPanel = new JPanel();
	topPanel.setOpaque(false);
	topPanel.setLayout(topLayout);

	topLayout.setConstraints(dirLabel,
                                 new GridBagConstraints(0, 0, 1, 1, 0, 0, GridBagConstraints.WEST, 
                                                        GridBagConstraints.NONE, new Insets(10,0,0,0), 0, 0));
	topLayout.setConstraints(dirTF,
                                 new GridBagConstraints(1, 0, 1, 1, 1, 0, GridBagConstraints.WEST, 
                                                        GridBagConstraints.HORIZONTAL, new Insets(vsp,2,0,0), 0, 0));
	topLayout.setConstraints(dirBttn,
                                 new GridBagConstraints(2, 0, 1, 1, 0, 0, GridBagConstraints.CENTER, 
                                                        GridBagConstraints.NONE, new Insets(10,sp,0,0), 0,0));
	topLayout.setConstraints(matchingLabel,
                                 new GridBagConstraints(0, 1, 1, 1, 0, 0, GridBagConstraints.EAST, 
                                                        GridBagConstraints.NONE, new Insets(10,0,0,0), 0,0));
	topLayout.setConstraints(matchingTF,
                                 new GridBagConstraints(1, 1, 1, 1, 1, 0, GridBagConstraints.WEST, 
                                                        GridBagConstraints.HORIZONTAL, new Insets(vsp,2,0,0), 0, 0));
	topLayout.setConstraints(recursiveCheckBox,
                                 new GridBagConstraints(2, 1, GridBagConstraints.REMAINDER, 1, 1, 0, 
                                                        GridBagConstraints.WEST, GridBagConstraints.NONE,
                                                        new Insets(vsp,10,0,0), 0,0));
	topLayout.setConstraints(backupLabel,
                                 new GridBagConstraints(0, 3, 1, 1, 0, 0, GridBagConstraints.EAST, 
                                                        GridBagConstraints.NONE, new Insets(10,0,0,0), 0, 0));
	topLayout.setConstraints(backupTF,
                                 new GridBagConstraints(1, 3, 1, 1, 1, 0, GridBagConstraints.CENTER, 
                                                        GridBagConstraints.HORIZONTAL, new Insets(vsp,2,0,0), 0, 0));
	topLayout.setConstraints(backupBttn,
                                 new GridBagConstraints(2, 3, 1, 1, 0, 0, GridBagConstraints.CENTER, 
                                                        GridBagConstraints.NONE, new Insets(10,sp,0,0), 0,0));
	topLayout.setConstraints(templateLabel,
                                 new GridBagConstraints(0, 4, 1, 1, 0, 0, GridBagConstraints.EAST, 
                                                        GridBagConstraints.NONE, new Insets(10,0,0,0), 0, 0));
	topLayout.setConstraints(templateCh,
                                 new GridBagConstraints(1, 4, 1, 1, 0, 0, GridBagConstraints.WEST, 
                                                        GridBagConstraints.HORIZONTAL, new Insets(vsp,2,0,0), 0, 0));
	topLayout.setConstraints(sep1,
                                 new GridBagConstraints(0, 5, GridBagConstraints.REMAINDER, 1, 1, 0, 
                                                        GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                                                        new Insets(10,10,10,10), 0, 0));
	topLayout.setConstraints(staticVersioningLabel,
                                 new GridBagConstraints(0, 6, 1, 1, 0, 0, GridBagConstraints.WEST, 
                                                        GridBagConstraints.NONE, new Insets(10,0,0,0), 0, 0));
	topLayout.setConstraints(staticVersioningRadioButton,
                                 new GridBagConstraints(0, 7, GridBagConstraints.REMAINDER, 1, 1, 0, 
                                                        GridBagConstraints.WEST, GridBagConstraints.NONE, 
                                                        new Insets(vsp,10,0,0), 0,0));
	topLayout.setConstraints(staticVersioningPanel,
                                 new GridBagConstraints(0, 8, GridBagConstraints.REMAINDER, 1, 0, 0, 
                                                        GridBagConstraints.WEST, GridBagConstraints.BOTH, 
                                                        new Insets(vsp,25,10,0), 0,0));
	topLayout.setConstraints(dynamicVersioningRadioButton,
                                 new GridBagConstraints(0, 9, GridBagConstraints.REMAINDER, 1, 1, 0, 
                                                        GridBagConstraints.WEST, GridBagConstraints.NONE, 
                                                        new Insets(vsp,10,0,0), 0,0));
	topLayout.setConstraints(dynamicVersioningPanel,
                                 new GridBagConstraints(0, 10, GridBagConstraints.REMAINDER, 1, 0, 0, 
                                                        GridBagConstraints.WEST, GridBagConstraints.BOTH, 
                                                        new Insets(vsp,25,0,0), 0,0));
	topLayout.setConstraints(sep2,
                                 new GridBagConstraints(0, 11, GridBagConstraints.REMAINDER, 1, 1, 0, 
                                                        GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
                                                        new Insets(10,10,0,10), 0, 0));

	invisibleBttn = new JButton();
	invisibleBttn.setVisible(false);
	topLayout.setConstraints(invisibleBttn, 
                                 new GridBagConstraints(2, 6, 1, 1, 0, 0, GridBagConstraints.CENTER, 
                                                        GridBagConstraints.CENTER, new Insets(indent,sp,0,0), 0,0));

	topPanel.add(dirLabel);
	topPanel.add(dirTF);
	topPanel.add(dirBttn);
	topPanel.add(matchingLabel);
	topPanel.add(matchingTF);
	topPanel.add(recursiveCheckBox);
	topPanel.add(backupLabel);
	topPanel.add(backupTF);
	topPanel.add(backupBttn);
	topPanel.add(templateLabel);
	topPanel.add(templateCh);
	topPanel.add(sep1);
        topPanel.add(staticVersioningLabel);
        topPanel.add(staticVersioningRadioButton);
        topPanel.add(staticVersioningPanel);
        topPanel.add(dynamicVersioningRadioButton);
        topPanel.add(dynamicVersioningPanel);
	topPanel.add(sep2);
	topPanel.add(invisibleBttn);


	// 
	// Setup bottom panel
	//
	GridBagLayout buttomLayout = new GridBagLayout();
	JPanel buttomPanel = new JPanel();
	buttomPanel.setOpaque(false);
	buttomPanel.setLayout(buttomLayout);
	
	buttomLayout.setConstraints(runBttn, new GridBagConstraints(3, 0, 1, 1, 0, 0, GridBagConstraints.CENTER, 
					GridBagConstraints.NONE, new Insets(sp,0,0,0), 0, 0));
	buttomPanel.add(runBttn);	



	//
	// Setup main panel
	//
	GridBagLayout mainLayout = new GridBagLayout();
	JPanel mainPanel = new JPanel();

	mainPanel.setOpaque(false);
	mainPanel.setLayout(mainLayout);
	
	mainLayout.setConstraints(topPanel, new GridBagConstraints(0, 0, 1, 1, 1, 0, GridBagConstraints.CENTER, 
					GridBagConstraints.HORIZONTAL, new Insets(buf,buf,0,buf), 0, 0));
	mainLayout.setConstraints(buttomPanel, new GridBagConstraints(0, 1, 1, 1, 1, 1, GridBagConstraints.SOUTH, 
					GridBagConstraints.HORIZONTAL, new Insets(0,buf,buf,buf), 0, 0));
	mainPanel.add(topPanel);	
	mainPanel.add(buttomPanel);	


	Border border = BorderFactory.createEtchedBorder();
	mainPanel.setBorder(border);


	GridBagLayout layout = new GridBagLayout();
	getContentPane().setLayout(layout);
	    
	layout.setConstraints(mainPanel, 
			      new GridBagConstraints(0, 0, 1, 1, 
						     1, 1, GridBagConstraints.CENTER, GridBagConstraints.BOTH, 
						     new Insets(0,0,0,0), 0, 0));

	getContentPane().add(mainPanel);

        pack();
        setResizable(false);
    }


    /**
     * Used to initialize the componenets.  Also called after every event that
     * is required to update other fields.
     */
    public void setup()
    {
	if (firstTime)
	{
	    defaultDirBackupPath = convertSet.getBackupPath().getPath();
	    defaultOneFileBackupPath = System.getProperty("user.dir");
	    firstTime = false;
	}

	dirTF.setText(convertSet.getSourcePath().getPath());
	recursiveCheckBox.setSelected(converter.isRecurse());
	if ( converter.isStaticVersioning() ) {
	    staticVersioningRadioButton.setSelected(true);
	}
	else {
	    dynamicVersioningRadioButton.setSelected(true);
	}	    
	backupTF.setText(defaultDirBackupPath);
    }

    /**
     * Reset to defaults.
     */
    private void resetDefaults()
    {
	firstTime        = true;

	setup();
    }

    /**
     * Add listeners to components.
     */
    private void addListeners()
    {
	dirBttn.addActionListener(this);
	backupBttn.addActionListener(this);
	runBttn.addActionListener(this);

	recursiveCheckBox.addItemListener(this);
	templateCh.addItemListener(this);

	dirTF.addActionListener(this);

        staticVersioningRadioButton.addItemListener(this);
        dynamicVersioningRadioButton.addItemListener(this);

	addWindowListener(new WindowAdapter()
	{
	    public void windowClosing(WindowEvent e)
	    {
	        quit();
	    }
	});
    }

    /**
     * Handle ActionEvents.
     */
    public void actionPerformed(ActionEvent e)
    {
	Component target = (Component) e.getSource();

	if (target == dirBttn)
	{
	    JFileChooser chooser = new JFileChooser(); 
	    
	    //Gabe Boys: Changed form just dirs to files and dirs 
	    chooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES); 

	    try
	    {
		chooser.setCurrentDirectory(new File(dirTF.getText()));
	    }
	    catch (Exception ex)
	    {
	    }

	    int returnVal = chooser.showOpenDialog(this); 
	    if(returnVal == JFileChooser.APPROVE_OPTION) 
	    { 
    		dirTF.setText(chooser.getSelectedFile().getAbsolutePath());
    		
		//Gabe Boys : If the source is a file set the backupPath to the directory instead of the file
		if(!chooser.getSelectedFile().isDirectory())
		{
		    backupTF.setText(chooser.getSelectedFile().getParentFile().getAbsolutePath() + "_BAK");
		}
		else 
		{
		    backupTF.setText(chooser.getSelectedFile().getAbsolutePath() + "_BAK");
		}
	    } 
	}
	else if (target == backupBttn)
	{
	    JFileChooser chooser = new JFileChooser(); 
	    chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY); 

	    try  {
		chooser.setCurrentDirectory(new File(backupTF.getText()));
	    } catch (Exception ex)  {
	    }

	    int returnVal = chooser.showOpenDialog(this); 
	    if(returnVal == JFileChooser.APPROVE_OPTION) 
	    { 
		backupTF.setText(chooser.getSelectedFile().getAbsolutePath());
	    } 
	}
	else if (target == optionMenuItem)
	{
	    showOptionDialog();
	}
	else if (target == helpMenuItem)
	{
	    showHelpDialog();
	}
	else if (target == runBttn)
	{
	    boolean noExeceptionThrown = true;
	    noExeceptionThrown = setDirectory();

	    if(noExeceptionThrown == true)
	    {
		if (!testSourceVsBackup())
		{
		    // Set backup 
		    convertSet.setBackupPath(new File(backupTF.getText()));

    		    noExeceptionThrown = (setTemplateFile((String)templateCh.getSelectedItem()) && noExeceptionThrown);

    		    if (noExeceptionThrown)
    			(new ProgressGUI(converter)).setVisible(true);
		}
	    }	
	}
	else 
	if (target == exitMenuItem)
	{
	    converter.persistConverterSetting();
	    quit();
	}
	else 
	if (target == aboutMenuItem)
	{
	    showAboutDialog();
	}
    }

    /**
     * Handle ItemEvents.
     */
    public void itemStateChanged(ItemEvent e) {
	
        final String dialog_title = ResourceHandler.getMessage("template_dialog.title");
	
        Component target = (Component) e.getSource();

	if (target == recursiveCheckBox) {
	    converter.setRecurse(recursiveCheckBox.isSelected());
	}
        else if (target == staticVersioningRadioButton || target == dynamicVersioningRadioButton) {
            converter.setStaticVersioning(staticVersioningRadioButton.isSelected());
        }
	else if (target == templateCh && (e.getStateChange() == e.SELECTED)) {//Process only when item is Selected

	    // Get the current template selection
	    String choiceStr = (String)templateCh.getSelectedItem();


	    // If the user chooses 'other', display a file dialog to allow
	    // them to select a template file.
	    if (choiceStr.equals(TemplateFileChoice.OTHER_STR)) {
		String templatePath = null;
		FileDialog fd = new FileDialog(this, dialog_title, FileDialog.LOAD);
		fd.show();

		// Capture the path entered, if any.
		if (fd.getDirectory() != null && fd.getFile() != null) {
		    templatePath = fd.getDirectory() + fd.getFile();
		}

		// If the template file is valid add it and select it.
		if ( templatePath != null && setTemplateFile(templatePath) ) {
		    if (!templateCh.testIfInList(templatePath)) {
			templateCh.addItem(templatePath);
		    }
		    templateCh.select(templatePath);
		}
		else {
		    templateCh.select(templateCh.getPreviousSelection());
		}    
		fd.dispose();
	    }
	    else {
		templateCh.select(choiceStr);
	    }
	}
    }

    /**
     * Pop-up a Dialog if the source and backup fields are the same.
     * Returns true if the source and backup remain the same.
     */
    private boolean testSourceVsBackup() 
    { 
    if (dirTF.getText().equals(backupTF.getText()))
    {
	final String caption = ResourceHandler.getMessage("caption.warning");
        
        MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("warning_dialog.info"));
        final String info = formatter.format(new Object [] {backupTF.getText()});
        
	int n = JOptionPane.showConfirmDialog(this, info, caption, JOptionPane.WARNING_MESSAGE);

        if (n == JOptionPane.YES_OPTION)
        {
            backupTF.setText(backupTF.getText() + "_BAK");

            return false;
        }
        else if (n == JOptionPane.NO_OPTION)
            return true;
    }

    return false;
    }

    /**
     * Returns false if Exception is thrown.
     */
    private boolean setDirectory()
    {
	String pathStr = dirTF.getText().trim();
	if (pathStr.equals(""))
	    pathStr = System.getProperty("user.dir");
	try
	{
	    File dirPath = new File(pathStr);
	    if(!dirPath.isDirectory())
	    {
		if(!dirPath.exists())
		{
		    if(recursiveCheckBox.isSelected())
			throw new NotDirectoryException(dirPath.getAbsolutePath());
		    else
			throw new NotFileException(dirPath.getAbsolutePath());
		}
		else
		{
		    convertSet.setFile(dirPath);
		    convertSet.setDestinationPath(dirPath.getParentFile());
		}
	    }
	    else
	    {
		//Set the descriptors
		setMatchingFileNames();

		FlexFilter flexFilter = new FlexFilter();
		flexFilter.addDescriptors(descriptors);
		flexFilter.setFilesOnly(!recursiveCheckBox.isSelected());

		convertSet.setSourcePath(dirPath, flexFilter);
		convertSet.setDestinationPath(dirPath);
	    }
	}
	catch (NotDirectoryException e1)
	{
	    final String caption = ResourceHandler.getMessage("notdirectory_dialog.caption1");
            
            MessageFormat formatter;
            String info_msg;
            if (pathStr.equals("")){
                info_msg = ResourceHandler.getMessage("notdirectory_dialog.info5");                
            }
            else {
                formatter = new MessageFormat(ResourceHandler.getMessage("notdirectory_dialog.info0")); 
                info_msg = formatter.format(new Object [] { pathStr });
            }
            final String info = info_msg;
            
	    JOptionPane.showMessageDialog(this, info, caption, JOptionPane.ERROR_MESSAGE);
	    return false;
	}
	catch (NotFileException e2)
	{
	    final String caption = ResourceHandler.getMessage("notdirectory_dialog.caption0");
            
            MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("notdirectory_dialog.info1"));
            final String info = formatter.format(new Object [] {pathStr});
            
	    JOptionPane.showMessageDialog(this, info, caption, JOptionPane.ERROR_MESSAGE);
	    return false;
	}

	return true; // no exception thrown
    }

    /**
     * Set the template file path.
     */
    private boolean setTemplateFile(String pathStr)
    {
	File templateFile = new File(templateCh.getSelectedPath(pathStr));

	if (!templateFile.getName().toLowerCase().endsWith(".tpl"))
	{
	    final String templateCaption = ResourceHandler.getMessage("nottemplatefile_dialog.caption");
            
            MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("nottemplatefile_dialog.info0"));
            final String errorMessage = formatter.format(new Object [] {templateFile.getName()});
            
	    String defaultTemplateName = PluginConverter.getDefaultTemplateFileName();
	    JOptionPane.showMessageDialog(this, errorMessage, templateCaption, JOptionPane.ERROR_MESSAGE);
            
	    templateCh.select(templateCh.getPreviousSelection());
	    return false;
	}

	try
	{
	    converter.setTemplateFilePath(templateCh.getSelectedPath(pathStr));
	}
	catch (FileNotFoundException e)
	{
		// TO-DO:  found it, but it's not a file.
		// TO-DO:  Throw up a Dialog -- "Not a valid file, resetting to default file"

	    final String caption = ResourceHandler.getMessage("notdirectory_dialog.caption0");
            
            MessageFormat formatter;
            String info_msg;
            if (pathStr.equals("")){
                info_msg = ResourceHandler.getMessage("notdirectory_dialog.info5");                
            }
            else {
                formatter = new MessageFormat(ResourceHandler.getMessage("notdirectory_dialog.info1")); 
                info_msg = formatter.format(new Object [] { pathStr });
            }
            final String info = info_msg;
	
            JOptionPane.showMessageDialog(this, info, caption, JOptionPane.ERROR_MESSAGE);
	    return false;
	}

	return true;
    }

    /**
     * Set the matching descriptors.
     */
    private void setMatchingFileNames()
    {
	StringTokenizer st = new StringTokenizer(matchingTF.getText(), ",");

	int numTokens = st.countTokens();

	if (numTokens == 0)
	{
	    descriptors = new String[1];
	    descriptors[0] = "*";
	    return;
	}

	descriptors = new String[numTokens];

	int i=0;
	while (st.hasMoreElements())
	{
	    descriptors[i++] = st.nextToken().trim();
	}
    }

    /**
     * Accessor to the converter.
     * @return PluginConverter
     */
    public static PluginConverter getPluginConverter()
    {
	return converter;
    }

    /**
     * Hide, reclaim resources, and exit.
     */
    protected void quit()
    {
	setVisible(false);
	dispose();
	System.exit(0);
    }

    /**
     * Displayed when the template file is not found in the classpath or
     * the working directory.
     */
    private void showNoTemplateDialog()
    {
	final String caption = ResourceHandler.getMessage("notemplate_dialog.caption");
        MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("notemplate_dialog.info"));
        final String info = formatter.format(new Object [] { PluginConverter.getDefaultTemplateFileName() });
        
	JOptionPane.showMessageDialog(this, info, caption, JOptionPane.ERROR_MESSAGE);

	System.exit(0);
    }

    private void showAboutDialog()
    {
	final String aboutCaption = ResourceHandler.getMessage("about_dialog.caption");

	// Version string
	final String version = System.getProperty("java.version");

        
	MessageFormat formatter = new MessageFormat(ResourceHandler.getMessage("about_dialog.info"));
        final String aboutInfo = formatter.format(new Object[] { version } );


	JOptionPane.showMessageDialog(this, aboutInfo, aboutCaption, JOptionPane.INFORMATION_MESSAGE);
    }

    private void showHelpDialog()
    {
	HelpDialog dialog = new HelpDialog(this, true);
	dialog.setVisible(true);
    }

    private void showOptionDialog()
    {
	AdvancedDialog dialog = new AdvancedDialog(this, true, converter);
	dialog.setVisible(true);
    }   

    /**
     * TESTING...
     */
    public static void main(String[] argv)
    {
	(new ConverterGUI()).setVisible(true);
    }
}


// ---------------------------------------------------------------------------------------- //
class TemplateFileChoice extends JComboBox
{
    public static final int DEFAULT   = 0,
                            EXTEND    = 1,
                            IE        = 2,
                            NS        = 3,
                            SEPARATOR = 4,
                            OTHER     = 5;

    public static final String DEFAULT_STR   = ResourceHandler.getMessage("template.default"),
                               EXTEND_STR    = ResourceHandler.getMessage("template.extend"),
                               IE_STR        = ResourceHandler.getMessage("template.ieonly"),
                               NS_STR        = ResourceHandler.getMessage("template.nsonly"),
                               SEPARATOR_STR = "-----------------------------------------------------------------------",
                               OTHER_STR     = ResourceHandler.getMessage("template.other");

    public static final String DEFAULT_PATH = "templates/default.tpl",
                               EXTEND_PATH  = "templates/extend.tpl",
                               IE_PATH      = "templates/ieonly.tpl",
                               NS_PATH      = "templates/nsonly.tpl";

    private String previousSelection = DEFAULT_STR;

    private PluginConverter converter = ConverterGUI.getPluginConverter();

    public TemplateFileChoice() {
        super();

        // Define and set our JComboBox model
        String[] modelStrings = {
            DEFAULT_STR, EXTEND_STR, IE_STR, NS_STR, SEPARATOR_STR, OTHER_STR 
        };
        setModel(new DefaultComboBoxModel(modelStrings));
    }

    public void select(String item)
    {
        if (item.equals(SEPARATOR_STR))
            item = previousSelection;
        else
            previousSelection = item;

        setSelectedItem(item);
    }

    public String getPreviousSelection()
    {
        return previousSelection;
    }

    public String getSelectedPath(String item)
    {
        String path = "", fileSep = System.getProperty("file.separator");

/*
        try
        {
            path = converter.getDefaultTemplateFilePath().getParent() + fileSep;
        }
        catch (FileNotFoundException e)
        {
	    final String caption = ResourceHandler.getMessage("notemplate_dialog.caption");
	    final String info = ResourceHandler.getMessage("notemplate_dialog.info0") 
				+ PluginConverter.getDefaultTemplateFileName()
				+ ResourceHandler.getMessage("notemplate_dialog.info1");

	    JOptionPane.showMessageDialog(this, info, caption, JOptionPane.ERROR_MESSAGE);

            System.exit(0);
        }
*/
        if (item.equals(DEFAULT_STR))
            return path + DEFAULT_PATH;
        else if (item.equals(EXTEND_STR))
            return path + EXTEND_PATH;
        else if (item.equals(IE_STR))
            return path + IE_PATH;
    	else if (item.equals(NS_STR))
    	    return path + NS_PATH;
    	else
    	    return item;
    }

    public boolean testIfInList(String item)
    {
        for (int i=0; i < getItemCount(); i++)
        {
            if (getItemAt(i).equals(item))
                return true;
        }

        return false;
    }    
}
