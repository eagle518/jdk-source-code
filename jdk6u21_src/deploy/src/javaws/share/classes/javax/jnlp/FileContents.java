/*
 * @(#)FileContents.java	1.23 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;
import java.io.IOException;

/**
 * <code>FileContents</code> objects encapsulate the name
 * and contents of a file. An implementation of this class is
 * used by the <code>FileOpenService</code>,
 * <code>FileSaveService</code>, and
 * <code>PersistenceService</code>.
 * <p>
 * The <code>FileContents</code> implementation returned by
 * {@link PersistenceService#get}, <code>FileOpenService</code>,
 * and <code>FileSaveService</code> should never truncate a file
 * if the maximum file length is set to be less that the current
 * file length.
 *
 * @since 1.0
 *
 * @see FileOpenService
 * @see FileSaveService
 *
 */
public interface FileContents {
    
    /**
     * Gets the file name as a <code>String</code>.
     *
     * @return a string containing the file name.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public String getName() throws IOException;
    
    /**
     * Gets an <code>InputStream</code> from the file.
     *
     * @return an InputStream to the file.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public InputStream getInputStream() throws IOException;
    
    /**
     * Gets an <code>OutputStream</code> to the file.  A JNLP
     * client may implement this interface to return an OutputStream
     * subclass which restricts the amount of data that can be
     * written to the stream.
     *
     * @return an OutputStream from the file.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public OutputStream getOutputStream(boolean overwrite) throws IOException;
    
    /**
     * Gets the length of the file.
     *
     * @return the length of the file as a long.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public long getLength() throws IOException;
    
    /**
     * Returns whether the file can be read.
     *
     * @return true if the file can be read, false otherwise.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public boolean canRead() throws IOException;
    
    /**
     * Returns whether the file can be written to.
     *
     * @return true if the file can be read, false otherwise.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public boolean canWrite() throws IOException;
    
    /**
     * Returns a <code>JNLPRandomAccessFile</code> representing a
     * random access interface to the file's contents.
     * The mode argument must either be equal to "r" or "rw",
     * indicating the file is to be opened for input only or for both
     * input and output, respectively.  An IllegalArgumentException
     * will be thrown if the mode is not equal to "r" or "rw".
     *
     * @param mode  the access mode.
     *
     * @return a JNLPRandomAccessFile.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public JNLPRandomAccessFile getRandomAccessFile(String mode) throws IOException;
    
    /**
     * Gets the maximum file length for the file,
     * as set by the creator of this object.
     *
     * @return the maximum length of the file.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public long getMaxLength() throws IOException;
    
    /**
     * Sets the maximum file length for the file.  A JNLP client
     * may enforce restrictions on setting the maximum file length.
     * A JNLP client should not truncate a file if the maximum file length
     * is set that is less than the current file size, but it also should
     * not allow further writes to that file.
     *
     * @param maxlength the requested new maximum file length.
     *
     * @return the maximum file length that was granted.
     *
     * @throws IOException if an I/O exception occurs.
     */
    public long setMaxLength(long maxlength) throws IOException;
    
}




