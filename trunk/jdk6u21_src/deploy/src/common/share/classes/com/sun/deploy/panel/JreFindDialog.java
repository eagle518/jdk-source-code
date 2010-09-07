/*
 * @(#)JreFindDialog.java	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.beans.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.resources.ResourceManager;

/**
 * Searchs for JRE's from a given location. The user is prompted for the
 * directory to search from, any file with java or javaw.exe is considered
 * a valid JRE and is executed to determine the version.
 *
 * @version 1.19 11/29/01
 */
public class JreFindDialog extends JDialog implements ActionListener {

    private final JButton _cancelButton = new JButton(
		ResourceManager.getMessage("find.cancelButton"));
    private final JButton _prevButton = new JButton(
		ResourceManager.getMessage("find.prevButton"));
    private final JButton _nextButton = new JButton(
		ResourceManager.getMessage("find.nextButton"));
    
    private JLabel _titleLabel;
    private JComponent _mainComponent = null;
    private JPanel _buttonPanel;

    private File _directory;
    private int _state;
    private GridBagConstraints _constraints;
    private JTextArea _ta;
    private PathChooser _pc;
    private Dimension _panelPref; 

    /** JREs that are to be returned. */
    private JREInfo[] _jres;

    public JreFindDialog(JDialog parent){
        super(parent, ResourceManager.getMessage("find.dialog.title"), true);
        initComponents();
    }

    private void initComponents(){
        setJREs(null);
 
        _titleLabel = new JLabel(ResourceManager.getMessage("find.title"));
        setFont(_titleLabel, Font.ITALIC, 18);
	
        _cancelButton.addActionListener(this);
        _nextButton.addActionListener(this);
        _prevButton.addActionListener(this);
        
        _cancelButton.setMnemonic(ResourceManager.getVKCode("find.cancelButtonMnemonic"));
        _prevButton.setMnemonic(ResourceManager.getVKCode("find.prevButtonMnemonic"));
        _nextButton.setMnemonic(ResourceManager.getVKCode("find.nextButtonMnemonic"));

        _buttonPanel = new JPanel(new BorderLayout());
        JPanel p = new JPanel();
        p.add(_cancelButton);
        p.add(_prevButton);
        p.add(_nextButton);
        _buttonPanel.add(p, BorderLayout.EAST);
 
        String description = ResourceManager.getMessage("find.intro");

        _ta = new JSmartTextArea(description);
        _pc = new PathChooser(); 
        // Save the biggest panel's size to avoid resize the dialog when fit 
        // different panel. See bug 5091138:"JRE Finder" dialog size is not 
        // maintaned in JavaControlPanel.
        _panelPref = _pc.getUI().getPreferredSize(_pc);
        _directory = null;

        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                setVisible(false);
		dispose();
            }
        };
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        setState(0);
    }

    /**
     * Searches for JREs by prompting the user for a place to search from
     * and then searching for valid java programs. A null return value
     * indicates none were found.
     */
    public static JREInfo[] search(JDialog c) {
        JreFindDialog find = new JreFindDialog(c);
	find.show();
	return find._jres;
    }

    private static void setFont(JComponent c, int style, int size) {
	int sizeInUse = c.getFont().getSize();
	size = Math.max(sizeInUse, size);
	c.setFont(c.getFont().deriveFont(style, size));
    }

    public void show() {

	getContentPane().setLayout(new GridBagLayout());

        GridBagConstraints cons = new GridBagConstraints();
        cons.gridx = cons.gridy = 0;
        cons.weightx = cons.weighty = 0;
        cons.fill = GridBagConstraints.NONE;
        cons.anchor = GridBagConstraints.CENTER;
        cons.insets = new Insets(5, 5, 5, 5);
        getContentPane().add(_titleLabel, cons);

        cons.gridy = 2;
        cons.weightx = cons.weighty = 0;
        cons.fill = GridBagConstraints.NONE;
        cons.anchor = GridBagConstraints.EAST;
	getContentPane().add(_buttonPanel, cons);

        cons.gridy = 1;
	cons.weightx = cons.weighty = 1;
        cons.fill = GridBagConstraints.BOTH;
        cons.anchor = GridBagConstraints.WEST;
	if (_mainComponent != null) {
            getContentPane().add(_mainComponent, cons);
	}
	_constraints = cons;
 
	pack();
	// Constrain the size.
	Dimension size = getSize();

	size.width = Math.max(size.width, 500);
	size.height = Math.max(size.height, 420);
	Dimension ss = Toolkit.getDefaultToolkit().getScreenSize();
	setBounds((ss.width - size.width) / 2,
			 (ss.height - size.height) / 2, size.width,
			 size.height);

	UIFactory.placeWindow(this);

        addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                int cancel = ResourceManager.getVKCode("find.cancelButtonMnemonic");
                if (e.getKeyCode() == cancel) {
                    _cancelButton.doClick();
                }
            }
        });

	super.show();
	dispose();
	setState(-1);
    }

    /**
     * Cancels the current session.
     */
    protected void cancel() {
	setJREs(null);
	setVisible(false);
    }

    /**
     * Shows the next screen.
     */
    protected void next() {
	int state = getState();
	if (++state == 3) {
	    // We're done.
            setVisible(false);
	}
	else {
	    setState(state);
	}
    }

    /**
     * Shows the previous screen.
     */
    protected void previous() {
	int state = getState();
	setState(--state);
    }

    public void actionPerformed(ActionEvent e) {
	Object src = e.getSource();
	if (src.equals(_cancelButton)) {
	    cancel();
	} else if (src.equals(_prevButton)) {
	    previous();
	} else if (src.equals(_nextButton)) {
	    next();
	}
    }

    /**
     * Sets the currently selected directory to search from.
     */
    private void setDirectory(File file) {
	_directory = file;
	_nextButton.setEnabled(file != null);
    }

    /**
     * Returns the current directory to search from.
     */
    private File getDirectory() {
	return _directory;
    }

    /**
     * Sets the JREs that are to be returned.
     */
    private void setJREs(JREInfo[] jres) {
	_jres = jres;
    }

    /**
     * Sets the current state, updating the display accordingly.
     */
    private void setState(int newState) {
	_cancelButton.setEnabled(true);
	_prevButton.setEnabled(false);
	_nextButton.setEnabled(true);
	
	_state = newState;
	if (_mainComponent != null) {
	    getContentPane().remove(_mainComponent);
	    _mainComponent = null;
	}
	switch (_state) {
	    case 0:
		_mainComponent = _ta;
		_nextButton.setText(ResourceManager.getMessage("find.nextButton"));
		break;
	    case 1:
		_mainComponent = _pc;
		_pc.setCurrentDirectory(_directory);
		if(_directory==null)
		    _pc.updateDirectory();
		_prevButton.setEnabled(true);
		_nextButton.setText(ResourceManager.getMessage("find.nextButton"));
     		break;
	    case 2:
		SearchPanel s = new SearchPanel();
		_mainComponent = s;
		s.start();
		_nextButton.setText(ResourceManager.getMessage("find.finishButton"));
		break;
	    default:
    	        break;
	}
	
	if (_mainComponent != null) {
	    _mainComponent.setPreferredSize(_panelPref);
	    getContentPane().add(_mainComponent, _constraints);
	    _mainComponent.revalidate();
	    
            // Grow if necessary ...
            Dimension size = getSize();
            Dimension pref = getPreferredSize();
            size.width = Math.max(size.width, pref.width);
            size.height = Math.max(size.height, pref.height);
            setSize(size.width, size.height);
	}
	repaint();
    }

    /**
     * Returns the current state, which indicates what is currently being
     * displayed.
     */
    private int getState() {
	return _state;
    }

    /**
     * Controller to allow the user to select a directory. This is done
     * by way of a JFileChooser.
     */
    private class PathChooser extends JFileChooser {
    	 
	public PathChooser() {
	    super();
	    setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent ae) {
		    if (JFileChooser.APPROVE_SELECTION.equals
			(ae.getActionCommand())) {
			if (ae.getSource() instanceof JFileChooser) {
			    JFileChooser fc = (JFileChooser) ae.getSource();
			    File file = fc.getSelectedFile();
			    if (file != null && file.isDirectory()) {
				fc.setCurrentDirectory(file);
				setDirectory(file);
			    }
			}
			updateDirectory();
		    }
		    else if (JFileChooser.CANCEL_SELECTION.equals
			     (ae.getActionCommand())) {
			cancel();
		    }
		}
	    });
	    // Adds a change listener to update the directory as the selection
	    // changes.
	    addPropertyChangeListener(new PropertyChangeListener() {
		public void propertyChange(PropertyChangeEvent e) {
		    if (JFileChooser.SELECTED_FILE_CHANGED_PROPERTY.
			equals(e.getPropertyName()) ||
			JFileChooser.DIRECTORY_CHANGED_PROPERTY.
			equals(e.getPropertyName())) {
			updateDirectory();
		    }
		}
	    });
	    // Use reflection to invoke setControlButtonsAreShown as only
	    // available in 1.3.
	    try {
		Class[] showParams = new Class[] { boolean.class };
		Method showMethod = JFileChooser.class.getMethod
		    ("setControlButtonsAreShown", showParams);
		if (showMethod != null) {
		    Object[] args = new Object[] { Boolean.FALSE };
		    showMethod.invoke(this, args);
		}
	    }
	    catch (NoSuchMethodException nsme) {}
	    catch (InvocationTargetException ite) {}
	    catch (IllegalAccessException iae) {}
	    setDialogType(JFileChooser.OPEN_DIALOG);
	}

	private void updateDirectory() {
	    File file = getSelectedFile();
	    if (file == null) {
		file = getCurrentDirectory();
	    }
	    setDirectory(file);
	}
    }


    /**
     * Controller to do the searching. When start()ed, a Thread is spawned
     * to search all the files. If a file's name is java or javaw.exe it is
     * executed to determine the version. If succesfully executed the
     * path is added to a list.
     */
    private class SearchPanel extends JPanel implements ActionListener {
	/** Model for the list, will contain the paths. */
	private DefaultListModel model;
	/** List showing any found JREs. */
	private JList list;
	/** Shows the file currently being checked. */
	private JLabel searchLabel;
	/** Prefix before file name in searchLabel. */
	private String searchPrefix;
	/** Displays a descriptive title of what is happening. */
	private JLabel titleLabel;

	/** Handles the searching. */
	private Searcher searcher;
	/** Updates the state (searchLabel, next button) based on the
	 * searcher's state. */
	private javax.swing.Timer timer;

	/** Are we currently active. */
	private boolean active;

	public SearchPanel() {
            searchPrefix = ResourceManager.getMessage("find.searching.prefix");
            model = new DefaultListModel();
            list = new JList(model);
            list.setCellRenderer(new DefaultListCellRenderer() {
                public Component getListCellRendererComponent(
                          JList list, Object value, int index,
                          boolean isSelected, boolean cellHasFocus) {
                    if (value instanceof JREInfo) {
                        value = ((JREInfo)value).getPath();
                    }
                    return super.getListCellRendererComponent(
                        list, value, index, isSelected, cellHasFocus);
                }
            });
            list.addListSelectionListener(new ListSelectionListener() {
                public void valueChanged(ListSelectionEvent lse) {
                    updateJREs();
                }
            });
            searchLabel = new JLabel(" ");
	    JreFindDialog.setFont(searchLabel, Font.PLAIN, 12);
            titleLabel = new JLabel(
                ResourceManager.getMessage("find.searching.title"));
	    JreFindDialog.setFont(titleLabel, Font.PLAIN, 12);
            setLayout(new GridBagLayout());
 
            GridBagConstraints cons = new GridBagConstraints();
            cons.gridx = cons.gridy = 0;
            cons.weightx = 1;
            cons.weighty = 0;
            cons.fill = GridBagConstraints.HORIZONTAL;
            cons.insets = new Insets(5, 2, 0, 2);
            cons.gridy = 0;
            add(titleLabel, cons);
 
            cons.gridy = 2;
            add(searchLabel, cons);
 
            cons.gridy = 1;
            cons.weighty = 1;
            cons.fill = GridBagConstraints.BOTH;
            cons.insets.bottom = 0;
            add(new JScrollPane(list), cons);
	}

	public void start() {
	    active = true;
	    _nextButton.setEnabled(false);
	    _prevButton.setEnabled(true);
	    if (model != null) {
		model.removeAllElements();
	    }
	    // Create the timer top uddate the display
	    timer = new javax.swing.Timer(100, this);
	    timer.setRepeats(true);
	    timer.start();
	    // Start searching
	    searcher = new Searcher();
	    searcher.start(getDirectory());
	}

	public void stop() {
	    active = false;
	    stopSearching();
	}

	private void stopSearching() {
	    if (searcher != null) {
		// Stop the timer and stop searching.
		searcher.stop();
		searcher = null;
		timer.stop();
		timer = null;
	    }
	}

	/**
	 * Invoked as a result of the Timer. Updates the display based on
	 * the state of the Searcher to show what is is currently being
	 * searched. If the searcher is no longer searching, this calls
	 * stop.
	 */
	public void actionPerformed(ActionEvent ae) {
	    if (!searcher.isFinished()) {
		File f = searcher.getCurrentFile();
		if (f != null) {
		    searchLabel.setText(searchPrefix + f.getPath());
		}
		else {
		    searchLabel.setText(searchPrefix);
		}
	    }
	    else {
		if (model.getSize() > 0) {
		    titleLabel.setText(
			ResourceManager.getMessage("find.foundJREs.title"));
		    _nextButton.setEnabled(true);
		}
		else {
		    titleLabel.setText(
			ResourceManager.getMessage("find.noJREs.title"));
		    _nextButton.setEnabled(false);
		}		
		searchLabel.setText(" ");
		stopSearching();
	    }
	}

	private void updateJREs() {
	    if (!active) {
		return;
	    }
	    if (model != null) {
		int size = model.getSize();
		if (size > 0) {
		    _nextButton.setEnabled(true);
		    int[] selected = list.getSelectedIndices();
		    if (selected != null && selected.length > 0) {
			JREInfo jres[] = new JREInfo[selected.length];
			for (int counter = 0; counter < selected.length;
			     counter++) {
			    jres[counter] = (JREInfo) model.getElementAt(selected[counter]);
			}
			setJREs(jres);
		    }
		    else {
			JREInfo jres[] = new JREInfo[size];
			model.copyInto(jres);
			setJREs(jres);
		    }
		}
		else {
		    setJREs(null);
		    _nextButton.setEnabled(false);
		}
	    }
	    else {
		setJREs(null);
	    }
	}

        private boolean alreadyFound(JREInfo jre) {
            for (int i=0; i<model.getSize(); i++) {
		Object obj = model.get(i);
		if (obj != null && obj instanceof JREInfo) {
		   if (jre.getPath().equals(((JREInfo) obj).getPath())) {
			return true;
		   }
		}
	    }
	    return false;
        }
		   

	/**
	 * Indicates the Searcher has found a valid JRE.
	 */
	// Can come in on any thread.
	private void add(final Searcher searcher, final JREInfo jre) {
	    SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    if (SearchPanel.this.searcher == searcher) {
			if (!alreadyFound(jre)) {
			    model.addElement(jre);
			    updateJREs();
			}
		    }
		}
	    });
	}


	/**
	 * Handles the actual searching. This will spawn a thread to
	 * descend the file system looking for any files named
	 * java or javaw.exe. If the name is java/javaw.exe it is executed
	 * to determine the version. If the process returns 0 then
	 * <code>add</code> is invoked add the path with the specified
	 * version.
	 * <p>
	 * Searching can be stopped by invoking <code>stop</code>.
	 * The current file that is being searched can be determined
	 * by <code>getCurrentFile</code>.
	 * <p>
	 * If the child process doesn't finish executing in around 5 seconds,
	 * it is killed.
	 */
	private class Searcher implements Runnable {
	    /** File to start searching from. */
	    private File file;
	    /** When true, indicates the search should stop. */
	    private boolean stop;
	    /** Current file being checked. */
	    private File currentFile;
	    /** True indicates we're done. */
	    private boolean finished;

	    /**
	     * Starts the traversal, spawning the thread.
	     */
	    void start(File f) {
		file = f;
		new Thread(this).start();
		updateJREs();
	    }

	    /**
	     * Returns the current file being checked.
	     */
	    public File getCurrentFile() {
		return currentFile;
	    }

	    /**
	     * Stops the processing.
	     */
	    public void stop() {
		stop = true;
	    }

	    /**
	     * Runnable method to start the searching.
	     */
	    public void run() {
		check(file, false);
		finished = true;
	    }

	    /**
	     * Returns true if searching has finished.
	     */
	    public boolean isFinished() {
		return finished;
	    }

	    /**
	     * If <code>f</code> is a file, and is named java or javaw.exe,
	     * <code>getVersion</code> is invoked to obtain the version,
	     * followed by <code>add</code>. If <code>f</code> is a directory,
	     * this is recursively called for all the children.
	     */
	    private boolean check(File f, boolean foundOne) {
		currentFile = f;
		String fileName = f.getName();
		if (f.isFile() && (fileName.equals("java") ||
				   fileName.equals("javaw.exe"))) {
		    JREInfo version = getVersion(f, foundOne);
		    if (version != null) {
			add(Searcher.this, version);
			return true;
		    }
		}
		else if (!f.isFile()) {
		    String[] kids = f.list();
		    if (kids != null) {
                        java.util.Arrays.sort(kids);
			for (int counter = 0, maxCounter = kids.length;
		            counter < maxCounter && !stop; counter++) {
			    foundOne =
				check(new File(f, kids[counter]), foundOne);
			}
		    }
		}
		return foundOne;
	    }

	    /**
	     * Returns the version for the JVM at <code>javaPath</code>.
	     * This will return null if there is problem with executing
	     * the path. This gives the child process at most 15 seconds to
	     * execute before assuming it is bogus.
	     */
	    private JREInfo getVersion(File javaPath, boolean foundOne) {
		// Make sure its valid
		if (!isValidJavaPath(javaPath, foundOne)) {
		    return null;
		}
		try {
		    File f = javaPath.getCanonicalFile();
		    // add in jre osname and osarch
		    JREInfo jre = JreLocator.getVersion(f);
		    if (jre != null) {
                        // only allow jres 1.3+
                        if (jre.getPlatform().compareTo("1.3") < 0) {
                            return null;
                        }
		        jre.setOSName(Config.getOSName());
		        jre.setOSArch(Config.getOSArch());
		    }
		    return jre;
		} catch (IOException ioe) {
		    return null;
		}
	    }

	    /**
	     * Returns true if <code>path</code> is a valid path to check
	     * for a JRE.
	     */
	    private boolean isValidJavaPath(File path, boolean foundOne) {
		String parent = path.getParent();
		if (parent.endsWith(File.separator + "native_threads") ||
		    parent.endsWith(File.separator + "green_threads")) {
		    return false;
		}

		// if path is of the form x/jre/bin and either x/bin/java or 
		// x/bin/javaw.exe exists then return false only if you have 
		// already foundOne.   If you search from above x, 
		// you will find x/bin/java{.exe} before x/jre/bin/java{.exe}.
		String jreBin = File.separator + "jre" + File.separator +"bin";
		if (parent.endsWith(jreBin) && 
		    (parent.length() > jreBin.length()) && foundOne) {
		    
		    String binPath = parent.substring(0, parent.length() -
						      jreBin.length() + 1) +
			"bin" + File.separator;
		    File file = new File(binPath + "java");
		    if (file.exists() && file.isFile()) {
			return false;
		    }
		    file = new File(binPath + "javaw.exe");
		    if (file.exists() && file.isFile()) {
			return false;
		    }
		}
		// also reject the 64 bit sparc v9 version, this cannot work
		if (parent.endsWith("sparcv9")) {
		    return false;
		}
		return true;
	    }
	}
    }
}
