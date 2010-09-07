/*
 * @(#)XFileDialogPeer.java	1.17 04/07/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import java.awt.peer.*;
import java.io.*;
import java.util.Locale;
import java.util.Arrays;
import com.sun.java.swing.plaf.motif.*;
import javax.swing.plaf.ComponentUI;
import java.util.logging.*;
import java.security.AccessController;
import java.security.PrivilegedAction;

class XFileDialogPeer extends XDialogPeer implements FileDialogPeer, ActionListener, ItemListener, KeyEventDispatcher {
    private static final Logger log = Logger.getLogger("sun.awt.X11.XFileDialogPeer");

    FileDialog target;
    String      file;
    String      dir;
    String      title;
    int         mode;
    FilenameFilter  filter;
    private static final int PATH_CHOICE_WIDTH = 20;

    String savedFile;
    String savedDir;

    Dialog fileDialog;

    GridBagLayout       gbl;
    GridBagLayout       gblButtons;
    GridBagConstraints  gbc;

    // Components in the fileDialogWindow
    TextField       filterField;
    TextField       selectionField;
    List        directoryList;
    List        fileList;
    Panel       buttons;
    Button      openButton;
    Button      filterButton;
    Button      cancelButton;
    Choice  pathChoice;
    TextField  pathField;
    Panel pathPanel;

    String cancelButtonText = null;
    String enterFileNameLabelText = null;
    String filesLabelText= null;
    String foldersLabelText= null;
    String pathLabelText= null;
    String filterLabelText= null;
    String openButtonText= null;
    String saveButtonText= null;
    String actionButtonText= null;

    
    void installStrings() {
        Locale l = target.getLocale();
        UIDefaults uid = XToolkit.getUIDefaults();
        cancelButtonText = uid.getString("FileChooser.cancelButtonText",l);
        enterFileNameLabelText = uid.getString("FileChooser.enterFileNameLabelText",l);
        filesLabelText = uid.getString("FileChooser.filesLabelText",l);
        foldersLabelText = uid.getString("FileChooser.foldersLabelText",l);
        pathLabelText = uid.getString("FileChooser.pathLabelText",l);
        filterLabelText = uid.getString("FileChooser.filterLabelText",l);
        openButtonText = uid.getString("FileChooser.openButtonText",l);
        saveButtonText  = uid.getString("FileChooser.saveButtonText",l);

    }

    XFileDialogPeer(FileDialog target) {
        super((Dialog)target);
        this.target = target;
    } 

    private void init(FileDialog target) {
        fileDialog = target; //new Dialog(target, target.getTitle(), false);
        this.title = target.getTitle();
        this.mode = target.getMode();
        this.target = target;
        this.filter = target.getFilenameFilter();    

        savedFile = target.getFile();
        savedDir = target.getDirectory();
        if (savedDir == null) {
            savedDir = (String)AccessController.doPrivileged(
                new PrivilegedAction() {
                    public Object run() {
                       return System.getProperty("user.dir");
                    }
                });
        }

        installStrings();
        gbl = new GridBagLayout();
        gblButtons = new GridBagLayout();
        gbc = new GridBagConstraints();
        fileDialog.setLayout(gbl);

        // create components
        buttons = new Panel();
        buttons.setLayout(gblButtons);
        actionButtonText = (target.getMode() == FileDialog.SAVE) ? saveButtonText : openButtonText;  
        openButton = new Button(actionButtonText);
        
        filterButton = new Button(filterLabelText);
        cancelButton = new Button(cancelButtonText);
        directoryList = new List();
        fileList = new List();
        filterField = new TextField();
        selectionField = new TextField();

        // the insets used by the components in the fileDialog
        Insets noInset = new Insets(0, 0, 0, 0);
        Insets textFieldInset = new Insets(0, 8, 0, 8);
        Insets leftListInset = new Insets(0, 8, 0, 4);
        Insets rightListInset = new Insets(0, 4, 0, 8);
        Insets separatorInset = new Insets(8, 0, 0, 0);
        Insets labelInset = new Insets(0, 8, 0, 0);
        Insets buttonsInset = new Insets(10, 8, 10, 8);

        // add components to GridBagLayout "gbl"

        Font f = new Font("Dialog", Font.PLAIN, 12);

        Label label = new Label(pathLabelText); 
        label.setFont(f);
        addComponent(label, gbl, gbc, 0, 0, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.NONE, labelInset);
        pathField = new TextField(savedDir); 
        pathChoice = new Choice() {
                public Dimension getPreferredSize() {
                    return new Dimension(PATH_CHOICE_WIDTH, pathField.getPreferredSize().height);
                }
            };
        pathPanel = new Panel();
        pathPanel.setLayout(new BorderLayout());
        
        pathPanel.add(pathField,BorderLayout.CENTER);
        pathPanel.add(pathChoice,BorderLayout.EAST);
        //addComponent(pathField, gbl, gbc, 0, 1, 2,
        //             GridBagConstraints.WEST, (Container)fileDialog,
        //             1, 0, GridBagConstraints.HORIZONTAL, textFieldInset);
        //addComponent(pathChoice, gbl, gbc, 1, 1, GridBagConstraints.RELATIVE,
         //            GridBagConstraints.WEST, (Container)fileDialog,
          //           1, 0, GridBagConstraints.HORIZONTAL, textFieldInset);
        addComponent(pathPanel, gbl, gbc, 0, 1, 2,
                    GridBagConstraints.WEST, (Container)fileDialog,
                   1, 0, GridBagConstraints.HORIZONTAL, textFieldInset);
        

       
        label = new Label(filterLabelText);

        label.setFont(f);
        addComponent(label, gbl, gbc, 0, 2, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.NONE, labelInset);
        addComponent(filterField, gbl, gbc, 0, 3, 2,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.HORIZONTAL, textFieldInset);

        label = new Label(foldersLabelText);

        label.setFont(f);
        addComponent(label, gbl, gbc, 0, 4, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.NONE, labelInset);

        label = new Label(filesLabelText);

        label.setFont(f);
        addComponent(label, gbl, gbc, 1, 4, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.NONE, labelInset);
        addComponent(directoryList, gbl, gbc, 0, 5, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 1, GridBagConstraints.BOTH, leftListInset);
        addComponent(fileList, gbl, gbc, 1, 5, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 1, GridBagConstraints.BOTH, rightListInset);

        label = new Label(enterFileNameLabelText); 

        label.setFont(f);
        addComponent(label, gbl, gbc, 0, 6, 1,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.NONE, labelInset);
        addComponent(selectionField, gbl, gbc, 0, 7, 2,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.HORIZONTAL, textFieldInset);
        addComponent(new Separator(fileDialog.size().width, 2, Separator.HORIZONTAL), gbl, gbc, 0, 8, 15,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.HORIZONTAL, separatorInset);

        // add buttons to GridBagLayout Buttons
        addComponent(openButton, gblButtons, gbc, 0, 0, 1,
                     GridBagConstraints.WEST, (Container)buttons,
                     1, 0, GridBagConstraints.NONE, noInset);
        addComponent(filterButton, gblButtons, gbc, 1, 0, 1,
                     GridBagConstraints.CENTER, (Container)buttons,
                     1, 0, GridBagConstraints.NONE, noInset);
        addComponent(cancelButton, gblButtons, gbc, 2, 0, 1,
                     GridBagConstraints.EAST, (Container)buttons,
                     1, 0, GridBagConstraints.NONE, noInset);

        // add ButtonPanel to the GridBagLayout of this class
        addComponent(buttons, gbl, gbc, 0, 9, 2,
                     GridBagConstraints.WEST, (Container)fileDialog,
                     1, 0, GridBagConstraints.HORIZONTAL, buttonsInset);

        fileDialog.setSize(400, 400);

        // Update choice's popup width
        XChoicePeer choicePeer = (XChoicePeer)pathChoice.getPeer();
        choicePeer.setDrawSelectedItem(false);
        choicePeer.setAlignUnder(pathField);
        
        filterField.addActionListener(this);
        selectionField.addActionListener(this);
        directoryList.addActionListener(this);
        directoryList.addItemListener(this);
        fileList.addItemListener(this);
        fileList.addActionListener(this);
        openButton.addActionListener(this);
        filterButton.addActionListener(this);
        cancelButton.addActionListener(this);
        pathChoice.addItemListener(this);
        pathField.addActionListener(this);

        KeyboardFocusManager.getCurrentKeyboardFocusManager()
            .addKeyEventDispatcher(this);
    }

    /**
     * add Component comp to the container cont.
     * add the component to the correct GridBagLayout
     */
    void addComponent(Component comp, GridBagLayout gb, GridBagConstraints c, int gridx,
                      int gridy, int gridwidth, int anchor, Container cont, int weightx, int weighty,
                      int fill, Insets in) {
        c.gridx = gridx;
        c.gridy = gridy;
        c.gridwidth = gridwidth;
        c.anchor = anchor;
        c.weightx = weightx;
        c.weighty = weighty;
        c.fill = fill;
        c.insets = in;
        gb.setConstraints(comp, c);
        cont.add(comp);
    }

    /**
     * get fileName
     */
    String getFileName(String str) {
        if (str == null) {
            return "";
        }

        int index = str.lastIndexOf('/');

        if (index == -1) {
            return str;
        } else {
            return str.substring(index + 1);
        }
    }

    /** handleFilter
     *
     */
    void handleFilter(String f) {

        if (f == null) {
            return;
        }
        setFilterEntry(dir,f);
    }

    /**
     * handle the selection event
     */
    void handleSelection(String file) {
        int index = file.lastIndexOf('/');

        if (index == -1) {
            savedDir = this.dir;
            savedFile = file;
        } else {
            savedDir = file.substring(0, index+1);
            savedFile = file.substring(index+1);
        }
        target.setDirectory(savedDir);
        target.setFile(savedFile);
    }

    /**
     * handle the cancel event
     */
    void handleCancel() {
        KeyboardFocusManager.getCurrentKeyboardFocusManager()
            .removeKeyEventDispatcher(this);

        setSelectionField(null);
        setFilterField(null);
        directoryList.clear();
        fileList.clear();
        target.setFile(null);
        target.setDirectory(savedDir);
        handleQuitButton();
    }

    /**
     * handle the quit event
     */
    void handleQuitButton() {        
        dir = null;
        file = null;
        target.hide();
    }

    /**
     * set the entry of the new dir with f
     */
    void setFilterEntry(String d, String f) {
        File fe = new File(d);

        if (fe.isDirectory() && fe.canRead()) {
            setSelectionField(d);

            if (f.equals("")) {
                f = "*";
                setFilterField(f);
            } else {
                setFilterField(f);
            }
            String l[];

            if (f.equals("*")) {
                l = fe.list();
            } else {
                // REMIND: fileDialogFilter is not implemented yet
                FileDialogFilter ff = new FileDialogFilter(f);
                l = fe.list(ff);
            }
            directoryList.clear();
            fileList.clear();
            directoryList.setVisible(false);
            fileList.setVisible(false);

            directoryList.addItem("..");
            Arrays.sort(l);
            for (int i = 0 ; i < l.length ; i++) {
                File file = new File(d + l[i]);
                if (file.isDirectory()) {
                    directoryList.addItem(l[i] + "/");
                } else {
                    if (filter != null) {
                        if (filter.accept(new File(l[i]),l[i]))  fileList.addItem(l[i]);
                    }
                    else fileList.addItem(l[i]);
                }
            }
            this.dir = d;

            pathField.setText(dir);           
            pathChoice.removeAll();
            String dirList[] = getDirList(dir);
            for (int i=0;i<dirList.length;i++) pathChoice.addItem(dirList[i]); 

            target.setDirectory(this.dir);
            directoryList.setVisible(true);
            fileList.setVisible(true);
        }
    }


    String[] getDirList(String dir) {
        if (!dir.endsWith("/"))
            dir = dir + "/";
        char[] charr = dir.toCharArray();
        int numSlashes = 0;
        for (int i=0;i<charr.length;i++) {
           if (charr[i] == '/')
               numSlashes++;
        }
        String[] starr =  new String[numSlashes];
        int j=0;
        for (int i=charr.length-1;i>=0;i--) {
            if (charr[i] == '/')
            {
                starr[j++] = new String(charr,0,i+1);
            }
        }
        return starr;
    }

    /**
     * set the text in the selectionFiel
     */
    void setSelectionField(String str) {
        selectionField.setText(str);
    }

    /**
     * set the text in the filterField
     */
    void setFilterField(String str) {
        filterField.setText(str);
    }

    /**
     *
     * @see java.awt.event.ItemEvent
     * ItemEvent.ITEM_STATE_CHANGED
     */
    public void itemStateChanged( ItemEvent itemEvent ) {

        Object source = itemEvent.getSource();
        if ( source instanceof Choice) {
            String dir = pathChoice.getSelectedItem(); 
            target.setDirectory(dir); 
        }
        else {
            if (itemEvent.getID() == ItemEvent.ITEM_STATE_CHANGED) {
                Object item = itemEvent.getItem();

                switch( itemEvent.getStateChange() ) {
                    case ItemEvent.SELECTED:
                        if ( directoryList == ( (List) source ) ) {
                            int index = ( (Integer) item ).intValue();
                            setFilterField( /*directoryList.getItem( index ) +*/
                                    getFileName( filterField.getText() ) );
                        } else if ( fileList == ( (List) source ) ) {
                            int index = ( (Integer) item ).intValue();
                            setSelectionField( fileList.getItem( index ) );
                        }
                        break;

                    case ItemEvent.DESELECTED:
                        if ( directoryList == ( (List) source ) ) {
                            setFilterField( /*dir +*/
                                    getFileName( filterField.getText() ) );
                        } else if ( fileList == ( (List) source ) ) {
                            setSelectionField( dir );
                        }
                        break;
                }
            }
        }
    }

    void checkForUpDir(String str) {

        if (str.equals("..")) {
            String temp = this.dir;
            if (!this.dir.equals("/"))   // If the current directory is "/" leave it alone.
            {
                if (dir.endsWith("/"))
                    temp = temp.substring(0,temp.lastIndexOf("/"));

                temp = temp.substring(0,temp.lastIndexOf("/")+1);
            }
            this.dir = temp;
        }
        else
            this.dir = this.dir + str;
    }

    public void actionPerformed( ActionEvent actionEvent ) {

        switch( actionEvent.getID() ) {
        case ActionEvent.ACTION_PERFORMED: {
            String actionCommand = actionEvent.getActionCommand();
            Object source = actionEvent.getSource();

            if (actionCommand.equals(actionButtonText)) {
                handleSelection( selectionField.getText() );
                handleQuitButton();
            } else if (actionCommand.equals(filterLabelText)) {
                handleFilter( filterField.getText() );
            } else if (actionCommand.equals(cancelButtonText)) {
                handleCancel();
            } else if ( source instanceof TextField ) {
                if ( selectionField == ( (TextField) source ) ) {
                    handleFilter( selectionField.getText() );
                    handleQuitButton();

                } else if ( filterField == ( (TextField) source ) ) {
                    handleFilter( filterField.getText() );
                }
                else if ( pathField ==  ( (TextField) source ) ) {
                    target.setDirectory(pathField.getText());
                } 
            } else if ( source instanceof List ) {
                if ( directoryList == ( (List) source ) ) {
                    //handleFilter( actionCommand + getFileName( filterField.getText() ) );
                    checkForUpDir(actionCommand);
                    handleFilter( getFileName( filterField.getText() ) );

                } else if ( fileList == ( (List) source ) ) {
                    handleSelection( actionCommand );
                    handleQuitButton();
                }
            }
        }
        break;
        }
    }
 
    public boolean dispatchKeyEvent(KeyEvent keyEvent) {
        int id = keyEvent.getID();
        int keyCode = keyEvent.getKeyCode();

        if (id == KeyEvent.KEY_PRESSED && keyCode == KeyEvent.VK_ESCAPE) {
            synchronized (target.getTreeLock()) {
                Component comp = (Component) keyEvent.getSource();
                while (comp != null) {
                    if (comp.getPeer() == this) {
                        handleCancel();
                        return true;
                    }
                    comp = comp.getParent();
                }
            }
        }

        return false;
    }
    
    
    /**
     * set the file
     */
    public void setFile(String file) {

        if (file == null) {
            this.file = null;
            return;
        }

        if (this.dir == null) {
            String d = "./";
            File f = new File(d, file);

            if (f.isFile()) {
                this.file = file;
                setDirectory(d);
            }
        } else {
            File f = new File(this.dir, file);
            if (f.isFile()) {
                this.file = file;
            }
        }

        selectionField.setText(file);
    }

    /**
     * set the directory
     */
    public void setDirectory(String dir) {

        if (dir == null) {
            this.dir = null;
            return;
        }

        if (dir.equals(this.dir)) {
            return;
        }

        int i;
        if ((i=dir.indexOf("~")) != -1) {

            dir = dir.substring(0,i) + System.getProperty("user.home") + dir.substring(i+1,dir.length());
        }

        File fe = new File(dir).getAbsoluteFile();
        log.fine("Current directory : " + fe);

        if (!fe.isDirectory()) {
            dir = "./";
            fe = new File(dir).getAbsoluteFile();

            if (!fe.isDirectory()) {
                return;
            }
        }
        try {
            dir = this.dir = fe.getCanonicalPath();
        } catch (java.io.IOException ie) {
            dir = this.dir = fe.getAbsolutePath();            
        }
        pathField.setText(this.dir);


        if (dir.endsWith("/")) {
            this.dir = dir;
            handleFilter("");
        } else {
            this.dir = dir + "/";
            handleFilter("");
        }

        pathChoice.removeAll();
        String dirList[] = getDirList(dir);
        for (i=0;i<dirList.length;i++) pathChoice.addItem(dirList[i]); 
        pathPanel.validate();
    }

    /**
     * set filenameFilter
     * 
     */
    public void setFilenameFilter(FilenameFilter filter) {
        this.filter = filter;
    }

    public void show() {
        if (fileDialog == null) {
            init((FileDialog)target);
        }
        if (savedDir != null) {
            setDirectory(savedDir);
        }
        if (savedFile != null) {
            setFile(savedFile);
        }
        super.show();
    }

    public void dispose() {
        FileDialog fd = (FileDialog)fileDialog;
        if (fd != null) {
            fd.removeAll();
        }
        super.dispose();
    }
}

class Separator extends Canvas {
    public final static int HORIZONTAL = 0;
    public final static int VERTICAL = 1;
    int orientation;

    public Separator(int length, int thickness, int orient) {
        super();
        orientation = orient;
        if (orient == HORIZONTAL) {
            resize(length, thickness);
        } else {
            // VERTICAL
            resize(thickness, length);
        }
    }

    public void paint(Graphics g) {
        int x1, y1, x2, y2;
        Rectangle bbox = bounds();
        Color c = getBackground();
        Color brighter = c.brighter();
        Color darker = c.darker();

        if (orientation == HORIZONTAL) {
            x1 = 0;
            x2 = bbox.width - 1;
            y1 = y2 = bbox.height/2 - 1;

        } else {
            // VERTICAL
            x1 = x2 = bbox.width/2 - 1;
            y1 = 0;
            y2 = bbox.height - 1;
        }
        g.setColor(darker);
        g.drawLine(x1, y2, x2, y2);
        g.setColor(brighter);
        if (orientation == HORIZONTAL)
            g.drawLine(x1, y2+1, x2, y2+1);
        else
            g.drawLine(x1+1, y2, x2+1, y2);
    }
}

/**
* checks the filename and directory with the specified filter
* checks with multiple "*".
* the filter has to start with a '*' character.
* this to keep the search the same as in the motif version
*/
class FileDialogFilter implements FilenameFilter {

    String filter;

    public FileDialogFilter(String f) {
        filter = f;
    }

    /**
     * return true if match
     */
    public boolean accept(File dir, String fileName) {

        File f = new File(dir, fileName);

        if (f.isDirectory()) {
            return true;
        } else {
            return searchPattern(fileName, filter);
        }
    }

    /**
     * start searching
     */
    boolean searchPattern(String fileName, String filter) {
        int filterCursor = 0;
        int fileNameCursor = 0;

        int filterChar = filter.charAt(filterCursor);

        if (filterCursor == 0 && filterChar != '*') {
            return false;
        }
        String ls = filter.substring(filterCursor + 1);
        return handleStar(fileName, ls);
    }

    /**
     * call this method when character was an *
     */
    boolean handleStar(String fileName, String filter) {
        int     ftLen = filter.length();
        int     flLen = fileName.length();
        char    ftChar;
        char    flChar;
        int     ftCur = 0;
        int     flCur = 0;
        int     c = 0;

        if (ftLen == 0) {
            return true;
        }

        while (c < flLen) {
            ftChar = filter.charAt(ftCur);

            if (ftChar == '*') {
                String ls = filter.substring(ftCur + 1);
                String fs = fileName.substring(flCur);
                if (handleStar(fs, ls)) {
                    return true;
                }
                c++;
                flCur = c;
                ftCur = 0;
                continue;
            }
            flChar = fileName.charAt(flCur);

            if (ftChar == flChar) {
                ftCur++;
                flCur++;

                if (flCur == flLen && ftCur == ftLen) {
                    return true;
                }

                if (flCur < flLen && ftCur == ftLen) {
                    return false;
                }

                if (flCur == flLen) {
                    c = flLen;
                }
            } else {
                c++;
                flCur = c;
                ftCur = 0;
                if (c == flLen) {
                    return false;
                }
            }
        }

        for (int i = ftCur ; i < ftLen ; i++) {
            ftChar = filter.charAt(i);
            if (ftChar != '*') {
                return false;
            }
        }
        return true;
    }
}



