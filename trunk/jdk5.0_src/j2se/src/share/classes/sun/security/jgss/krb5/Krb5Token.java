/*
 * @(#)Krb5Token.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.EOFException;
import sun.security.util.*;

/**
 * This class represents a base class for all Kerberos v5 GSS-API
 * tokens. It contains commonly used definitions and utilities.
 *
 * @author Mayank Upadhyay
 * @version 1.9, 12/19/03
 */

abstract class Krb5Token {
  
    // TBD: Delete this
    public static final int TBD = -1;

    /**
     * The token id defined for the token emitted by the initSecContext call
     * carrying the AP_REQ .
     */
    public static final int AP_REQ_ID = 0x0100;
    
    /**
     * The token id defined for the token emitted by the acceptSecContext call
     * carrying the AP_REP .
     */
    public static final int AP_REP_ID = 0x0200;
    
    /**
     * The token id defined for any token carrying a KRB-ERR message.
     */
    public static final int ERR_ID    = 0x0300;
    
    /**
     * The token id defined for the token emitted by the getMIC call.
     */
    public static final int MIC_ID    = 0x0101;
    
    /**
     * The token id defined for the token emitted by the wrap call.
     */
    public static final int WRAP_ID   = 0x0201;
    
    /**
     * The object identifier corresponding to the Kerberos v5 GSS-API
     * mechanism.
     */
    public static ObjectIdentifier OID;

    static {
	try {
	    OID = new ObjectIdentifier(Krb5MechFactory.
				       GSS_KRB5_MECH_OID.toString());
      } catch (IOException ioe) {
          // should not happen
      }
    }

    /**
     * Copies an integer value to a byte array in little endian form.
     * @param value the integer value to write
     * @param array the byte array into which the integer must be copied. It
     * is assumed that the array will be large enough to hold the 4 bytes of
     * the integer.
     */
    public static final void writeLittleEndian(int value, byte[] array) {
	writeLittleEndian(value, array, 0);
    }
    
    /**
     * Copies an integer value to a byte array in little endian form.
     * @param value the integer value to write
     * @param array the byte array into which the integer must be copied. It
     * is assumed that the array will be large enough to hold the 4 bytes of
     * the integer.
     * @param pos the position at which to start writing
     */
    public static final void writeLittleEndian(int value, byte[] array,
					       int pos) {
	array[pos++] = (byte)(value);
	array[pos++] = (byte)((value>>>8));
	array[pos++] = (byte)((value>>>16));
	array[pos++] = (byte)((value>>>24));
    }
    
    /**
     * Reads an integer value from a byte array in little endian form. This
     * method allows the reading of two byte values as well as four bytes
     * values both of which are needed in the Kerberos v5 GSS-API mechanism.
     *
     * @param data the array containing the bytes of the integer value
     * @param pos the offset in the array
     * @size the number of bytes to read from the array.
     * @return the integer value
     */
    public static final int readLittleEndian(byte[] data, int pos, int size) {
	int retVal = 0;
	int shifter = 0;
	while (size > 0) {
	    retVal += (data[pos] & 0xff) << shifter;
	    shifter += 8;
	    pos++;
	    size--;
	}
	return retVal;
    }
    
    /**
     * Writes a two byte integer value to a OutputStream.
     * 
     * @param val the integer value. It will lose the high-order two bytes.
     * @param os the OutputStream to write to
     * @throws IOException if an error occurs while writing to the OutputStream
     */
    public static final void writeInt(int val, OutputStream os) 
	throws IOException { 
	os.write(val>>>8);
	os.write(val);
    }
    
    /**
     * Writes a two byte integer value to a byte array.
     * 
     * @param val the integer value. It will lose the high-order two bytes.
     * @param dest the byte array to write to
     * @param pos the offset to start writing to
     */
    public static final int writeInt(int val, byte[] dest, int pos) {
	dest[pos++] = (byte)(val>>>8);
	dest[pos++] = (byte)val;
	return pos;
    }
    
    /**
     * Reads a two byte integer value from an InputStream.
     *
     * @param is the InputStream to read from
     * @returns the integer value
     * @throws IOException if some errors occurs while reading the integer
     * bytes.
     */
    public static final int readInt(InputStream is) throws IOException {
	return ( ((0xFF & is.read()) << 8)
		 | (0xFF & is.read()) );
    }
    
    /**
     * Reads a two byte integer value from a byte array.
     *
     * @param src the byte arra to read from
     * @param pos the offset to start reading from
     * @returns the integer value
     */
    public static final int readInt(byte[] src, int pos) {
	return ((0xFF & src[pos])<<8 | (0xFF & src[pos+1]));
    }

    /**
     * Blocks till the required number of bytes have been read from the
     * input stream.
     *
     * @param is the InputStream to read from
     * @param buffer the buffer to store the bytes into
     * @param throws EOFException if EOF is reached before all bytes are
     * read.
     * @throws IOException is an error occurs while reading
     */
    public static final void readFully(InputStream is, byte[] buffer)
	throws IOException {
	readFully(is, buffer, 0, buffer.length);
    }

    /**
     * Blocks till the required number of bytes have been read from the
     * input stream.
     *
     * @param is the InputStream to read from
     * @param buffer the buffer to store the bytes into
     * @param offset the offset to start storing at
     * @param len the number of bytes to read
     * @param throws EOFException if EOF is reached before all bytes are
     * read.
     * @throws IOException is an error occurs while reading
     */
    public static final void readFully(InputStream is, 
				       byte[] buffer, int offset, int len)
	throws IOException {
	int temp;
	while (len > 0) {
	    temp = is.read(buffer, offset, len);
	    if (temp == -1)
		throw new EOFException("Cannot read all " 
				       + len 
				       + " bytes needed to form this token!");
	    offset += temp;
	    len -= temp;
	}
    }

  // TBD: Consider moving these to a central class

  public static final void debug(String str) {
	System.err.print(str);
  }

  public static final  String getHexBytes(byte[] bytes) {
	return getHexBytes(bytes, 0, bytes.length);
  }

  public static final  String getHexBytes(byte[] bytes, int len) {
	return getHexBytes(bytes, 0, len);
  }
        
  public static final String getHexBytes(byte[] bytes, int pos, int len) {

    StringBuffer sb = new StringBuffer();
    for (int i = pos; i < (pos+len); i++) {
        
      int b1 = (bytes[i]>>4) & 0x0f;
      int b2 = bytes[i] & 0x0f;
        
      sb.append(Integer.toHexString(b1));
      sb.append(Integer.toHexString(b2));
      sb.append(' ');
    }
    return sb.toString();
  }
  
  /**
   * Returns a strign representing the token type.
   *
   * @param tokenId the token id for which a string name is desired
   * @return the String name of this token type
   */
  public static String getTokenName(int tokenId) {
    String retVal = null;
    switch (tokenId) {
    case AP_REQ_ID: 
    case AP_REP_ID:
      retVal = "Context Establishment Token";
      break;
    case MIC_ID:
      retVal = "MIC Token";
      break;
    case WRAP_ID:
      retVal = "Wrap Token";
      break;
    default:
      retVal = "Kerberos GSS-API Mechanism Token";
      break;
    }
    return retVal;
  }
}
