/*
 * @(#)JNLPRandomAccessFile.java	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;
import java.io.File;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 * Instances of this class support both reading and writing to a
 * random access file. A random access file behaves like a large
 * array of bytes stored in the file system. There is a kind of cursor,
 * or index into the implied array, called the <em>file pointer</em>;
 * input operations read bytes starting at the file pointer and advance
 * the file pointer past the bytes read. If the random access file is
 * created in read/write mode, then output operations are also available;
 * output operations write bytes starting at the file pointer and advance
 * the file pointer past the bytes written. Output operations that write
 * past the current end of the implied array cause the array to be
 * extended. The file pointer can be read by the
 * <code>getFilePointer</code> method and set by the <code>seek</code>
 * method.
 * <p>
 * It is generally true of all the reading routines in this class that
 * if end-of-file is reached before the desired number of bytes has been
 * read, an <code>EOFException</code> (which is a kind of
 * <code>IOException</code>) is thrown. If any byte cannot be read for
 * any reason other than end-of-file, an <code>IOException</code> other
 * than <code>EOFException</code> is thrown. In particular, an
 * <code>IOException</code> may be thrown if the stream has been closed.
 * <p>
 * Implementations are encouraged to enforce a maximum amount of data
 * that may be written to a file.
 *
 * @since 1.0
 */

public interface JNLPRandomAccessFile extends DataInput, DataOutput {
    
    /**
     * Closes this random access file stream and releases any system
     * resources associated with the stream. A closed random access
     * file cannot perform input or output operations and cannot be
     * reopened.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public void close() throws IOException;
    
    /**
     * Returns the length of this file.
     *
     * @return     the length of this file, measured in bytes.
     * @exception  IOException  if an I/O error occurs.
     */
    public long length() throws IOException;
    
    /**
     * Returns the current offset in this file.
     *
     * @return     the offset from the beginning of the file, in bytes,
     *             at which the next read or write occurs.
     * @exception  IOException  if an I/O error occurs.
     */
    public long getFilePointer() throws IOException;
    
    /**
     * Reads a byte of data from this file. The byte is returned as an
     * integer in the range 0 to 255 (<code>0x00-0x0ff</code>). This
     * method blocks if no input is yet available.
     * <p>
     * Although <code>JNLPRandomAccessFile</code> is not a subclass of
     * <code>InputStream</code>, this method behaves in exactly the same
     * way as the read method of
     * <code>InputStream</code>.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             file has been reached.
     * @exception  IOException  if an I/O error occurs. Not thrown if
     *                          end-of-file has been reached.
     */
    public int read() throws IOException;
    
    /**
     * Reads up to <code>len</code> bytes of data from this file into an
     * array of bytes. This method blocks until at least one byte of input
     * is available.
     * <p>
     * Although <code>JNLPRandomAccessFile</code> is not a subclass of
     * <code>InputStream</code>, this method behaves in the exactly the
     * same way as the read(byte[], int, int) method of
     * <code>InputStream</code>.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the file has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public int read(byte [] b, int off, int len) throws IOException;
    
    /**
     * Reads up to <code>b.length</code> bytes of data from this file
     * into an array of bytes. This method blocks until at least one byte
     * of input is available.
     * <p>
     * Although <code>JNLPRandomAccessFile</code> is not a subclass of
     * <code>InputStream</code>, this method behaves in the exactly the
     * same way as the read(byte[]) method of
     * <code>InputStream</code>.
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             this file has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public int read(byte [] b) throws IOException;
    
    /**
     * Reads <code>b.length</code> bytes from this file into the byte
     * array, starting at the current file pointer. This method reads
     * repeatedly from the file until the requested number of bytes are
     * read. This method blocks until the requested number of bytes are
     * read, the end of the stream is detected, or an exception is thrown.
     *
     * @param      b   the buffer into which the data is read.
     * @exception  EOFException  if this file reaches the end before reading
     *               all the bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public void readFully(byte [] b) throws IOException;
    
    /**
     * Reads exactly <code>len</code> bytes from this file into the byte
     * array, starting at the current file pointer. This method reads
     * repeatedly from the file until the requested number of bytes are
     * read. This method blocks until the requested number of bytes are
     * read, the end of the stream is detected, or an exception is thrown.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the number of bytes to read.
     * @exception  EOFException  if this file reaches the end before reading
     *               all the bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public void readFully(byte b[], int off, int len) throws IOException;
    
    /**
     * Attempts to skip over <code>n</code> bytes of input discarding the
     * skipped bytes.
     * <p>
     *
     * This method may skip over some smaller number of bytes, possibly zero.
     * This may result from any of a number of conditions; reaching end of
     * file before <code>n</code> bytes have been skipped is only one
     * possibility. This method never throws an <code>EOFException</code>.
     * The actual number of bytes skipped is returned.  If <code>n</code>
     * is negative, no bytes are skipped.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    public int skipBytes(int n) throws IOException;
    
    /**
     * Reads a <code>boolean</code> from this file. This method reads a
     * single byte from the file, starting at the current file pointer.
     * A value of <code>0</code> represents
     * <code>false</code>. Any other value represents <code>true</code>.
     * This method blocks until the byte is read, the end of the stream
     * is detected, or an exception is thrown.
     *
     * @return     the <code>boolean</code> value read.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    public boolean readBoolean() throws IOException;
    
    /**
     * Reads a signed eight-bit value from this file. This method reads a
     * byte from the file, starting from the current file pointer.
     * If the byte read is <code>b</code>, where
     * <code>0&nbsp;&lt;=&nbsp;b&nbsp;&lt;=&nbsp;255</code>,
     * then the result is:
     * <blockquote><pre>
     *     (byte)(b)
     * </pre></blockquote>
     * <p>
     * This method blocks until the byte is read, the end of the stream
     * is detected, or an exception is thrown.
     *
     * @return     the next byte of this file as a signed eight-bit
     *             <code>byte</code>.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    public byte readByte() throws IOException;
    
    /**
     * Reads an unsigned eight-bit number from this file. This method reads
     * a byte from this file, starting at the current file pointer,
     * and returns that byte.
     * <p>
     * This method blocks until the byte is read, the end of the stream
     * is detected, or an exception is thrown.
     *
     * @return     the next byte of this file, interpreted as an unsigned
     *             eight-bit number.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    public int readUnsignedByte() throws IOException;
    
    /**
     * Reads a signed 16-bit number from this file. The method reads two
     * bytes from this file, starting at the current file pointer.
     * If the two bytes read, in order, are
     * <code>b1</code> and <code>b2</code>, where each of the two values is
     * between <code>0</code> and <code>255</code>, inclusive, then the
     * result is equal to:
     * <blockquote><pre>
     *     (short)((b1 &lt;&lt; 8) | b2)
     * </pre></blockquote>
     * <p>
     * This method blocks until the two bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next two bytes of this file, interpreted as a signed
     *             16-bit number.
     * @exception  EOFException  if this file reaches the end before reading
     *               two bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public short readShort() throws IOException;
    
    /**
     * Reads an unsigned 16-bit number from this file. This method reads
     * two bytes from the file, starting at the current file pointer.
     * If the bytes read, in order, are
     * <code>b1</code> and <code>b2</code>, where
     * <code>0&nbsp;&lt;=&nbsp;b1, b2&nbsp;&lt;=&nbsp;255</code>,
     * then the result is equal to:
     * <blockquote><pre>
     *     (b1 &lt;&lt; 8) | b2
     * </pre></blockquote>
     * <p>
     * This method blocks until the two bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next two bytes of this file, interpreted as an unsigned
     *             16-bit integer.
     * @exception  EOFException  if this file reaches the end before reading
     *               two bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public int readUnsignedShort() throws IOException;
    
    /**
     * Reads a Unicode character from this file. This method reads two
     * bytes from the file, starting at the current file pointer.
     * If the bytes read, in order, are
     * <code>b1</code> and <code>b2</code>, where
     * <code>0&nbsp;&lt;=&nbsp;b1,&nbsp;b2&nbsp;&lt;=&nbsp;255</code>,
     * then the result is equal to:
     * <blockquote><pre>
     *     (char)((b1 &lt;&lt; 8) | b2)
     * </pre></blockquote>
     * <p>
     * This method blocks until the two bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next two bytes of this file as a Unicode character.
     * @exception  EOFException  if this file reaches the end before reading
     *               two bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public char readChar() throws IOException;
    
    /**
     * Reads a signed 32-bit integer from this file. This method reads 4
     * bytes from the file, starting at the current file pointer.
     * If the bytes read, in order, are <code>b1</code>,
     * <code>b2</code>, <code>b3</code>, and <code>b4</code>, where
     * <code>0&nbsp;&lt;=&nbsp;b1, b2, b3, b4&nbsp;&lt;=&nbsp;255</code>,
     * then the result is equal to:
     * <blockquote><pre>
     *     (b1 &lt;&lt; 24) | (b2 &lt;&lt; 16) + (b3 &lt;&lt; 8) + b4
     * </pre></blockquote>
     * <p>
     * This method blocks until the four bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next four bytes of this file, interpreted as an
     *             <code>int</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *               four bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public int readInt() throws IOException;
    
    /**
     * Reads a signed 64-bit integer from this file. This method reads eight
     * bytes from the file, starting at the current file pointer.
     * If the bytes read, in order, are
     * <code>b1</code>, <code>b2</code>, <code>b3</code>,
     * <code>b4</code>, <code>b5</code>, <code>b6</code>,
     * <code>b7</code>, and <code>b8,</code> where:
     * <blockquote><pre>
     *     0 &lt;= b1, b2, b3, b4, b5, b6, b7, b8 &lt;=255,
     * </pre></blockquote>
     * <p>
     * then the result is equal to:
     * <p><blockquote><pre>
     *     ((long)b1 &lt;&lt; 56) + ((long)b2 &lt;&lt; 48)
     *     + ((long)b3 &lt;&lt; 40) + ((long)b4 &lt;&lt; 32)
     *     + ((long)b5 &lt;&lt; 24) + ((long)b6 &lt;&lt; 16)
     *     + ((long)b7 &lt;&lt; 8) + b8
     * </pre></blockquote>
     * <p>
     * This method blocks until the eight bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next eight bytes of this file, interpreted as a
     *             <code>long</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *               eight bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public long readLong() throws IOException;
    
    /**
     * Reads a <code>float</code> from this file. This method reads an
     * <code>int</code> value, starting at the current file pointer,
     * as if by the <code>readInt</code> method
     * and then converts that <code>int</code> to a <code>float</code>
     * using the <code>intBitsToFloat</code> method in class
     * <code>Float</code>.
     * <p>
     * This method blocks until the four bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next four bytes of this file, interpreted as a
     *             <code>float</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *             four bytes.
     * @exception  IOException   if an I/O error occurs.
     * @see        javax.jnlp.JNLPRandomAccessFile#readInt()
     */
    public float readFloat() throws IOException;
    
    /**
     * Reads a <code>double</code> from this file. This method reads a
     * <code>long</code> value, starting at the current file pointer,
     * as if by the <code>readLong</code> method
     * and then converts that <code>long</code> to a <code>double</code>
     * using the <code>longBitsToDouble</code> method in
     * class <code>Double</code>.
     * <p>
     * This method blocks until the eight bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     the next eight bytes of this file, interpreted as a
     *             <code>double</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *             eight bytes.
     * @exception  IOException   if an I/O error occurs.
     * @see        javax.jnlp.JNLPRandomAccessFile#readLong()
     */
    public double readDouble() throws IOException;
    
    /**
     * Reads the next line of text from this file.  This method successively
     * reads bytes from the file, starting at the current file pointer,
     * until it reaches a line terminator or the end
     * of the file.  Each byte is converted into a character by taking the
     * byte's value for the lower eight bits of the character and setting the
     * high eight bits of the character to zero.  This method does not,
     * therefore, support the full Unicode character set.
     *
     * <p> A line of text is terminated by a carriage-return character
     * (<code>'&#92;r'</code>), a newline character (<code>'&#92;n'</code>), a
     * carriage-return character immediately followed by a newline character,
     * or the end of the file.  Line-terminating characters are discarded and
     * are not included as part of the string returned.
     *
     * <p> This method blocks until a newline character is read, a carriage
     * return and the byte following it are read (to see if it is a newline),
     * the end of the file is reached, or an exception is thrown.
     *
     * @return     the next line of text from this file, or null if end
     *             of file is encountered before even one byte is read.
     * @exception  IOException  if an I/O error occurs.
     */
    public String readLine() throws IOException;
    
    /**
     * Reads in a string from this file. The string has been encoded
     * using a modified UTF-8 format.
     * <p>
     * The first two bytes are read, starting from the current file
     * pointer, as if by
     * <code>readUnsignedShort</code>. This value gives the number of
     * following bytes that are in the encoded string, not
     * the length of the resulting string. The following bytes are then
     * interpreted as bytes encoding characters in the UTF-8 format
     * and are converted into characters.
     * <p>
     * This method blocks until all the bytes are read, the end of the
     * stream is detected, or an exception is thrown.
     *
     * @return     a Unicode string.
     * @exception  EOFException            if this file reaches the end before
     *               reading all the bytes.
     * @exception  IOException             if an I/O error occurs.
     * @exception  UTFDataFormatException  if the bytes do not represent
     *               valid UTF-8 encoding of a Unicode string.
     * @see        javax.jnlp.JNLPRandomAccessFile#readUnsignedShort()
     */
    public String readUTF() throws IOException;
    
    /**
     * Sets the file-pointer offset, measured from the beginning of this
     * file, at which the next read or write occurs.  The offset may be
     * set beyond the end of the file. Setting the offset beyond the end
     * of the file does not change the file length.  The file length will
     * change only by writing after the offset has been set beyond the end
     * of the file.
     *
     * @param      pos   the offset position, measured in bytes from the
     *                   beginning of the file, at which to set the file
     *                   pointer.
     * @exception  IOException  if <code>pos</code> is less than
     *                          <code>0</code> or if an I/O error occurs.
     */
    public void seek(long pos) throws IOException;
    
    /**
     * Sets the length of this file.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is greater than the <code>newLength</code>
     * argument then the file will be truncated.  In this case, if the file
     * offset as returned by the <code>getFilePointer</code> method is greater
     * then <code>newLength</code> then after this method returns the offset
     * will be equal to <code>newLength</code>.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is smaller than the <code>newLength</code>
     * argument then the file will be extended.  In this case, the contents of
     * the extended portion of the file are not defined.
     *
     * @param      newLength    The desired length of the file
     * @exception  IOException  If an I/O error occurs
     */
    public void setLength(long newLength) throws IOException;
    
    /**
     * Writes the specified byte to this file. The write starts at
     * the current file pointer.
     *
     * @param      b   the <code>byte</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(int b) throws IOException;
    
    /**
     * Writes <code>b.length</code> bytes from the specified byte array
     * to this file, starting at the current file pointer.
     *
     * @param      b   the data.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(byte b[]) throws IOException;
    
    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this file.
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(byte b[], int off, int len) throws IOException;
    
    /**
     * Writes a <code>boolean</code> to the file as a one-byte value. The
     * value <code>true</code> is written out as the value
     * <code>(byte)1</code>; the value <code>false</code> is written out
     * as the value <code>(byte)0</code>. The write starts at
     * the current position of the file pointer.
     *
     * @param      v   a <code>boolean</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeBoolean(boolean v) throws IOException;
    
    /**
     * Writes a <code>byte</code> to the file as a one-byte value. The
     * write starts at the current position of the file pointer.
     *
     * @param      v   a <code>byte</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeByte(int v) throws IOException;
    
    /**
     * Writes a <code>short</code> to the file as two bytes, high byte first.
     * The write starts at the current position of the file pointer.
     *
     * @param      v   a <code>short</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeShort(int v) throws IOException;
    
    /**
     * Writes a <code>char</code> to the file as a two-byte value, high
     * byte first. The write starts at the current position of the
     * file pointer.
     *
     * @param      v   a <code>char</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeChar(int v) throws IOException;
    
    /**
     * Writes an <code>int</code> to the file as four bytes, high byte first.
     * The write starts at the current position of the file pointer.
     *
     * @param      v   an <code>int</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeInt(int v) throws IOException;
    
    /**
     * Writes a <code>long</code> to the file as eight bytes, high byte first.
     * The write starts at the current position of the file pointer.
     *
     * @param      v   a <code>long</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeLong(long v) throws IOException;
    
    /**
     * Converts the float argument to an <code>int</code> using the
     * <code>floatToIntBits</code> method in class <code>Float</code>,
     * and then writes that <code>int</code> value to the file as a
     * four-byte quantity, high byte first. The write starts at the
     * current position of the file pointer.
     *
     * @param      v   a <code>float</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeFloat(float v) throws IOException;
    
    /**
     * Converts the double argument to a <code>long</code> using the
     * <code>doubleToLongBits</code> method in class <code>Double</code>,
     * and then writes that <code>long</code> value to the file as an
     * eight-byte quantity, high byte first. The write starts at the current
     * position of the file pointer.
     *
     * @param      v   a <code>double</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeDouble(double v) throws IOException;
    
    /**
     * Writes the string to the file as a sequence of bytes. Each
     * character in the string is written out, in sequence, by discarding
     * its high eight bits. The write starts at the current position of
     * the file pointer.
     *
     * @param      s   a string of bytes to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeBytes(String s) throws IOException;
    
    /**
     * Writes a string to the file as a sequence of characters. Each
     * character is written to the data output stream as if by the
     * <code>writeChar</code> method. The write starts at the current
     * position of the file pointer.
     *
     * @param      s   a <code>String</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.jnlp.JNLPRandomAccessFile#writeChar(int)
     */
    public void writeChars(String s) throws IOException;
    
    /**
     * Writes a string to the file using UTF-8 encoding in a
     * machine-independent manner.
     * <p>
     * First, two bytes are written to the file, starting at the
     * current file pointer, as if by the
     * <code>writeShort</code> method giving the number of bytes to
     * follow. This value is the number of bytes actually written out,
     * not the length of the string. Following the length, each character
     * of the string is output, in sequence, using the UTF-8 encoding
     * for each character.
     *
     * @param      str   a string to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void writeUTF(String str) throws IOException;
    
}


