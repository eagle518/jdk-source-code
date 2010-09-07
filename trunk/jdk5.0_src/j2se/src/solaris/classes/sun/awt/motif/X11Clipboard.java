/*
 * @(#)X11Clipboard.java	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.datatransfer.ClipboardOwner;
import java.awt.datatransfer.Transferable;

import java.io.IOException;

import java.security.AccessController;

import sun.awt.datatransfer.SunClipboard;
import sun.awt.datatransfer.TransferableProxy;
import sun.awt.datatransfer.DataTransferer;

import sun.security.action.GetIntegerAction;


/**
 * A class which interfaces with the X11 selection service in order to support
 * data transfer via Clipboard operations. Most of the work is provided by
 * sun.awt.datatransfer.DataTransferer.
 *
 * @author Amy Fowler
 * @author Roger Brinkley
 * @author Danila Sinopalnikov
 * @author Alexander Gerasimov
 * @version 1.28, 12/19/03
 *
 * @since JDK1.1
 */
public class X11Clipboard extends SunClipboard implements X11SelectionHolder {

    private final X11Selection clipboardSelection;

    private static final Object classLock = new Object();

    private static final int defaultPollInterval = 200;

    private static int pollInterval;

    private static int listenedClipboardsCount;

    /**
     * Creates a system clipboard object.
     */
    public X11Clipboard(String name, String selectionName) {
        super(name);
        clipboardSelection = new X11Selection(selectionName, this);
    }

    protected void setContentsNative(Transferable contents) {
        if (!clipboardSelection.getSelectionOwnership(contents, this)) {
            // Need to figure out how to inform owner the request failed...
            this.owner = null;
            this.contents = null;
        }
    }

    public long getID() {
        return clipboardSelection.atom;
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void lostSelectionOwnership() {
        lostOwnershipImpl();
    }

    protected void clearNativeContext() {
        clipboardSelection.clearNativeContext();
    }

    protected long[] getClipboardFormats() {
        return getClipboardFormats(getID());
    }
    private static native long[] getClipboardFormats(long clipboardID);
  
    protected byte[] getClipboardData(long format)
          throws IOException {
        return getClipboardData(getID(), format);
    }
    private static native byte[] getClipboardData(long clipboardID, long format)
            throws IOException;


    // Called on the toolkit thread under awtLock.
    public void checkChange(long[] formats) {
        if (!clipboardSelection.isOwner()) {
            super.checkChange(formats);
        }
    }

    void checkChangeHere(Transferable contents) {
        if (areFlavorListenersRegistered()) {
            super.checkChange(DataTransferer.getInstance().
                        getFormatsForTransferableAsArray(contents, flavorMap));
        }
    }

    protected void registerClipboardViewerChecked() {
        if (pollInterval <= 0) {
            pollInterval = ((Integer)AccessController.doPrivileged(
                    new GetIntegerAction("awt.datatransfer.clipboard.poll.interval",
                                         defaultPollInterval))).intValue();
            if (pollInterval <= 0) {
                pollInterval = defaultPollInterval;
            }
        }
        synchronized (X11Clipboard.classLock) {
            if (listenedClipboardsCount++ == 0) {
                registerClipboardViewer(pollInterval);
            }
        }
    }

    private native void registerClipboardViewer(int pollInterval);

    protected void unregisterClipboardViewerChecked() {
        synchronized (X11Clipboard.classLock) {
            if (--listenedClipboardsCount == 0) {
                unregisterClipboardViewer();
            }
        }
    }

    private native void unregisterClipboardViewer();

}
