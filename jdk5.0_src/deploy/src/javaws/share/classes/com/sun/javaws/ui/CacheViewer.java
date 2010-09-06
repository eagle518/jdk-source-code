/*
 * @(#)CacheViewer.java	1.33 04/04/05
 *
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.net.URL;
import java.text.MessageFormat;

import com.sun.javaws.Main;
import com.sun.javaws.SplashScreen;
import com.sun.javaws.Globals;
import com.sun.javaws.BrowserSupport;
import com.sun.javaws.LocalInstallHandler;
import com.sun.javaws.LocalApplicationProperties;
import com.sun.javaws.util.JavawsConsoleController;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.cache.Cache;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.AboutDialog;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.si.*;


public class CacheViewer extends JFrame implements ActionListener,
	ChangeListener, ListSelectionListener, DeploySIListener {

    private final JButton _removeBtn;
    private final JButton _launchOnlineBtn;
    private final JButton _launchOfflineBtn;

    private final JTabbedPane _tabbedPane;
    private final CacheTable _userTable;
    private final CacheTable _sysTable;
    private final JScrollPane _userTab;
    private final JScrollPane _systemTab;

    private final static String BOUNDS_PROPERTY_KEY =
        "deployment.javaws.viewer.bounds";

    private JMenuItem _launchOnlineMI;
    private JMenuItem _launchOfflineMI;
    private JMenuItem _removeMI;
    private JMenuItem _showMI;
    private JMenuItem _installMI;
    private JMenuItem _browseMI;

    private JMenu _fileMenu;
    private JMenu _editMenu;
    private JMenu _appMenu;
    private JMenu _viewMenu;
    private JMenu _helpMenu;

    private TitledBorder _titledBorder;

    public final static int STATUS_OK = 0;
    public final static int STATUS_REMOVING = 1;
    public final static int STATUS_LAUNCHING = 2;
    public final static int STATUS_BROWSING = 3;
    public final static int STATUS_SORTING = 4;
    public final static int STATUS_SEARCHING = 5;
    public final static int STATUS_INSTALLING = 6;

    private static int _status = STATUS_OK;
    private static JLabel _statusLabel;
    private static final JLabel _totalSize = new JLabel();

    private final static LocalInstallHandler _lih =
	LocalInstallHandler.getInstance();
    private final static boolean _isLocalInstallSupported =
	_lih.isLocalInstallSupported();

    private static long t0, t1, t2, t3, t4;
    private SingleInstanceImpl _sil;
    private final static String JAVAWS_CV_ID = "JNLP Cache Viewer" +
	Config.getInstance().getSessionSpecificString();


    private static final int SLEEP_DELAY = 2000;

    public CacheViewer() {

	_sil = new SingleInstanceImpl();
	_sil.addSingleInstanceListener(this, JAVAWS_CV_ID);

        _removeBtn = makeButton("jnlp.viewer.remove.btn");
        _launchOnlineBtn = makeButton("jnlp.viewer.launch.online.btn");
        _launchOfflineBtn = makeButton("jnlp.viewer.launch.offline.btn");

        _statusLabel = new JLabel(" ");

	_tabbedPane = new JTabbedPane();
        _userTable = new CacheTable(this, false);
        _sysTable = new CacheTable(this, true);
        _userTab = new JScrollPane(_userTable);
        _systemTab = new JScrollPane(_sysTable);

        initComponents();

    }

    private void initComponents() {
        setTitle(ResourceManager.getMessage("jnlp.viewer.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                exitViewer();
            }
        });

        JPanel mainPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout());
	_titledBorder = new TitledBorder(
		ResourceManager.getMessage("jnlp.viewer.all"));
	Border b1 = BorderFactory.createEmptyBorder(4,4,4,4);
	Border b2 = BorderFactory.createCompoundBorder(b1, _titledBorder);

        mainPanel.setBorder(BorderFactory.createCompoundBorder(b2, b1));

	if (Globals.isSystemCache()) {
	    _tabbedPane.addTab(ResourceManager.getMessage(
		"cert.dialog.system.level"), _userTab);
	} else {
	    _tabbedPane.addTab(
	        ResourceManager.getMessage("cert.dialog.user.level"), _userTab);
            _tabbedPane.addTab(ResourceManager.getMessage(
       		"cert.dialog.system.level"), _systemTab);
	}

	_tabbedPane.setSelectedIndex(0);
	_tabbedPane.addChangeListener(this);
	mainPanel.add(_tabbedPane, BorderLayout.CENTER);

        Box buttonsBox = Box.createHorizontalBox();
	buttonsBox.setBorder(BorderFactory.createEmptyBorder(6,0,0,0));

        buttonsBox.add(_removeBtn);
        buttonsBox.add(Box.createHorizontalGlue());
        buttonsBox.add(_launchOnlineBtn);
        buttonsBox.add(Box.createHorizontalStrut(5));
        buttonsBox.add(_launchOfflineBtn);

        mainPanel.add(buttonsBox, BorderLayout.SOUTH);

	JPanel bottomPanel = new JPanel(new BorderLayout());
	_totalSize.setText(getAppMessage("jnlp.viewer.totalSize", ""));
	_totalSize.setHorizontalAlignment(JLabel.CENTER);
	_totalSize.setFont(ResourceManager.getUIFont());

        JPanel statusPanel = new JPanel(new BorderLayout());
        _statusLabel = new JLabel(" ");
        _statusLabel.setBorder(BorderFactory.createEmptyBorder(0,6,0,6));
	_statusLabel.setFont(ResourceManager.getUIFont());

        statusPanel.add(_statusLabel, BorderLayout.WEST);
	statusPanel.add(_totalSize, BorderLayout.CENTER);
        statusPanel.setBorder(BorderFactory.createEtchedBorder(
		EtchedBorder.LOWERED));

	bottomPanel.add(statusPanel, BorderLayout.SOUTH);
	bottomPanel.setBorder(BorderFactory.createEmptyBorder(6,6,6,6));

        // Add everything  to the contentPane.
	getContentPane().add(Box.createVerticalStrut(8), BorderLayout.NORTH);
        getContentPane().add(mainPanel, BorderLayout.CENTER);
        getContentPane().add(bottomPanel, BorderLayout.SOUTH);

	createMenuBar();
        pack();

	_userTable.getSelectionModel().addListSelectionListener(this);
        _sysTable.getSelectionModel().addListSelectionListener(this);
    }

    // File             - Java Control Panel, Exit
    // Application      - Start, Remove, DTI
    // View             - Jnlp, homepage
    // Help             - About
    private void createMenuBar() {
        _fileMenu = new JMenu(
		ResourceManager.getMessage("jnlp.viewer.file.menu"));
        _fileMenu.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.file.menu.mnemonic"));
        JMenuItem mi;

/*
        mi = _fileMenu.add(
		ResourceManager.getMessage("jnlp.viewer.cpl.mi"));
        mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.cpl.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                Main.launchJavaControlPanel("general");
            }
        });
        mi.setEnabled(true);

        _fileMenu.addSeparator();
*/

        mi = _fileMenu.add(
		ResourceManager.getMessage("jnlp.viewer.exit.mi"));
        mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.exit.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                exitViewer();
            }
        });

	_editMenu = new JMenu(
		ResourceManager.getMessage("jnlp.viewer.edit.menu"));
        _editMenu.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.edit.menu.mnemonic"));

	mi = _editMenu.add(
		ResourceManager.getMessage("jnlp.viewer.reinstall.mi"));
        mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.reinstall.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                showReInstallDialog();
            }
        });

        _editMenu.addSeparator();

        mi = _editMenu.add(
		ResourceManager.getMessage("jnlp.viewer.preferences.mi"));
        mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.preferences.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                Main.launchJavaControlPanel("general");
            }
        });

        _appMenu = new JMenu(
		ResourceManager.getMessage("jnlp.viewer.app.menu"));
        _appMenu.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.app.menu.mnemonic"));
        _appMenu.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent ae) {
	        refresh();
            }
        });

        _launchOfflineMI = _appMenu.add("");	// text set in refresh()
        _launchOfflineMI.setMnemonic(ResourceManager.getVKCode(
		"jnlp.viewer.launch.offline.mi.mnemonic"));

        _launchOnlineMI = _appMenu.add("");	// text set in refresh()
        _launchOnlineMI.setMnemonic(ResourceManager.getVKCode(
		"jnlp.viewer.launch.online.mi.mnemonic"));

        _appMenu.addSeparator();

        // Only add the install/uninstall menu item if the platform
        // supports it.
        LocalInstallHandler lih = LocalInstallHandler.getInstance();
        if (_isLocalInstallSupported) {
            _installMI = _appMenu.add("");	// text set in refresh()
            _installMI.setMnemonic(
                ResourceManager.getVKCode("jnlp.viewer.install.mi.mnemonic"));
            _installMI.addActionListener(new ActionListener() {
                 public void actionPerformed(ActionEvent ae) {
                     integrateApplication();
                 }
            });
        }
        _showMI = _appMenu.add("");	// text set in refresh()
        _showMI.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.show.mi.mnemonic"));

        _browseMI = _appMenu.add("");	// text set in refresh()
        _browseMI.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.browse.mi.mnemonic"));

        _appMenu.addSeparator();

        _removeMI = _appMenu.add("");	// text set in refresh()
	_removeMI.setMnemonic(
                ResourceManager.getVKCode("jnlp.viewer.remove.mi.mnemonic"));


        _launchOfflineMI.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		launchApplication(false);
	    }
	});

        _launchOnlineMI.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		launchApplication(true);
	    }
	});

        _showMI.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent ae) {
                showApplication();
            }
        });

        _browseMI.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent ae) {
                browseApplication();
            }
        });

        _removeMI.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
	        removeApplications();
	    }
	});

        _viewMenu = new JMenu(
		ResourceManager.getMessage("jnlp.viewer.view.menu"));
        _viewMenu.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.view.menu.mnemonic"));

	for (int i=0; i<5; i++) {
	    mi = _viewMenu.add(new JCheckBoxMenuItem(
		ResourceManager.getMessage("jnlp.viewer.view."+i+".mi"), 
		(i==0)));
            mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.view."+i+".mi.mnemonic"));
            mi.addActionListener(new ActionListener() {
	        public void actionPerformed(ActionEvent ae) {
		    final Object source = ae.getSource();
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                   	    for (int pos = 0; pos < 5; pos++) {
				JMenuItem jmi = _viewMenu.getItem(pos);
				if (jmi instanceof JCheckBoxMenuItem) {
                        	    JCheckBoxMenuItem cbmi =
                            		(JCheckBoxMenuItem) jmi;
                        	    if (source.equals(cbmi)) {
                                        cbmi.setState(true);
                                        setFilter(pos);
                                    } else {
                                        cbmi.setState(false);
                                    }
				}
                    	    }    
                        }
                    });

	        }
	    });
	}


        _helpMenu = new JMenu(
		ResourceManager.getMessage("jnlp.viewer.help.menu"));
        _helpMenu.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.help.menu.mnemonic"));


	mi = _helpMenu.add(
		ResourceManager.getMessage("jnlp.viewer.help.java.mi"));
	mi.setMnemonic(
	        ResourceManager.getVKCode("jnlp.viewer.help.java.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
		String j2se = Config.getProperty(Config.REMOTE_J2SE_DOCS_KEY);
		try {
		    URL url = new URL(j2se);
	            showDocument(url);
		} catch (Exception ex) {
		    Trace.ignoredException(ex);
		}
	    }
        });


	mi = _helpMenu.add(
		ResourceManager.getMessage("jnlp.viewer.help.jnlp.mi"));
	mi.setMnemonic(
	        ResourceManager.getVKCode("jnlp.viewer.help.jnlp.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
		String jnlp = Config.getProperty(Config.REMOTE_JNLP_DOCS_KEY);
		try {
		    URL url = new URL(jnlp);
	            showDocument(url);
		} catch (Exception ex) {
		    Trace.ignoredException(ex);
		}
	    }
        });

        _appMenu.addSeparator();

        mi = _helpMenu.add(
		ResourceManager.getMessage("jnlp.viewer.about.mi"));
        mi.setMnemonic(
		ResourceManager.getVKCode("jnlp.viewer.about.mi.mnemonic"));
        mi.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
		showAbout();
	    }
        });

        JMenuBar mb = new JMenuBar();
	mb.add(_fileMenu);
	mb.add(_editMenu);
	mb.add(_appMenu);
	mb.add(_viewMenu);
	//mb.add(Box.createHorizontalGlue());
	mb.add(_helpMenu);
        setJMenuBar(mb);
        resetSizes();
	refresh();

    }

    private void setFilter(final int filter) {
	String title;
	if (filter == 0) {
	    title = ResourceManager.getMessage("jnlp.viewer.all");
	} else {
	    MessageFormat mf = new MessageFormat(
		ResourceManager.getMessage("jnlp.viewer.type"));
	    Object[] args = new Object[1];
	    args[0] = ResourceManager.getMessage("jnlp.viewer.view."+filter);
	    title = mf.format(args);
	}
	_titledBorder.setTitle(title);
	getSelectedTable().setFilter(filter);
	getContentPane().repaint();
    }

    public JButton makeButton(String key) {
	JButton b = new JButton(ResourceManager.getMessage(key));
	b.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
	b.addActionListener(this);
	return b;
    }

    public void valueChanged(ListSelectionEvent e) {
	refresh();
    }

    public void stateChanged(ChangeEvent ev) {
	refresh();
	resetSizes();
    }

    private void resetSizes() {
        Component c = _tabbedPane.getSelectedComponent();
        final boolean system = (! c.equals(_userTab));
        new Thread(new Runnable() {
            public void run() {
		if (getStatus() == STATUS_OK) {
                    setStatus(STATUS_SEARCHING);
		}
                try {
		    long size = Cache.getCacheSize(system);
		    if (size > 0) {
		        _totalSize.setText(getAppMessage(
			    "jnlp.viewer.totalSize", tformat(size)));
 		    } else if (size < 0) {
 		 	String tab = _tabbedPane.getTitleAt(
 			    _tabbedPane.getSelectedIndex());
 		        _totalSize.setText(getMessage("jnlp.viewer.noCache"));
		    } else {
		 	String tab = _tabbedPane.getTitleAt(
			    _tabbedPane.getSelectedIndex());
		        _totalSize.setText(getAppMessage(
			    "jnlp.viewer.emptyCache", tab));
		    }
                } finally {
		    if (getStatus() == STATUS_SEARCHING) {
                        setStatus(STATUS_OK);
		    }
                }
            }
        }).start();
    }

    private static String tformat(long size) {
	if (size > 10240) {
	    return "" + (size/1024) + " KB";
	}
	return "" + (size/1024) + "." + (size%1024)/102 + " KB";
    }

    public void refresh() {
	int selected[];
	CacheTable table;
	Component c = _tabbedPane.getSelectedComponent();
	boolean system = (! c.equals(_userTab));
	if (system) {
	    table = _sysTable;
	} else {
	    table = _userTable;
	}
	selected =  table.getSelectedRows();

	boolean launchOn = false;
	boolean launchOff = false;
	boolean install = false;
	boolean show = false;
	boolean browse = false;
	boolean isInstalled = false;
	boolean remove = (!system && selected.length > 0);

	String appString = "";

	if (selected.length == 1) {
	    show = true;
	    CacheObject co = table.getCacheObject(selected[0]);
	    if (co != null) {
		LaunchDesc ld = co.getLaunchDesc();
		appString = co.getTypeString();
	        InformationDesc id = ld.getInformation();
	        if (ld.isApplication() || ld.isApplet()) {
		    if (_isLocalInstallSupported) {
        		LocalApplicationProperties lap =
				co.getLocalApplicationProperties();
			isInstalled = lap.isLocallyInstalled();
                        install = !system && !lap.isLocallyInstalledSystem();
		    }
		    if (id.supportsOfflineOperation()) {
			launchOff = true;
		    }
		    if (ld.getLocation() != null) {
		        launchOn = true;
		    }
	        }
	        if (id.getHome() != null) {
		    browse = true;
	        }
	        _removeBtn.setText(
			getAppMessage("jnlp.viewer.remove.1.btn", appString));
	    }
	} else if (selected.length == 0) {
	    _removeBtn.setText(ResourceManager.getMessage(
		"jnlp.viewer.remove.btn"));
	} else {
	    _removeBtn.setText(ResourceManager.getMessage(
                "jnlp.viewer.remove.2.btn"));
	}

	_launchOnlineBtn.setEnabled(launchOn);
	_launchOnlineMI.setEnabled(launchOn);
	_launchOnlineMI.setText(
		getMessage("jnlp.viewer.launch.online.mi"));

	_launchOfflineBtn.setEnabled(launchOff);
	_launchOfflineMI.setEnabled(launchOff);
	_launchOfflineMI.setText(
		getMessage("jnlp.viewer.launch.offline.mi"));

	if (_isLocalInstallSupported) {
	    _installMI.setEnabled(install);
	    if (isInstalled) {
	        _installMI.setText(
		    getMessage("jnlp.viewer.uninstall.mi"));
	        _installMI.setMnemonic(ResourceManager.getVKCode(
				"jnlp.viewer.uninstall.mi.mnemonic"));
	    } else {
	        _installMI.setText(
		    getMessage("jnlp.viewer.install.mi"));
	        _installMI.setMnemonic(ResourceManager.getVKCode(
					"jnlp.viewer.install.mi.mnemonic"));
	    }
	}

	_showMI.setEnabled(show);
	_showMI.setText(getMessage("jnlp.viewer.show.mi"));

	_browseMI.setEnabled(browse);
	_browseMI.setText(getMessage("jnlp.viewer.browse.mi"));

	_removeBtn.setEnabled(remove);
	_removeMI.setEnabled(remove);
        if (selected.length == 1) {
            _removeMI.setText(getAppMessage("jnlp.viewer.remove.mi",appString));
        } else { 
            _removeMI.setText(getMessage("jnlp.viewer.remove.0.mi")); 
        } 
    }

    private String getMessage(String key) {
	return ResourceManager.getMessage(key);
    }

    // replace the {0} with the appString
    private String getAppMessage(String key, String appString) {
	JTable table;

	MessageFormat mf = new MessageFormat(ResourceManager.getMessage(key));
	Object args[] = new Object[1];
	args[0] = (Object) appString;
	return mf.format(args);
    }

    private CacheObject getSelectedCacheObject() {
	int selected[];
	CacheTable table;
        Component c = _tabbedPane.getSelectedComponent();
        if (c.equals(_userTab)) {
            table = _userTable;
        } else {
            table = _sysTable;
        }
	selected =  table.getSelectedRows();
        if (selected.length == 1) {
		return table.getCacheObject(selected[0]);
	}
	return null;
    }



    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
	exitViewer();
    }

    private void exitViewer() {
	_sil.removeSingleInstanceListener(this);
        setVisible(false);
        dispose();
	Rectangle r = getBounds();
	Config.setProperty(BOUNDS_PROPERTY_KEY,
			   ""+r.x+ ","+r.y+ ","+r.width+ ","+r.height);
	Config.storeIfDirty();
	Main.systemExit(0);
    }

    public void actionPerformed(ActionEvent ae) {
	JButton src = (JButton) ae.getSource();

	if (src == _removeBtn) {
	    removeApplications();
	} else if (src == _launchOnlineBtn) {
	    launchApplication(true);
	} else if (src == _launchOfflineBtn) {
	    launchApplication(false);
	}
    }

    private CacheTable getSelectedTable() {
	return ((_tabbedPane.getSelectedComponent() == _userTab) ?
            	 _userTable : _sysTable);
    }

    private void launchApplication(boolean bOnline) {
	if (getStatus() != STATUS_LAUNCHING) {
	    if (getStatus() == STATUS_OK) {
	        setStatus(STATUS_LAUNCHING);
	    }
	    try {
	        CacheObject co = getSelectedCacheObject();
	        if (co != null) try {
	            File jnlpFile = co.getJnlpFile();
                    String cmd[] = new String[3];
                    cmd[0] = Config.getJavawsCommand();
	            cmd[1] = ((bOnline) ? "-online" : "-offline");
                    cmd[2] = jnlpFile.getPath();
                    Runtime.getRuntime().exec(cmd);
                } catch (java.io.IOException ioe) {
                    // FIXIT: Should show an error!
                    Trace.ignoredException(ioe);
                }
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        reset(_userTable);
                    }
                });

	    } finally {
		if (getStatus() == STATUS_LAUNCHING) {
	            setStatus(STATUS_OK);
		}
	    }
	}
    }

    public void launchApplication() {
	if (_launchOnlineBtn.isEnabled()) {
	    launchApplication(true);
	} else if (_launchOfflineBtn.isEnabled()) {
	    launchApplication(false);
	}

    }

    private void browseApplication() {
	CacheObject co = getSelectedCacheObject();
	if (co != null) {
	    LaunchDesc ld = co.getLaunchDesc();
	    if (ld != null) {
		URL home = ld.getInformation().getHome();
		showDocument(home);
	    }
	}
    }

    private void showDocument(final URL page) {
	if (getStatus() != STATUS_BROWSING) {
            new Thread(new Runnable() {
                public void run() {
		    if (getStatus() == STATUS_OK) {
			setStatus(STATUS_BROWSING);
		    }
		    try {
		        BrowserSupport.showDocument(page);
		    } finally {
          		if (getStatus() == STATUS_BROWSING) { 
		            setStatus(STATUS_OK);
			}
		    }
	        }
	    }).start();
	}
    }

    private void showApplication() {
	CacheObject co = getSelectedCacheObject();
	if (co != null) {
	    LaunchDesc  ld = co.getLaunchDesc();
	    String str = ld.toString();

            JTextArea text = new JTextArea(str, 24, 81);
            text.setEditable(false);

	    JScrollPane sp = new JScrollPane(text,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

            DialogFactory.showMessageDialog(this,
                DialogFactory.INFORMATION_MESSAGE,
                sp,
                getAppMessage("jnlp.viewer.show.title",
                    ld.getInformation().getTitle()),
                false);
	}
    }

    private void showAbout() {
	(new AboutDialog(this, true)).setVisible(true);
    }

    private void cleanCache() {
	if (getStatus() == STATUS_OK) {
            new Thread(new Runnable() {
                public void run() {
		    setStatus(STATUS_REMOVING);
                    SwingUtilities.invokeLater(new Runnable() {
                         public void run() {
			    try {
			        Cache.clean();
			        reset(_userTable);
			    } finally {
				if (getStatus() == STATUS_REMOVING) {
			            setStatus(STATUS_OK);
				}
			    }
			}
                    });
                }
            }).start();
	}
    }

    private void removeApplications() {
	if (getStatus() == STATUS_OK) {
            new Thread(new Runnable() {
                public void run() {
	            setStatus(STATUS_REMOVING);
                    Component c = _tabbedPane.getSelectedComponent();
                    boolean system = (!c.equals(_userTab));
                    CacheTable table =  (system) ? _sysTable : _userTable;

                    int [] selected =  table.getSelectedRows();
               	    for (int i=0; i<selected.length; i++) {
                        CacheObject co = table.getCacheObject(selected[i]);
                        Cache.remove(co.getDCE(),
				     co.getLocalApplicationProperties(),
				     co.getLaunchDesc());
		    }
		    Cache.clean();
                    SwingUtilities.invokeLater(new Runnable() {
                         public void run() {
			     try {
	               	         reset(_userTable);
			     } finally {
				if (getStatus() == STATUS_REMOVING) {
	               	             setStatus(STATUS_OK);
				}
			     }
                         }
                    });
                }
            }).start();
	}
    }

    public void popupApplicationMenu(Component tree, int x, int y) {
	CacheObject co = getSelectedCacheObject();
	if (co != null) {
	    JPopupMenu pop = new JPopupMenu();
	    Component comps[] = _appMenu.getMenuComponents();
	    for (int i=0; i<comps.length; i++) {
		if (comps[i] instanceof JMenuItem) {
		    JMenuItem mi = (JMenuItem) comps[i];
		    JMenuItem pi = pop.add(new JMenuItem(
			mi.getText(), mi.getMnemonic()));
		    pi.setEnabled(mi.isEnabled());
		    ActionListener al[] = mi.getActionListeners();
		    for (int j=0; j<al.length; pi.addActionListener(al[j++]));
		} else {
		   pop.addSeparator();
		}
	    }
	    pop.show(tree, x, y);
	}
    }

    private void integrateApplication() {
	CacheObject co = getSelectedCacheObject();
        if ((co != null) && _isLocalInstallSupported) {
            LocalApplicationProperties lap = co.getLocalApplicationProperties();
            Component c = _tabbedPane.getSelectedComponent();
            boolean system = (!c.equals(_userTab));
            CacheTable table =  (system) ? _sysTable : _userTable;

            new Thread(new Installer(co.getLaunchDesc(), lap, table)).start();
        }
    }

    class Installer implements Runnable {

        private final LaunchDesc _ld;
        private final LocalApplicationProperties _lap;
        private final CacheTable _table;

        public Installer(LaunchDesc ld,
                         LocalApplicationProperties lap,
                         CacheTable table) {
            _ld = ld;
            _lap = lap;
            _table = table;
        }

        public void run() {
            _lap.refreshIfNecessary();
            if (_lap.isLocallyInstalled()) {
                 _lih.uninstall(_ld, _lap, true);
            } else {
                 _lih.doInstall(_ld, _lap);
            }
            _lap.setAskedForInstall(true);
            try {
                _lap.store();
            } catch (Exception ioe) {
                Trace.ignoredException(ioe);
            }
	    refresh();
        }
    }


    public void reset(CacheTable table) {
	resetSizes();
	table.reset();
	refresh();
    }

    public static int getStatus() {
	return _status;
    }

    public static void setStatus(int status) {
	_status = status;
	String text;
	switch (status) {
	    case STATUS_REMOVING:
		text = ResourceManager.getMessage("jnlp.viewer.removing");
		break;
	    case STATUS_LAUNCHING:
		text = ResourceManager.getMessage("jnlp.viewer.launching");
		break;
	    case STATUS_BROWSING:
		text = ResourceManager.getMessage("jnlp.viewer.browsing");
		break;
	    case STATUS_SORTING:
		text = ResourceManager.getMessage("jnlp.viewer.sorting");
		break;
	    case STATUS_SEARCHING:
		text = ResourceManager.getMessage("jnlp.viewer.searching");
		break;
	    case STATUS_INSTALLING:
		text = ResourceManager.getMessage("jnlp.viewer.installing");
		break;
	    default:
	    case STATUS_OK:
		text = "";
		break;
	}
	if (status == STATUS_OK) {
	    _statusLabel.setText(text);
	    _totalSize.setVisible(true);
	} else {
	    _totalSize.setVisible(false);
	    _statusLabel.setText(text);
	}
    }


    private void showReInstallDialog() {

	Properties p = Cache.getRemovedApps();
	{	// filter out those allready re-installed
		String [] hrefs = _userTable.getAllHrefs();
		boolean removed = false;
		for (int i=0; i<hrefs.length; i++ ) {
		    if (p.getProperty(hrefs[i]) != null) {
		        p.remove(hrefs[i]);
			removed = true;
		    }
		}
		if (removed) {
	            Cache.setRemovedApps(p);
		}
	}
	final ArrayList hrefs = new ArrayList();
	final ArrayList titles = new ArrayList();

        Enumeration e = p.propertyNames();
        while (e.hasMoreElements()) {
	    String url = (String) e.nextElement();
	    hrefs.add(url);
	    titles.add(p.getProperty(url));
        }
        final String titleName = ResourceManager.getMessage(
		"jnlp.viewer.reinstall.column.title");
        final String hrefName = ResourceManager.getMessage(
		"jnlp.viewer.reinstall.column.location");

	final AbstractTableModel tm = new AbstractTableModel() {
		public String getColumnName(int col) {
		    return (col == 0) ? titleName : hrefName;
		}
		public Object getValueAt(int row, int col) {
		    return (col == 0) ? titles.get(row) : hrefs.get(row);
		}
		public int getColumnCount() {
		    return 2;
		}
		public int getRowCount() {
		    return hrefs.size();
		}
		public Class getColumnClass(int col) {
		    return String.class;
		}
	};

        String key = "jnlp.viewer.reinstallBtn";
        final JButton reinstall = new JButton(ResourceManager.getMessage(key));
        reinstall.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
	reinstall.setEnabled(false);

        key = "jnlp.viewer.closeBtn";
        JButton close = new JButton(ResourceManager.getMessage(key));
        close.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));

        Object [] options = { reinstall, close };

        final JTable table = new JTable(tm);
        reinstall.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int [] selected =  table.getSelectedRows();
		String [] remove = new String[selected.length];
		for (int i=0; i<remove.length; i++) {
		    remove[i] = (String)hrefs.get(selected[i]);
		}
		do_reinstall(remove);
            }
        });

	table.getColumnModel().getColumn(0).setPreferredWidth(200);
	table.getColumnModel().getColumn(1).setPreferredWidth(440);
	table.setPreferredScrollableViewportSize(new Dimension(640, 180));
	JScrollPane sp = new JScrollPane(table);
	table.getSelectionModel().addListSelectionListener(
	    new ListSelectionListener() {
	        public void valueChanged(ListSelectionEvent e) {
		    reinstall.setEnabled(table.getSelectedRowCount() > 0);
	        }
	    });

        int ret = DialogFactory.showOptionDialog(this,
	    DialogFactory.PLAIN_MESSAGE,
	    sp,
	    ResourceManager.getMessage("jnlp.viewer.reinstall.title"),
	    options,
	    close );
    }

    public void do_reinstall(final String[] hrefs) {
        new Thread(new Runnable() {
            public void run() {
		if (getStatus() == STATUS_OK) {
                    setStatus(STATUS_INSTALLING);
		}
                try {
                    for (int i=0; i<hrefs.length; i++) {
                        Main.importApp(hrefs[i]);
			// don't let more than 8 be going at once or 
                        // you can get resource exhaustion
			int safty = 0;
			while (Main.getLaunchThreadGroup().activeCount() > 8) {
			    // take a nap
			    try { Thread.sleep(2000); } catch (Exception e) {};
			    if (++safty > 5) { break; }
			}
		        if (Main.getLaunchThreadGroup().activeCount() > 8) {
			    Trace.println("Warning: after waiting, still "+
			        Main.getLaunchThreadGroup().activeCount() + 
			        " launching threads" );
			}
                    }
                } catch (Exception excep) {
                    Trace.ignoredException(excep);
                } finally {
		    for (int i=10; i>0; i--) {
			int count = Main.getLaunchThreadGroup().activeCount();
			if (count <= 0) break;
			try { Thread.sleep(2000); } catch (Exception e) {};
		    }
		    if (Main.getLaunchThreadGroup().activeCount() > 0) {
			Trace.println("Warning: after waiting 20 sec., still "+
			    Main.getLaunchThreadGroup().activeCount() + 
			    " launching threads" );
		    }
		    if (getStatus() == STATUS_INSTALLING) {
                        setStatus(STATUS_OK);
		    }
                }
            }    
        }).start();
    }

    public void newActivation(String[] params) {
	_userTable.setFilter(0);
	_sysTable.setFilter(0);
	this.setExtendedState(this.getExtendedState() & ~Frame.ICONIFIED);
 	this.toFront();
    }

    public Object getSingleInstanceListener() {
	return this;
    }

    public static void main(String[] args) {

	SplashScreen.hide();

        if (SingleInstanceManager.isServerRunning(JAVAWS_CV_ID)) {
	    if (SingleInstanceManager.connectToServer("dummy")) {
                System.exit(0);
	    }
 	}

	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    if (Config.getBooleanProperty("deployment.debug.console")) {
	 	JavawsConsoleController.showConsoleIfEnable();
	    }

            final CacheViewer cv = new CacheViewer();

            String p = Config.getProperty(BOUNDS_PROPERTY_KEY);
            if (p != null) {
                StringTokenizer st = new StringTokenizer(p,",");
                int xywh[] = new int[4];
                int i;
                for (i=0; i<4; i++) {
                    if (st.hasMoreTokens()) {
                        String str = st.nextToken();
                        try {
                            xywh[i]=Integer.parseInt(str);
                            continue;
                        } catch (NumberFormatException e) { };
                    }
                }
                if (i == 4) {
                    cv.setBounds(xywh[0], xywh[1], xywh[2], xywh[3]);
                }
            }
	    cv.setVisible(true);

	    long _lastTimeUser = Cache.getLastAccessed(false);
	    long _lastTimeSys = Cache.getLastAccessed(true);

	    for (;;) {
                try {
                    Thread.sleep(SLEEP_DELAY);
                } catch (InterruptedException ie) {
                    break;
                }
                long newTimeUser = Cache.getLastAccessed(false);
                long newTimeSys = Cache.getLastAccessed(true);

                if (newTimeUser != _lastTimeUser) {
		    if (cv.getStatus() == STATUS_OK) {
                        _lastTimeUser = newTimeUser;

                        SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                cv.reset(cv._userTable);
                            }
                        });
                    }
		}
                if (newTimeSys != _lastTimeSys) {
		    if (cv.getStatus() == STATUS_OK) {
                        _lastTimeSys = newTimeSys;

                        SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                cv.reset(cv._sysTable);
                            }
                        });
                    }
		}
	    }
	} finally {
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }
}
