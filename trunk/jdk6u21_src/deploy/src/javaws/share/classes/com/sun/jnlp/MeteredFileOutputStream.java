/*
 * @(#)MeteredFileOutputStream.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.File;
import javax.jnlp.FileContents;
import com.sun.deploy.resources.ResourceManager;

final class MeteredFileOutputStream extends OutputStream {
    static String _message = null;
    private FileContentsImpl _contents;
    private long _written = 0;
    private FileOutputStream _fis;
    
    MeteredFileOutputStream(File file, boolean overwrite, 
    		FileContentsImpl callback) throws FileNotFoundException {
	_fis = new FileOutputStream(file.getAbsolutePath(), overwrite);
	_contents = callback;
	_written = file.length();
	// Initialize message here. 
	// This will always be called by priviledge code (i.e., Java Web Start).
	// The other entry points might be called by sandboxed code 
	// and therefore cannot be initialized
	if (_message == null) {
	    _message = 
		ResourceManager.getString("api.persistence.filesizemessage");
	}
    }
    
    public void write(int b) throws IOException {
	checkWrite(1);
	_fis.write(b);
	_written++;
    }
    
    public void write(byte b[], int off, int len) throws IOException {
	checkWrite(len);
	_fis.write(b, off, len);
	_written += len;
    }
    
    public void write(byte b[]) throws IOException {
	write(b, 0, b.length);
    }
    
    public void close() throws IOException {
        _fis.close();
        super.close();
    }
    
    public void flush() throws IOException {
        _fis.flush();
        super.flush();
    }
    
    private void checkWrite(int len) throws IOException {
	if (_written + len > _contents.getMaxLength()) {
	    throw new IOException(_message);
	}
    }
}

