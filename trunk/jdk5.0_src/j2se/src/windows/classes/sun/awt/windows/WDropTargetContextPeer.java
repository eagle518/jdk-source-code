/*
 * @(#)WDropTargetContextPeer.java	1.38 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.UnsupportedFlavorException;

import java.awt.dnd.DnDConstants;
import java.awt.dnd.DropTarget;
import java.awt.dnd.InvalidDnDOperationException;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import java.util.Map;

import sun.awt.PeerEvent;
import sun.awt.SunToolkit;
import sun.awt.dnd.SunDropTargetContextPeer;
import sun.awt.dnd.SunDropTargetEvent;

/**
 * <p>
 * The WDropTargetContextPeer class is the class responsible for handling
 * the interaction between the win32 DnD system and Java.
 * </p>
 *
 * @version 1.38
 *
 */

final class WDropTargetContextPeer extends SunDropTargetContextPeer {
    /**
     * create the peer typically upcall from C++
     */

    static WDropTargetContextPeer getWDropTargetContextPeer() {
	return new WDropTargetContextPeer();
    }

    /**
     * create the peer
     */

    private WDropTargetContextPeer() {
	super();
    }

    /**
     * upcall to encapsulate file transfer
     */

    private static FileInputStream getFileStream(String file, long stgmedium)
        throws IOException
    {
	return new WDropTargetContextPeerFileStream(file, stgmedium);
    }

    /**
     * upcall to encapsulate IStream buffer transfer
     */

    private static Object getIStream(long istream) throws IOException {
	return new WDropTargetContextPeerIStream(istream);
    }

    protected Object getNativeData(long format) {
	return getData(getNativeDragContext(), format);
    }

    /**
     * signal drop complete
     */

    protected void doDropDone(boolean success, int dropAction, 
                              boolean isLocal) {
        dropDone(getNativeDragContext(), success, dropAction);
    }
 
    protected void eventPosted(final SunDropTargetEvent e) {
        if (e.getID() != SunDropTargetEvent.MOUSE_DROPPED) {
            Runnable runnable = new Runnable() {
                    public void run() {
                        e.getDispatcher().unregisterAllEvents();
                    }
                };
            // NOTE: this PeerEvent must be a NORM_PRIORITY event to be
            // dispatched after this SunDropTargetEvent, but before the next 
            // one, so we should pass zero flags.
            PeerEvent peerEvent = new PeerEvent(e.getSource(), runnable, 0);
            SunToolkit.executeOnEventHandlerThread(peerEvent);
        }
    }

    /**
     * downcall to bind transfer data.
     */

     private native Object getData(long nativeContext, long format);

    /**
     * downcall to notify that drop is complete
     */

     private native void dropDone(long nativeContext, boolean success, int action);
}

/**
 * package private class to handle file transfers
 */

class WDropTargetContextPeerFileStream extends FileInputStream {

    /**
     * construct file input stream
     */

    WDropTargetContextPeerFileStream(String name, long stgmedium)
        throws FileNotFoundException
    {
	super(name);

	this.stgmedium  = stgmedium;
    }

    /**
     * close
     */

    public void close() throws IOException {
        if (stgmedium != 0) {
            super.close();
            freeStgMedium(stgmedium);
            stgmedium = 0;
        }
    }

    /**
     * free underlying windows data structure
     */

    private native void freeStgMedium(long stgmedium);

    /*
     * fields
     */

    private long    stgmedium;
}

/**
 * Package private class to access IStream objects
 */

class WDropTargetContextPeerIStream extends InputStream {

    /**
     * construct a WDropTargetContextPeerIStream wrapper
     */

    WDropTargetContextPeerIStream(long istream) throws IOException {
	super();

	if (istream == 0) throw new IOException("No IStream");

	this.istream    = istream;
    }

    /**
     * @return bytes available
     */

    public int available() throws IOException {
	if (istream == 0) throw new IOException("No IStream");
        return Available(istream);
    }

    private native int Available(long istream);

    /**
     * read
     */

    public int read() throws IOException {
	if (istream == 0) throw new IOException("No IStream");
        return Read(istream);
    }

    private native int Read(long istream) throws IOException;

    /**
     * read into buffer
     */

    public int read(byte[] b, int off, int len) throws IOException {
	if (istream == 0) throw new IOException("No IStream");
        return ReadBytes(istream, b, off, len);
    }

    private native int ReadBytes(long istream, byte[] b, int off, int len) throws IOException;

    /**
     * close
     */

    public void close() throws IOException {
        if (istream != 0) {
            super.close();
            Close(istream);
            istream = 0;
        }
    }

    private native void Close(long istream);

    /*
     * fields
     */

    private long istream;
}
