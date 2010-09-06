
/*
 * @(#)FilePane.java	1.22 04/06/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing;

import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.*;
import java.security.AccessController;
import java.security.AccessControlException;
import java.text.DateFormat;
import java.util.*;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.filechooser.*;
import javax.swing.plaf.*;
import javax.swing.plaf.basic.*;
import javax.swing.table.*;
import javax.swing.text.*;

import com.sun.java.swing.*;
import sun.awt.shell.*;
import sun.security.action.GetPropertyAction;

/**
 * <b>WARNING:</b> This class is an implementation detail and is only
 * public so that it can be used by two packages. You should NOT consider
 * this public API.
 * <p>
 * This component is intended to be used in a subclass of
 * javax.swing.plaf.basic.BasicFileChooserUI. It realies heavily on the
 * implementation of BasicFileChooserUI, and is intended to be API compatible
 * with earlier implementations of MetalFileChooserUI and WindowsFileChooserUI.
 * 
 * @version 1.22, 06/15/04
 * @author Leif Samuelsson
 */
public class FilePane extends JPanel implements PropertyChangeListener {
    // Constants for actions. These are used for the actions' ACTION_COMMAND_KEY
    // and as keys in the action maps for FilePane and the corresponding UI classes

    public final static String ACTION_APPROVE_SELECTION = "approveSelection";
    public final static String ACTION_CANCEL            = "cancelSelection";
    public final static String ACTION_EDIT_FILE_NAME    = "editFileName";
    public final static String ACTION_REFRESH           = "refresh";
    public final static String ACTION_CHANGE_TO_PARENT_DIRECTORY = "Go Up";
    public final static String ACTION_NEW_FOLDER        = "New Folder";
    public final static String ACTION_VIEW_LIST         = "viewTypeList";
    public final static String ACTION_VIEW_DETAILS      = "viewTypeDetails";

    private Action[] actions;

    // "enums" for setViewType()
    public  static final int VIEWTYPE_LIST     = 0;
    public  static final int VIEWTYPE_DETAILS  = 1;
    private static final int VIEWTYPE_COUNT    = 2;

    private int viewType = -1;
    private JPanel[] viewPanels = new JPanel[VIEWTYPE_COUNT];
    private JPanel currentViewPanel;
    private String[] viewTypeActionNames;

    private JPopupMenu contextMenu;
    private JMenu viewMenu;

    private String viewMenuLabelText;
    private String refreshActionLabelText;
    private String newFolderActionLabelText;

    private FocusListener editorFocusListener = new FocusAdapter() {
	public void focusLost(FocusEvent e) {
	    if (! e.isTemporary()) {
		applyEdit();
	    }
	}
    };

    private static FocusListener repaintListener = new FocusListener() {
        public void focusGained(FocusEvent fe) {
            repaintSelection(fe.getSource());
        }

        public void focusLost(FocusEvent fe) {
            repaintSelection(fe.getSource());
        }

        private void repaintSelection(Object source) {
            if (source instanceof JList) {
                repaintListSelection((JList)source);
            } else if (source instanceof JTable) {
                repaintTableSelection((JTable)source);
            }
        }
        
        private void repaintListSelection(JList list) {
            int[] indices = list.getSelectedIndices();
            for (int i : indices) {
                Rectangle bounds = list.getCellBounds(i, i);
                list.repaint(bounds);
            }
        }
        
        private void repaintTableSelection(JTable table) {
            int minRow = table.getSelectionModel().getMinSelectionIndex();
            int maxRow = table.getSelectionModel().getMaxSelectionIndex();
            if (minRow == -1 || maxRow == -1) {
                return;
            }

            int col0 = table.convertColumnIndexToView(COLUMN_FILENAME);

            Rectangle first = table.getCellRect(minRow, col0, false);
            Rectangle last = table.getCellRect(maxRow, col0, false);
            Rectangle dirty = first.union(last);
            table.repaint(dirty);
        }
    };

    private boolean smallIconsView = false;
    private Border  listViewBorder;
    private Color   listViewBackground;
    private boolean listViewWindowsStyle;
    private boolean readOnly;

    private ListSelectionModel listSelectionModel;
    private JList list;
    private JTable detailsTable;

    private static final int COLUMN_FILENAME = 0;
    private static final int COLUMN_FILESIZE = 1;
    private static final int COLUMN_FILETYPE = 2;
    private static final int COLUMN_FILEDATE = 3;
    private static final int COLUMN_FILEATTR = 4;
    private static final int COLUMN_COLCOUNT = 5;

    private int[] COLUMN_WIDTHS = { 150,  75,  130,  130,  40 };

    private String fileNameHeaderText = null;
    private String fileSizeHeaderText = null;
    private String fileTypeHeaderText = null;
    private String fileDateHeaderText = null;
    private String fileAttrHeaderText = null;

    // Provides a way to recognize a newly created folder, so it can
    // be selected when it appears in the model.
    private File newFolderFile;

    // Used for accessing methods in the corresponding UI class
    private FileChooserUIAccessor fileChooserUIAccessor;

    public FilePane(FileChooserUIAccessor fileChooserUIAccessor) {
	super(new BorderLayout());

	this.fileChooserUIAccessor = fileChooserUIAccessor;

	installDefaults();
	createActionMap();
    }

    protected JFileChooser getFileChooser() {
	return fileChooserUIAccessor.getFileChooser();
    }

    protected BasicDirectoryModel getModel() {
	return fileChooserUIAccessor.getModel();
    }

    public int getViewType() {
	return viewType;
    }

    public void setViewType(int viewType) {
	int oldValue = this.viewType;
	if (viewType == oldValue) {
	    return;
	}
	this.viewType = viewType;

	switch (viewType) {
	  case VIEWTYPE_LIST:
	    if (viewPanels[viewType] == null) {
		JPanel p = fileChooserUIAccessor.createList();
		if (p == null) {
		    p = createList();
		}
		setViewPanel(viewType, p);
	    }
	    list.setLayoutOrientation(JList.VERTICAL_WRAP);
	    break;

	  case VIEWTYPE_DETAILS:
	    if (viewPanels[viewType] == null) {
		JPanel p = fileChooserUIAccessor.createDetailsView();
		if (p == null) {
		    p = createDetailsView();
		}
		setViewPanel(viewType, p);
	    }
	    break;
	}
	JPanel oldViewPanel = currentViewPanel;
	currentViewPanel = viewPanels[viewType];
	if (currentViewPanel != oldViewPanel) {
	    if (oldViewPanel != null) {
		remove(oldViewPanel);
	    }
	    add(currentViewPanel, BorderLayout.CENTER);
	    revalidate();
	    repaint();
	}
	updateViewMenu();
	firePropertyChange("viewType", oldValue, viewType);
    }

    class ViewTypeAction extends AbstractAction {
	private int viewType;

	ViewTypeAction(int viewType) {
	    super(viewTypeActionNames[viewType]);
	    this.viewType = viewType;

	    String cmd;
	    switch (viewType) {
		case VIEWTYPE_LIST:    cmd = ACTION_VIEW_LIST;    break;
		case VIEWTYPE_DETAILS: cmd = ACTION_VIEW_DETAILS; break;
		default:               cmd = (String)getValue(Action.NAME);
	    }		
	    putValue(Action.ACTION_COMMAND_KEY, cmd);
	}

	public void actionPerformed(ActionEvent e) {
	    setViewType(viewType);
	}
    }

    public Action getViewTypeAction(int viewType) {
	return new ViewTypeAction(viewType);
    }

    private static void recursivelySetInheritsPopupMenu(Container container, boolean b) {
	if (container instanceof JComponent) {
	    ((JComponent)container).setInheritsPopupMenu(b);
	}
	int n = container.getComponentCount();
	for (int i = 0; i < n; i++) {
	    recursivelySetInheritsPopupMenu((Container)container.getComponent(i), b);
	}
    }

    public void setViewPanel(int viewType, JPanel viewPanel) {
	viewPanels[viewType] = viewPanel;
	recursivelySetInheritsPopupMenu(viewPanel, true);

	switch (viewType) {
	  case VIEWTYPE_LIST:
	    list = (JList)findChildComponent(viewPanels[viewType], JList.class);
	    if (listSelectionModel == null) {
		listSelectionModel = list.getSelectionModel();
		if (detailsTable != null) {
		    detailsTable.setSelectionModel(listSelectionModel); 
		}
	    } else {
		list.setSelectionModel(listSelectionModel); 
	    }
	    break;

	  case VIEWTYPE_DETAILS:
	    detailsTable = (JTable)findChildComponent(viewPanels[viewType], JTable.class);
	    detailsTable.setRowHeight(Math.max(detailsTable.getFont().getSize() + 4, 16+1));
	    if (listSelectionModel != null) {
		detailsTable.setSelectionModel(listSelectionModel); 
	    }
	    break;
	}
	if (this.viewType == viewType) {
	    if (currentViewPanel != null) {
		remove(currentViewPanel);
	    }
	    currentViewPanel = viewPanel;
	    add(currentViewPanel, BorderLayout.CENTER);
	    revalidate();
	    repaint();
	}
    }

    protected void installDefaults() {
	Locale l = getFileChooser().getLocale();

	listViewBorder       = UIManager.getBorder("FileChooser.listViewBorder");
	listViewBackground   = UIManager.getColor("FileChooser.listViewBackground");
	listViewWindowsStyle = UIManager.getBoolean("FileChooser.listViewWindowsStyle");
	readOnly	     = UIManager.getBoolean("FileChooser.readOnly");

	// TODO: On windows, get the following localized strings from the OS

	viewMenuLabelText =
			UIManager.getString("FileChooser.viewMenuLabelText", l); 
	refreshActionLabelText =
			UIManager.getString("FileChooser.refreshActionLabelText", l); 
	newFolderActionLabelText =
			UIManager.getString("FileChooser.newFolderActionLabelText", l); 

	viewTypeActionNames = new String[VIEWTYPE_COUNT];
	viewTypeActionNames[VIEWTYPE_LIST] =
			UIManager.getString("FileChooser.listViewActionLabelText", l);
	viewTypeActionNames[VIEWTYPE_DETAILS] =
			UIManager.getString("FileChooser.detailsViewActionLabelText", l);
    }

    /**
     * Fetches the command list for the FilePane. These commands
     * are useful for binding to events, such as in a keymap.
     *
     * @return the command list
     */
    public Action[] getActions() {      
	if (actions == null) {
	    class FilePaneAction extends AbstractAction {
		FilePaneAction(String name) {
		    this(name, name);
		}

		FilePaneAction(String name, String cmd) {
		    super(name);
		    putValue(Action.ACTION_COMMAND_KEY, cmd);
		}

		public void actionPerformed(ActionEvent e) {
		    String cmd = (String)getValue(Action.ACTION_COMMAND_KEY);

		    if (cmd == ACTION_CANCEL) {
			if (editFile != null) {
			   cancelEdit();
			} else {
			   getFileChooser().cancelSelection();
			}
		    } else if (cmd == ACTION_EDIT_FILE_NAME) {
			JFileChooser fc = getFileChooser();
			int index = listSelectionModel.getMinSelectionIndex();
			if (index >= 0 && editFile == null &&
			    (!fc.isMultiSelectionEnabled() ||
			     fc.getSelectedFiles().length <= 1)) {

			    editFileName(index);
			}
		    } else if (cmd == ACTION_REFRESH) {
			getFileChooser().rescanCurrentDirectory();
		    }
		}

		public boolean isEnabled() {
		    String cmd = (String)getValue(Action.ACTION_COMMAND_KEY);
		    if (cmd == ACTION_CANCEL) {
			return getFileChooser().isEnabled();
		    } else if (cmd == ACTION_EDIT_FILE_NAME) {
			return !readOnly && getFileChooser().isEnabled();
		    } else {
			return true;
		    }
		}
	    }

	    ArrayList<Action> actionList = new ArrayList<Action>(8);
	    Action action;

	    actionList.add(new FilePaneAction(ACTION_CANCEL));
	    actionList.add(new FilePaneAction(ACTION_EDIT_FILE_NAME));
	    actionList.add(new FilePaneAction(refreshActionLabelText, ACTION_REFRESH));

	    action = fileChooserUIAccessor.getApproveSelectionAction();
	    if (action != null) {
		actionList.add(action);
	    }
	    action = fileChooserUIAccessor.getChangeToParentDirectoryAction();
	    if (action != null) {
		actionList.add(action);
	    }
	    action = getNewFolderAction();
	    if (action != null) {
		actionList.add(action);
	    }
	    action = getViewTypeAction(VIEWTYPE_LIST);
	    if (action != null) {
		actionList.add(action);
	    }
	    action = getViewTypeAction(VIEWTYPE_DETAILS);
	    if (action != null) {
		actionList.add(action);
	    }
	    actions = actionList.toArray(new Action[actionList.size()]);
	}

	return actions;
    }

    protected void createActionMap() {
	addActionsToMap(super.getActionMap(), getActions());
    }


    public static void addActionsToMap(ActionMap map, Action[] actions) {
	if (map != null && actions != null) {
	    for (int i = 0; i < actions.length; i++) {
		Action a = actions[i];
		String cmd = (String)a.getValue(Action.ACTION_COMMAND_KEY);
		if (cmd == null) {
		    cmd = (String)a.getValue(Action.NAME);
		}
		map.put(cmd, a);
	    }
	}
    }


    private void updateListRowCount(JList list) {
	if (listViewWindowsStyle && smallIconsView) {
	    list.setVisibleRowCount(getModel().getSize() / 3);
	} else {
	    list.setVisibleRowCount(-1);
	}
    }

    public JPanel createList() {
	JPanel p = new JPanel(new BorderLayout());
	final JFileChooser fileChooser = getFileChooser();
	final JList list = new JList() {
	    public int getNextMatch(String prefix, int startIndex, Position.Bias bias) {
		ListModel model = getModel();
		int max = model.getSize();
		if (prefix == null || startIndex < 0 || startIndex >= max) {
		    throw new IllegalArgumentException();
		}
		// start search from the next element before/after the selected element
		boolean backwards = (bias == Position.Bias.Backward);
		for (int i = startIndex; backwards ? i >= 0 : i < max; i += (backwards ?  -1 : 1)) {
		    String filename = fileChooser.getName((File)model.getElementAt(i));
		    if (filename.regionMatches(true, 0, prefix, 0, prefix.length())) {
			return i;
		    }
		}
		return -1;
	    }
	};
	Font listFont = UIManager.getFont("FileChooser.listFont");
	if (listFont != null) {
	    // Make sure the font is not a UIResource to avoid the ListUI overriding it
	    list.setFont(new Font(listFont.getAttributes()));
	}
	list.setCellRenderer(new FileRenderer());
	list.setLayoutOrientation(JList.VERTICAL_WRAP);

	if (listViewWindowsStyle) {
	    // 4835633 : tell BasicListUI that this is a file list
	    list.putClientProperty("List.isFileList", Boolean.TRUE);
            list.addFocusListener(repaintListener);
	}

	updateListRowCount(list);

	getModel().addListDataListener(new ListDataListener() {
	    public void intervalAdded(ListDataEvent e) {
		updateListRowCount(list);
	    }
	    public void intervalRemoved(ListDataEvent e) {
		updateListRowCount(list);
	    }
	    public void contentsChanged(ListDataEvent e) {
		if (isShowing()) {
		    clearSelection();
                }
		updateListRowCount(list);
	    }
	});
	if (fileChooser.isMultiSelectionEnabled()) {
	    list.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
	} else {
	    list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
	}
	list.setModel(getModel());

	list.addListSelectionListener(createListSelectionListener());
	list.addMouseListener(getMouseHandler());
	getModel().addListDataListener(new ListDataListener() {
	    public void contentsChanged(ListDataEvent e) {
		// Update the selection after JList has been updated
		new DelayedSelectionUpdater();
	    }
	    public void intervalAdded(ListDataEvent e) {
		int i0 = e.getIndex0();
		int i1 = e.getIndex1();
		if (i0 == i1) {
		    File file = (File)getModel().getElementAt(i0);
		    if (file.equals(newFolderFile)) {
			new DelayedSelectionUpdater(file);
			newFolderFile = null;
		    }
		}
	    }
	    public void intervalRemoved(ListDataEvent e) {
	    }
	});

	JScrollPane scrollpane = new JScrollPane(list);
	if (listViewBackground != null) {
	    list.setBackground(listViewBackground);
	}
	if (listViewBorder != null) {
	    scrollpane.setBorder(listViewBorder);
	}
	p.add(scrollpane, BorderLayout.CENTER);
	return p;
    }

    class DetailsTableModel extends AbstractTableModel implements ListDataListener {
	String[] columnNames = {
	    fileNameHeaderText,
	    fileSizeHeaderText,
	    fileTypeHeaderText,
	    fileDateHeaderText,
	    fileAttrHeaderText
	};
	JFileChooser chooser;
	ListModel listModel;

	DetailsTableModel(JFileChooser fc) {
	    this.chooser = fc;
	    listModel = getModel();
	    listModel.addListDataListener(this);
	}

	public int getRowCount() {
	    return listModel.getSize();
	}

	public int getColumnCount() {
	    return COLUMN_COLCOUNT;
	}

	public String getColumnName(int column) {
	    return columnNames[column];
	}

	public Class getColumnClass(int column) {
	    switch (column) {
	      case COLUMN_FILENAME:
		  return File.class;
	      case COLUMN_FILEDATE:
		  return Date.class;
	      default:
		  return super.getColumnClass(column);
	    }
	}

	public Object getValueAt(int row, int col) {
	    // Note: It is very important to avoid getting info on drives, as
	    // this will trigger "No disk in A:" and similar dialogs.
	    //
	    // Use (f.exists() && !chooser.getFileSystemView().isFileSystemRoot(f)) to
	    // determine if it is safe to call methods directly on f.

	    File f = (File)listModel.getElementAt(row);
	    switch (col) {
	      case COLUMN_FILENAME:
		  return f;

	      case COLUMN_FILESIZE:
		  if (!f.exists() || f.isDirectory()) {
		      return null;
		  }
		  if (listViewWindowsStyle) {
		      return (f.length() / 1024 + 1) + "KB";
		  } else {
		      long len = f.length() / 1024L;
		      if (len < 1024L) {
			  return ((len == 0L) ? 1L : len) + " KB";
		      } else {
			  len /= 1024L;
			  if (len < 1024L) {
			      return len + " MB";
			  } else {
			      len /= 1024L;
			      return len + " GB";
			  }
		      }
		  }

	      case COLUMN_FILETYPE:
		  if (!f.exists()) {
		      return null;
		  }
		  return chooser.getFileSystemView().getSystemTypeDescription(f);

	      case COLUMN_FILEDATE:
		  if (!f.exists() || chooser.getFileSystemView().isFileSystemRoot(f)) {
		      return null;
		  }
		  long time = f.lastModified();
		  return (time == 0L) ? null : new Date(time);

	      case COLUMN_FILEATTR:
		  if (!f.exists() || chooser.getFileSystemView().isFileSystemRoot(f)) {
		      return null;
		  }
		  String attributes = "";
		  try {
		      if (!f.canWrite()) {
			  attributes += "R";
		      }
		  } catch (AccessControlException ex) {
		      // No point in flagging R if we don't
		      // have the right to ask.
		  }
		  if (f.isHidden()) {
		      attributes += "H";
		  }
		  return attributes;
	    }
	    return null;
	}

	public void setValueAt(Object value, int row, int col) {
	    if (col == COLUMN_FILENAME) {
		JFileChooser chooser = getFileChooser();
		File f = (File)getValueAt(row, col);
		if (f != null) {
		    String oldDisplayName = chooser.getName(f);
		    String oldFileName = f.getName();
		    String newDisplayName = ((String)value).trim();
		    String newFileName;

		    if (!newDisplayName.equals(oldDisplayName)) {
			newFileName = newDisplayName;
			//Check if extension is hidden from user
			int i1 = oldFileName.length();
			int i2 = oldDisplayName.length();
			if (i1 > i2 && oldFileName.charAt(i2) == '.') {
			    newFileName = newDisplayName + oldFileName.substring(i2);
			}

			// rename
			FileSystemView fsv = chooser.getFileSystemView();
			File f2 = fsv.createFileObject(f.getParentFile(), newFileName);
			if (!f2.exists() && FilePane.this.getModel().renameFile(f, f2)) {
			    if (fsv.isParent(chooser.getCurrentDirectory(), f2)) {
				if (chooser.isMultiSelectionEnabled()) {
				    chooser.setSelectedFiles(new File[] { f2 });
				} else {
				    chooser.setSelectedFile(f2);
				}
			    } else {
				//Could be because of delay in updating Desktop folder
				//chooser.setSelectedFile(null);
			    }
			} else {
			    // PENDING(jeff) - show a dialog indicating failure
			}
		    }
		}
	    }
	}

	public boolean isCellEditable(int row, int column) {
	    File currentDirectory = getFileChooser().getCurrentDirectory();
	    return (!readOnly && column == COLUMN_FILENAME && canWrite(currentDirectory));
	}

	public void contentsChanged(ListDataEvent e) {
	    fireTableDataChanged();
	}
	public void intervalAdded(ListDataEvent e) {
	    fireTableDataChanged();
	}
	public void intervalRemoved(ListDataEvent e) {
	    fireTableDataChanged();
	}
    }

    class DetailsTableCellRenderer extends DefaultTableCellRenderer {
	JFileChooser chooser;
	DateFormat df;

	DetailsTableCellRenderer(JFileChooser chooser) {
	    this.chooser = chooser;
	    df = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT,
						chooser.getLocale());
	}

	public void setBounds(int x, int y, int width, int height) { 
	    if (listViewWindowsStyle && getHorizontalAlignment() == SwingConstants.LEADING) {
		// Restrict width to actual text
		width = Math.min(width, this.getPreferredSize().width+4);
	    } else {
		x -= 4;
	    }
	    super.setBounds(x, y, width, height); 
	}


	public Insets getInsets(Insets i) {
	    // Provide some space between columns
	    i = super.getInsets(i);
	    i.left  += 4;
	    i.right += 4;
	    return i;
	}

	public Component getTableCellRendererComponent(JTable table, Object value,
			      boolean isSelected, boolean hasFocus, int row, int column) {

            if (listViewWindowsStyle &&
                    (table.convertColumnIndexToModel(column) != COLUMN_FILENAME ||
                     !table.isFocusOwner())) {

                isSelected = false;
            }

            return super.getTableCellRendererComponent(table, value, isSelected,
                                                       hasFocus, row, column);
	}

	public void setValue(Object value) { 
	    setIcon(null);
	    if (value instanceof File) {
		File file = (File)value;
		String fileName = chooser.getName(file);
		setText(fileName);
		Icon icon = chooser.getIcon(file);
		setIcon(icon);
	    } else if (value instanceof Date) {
		setText((value == null) ? "" : df.format((Date)value));
	    } else {
		super.setValue(value);
	    }
	}
    }

    public JPanel createDetailsView() {
	final JFileChooser chooser = getFileChooser();

        Locale l = chooser.getLocale();
	fileNameHeaderText = UIManager.getString("FileChooser.fileNameHeaderText",l);
	fileSizeHeaderText = UIManager.getString("FileChooser.fileSizeHeaderText",l);
	fileTypeHeaderText = UIManager.getString("FileChooser.fileTypeHeaderText",l);
	fileDateHeaderText = UIManager.getString("FileChooser.fileDateHeaderText",l);
	fileAttrHeaderText = UIManager.getString("FileChooser.fileAttrHeaderText",l);

	JPanel p = new JPanel(new BorderLayout());

	DetailsTableModel detailsTableModel = new DetailsTableModel(chooser);

	final JTable detailsTable = new JTable(detailsTableModel) {
	    // Handle Escape key events here
	    protected boolean processKeyBinding(KeyStroke ks, KeyEvent e, int condition, boolean pressed) {
		if (e.getKeyCode() == KeyEvent.VK_ESCAPE && getCellEditor() == null) {
		    // We are not editing, forward to filechooser.
		    chooser.dispatchEvent(e);
		    return true;
		}
		return super.processKeyBinding(ks, e, condition, pressed);
	    }

	    public Component prepareRenderer(TableCellRenderer renderer, int row, int column) {
		Component comp = super.prepareRenderer(renderer, row, column);
		if (comp instanceof JLabel) {
                    if (convertColumnIndexToModel(column) == COLUMN_FILESIZE) {
			// Numbers are right-adjusted, regardless of component orientation
			((JLabel)comp).setHorizontalAlignment(SwingConstants.RIGHT);
		    } else {
			((JLabel)comp).setHorizontalAlignment(SwingConstants.LEADING);
		    }
		}
		return comp;
	    }
	};

	detailsTable.setComponentOrientation(chooser.getComponentOrientation());
	detailsTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
	detailsTable.setShowGrid(false);
	detailsTable.putClientProperty("JTable.autoStartsEdit", Boolean.FALSE);

	Font font = list.getFont();
	detailsTable.setFont(font);
	detailsTable.setIntercellSpacing(new Dimension(0, 0));

	TableColumnModel columnModel = detailsTable.getColumnModel();
	TableColumn[] columns = new TableColumn[COLUMN_COLCOUNT];

	for (int i = 0; i < COLUMN_COLCOUNT; i++) {
	    columns[i] = columnModel.getColumn(i);
	    columns[i].setPreferredWidth(COLUMN_WIDTHS[i]);
	}

	String osName =
	    (String)AccessController.doPrivileged(new GetPropertyAction("os.name"));

        if (osName == null || !osName.startsWith("Windows")) {
	    columnModel.removeColumn(columns[COLUMN_FILETYPE]);
	    columnModel.removeColumn(columns[COLUMN_FILEATTR]);
	}

	TableCellRenderer cellRenderer = new DetailsTableCellRenderer(chooser);
	detailsTable.setDefaultRenderer(File.class, cellRenderer);
	detailsTable.setDefaultRenderer(Date.class, cellRenderer);
	detailsTable.setDefaultRenderer(Object.class, cellRenderer);

	// Install cell editor for editing file name
	if (!readOnly) {
	    final JTextField tf = new JTextField();
	    tf.addFocusListener(editorFocusListener);
	    columns[COLUMN_FILENAME].setCellEditor(new DefaultCellEditor(tf) {
		public boolean isCellEditable(EventObject e) {
		    if (e instanceof MouseEvent) { 
			MouseEvent me = (MouseEvent)e;
			int index = detailsTable.rowAtPoint(me.getPoint());
			return (me.getClickCount() == 1 && detailsTable.isRowSelected(index));
		    }
		    return super.isCellEditable(e);
		}

		public Component getTableCellEditorComponent(JTable table, Object value,
							     boolean isSelected, int row, int column) {
		    Component comp = super.getTableCellEditorComponent(table, value,
								       isSelected, row, column);
		    if (value instanceof File) {
			tf.setText(chooser.getName((File)value));
			tf.requestFocus();
			tf.selectAll();
		    }		
		    return comp;
		}
	    });
	}
	detailsTable.addMouseListener(getMouseHandler());
	// No need to addListSelectionListener because selections are forwarded
	// to our JList.

	if (listViewWindowsStyle) {
	    // 4835633 : tell BasicTableUI that this is a file list
	    detailsTable.putClientProperty("Table.isFileList", Boolean.TRUE);
            detailsTable.addFocusListener(repaintListener);
	}

        // TAB/SHIFT-TAB should transfer focus and ENTER should select an item.
        // We don't want them to navigate within the table
        ActionMap am = SwingUtilities.getUIActionMap(detailsTable);
        am.remove("selectNextRowCell");
        am.remove("selectPreviousRowCell");
        am.remove("selectNextColumnCell");
        am.remove("selectPreviousColumnCell");
        detailsTable.setFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS,
                     null);
        detailsTable.setFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS,
                     null);

	JScrollPane scrollpane = new JScrollPane(detailsTable);
	scrollpane.setComponentOrientation(chooser.getComponentOrientation());
        LookAndFeel.installColors(scrollpane.getViewport(), "Table.background", "Table.foreground");

	// Adjust width of first column so the table fills the viewport when
	// first displayed (temporary listener).
	scrollpane.addComponentListener(new ComponentAdapter() {
	    public void componentResized(ComponentEvent e) {
		JScrollPane sp = (JScrollPane)e.getComponent();
		fixNameColumnWidth(sp.getViewport().getSize().width);
		sp.removeComponentListener(this);
	    }
	});

	if (listViewWindowsStyle) {
	    // 4835633.
	    // If the mouse is pressed in the area below the Details view table, the
	    // event is not dispatched to the Table MouseListener but to the
	    // scrollpane.  Listen for that here so we can clear the selection.
	    scrollpane.addMouseListener(new MouseAdapter() {
		public void mousePressed(MouseEvent e) {
		    JScrollPane jsp = ((JScrollPane)e.getComponent());
		    JTable table = (JTable)jsp.getViewport().getView();

		    if (!e.isShiftDown() || table.getSelectionModel().getSelectionMode() == ListSelectionModel.SINGLE_SELECTION) {
			clearSelection();
			TableCellEditor tce = table.getCellEditor();
			if (tce != null) {
			    tce.stopCellEditing();
			}
		    }
		}
	    });
	}

	if (listViewBackground != null) {
	    detailsTable.setBackground(listViewBackground);
	}
	if (listViewBorder != null) {
	    scrollpane.setBorder(listViewBorder);
	}
	p.add(scrollpane, BorderLayout.CENTER);
	return p;
    } // createDetailsView

    private void fixNameColumnWidth(int viewWidth) {
	TableColumn nameCol = detailsTable.getColumnModel().getColumn(COLUMN_FILENAME);
	int tableWidth = detailsTable.getPreferredSize().width;

	if (tableWidth < viewWidth) {
	    nameCol.setPreferredWidth(nameCol.getPreferredWidth() + viewWidth - tableWidth);
	}
    }

    private class DelayedSelectionUpdater implements Runnable {
	File editFile;

	DelayedSelectionUpdater() {
	    this(null);
	}

	DelayedSelectionUpdater(File editFile) {
	    this.editFile = editFile;
            if (isShowing()) {
                SwingUtilities.invokeLater(this);
            }
	}

	public void run() {
	    setFileSelected();
	    if (editFile != null) {
		editFileName(getModel().indexOf(editFile));
		editFile = null;
	    }
	}
    }


    /**
     * Creates a selection listener for the list of files and directories.
     *
     * @return a <code>ListSelectionListener</code>
     */
    public ListSelectionListener createListSelectionListener() {
	return fileChooserUIAccessor.createListSelectionListener();
    }

    int lastIndex = -1;
    File editFile = null;
    int editX = 20;

    private int getEditIndex() {
	return lastIndex;
    }

    private void setEditIndex(int i) {
	lastIndex = i;
    }

    private void resetEditIndex() {
	lastIndex = -1;
    }

    private void cancelEdit() {
	if (editFile != null) {
	    editFile = null;
	    list.remove(editCell);
	    repaint();
	} else if (detailsTable != null && detailsTable.isEditing()) {
	    detailsTable.getCellEditor().cancelCellEditing();
	}
    }

    JTextField editCell = null;

    private void editFileName(int index) {
	File currentDirectory = getFileChooser().getCurrentDirectory();
	if (readOnly || !canWrite(currentDirectory)) {
	    return;
	}

	ensureIndexIsVisible(index);
	switch (viewType) {
	  case VIEWTYPE_LIST:
	    editFile = (File)getModel().getElementAt(index);
	    Rectangle r = list.getCellBounds(index, index);
	    if (editCell == null) {
		editCell = new JTextField();
		editCell.addActionListener(new EditActionListener());
		editCell.addFocusListener(editorFocusListener);
		editCell.setNextFocusableComponent(list);
	    }
	    list.add(editCell);
	    editCell.setText(getFileChooser().getName(editFile));
	    ComponentOrientation orientation = list.getComponentOrientation();
	    editCell.setComponentOrientation(orientation);
	    if (orientation.isLeftToRight()) {
		editCell.setBounds(editX + r.x, r.y, r.width - editX, r.height);
	    } else {
		editCell.setBounds(r.x, r.y, r.width - editX, r.height);
	    }
	    editCell.requestFocus();
	    editCell.selectAll();
	    break;

	  case VIEWTYPE_DETAILS:
	    detailsTable.editCellAt(index, COLUMN_FILENAME);
	    break;
	}
    }


    class EditActionListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    applyEdit();
	} 
    }

    private void applyEdit() {
	if (editFile != null && editFile.exists()) {
	    JFileChooser chooser = getFileChooser();
	    String oldDisplayName = chooser.getName(editFile);
	    String oldFileName = editFile.getName();
	    String newDisplayName = editCell.getText().trim();
	    String newFileName;

	    if (!newDisplayName.equals(oldDisplayName)) {
		newFileName = newDisplayName;
		//Check if extension is hidden from user
		int i1 = oldFileName.length();
		int i2 = oldDisplayName.length();
		if (i1 > i2 && oldFileName.charAt(i2) == '.') {
		    newFileName = newDisplayName + oldFileName.substring(i2);
		}

		// rename
		FileSystemView fsv = chooser.getFileSystemView();
		File f2 = fsv.createFileObject(editFile.getParentFile(), newFileName);
		if (!f2.exists() && getModel().renameFile(editFile, f2)) {
		    if (fsv.isParent(chooser.getCurrentDirectory(), f2)) {
			if (chooser.isMultiSelectionEnabled()) {
			    chooser.setSelectedFiles(new File[] { f2 });
			} else {
			    chooser.setSelectedFile(f2);
			}
		    } else {
			//Could be because of delay in updating Desktop folder
			//chooser.setSelectedFile(null);
		    }
		} else {
		    // PENDING(jeff) - show a dialog indicating failure
		}
	    }
	} 
        if (detailsTable != null && detailsTable.isEditing()) {
            detailsTable.getCellEditor().stopCellEditing();
        }
	cancelEdit();
    }

    protected Action newFolderAction;

    public Action getNewFolderAction() {
	if (!readOnly && newFolderAction == null) {
	    newFolderAction = new AbstractAction(newFolderActionLabelText) {
		private Action basicNewFolderAction;

		// Initializer
		{
		    putValue(Action.ACTION_COMMAND_KEY, FilePane.ACTION_NEW_FOLDER);

		    File currentDirectory = getFileChooser().getCurrentDirectory();
		    if (currentDirectory != null) {
			setEnabled(canWrite(currentDirectory));
		    }
		}

		public void actionPerformed(ActionEvent ev) {
		    if (basicNewFolderAction == null) {
			basicNewFolderAction = fileChooserUIAccessor.getNewFolderAction();
		    }
		    JFileChooser fc = getFileChooser();
		    File oldFile = fc.getSelectedFile();
		    basicNewFolderAction.actionPerformed(ev);
		    File newFile = fc.getSelectedFile();
		    if (newFile != null && !newFile.equals(oldFile) && newFile.isDirectory()) {
			newFolderFile = newFile;
		    }
		}
	    };
	}
	return newFolderAction;
    }

    protected class FileRenderer extends DefaultListCellRenderer  {

	public Component getListCellRendererComponent(JList list, Object value,
						      int index, boolean isSelected,
						      boolean cellHasFocus) {

            if (listViewWindowsStyle && !list.isFocusOwner()) {
                isSelected = false;
            }

	    super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
	    File file = (File) value;
	    String fileName = getFileChooser().getName(file);
	    setText(fileName);
	    setFont(list.getFont());

	    Icon icon = getFileChooser().getIcon(file);
	    if (icon != null) {
		setIcon(icon);

		if (isSelected) {
		    // PENDING - grab padding (4) below from defaults table.
		    editX = icon.getIconWidth() + 4;
		}
	    } else {
		if (getFileChooser().getFileSystemView().isTraversable(file)) {
		    setText(fileName+File.separator);
		}
	    }

	    return this;
	}
    }


    void setFileSelected() {
	if (getFileChooser().isMultiSelectionEnabled() && !isDirectorySelected()) {
	    File[] files = getFileChooser().getSelectedFiles();	// Should be selected
	    Object[] selectedObjects = list.getSelectedValues(); // Are actually selected

	    listSelectionModel.setValueIsAdjusting(true);
	    try {
                int lead = listSelectionModel.getLeadSelectionIndex();
                int anchor = listSelectionModel.getAnchorSelectionIndex();

		Arrays.sort(files);
		Arrays.sort(selectedObjects);

		int shouldIndex = 0;
		int actuallyIndex = 0;

		// Remove files that shouldn't be selected and add files which should be selected
		// Note: Assume files are already sorted in compareTo order.
		while (shouldIndex < files.length &&
		       actuallyIndex < selectedObjects.length) {
		    int comparison = files[shouldIndex].compareTo((File)selectedObjects[actuallyIndex]);
		    if (comparison < 0) {
			int index = getModel().indexOf(files[shouldIndex]);
			listSelectionModel.addSelectionInterval(index, index);
			shouldIndex++;
		    } else if (comparison > 0) {
			int index = getModel().indexOf(selectedObjects[actuallyIndex]);
			listSelectionModel.removeSelectionInterval(index, index);
			actuallyIndex++;
		    } else {
			// Do nothing
			shouldIndex++;
			actuallyIndex++;
		    }

		}

		while (shouldIndex < files.length) {
		    int index = getModel().indexOf(files[shouldIndex]);
		    listSelectionModel.addSelectionInterval(index, index);
		    shouldIndex++;
		}

		while (actuallyIndex < selectedObjects.length) {
		    int index = getModel().indexOf(selectedObjects[actuallyIndex]);
		    listSelectionModel.removeSelectionInterval(index, index);
		    actuallyIndex++;
		}

                // restore the anchor and lead
                if (listSelectionModel instanceof DefaultListSelectionModel) {
                    ((DefaultListSelectionModel)listSelectionModel).
                        moveLeadSelectionIndex(lead);
                    ((DefaultListSelectionModel)listSelectionModel).
                        setAnchorSelectionIndex(anchor);
                }
	    } finally {
		listSelectionModel.setValueIsAdjusting(false);
	    }
	} else {
	    JFileChooser chooser = getFileChooser();
	    File f = null;
	    if (isDirectorySelected()) {
		f = getDirectory();
	    } else {
		f = chooser.getSelectedFile();
	    }
	    int i;
	    if (f != null && (i = getModel().indexOf(f)) >= 0) {
		listSelectionModel.setSelectionInterval(i, i);
		ensureIndexIsVisible(i);
	    } else {
		clearSelection();
	    }
	}
    }


    /* The following methods are used by the PropertyChange Listener */

    private void doSelectedFileChanged(PropertyChangeEvent e) {
	applyEdit();
	File f = (File) e.getNewValue();
	JFileChooser fc = getFileChooser();
	if (f != null 
	    && ((fc.isFileSelectionEnabled() && !f.isDirectory())
		|| (f.isDirectory() && fc.isDirectorySelectionEnabled()))) {

	    setFileSelected();
	}
    }
    
    private void doSelectedFilesChanged(PropertyChangeEvent e) {
	applyEdit();
	File[] files = (File[]) e.getNewValue();
	JFileChooser fc = getFileChooser();
	if (files != null
	    && files.length > 0
	    && (files.length > 1 || fc.isDirectorySelectionEnabled() || !files[0].isDirectory())) {
	    setFileSelected();
	}
    }
    
    private void doDirectoryChanged(PropertyChangeEvent e) {
	JFileChooser fc = getFileChooser();
	FileSystemView fsv = fc.getFileSystemView();

	applyEdit();
	resetEditIndex();
 	ensureIndexIsVisible(0);
	File currentDirectory = fc.getCurrentDirectory();
	if (currentDirectory != null) {
	    if (!readOnly) {
		getNewFolderAction().setEnabled(canWrite(currentDirectory));
	    }
	    fileChooserUIAccessor.getChangeToParentDirectoryAction().setEnabled(!fsv.isRoot(currentDirectory));
	}
    }

    private void doFilterChanged(PropertyChangeEvent e) {
	applyEdit();
	resetEditIndex();
	clearSelection();
    }

    private void doFileSelectionModeChanged(PropertyChangeEvent e) {
	applyEdit();
	resetEditIndex();
	clearSelection();
    }

    private void doMultiSelectionChanged(PropertyChangeEvent e) {
	if (getFileChooser().isMultiSelectionEnabled()) {
	    listSelectionModel.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
	} else {
	    listSelectionModel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
	    clearSelection();
	    getFileChooser().setSelectedFiles(null);
	}
    }
    
    /*
     * Listen for filechooser property changes, such as
     * the selected file changing, or the type of the dialog changing.
     */
    public void propertyChange(PropertyChangeEvent e) {
	    if (viewType == -1) {
		setViewType(VIEWTYPE_LIST);
	    }

	String s = e.getPropertyName();
	if (s.equals(JFileChooser.SELECTED_FILE_CHANGED_PROPERTY)) {
	    doSelectedFileChanged(e);
	} else if (s.equals(JFileChooser.SELECTED_FILES_CHANGED_PROPERTY)) {
	    doSelectedFilesChanged(e);
	} else if (s.equals(JFileChooser.DIRECTORY_CHANGED_PROPERTY)) {
	    doDirectoryChanged(e);
	} else if (s.equals(JFileChooser.FILE_FILTER_CHANGED_PROPERTY)) {
	    doFilterChanged(e);
	} else if (s.equals(JFileChooser.FILE_SELECTION_MODE_CHANGED_PROPERTY)) {
	    doFileSelectionModeChanged(e);
	} else if (s.equals(JFileChooser.MULTI_SELECTION_ENABLED_CHANGED_PROPERTY)) {
	    doMultiSelectionChanged(e);
	} else if (s.equals(JFileChooser.CANCEL_SELECTION)) {
	    applyEdit();
	} else if (s.equals("componentOrientation")) {
	    ComponentOrientation o = (ComponentOrientation)e.getNewValue();
	    JFileChooser cc = (JFileChooser)e.getSource();
	    if (o != (ComponentOrientation)e.getOldValue()) {
		cc.applyComponentOrientation(o);
	    }
	    if (detailsTable != null) {
		detailsTable.setComponentOrientation(o);
		detailsTable.getParent().getParent().setComponentOrientation(o);
	    }
	}
    }

    private void ensureIndexIsVisible(int i) {
	if (i >= 0) {
	    if (list != null) {
		list.ensureIndexIsVisible(i);
	    }
	    if (detailsTable != null) {
		detailsTable.scrollRectToVisible(detailsTable.getCellRect(i, COLUMN_FILENAME, true));
	    }
	}
    }

    public void ensureFileIsVisible(JFileChooser fc, File f) {
	ensureIndexIsVisible(getModel().indexOf(f));
    }

    public void rescanCurrentDirectory() {
	getModel().validateFileCache();
    }

    public void clearSelection() {
	if (listSelectionModel != null) {
	    listSelectionModel.clearSelection();
	    if (listSelectionModel instanceof DefaultListSelectionModel) {
		((DefaultListSelectionModel)listSelectionModel).moveLeadSelectionIndex(0);
		((DefaultListSelectionModel)listSelectionModel).setAnchorSelectionIndex(0);
	    }
	}
    }

    public JMenu getViewMenu() {
	if (viewMenu == null) {
	    viewMenu = new JMenu(viewMenuLabelText);
	    ButtonGroup viewButtonGroup = new ButtonGroup();

	    for (int i = 0; i < VIEWTYPE_COUNT; i++) {
		JRadioButtonMenuItem mi =
		    new JRadioButtonMenuItem(new ViewTypeAction(i));
		viewButtonGroup.add(mi);
		viewMenu.add(mi);
	    }
	    updateViewMenu();
	}
	return viewMenu;
    }

    private void updateViewMenu() {
	if (viewMenu != null) {
	    Component[] comps = viewMenu.getMenuComponents();
	    for (int i = 0; i < comps.length; i++) {
		if (comps[i] instanceof JRadioButtonMenuItem) {
		    JRadioButtonMenuItem mi = (JRadioButtonMenuItem)comps[i];
		    if (((ViewTypeAction)mi.getAction()).viewType == viewType) {
			mi.setSelected(true);
		    }
		}
	    }
	}
    }

    public JPopupMenu getComponentPopupMenu() {
	JPopupMenu popupMenu = getFileChooser().getComponentPopupMenu();
	if (popupMenu != null) {
	    return popupMenu;
	}

	JMenu viewMenu = getViewMenu();
	if (contextMenu == null) {
	    contextMenu = new JPopupMenu();
	    if (viewMenu != null) {
		contextMenu.add(viewMenu);
		if (listViewWindowsStyle) {
		    contextMenu.addSeparator();
		}
	    }
	    ActionMap actionMap = getActionMap();
	    Action refreshAction   = actionMap.get(ACTION_REFRESH);
	    Action newFolderAction = actionMap.get(ACTION_NEW_FOLDER);
	    if (refreshAction != null) {
		contextMenu.add(refreshAction);
		if (listViewWindowsStyle && newFolderAction != null) {
		    contextMenu.addSeparator();
		}
	    }
	    if (newFolderAction != null) {
		contextMenu.add(newFolderAction);
	    }
	}
	if (viewMenu != null) {
	    viewMenu.getPopupMenu().setInvoker(viewMenu);
	}
	return contextMenu;
    }


    private Handler handler;

    protected Handler getMouseHandler() {
        if (handler == null) {
            handler = new Handler();
        }
        return handler;
    }

    private class Handler implements MouseListener {
	private MouseListener doubleClickListener;

        public void mouseClicked(MouseEvent evt) {
	    JComponent source = (JComponent)evt.getSource();

	    int index;
	    if (source instanceof JList) {
		index = SwingUtilities2.loc2IndexFileList(list, evt.getPoint());
	    } else if (source instanceof JTable) {
		index = ((JTable)source).rowAtPoint(evt.getPoint());

		// Translate point from table to list
		if (index >= 0 && list != null &&
		    listSelectionModel.isSelectedIndex(index)) {

		    // Make a new event with the list as source, placing the
		    // click in the corresponding list cell.
		    Rectangle r = list.getCellBounds(index, index);
		    evt = new MouseEvent(list, evt.getID(),
					 evt.getWhen(), evt.getModifiers(), 
					 r.x + 1, r.y + r.height/2,
					 evt.getClickCount(), evt.isPopupTrigger(),
					 evt.getButton());
		}
	    } else {
		return;
	    }

	    if (index >= 0 && SwingUtilities.isLeftMouseButton(evt)) {
		JFileChooser fc = getFileChooser();

		// For single click, we handle editing file name
		if (evt.getClickCount() == 1 && source instanceof JList) {
		    if ((!fc.isMultiSelectionEnabled() || fc.getSelectedFiles().length <= 1)
			&& index >= 0 && listSelectionModel.isSelectedIndex(index)
			&& getEditIndex() == index && editFile == null) {

			editFileName(index);
		    } else {
			if (index >= 0) {
			    setEditIndex(index);
			} else {
			    resetEditIndex();
			}
		    }
		} else if (evt.getClickCount() == 2) {
		    // on double click (open or drill down one directory) be
		    // sure to clear the edit index
		    resetEditIndex();
		}
	    }

	    // Forward event to Basic
	    if (getDoubleClickListener() != null) {
		getDoubleClickListener().mouseClicked(evt);
	    }
        }

        public void mouseEntered(MouseEvent evt) {
	    JComponent source = (JComponent)evt.getSource();
	    if (source instanceof JTable) {
		JTable table = (JTable)evt.getSource();

		TransferHandler th1 = getFileChooser().getTransferHandler();
		TransferHandler th2 = table.getTransferHandler();
		if (th1 != th2) {
		    table.setTransferHandler(th1);
		}

		boolean dragEnabled = getFileChooser().getDragEnabled();
		if (dragEnabled != table.getDragEnabled()) {
		    table.setDragEnabled(dragEnabled);
		}
	    } else if (source instanceof JList) {
		// Forward event to Basic
		if (getDoubleClickListener() != null) {
		    getDoubleClickListener().mouseEntered(evt);
		}
	    }
        }

        public void mouseExited(MouseEvent evt) {
	    if (evt.getSource() instanceof JList) {
		// Forward event to Basic
		if (getDoubleClickListener() != null) {
		    getDoubleClickListener().mouseExited(evt);
		}
	    }
        }

        public void mousePressed(MouseEvent evt) {
	    if (evt.getSource() instanceof JList) {
		// Forward event to Basic
		if (getDoubleClickListener() != null) {
		    getDoubleClickListener().mousePressed(evt);
		}
	    }
        }

        public void mouseReleased(MouseEvent evt) {
	    if (evt.getSource() instanceof JList) {
		// Forward event to Basic
		if (getDoubleClickListener() != null) {
		    getDoubleClickListener().mouseReleased(evt);
		}
	    }
        }

	private MouseListener getDoubleClickListener() {
	    // Lazy creation of Basic's listener
	    if (doubleClickListener == null && list != null) {
		doubleClickListener =
		    fileChooserUIAccessor.createDoubleClickListener(list);
	    }
	    return doubleClickListener;
	}
    }

    /**
     * Property to remember whether a directory is currently selected in the UI.
     *
     * @return <code>true</code> iff a directory is currently selected.
     */
    protected boolean isDirectorySelected() {
	return fileChooserUIAccessor.isDirectorySelected();
    }


    /**
     * Property to remember the directory that is currently selected in the UI.
     *
     * @return the value of the <code>directory</code> property
     * @see javax.swing.plaf.basic.BasicFileChooserUI#setDirectory
     */
    protected File getDirectory() {
	return fileChooserUIAccessor.getDirectory();
    }

    private Component findChildComponent(Container container, Class cls) {
	int n = container.getComponentCount();
	for (int i = 0; i < n; i++) {
	    Component comp = container.getComponent(i);
	    if (cls.isInstance(comp)) {
		return comp;
	    } else if (comp instanceof Container) {
		Component c = findChildComponent((Container)comp, cls);
		if (c != null) {
		    return c;
		}
	    }
	}
	return null;
    }

    public static boolean canWrite(File f) {
	boolean writeable = false;
	if (f != null) {
	    try {
		writeable = f.canWrite();
	    } catch (AccessControlException ex) {
		writeable = false;
	    }
	}
	return writeable;
    }

    // This interface is used to access methods in the FileChooserUI
    // that are not public.
    public interface FileChooserUIAccessor {
	public JFileChooser getFileChooser();
	public BasicDirectoryModel getModel();
	public JPanel createList();
	public JPanel createDetailsView();
	public boolean isDirectorySelected();
	public File getDirectory();
	public Action getApproveSelectionAction();
	public Action getChangeToParentDirectoryAction();
	public Action getNewFolderAction();
	public MouseListener createDoubleClickListener(JList list);
	public ListSelectionListener createListSelectionListener();
    }
}
