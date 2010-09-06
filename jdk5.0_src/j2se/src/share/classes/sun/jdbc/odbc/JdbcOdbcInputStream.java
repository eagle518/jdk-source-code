/*
 * @(#)JdbcOdbcInputStream.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcInputStream.java
//
// Description: Implementation of the java.io.InputStream class.  This
//              class is used for reading large amounts of data from
//              an input stream.  
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Karl Moss
//
// Date:        March, 1996
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.io.*;
import java.sql.*;

public class JdbcOdbcInputStream
	extends		InputStream {

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcInputStream (
		JdbcOdbc odbcapi,
		long hstmt,
		int col,
		short streamType,
		int sourceSqlType,
		Statement ownerStmt)
	{
		OdbcApi  = odbcapi;
		hStmt    = hstmt;
		column   = col;
		type 	 = streamType;
		invalid  = false;

		// Keep a reference to our owning statement object so that
		// the garbage collector will not finalize the statement before
		// we are done with it

		ownerStatement = ownerStmt;

		sqlType = Types.BINARY;
		
		switch (sourceSqlType) {
		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
		case JdbcOdbcTypes.NCHAR:
		case JdbcOdbcTypes.NVARCHAR:
		case JdbcOdbcTypes.NLONGVARCHAR:
			sqlType = Types.CHAR;
			break;
		}
		// Some sql types to stream types will need a data conversion.
		// Determine the conversion type and calculate the size of the
		// buffer needed.
		//
		// Example:
		//
		// Binary data (hex):          01 22 39
		//
		// Converted to ASCII (hex):   00 31 32 32 33 39
		//
		// Converted to UNICODE (hex): 00 00 00 31 00 32 00 32 00 33 00 39

		convertMultiplier = 1; 
		convertType = CONVERT_NONE;

		switch (type) {
		case ASCII:

			// Ascii input stream from binary data requires each hex
			// digit to be converted into 2 hex characters

			if (sqlType == Types.BINARY) {
				convertMultiplier = 2;
				convertType = CONVERT_ASCII;
			}
			break;

		case UNICODE:

			// Each hex digit must also be converted into 2 hex characters

			if (sqlType == Types.BINARY) {
				convertType = CONVERT_BOTH;
				convertMultiplier = 4;
			}
			else {
				// Each ascii character is 2 characters in unicode
				convertType = CONVERT_UNICODE;
				convertMultiplier = 2;
			}
			break;
		case CHARACTER:
		    convertType = CONVERT_NONE;
		    convertMultiplier = 1;
		    break;
		}

		// Create our read/data conversion buffer

		buf = new byte[MAX_BUF_LEN * convertMultiplier];

		bytesInBuf = 0;
		bufOffset = 0;
	}

	public JdbcOdbcInputStream (
		JdbcOdbc odbcapi,
		long hstmt,
		int col,
		byte byteArray[])
	{
		OdbcApi = odbcapi;
		hStmt   = hstmt;
		column  = col;
		type	= LOCAL;
		localByteArray = byteArray;
		localOffset = 0;
		invalid = false;
	}


	//--------------------------------------------------------------------
	// read
	// Reads a single byte of data.  Returns the byte read, or -1 when
	// end of stream is reached
	//--------------------------------------------------------------------
	public int read ()
		throws IOException
	{
		int  n;
		byte b[];

		// Allocate buffer
		b = new byte[1];

		// Read a single byte of data

		n = read (b);

		// If not eof, return the byte read
		if (n != -1) {
                        n = (b[0] & 0xff) ;
		}

		return n;
	}

	//--------------------------------------------------------------------
	// read
	// Reads data into an array of bytes.  Returns the number of bytes
	// read, or -1 when end of stream is reached
	//--------------------------------------------------------------------
	public int read (
		byte b[])
		throws IOException
	{
		return read(b, 0, b.length);
	}
	public byte[] readAllData () throws IOException
	{		
		int n = 0;
		byte[] retBuf;
		// If the input stream has been marked as invalid, raise
		// an exception.  This will occur if the input stream is
		// read from after a Statement has been closed, or the
		// cursor has moved.

		if (invalid) {
			throw new IOException (
				"InputStream is no longer valid - the Statement has been closed, or the cursor has been moved");
		}

		switch (type) {
		case LOCAL:

			// Calculate the number of bytes to read
			//n = b.length;

			if ((localOffset + n) > localByteArray.length) {
				n = localByteArray.length - localOffset;
			}
			retBuf = new byte[localByteArray.length];
			// If there is no more data to be read, return
			// a -1 to indicate end of file

			if (n == 0) {
				n = -1;
			}
			else {
				// Copy bytes to the receiving array
				System.arraycopy(localByteArray, localOffset, retBuf, localOffset, n);				
				localOffset += n; 
			}
			break;

		default:
			retBuf = readData();
			break;
		}

		// Return the number of bytes read
		return retBuf;
	}

	//--------------------------------------------------------------------
	// read
	// Reads data into an array of bytes at a given offset.
	// This method is not implemented.
	//--------------------------------------------------------------------
	public int read (
		byte b[],
		int off,
		int len)
		throws IOException
	{
		if ((off < 0) || (off > b.length) || (len < 0) ||
            ((off + len) > b.length) || ((off + len) < 0))
			throw new IndexOutOfBoundsException();
		if (len == 0) {
			return -1;
		}

		int n = 0;

		// If the input stream has been marked as invalid, raise
		// an exception.  This will occur if the input stream is
		// read from after a Statement has been closed, or the
		// cursor has moved.

		if (invalid) {
			throw new IOException (
				"InputStream is no longer valid - the Statement has been closed, or the cursor has been moved");
		}

		switch (type) {
		case LOCAL:

			// Calculate the number of bytes to read
			n = len;

			if ((localOffset + n) > localByteArray.length) {
				n = localByteArray.length - localOffset;
			}

			// If there is no more data to be read, return
			// a -1 to indicate end of file

			if (n == 0) {
				n = -1;
			}
			else {
				// Copy bytes to the receiving array

				for (int i = off; i < n; i++) {
					b[i] = localByteArray[localOffset + i];
				}
				localOffset += n; 
			}
			break;

		default:
			n = readData (b, off, len);
			break;
		}

		// Return the number of bytes read
		return n;
	}

    //--------------------------------------------------------------------
    // available
    // Returns the number of bytes that can be read without blocking.
    //
    // This operation is not implemented. If called it will throw an
    // exception.
    //--------------------------------------------------------------------

    public int available ()
        throws IOException
    {
        throw new IOException();
    }

	//--------------------------------------------------------------------
	// invalidate
	// Marks this input stream as invalid
	//--------------------------------------------------------------------
	public void invalidate ()
	{
		invalid = true;
	}
	
	public byte[] readData() throws IOException
	{
		byte[] retBuf = null;
		int bytesCopied = 0;
		//loop while there is more data to be read
		while (true)
		{
			// Read data from the data source
			bytesInBuf = readBinaryData (buf, MAX_BUF_LEN);
			// Convert the data
			bytesInBuf = convertData (buf, bytesInBuf);			
			if (bytesInBuf == -1)
				return retBuf;
			try
			{
				if (retBuf == null) //this is the first time
					retBuf = new byte[bytesInBuf];
				else
				{
					byte[] newBuf = new byte[bytesCopied+bytesInBuf];
					System.arraycopy(retBuf, 0, newBuf, 0, bytesCopied);
					retBuf = newBuf;
				}
			}//try
			catch (OutOfMemoryError oofe)
			{
				((JdbcOdbcStatement)ownerStatement).setWarning(
					new SQLWarning("Data has been truncated. "+oofe.getMessage()));
				return retBuf;
			}
			System.arraycopy(buf, 0, retBuf, bytesCopied, bytesInBuf);			
			bytesCopied += bytesInBuf;
		}//while
	}//readData
	//--------------------------------------------------------------------
	// readData
	// Read data and populate the given buffer.  For some types of column
	// sql types and input stream types, a direct read can be made.  For
	// others (such as UNICODE and ASCII data from a BINARY sql type), the
	// data must be read and converted.  For these types, the data will
	// be cached here.
	//--------------------------------------------------------------------

	protected int readData (
		byte b[], int offset, int length)
		throws IOException
	{
		int n = -1;
		int bytesCopied = offset;

		// Loop while there is more data and the buffer has not
		// been filled		

		while ((bytesInBuf != -1) &&
			   (bytesCopied - offset < length)) {

			// Need to read more data

			if (bufOffset >= bytesInBuf) {

				// Read data from the data source

				bytesInBuf = readBinaryData (buf, MAX_BUF_LEN);

				// Convert the data

				bytesInBuf = convertData (buf, bytesInBuf);
				bufOffset = 0;
			}
			else {
				b[bytesCopied] = buf[bufOffset];
				bytesCopied++;
				bufOffset++;
			}
		}		
		// Return the number of bytes in the buffer, unless no bytes
		// have been copied

		if (bytesCopied > offset) {
			n = bytesCopied;
		}
		
		return n;		
	}

	//--------------------------------------------------------------------
	// readBinaryData
	// Fill the given buffer with binary data.  Returns -1 when the
	// end of data has been reached.
	//--------------------------------------------------------------------

	protected int readBinaryData (
		byte b[],
		int len)
		throws IOException
	{
		int n = 0;

		try {


			n = OdbcApi.SQLGetDataBinary (hStmt, column,
								OdbcDef.SQL_C_BINARY, b, len);
		}
		catch (JdbcOdbcSQLWarning ex) {

		        // Data truncation.  Ignore the error and get the length

			Integer value = (Integer) ex.value;
			n = value.intValue ();
		}
		catch (SQLException ex) {
			throw new IOException (ex.getMessage ());
		}

		return n;
	}

	//--------------------------------------------------------------------
	// convertData
	// Given a buffer of bytes, convert to the appropriate data type
	//--------------------------------------------------------------------

	protected int convertData (
		byte b[],
		int len)
	{
		// No conversion required

		if (convertType == CONVERT_NONE) {
			return len;
		}

		String digits = "0123456789ABCDEF";

		if (len <= 0) {
			return len;
		}

		// We always need to convert the binary data into a character
		// representation.  We also may need to convert to unicode.  The
		// buffer is large enough to hold the converted data.
		//
		// Example:
		//
		// Binary data (hex):          01 22 39
		//
		// Converted to ASCII (hex):   00 31 32 32 33 39
		//
		// Converted to UNICODE (hex): 00 00 00 31 00 32 00 32 00 33 00 39

		for (int i = (len - 1); i >= 0; i--) {
			if (convertType == CONVERT_BOTH) {
				b[(i * 4) + 3] = (byte) digits.charAt (b[i] & 0x0F);
				b[(i * 4) + 2] = 0x00;
				b[(i * 4) + 1] = (byte) digits.charAt ((b[i] >> 4) & 0x0F);
				b[(i * 4)] = 0x00;
			}
			else if (convertType == CONVERT_ASCII) {
				b[(i * 2) + 1] = (byte) digits.charAt (b[i] & 0x0F);
				b[(i * 2)] = (byte) digits.charAt ((b[i] >> 4) & 0x0F);
			}
			else {
				b[(i * 2) + 1] = b[i];
				b[(i * 2)] = 0x00;

			}
		}

		return (len * convertMultiplier);
	}

	//====================================================================
	// Data attributes
	//====================================================================

	protected JdbcOdbc OdbcApi;			// ODBC API interface object

	protected long	hStmt;				// Statement handle

	protected int	column;				// Result set column number

	protected short	type;				// Stream type
		public final static short ASCII   = 1;
		public final static short UNICODE = 2;
		public final static short BINARY  = 3;
		public final static short LOCAL   = 4;
    public final static short CHARACTER = 5;

	protected byte localByteArray[];	// Another type of input
										// stream is simply an array
										// of bytes.

	protected int localOffset;			// Current offset of byte array

	protected boolean invalid;			// true if this input stream
										//  has been marked as invalid.
										//  If so, any attempts to
										//  read data will raise an
										//  exception

	protected boolean highRead;			// For unicode streams, set to
										//  true if the high byte has been read
										//  without reading the low byte

	protected int sqlType;				// SQL type of the underlying column

	protected byte buf[];				// For some types, we'll need to read
										// the data and convert it.

		public final static int MAX_BUF_LEN = 5120;

	protected int convertType;			// Type of conversion needed

		public final static int CONVERT_NONE	= 0;
		public final static int CONVERT_UNICODE	= 1;
		public final static int CONVERT_ASCII	= 2;
		public final static int CONVERT_BOTH	= 3;

	protected int convertMultiplier;	// Number of bytes needed to convert
										//  a single byte

	protected int bytesInBuf;			// Number of bytes in the buffer
	protected int bufOffset;			// Current offset in buffer

	protected Statement ownerStatement;	// Owning statement object
}
