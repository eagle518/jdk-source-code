/*
 * @(#)FileContentsImpl.java	1.20 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import javax.jnlp.FileContents;
import javax.jnlp.JNLPRandomAccessFile;
import javax.jnlp.PersistenceService;
import com.sun.jnlp.PersistenceServiceImpl;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.net.MalformedURLException;

/**
 *  <code>FileContents</code> objects encapsulate the name
 *  and contents of a file. An object of this class is
 *  used by the <code>FileOpenService</code> and
 *  <code>FileSaveService</code>.
 *
 *
 * @see 	FileOpenService
 * @see FileSaveService
 *
 */
public final class FileContentsImpl implements FileContents {
    private String _name = null;
    private File _file = null;
    private long _limit = Long.MAX_VALUE;
    private URL _url = null;
    private JNLPRandomAccessFile _raf = null;
    private PersistenceServiceImpl _psCallback = null;
    
    FileContentsImpl(File file, long maxlength) throws IOException {
	_file = file;
	_limit = maxlength;
	_name = _file.getName();
    }
    
    FileContentsImpl(File file, PersistenceServiceImpl callback, URL url, long maxlength) {
	_file = file;
	_url = url;
	_psCallback = callback;
	_limit = maxlength;
	// Name is given by URL
	int index = url.getFile().lastIndexOf('/');
	_name = (index != -1) ? url.getFile().substring(index + 1) : url.getFile();
    }
    
    /** Return name of entry - without path */
    public String getName() { return _name; }
    
    /** Get length of file */
    public long getLength() {
	Long ll = (Long)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return new Long(_file.length());
		    }
		});
	return ll.longValue();
    }
    
    /** Returns an inputstream to file */
    public InputStream getInputStream() throws IOException {
	try {
	    InputStream is = (InputStream)AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    return new FileInputStream(_file);
			}
		    });
	    return is;
	} catch (PrivilegedActionException e) {
	    throw rethrowException(e);
	}
    }
    
    public OutputStream getOutputStream(final boolean append) throws IOException {
	try {
	    OutputStream  os = (OutputStream)AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    return new MeteredFileOutputStream(_file, !append, FileContentsImpl.this);
			}
		    });
	    return os;
	} catch (PrivilegedActionException e) {
	    throw rethrowException(e);
	}
    }
    
    public boolean canRead() throws IOException {
	Boolean bb = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return new Boolean(_file.canRead());
		    }
		});
	return bb.booleanValue();
    }
    
    
    public boolean canWrite() throws IOException {
	Boolean bb = (Boolean)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return new Boolean(_file.canWrite());
		    }
		});
	return bb.booleanValue();
    }
    
    
    public JNLPRandomAccessFile getRandomAccessFile(final String mode) throws IOException {
	try {
	    return (JNLPRandomAccessFile)AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws MalformedURLException, IOException {
			    return new JNLPRandomAccessFileImpl(_file, mode, FileContentsImpl.this);
			}
		    });
	} catch (PrivilegedActionException e) {
	    throw rethrowException(e);
	}
    }
    
    
    public long getMaxLength() throws IOException {
	return _limit;
    }
    
    
    public long setMaxLength(final long limit) throws IOException {
	if (_psCallback != null) {
	    _limit = _psCallback.setMaxLength(_url, limit);
	    return _limit;
	} else {
	    // We could put up a security advisory dialog in this case
	    // too (i.e., when a FileContents is obtained from a FileOpen/FileSave service)
	    // However, it does not really seem like that is neccesary
	    _limit = limit;
	    return _limit;
	}
    }
    
    /** Convers any exception into an IOException */
    private IOException rethrowException(PrivilegedActionException e) throws IOException {
	Exception ee = e.getException();
	if (ee instanceof IOException) {
	    throw new IOException("IOException from FileContents");
	} else if (ee instanceof RuntimeException) {
	    // These are special unchecked exceptions
	    throw (RuntimeException)ee;
	} else {
	    throw new IOException(ee.getMessage());
	}
    }
}

