/*
 * @(#)SyncFileAccess.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.util;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileNotFoundException;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.OverlappingFileLockException;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;

public class SyncFileAccess {

    public SyncFileAccess(File file) {
        this.file = file;
        this.fileAccSync = new SyncAccess(SyncAccess.SHARED_READ_MODE);
    }

    /**
     * Open the index file, using a RandomAccessFile object with the given mode.
     *
     * The FileChannel will be locked in shared mode, if mode is "r" (read only),
     * otherwise it will be locked exclusively.
     *
     * Open and locking will be tried up to 9 times, while waiting 100ms 
     * after each trial.
     * 
     * @param mode the file mode, either "r" or "rw"
     *
     * @param timeout the max time in ms to wait for the lock.
     *                timeout==0 wait forever.
     *
     * @param privileged if true, the RandomAccessFile cstr is called within
     *                   an AccessController.doPrivileged() block.    
     *
     * @return the opened and locked index file RandomAccessFile instance
     *
     * @throws FileNotFoundException 
     *         index file could not be found
     * @throws IOException 
     *         open and lock failed more than 9 times
     */
    public RandomAccessFileLock openLockRandomAccessFile(final String mode, int timeout, boolean privileged) 
    throws IOException {

        Object fobjL = openLockFileObject(TYPE_RANDOM_ACCESS_FILE, mode, timeout, privileged, false /*dummy*/);
        return (RandomAccessFileLock) fobjL;
    }

    public FileInputStreamLock openLockFileInputStream(int timeout, boolean privileged) 
    throws IOException {

        Object fobjL = openLockFileObject(TYPE_FILE_INPUT_STREAM, "r", timeout, privileged, false /*dummy*/);
        return (FileInputStreamLock) fobjL;
    }

    public FileOutputStreamLock openLockFileOutputStream(boolean append, int timeout, boolean privileged) 
    throws IOException {

        Object fobjL = openLockFileObject(TYPE_FILE_OUTPUT_STREAM, "rw", timeout, privileged, append);
        return (FileOutputStreamLock) fobjL;
    }

    public static class RandomAccessFileLock extends FObjLock {

        private RandomAccessFileLock(RandomAccessFile raf, SyncAccess.Lock lock) {
            super(raf, lock);
        }

        public RandomAccessFile getRandomAccessFile() {
            return (RandomAccessFile) fobj;
        }

    }

    public static class FileInputStreamLock extends FObjLock {

        private FileInputStreamLock(FileInputStream fis, SyncAccess.Lock lock) {
            super(fis, lock);
        }

        public FileInputStream getFileInputStream() {
            return (FileInputStream) fobj;
        }

    }

    public static class FileOutputStreamLock extends FObjLock {

        private FileOutputStreamLock(FileOutputStream fos, SyncAccess.Lock lock) {
            super(fos, lock);
        }

        public FileOutputStream getFileOutputStream() {
            return (FileOutputStream) fobj;
        }

    }

    private static class FObjLock {

        private FObjLock(Object fobj, SyncAccess.Lock lock) {
            this.fobj = fobj;
            this.lock = lock;
        }

        public void release() {
            if(lock!=null) {
                lock.release();
                lock=null;
            }
        }

        protected static Object createFObjLock(int type, Object fobj, SyncAccess.Lock lock) {
            Object fobjL;
            switch(type) {
                case TYPE_RANDOM_ACCESS_FILE:
                    fobjL = new RandomAccessFileLock((RandomAccessFile)fobj, lock);
                    break;
                case TYPE_FILE_INPUT_STREAM:
                    fobjL = new FileInputStreamLock((FileInputStream)fobj, lock);
                    break;
                case TYPE_FILE_OUTPUT_STREAM:
                    fobjL = new FileOutputStreamLock((FileOutputStream)fobj, lock);
                    break;
                default:
                    throw new InternalError("wrong fobj type: "+type);
            }
            return fobjL;
        }

        protected Object fobj;
        private SyncAccess.Lock lock;
    }

    private static final int TYPE_RANDOM_ACCESS_FILE = 1;
    private static final int TYPE_FILE_INPUT_STREAM  = 2;
    private static final int TYPE_FILE_OUTPUT_STREAM = 3;

    private Object openLockFileObject(final int type, final String mode, int timeout, final boolean privileged, final boolean fopt1) 
    throws IOException {

        boolean forever = timeout==0;
        boolean readOnly = mode.equals("r");

        SyncAccess.Lock fileAccLock= fileAccSync.lock(readOnly?SyncAccess.READ_OP:SyncAccess.WRITE_OP);

        Object fobj=null;

        try {
          while (null==fobj) {
            if(privileged) {
                fobj = AccessController.doPrivileged(
                    new PrivilegedAction() {

                        public Object run() {
                            Object o = null;
                            try {
                                switch(type) {
                                    case TYPE_RANDOM_ACCESS_FILE:
                                        o = new RandomAccessFile(file, mode);
                                        break;
                                    case TYPE_FILE_INPUT_STREAM:
                                        o = new FileInputStream(file);
                                        break;
                                    case TYPE_FILE_OUTPUT_STREAM:
                                        o = new FileOutputStream(file.getPath(), fopt1);
                                        break;
                                    default:
                                        throw new InternalError("wrong fobj type: "+type);
                                }
                            } catch (FileNotFoundException fnfe) {
                                Trace.ignoredException(fnfe);
                            }
                            return o;
                        }
                        });
            } else {
                try {
                    switch(type) {
                        case TYPE_RANDOM_ACCESS_FILE:
                            fobj = new RandomAccessFile(file, mode);
                            break;
                        case TYPE_FILE_INPUT_STREAM:
                            fobj = new FileInputStream(file);
                            break;
                        case TYPE_FILE_OUTPUT_STREAM:
                            fobj = new FileOutputStream(file.getPath(), fopt1);
                            break;
                        default:
                            throw new InternalError("wrong fobj type: "+type);
                    }
                } catch (FileNotFoundException fnfe) {
                    Trace.ignoredException(fnfe);
                }
            }
            if(null==fobj) {
                throw new FileNotFoundException("index file not found");
            }

            if (Config.isJavaVersionAtLeast14() == false) {
                // no nio for file locking support before JDK 1.4
                if(fobj!=null) {
                    return FObjLock.createFObjLock(type, fobj, fileAccLock);
                }
                return null;
            }

            FileChannel fc = null;
            switch(type) {
                case TYPE_RANDOM_ACCESS_FILE:
                    fc = ((RandomAccessFile)fobj).getChannel();
                    break;
                case TYPE_FILE_INPUT_STREAM:
                    fc = ((FileInputStream)fobj).getChannel();
                    break;
                case TYPE_FILE_OUTPUT_STREAM:
                    fc = ((FileOutputStream)fobj).getChannel();
                    break;
                default:
                    throw new InternalError("wrong fobj type: "+type);
            }

            if(!fc.isOpen()) {
                fc  = null;
                fobj = null;
                if(!forever && timeout<=0) {
                    throw new IOException("index file could not be opened, timeout reached");
                }
                Trace.println("SyncFileAccess.openLock: index file not " +
                        "opened, remaining TO : "+timeout, TraceLevel.NETWORK);
                try {
                    if(!forever) {
                        Thread.sleep((timeout>100)?100:timeout);
                        timeout-=100;
                    } else {
                        Thread.sleep(100);
                    }
                } catch (Exception e) { }
                continue; // try again ..
            }

            try {
                FileLock fl = null;
                while (fl == null) {
                    try {
                        fl = fc.lock(0L, Long.MAX_VALUE, readOnly);
                    } catch (OverlappingFileLockException ofle) {
                        // File is locked, wait some more
                        // This is an expected race condition of file locking
                        if(!forever && timeout<=0) {
                            throw new IOException("handled OverlappingFileLockException, timeout reached", ofle);
                        }
                        Trace.println("SyncFileAccess.openLock: handled " +
                                "OverlappingFileLockException, remainint TO : "+
                                timeout,TraceLevel.NETWORK);
                        try {
                            if(!forever) {
                                Thread.sleep((timeout>100)?100:timeout);
                                timeout-=100;
                            } else {
                                Thread.sleep(100);
                            }
                        } catch (Exception e) { }
                        fl = null;
                    } catch (IOException e) {
                        // In some situations, such as network mounted home drives, the NIO file
                        // lock operation will fail, and we must unwind properly
                        fobj = null;
                        return null;
                    }
                }
            } catch (ClosedChannelException cce) {
                fc  = null;
                fobj = null;
                if(!forever && timeout<=0) {
                    throw new IOException("handled ClosedChannelException, timeout reached", cce);
                }
                Trace.println("SyncFileAccess.openLock: handled " +
                        "ClosedChannelException, remaining TO: "+timeout,
                        TraceLevel.NETWORK);
                try {
                    if(!forever) {
                        Thread.sleep((timeout>100)?100:timeout);
                        timeout-=100;
                    } else {
                        Thread.sleep(100);
                    }
                } catch (Exception e) { }
            }
          }
        } finally {
            if(fobj==null) {
                fileAccLock.release();
            }
        }
        if(fobj!=null) {
            return FObjLock.createFObjLock(type, fobj, fileAccLock);
        }
        return null;
    }

    private SyncAccess fileAccSync;
    private File       file;
}

