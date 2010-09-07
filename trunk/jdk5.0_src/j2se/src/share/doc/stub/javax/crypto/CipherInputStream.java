/*
 * @(#)CipherInputStream.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.io.*;

/** 
 * A CipherInputStream is composed of an InputStream and a Cipher so
 * that read() methods return data that are read in from the
 * underlying InputStream but have been additionally processed by the
 * Cipher.  The Cipher must be fully initialized before being used by
 * a CipherInputStream.
 *
 * <p> For example, if the Cipher is initialized for decryption, the
 * CipherInputStream will attempt to read in data and decrypt them,
 * before returning the decrypted data.
 *
 * <p> This class adheres strictly to the semantics, especially the
 * failure semantics, of its ancestor classes
 * java.io.FilterInputStream and java.io.InputStream.  This class has
 * exactly those methods specified in its ancestor classes, and
 * overrides them all.  Moreover, this class catches all exceptions
 * that are not thrown by its ancestor classes.  In particular, the
 * <code>skip</code> method skips, and the <code>available</code>
 * method counts only data that have been processed by the encapsulated Cipher.
 *
 * <p> It is crucial for a programmer using this class not to use
 * methods that are not defined or overriden in this class (such as a
 * new method or constructor that is later added to one of the super
 * classes), because the design and implementation of those methods
 * are unlikely to have considered security impact with regard to
 * CipherInputStream.
 *
 * @author  Li Gong
 * @version 1.27, 10/29/03
 * @see     java.io.InputStream
 * @see     java.io.FilterInputStream
 * @see     javax.crypto.Cipher
 * @see     javax.crypto.CipherOutputStream
 * 
 * @since 1.4
 */
public class CipherInputStream extends FilterInputStream
{

    /** 
     * Constructs a CipherInputStream from an InputStream and a
     * Cipher.
     * <br>Note: if the specified input stream or cipher is
     * null, a NullPointerException may be thrown later when
     * they are used.
     * @param is the to-be-processed input stream
     * @param c an initialized Cipher object
     */
    public CipherInputStream(InputStream is, Cipher c) { }

    /** 
     * Constructs a CipherInputStream from an InputStream without
     * specifying a Cipher. This has the effect of constructing a
     * CipherInputStream using a NullCipher.
     * <br>Note: if the specified input stream is null, a
     * NullPointerException may be thrown later when it is used.
     * @param is the to-be-processed input stream
     */
    protected CipherInputStream(InputStream is) { }

    /** 
     * Reads the next byte of data from this input stream. The value 
     * byte is returned as an <code>int</code> in the range 
     * <code>0</code> to <code>255</code>. If no byte is available 
     * because the end of the stream has been reached, the value 
     * <code>-1</code> is returned. This method blocks until input data 
     * is available, the end of the stream is detected, or an exception 
     * is thrown. 
     * <p>
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     * @exception  IOException  if an I/O error occurs.
     * @since JCE1.2
     */
    public int read() throws IOException { }

    /** 
     * Reads up to <code>b.length</code> bytes of data from this input 
     * stream into an array of bytes. 
     * <p>
     * The <code>read</code> method of <code>InputStream</code> calls 
     * the <code>read</code> method of three arguments with the arguments 
     * <code>b</code>, <code>0</code>, and <code>b.length</code>. 
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> is there is no more data because the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.InputStream#read(byte[], int, int)
     * @since      JCE1.2
     */
    public int read(byte[] b) throws IOException { }

    /** 
     * Reads up to <code>len</code> bytes of data from this input stream 
     * into an array of bytes. This method blocks until some input is 
     * available. If the first argument is <code>null,</code> up to 
     * <code>len</code> bytes are read and discarded. 
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.InputStream#read()
     * @since      JCE1.2
     */
    public int read(byte[] b, int off, int len) throws IOException { }

    /** 
     * Skips <code>n</code> bytes of input from the bytes that can be read
     * from this input stream without blocking.
     *
     * <p>Fewer bytes than requested might be skipped.
     * The actual number of bytes skipped is equal to <code>n</code> or
     * the result of a call to
     * {@link #available() <code>available</code>},
     * whichever is smaller.
     * If <code>n</code> is less than zero, no bytes are skipped.
     *
     * <p>The actual number of bytes skipped is returned. 
     *
     * @param      n the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     * @since JCE1.2
     */
    public long skip(long n) throws IOException { }

    /** 
     * Returns the number of bytes that can be read from this input 
     * stream without blocking. The <code>available</code> method of 
     * <code>InputStream</code> returns <code>0</code>. This method 
     * <B>should</B> be overridden by subclasses. 
     *
     * @return     the number of bytes that can be read from this input stream
     *             without blocking.
     * @exception  IOException  if an I/O error occurs.
     * @since	   JCE1.2
     */
    public int available() throws IOException { }

    /** 
     * Closes this input stream and releases any system resources 
     * associated with the stream. 
     * <p>
     * The <code>close</code> method of <code>CipherInputStream</code>
     * calls the <code>close</code> method of its underlying input
     * stream.
     *
     * @exception  IOException  if an I/O error occurs.
     * @since JCE1.2
     */
    public void close() throws IOException { }

    /** 
     * Tests if this input stream supports the <code>mark</code> 
     * and <code>reset</code> methods, which it does not.
     *
     * @return  <code>false</code>, since this class does not support the
     *          <code>mark</code> and <code>reset</code> methods.
     * @see     java.io.InputStream#mark(int)
     * @see     java.io.InputStream#reset()
     * @since   JCE1.2
     */
    public boolean markSupported() { }
}
