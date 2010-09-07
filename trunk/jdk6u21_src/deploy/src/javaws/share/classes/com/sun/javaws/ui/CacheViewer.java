/*
 * @(#)CacheViewer.java	1.100 10/03/24
 *
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.InputStream;
import java.net.URL;

import com.sun.javaws.Main;
import com.sun.javaws.CacheUtil;
import com.sun.javaws.BrowserSupport;
import com.sun.javaws.LocalInstallHandler;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.jnl.ShortcutDesc;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.Environment;


public class CacheViewer extends JDialog implements 
        ListSelectionListener, PopupMenuListener {

    private CacheViewer(JFrame parent) {
        super(parent);
        
        // no need to check cached ip address for cache viewer entries
        Cache.setDoIPLookup(false);
        
        setTitle(getResource("viewer.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                exitViewer();
            }
        });
        initComponents();
        wasDirty = Config.isDirty();
    }

    private final JPanel tablePanel = new JPanel(new BorderLayout());
    
    private CacheTable jnlpTable = null;
    private CacheTable resourceTable = null;
    private CacheTable importTable = null;
    private CacheTable sysJnlpTable = null;
    private CacheTable sysResourceTable = null;
    private CacheTable activeTable;
    
    private boolean noSeparator = false;
    private boolean wasDirty = false;
    private int leadinSpace;
    private int minSpace;

    private JComboBox viewComboBox;

    private JLabel viewLabel;
    private JToolBar toolbar;

    private JLabel sizeLabel = new JLabel("");
    private JButton runButton;
    private JButton removeButton;
    private JButton installButton;
    private JButton showButton;
    private JButton showResourceButton;
    private JButton homeButton;
    private JButton removeResourceButton;
    private JButton removeRemovedButton;
    private JButton importButton;
    private JButton closeButton;

    private JPopupMenu runPopup;
    private JMenuItem onlineMI;
    private JMenuItem offlineMI;

    private JPopupMenu popup;
    private JMenuItem runOnlineMI;
    private JMenuItem runOfflineMI;
    private JMenuItem installMI;
    private JMenuItem removeMI;
    private JMenuItem showMI;
    private JMenuItem showResourceMI;
    private JMenuItem homeMI;
    private JMenuItem importMI;


    private void refresh() {
        final int index = viewComboBox.getSelectedIndex();
        // extra space between unseparated items
        final Dimension d0 = new Dimension(minSpace, 0);
        // extra space between image and JToolBar separators
        final Dimension d1 = new Dimension(8, 0);
        // extra space before first item
        final Dimension d2 = new Dimension(leadinSpace, 0);
        // extra space between viewLabel and viewCombo
        final Dimension d4 = new Dimension(4, 0);

        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                JButton firstButton, lastButton;
                Component[] children = toolbar.getComponents();
                
                if (children.length == 0) {
                    // the first time put in these first 5 components;
                    toolbar.add(new Box.Filler(d1, d1, d1));
                    toolbar.add(viewLabel);
                    toolbar.add(new Box.Filler(d4, d4, d4));
                    toolbar.add(viewComboBox);
                    toolbar.add(new Box.Filler(d2, d2, d2));
                } else {
                    // subsequently, remove all the items beyond those 5
                    for (int i = children.length - 1; i > 4; i--) {
                        toolbar.remove(i);
                    }
                }
                switch (index) {
                    default:
                    case 0:
                        if (jnlpTable == null) {
                            jnlpTable = new CacheTable(CacheViewer.this, 
                                        CacheTable.JNLP_TYPE, false);
                        }
                        sizeLabel.setText(jnlpTable.getSizeLabelText());
                        activeTable = jnlpTable;

                              // 5 buttons for user jnlp files:
                        toolbar.add(runButton);
                        firstButton = runButton;
                        if (noSeparator) {
                            toolbar.add(new Box.Filler(d0, d0, d0));
                        } else {
                            toolbar.addSeparator(d1);
                            toolbar.add(new VSeparator());
                            toolbar.addSeparator(d1);
                        }
                        toolbar.add(showButton);
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(installButton);
                        if (noSeparator) {
                            toolbar.add(new Box.Filler(d0, d0, d0));
                        } else {
                            toolbar.addSeparator(d1);
                            toolbar.add(new VSeparator());
                            toolbar.addSeparator(d1);
                        }
                        toolbar.add(removeButton);
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(homeButton);
                        runButton.setNextFocusableComponent(showButton);
                        showButton.setNextFocusableComponent(installButton);
                        installButton.setNextFocusableComponent(removeButton);
                        removeButton.setNextFocusableComponent(homeButton);
                        lastButton = homeButton;
                        break;
                    case 1:
                        if (resourceTable == null) {
                            resourceTable = new CacheTable(CacheViewer.this, 
                                        CacheTable.RESOURCE_TYPE, false);
                        }
                        sizeLabel.setText(resourceTable.getSizeLabelText());
                        activeTable = resourceTable;

                        // only removeResource and show button for resources
                        toolbar.add(showResourceButton);
                        firstButton = showResourceButton;
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(removeResourceButton);
                        showResourceButton.setNextFocusableComponent(
                             removeResourceButton);
                        lastButton = removeResourceButton;
                        break;
                    case 2:
                        if (importTable == null) {
                            importTable = new CacheTable(CacheViewer.this, 
                                        CacheTable.DELETED_TYPE, false);
                        }
                        sizeLabel.setText(importTable.getSizeLabelText());
                        activeTable = importTable;

                        // only import and delete button for deleted files view
                        toolbar.add(importButton);
                        firstButton = importButton;
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(removeRemovedButton);
                        importButton.setNextFocusableComponent(
                            removeRemovedButton);
                        lastButton = removeRemovedButton;
                        break;
                    case 3:
                        if (sysJnlpTable == null) {
                            sysJnlpTable = new CacheTable(CacheViewer.this, 
                                        CacheTable.JNLP_TYPE, true);
                        }
                        sizeLabel.setText(sysJnlpTable.getSizeLabelText());
                        activeTable = sysJnlpTable;

                        // 5 buttons for user jnlp files:
                        toolbar.add(runButton);
                        firstButton = runButton;
                        if (noSeparator) {
                            toolbar.add(new Box.Filler(d0, d0, d0));
                        } else {
                            toolbar.addSeparator(d1);
                            toolbar.add(new VSeparator());
                            toolbar.addSeparator(d1);
                        }
                        toolbar.add(showButton);
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(installButton);
                        if (noSeparator) {
                            toolbar.add(new Box.Filler(d0, d0, d0));
                        } else {
                            toolbar.addSeparator(d1);
                            toolbar.add(new VSeparator());
                            toolbar.addSeparator(d1);
                        }
                        toolbar.add(removeButton);
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(homeButton);
                        runButton.setNextFocusableComponent(showButton);
                        showButton.setNextFocusableComponent(installButton);
                        installButton.setNextFocusableComponent(removeButton);
                        removeButton.setNextFocusableComponent(homeButton);
                        lastButton = homeButton;
                        break;
                    case 4:
                        if (sysResourceTable == null) {
                            sysResourceTable = new CacheTable(CacheViewer.this, 
                                        CacheTable.RESOURCE_TYPE, true);
                        }
                        sizeLabel.setText(sysResourceTable.getSizeLabelText());
                        activeTable = sysResourceTable;

                        // only removeResource and show button for resources
                        toolbar.add(showResourceButton);
                        firstButton = showResourceButton;
                        toolbar.add(new Box.Filler(d0, d0, d0));
                        toolbar.add(removeResourceButton);
                        showResourceButton.setNextFocusableComponent(
                                removeResourceButton);
                        lastButton=removeResourceButton;
                        break;
                }
                popup = new JPopupMenu();
                popup.addPopupMenuListener(CacheViewer.this);

                switch (index) {
                    case 0:
                    case 3:
                       popup.add(runOnlineMI);
                       popup.add(runOfflineMI);
                       popup.addSeparator();
                       popup.add(installMI);
                       popup.add(removeMI);
                       popup.addSeparator();
                       popup.add(showMI);
                       popup.add(homeMI);
                       break;
                    case 1:
                    case 4:
                       popup.add(showResourceMI);
                       popup.add(removeMI);
                       break;
                    case 2:
                       popup.add(importMI);
                       popup.add(removeMI);
                       break;
                }


                final JScrollPane sp = new JScrollPane(activeTable);

                toolbar.add(new Box.Filler(d0, d0, d0));
                toolbar.add(new remainingSpacer());
                toolbar.add(sizeLabel);
                toolbar.add(new Box.Filler(d1, d1, d1));
                tablePanel.removeAll();
                tablePanel.add(sp); 
                CacheViewer.this.enableButtons();
                closeButton.setNextFocusableComponent(viewComboBox);
                viewComboBox.setNextFocusableComponent(firstButton);
                lastButton.setNextFocusableComponent(activeTable);

                CacheViewer.this.validate();
                CacheViewer.this.repaint();
            }
        });

    }

    private class remainingSpacer extends JComponent {
        public Dimension getPreferredSize() {
             Dimension d = super.getPreferredSize();
             Container parent = getParent();
             int width = parent.getWidth();
             int otherWidth = 0;
             Component [] child = parent.getComponents();
             for (int i=0; i<child.length; i++) {
                if (!this.equals(child[i])) {
                    Dimension siz = child[i].getPreferredSize();
                    otherWidth += siz.width;
                }
            }
            if (width > otherWidth) {
                d.width = (width - otherWidth);
            }
            return d;
        }
    }
        
    private void initComponents() {

        JPanel bottomPanel = new JPanel(new BorderLayout());
        JPanel bottomRight = new JPanel();

        toolbar = new JToolBar();
        toolbar.setBorderPainted(false);
        toolbar.setFloatable(false);
        toolbar.setMargin(new Insets(2, 2, 0, 0));
        toolbar.setRollover(true);

        String id = UIManager.getLookAndFeel().getID();
        if (id.startsWith("Windows")) {
            leadinSpace = 27;
            minSpace = 4;
            noSeparator = false;
        } else {
            leadinSpace = 30;
            noSeparator = true;
            if (id.startsWith("GTK")) {
                minSpace = 2;
            } else {
                minSpace = 10;
            }
        }

        ActionListener runOnlineActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                runApplication(true);
            }
        };

        ActionListener runOfflineActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                runApplication(false);
            }
        };

        ActionListener installActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                installApplication();
            }
        };

        ActionListener importActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                importApplication();
            }
        };

        ActionListener removeActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                delete();
            }
        };

        ActionListener showActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (activeTable == jnlpTable || activeTable == sysJnlpTable) {
                    showApplication();
                } else if (activeTable == resourceTable || 
                           activeTable == sysResourceTable) {
                    showInformation();
                }
            }
        };

        ActionListener homeActionListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                browseApplication();
            }
        };

        runOnlineMI = createMI("viewer.run.online.menuitem", 
            runOnlineActionListener);
        runOfflineMI = createMI("viewer.run.offline.menuitem", 
            runOfflineActionListener);
        installMI = createMI("viewer.install.menuitem", 
            installActionListener);
        removeMI = createMI("viewer.remove.menuitem", 
            removeActionListener);
        showMI = createMI("viewer.show.menuitem", showActionListener);
        showResourceMI = createMI("viewer.show.resource.menuitem", 
            showActionListener);
        homeMI = createMI("viewer.home.menuitem", homeActionListener);
        importMI = createMI("viewer.import.menuitem", importActionListener);
        
        boolean hasSystemCache = ((Config.getSystemCacheDirectory() != null) &&
                                  (!Environment.isSystemCacheMode()));
        String [] views = (hasSystemCache) ? new String[5] : new String[3];
        views[0] = getResource("viewer.view.jnlp");
        views[1] = getResource("viewer.view.res");
        views[2] = getResource("viewer.view.import");
        if (hasSystemCache) {
            views[3] = getResource("viewer.sys.view.jnlp");
            views[4] = getResource("viewer.sys.view.res");
        }
        viewComboBox = new JComboBox(views){
            public Dimension getMinimumSize() {
                return getPreferredSize();
            }
            public Dimension getMaximumSize() {
                return getPreferredSize();
            }
        };
        viewComboBox.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                refresh();
            }
        });

        viewLabel = new JLabel(getResource("viewer.view.label"));

        runButton = createRunButton("viewer.run.online");

        importButton = createImageButton("viewer.import", 0);
        importButton.addActionListener(importActionListener);

        removeButton = createImageButton("viewer.remove", 0);
        removeButton.addActionListener(removeActionListener);

        removeResourceButton = createImageButton("viewer.remove.res", 0);
        removeResourceButton.addActionListener(removeActionListener);

        removeRemovedButton = createImageButton("viewer.remove.removed", 0);
        removeRemovedButton.addActionListener(removeActionListener);

        installButton = createImageButton("viewer.install", 0);
        installButton.addActionListener(installActionListener);

        showButton = createImageButton("viewer.show", 0);
        showButton.addActionListener(showActionListener);

        showResourceButton = createImageButton("viewer.info", 0);
        showResourceButton.addActionListener(showActionListener);

        homeButton = createImageButton("viewer.home", 0);
        homeButton.addActionListener(homeActionListener);

        tablePanel.setBorder(BorderFactory.createEmptyBorder(12, 12, 6, 12));

        closeButton = new JButton(getResource("viewer.close"));
        closeButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                exitViewer();
            }
        });

        addCancelAction();

        JButton helpButton = new JButton(getResource("viewer.help"));
        helpButton.setMnemonic(
            ResourceManager.getVKCode("viewer.help.mnemonic"));
        helpButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                help();
            }
        });
        bottomRight.add(closeButton);
        // bottomRight.add(helpButton);
        bottomPanel.add(bottomRight, BorderLayout.EAST);
        bottomPanel.setBorder(BorderFactory.createEmptyBorder(00, 8, 8, 8));

        JPanel p1 = new JPanel(new BorderLayout());
        p1.add(toolbar, BorderLayout.CENTER);
        p1.add(new JSeparator(), BorderLayout.SOUTH);

        JPanel p2 = new JPanel(new BorderLayout());
        p2.add(tablePanel, BorderLayout.CENTER);
        p2.add(bottomPanel, BorderLayout.SOUTH);

        getContentPane().add(p1, BorderLayout.NORTH);
        getContentPane().add(p2, BorderLayout.CENTER);

        runPopup = new JPopupMenu();
        onlineMI = runPopup.add(getResource("viewer.run.online.mi"));
        onlineMI.setEnabled(false);
        onlineMI.setIcon(getIcon("viewer.run.online.mi.icon"));
        onlineMI.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                runApplication(true);
            }
        });
        offlineMI = runPopup.add(getResource("viewer.run.offline.mi"));
        offlineMI.setEnabled(false);
        offlineMI.setIcon(getIcon("viewer.run.offline.mi.icon"));
        offlineMI.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                runApplication(false);
            }
        });

        runPopup.addPopupMenuListener(this);
        runPopup.addMenuKeyListener(new MenuKeyListener() {
            public void menuKeyPressed(MenuKeyEvent e) {
                int key = e.getKeyCode();
                if (key == KeyEvent.VK_TAB ||
                    key == KeyEvent.VK_RIGHT || key == KeyEvent.VK_LEFT) {
                    runPopup.setVisible(false);
                }
            }
            public void menuKeyReleased(MenuKeyEvent e) {
            }
            public void menuKeyTyped(MenuKeyEvent e) {
            }
        });

        // ok construct the first table, and if not empty - leave focus there
        jnlpTable = new CacheTable(CacheViewer.this,
                                   CacheTable.JNLP_TYPE, false);
        if (jnlpTable.getModel().getRowCount() != 0) {
            viewComboBox.setSelectedIndex(0);
            focusLater(jnlpTable);
            return;
        } 

        // construct the second table, and if not empty - leave focus there
        resourceTable = new CacheTable(CacheViewer.this,
                                       CacheTable.RESOURCE_TYPE, false);
        if (resourceTable.getModel().getRowCount() != 0) {
            viewComboBox.setSelectedIndex(1);
            focusLater(resourceTable);
            return;
        }

        // construct the third table, and if not empty - leave focus there
        importTable = new CacheTable(CacheViewer.this,
                                     CacheTable.DELETED_TYPE, false);
        if (importTable.getModel().getRowCount() != 0) {
            viewComboBox.setSelectedIndex(2);
            focusLater(importTable);
            return;
        }

        // all user tables empty - default focus to close
        focusLater(closeButton);
        refresh();
    }

    private void focusLater(final Component component) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                component.requestFocus();
            }
        });
    }
                

    public void addCancelAction() {
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),"cancelViewer");
        getRootPane().getActionMap().put("cancelViewer", new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
              exitViewer();
            }
        });
    }

    public void removeCancelAction() {
        InputMap im = getRootPane().getInputMap(
            JComponent.WHEN_IN_FOCUSED_WINDOW);
        KeyStroke esc = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0);
        if (im != null) {
            im.remove(esc);
            getRootPane().setInputMap(
                JComponent.WHEN_IN_FOCUSED_WINDOW, im);
        }
    }

    public void popupMenuCanceled(PopupMenuEvent e) {
    }

    public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {
        addCancelAction();
    }

    public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
        removeCancelAction();
    }

    void runApplication() {
        CacheObject co = getSelectedCacheObject();
        LaunchDesc ld = co.getLaunchDesc();
        // try to run online if there is an href, try offline if not
        runApplication(ld.getLocation() != null);
    }

    void delete() {
        if (activeTable == jnlpTable || activeTable == sysJnlpTable) {
            removeApplications();
        } else if (activeTable == resourceTable || 
                   activeTable == sysResourceTable) {
            removeResources();
        } else if (activeTable == importTable) {
            removeRemoved();
        }
    }

    class VSeparator extends JSeparator {
        public VSeparator() {
            super(SwingConstants.VERTICAL);
        }
        public Dimension getPreferredSize() {
            Dimension  d = getUI().getPreferredSize(this);
            d.height = 20;
            return d;
        }
        public Dimension getMaximumSize() {
            return getPreferredSize();
        }
    }
        
    void runApplication(boolean online) {
        try {
            CacheObject co = getSelectedCacheObject();
            if (co != null) {
                LaunchDesc ld = co.getLaunchDesc();
                if (ld != null && ld.isApplicationDescriptor()) {
                    if ((online && ld.getLocation() != null) ||
                        (ld.getInformation().supportsOfflineOperation())) {
                        String cmd[] = new String[4];
                        cmd[0] = Config.getJavawsCommand();
                        cmd[1] = ((online) ? "-online" : "-offline");
			cmd[2] = "-localfile";
                        cmd[3] = co.getJnlpFile().getPath();
                        Process p = Runtime.getRuntime().exec(cmd);
                        traceStream(p.getInputStream());
                        traceStream(p.getErrorStream());
                    }
                }
            }
        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }

    void removeApplications() {
        removeResources();
    }

    private static final int WAIT_REMOVE = 0;
    private static final int WAIT_IMPORT = 1;

    void importApplication() {
        if (activeTable == importTable) try {
            CacheObject [] co = getSelectedCacheObjects();
            if (co.length > 0) {
                showWaitDialog(co, WAIT_IMPORT);
            }
        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }


    void installApplication() {
        LocalInstallHandler lih = LocalInstallHandler.getInstance();
        CacheObject co = getSelectedCacheObject();
        if ((co != null) && lih.isLocalInstallSupported()) {
            LocalApplicationProperties lap = co.getLocalApplicationProperties();
            lap.refreshIfNecessary();
            if (activeTable == sysJnlpTable) {
                // system cache
                if (!lap.isLocallyInstalledSystem()) {
                    // no system install yet.
                }
            } else {
                // user shortcut
                if (!lap.isLocallyInstalled() || 
                    !lih.isShortcutExists(lap)) {
                    LaunchDesc ld = co.getLaunchDesc();
                    lih.uninstallShortcuts(ld, lap);
                    lih.installShortcuts(ld, lap);
                    enableButtons();
                }
            }
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

    private void showApplication() {
        CacheObject co = getSelectedCacheObject();
        if (co != null) {
            LaunchDesc ld = co.getLaunchDesc();
            if (ld != null) {
                String str = ld.toString();
                UIFactory.showContentDialog(this, new AppInfo(), 
                        getResource("viewer.show.title"),
                        str,
                        true,
                        getResource("common.ok_btn"),
                        null);
            }
        }
    }

    void showInformation() {
        CacheObject co = getSelectedCacheObject();
        if (co != null) {
            switch (co.getObjectType()) {
                case CacheObject.TYPE_JNLP:
                    showApplication();
                    break;

                case CacheObject.TYPE_JAR:
                case CacheObject.TYPE_CLASS:
                    showResource(co);
                    break;

                case CacheObject.TYPE_IMAGE:
                    showImage(co);
                    break;

                default:
                    break;
            }
        }
    }

    void showResource(CacheObject co) {
    }

    void showImage(CacheObject co) {
    }

    void removeRemoved() {
        CacheObject [] co = getSelectedCacheObjects();
        for (int i=0; i<co.length; i++) {
            Cache.removeRemovedApp(co[i].getDeletedUrl(), 
                                   co[i].getDeletedTitle());
        }
    }

    void removeResources() {
        try {
            CacheObject [] co = getSelectedCacheObjects();
            showWaitDialog(co, WAIT_REMOVE);
        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }


    public void showWaitDialog(final CacheObject[] co, final int type) {
        String key;
        String title;
        boolean includeVendor;
        if (type == WAIT_REMOVE) {
            includeVendor = true;
            key = (co.length > 1) ?
                "viewer.wait.remove" : "viewer.wait.remove.single";
            title = getResource("viewer.wait.remove.title");
        } else {
            includeVendor = false;
            key = (co.length > 1) ?
            "viewer.wait.import" : "viewer.wait.import.single";
            title = getResource("viewer.wait.import.title");
        }
        String msg = getResource(key);
        final DownloadWindow dw = new DownloadWindow();
        dw.initialize(this, null, false, false, includeVendor);
        dw.setHeading(msg, false);
        dw.setTitle(title);
        dw.setProgressBarVisible(true);

        // for more than one application, we show progress as percent of 
        // total applications completed, but for a single application, we have
        // no progress feedback, so we show the indeterminate progress bar.
        dw.setIndeterminate(co.length == 1);
        dw.setProgressBarValue(0);
        dw.setVisible(true);
                
        Thread thread = new Thread(new Runnable() {
            public void run() {
                for (int i=0; i<co.length; i++) {
                    try {
                        if (type == WAIT_REMOVE) {
                            boolean isSys = (activeTable == sysJnlpTable ||
                                             activeTable == sysResourceTable);
                            if (isSys) {
                                Environment.setSystemCacheMode(true);
                                Cache.reset();
                            }
                            LaunchDesc ld = co[i].getLaunchDesc();
                            CacheEntry ce = co[i].getCE();
                            String title = co[i].getNameString();
                            String url = co[i].getUrlString();
                            if (ld == null) {
                                dw.setApplication(title, null, url);
                                // remove resource from the cache.
                                Cache.removeAllCacheEntries(ce);
                            } else {
                                // remove JNLP file (and it's resources) 
                                title = ld.getInformation().getTitle();
                                String ven = ld.getInformation().getVendor();
                                dw.setApplication(title, ven, url);
                                CacheUtil.remove(ce, ld);
                            }
                            if (isSys) {
                                Environment.setSystemCacheMode(false);
                                Cache.reset();
                            }
                        } else {
                            String title = co[i].getDeletedTitle();
                            String url = co[i].getDeletedUrl();
                            dw.setApplication(title, null, url);
                            String cmd[] = new String[5];
                            cmd[0] = Config.getJavawsCommand();
                            cmd[1] = "-wait";
                            cmd[2] = "-quiet";
                            cmd[3] = "-import";
                            cmd[4] = url;
                            Process p = Runtime.getRuntime().exec(cmd);
                            traceStream(p.getInputStream());
                            traceStream(p.getErrorStream());
                            int ret = p.waitFor();
                        }
                        if (!dw.isVisible()) {
                            break;
                        } else {
                            int percent = ((i + 1) * 100) / co.length;
                            dw.setProgressBarValue(percent);
                        }
                    } catch (Throwable t) {
                        Trace.ignored(t);
                    }
                }
                dw.setVisible(false);
                enableButtons();
            }
        });

        thread.start();
    }

    private void traceStream(final InputStream is) {
        new Thread(new Runnable() {
            public void run() {
                byte[] buffer = new byte[1024];
                try {
                    int n=0;
                    while(n != -1) {
                        n = is.read(buffer);
                        if (n > 0) {
			    // ignore output, it should be in log file
                            // of the child javaws
                        } else if (n == 0) {
                            try {
                                Thread.sleep(200);
                            } catch (Exception e) {
                            }
                        }
                    }
                } catch (Exception ioe) { /* just ignore */ }
            }
        }).start();
    }

    void help() {
    }

    private CacheObject getSelectedCacheObject() {
        int selected[];
        selected =  activeTable.getSelectedRows();
        if (selected.length == 1) {
            return activeTable.getCacheObject(selected[0]);
        }
        return null;
    }

    private CacheObject[] getSelectedCacheObjects() {
        int selected[];
        selected = activeTable.getSelectedRows();
        int len = activeTable.getRowCount();
        for (int i=0; i<selected.length; i++) {
            if (selected[i] >= len) {
                Trace.println("Bug in JTable ?, getRowCount() = " + len +
                    " , but getSelectedRows() contains: " + selected[i],
                    TraceLevel.BASIC);
                return new CacheObject[0];
            }
        }

        CacheObject [] selectedObjects = new CacheObject[selected.length];
        for (int i=0; i<selected.length; i++) {
            selectedObjects[i] = activeTable.getCacheObject(selected[i]);
        }
        return selectedObjects;
    }

    private void showDocument(final URL page) {
        new Thread(new Runnable() {
            public void run() {
                BrowserSupport.showDocument(page);
            }
        }).start();
    }

    private String getResource(String key) {
        return ResourceManager.getMessage(key);
    }

    private ImageIcon dummy = null;
    private Icon getDummyIcon() {
        if (dummy == null) {
            try {
                dummy = ResourceManager.getIcon("java32.image");
            } catch (Throwable t) {}
            if (dummy == null) {
                dummy = new ImageIcon();
            }
            Image im = dummy.getImage().getScaledInstance(
                                        20, 20, Image.SCALE_DEFAULT);
            dummy.setImage(im);
        }
        return dummy;
    }

    private Icon getIcon(String key) {
        try {
            return ResourceManager.getIcon(key);
        } catch (Throwable t) {
            return getDummyIcon();
        }
    }

    public JButton createRunButton(String key) {
        final JButton b = createImageButton(key, 8);

        b.addMouseListener(new MouseAdapter() {
            boolean clicked = false;
            javax.swing.Timer t;
            public void mouseClicked(MouseEvent e) {
                if (!runPopup.isVisible()) {
                    clicked = true;
                }
            }
            public void mousePressed(MouseEvent e) {
                clicked = false;
                t = new javax.swing.Timer(500, new ActionListener() {
                    public void actionPerformed(ActionEvent ae) {
                        if (!clicked && runButton.isEnabled()) {
                            runPopup.show(b, 0, b.getHeight());
                            // Popupmenu have swallowed mouse release event,
                            // so we need set button state explicitly.
                            b.getModel().setPressed(false);
                        }
                    }
                });
                t.setRepeats(false);
                t.start();
            }
        });

        b.addKeyListener(new KeyAdapter() {
             public void keyPressed (KeyEvent keyEvent) {
                if (keyEvent.getKeyCode()== KeyEvent.VK_DOWN ||
                    keyEvent.getKeyCode()== KeyEvent.VK_KP_DOWN) {
                    javax.swing.Timer kt;
                    kt = new javax.swing.Timer(50, new ActionListener() {
                        public void actionPerformed(ActionEvent ae) {
                            if (!runPopup.isVisible() && 
                                runButton.isEnabled()) {
                                runPopup.show(b, 0, b.getHeight());
                            }
                        }
                    });
                    kt.setRepeats(false);
                    kt.start();
                }
            }
        });

        b.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (!runPopup.isVisible()) {
                    runApplication();
                }
            }
        });

        return b;
    }
        
    private JButton createImageButton(String key, int additionalWidth) {
        final int w = 32 + additionalWidth;
        final int h = 32;
        JButton b = new JButton() {
            public Dimension getPreferredSize(){ return new Dimension(w,h); };
            public Dimension getMinimumSize() { return new Dimension(w,h); };
            public Dimension getMaximumSize() { return new Dimension(w,h); };
        };
        setButtonIcons(b, key + ".icon");
        b.setToolTipText(getResource(key + ".tooltip"));
        return b;
    }

    private void setButtonIcons(JButton button, String key) {
        Icon icons [] = ResourceManager.getIcons(key);

        button.setIcon((icons[0] != null) ?  icons[0] : getDummyIcon());
        if (icons[1] != null) {
            button.setPressedIcon(icons[1]);
        }
        if (icons[2] != null) {
            button.setDisabledIcon(icons[2]);
        }
        if (icons[3] != null) {
            button.setRolloverIcon(icons[3]);
        }
    }

    private JMenuItem createMI(String key, ActionListener al) {
        JMenuItem mi = new JMenuItem(ResourceManager.getMessage(key));

        // add a mnemonic if one there
        String keyMnemonic = key + ".mnemonic";
        String mnemonic = ResourceManager.getMessage(keyMnemonic);
        if (!mnemonic.equals(keyMnemonic)) {
            mi.setMnemonic(ResourceManager.getVKCode(keyMnemonic));
        }

        mi.addActionListener(al);
        return mi;
    }

    private final static String BOUNDS_PROPERTY_KEY =
        "deployment.javaws.viewer.bounds";

    private void exitViewer() {
        Rectangle r = getBounds();
        Config.setProperty(BOUNDS_PROPERTY_KEY,
                           ""+r.x+ ","+r.y+ ","+r.width+ ","+r.height);
        // don't save any propertys pending in control panel - instead
        // save bounds only when user hits "Apply"
        if (!wasDirty) {
            Config.storeIfDirty();
        }

        setVisible(false);
        dispose();
    }

    public void valueChanged(ListSelectionEvent e) {
        enableButtons();
    }

    public void popupApplicationMenu(Component tree, int x, int y) {
        CacheObject co = getSelectedCacheObject();
        if (co != null) {
            popup.show(tree, x, y);
        }
    }

    private boolean canWriteSys() {
        String sysDir = Config.getSystemCacheDirectory();
        if (sysDir != null) try {
            File sys = new File(sysDir);
            return sys.canWrite();
        } catch (Exception e) {
        }
        return false;
    }

    public void enableButtons() {
        CacheObject [] co = getSelectedCacheObjects();
        boolean one = (co.length == 1);
        boolean any = (co.length > 0);
        boolean nonSys = ((activeTable != sysResourceTable) &&
                          (activeTable != sysJnlpTable));

        // enable these buttons when exactaly 1 item is selected.
        showButton.setEnabled(one);
        showMI.setEnabled(one);

        // enable these when any items are selected
        removeRemovedButton.setEnabled(any && nonSys);
        importButton.setEnabled(any);
        importMI.setEnabled(any);
        removeButton.setEnabled(any && (nonSys || canWriteSys()));
        removeMI.setEnabled(any && nonSys);
        removeResourceButton.setEnabled(any && (nonSys || canWriteSys()));

        // these depend on what is selected
        runButton.setEnabled(false);
        runOnlineMI.setEnabled(false);
        runOfflineMI.setEnabled(false);
        installButton.setEnabled(false); 
        installMI.setEnabled(false);
        homeButton.setEnabled(false);
        homeMI.setEnabled(false);
        showResourceButton.setEnabled(false);
        showResourceMI.setEnabled(false);
        onlineMI.setEnabled(false);
        offlineMI.setEnabled(false);

        if (one) {
            LaunchDesc ld = co[0].getLaunchDesc();
            if (ld != null) {
                if (ld.isApplicationDescriptor()) {
                    if (ld.getLocation() != null || 
                        ld.getInformation().supportsOfflineOperation()) {
                        // ok can launch online
                        runButton.setEnabled(true);
                        onlineMI.setEnabled(true);
                        runOnlineMI.setEnabled(true);
                        setButtonIcons(runButton, "viewer.run.online.icon");
                        runButton.setToolTipText(
                            getResource("viewer.run.online.tooltip"));
                    }
                    if (ld.getInformation().supportsOfflineOperation()) {
                        // ok can launch offline
                        runButton.setEnabled(true);
                        offlineMI.setEnabled(true);
                        runOfflineMI.setEnabled(true);
                        if (ld.getLocation() == null) {
                            // offline is default
                            setButtonIcons(runButton,"viewer.run.offline.icon");
                            runButton.setToolTipText(getResource(
                                "viewer.run.offline.tooltip"));
                        }
                    }
                    LocalInstallHandler lih = LocalInstallHandler.getInstance();
                    if (lih.isLocalInstallSupported()) {
                            LocalApplicationProperties lap = 
                            co[0].getLocalApplicationProperties();
                        lap.refreshIfNecessary();
                        if (!lih.isShortcutExists(lap)) {
                            if (activeTable == jnlpTable) {
                                ShortcutDesc 
                                    sd = ld.getInformation().getShortcut();
                                if (sd == null || 
                                    sd.getMenu() || sd.getDesktop()) {
                                    installButton.setEnabled(true);
                                    installMI.setEnabled(true);
                                }
                            }
                        }
                    }
                }
                if (ld.getInformation().getHome() != null) {
                    homeButton.setEnabled(true);
                    homeMI.setEnabled(true);
                }
                showResourceButton.setEnabled(true);
                showResourceMI.setEnabled(true);
            }        
        }
        activeTable.setEnabled(activeTable.getRowCount() > 0);
    }

    private final int SLEEP_DELAY = 2000;
    private void startWatchers() {

        new Thread(new Runnable() {
            public void run() {

                long lastTimeUser = Cache.getLastAccessed(false);

                long lastTimeSys = Cache.getLastAccessed(true);
                long lastTimeDel =  0;

                String delPath = Cache.getRemovePath();
                File delFile = new File(delPath);
                if ((delFile != null) && (delFile.exists())) {
                    lastTimeDel = delFile.lastModified();
                }

                do {
                    try {
                        Thread.sleep(SLEEP_DELAY);
                    } catch (InterruptedException ie) {
                        Trace.ignored(ie);
                        continue;
                    }
                    long newTimeUser = Cache.getLastAccessed(false);

                    long newTimeSys = Cache.getLastAccessed(true);
                    long newTimeDel = 0;
                    if ((delFile != null) && (delFile.exists())) {
                        newTimeDel = delFile.lastModified();
                    }
    
                    if (newTimeUser != lastTimeUser) {
                        lastTimeUser = newTimeUser;
   
                        SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                if (jnlpTable != null) {
                                    jnlpTable.reset();
                                }
                                if (resourceTable != null) {

                                    resourceTable.reset();
                                }
                                enableButtons();
                                sizeLabel.setText(
                                    activeTable.getSizeLabelText());
                            }        
                        });
                    }
                    if (newTimeSys != lastTimeSys) {
                        lastTimeSys = newTimeSys;
    
                        SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                if (sysJnlpTable != null) {
                                    sysJnlpTable.reset();
                                }
                                if (sysResourceTable != null) {
                                    sysResourceTable.reset();
                                }
                                CacheViewer.this.enableButtons();
                                sizeLabel.setText(
                                    activeTable.getSizeLabelText());
                            }
                        });
                    }
                    if (newTimeDel != lastTimeDel) {
                        lastTimeDel = newTimeDel;
                        SwingUtilities.invokeLater(new Runnable() {
                            public void run() {
                                if (importTable != null) {
                                    importTable.reset();
                                }
                                CacheViewer.this.enableButtons();
                            }
                        });
                    }

                } while (isShowing());
            }

       }).start();
    }


    public static void showCacheViewer(JFrame parent) {

	// initialize cache
	Cache.reset();
	
        // need to set up proxys so icons can be downloaded.
        Main.initializeExecutionEnvironment();

        final CacheViewer viewer = new CacheViewer(parent);
        viewer.setModal(true);

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
                viewer.setBounds(xywh[0], xywh[1], xywh[2], xywh[3]);
            }
        } else {
            viewer.setBounds(100, 100, 720, 360);
            UIFactory.placeWindow(viewer);
        }

        viewer.startWatchers();
        viewer.setVisible(true);

    }
        
}



