/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.xml.internal.org.jvnet.mimepull;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Logger;

/**
 * Removing files based on this
 * <a href="http://java.sun.com/developer/technicalArticles/javase/finalization/">article</a>
 *
 * @author Jitendra Kotamraju
 */
final class WeakDataFile extends WeakReference<DataFile> {

    private static final Logger LOGGER = Logger.getLogger(WeakDataFile.class.getName());
    private static final int MAX_ITERATIONS = 2;
    private static ReferenceQueue<DataFile> refQueue = new ReferenceQueue<DataFile>();
    private static List<WeakDataFile> refList = new ArrayList<WeakDataFile>();
    private final File file;
    private final RandomAccessFile raf;

    WeakDataFile(DataFile df, File file) {
        super(df, refQueue);
        refList.add(this);
        this.file = file;
        try {
            raf = new RandomAccessFile(file, "rw");
        } catch(IOException ioe) {
            throw new MIMEParsingException(ioe);
        }
        drainRefQueueBounded();
    }

    synchronized void read(long pointer, byte[] buf, int offset, int length ) {
        try {
            raf.seek(pointer);
            raf.readFully(buf, offset, length);
        } catch(IOException ioe) {
            throw new MIMEParsingException(ioe);
        }
    }

    synchronized long writeTo(long pointer, byte[] data, int offset, int length) {
        try {
            raf.seek(pointer);
            raf.write(data, offset, length);
            return raf.getFilePointer();    // Update pointer for next write
        } catch(IOException ioe) {
            throw new MIMEParsingException(ioe);
        }
    }

    void close() {
        LOGGER.fine("Deleting file = "+file.getName());
        refList.remove(this);
        try {
            raf.close();
            file.delete();
        } catch(IOException ioe) {
            throw new MIMEParsingException(ioe);
        }
    }

    void renameTo(File f) {
        LOGGER.fine("Moving file="+file+" to="+f);
        refList.remove(this);
        try {
            raf.close();
            file.renameTo(f);
        } catch(IOException ioe) {
            throw new MIMEParsingException(ioe);
        }

    }

    static void drainRefQueueBounded() {
        int iterations = 0;
        WeakDataFile weak = (WeakDataFile) refQueue.poll();
        while (weak != null && iterations < MAX_ITERATIONS) {
            LOGGER.fine("Cleaning file = "+weak.file+" from reference queue.");
            weak.close();
            ++iterations;
            weak = (WeakDataFile) refQueue.poll();
        }
    }

}
