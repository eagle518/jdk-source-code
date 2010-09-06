/*
 * @(#)JarCacheViewer.java	1.26 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import java.text.MessageFormat;
import java.util.*;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.UserProfile;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.AboutDialog;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.si.*;

public class JarCacheViewer extends JFrame implements DeploySIListener{
    
    static
    {
        com.sun.deploy.util.DeployUIManager.setLookAndFeel();
    }
    
    private  JarCacheTable viewerTable;
    private static String cacheLocation;
    private ActionListener closeViewer, refreshViewer, removeEntry;
    private JCheckBox disableCache;
    private SingleInstanceImpl _sil;

    public JarCacheViewer(String location) {
        // initialize single instance listener
        _sil = new SingleInstanceImpl(); 
        _sil.addSingleInstanceListener(this, VIEWER_ID);

	if(location != null) {
	    cacheLocation = location;
	}else{
	    cacheLocation = getPluginCacheLocation();
	}
	Container contentPane = getContentPane();
	setTitle(ResourceHandler.getMessage("cache_viewer.caption"));	
	
	viewerTable = new JarCacheTable();
	JScrollPane pane = new JScrollPane (viewerTable);
	contentPane.add(pane, BorderLayout.CENTER);
        
	JPanel southPanel = new JPanel();
	southPanel.setLayout(new BorderLayout());
        
	JPanel checkBoxPanel = new JPanel();
	checkBoxPanel.setLayout(new BorderLayout());
	disableCache = new JCheckBox(ResourceHandler.getMessage("cache_viewer.disable"));
	disableCache.setSelected(Config.getBooleanProperty(Config.CACHE_ENABLED_KEY));
	disableCache.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.disable"));
	disableCache.addItemListener(new ItemListener() {
	        public void itemStateChanged( ItemEvent e ){
	            Config.setBooleanProperty(Config.CACHE_ENABLED_KEY, disableCache.isSelected());
                    disable_mi.setSelected(disableCache.isSelected());
	        }
	    });
                
	checkBoxPanel.add(disableCache, BorderLayout.CENTER);
        
	southPanel.add(checkBoxPanel, BorderLayout.CENTER);

	// refresh/remove/close button panel
	JPanel buttonPanel = new JPanel();
	buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));

	JButton refresh = new JButton(ResourceHandler.getMessage("cache_viewer.refresh"));
	JButton remove = new JButton(ResourceHandler.getMessage("cache_viewer.remove"));
	JButton OK = new JButton(ResourceHandler.getMessage("cache_viewer.OK"));

	refresh.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.refresh")); 
	remove.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.remove")); 
	OK.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.OK")); 

	buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	buttonPanel.add(refresh);
	buttonPanel.add(remove);
	buttonPanel.add(OK);

	southPanel.add(buttonPanel, BorderLayout.SOUTH);
	contentPane.add(southPanel, BorderLayout.SOUTH);

	this.addWindowListener (
	    new WindowAdapter() {
		public void windowClosing(WindowEvent e) {
                    exitForm();
		}
	    }
        );

	//Action listeners for refresh button
	refreshViewer = new ActionListener() 
	{
	    public void actionPerformed(ActionEvent e)  
	    {
		viewerTable.refresh();
	    }
	};

	//Action listeners for remove button
	removeEntry = new ActionListener() 
	{
	    public void actionPerformed(ActionEvent e)  
	    {
		viewerTable.removeRows();
	    }
	};

	//Action listeners for close button
	closeViewer = new ActionListener() 
	{ 
	    public void actionPerformed(ActionEvent e)  
	    {
	        Config.store();
                exitForm();
	    }
	};

	refresh.addActionListener(refreshViewer);
	remove.addActionListener(removeEntry);
	OK.addActionListener(closeViewer);

	getRootPane().addComponentListener (
	    new ComponentAdapter() {
	            public void componentResized(ComponentEvent e) {
                        viewerTable.adjustColumnSize((Container)e.getComponent());
	            }
	        }
	    );

	createMenuBar();
        
        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(OK);        

	setSize(800, 400);
	setVisible(true);
    }
    
    private void exitForm(){
        _sil.removeSingleInstanceListener(this);
	dispose();
	System.exit(0);
    }

    
    JMenu file_menu, options_menu, help_menu;
    JMenuItem exit_mi, refresh_mi, about_mi;
    JCheckBoxMenuItem disable_mi;
    
    /*
     * Create menu:
     *  File - exit
     *  Options - disable caching in Plug-in; Refresh
     *  Help - about
     */   
    private void createMenuBar(){
        file_menu = new JMenu (ResourceHandler.getMessage("cache_viewer.menu.file"));
        file_menu.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.menu.file"));        
        
        options_menu = new JMenu (ResourceHandler.getMessage("cache_viewer.menu.options"));
        options_menu.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.menu.options"));
        
        help_menu = new JMenu (ResourceHandler.getMessage("cache_viewer.menu.help"));
        help_menu.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.menu.help"));
        
        // File menu items:
        exit_mi = new JMenuItem (ResourceHandler.getMessage("cache_viewer.menu.item.exit"));
        exit_mi.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.menu.item.exit"));
        exit_mi.addActionListener(closeViewer);
        
        // Options menu items:
        disable_mi = new JCheckBoxMenuItem (ResourceHandler.getMessage("cache_viewer.disable"));
        disable_mi.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.disable"));
        disable_mi.setSelected(disableCache.isSelected());
        disable_mi.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                if ( e.getStateChange() == ItemEvent.SELECTED ){
                    disableCache.setSelected( true );
                }else{
                    disableCache.setSelected( false );
                }
            }
        });
        
        refresh_mi = new JMenuItem (ResourceHandler.getMessage("cache_viewer.refresh"));
        refresh_mi.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.refresh"));
        refresh_mi.addActionListener(refreshViewer);
        
        // Help menu items:
        about_mi = new JMenuItem(ResourceHandler.getMessage("cache_viewer.menu.item.about"));
        about_mi.setMnemonic(ResourceHandler.getAcceleratorKey("cache_viewer.menu.item.about"));
        about_mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e){
                aboutActionPerformed(e);
            }
        });
        
        file_menu.add(exit_mi);
        options_menu.add(disable_mi);
        options_menu.add(refresh_mi);
        help_menu.add(about_mi);
        
        JMenuBar mb = new JMenuBar();
	mb.add(file_menu);
	mb.add(options_menu);
	mb.add(Box.createHorizontalGlue());
	mb.add(help_menu);
        setJMenuBar(mb);
    }
    
    private void aboutActionPerformed(ActionEvent e){
        AboutDialog about = new AboutDialog((JFrame)this, true);
        about.setVisible(true);
    }

    public static String getPluginCacheLocation() {
	if(cacheLocation != null) {
	    return cacheLocation;
	}else {
	    return UserProfile.getPluginCacheDirectory();
	}
    }

    public static void main(String args[]) {
        String location = null;
        
        if ( args.length != 0 ){   
            location = args[0];
        }
        
        if (SingleInstanceManager.isServerRunning(VIEWER_ID)) {
	    if (SingleInstanceManager.connectToServer("")) {
                System.exit(0);
            }
	}
        
        JarCacheViewer viewer = new JarCacheViewer(location);   
    }
    
    /**
     * This method should be implemented by the application to
     * handle the single instance behaviour - how should the application
     * handle the arguments when another instance of the application is
     * invoked with params.
     *
     * @param[] params   Array of parameters for the application main
     *                   (arguments supplied in the jnlp file)
     *
     */
    public void newActivation(String[] params) {
	this.setExtendedState(this.getExtendedState() & ~Frame.ICONIFIED);
	this.toFront();
    }
    
    public Object getSingleInstanceListener() {
        return this;
    }
    private final static String VIEWER_ID = "JarCacheViewer" + 
	    Config.getInstance().getSessionSpecificString();
}
