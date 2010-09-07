/*
 * @(#)ControlPanel.java	1.60 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.util.*;
import com.sun.deploy.config.*;
import com.sun.deploy.util.*;
import com.sun.deploy.si.*;
import com.sun.deploy.resources.ResourceManager;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;
import com.sun.deploy.Environment;
import com.sun.deploy.ui.DialogTemplate;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.services.PlatformType;
import sun.jkernel.DownloadManager;
import sun.jkernel.Mutex;


public class ControlPanel extends JFrame 
	     implements DeploySIListener, PropertyChangeListener
{
    static
    {
        Environment.setEnvironmentType(Environment.ENV_JCP);
        com.sun.deploy.util.DeployUIManager.setLookAndFeel();
    }
    
    /** Creates new form ControlPanel */
    public ControlPanel() {

        // initialize single instance listener
        _sil = new SingleInstanceImpl(); 
        _sil.addSingleInstanceListener(this, JCP_ID);

        // initialize the components in Control Panel
        initComponents();
	
	Config.setControlPanel(this);

        // Refresh the notion of the available JREs
	Vector list = Config.getInstance().getInstalledJREList();
	if (list != null) {
	    Config.storeInstalledJREList(list);
	}        

        // first initialization may set some properties - apply them
        // Do not show any browser settings related dialogs on open action
        // for Control Panel. This is to work around the browser settings
        // changed dialog appearance on control panel start up.
        Config.applyButtonAction = false;
        
	apply();
        
        // Set action to true.
        Config.applyButtonAction = true;

    }
    
    /** This method is called from within the constructor to
     * initialize the form.     
     */
    private void initComponents() {

        tabbedPane = new JTabbedPane();

        securityPanel = new SecurityPanel(); 
        javaPanel = new JavaPanel();  
        generalPanel = new GeneralPanel();

	// Update tab is added only on Windows and if Java Update is enabled
	if (System.getProperty("os.name").indexOf("Windows") != -1)
	{
	    UpdatePanel up = new UpdatePanel();

	    if (up.isJavaUpdateEnabled())
		updatePanel = up;    
	} 

                    
        // Special registry settings for APPLET tag handling.
        // This has to be done before AdvancedPanel is created.
        Config.getBrowserSettings();
        
        // Special settings for JQS checkbox.
        // This has to be done before AdvancedPanel is created.
        Config.getJqsSettings();

	// Special settings for Java Plug-in mode - use old or new plugin
	Config.getJavaPluginSettings();

        advancedPanel = new AdvancedPanel() {
	    // don't let his preferred size mater ...
	    public Dimension getPreferredSize() {
		return new Dimension(0,0);
	    }
	};
        
        setTitle(getMessage("control.panel.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                exitForm();
            }
        });
        
        tabbedPane.setName(getMessage("control.panel.general"));
        
	// General tab first. 
        tabbedPane.addTab(getMessage("control.panel.general"), generalPanel);   

	// Update tab second.  On Windows only.
 	  if (updatePanel != null)
	      tabbedPane.addTab(getMessage("control.panel.update"), updatePanel);
       
	// Java tab next.
	  tabbedPane.addTab(getMessage("control.panel.java"), javaPanel);
        
	// Security tab.
        tabbedPane.addTab(getMessage("control.panel.security"), securityPanel);        

       
        getContentPane().add(tabbedPane, BorderLayout.CENTER);
        
        decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        
        okButton = new JButton(getMessage("common.ok_btn"));
        okButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okBtnActionPerformed(evt);
            }
        });  
        okButton.setToolTipText(getMessage("cpl.ok_btn.tooltip"));      
        decisionPanel.add(okButton);

        cancelButton = new JButton(getMessage("common.cancel_btn"));
        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelBtnActionPerformed(evt);
            }
        };
        cancelButton.addActionListener(cancelAction);

        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        cancelButton.setToolTipText(getMessage("cpl.cancel_btn.tooltip"));
        decisionPanel.add(cancelButton);
                
        applyButton = makeButton("common.apply_btn");
        applyButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                applyBtnActionPerformed(evt);
            }
        });  
        applyButton.setToolTipText(getMessage("cpl.apply_btn.tooltip"));
        decisionPanel.add(applyButton);

        JButton [] btns = {okButton, cancelButton, applyButton};
        DialogTemplate.resizeButtons( btns );     
        
        getContentPane().add(decisionPanel, BorderLayout.SOUTH);
        
        getContentPane().invalidate();
        
        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(okButton);        
        

	// Advanced tab is last.
        tabbedPane.addTab(getMessage("control.panel.advanced"), advancedPanel);
        applyButton.setEnabled(false);

	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
	        resetBounds();
	    }
	});

	tabbedPane.addPropertyChangeListener(this);
	checkPreferredSizes(tabbedPane);
    }

    /*
     * PropertyChangeListener implementation
     */
    public void propertyChange(PropertyChangeEvent e) {
	if (e.getPropertyName().equals("font")) {
	    SwingUtilities.invokeLater(new Runnable() {
	        public void run() {
	            resetBounds();
	        }
	    });
	}
    }

    /*
     * resetBounds() - initially set up the bounds,
     * we may need to reset when font sizes changes in theme
     */
    public void resetBounds() {
	pack();

	Dimension dim = getPreferredSize();
	Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();
	dim.width = Math.min(dim.width, screen.width);
	dim.height = Math.min(dim.height, screen.height);
	if (dim.width > 440) {
	    int area = dim.width * dim.height;
	    dim.width = 440;
	    dim.height = (area / 440) + 1;
	}

	setSize(dim.width, dim.height);
        UIFactory.placeWindow(this);
	setResizable(false);
    }
    
        
    /*
     * When "OK" button is clicked, the changes should be saved
     * in the deployment.properties file and Control Panel should
     * be dismissed.
     */
    private void okBtnActionPerformed(ActionEvent evt) {
	apply();
	exitForm();
    }
    
    /*
     * When "Apply" button is clicked, the changes should be
     * saved in the deployment.properties file.
     */
    private void applyBtnActionPerformed(ActionEvent evt) {
	apply();
        enableApplyButton(false);
    }

    private void apply() {
	// reset successDialogShown flag
	Config.successDialogShown = false;

        // Save update changes.
        if ( System.getProperty("os.name").indexOf("Windows") != -1 &&
             updatePanel != null ){
            updatePanel.saveUpdateSettingsInReg();
        }
        
        // Store properties in config
        Config.store();
        
	// Apply Java Plug-in selection
	int result = Config.setJavaPluginSettings(new Boolean(
	    Config.getProperty("deployment.jpi.mode.new")).booleanValue());
	
	// check result here and popup dialog if needed
	if (result == 1) {
	    Config.showJPISettingsErrorDialog();
	    // prevent the "Browser Settings changed" success dialog from being shown
	    Config.successDialogShown = true;
	} else {
	    // result of 2 means there's no change in the Java Plug-in selections
	    // if that's the case, no need to popup a success dialog
	    if (result != 2) {
		Config.showJPISettingsSuccessDialog();
	    }
	}

        // Apply APPLET tag handling for browsers.
        Config.setBrowserSettings();
        
        // Apply Java Quick Starter changes to the service
        Config.setJqsSettings(new Boolean(
                Config.getProperty("java.quick.starter")).booleanValue());
    }

    
    /*
     * When "Cancel" button is clicked, all changes that user
     * made this far should be cancelled, and Control Panel dialog
     * should be dismissed.
     */
    private void cancelBtnActionPerformed(ActionEvent evt) {
	exitForm();
    }

    
    /** Exit the Application */
    private void exitForm() {
	_sil.removeSingleInstanceListener(this);
        System.exit(0);
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(final String args[]) {

        boolean isWindows = (Config.getOSName().indexOf("Windows") != -1);
        Mutex mutex = null;
 
	// Setup services
        if (isWindows) {
            com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_TIGER_WIN32);
            if (!DownloadManager.isJREComplete()) {
                mutex = Mutex.create("com.sun.deploy.panel.ControlPanelMutex");
                mutex.acquire();
            }
        } else {
            com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_TIGER_UNIX);
        }
        
	// first check for existing instance
	String outputString = "";
	for (int i=0; i<args.length; i++) {
	    outputString += ((i > 0) ? " " : "") + args[i];
	}
	if (SingleInstanceManager.isServerRunning(JCP_ID)) {
	    if (SingleInstanceManager.connectToServer(outputString)) {
                if (mutex != null) {
                    mutex.release();
                    mutex = null;
                }
                System.exit(0);
            }
	}

	initTrace();
	
        final ControlPanel cpl = new ControlPanel();        
	if (args.length == 2) {
	    if (args[0].equals("-tab")) {
	        int index = getTabIndex(args[1], cpl);
		cpl.tabbedPane.setSelectedIndex(index);
	    }
	} 
	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
	        cpl.resetBounds();
		cpl.setVisible(true);
		if (args.length == 1 && args[0].equals("-viewer")) {
		    if (cpl.generalPanel.cacheViewBtn.isEnabled()) {
		        cpl.generalPanel.viewBtnAction();
		    }
		}
	    }
	});
        if (mutex != null) {
            mutex.release();
            mutex = null;
        }
    }

    public void newActivation(String[] params) {
	if (params.length > 0) {
	    String param = params[0];
	    if (param.startsWith("-tab")) {
		int index = getTabIndex(param.substring(4), this);
		this.tabbedPane.setSelectedIndex(index);
	    }
	}
	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
		ControlPanel.this.setExtendedState(
		  ControlPanel.this.getExtendedState() & ~Frame.ICONIFIED);
		ControlPanel.this.toFront();
	    }
	});
    }

    public Object getSingleInstanceListener() {
	return this;
    }

    private static int getTabIndex(String arg, ControlPanel cpl) {
        int tabIndex = 0;
	String tab = arg.trim();
        if (tab.equals("general")) {
            tabIndex = 0;
        } else if (tab.equals("update")) {
            if (cpl.updatePanel != null) {
                tabIndex = 1;
            }
        } else if (tab.equals("java")) {
            tabIndex = (cpl.updatePanel == null) ? 1 : 2;
        } else if (tab.equals("security")) {
            tabIndex = (cpl.updatePanel == null) ? 2 : 3;
        } else if (tab.equals("advanced")) {
            tabIndex = (cpl.updatePanel == null) ? 3 : 4;
        }
	return tabIndex;
    }

    private static void initTrace() {
	if (Config.getBooleanProperty(Config.TRACE_MODE_KEY)) {
		File dir = new File(Config.getProperty(Config.LOGDIR_KEY));

		if (dir.exists() && dir.isDirectory()) {
	            FileTraceListener ftl = new FileTraceListener(
                                      new File(dir, "jcp.trace"), false);
                    Trace.addTraceListener(ftl);
		}
		Trace.redirectStdioStderr();
		Trace.setBasicTrace(true);
		Trace.setTempTrace(true);
	}
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
    
    public static void propertyHasChanged(){
        enableApplyButton(true);
    }
    public static void propertyChanged(boolean changed) {
        enableApplyButton(changed);
    }
    
    private static void enableApplyButton(boolean enabled){
        if (applyButton != null){
            applyButton.setEnabled(enabled);
        }
        if (generalPanel != null){
            generalPanel.enableViewButton(!enabled);
        }
    }

    private static void checkPreferredSizes(Component c) {
	// some bugs are cleared up by getting the preferred size
        // of everything, also a convienient place to trace sizes
	Dimension s = c.getPreferredSize();
	//Trace.println("item: "+c.getClass().getName()+", size: "+s);
	if (c instanceof Container) {
	    Component [] kid = ((Container)c).getComponents();
	    for (int i=0; i<kid.length; checkPreferredSizes(kid[i++]));
	}
    }
                
    private final static String JCP_ID = "JavaControlPanel";

    private JTabbedPane tabbedPane;
    private static GeneralPanel generalPanel;
    private SecurityPanel securityPanel;
    private JavaPanel javaPanel;
    private UpdatePanel updatePanel = null;
    private AdvancedPanel advancedPanel;
    private SingleInstanceImpl _sil;

    private JPanel decisionPanel;
    private JButton okButton;
    private static JButton applyButton;
    private JButton cancelButton;    

}
