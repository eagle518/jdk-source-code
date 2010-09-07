/*
 * @(#)XClipboard.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.datatransfer.Transferable;

import java.util.SortedMap;
import java.util.Map;
import java.util.Set;
import java.util.Iterator;
import java.util.HashSet;

import java.io.IOException;

import java.security.AccessController;

import sun.awt.datatransfer.DataTransferer;
import sun.awt.datatransfer.SunClipboard;
import sun.awt.datatransfer.ClipboardTransferable;

import sun.security.action.GetIntegerAction;



/**
 * A class which interfaces with the X11 selection service in order to support
 * data transfer via Clipboard operations.
 */
public class XClipboard extends SunClipboard implements Runnable {
    private final XSelection selection;

    private static final Object classLock = new Object();

    private static final int defaultPollInterval = 200;

    private static int pollInterval;

    private static Set listenedClipboards;


    /**
     * Creates a system clipboard object.
     */
    public XClipboard(String name, String selectionName) {
        super(name);
        selection = new XSelection(XAtom.get(selectionName), this);
    }

    /**
     * The action to be run when we lose ownership
     * NOTE: This method may be called by privileged threads.
     *       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
     */
    public void run() {
        lostOwnershipImpl();
    }

    protected synchronized void setContentsNative(Transferable contents) {
        SortedMap formatMap = DataTransferer.getInstance().getFormatsForTransferable
	        (contents, DataTransferer.adaptFlavorMap(flavorMap));
        long[] formats =
            DataTransferer.getInstance().keysToLongArray(formatMap);

        if (!selection.setOwner(contents, formatMap, formats, 
                                XlibWrapper.CurrentTime)) {
            this.owner = null;
            this.contents = null;
        }
    }

    public long getID() {
        return selection.getSelectionAtom().getAtom();
    }

    public synchronized Transferable getContents(Object requestor) {
        if (contents != null) {
            return contents;
        }
        return new ClipboardTransferable(this);
    }

    /* Caller is synchronized on this. */
    protected void clearNativeContext() {
        selection.reset();
    }


    protected long[] getClipboardFormats() {
        return selection.getTargets(XlibWrapper.CurrentTime);
    }

    protected byte[] getClipboardData(long format) throws IOException {
        return selection.getData(format, XlibWrapper.CurrentTime);
    }

    // Called on the toolkit thread under awtLock.
    public void checkChange(long[] formats) {
        if (!selection.isOwner()) {
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
        selection.initializeSelectionForTrackingChanges();
        boolean mustSchedule = false;
        synchronized (XClipboard.classLock) {
            if (listenedClipboards == null) {
                listenedClipboards = new HashSet(2);
            }
            mustSchedule = listenedClipboards.isEmpty();
            listenedClipboards.add(this);
        }
        if (mustSchedule) {
            XToolkit.schedule(new CheckChangeTimerTask(), pollInterval);
        }
    }

    private static class CheckChangeTimerTask implements Runnable {
        public void run() {
            for (Iterator iter = listenedClipboards.iterator(); iter.hasNext();) {
                XClipboard clpbrd = (XClipboard)iter.next();
                clpbrd.selection.getTargetsDelayed();
            }
            synchronized (XClipboard.classLock) {
                if (listenedClipboards != null && !listenedClipboards.isEmpty()) {
                    XToolkit.schedule(this, pollInterval);
                }
            }
        }
    }

    protected void unregisterClipboardViewerChecked() {
        selection.deinitializeSelectionForTrackingChanges();
        synchronized (XClipboard.classLock) {
            listenedClipboards.remove(this);
        }
    }

}
