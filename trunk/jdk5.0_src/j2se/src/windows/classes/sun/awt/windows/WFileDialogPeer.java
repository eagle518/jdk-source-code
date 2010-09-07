/*
 * @(#)WFileDialogPeer.java	1.31 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.dnd.DropTarget;
import java.awt.peer.*;
import java.io.File;
import java.io.FilenameFilter;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ResourceBundle;
import java.util.MissingResourceException;

public class WFileDialogPeer extends WWindowPeer implements FileDialogPeer {

    static {
        initIDs();
    }

    private long hwnd = 0;
    private WComponentPeer parent;
	private FilenameFilter fileFilter;

    public void setDirectory(String dir) {}
    public void setFile(String file) {}
    public void setTitle(String title) {}

    //Needed to fix 4152317
    private static native void setFilterString(String allFilter);

    public void	setFilenameFilter(FilenameFilter filter) {
		this.fileFilter = filter;
    }

    boolean checkFilenameFilter(String filename) {
        FileDialog fileDialog = (FileDialog)target;
        if (fileFilter == null) {
            return true;
        }
        File file = new File(filename);
        return fileFilter.accept(new File(file.getParent()), file.getName());
    }

    // Toolkit & peer internals
    WFileDialogPeer(FileDialog target) {
	super(target);
    }

    void create(WComponentPeer parent) {
        this.parent = parent;
    }

    void initialize() {}

    private native void _dispose();
    protected void disposeImpl() {
	WToolkit.targetDisposedPeer(target, this);
	_dispose();
    }


    private native void _show();
    private native void _hide();

    public void show() {
        new Thread(new Runnable() {
            public void run() {
                _show();
		WToolkit.getWToolkit().notifyModalityChange(ModalityEvent.MODALITY_PUSHED);
            }
        }).start();
    }

    public void hide() {
	WToolkit.getWToolkit().notifyModalityChange(ModalityEvent.MODALITY_POPPED);
	_hide();
    }

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    void handleSelected(final String file) {
        final FileDialog fileDialog = (FileDialog)target;
	WToolkit.executeOnEventHandlerThread(fileDialog, new Runnable() {
	    public void run() {
                int index = file.lastIndexOf(java.io.File.separatorChar);/*2509*//*ibm*/
		String dir;

		if (index == -1) {
                    dir = "."+java.io.File.separator;
		    fileDialog.setFile(file);
		}
		else {
		    dir = file.substring(0, index + 1);
		    fileDialog.setFile(file.substring(index + 1));
		}
		fileDialog.setDirectory(dir);
		fileDialog.hide();
	    }
	});
    } // handleSelected()

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    void handleCancel() {
        final FileDialog fileDialog = (FileDialog)target;
	WToolkit.executeOnEventHandlerThread(fileDialog, new Runnable() {
	    public void run() {
	        fileDialog.setFile(null);
		fileDialog.hide();
	    }
	});
    } // handleCancel()

    //This whole static block is a part of 4152317 fix
    static {
        String filterString = (String) AccessController.doPrivileged(
            new PrivilegedAction() {
                public Object run() {
                    try {
                        ResourceBundle rb = ResourceBundle.getBundle("sun.awt.windows.awtLocalization");
                        return rb.getString("allFiles");
                    } catch (MissingResourceException e) {
                        return "All Files";
                    }
                }
            });
        setFilterString(filterString);
    }

    // unused methods.
    public void toFront() {}
    public void toBack() {}
    public void setResizable(boolean resizable) {}
    public void enable() {}
    public void disable() {}
    public void reshape(int x, int y, int width, int height) {}
    public boolean handleEvent(Event e) { return false; }
    public void setForeground(Color c) {}
    public void setBackground(Color c) {}
    public void setFont(Font f) {}
    public boolean requestFocus(boolean temporary,
				boolean focusedWindowChangeAllowed) {
	return false;
    }
    void start() {}
    void invalidate(int x, int y, int width, int height) {}
    public void addDropTarget(DropTarget dt) {}
    public void removeDropTarget(DropTarget dt) {}

    /**
     * Initialize JNI field and method ids
     */
    private static native void initIDs();

    /**
     * WFileDialogPeer doesn't have native pData so we don't do restack on it
     * @see java.awt.peer.ContainerPeer#restack
     */
    public void restack() {
    }    

    /**
     * @see java.awt.peer.ContainerPeer#isRestackSupported
     */
    public boolean isRestackSupported() {
        return false;
    }
}
