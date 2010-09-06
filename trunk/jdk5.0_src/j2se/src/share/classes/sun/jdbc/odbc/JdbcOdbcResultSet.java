/*
 * @(#)JdbcOdbcResultSet.java	1.60 04/05/05
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcResultSet.java
//
// Description: Impementation of the ResultSet interface class
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

import java.math.BigDecimal;
import java.sql.*;
import java.util.Calendar;
import java.util.Map;
import java.io.Reader;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.StringTokenizer;

public class JdbcOdbcResultSet
	extends		JdbcOdbcObject
	implements	JdbcOdbcResultSetInterface
{

	//====================================================================
	// Public methods
	//====================================================================

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcResultSet ()
	{
		OdbcApi = null;
		hDbc    = OdbcDef.SQL_NULL_HDBC;
		hStmt   = OdbcDef.SQL_NULL_HSTMT;
		lastWarning = null;
		keepHSTMT = false;
		numResultCols = -1;
		lastColumnNull = false;
	}

	//--------------------------------------------------------------------
	// finalize
	// Perform any cleanup when this object is garbage collected
	//--------------------------------------------------------------------

	protected void finalize ()
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("ResultSet.finalize " + this);
		}

		try {
			// when the Resultset gets garbage collected
			// the close() method should not attempt close or drop
			// the Statement handle, since it may be invalid.
			
			if (!closed)
			{
			    hStmt   = OdbcDef.SQL_NULL_HSTMT;
			    close ();
			}

		}
		catch (SQLException ex) {
			// If an exception is thrown, ignore it
		}
	}

	//--------------------------------------------------------------------
	// initialize
	// Initialize the result set object.  Give the ODBC API interface
	// object, the connection handle, and the statement handle.  The
	// keep flag is used to determine if the statement handle should
	// actually be dropped when closing the ResultSet
	//--------------------------------------------------------------------

	public void initialize (
		JdbcOdbc odbcApi,
		long hdbc,
		long hstmt,
		boolean keep,
		JdbcOdbcStatement ownerStmt)
		throws SQLException
	{
		OdbcApi = odbcApi;
		hDbc = hdbc;
		hStmt = hstmt;
		keepHSTMT = keep;

		// Now allocate an array of bound column objects.  Each
		// column in the result set will have a placeholder to store
		// information as needed (i.e. InputStreams)

		numberOfCols = getColumnCount ();

		boundCols = new JdbcOdbcBoundCol[numberOfCols];
		int colType;

		for (int i = 0; i < numberOfCols; i++) {
			boundCols[i] = new JdbcOdbcBoundCol ();
		}

		// Keep a reference of our owning statement object

		ownerStatement = ownerStmt;
		
		// initialize cursor positions.
		rowPosition	    = 0; 
		lastForwardRecord   = 0;
		lastRowPosition	    = 0;
		lastColumnData	    = 0; 
		currentBlockCell    = 0;
		blockCursor	    = false;
 		rowSet		    = 1; // ODBC default.

		if (getType () != ResultSet.TYPE_FORWARD_ONLY)
		{
			
			if (ownerStatement != null)
			{
			    // get user's odbcRowSetSize property value.
			    // if this property was not set, then we will
			    // get the JdbcOdbcLimits.DEFAULT_ROW_SET.
			    rowSet = ownerStatement.getBlockCursorSize ();
			}
			
			// 4672508
			setRowStatusPtr();
			
			// Fix for 4628693, 4668340
			setResultSetVisibilityIndicators();
			
			// Get row count if it is scrollable cursor
			calculateRowCount ();

			if (numberOfRows >= 0)
			{
			    boolean rowSizeIsSet = false;

			    // if block-cursor is desire
			    // try to set a row Array Size > 1.
 			    rowSizeIsSet = setRowArraySize();
 
			    // if block-cursor is not supported
			    // initialize staging Area w/ odbc defaults.
 			    if (!rowSizeIsSet)
 				rowSet = 1;
 			
 			    // 4672508
			    if(pA != null){
				if(pA[0] != 0){
				    OdbcApi.ReleaseStoredIntegers(pA[0], pA[1]);
				    pA[0] = 0;
				    pA[1] = 0;
				}
			    }
 			
			    // even if rowSet = 1, we need an status
			    // array to monitor row updates.
			    setRowStatusPtr();
			    
			    if (rowSet > 1)
			    {
				blockCursor = true;
				setCursorType();
			    }

			    // initialize staging Area with
			    // COLUMN_IGNORE state.
			    for (int i = 0; i < numberOfCols; i++)
			    {
				boundCols[i].initStagingArea(rowSet);
			    }
			}
		}
	}

	//--------------------------------------------------------------------
	// wasNull
	// A column may have the value of SQL NULL; wasNull reports whether
	// the last column read had this special value.

	public boolean wasNull ()
		throws SQLException
	{
		return lastColumnNull;
	}


        //--------------------------------------------------------------------
        // setAliasColumnName // sun's 4234318 fix.
        // Given a column number and column name to be used, set the column alias
        // name for the corresponding column number. If the column number is
        // out-of-range, nothing is set and original column name will be used and
        // not renamed.
        //--------------------------------------------------------------------

        public void setAliasColumnName (
                        String AliasColumnName,
                        int column)
        {
                if ((column > 0) && (column <= numberOfCols)) {
                        boundCols[column-1].setAliasName (AliasColumnName);
                }
        }


        //--------------------------------------------------------------------
        // mapColumnName // sun's 4234318 fix.
        // Given a column name, map it to the corresponding column rename in the
        // result set. If no column rename exist, return the original column name.
        // If the column number is out-of-range, return original column name.
        //--------------------------------------------------------------------

        public String mapColumnName (
                        String columnName,
                        int column)
        {
                if ((column > 0) && (column <= numberOfCols)) {
                        return boundCols[column - 1].mapAliasName (columnName);
                }
                else {
                        return columnName;
                }
        }


	//--------------------------------------------------------------------
	// getString
	//--------------------------------------------------------------------

	public String getString (
		int column)
		throws SQLException
	{
		checkOpen ();
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);
		
		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}
		
		int maxLen = getMaxCharLen (column);
		int columnLen = getColumnLength(column);

		String value = null;
		if (columnLen > JdbcOdbcLimits.MAX_GET_DATA_LENGTH) {

		    //we should go for streams now
		    JdbcOdbcInputStream jois = 
			(JdbcOdbcInputStream)getAsciiStream(column);

			try {
			
			    byte[] outBuf = jois.readAllData();
			    //fix for Sun Bug id 4330997

			    // SQLServer and other ODBC drives will
			    // return data in unicode columns as sqltypes
			    // -8 -> -10 (SQL_WCHAR types, new in ODBC
			    // 3.5). Since JDBC does not have those 
			    // types, we need to look at the data that
			    // the driver send back to see if it is 
			    // LITTLE_ENDIAN unicode data and if it is
			    // convert it appropriately.

			    if (outBuf != null) {
				if ((outBuf.length > 2) && 
				    (outBuf[1] == 0)) {
				    value = BytesToChars("UnicodeLittleUnmarked", outBuf);
				} else if ((outBuf.length >= 2) &&
				    (outBuf[0] == 0)) {
				    value = BytesToChars("UnicodeBigUnmarked", outBuf);
			    	} else {
			            value = BytesToChars(OdbcApi.charSet, outBuf);
				}
			    } else {
			    	lastColumnNull = true;
			    }
				
		    } catch (Exception e) {
			SQLException se = new SQLException(e.getMessage());
			se.fillInStackTrace();
			throw se;
		    }
			/************************************/
		}
		else
		{

			// If the column is null, return null

			if (maxLen == OdbcDef.SQL_NULL_DATA) {
			    lastColumnNull = true;
				return null;
			}

			// Some data types should not have spaces trimmed

			boolean trimSpaces = true;
	
			int colType = getColumnType (column);
			switch (colType)
			{
				case Types.CHAR:
				case Types.VARCHAR:
				case Types.LONGVARCHAR:
					trimSpaces = false;
					break;
				default:
					break;
			}

			// Allow room for a null terminator

			maxLen++;

			// Get the column, trimming spaces if necessary
			value = getDataString (column, maxLen, trimSpaces);

			if (value == null) {
			    lastColumnNull = true;
				return value;
			}

			// If we are not trimming spaces from the String, we need to return
			// the proper length (i.e. no null terminator)

			int valueLen = value.length ();

			if ((valueLen == (maxLen - 1)) && (!trimSpaces)) {
				value = value.substring (0, maxLen - 1);
			}

			// If we read in our entire buffer for LONG data, there may be
			// more to read.

			if (((colType == Types.LONGVARCHAR) ||
				(colType == Types.LONGVARBINARY)) &&
				(valueLen == (maxLen - 1))) {

				String value2 = value;

				// Loop while we have filled the read buffer

				while (value2.length () == JdbcOdbcLimits.MAX_GET_DATA_LENGTH) {
					value2 = getDataString (column, maxLen, trimSpaces);
					if (value2 != null) {
						if (OdbcApi.getTracer().isTracing ()) {
							OdbcApi.getTracer().trace ("" + value2.length () + " byte(s) read");
						}

						// Get rid of the null terminator
			                                        if (value2.length () == maxLen) {
							value2 = value2.substring (0, maxLen - 1);
						}
						value += value2;

						if (OdbcApi.getTracer().isTracing ()) {
							OdbcApi.getTracer().trace ("" + value.length () + " bytes total");
						}
					}
					else {
						break;
					}
				}
			}
		}//else normal column
		return value;
	}

	public String getString (
		String columnName)
		throws SQLException
	{
		return getString (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getBoolean
	//--------------------------------------------------------------------

	public boolean getBoolean (
		int column)
		throws SQLException
	{
		checkOpen ();
		boolean returnValue = false;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {

			// Get the value as an int and convert

			returnValue = (getInt (column) != 0);
		}
		else {
			lastColumnNull = true;
		}
		return returnValue;
	}

	public boolean getBoolean (
		String columnName)
		throws SQLException
	{
		return getBoolean (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getByte
	//--------------------------------------------------------------------

	public byte getByte (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		byte returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {

			// Get the value as an int and convert

			returnValue = (byte) (getInt (column));
		}
		else {
			lastColumnNull = true;
		}
		return returnValue;
	}

	public byte getByte (
		String columnName)
		throws SQLException
	{
		return getByte (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getShort
	//--------------------------------------------------------------------

	public short getShort (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		short returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {

			// Get the value as an int and convert

			returnValue = (short) (getInt (column));
		}
		else {
			lastColumnNull = true;
		}

		return returnValue;
	}

	public short getShort (
		String columnName)
		throws SQLException
	{
		return getShort (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getInt
	//--------------------------------------------------------------------

	public int getInt (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		int returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);
		    
		// If a pseudo column does not exist, get the data from the
		// column
		if (getPseudoCol (column) == null) {
			Integer value = getDataInteger (column);

			// If the column was not null, get the value from the
			// Integer object
			if (value != null) {
				returnValue = value.intValue ();
			}
		}
		else {
			lastColumnNull = true;
		}
		return returnValue;
	}

	public int getInt (
		String columnName)
		throws SQLException
	{
		return getInt (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getLong
	//--------------------------------------------------------------------

	public long getLong (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		long returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {
			Double value = getDataDouble (column);

			// If the column was not null, get the value from the
			// Double object
			if (value != null) {
				returnValue = value.longValue ();
			}
		}
		else {
			lastColumnNull = true;
		}
		return returnValue;
	}

	public long getLong (
		String columnName)
		throws SQLException
	{
		return getLong (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getFloat
	//--------------------------------------------------------------------

	public float getFloat (
		int column)
		throws SQLException
	{
		// Fix 4532167. Now uses getDouble to retrive maximum value of java float.	
		return (float) getDouble(column);
		/*
		checkOpen ();
		
		float returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {
			Float value = getDataFloat (column);

			// If the column was not null, get the value from the
			// Float object
			if (value != null) {
				returnValue = value.floatValue ();
			}
		}
		else {
			lastColumnNull = true;
		}

		return returnValue;
		*/
	}

	public float getFloat (
		String columnName)
		throws SQLException
	{
		return getFloat (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getDouble
	//--------------------------------------------------------------------

	public double getDouble (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		double returnValue = 0;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {
			Double value = getDataDouble (column);

			// If the column was not null, get the value from the
			// Double object
			if (value != null) {
				returnValue = value.doubleValue ();
			}
		}
		else {
			lastColumnNull = true;
		}
		return returnValue;
	}

	public double getDouble (
		String columnName)
		throws SQLException
	{
		return getDouble (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getBigDecimal
	//--------------------------------------------------------------------

	public BigDecimal getBigDecimal ( 
		int column,
		int scale)
		throws SQLException
	{
		checkOpen ();
		
		BigDecimal n = null;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column does not exist, get the data from the
		// column

		if (getPseudoCol (column) == null) {

			// Get the column as a String, so we don't lose any precision

			String value = getDataString (column, 300, true);

			// If the column was not null, get the value from the
			// String object

			if (value != null) {
				n = new BigDecimal(value);
				n = n.setScale(scale, BigDecimal.ROUND_HALF_EVEN);
			    }
		}
		else {
			lastColumnNull = true;
		}

		return n;
	}

	public BigDecimal getBigDecimal ( 
		String columnName,
		int scale)
		throws SQLException
	{
		return getBigDecimal (findColumn (columnName), scale);
	}

	//--------------------------------------------------------------------
	// getBytes
	//--------------------------------------------------------------------

	public synchronized byte[] getBytes (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}

		int maxLen = getMaxBinaryLen (column);

		// If the column is null, return null

		if (maxLen == OdbcDef.SQL_NULL_DATA) {
		    lastColumnNull = true;
			return null;
		}

		int columnLen = getColumnLength(column);		
		if (columnLen > JdbcOdbcLimits.MAX_GET_DATA_LENGTH)
		{					
			//we should go for streams now
			JdbcOdbcInputStream jois = (JdbcOdbcInputStream)getBinaryStream(column);
			try
			{
				return jois.readAllData();				
			}
			catch (Exception e)
			{
				throw new SQLException(e.getMessage());
			}
		}//if 
		else
		{
			// Get the column type
			int colType = getColumnType (column);
			// Create new byte array to read data into
			byte b[] = new byte[maxLen];
			// Read the binary data.  The number of bytes read will be
			// returned, -1 for NULL
			int numBytes;
			try
			{
				numBytes = OdbcApi.SQLGetDataBinary (hStmt, column, b);
			}
			catch (JdbcOdbcSQLWarning warn)
			{
				// Data truncation.  Ignore the error and get the length
				Integer value = (Integer) warn.value;
				numBytes = value.intValue ();
			}		
			// If we attempted to read a null column, return a null
			if (numBytes == OdbcDef.SQL_NULL_DATA)
			{
				lastColumnNull = true;
				b = null;
			}
			// If we read less than the maximum number of bytes, create a
			// new byte array of the proper size.  Only do this if the column
			// is not a BINARY type, which is fixed length.			
			if ((colType != Types.BINARY) &&
				(numBytes != maxLen)  && 
				(b != null))
			{
				byte b2[] = new byte[numBytes];
				System.arraycopy (b, 0, b2, 0, numBytes);
				return b2;
			}					
			// We read all of the requested buffer, and we knew
			// the actual size.  Return the buffer.
			return b;
		}//else
	}//getBytes

	public byte[] getBytes (
		String columnName)
		throws SQLException
	{
		return getBytes (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getDate
	// Get the date as a string, and convert to a Date object
	//--------------------------------------------------------------------

	public java.sql.Date getDate (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}

		String s = getDataStringDate (column);

		// If the column is null, return a null object

		if (s == null) {
			return null;
		}

		// Now convert to a Date object

		return Date.valueOf (s);
	}

	public java.sql.Date getDate (
		String columnName)
		throws SQLException
	{
		return getDate (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getTime
	// Get the time as a string, and convert to a Time object
	//--------------------------------------------------------------------

	public java.sql.Time getTime (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}

		String s = getDataStringTime (column);

		// If the column is null, return a null object

		if (s == null) {
			return null;
		}

		// Now convert to a Time object

		return Time.valueOf (s);
	}

	public java.sql.Time getTime (
		String columnName)
		throws SQLException
	{
		return getTime (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getTimestamp
	// Get the timestamp as a string, and convert to a Timestamp object
	//--------------------------------------------------------------------

	public java.sql.Timestamp getTimestamp (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}

		String s = getDataStringTimestamp (column);

		// If the column is null, return a null object

		if (s == null) {
			return null;
		}

		// If all we got was a date, put on default seconds

		if (s.length () == 10) {
			s += " 00:00:00";
		}

		// Now convert to a Timestamp object

		return Timestamp.valueOf (s);
	}

	public java.sql.Timestamp getTimestamp (
		String columnName)
		throws SQLException
	{
		return getTimestamp (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// The normal getString and getBytes methods are suitable for
	// reading normal sized data.  However occasionally it may be 
	// necessary to access LONGVARCHAR or LONGVARBINARY fields that
	// are multiple Megabytes in size.  To support this case we provide
	// getCharStream and getBinaryStream methods that return Java
	// IO streams from which data can be read in chunks.
	//--------------------------------------------------------------------

	public java.io.InputStream getAsciiStream (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// We need to know the source column data type

		int colType = getColumnType (column);
		int dataType = Types.BINARY;
		
		switch (colType) {
		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
		case JdbcOdbcTypes.NCHAR:
		case JdbcOdbcTypes.NVARCHAR:
		case JdbcOdbcTypes.NLONGVARCHAR:
			dataType = Types.CHAR;
			break;
		}

		JdbcOdbcInputStream inStream =  new JdbcOdbcInputStream (
			OdbcApi, hStmt, column,	JdbcOdbcInputStream.ASCII, dataType,
			ownerStatement);

		setInputStream (column, inStream);

		return inStream;
	}

	public java.io.InputStream getAsciiStream (
		String columnName)
		throws SQLException
	{
		return getAsciiStream (findColumn (columnName));
	}

	public java.io.InputStream getUnicodeStream (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// We need to know the source column data type

		int colType = getColumnType (column);
		int dataType = Types.BINARY;
		
		switch (colType) {
		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
		case JdbcOdbcTypes.NCHAR:
		case JdbcOdbcTypes.NVARCHAR:
		case JdbcOdbcTypes.NLONGVARCHAR:
			dataType = Types.CHAR;
			break;
		}

		JdbcOdbcInputStream inStream =  new JdbcOdbcInputStream (
			OdbcApi, hStmt, column,	JdbcOdbcInputStream.UNICODE, dataType,
			ownerStatement);
		setInputStream (column, inStream);

		return inStream;
	}

	public java.io.InputStream getUnicodeStream (
		String columnName)
		throws SQLException
	{
		return getUnicodeStream (findColumn (columnName));
	}

	public java.io.InputStream getBinaryStream (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// We need to know the source column data type

		int colType = getColumnType (column);
		int dataType = Types.BINARY;
		
		switch (colType) {
		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
		case JdbcOdbcTypes.NCHAR:
		case JdbcOdbcTypes.NVARCHAR:
		case JdbcOdbcTypes.NLONGVARCHAR:
			dataType = Types.CHAR;
			break;
		}

		JdbcOdbcInputStream inStream =  new JdbcOdbcInputStream (
			OdbcApi, hStmt, column,	JdbcOdbcInputStream.BINARY, dataType,
			ownerStatement);
		setInputStream (column, inStream);

		return inStream;
	}

	public java.io.InputStream getBinaryStream (
		String columnName)
		throws SQLException
	{
		return getBinaryStream (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// next
	// next moves us to the next row of the results.
	// It returns true if if has moved to a valid row, false if
	// all the rows have been processed.
	//--------------------------------------------------------------------

	public boolean next ()
		throws SQLException
	{

		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {
					
			boolean done = false;
			boolean fetchBlock = false;

			if (getFetchDirection () == ResultSet.FETCH_FORWARD)
			{

				if (rowPosition == numberOfRows)
				{
				    afterLast();
				    return false;
				}

				if (blockCursor)
				{
				    done = relative(1, false);
				}
				else
				{
				    rowPosition++;
				    done = fetchScrollOption (rowPosition, OdbcDef.SQL_FETCH_ABSOLUTE);
				}
			}
			else
			{
				if (rowPosition == 1)
				{
				    beforeFirst();
				    return false;
				}

				if (blockCursor)
				{
				    done = relative(-1, false);
				}
				else
				{
				    rowPosition--;
				    done = fetchScrollOption (rowPosition, OdbcDef.SQL_FETCH_ABSOLUTE);
				}

			}
			return done;

		} 
		

 		boolean rc = true;
		lastColumnNull = false;

		// Close any/all input streams for the statement
		
		closeInputStreams ();

		// Reset last warning message

		clearWarnings ();

		try {
			rc = OdbcApi.SQLFetch (hStmt);
		}
		catch (SQLWarning ex) {

			// If we got a warning, set the last warning for
			// the result set.  We will assume that the fetch
			// positioned us to a vaild row.

			setWarning (ex);
		}
		// next increases rowPosition as long 
		// as there are rows or mark the last.		
		if (rc == true)
			rowPosition++;
		else if (rc == false)
		{
		    if (lastForwardRecord == 0)
		    {
			lastForwardRecord = rowPosition;
			rowPosition = 0;
		    }
		    else
			rowPosition = 0;
		}

		return rc;
	}

	//--------------------------------------------------------------------
	// getRowNumber
	// Return the number of the current row.  The first row containing
	// actual data is row 1.
	//--------------------------------------------------------------------

	public int getRowNumber ()
		throws SQLException
	{
		checkOpen ();
		
		int	rowNum = 0;

		// Reset last warning message

		clearWarnings ();

		try {
			rowNum = (int)OdbcApi.SQLGetStmtOption (hStmt,
					OdbcDef.SQL_ROW_NUMBER);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

		        BigDecimal n = (BigDecimal) ex.value;
			rowNum = n.intValue ();

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		return rowNum;
	}


	//--------------------------------------------------------------------
	// getColumnCount
	// Return the number of columns in the ResultSet
	//--------------------------------------------------------------------

	public int getColumnCount ()
		throws SQLException
	{
		checkOpen ();
		
		int	numCols = 0;

		// Reset last warning message

		clearWarnings ();

		// If pseudo columns exist, return the last pseudo
		// column

		if (lastPseudoCol > 0) {
			return lastPseudoCol;
		}

		// If column mappings exist, return the number of columns
		// that are being mapped

		if (colMappings != null) {
			return colMappings.length;
		}

		try {
			// If we haven't queried for the number of result
			// columns in the result set yet, do it now and
			// cache the result

		    // We also want to do this if there is more than 1
		    // result set, so don't check for numResultCols = -1
		    // sun bugID # 4400352

		    // if (numResultCols == -1) {
				numResultCols = OdbcApi.SQLNumResultCols (
						hStmt);
		    // }
			numCols = numResultCols;
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			numCols = n.intValue ();

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		return numCols;
	}

	//--------------------------------------------------------------------
	// getRowCount
	// The total number of rows returned by the query.  Note that on some
	// databases this method may be very expensive.
	//--------------------------------------------------------------------

	public int getRowCount ()
		throws SQLException
	{
		checkOpen ();
		return numberOfRows;
	}

	//--------------------------------------------------------------------
	// close
	// close frees up internal state associated with the ResultSet
	// You should normally call close when you are done with a ResultSet
	// ResultSets are also closed automatically when their Statement is
	// closed.
	//--------------------------------------------------------------------

	public synchronized void close()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSet.close");
		}
		// Close any/all input streams for the statement
		
		closeInputStreams ();


		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		if ((OdbcApi != null) &&
		    (hStmt != OdbcDef.SQL_NULL_HSTMT)) {
			// If we are keeping the hStmt, simply close it

			if (keepHSTMT) {
				/*************************				 
				if (!freed) {
					OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_CLOSE);
					freed = true;
				}
				*************************/
			}
			else {
				// Not keeping the hStmt, drop it
				OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);
				hStmt = OdbcDef.SQL_NULL_HSTMT;
			}

			// Turn on flag indicating that the result set is now closed

			closed = true;

			// clean up Column pointers.
			FreeCols();
			
			//4486684
			if(pA != null)
			{
				if(pA[0] != 0)
				{
					OdbcApi.ReleaseStoredIntegers(pA[0], pA[1]);
					pA[0] = 0;
					pA[1] = 0;
				}
			}
			
			if (ownerStatement != null)
			{
				ownerStatement.myResultSet = null;
			}
			if (OdbcApi.getTracer().isTracing ())
				OdbcApi.getTracer().trace ("*ResultSet has been closed");
		}
		//else
		    //closed = true;

	}

	//--------------------------------------------------------------------
	// freeCols
	// free any unused bound resources.
	//--------------------------------------------------------------------

	public synchronized void FreeCols()
        throws NullPointerException
	{
	    try
	    {
		 for (int pindex=0; pindex < boundCols.length; pindex++)
		{
			if (boundCols[pindex].pA1!=0)
			{
			    OdbcApi.ReleaseStoredBytes (boundCols[pindex].pA1, boundCols[pindex].pA2);//4486195
			    boundCols[pindex].pA1 = 0;
			    boundCols[pindex].pA2 = 0;
			}

			if (boundCols[pindex].pB1!=0)
			{
			    OdbcApi.ReleaseStoredBytes (boundCols[pindex].pB1, boundCols[pindex].pB2);//4486195
			    boundCols[pindex].pB1 = 0;
			    boundCols[pindex].pB2 = 0;
			}

			if (boundCols[pindex].pC1!=0)
			{
			    // Fix 4531124. pC1 will be used only from bindColBinary.
			    // Changing this to free Integers.
			    //4691886
			    OdbcApi.ReleaseStoredBytes (boundCols[pindex].pC1, boundCols[pindex].pC2);//4486195
			    boundCols[pindex].pC1 = 0;
			    boundCols[pindex].pC2 = 0;
			}

			if (boundCols[pindex].pS1!=0)
			{
			    OdbcApi.ReleaseStoredChars (boundCols[pindex].pS1, boundCols[pindex].pS2);//4486195
			    boundCols[pindex].pS1 = 0;
			    boundCols[pindex].pS2 = 0;
			}
		}
	    }
	    catch (NullPointerException npx)
	    {
		//Do nothing
	    }	
	} 


	//--------------------------------------------------------------------
	// getCursorName
	// Returns a cursor name that can be used to identify the
	// currnet position in the Resultset to a separate statement that is
	// executing a positioned update or positioned delete.  If the database
	// doesn't support positioned delete or update, it may return "" here.
	//--------------------------------------------------------------------

	public String getCursorName ()
		throws SQLException
	{
		checkOpen ();
		String	value = "";

		// Reset last warning message

		clearWarnings ();

		try {
			value = OdbcApi.SQLGetCursorName (hStmt);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			value = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		return value.trim ();
	}

	//--------------------------------------------------------------------
	// getMetaData
	// You can obtain a ResultMetaData object to obtain information
	// about the number, types and proerties of the result columns.
	//--------------------------------------------------------------------

	public ResultSetMetaData getMetaData ()
		throws SQLException
	{
		checkOpen ();
		
		if (OdbcApi.getTracer().isTracing()) {
			OdbcApi.getTracer().trace ("*ResultSet.getMetaData");
		}

		// If the result set has been closed, throw an exception

		if (closed) {
			throw new SQLException("ResultSet is closed");
		}

		return new JdbcOdbcResultSetMetaData (OdbcApi, this);
	}

	//--------------------------------------------------------------------
	// getObject
	// You can obtain a column result as a Java Object.
	// See the JDBC spec's "Dynamic Programming" chapter for details.
	// All of the getData methods above use the getObject interface.
	//--------------------------------------------------------------------

	public Object getObject (
		int column)
		throws SQLException
	{
		checkOpen ();
		
		Object	value = null;

		// Get the SQL type of the column

		int sqlType = getColumnType (column);

		// Save the original column number in case it is remapped

		int origColumn = column;
		
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		column = mapColumn (column);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(column);

		// If a pseudo column exists, return a null

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			return null;
		}

		// For each SQL type, call the appropriate routine and
		// convert to the proper object type

		switch (sqlType) {

		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
			value = (Object) getString (column);
			break;

		case Types.NUMERIC:
		case Types.DECIMAL:
			value = (Object) getBigDecimal (column, getScale (origColumn));
			break;

		case Types.BIT:
			value = (Object) new Boolean (getBoolean (column));
			break;

		case Types.TINYINT:
		case Types.SMALLINT:
		case Types.INTEGER:
			value = (Object) new Integer (getInt (column));
			break;

		case Types.BIGINT:
			value = (Object) new Long (getLong (column));
			break;

		case Types.REAL:
			value = (Object) new Float (getFloat (column));
			break;

		case Types.FLOAT:
		case Types.DOUBLE:
			value = (Object) new Double (getDouble (column));
			break;
			
		case Types.BINARY:
		case Types.VARBINARY:
		case Types.LONGVARBINARY:
			value = (Object) getBytes (column);
			break;

		case Types.DATE:
			value = (Object) getDate (column);
			break;

		case Types.TIME:
			value = (Object) getTime (column);
			break;

		case Types.TIMESTAMP:
			value = (Object) getTimestamp (column);
			break;

		}

		// If the column was null, always return a null object

		if (wasNull ()) {
			value = null;
		}


		return value;
	}

	public Object getObject (
		String columnName)
		throws SQLException
	{
		return getObject (findColumn (columnName));
	}

	//--------------------------------------------------------------------
	// getWarnings
	// Returns the first warning reported by calls on this ResultSet
	//--------------------------------------------------------------------
	public SQLWarning getWarnings ()
		throws SQLException
	{
		checkOpen ();
		return lastWarning;
	}

	//--------------------------------------------------------------------
	// clearWarnings
	// Clears any warning information for the statement
	//--------------------------------------------------------------------

	public void clearWarnings()
			throws SQLException
	{
		checkOpen ();
		lastWarning = null;
	}

	//--------------------------------------------------------------------
	// setWarning
	// Sets the warning 
	//--------------------------------------------------------------------

	public void setWarning (
			SQLWarning ex)
			throws SQLException
	{
		checkOpen ();
		lastWarning = ex;
	}

	//--------------------------------------------------------------------
	// getHSTMT
	// Returns the statement handle for the result set
	//--------------------------------------------------------------------
	
	public long getHSTMT ()
	{
		return hStmt;
	}

	//--------------------------------------------------------------------
	// findColumn
	// Map a ResultSet column name to a ResultSet column index
	//--------------------------------------------------------------------
	public synchronized int findColumn (
		String columnName)
		throws SQLException
	{
		// Get the MetaData object if we don't already have one

		if (rsmd == null) {
			rsmd = getMetaData ();
			colNameToNum = new java.util.Hashtable ();
			colNumToName = new java.util.Hashtable ();
		}

		// Search for the mapping in the hash table
		
		Integer col = (Integer) colNameToNum.get (columnName);

		// If it was not found, try to get the name from the
		// MetaData.  If it is not found here, raise an exception

		if (col == null) {

			String name;

			for (int i = 1; i <= rsmd.getColumnCount(); i++) {

				// As an optimization, keep a separate hash
				// table mapping column numbers to names.  If
				// we don't find the column number in the hash
				// table, then we have to hit the data source
				// to get the name.  Otherwise, get the name
				// from the hash table

				name = (String) colNumToName.get (
						new Integer (i));

				if (name == null) {
					name = rsmd.getColumnName (i);
					// Add it to our hash tables
					colNameToNum.put (name,
						new Integer (i));
					colNumToName.put (new Integer (i),
						name);
				}

				// If the name matches, return the column
				// pointer.

				if (name.equalsIgnoreCase (columnName)) {
					return i;
				}
			}

			// The column name was not found

			throw new SQLException ("Column not found",
				"S0022");
		}
		return col.intValue ();
	}


	// New JDBC 2.0 APIs

	public Reader getCharacterStream (
		int columnIndex)
		throws SQLException
	{ 		// Reset last warning message
		checkOpen ();

		// here is the Reader we want to return.
		InputStreamReader inReader = null;

		clearWarnings ();
		lastColumnNull = false;

		// Re-map column if necessary

		columnIndex = mapColumn (columnIndex);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(columnIndex);

		// We need to know the source column data type

		int colType = getColumnType (columnIndex);
		int dataType = Types.BINARY;
		
		switch (colType) {
		case Types.CHAR:
		case Types.VARCHAR:
		case Types.LONGVARCHAR:
	   	case JdbcOdbcTypes.NCHAR:
	   	case JdbcOdbcTypes.NVARCHAR:
	   	case JdbcOdbcTypes.NLONGVARCHAR:
			dataType = Types.CHAR;
			break;
		}
		
		String encoding = OdbcApi.charSet;

		JdbcOdbcInputStream inStream =  new JdbcOdbcInputStream (
			OdbcApi, hStmt, columnIndex, JdbcOdbcInputStream.CHARACTER, dataType,
			ownerStatement);
		
		setInputStream (columnIndex, inStream);
		
		try
		{
		    inReader = new InputStreamReader(inStream, encoding);
		}
		catch(java.io.UnsupportedEncodingException encExc)
		{
		    throw new SQLException("getCharacterStream() with Encoding (" + "\'encoding\'" + ") :" + encExc.getMessage() );
		}
		return inReader;
	}

	public Reader getCharacterStream (
		String columnName)
		throws SQLException
	{   	
		int columnIndex = findColumn (columnName);

		return getCharacterStream(columnIndex);
	}

	public BigDecimal getBigDecimal (
		int columnIndex)
		throws SQLException
	{
		checkOpen ();
		BigDecimal n = null;

		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		// Re-map columnIndex if necessary

		columnIndex = mapColumn (columnIndex);

		// Refetch data on a consecutive call 
		// to the same column or after an update.
		consecutiveFetch(columnIndex);

		// If a pseudo columnIndex does not exist, get the data from the
		// columnIndex

		if (getPseudoCol (columnIndex) == null) {

			// Get the columnIndex as a String, so we don't lose any precision

			String value = getDataString (columnIndex, 300, true);

			// If the columnIndex was not null, get the value from the
			// String object

			if (value != null) 
			{
				n = new BigDecimal(value);
			}
		}
		else {
			lastColumnNull = true;
		}

		return n; 
	}

	public BigDecimal getBigDecimal (
		String columnName)
		throws SQLException
	{	
		int columnIndex = findColumn (columnName);

		return getBigDecimal(columnIndex);
	}


	//--------------------------------------------------------------------
	// isBeforeFirst
	// Is cursor at before the first row?
	//--------------------------------------------------------------------

	public boolean isBeforeFirst ()
		throws SQLException
	{

		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if ( numberOfRows > 0 )
		    {
			    return (rowPosition == 0);
		    }
		    else 
			    return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}

	//--------------------------------------------------------------------
	// isAfterLast
	// Is cursor at after the last row?
	//--------------------------------------------------------------------

	public boolean isAfterLast ()
		throws SQLException
	{
		checkOpen ();

		if (closed) {
			throw new SQLException("ResultSet is closed");
		}

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if (numberOfRows > 0)
		    {
			    return (rowPosition > numberOfRows);
		    }
		    else
		    {
			    return false;
		    }
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}
	}

	//--------------------------------------------------------------------
	// isFirst
	// Is cursor on the first row?
	//--------------------------------------------------------------------

	public boolean isFirst ()
                throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {
			
		    if (numberOfRows > 0)
		    {
			return (rowPosition == 1);
		    }
		    else return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}
	}

	//--------------------------------------------------------------------
	// isLast
	// Is cursor on the last row?
	//--------------------------------------------------------------------

	public boolean isLast ()
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

			if (numberOfRows > 0)
			{
				return (rowPosition == numberOfRows);
			}
			else
				return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}
	
	//--------------------------------------------------------------------
	// beforeFirst
	// Move cursor before the first row
	//--------------------------------------------------------------------

	public void beforeFirst ()
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {
		    
			boolean done = false;

			done = fetchScrollOption( 0, OdbcDef.SQL_FETCH_ABSOLUTE);

			//if (done)
			rowPosition = 0;
			currentBlockCell = 0;

			if (atInsertRow)
			{
			    lastRowPosition = 0;
			    lastBlockPosition = 0;
			    atInsertRow = false;
			}
			    
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}

	//--------------------------------------------------------------------
	// afterLast
	// Move cursor after the last row
	//--------------------------------------------------------------------

	public void afterLast ()
		throws SQLException
	{
		checkOpen ();

		boolean done = false;

		if ( getType () != ResultSet.TYPE_FORWARD_ONLY )
		{

			done = fetchScrollOption(numberOfRows + 1, OdbcDef.SQL_FETCH_ABSOLUTE);

			//if (done)
			rowPosition = numberOfRows + 1;
			currentBlockCell = rowSet + 1;

			if (atInsertRow)
			{
			    lastRowPosition = 0;
			    lastBlockPosition = 0;
			    atInsertRow = false;
			}
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}

	//--------------------------------------------------------------------
	// first
	// Move cursor to the first row
	//--------------------------------------------------------------------

	public boolean first ()
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if (numberOfRows > 0)
		    {
			boolean fetchBlock = false;
			boolean done = false;
		
			if (blockCursor)
			{
			    fetchBlock = blockFetch(1, OdbcDef.SQL_FETCH_FIRST);

			    if (!fetchBlock)
				done = true;
			}
			
			if (!blockCursor || fetchBlock)
			{
			    resetInsertRow();

			    lastColumnNull = false;

			    // Close any/all input streams for the statement
			    
			    closeInputStreams ();

			    // Reset last warning message

			    clearWarnings ();

			    done = fetchScrollOption(rowPosition, OdbcDef.SQL_FETCH_FIRST);
			}
			
			if(done)
			{
			    rowPosition = 1;
			    currentBlockCell = rowPosition;
			}

			return done;
		    }
		    else
			return false;

		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}


	}

	//--------------------------------------------------------------------
	// last
	// Move cursor to the last row
	//--------------------------------------------------------------------

	public boolean last ()
		throws SQLException
	{ 
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if (numberOfRows > 0)
		    {
			// return to current row 
			// after inserts.
			moveToCurrentRow();

			boolean fetchBlock = false;
			boolean done = false;
			
			if (blockCursor)
			{			    
			    fetchBlock = blockFetch(numberOfRows, OdbcDef.SQL_FETCH_LAST);
				
			    if (!fetchBlock)
			    {
				setPos(currentBlockCell, OdbcDef.SQL_POSITION);
				done = true;
			    }
			    else
				done = true;
			
			}

			if (!blockCursor || fetchBlock)
			{
			    resetInsertRow();

			    lastColumnNull = false;

			    // Close any/all input streams for the statement
			    
			    closeInputStreams ();

			    // Reset last warning message

			    clearWarnings ();

			    if (fetchBlock)
				done = fetchScrollOption(numberOfRows, OdbcDef.SQL_FETCH_ABSOLUTE);
			    else
				done = fetchScrollOption(numberOfRows, OdbcDef.SQL_FETCH_LAST);

			    if (done)
				rowPosition = numberOfRows;
				currentBlockCell = 1;

			}
			return done;

		    }
		    else
			return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}


	}

	//--------------------------------------------------------------------
	// getRow
	// Retrieve the number of row that cursor is on
	//--------------------------------------------------------------------

	public int getRow ()
		throws SQLException
	{
	checkOpen ();
		if (getType () == ResultSet.TYPE_FORWARD_ONLY) {

		    if (lastForwardRecord == 0)
		    {
			    return rowPosition;
		    }
		    else
		    {
			    return 0;
		    }
		}
		else 
		{
		    //return ( (fetchCount - 1) * rowSet) + currentBlockCell;

		    if (numberOfRows > 0)
		    {
			if ( (rowPosition <= 0) || (rowPosition > numberOfRows) )
			{
				return 0;
			}
			else
			{
				//return getRowNumber();
				return rowPosition;
			}
		    }
		    else 
			return 0;
		}
	}

	//--------------------------------------------------------------------
	// absolute
	// Given a row number, move cursor to that row.
	// Return false if cursor moves out of result set.
	//--------------------------------------------------------------------

	public boolean absolute (
		int row)
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if (numberOfRows > 0)
		    {

			boolean done = false;
			boolean fetchBlock = false;
		
			if (row != 0)
			{

			    if (blockCursor)
			    {
				if (atInsertRow)
				{
				    rowPosition = lastRowPosition;
				    currentBlockCell = lastBlockPosition;
				    atInsertRow = false;
				}

			    	fetchBlock = blockFetch(row, OdbcDef.SQL_FETCH_ABSOLUTE);
				
				if (fetchBlock)
				{
				    currentBlockCell = 1;
				}
				else
				{
				    // set the blockPosition for the current row.
				    setPos(currentBlockCell, OdbcDef.SQL_POSITION);
				    done = true;
				}
				
			    }

			    if (!blockCursor || fetchBlock)
			    {

				if (row >= 0)
					rowPosition = row;
				else
					rowPosition = numberOfRows + 1 + row;

				if (rowPosition > numberOfRows)
				{
					afterLast();
					return false;
				}	
				else if (rowPosition < 1 )
				{
					beforeFirst();
					return false;
				}

				lastColumnNull = false;

				// Close any/all input streams for the statement
				
				closeInputStreams ();

				// Reset last warning message

				clearWarnings ();
				
				done = fetchScrollOption(row, OdbcDef.SQL_FETCH_ABSOLUTE);

			    }
			    return done;

			}
			else
			{
				throw new SQLException
					("Cursor position (" + row + ") is invalid");
			}
		    }
		    else
			return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}


	//--------------------------------------------------------------------
	// fetchScrollOption
	// Given a row number, move the cursor to that row.
	// Return false if cursor moves out of result set.
	//--------------------------------------------------------------------

	protected boolean fetchScrollOption (int row, short option) throws SQLException
	{		
		
		if (numberOfRows > 0)
		{
		    try {
			    OdbcApi.SQLFetchScroll (
				    hStmt,
				    option,
				    row);

		    }
		    catch (SQLWarning ex) {

			// If we got a warning, set the last warning for
			// the result set.  We will assume that the fetch
			// positioned us to a vaild row.

			setWarning (ex);
			    
			return true;
		    }
		    catch (SQLException se) {
			    // If an exception is thrown, ignore it.
			    return false;
		    }
		    return true;
		}
		else
		    return false;
	
	}


	//--------------------------------------------------------------------
	// consecutiveFetch
	// Allows getXXX() methods to return data on consecutive calls on the
	// same column index, or after and column update.
	//--------------------------------------------------------------------

	protected void consecutiveFetch (int colIndex) throws SQLException
	{		
	
		// If a consecutive call on the same column
		// refetch the data.

		boolean fetchForBlock = false;

		if (blockCursor && rowUpdated)
		    fetchForBlock = true;
		
		if (rowSet == 1)
		{
		    // do nothing for none block-cursor.
		}
		else if (lastColumnData == colIndex || fetchForBlock)
		{
		    try {
			    OdbcApi.SQLFetchScroll (
				    hStmt,
				    OdbcDef.SQL_FETCH_ABSOLUTE,
				    getRow());

			    lastColumnData = 0;

			    if (blockCursor)
			    {
				currentBlockCell = 1;
			    }


		    }
		    catch (SQLWarning ex) {

			// If we got a warning, set the last warning for
			// the result set.  We will assume that the fetch
			// positioned us to a vaild row.

			setWarning (ex);
			    
		    }
		    catch (SQLException se) {
			    // If an exception is thrown, ignore it.
		    }

		    rowUpdated = false;
		}
		else
			lastColumnData = colIndex;			
	}

	//--------------------------------------------------------------------
	// relative
	// Move cursor x number rows relative to the current row
	//--------------------------------------------------------------------

	public boolean relative (int rows)
		throws SQLException
	{
		checkOpen ();
	    return relative(rows, true);
	}

	//--------------------------------------------------------------------
	// relative w/ boolean option
	// Moves the cursor x number rows relative to the current row
	// or moves the cursor to the next row if the call is not relative.
	//--------------------------------------------------------------------

	protected boolean relative (int rows, boolean relative)
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    if (numberOfRows > 0)
		    {

			// return to current row 
			// after inserts.
			moveToCurrentRow();

			boolean done = false;
			boolean fetchBlock = false;
			int testPosition = -1;


			// if the call is relative and the cursorPosition
			// is before/after the ResultSet return the respective 
			// out of bounds exception.

			    if (relative)
			    {
				if (rowPosition == 0)
				{
				    throw new SQLException
					    ("Cursor is positioned before the ResultSet");
				}
				else if (rowPosition > numberOfRows)
				{
				    throw new SQLException
					    ("Cursor is positioned after the ResultSet");	    
				}
			    }
			
			// if the rowSet > 1, determine if the row needs to be fetch
			// or just repositioned within the block-cursor.

			    if (blockCursor)
			    {
				fetchBlock = blockFetch(rowPosition + rows, OdbcDef.SQL_FETCH_ABSOLUTE);

				// check boundaries for desire row position.
				// if fetchBlock is true, test the new rowPosition.
				// Also fetch for calls with refreshRow() or relative(0).
				if (fetchBlock)
				{
				    testPosition = rowPosition;
				    testPosition += rows;
				}
				else if(rows == 0)
				{
				    testPosition = rows;
				    fetchBlock = true;
				}
			    }
			    else
			    {
				    testPosition = rowPosition;
				    testPosition += rows;
			    }


			// if the cursorPosition is borderline
			// move cursor before/after the resultset.

			    if ( (testPosition <= 1) && (rows < 0) )
			    {
				beforeFirst();

				if (relative)
				    return false;
				else if( testPosition == 1 )
				    return true;
			    }
			    else if ( (testPosition >= numberOfRows) && (rows > 0) )
			    {
				afterLast();

				if (relative)
				    return false;
				else if( testPosition == numberOfRows )
				    return true;
			    }
			    else
			    {

				lastColumnNull = false;

				// Close any/all input streams for the statement
				
				closeInputStreams ();

				// Reset last warning message

				clearWarnings ();

				if (blockCursor)
				{
					if (fetchBlock)
					{
						done = fetchScrollOption (testPosition, OdbcDef.SQL_FETCH_ABSOLUTE);

						if (done)
						{
						    rowPosition = testPosition;						
						    currentBlockCell = 1;
						}
					}
					else
					{
					    // set the row position within the rowSet.
					    setPos(currentBlockCell, OdbcDef.SQL_POSITION);
					    done = true;
					}
				}
				else
				{
					done = fetchScrollOption (rows, OdbcDef.SQL_FETCH_RELATIVE);
					
					if (done)
					    rowPosition = testPosition;
				}
				return done;
			    }

		    }
		    else
			throw new SQLException
				("Call to relative(" + rows + ") when there is no current row.");
			return false;

		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}

	//--------------------------------------------------------------------
	// previous
	// Move cursor to previous row
	//--------------------------------------------------------------------

	public boolean previous ()
		throws SQLException
	{
		checkOpen ();

		if (getType () != ResultSet.TYPE_FORWARD_ONLY) {

		    int lastRow = 0;

		    if (numberOfRows > 0)
		    {
			
			if( atInsertRow )
			    lastRow = lastRowPosition;

			// return to current row 
			// after inserts.
			moveToCurrentRow();

			if (getFetchDirection () == ResultSet.FETCH_FORWARD)
			{
			    if (lastRow > 0) // back to current row after insert.
				    return absolute (lastRow - 1);
			    else if (rowPosition > 1)
				    return absolute(rowPosition - 1);
			    else if (rowPosition == 1)
			    {
				 beforeFirst();
				 return false;
			    }
			    else if ( isBeforeFirst() )
				    return false;
			    else
				    return false;
			}
			else
			{

			    if (lastRow > 0) // back to current row after insert.
				    return absolute (lastRow + 1);
			    else if (rowPosition < numberOfRows)
				    return absolute(rowPosition + 1);
			    else if (rowPosition == numberOfRows)
			    {
				    afterLast();
				    return false;
			    }
			    else if ( isAfterLast() )
				    return false;
			    else 
				    return false;
			
			}

		    }
		    else
			return false;
		}
		else
		{
			throw new SQLException
				("Result set type is TYPE_FORWARD_ONLY");
		}

	}

	//--------------------------------------------------------------------
	// blockFetch
	// Return true if SQLFetchScroll is required to obtain the needed row.
	//--------------------------------------------------------------------

	protected boolean blockFetch(int needRow, short fetchDirection)
		throws SQLException
	{

		boolean fetchBlock = false;
		
		if (isBeforeFirst() || isAfterLast())
		{
		    return true;
		}

		switch(fetchDirection)
		{
		
		    case OdbcDef.SQL_FETCH_FIRST:

			if (rowPosition != 1)
			{
			    if (isRowWithinTheBlock(1) == false)
			    {
				    fetchBlock = true;
			    }
			    else
			    {
				rowPosition = 1;
				currentBlockCell = rowPosition;
			    }
			}			

			break;

		    case OdbcDef.SQL_FETCH_LAST:

			if (rowPosition < numberOfRows)
			{			    
			    if (isRowWithinTheBlock(numberOfRows) == false)
			    {
				    fetchBlock = true;
			    }			   
			    else
			    {
				// advance row positions within
				// the block to get to the last row.
				while (rowPosition != numberOfRows)
				{
					rowPosition++;
					currentBlockCell++;
				}
				fetchBlock = false;

			    }

			}
			else if (rowPosition == numberOfRows)
			{
				 fetchBlock = false;
			}

			break;


		    case OdbcDef.SQL_FETCH_ABSOLUTE:
			
			if (rowPosition != needRow)
			{					
			    if ( (needRow < 0) || (needRow > numberOfRows) )
			    {
				// if out of bounds, 
				// let this be handle within absolute/relative method.
				fetchBlock = true;				    
			    }
			    else if (isRowWithinTheBlock(needRow) == false)
			    {
				fetchBlock = true;
			    }
			    else
			    {
				// advance row positions within
				// the block to get to the last row.
				while (rowPosition != needRow)
				{
				    if(moveUpBlock)
				    {
					rowPosition--;
					currentBlockCell--;
				    }
				    else if(moveDownBlock)
				    {
					rowPosition++;
					currentBlockCell++;
				    }

				}
				fetchBlock = false;
			    }
			}

			break;

		    default: break;
		    //case OdbcDef.SQL_FETCH_NEXT:
		    //case OdbcDef.SQL_FETCH_RELATIVE:
		    //case OdbcDef.SQL_FETCH_PREVIOUS:

		}

		return fetchBlock;
	}

	//--------------------------------------------------------------------
	// isRowWithinTheBlock
	// If and block-cursor is used, obtimize SQLFetchScroll when posible by
	// skiping fetch calls when the needed row is already in the rowSet.
	//--------------------------------------------------------------------

	protected boolean isRowWithinTheBlock(int needRow)
	{
	    boolean withInBlock = false;
	    
	    if (rowPosition != 0)
	    {		
		int lookUp	= rowPosition - (currentBlockCell - 1);
		int lookDown	= rowPosition + (rowSet - currentBlockCell);

		if( (lookDown < needRow ) || (lookUp > needRow) )
		{
		    withInBlock = false;
		}		
		else if(needRow > rowPosition)
		{
		    withInBlock = true;
		    moveUpBlock = false;
		    moveDownBlock = true;
		}
		else if(needRow < rowPosition)
		{
		    withInBlock = true;
		    moveUpBlock = true;
		    moveDownBlock = false;
		}
		
	    }

	    return withInBlock;
	}

	//--------------------------------------------------------------------
	// getRowIndex
	// updateXXX() methods need the column's Object[Index] to store new data.
	//--------------------------------------------------------------------

	protected int getRowIndex ()
	{
	    int rowIndex = 0;
	    	    
	    if ( blockCursor )
	    {
		rowIndex = currentBlockCell - 1;
	    }
	    else if (atInsertRow)
	    {
		rowIndex = rowSet;
	    }

	    return rowIndex;
	}

	//--------------------------------------------------------------------
	// setFetchDirection
	// Set fetch direction hint
	//--------------------------------------------------------------------

	public void setFetchDirection (
		int direction)
		throws SQLException
	{
		checkOpen ();
		ownerStatement.setFetchDirection (direction);
	}

	//--------------------------------------------------------------------
	// getFetchDirection
	// Get fetch direction hint
	//--------------------------------------------------------------------

	public int getFetchDirection ()
		throws SQLException
	{
		checkOpen ();
		return ownerStatement.getFetchDirection ();
	}

	//--------------------------------------------------------------------
	// setFetchSize
	// Set fetch size hint. Size hint is currently ignored.
	//--------------------------------------------------------------------

	public void setFetchSize (
		int rows)
		throws SQLException
	{
		checkOpen ();
		ownerStatement.setFetchSize (rows);
	}

	//--------------------------------------------------------------------
	// getFetchSize
	// get fetch size hint
	//--------------------------------------------------------------------

	public int getFetchSize ()
		throws SQLException
	{
		checkOpen ();
		return ownerStatement.getFetchSize ();
	}


	//--------------------------------------------------------------------
	// getType
	// Retrieve result set type
	//--------------------------------------------------------------------

	public int getType ()
		throws SQLException
	{
		checkOpen ();
		if (ownerStatement != null)
			return ownerStatement.getResultSetType ();
		else
			return ResultSet.TYPE_FORWARD_ONLY;
	}


	//--------------------------------------------------------------------
	// getConcurrency
	// Retrieve concurrency
	//--------------------------------------------------------------------

	public int getConcurrency ()
		throws SQLException
	{
		checkOpen ();
		return ownerStatement.getResultSetConcurrency ();
	}


	//--------------------------------------------------------------------
	// rowUpdated
	// true if the current row was updated.
	//--------------------------------------------------------------------

	public boolean rowUpdated()
		throws SQLException
	{
		checkOpen ();
	    if (numberOfRows > 0)
	    {
		int row = getRowIndex();

		if (blockCursor)
		{
		    return ( rowStatusArray[row] == OdbcDef.SQL_ROW_UPDATED );
		}
		else
		    return ( rowStatusArray[rowSet - 1] == OdbcDef.SQL_ROW_UPDATED );
	    }
	    else
		    return false;
	}


	//--------------------------------------------------------------------
	// rowInserted
	// true if the row was inserted.
	//--------------------------------------------------------------------

	public boolean rowInserted()
		throws SQLException
	{ 
		checkOpen ();
	    if (numberOfRows > 0)
	    {
		int row = getRowIndex();

		if (blockCursor)
		{
		    return ( rowStatusArray[row] == OdbcDef.SQL_ROW_ADDED );
		}
		else
		    return ( rowStatusArray[rowSet - 1] == OdbcDef.SQL_ROW_ADDED );
	    }
	    else
		    return false;
	}


	//--------------------------------------------------------------------
	// rowDeleted
	// true if the current row was deleted.
	//--------------------------------------------------------------------

	public boolean rowDeleted()
		throws SQLException
	{
		checkOpen ();
	    if (numberOfRows > 0)
	    {

		int row = getRowIndex();

		if (blockCursor)
		{
		    return ( rowStatusArray[row] == OdbcDef.SQL_ROW_DELETED );
		}
		else
		    return ( rowStatusArray[rowSet - 1] == OdbcDef.SQL_ROW_DELETED );
	    }
	    else
		    return false;
	}


	//--------------------------------------------------------------------
	// updateNull
	//--------------------------------------------------------------------

	public void updateNull (
		int columnIndex)
                throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();
	    
	    if ((columnIndex > 0) && (columnIndex <= numberOfCols))
	    {		    
		// Set column's Object value as an Integer.
		int currentType = getColumnType(columnIndex);

		// Set column value to null.
		if (currentType != OdbcDef.SQL_TYPE_UNKNOWN)
		{
		    // Since NUMERIC/DECIMAL Types are bound as Strings
		    // a 0 is expected when the value is null.
		    // 4672508
	    	    boundCols[columnIndex - 1].setRowValues (row, null, OdbcDef.SQL_NULL_DATA);
		}
		else
		    throw new SQLException ("Unknown Data Type for column [#" + columnIndex + "]");
	    }
	}


	//--------------------------------------------------------------------
	// updateBoolean
	//--------------------------------------------------------------------

	public void updateBoolean (
		int columnIndex,
		boolean x)
		throws SQLException
	{
		checkOpen ();
		int value = 0;

		// If the parameter is true, set the value to 1
		if (x) {
			value = 1;
		}

		// updateBoolean as if it were an integer
		updateInt (columnIndex, value);
	}


	//--------------------------------------------------------------------
	// updateByte
	//--------------------------------------------------------------------

	public void updateByte (
		int columnIndex,
		byte x)
                throws SQLException
	{
		checkOpen ();
		// updateByte as if it were an integer
		// updateInt (columnIndex, x);	
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols))
	    {		    
		// Set column's Object value as an Integer.
		int currentType = getColumnType(columnIndex);

		if (currentType != Types.TINYINT )
		    boundCols[columnIndex - 1].setType(Types.TINYINT);

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, new Integer(x), 4);		
	    }
	}


	//--------------------------------------------------------------------
	// updateShort
	//--------------------------------------------------------------------

	public void updateShort (
		int columnIndex,
		short x)
		throws SQLException
	{
		checkOpen ();
		// updateShort as if it were an integer
		// updateInt (columnIndex, x);	
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols))
	    {		    
		// Set column's Object value as an Integer.
		int currentType = getColumnType(columnIndex);

		if (currentType != Types.SMALLINT )
		    boundCols[columnIndex - 1].setType(Types.SMALLINT);

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, new Integer(x), 4);		
	    }


	}


	//--------------------------------------------------------------------
	// updateInt
	//--------------------------------------------------------------------	

	public void updateInt (
		int columnIndex,
		int x)
		throws SQLException
	{
		checkOpen ();
		
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols))
	    {		    
		// Set column's Object value as an Integer.
		int currentType = getColumnType(columnIndex);

		if (currentType != Types.INTEGER )
		    boundCols[columnIndex - 1].setType(Types.INTEGER);

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, new Integer(x), 4);		
	    }
	}


	//--------------------------------------------------------------------
	// updateLong
	//--------------------------------------------------------------------

	public void updateLong (
		int columnIndex,
		long x)
                throws SQLException
	{
		checkOpen ();
		// updateLong as if it were a float.
		updateFloat (columnIndex, x);	
	}


	//--------------------------------------------------------------------
	// updateFloat
	//--------------------------------------------------------------------

	public void updateFloat (
		int columnIndex,
		float x)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ( (columnIndex > 0) && (columnIndex <= numberOfCols) )
	    {		    
		// Set column's Object value as Float.
		int currentType = getColumnType(columnIndex);

		if (currentType != Types.FLOAT)
		    boundCols[columnIndex - 1].setType(Types.FLOAT);
		
		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, new Float(x), 4);
	    }
	}


	//--------------------------------------------------------------------
	// updateDouble
	//--------------------------------------------------------------------

	public void updateDouble (
		int columnIndex,
		double x)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {		    
		// Set column's Object value as Double.
		int currentType = getColumnType(columnIndex);

		if (currentType != Types.DOUBLE)
		    boundCols[columnIndex - 1].setType(Types.DOUBLE);
		
		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, new Double(x), 8 );
	    }		
	}


	//--------------------------------------------------------------------
	// updateBigDecimal
	//--------------------------------------------------------------------

	public void updateBigDecimal (
		int columnIndex,
		BigDecimal x)
		throws SQLException
	{
		checkOpen ();
		int currentType = getColumnType(columnIndex);

		if ( currentType == Types.DECIMAL || currentType == Types.NUMERIC )
		{
		    // 4672508
		    if(x == null) {
		        updateChar (columnIndex, currentType, null);
		    } else {
		        updateChar (columnIndex, currentType, x.toString ());
		    }
		}
		else // Bug 4668121
		{
		    // 4672508
		    if(x == null) {
		        updateChar (columnIndex, Types.NUMERIC, null);
		    } else {
		        updateChar (columnIndex, Types.NUMERIC, x.toString ());
		    }
		}

	}


	//--------------------------------------------------------------------
	// updateString
	//--------------------------------------------------------------------

	public void updateString (
		int columnIndex,
		String x)
		throws SQLException
	{
		checkOpen ();
		
		byte[] bytes;
		
		try {
		    // 4672508
		    if(x == null) {
		        bytes = null;
		    } else {
		        bytes = x.getBytes(OdbcApi.charSet);
		    }
		}
		catch (java.io.UnsupportedEncodingException uee) {
			throw new SQLException (uee.getMessage());
		}

		updateBytes (columnIndex, bytes);
		
    }


	//--------------------------------------------------------------------
	// updateBytes
	//--------------------------------------------------------------------

	public void updateBytes (
		int columnIndex,
		byte[] x)
		throws SQLException
	{
		checkOpen ();
		
	    int row = getRowIndex();
	    
	    // 4672508
	    if ( x != null && x.length > JdbcOdbcLimits.DEFAULT_IN_PRECISION )
	    {
		updateBinaryStream(columnIndex, new java.io.ByteArrayInputStream(x), x.length);
	    }

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {		    
		
		// Set column's Object value as byte[] Object.

		int currentType = getColumnType(columnIndex);

		if (currentType != Types.BINARY && currentType != Types.VARBINARY) {		
			boundCols[columnIndex - 1].setType(Types.BINARY);
			//4717222
			if(x != null) {
				boundCols[columnIndex - 1].setLength(x.length);
			}
		}
			
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		//keep track of the largest element in this column.
		if ( x.length > boundCols[columnIndex - 1].getLength() )
			boundCols[columnIndex - 1].setLength(x.length);

		boundCols[columnIndex - 1].setRowValues (row, (byte[])x, x.length );
	    }		
	
	}


	//--------------------------------------------------------------------
	// updateDate
	//--------------------------------------------------------------------

	public void updateDate (
		int columnIndex,
		Date x)
                throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {		    
		// Set column's value as Date Object.
		if (getColumnType(columnIndex) != Types.DATE)
		    boundCols[columnIndex - 1].setType(Types.DATE);
		    
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, (java.sql.Date)x, 6 );
	    }	
	}


	//--------------------------------------------------------------------
	// updateTime
	//--------------------------------------------------------------------

	public void updateTime (
		int columnIndex,
		Time x)
                throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {
		    
		// Set column's value as Time Object.
		if (getColumnType(columnIndex) != Types.TIME)
		    boundCols[columnIndex - 1].setType(Types.TIME);
		    
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, (java.sql.Time)x, 6);

	    }	
	}


	//--------------------------------------------------------------------
	// updateTimestamp
	//--------------------------------------------------------------------

	public void updateTimestamp (
		int columnIndex,
		Timestamp x)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols))
	    {
		    
		// Set column's value as Timestamp Object.
		if (getColumnType(columnIndex) != Types.TIMESTAMP)
		    boundCols[columnIndex - 1].setType(Types.TIMESTAMP);
		    
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		// bug 4668123, passing the length of the data type
		boundCols[columnIndex - 1].setRowValues (row, (java.sql.Timestamp)x , 16);

	    }	
	}


	//--------------------------------------------------------------------
	// The normal updateString and updateBytes methods are suitable for passing
	// normal sized data.  However occasionally it may be necessary to send
	// extremely large values as LONGVARCHAR or LONGVARBINARY column values.
	// In this case you can pass in a java.io.Reader or java.io.InputStream
	// object.
	//
	// JDBC runtimes will read data from that stream as needed, until they
	// reach end-of-file.  Note that these stream objects can either be
	// standard Java stream objects, or your own subclass that implements
	// the standard interface.
	// updateAsciiStream and updateCharacterStream imply the use of the 
	// SQL LONGVARCHAR type, and updateBinaryStream implies the 
	// SQL LONGVARBINARY type.
	// For each stream type you must specify the number of bytes to be
	// read from the stream and sent to the database.
	//--------------------------------------------------------------------

	public void updateAsciiStream (
		int columnIndex,
		InputStream x,
		int length)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {	    
		// Set column's InputStream value and lenth.
		if (getColumnType(columnIndex) != Types.LONGVARCHAR)
		    boundCols[columnIndex - 1].setType(Types.LONGVARCHAR);
		    
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		// reset column length for putColumnData use.
		if (length != boundCols[columnIndex - 1].getLength())
		    boundCols[columnIndex - 1].setLength (length);

		boundCols[columnIndex - 1].setRowValues (row, (java.io.InputStream)x , length);
		boundCols[columnIndex - 1].setStreamType (JdbcOdbcBoundCol.ASCII);		

	    }	
	}


	//--------------------------------------------------------------------
	// updateBinaryStream
	//--------------------------------------------------------------------

	public void updateBinaryStream (
		int columnIndex,
		InputStream x,
		int length)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {
		    
		// Set column's InputStream value and lenth.
		if (getColumnType(columnIndex) != Types.LONGVARBINARY)
		    boundCols[columnIndex - 1].setType(Types.LONGVARBINARY);
		    
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		if (length != boundCols[columnIndex - 1].getLength())
		    boundCols[columnIndex - 1].setLength (length);

		boundCols[columnIndex - 1].setRowValues (row, (java.io.InputStream)x , length);
		boundCols[columnIndex - 1].setStreamType (JdbcOdbcBoundCol.BINARY);
	    }	
	}


	//--------------------------------------------------------------------
	// updateCharacterStream.
	//--------------------------------------------------------------------

	public void updateCharacterStream (
		int columnIndex,
		Reader x,
		int length)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

	    // Set of Streams to read chars from:
	    java.io.BufferedReader bReader = null;
	    java.io.BufferedOutputStream bout = null;

	    // Set of Streams to write Bytes to:
	    java.io.ByteArrayOutputStream outBytes = null;
	    java.io.ByteArrayInputStream  inAsBytes = null;

	    // Get the connection's character Set.
	    // String encoding = System.getProperty("file.encoding");
	    String encoding = OdbcApi.charSet;

	    // set character buffer size.
	    int bufLength = JdbcOdbcLimits.DEFAULT_BUFFER_LENGTH; 

	    if (length < bufLength)
		bufLength = length;

	    int inputLength = 0;
	    int maxBytesPerChar = 0;

	    try
	    {
		// Create converter to be used in String to Byte conversion.
		sun.io.CharToByteConverter toBytes = sun.io.CharToByteConverter.getConverter (encoding);
		maxBytesPerChar = toBytes.getMaxBytesPerChar();
		//System.out.println("maxBytesPerChar =  " + maxBytesPerChar + " for (" + encoding + ") encoding");

	    }
	    catch (java.io.UnsupportedEncodingException uee){}

	    // make sure that maxBytesPerChar
	    // is at least > 0.
	    if (maxBytesPerChar == 0)
		maxBytesPerChar  = 1;	    	    
	    
	    try
	    {
		if (x != null)
		{
		    int totCount = 0;
		    int count = 0;

		    bReader	= new java.io.BufferedReader(x);
		    outBytes	= new java.io.ByteArrayOutputStream();
		    bout	= new java.io.BufferedOutputStream(outBytes);
			 
		    char[] charBuffer = new char[bufLength];

		    while ( count != -1 )
		    {
			byte[] bytesBuffer = new byte[0];
			
			count = bReader.read(charBuffer);

			if (count != -1)
			{

			    // re-size the char buffer to
			    // prevent conversion of unnessesary
			    // chars.

			    char[] tmpCharBuf  = new char[count];

			    for (int i = 0; i < count; i++)
			    {
				tmpCharBuf[i] = charBuffer[i];
			    }


			    //totCount += count;
			    //System.out.println("Chars read = " + count);
			    //System.out.println("total chars read = " + totCount);

			    // calculate the length of bytes to write to the outputstream.
			    // For ASCII, A one to one will do. However double or even triple
			    // byte characters will require for the stream to write more bytes.
			    // out of the bytes buffer. 

			    bytesBuffer = CharsToBytes (encoding, tmpCharBuf);

			    // Strip off the null terminator byte.
			    int byteCount = bytesBuffer.length - 1;
			    			
			    // write bytes to the outputStream	
			    bout.write(bytesBuffer, 0, byteCount);

			    // flush any unwriten bytes
			    bout.flush();			

							    			    
			} //end if.


		    } //end while.
		    
		    // set the InputStream length.
		    inputLength = outBytes.size();

		    // debug prior to sending into the database.
		    // System.out.println("Input length = " + inputLength);
		    // System.out.println("Input value = " + outBytes.toString());

		    inAsBytes = new java.io.ByteArrayInputStream(outBytes.toByteArray());
	    
		}

	    }
	    catch (java.io.IOException ioe)
	    {
		throw new SQLException("CharsToBytes Reader Conversion: " + ioe.getMessage());
	    }


	    if ((columnIndex > 0) && (columnIndex <= numberOfCols)) 
	    {
		// Set column's InputStream value and lenth.
		if (getColumnType(columnIndex) != Types.LONGVARCHAR ||
		    getColumnType(columnIndex) != Types.VARCHAR)
		{
		    boundCols[columnIndex - 1].setType(Types.LONGVARCHAR);  // 4638528
		}
		
		// 4672508
		if(x == null) {
		    updateNull(columnIndex);
		    return;
		}

		if (inputLength != boundCols[columnIndex - 1].getLength())
		    boundCols[columnIndex - 1].setLength (inputLength);

		boundCols[columnIndex - 1].setRowValues (row, (java.io.InputStream)inAsBytes , inputLength);
		boundCols[columnIndex - 1].setStreamType (JdbcOdbcBoundCol.BINARY);
		
	    }	

	}


	//--------------------------------------------------------------------
	// updateObject
	// You can update a column as a Java object.  See the JDBC spec's 
	// "Dynamic Programming" chapter for information on valid Java types.
	//--------------------------------------------------------------------

	public void updateObject (
		int columnIndex,
		Object x,
		int scale)
		throws SQLException
	{	    
	    updateObject ( columnIndex, x, scale, boundCols[columnIndex -1].getType() );
	}

	public void updateObject (
		int columnIndex,
		Object x)
		throws SQLException
	{
	    updateObject ( columnIndex, x, 0, boundCols[columnIndex -1].getType() );	
	}


	protected void updateObject (
		int columnIndex,
		Object x,
		int scale,
		int sqlType)
		throws SQLException
	{
		checkOpen ();
		
	    if (sqlType == OdbcDef.SQL_TYPE_UNKNOWN && (x != null) )
	    {
		sqlType = ownerStatement.getTypeFromObject(x);
	    }
	    else if (x == null)
	    {
		sqlType = Types.NULL;
	    }

	    if ((columnIndex > 0) &&
		(columnIndex <= numberOfCols))
	    {
		    
		// For each known SQL Type, call the appropriate
		// set routine
		switch (sqlType) {
		case Types.CHAR:
		case Types.VARCHAR:
			//4631409
			updateString (columnIndex, (String) x);
		 	break;	
		 	
		case Types.LONGVARCHAR:

			//4717222
			if ( (x instanceof byte[]) && ((byte[])x != null) )
			{
			    byte[] y = (byte[])x;
			     
			    updateAsciiStream(columnIndex, new java.io.ByteArrayInputStream(y), y.length);
			} //4717222
			else if ( (x instanceof java.io.Reader) && ((Reader)x != null))
			{
			    throw new SQLException ("Unknown length for Reader Object, try updateCharacterStream.");
			} //4717222
			else if ( (x instanceof String) && ((String)x != null) )
			{
			    //4717222
			    updateString (columnIndex, (String) x);
			}
			break;

		case Types.NUMERIC:
		case Types.DECIMAL:
			updateBigDecimal (columnIndex, (BigDecimal) x);
			break;

		case Types.BIT:
			updateBoolean (columnIndex,
					((Boolean) x).booleanValue ());
			break;

		case Types.TINYINT:
			updateByte (columnIndex, (byte)((Integer) x).intValue ());
			break;

		case Types.SMALLINT:
			updateShort (columnIndex, (short)
					((Integer) x).intValue ());
			break;

		case Types.INTEGER:
			updateInt (columnIndex,
					((Integer) x).intValue ());
			break;

		case Types.BIGINT:
			updateLong (columnIndex,
					((Integer) x).longValue ());
			break;

		case Types.REAL:
		case Types.FLOAT:
			updateFloat (columnIndex, ((Float) x).floatValue ());
			break;

		case Types.DOUBLE:
			updateDouble (columnIndex,
					((Double) x).doubleValue ());
			break;

		case Types.BINARY:
			// 4666823
			if(x instanceof String) {
			    try {
		    		updateBytes (columnIndex, ((String)x).getBytes(OdbcApi.charSet));
			    } catch (java.io.UnsupportedEncodingException uee) {
				throw new SQLException (uee.getMessage());
			    }
			} else {
			    updateBytes (columnIndex, (byte[]) x);
			}
			break;

		case Types.VARBINARY:
		case Types.LONGVARBINARY:

			// 4666823
			byte[] y = null;
			if(x instanceof String) {
			    try {
		    		y = ((String)x).getBytes(OdbcApi.charSet);
			    } catch (java.io.UnsupportedEncodingException uee) {
				throw new SQLException (uee.getMessage());
			    }
			} else {
			    y = (byte[]) x;
			}
			
			if ( y.length > JdbcOdbcLimits.DEFAULT_IN_PRECISION )
			{
			    updateBinaryStream (columnIndex, new java.io.ByteArrayInputStream(y), y.length);
			}
			else
			{
			    updateBytes (columnIndex, y);
			}
			break;

		case Types.DATE:
			updateDate (columnIndex, (java.sql.Date) x);
			break;

		case Types.TIME:
			updateTime (columnIndex, (java.sql.Time) x);
			break;

		case Types.TIMESTAMP:
			updateTimestamp (columnIndex,
					(java.sql.Timestamp) x);
			break;

		case Types.NULL:
			updateNull(columnIndex);
			break;

		default:
			throw new SQLException ("Unknown SQL Type for ResultSet.updateObject SQL Type = " + sqlType);
		}

	    }	
	}

	//--------------------------------------------------------------------
	// updateChar
	// Binds the given string with the given SQL type
	//--------------------------------------------------------------------

	protected void updateChar (
		int columnIndex,
		int SQLtype,
		String x)
		throws SQLException
	{
		checkOpen ();
	    int row = getRowIndex();

		if ( (columnIndex > 0) && (columnIndex <= numberOfCols) )
		{		    		    
		    // Set column's Object value as Double.
		    int currentType = getColumnType(columnIndex);

		    if ( currentType != SQLtype )
			boundCols[columnIndex - 1].setType(SQLtype);
			
		    // 4672508
		    if(x == null) {
		        updateNull(columnIndex);
		    } else {
		        boundCols[columnIndex - 1].setRowValues (row, (String)x, OdbcDef.SQL_NTS );
		    }
		}	
	    	    			
	}

	public void updateNull (
		String columnName)
                throws SQLException
	{
	    updateNull (findColumn (columnName));		
	}

	public void updateBoolean (
		String columnName,
		boolean x)
		throws SQLException
	{
	    updateBoolean (findColumn (columnName), x);		
	}

	public void updateByte (
		String columnName,
		byte x)
                throws SQLException
	{
	    updateInt (findColumn (columnName), x);		
	}

	public void updateShort (
		String columnName,
		short x)
		throws SQLException
	{
	    updateInt (findColumn (columnName), x);	
	}

	public void updateInt (
		String columnName,
		int x)
		throws SQLException
	{
	    updateInt (findColumn (columnName), x);	
	}

	public void updateLong (
		String columnName,
		long x)
                throws SQLException
	{
	    updateFloat (findColumn (columnName), x);	
	}

	public void updateFloat (
		String columnName,
		float x)
		throws SQLException
	{
	    updateFloat (findColumn (columnName), x);	
	}

	public void updateDouble (
		String columnName,
		double x)
		throws SQLException
	{
	    updateDouble (findColumn (columnName), x);	
	}

	public void updateBigDecimal (
		String columnName,
		BigDecimal x)
		throws SQLException
	{
	    updateBigDecimal (findColumn (columnName), x);	
	}

	public void updateString (
		String columnName,
		String x)
		throws SQLException
	{
	    updateString (findColumn (columnName), x);	
	}

	public void updateBytes (
		String columnName,
		byte[] x)
		throws SQLException
	{
	    updateBytes (findColumn (columnName), x);	
	}

	public void updateDate (
		String columnName,
		Date x)
                throws SQLException
	{
	    updateDate (findColumn (columnName), x);	
	}

	public void updateTime (
		String columnName,
		Time x)
                throws SQLException
	{
	    updateTime (findColumn (columnName), x);	
	}

	public void updateTimestamp (
		String columnName,
		Timestamp x)
		throws SQLException
	{
	    updateTimestamp (findColumn (columnName), x);	
	}

	public void updateAsciiStream (
		String columnName,
		InputStream x,
		int length)
		throws SQLException
	{
	    updateAsciiStream (findColumn (columnName), x, length);
	}

	public void updateBinaryStream (
		String columnName,
		InputStream x,
		int length)
		throws SQLException
	{
	    updateBinaryStream (findColumn (columnName), x, length);
	}

	public void updateCharacterStream (
		String columnName,
		Reader reader,
		int length)
		throws SQLException
	{
	    updateCharacterStream (findColumn (columnName), reader, length);
	}

	public void updateObject (
		String columnName,
		Object x,
		int scale)
		throws SQLException
	{
	    updateObject (findColumn (columnName), x, scale);
	}

	public void updateObject (
		String columnName,
		Object x)
		throws SQLException
	{
	    updateObject (findColumn (columnName), x);
	}


	//--------------------------------------------------------------------
	// insertRow
	// adds a new row to the ResultSet.
	//--------------------------------------------------------------------

	public void insertRow()
		throws SQLException
	{
		checkOpen ();
	    for (int i = 0; i < numberOfCols; i++)
	    {
		int type = boundCols[i].getType ();    
		bindCol(i + 1, type);		
	    }
	
	    if (getType () != ResultSet.TYPE_FORWARD_ONLY) 
	    {
		if ( blockCursor )
		{
		    setPos(currentBlockCell, OdbcDef.SQL_ADD);
		}
		else
		{
		    setPos(rowSet, OdbcDef.SQL_ADD);
		}
		FreeCols(); //4486195
		
		// Fix for 4628693, 4668340
		if (ownInsertsAreVisible){
		    numberOfRows++;
		}
		resetColumnState();
		resetInsertRow();		
	    }
	    else	    
		    throw new SQLException("Result set type is TYPE_FORWARD_ONLY");

	}

	//--------------------------------------------------------------------
	// updateRow
	// updates the current row in the ResultSet.
	//--------------------------------------------------------------------

	public void updateRow()
		throws SQLException
	{
		checkOpen ();
		
	    for (int i = 0; i < numberOfCols; i++)
	    {
		int type = boundCols[i].getType ();
		bindCol(i + 1, type);
	    }
		
	    if (getType () != ResultSet.TYPE_FORWARD_ONLY) 
	    {
		if ( blockCursor )
		{
		    setPos(currentBlockCell, OdbcDef.SQL_UPDATE);
		}
		else
		{
		    setPos(rowSet, OdbcDef.SQL_UPDATE);
		}
		FreeCols(); //4486195
		
		resetColumnState();

		// some databases required a refetch to be 
		// performed after and update.
		rowUpdated = true;
	    }
	    else	    
		    throw new SQLException("Result set type is TYPE_FORWARD_ONLY");
	    	
	}

	//--------------------------------------------------------------------
	// deleteRow
	// deletes the current row in the ResultSet.
	//--------------------------------------------------------------------

	public void deleteRow()
		throws SQLException
	{
	    checkOpen ();
	    if ( blockCursor )
	    {
		setPos(currentBlockCell, OdbcDef.SQL_DELETE);
	    }
	    else
		setPos(rowSet, OdbcDef.SQL_DELETE);
		
	    // Fix for 4628693, 4668340
	    if (ownDeletesAreVisible){
	    	numberOfRows--;
	    }

	}

	//--------------------------------------------------------------------
	// setResultSetVisibilityIndicators
	// Sets the Resultset visibility indicators.
	// Fix for 4628693, 4668340
	//--------------------------------------------------------------------	
	private void setResultSetVisibilityIndicators() throws SQLException{
	    int odbcCursorType = OdbcApi.SQLGetStmtAttr(hStmt,OdbcDef.SQL_ATTR_CURSOR_TYPE);
	    short attrName = 0;
	    switch (odbcCursorType) 
	    {
	    	case OdbcDef.SQL_CURSOR_KEYSET_DRIVEN:
		    attrName = OdbcDef.SQL_KEYSET_CURSOR_ATTRIBUTES2;
		    break;
		case OdbcDef.SQL_CURSOR_DYNAMIC:
		    attrName = OdbcDef.SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
		    break;
		case OdbcDef.SQL_CURSOR_STATIC:
		    attrName = OdbcDef.SQL_STATIC_CURSOR_ATTRIBUTES2;
		    break;
	    }    
	    
	    // If Attribute Name is properly set.
	    if ( attrName > 0 ) {
		try {
		    int visibleUpdates = OdbcApi.SQLGetInfo (hDbc, attrName);

	 	    if ( (visibleUpdates & OdbcDef.SQL_CA2_SENSITIVITY_DELETIONS) > 0) {
		     	ownDeletesAreVisible=true;
		    }	    	
		    if ( (visibleUpdates & OdbcDef.SQL_CA2_SENSITIVITY_ADDITIONS) > 0) {
			ownInsertsAreVisible=true;
		    }			    			    		    
		}
		catch (SQLException e) {	
		    // Do nothing.
		}
	    }	    
	    
	}	

	//--------------------------------------------------------------------
	// refreshRow
	// refetches the current row.
	//--------------------------------------------------------------------

	public void refreshRow()
                throws SQLException
	{
		checkOpen ();
	    if (getType () != ResultSet.TYPE_FORWARD_ONLY) 
	    {

		if (!atInsertRow && getRow() > 0)
		{
		    fetchScrollOption (0, OdbcDef.SQL_FETCH_RELATIVE);
		}
		else
		{
		    throw new SQLException("Cursor position is invalid");
		}
				
	    }
	    else
		    throw new SQLException("Result set type is TYPE_FORWARD_ONLY");

	}


	//--------------------------------------------------------------------
	// cancelRowUpdates.
	// clears any update-values prior to updating the row.
	//--------------------------------------------------------------------

	public void cancelRowUpdates()
		throws SQLException
	{
		checkOpen ();
	    if (!atInsertRow)
	    {
	    	resetColumnState();
	    }
	    else
	    {
		throw new SQLException("Cursor position on insert row");
	    }		
	    
	}

	//--------------------------------------------------------------------
	// moveToCurrentRow.
	// sets the rowPosition to the insert row.
	//--------------------------------------------------------------------

	public void moveToInsertRow()
		throws SQLException
	{

	    if (getType() == TYPE_FORWARD_ONLY) {
		throw new SQLException ("Invalid Cursor Type: " + getType());
	    }
	    
	    checkOpen ();
	
	    // 4627275
	    atInsertRow = true;
		
	    lastRowPosition = rowPosition;
	    lastBlockPosition = currentBlockCell;

	    if ( blockCursor )
		currentBlockCell = rowSet + 1;
	    	
	    resetInsertRow();
	}

	//--------------------------------------------------------------------
	// moveToCurrentRow.
	// sets the rowPosition back to the last valid row before inserts.
	//--------------------------------------------------------------------

	public void moveToCurrentRow()
		throws SQLException
	{	
		checkOpen ();
	    boolean done = false;
	        
	    if (atInsertRow)
	    {
		resetInsertRow();

		rowPosition = lastRowPosition;

		currentBlockCell = lastBlockPosition;

		done = absolute(rowPosition);

		if (done)
		{
		    lastRowPosition = 0;
		    lastBlockPosition = 0;
		}

		atInsertRow = false;
	    }
	}

	//--------------------------------------------------------------------
	// getStatement.
	// returns the ResultSet's owner Statement.
	//--------------------------------------------------------------------

	public Statement getStatement()
		throws SQLException
	{
		checkOpen ();
		if (ownerStatement != null)
		{
		    return ownerStatement;
		}
		else 
		    return null;
	}

	public Object getObject (
		int i,
		Map<String,Class<?>> map)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Ref getRef (
		int i)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Blob getBlob (
		int i)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Clob getClob (
		int i)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Array getArray (
		int i)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Object getObject (
		String colName,
		Map<String,Class<?>> map)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Ref getRef (
		String colName)
		throws SQLException
	{ throw new UnsupportedOperationException(); }


	public Blob getBlob (
		String colName)
		throws SQLException
	{ throw new UnsupportedOperationException(); }


	public Clob getClob (
		String colName)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

	public Array getArray (
		String colName)
		throws SQLException
	{ throw new UnsupportedOperationException(); }

    //--------------------------------------------------------------------
    // Jdbc 3.0 API Changes
    //--------------------------------------------------------------------

    public java.net.URL getURL(int i) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public java.net.URL getURL(String colName) throws SQLException {
	throw new UnsupportedOperationException();
    }
				
    public void updateRef(int i, java.sql.Ref ref) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateRef(String colName, java.sql.Ref ref) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateBlob(int i, java.sql.Blob blob) throws SQLException {
	throw new UnsupportedOperationException();
    }
    
    public void updateBlob(String colName, java.sql.Blob blob) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateClob(int i, java.sql.Clob clob) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateClob(String colName, java.sql.Clob clob) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateArray(int i, java.sql.Array array) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void updateArray(String colName, java.sql.Array array) throws SQLException {
	throw new UnsupportedOperationException();
    }

	//--------------------------------------------------------------------
	// getTimestamp w/ Calendar Object.
	//--------------------------------------------------------------------

	public Date getDate (
		int columnIndex, 
		Calendar cal) 
		throws SQLException
	{ 
		checkOpen ();
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;		       
		long lDateValue = 0L;

		// Re-map column if necessary

		columnIndex = mapColumn (columnIndex);

		// If a pseudo column exists, return a null

		if (getPseudoCol (columnIndex) != null) {
			lastColumnNull = true;
			return null;
		}

		lDateValue = getDataLongDate (columnIndex, cal);

		// If the column is null, return a null object

		if (lDateValue == 0L)
		{
			return null;
		}
		else

	      // Now convert to a Date object

		return new java.sql.Date(lDateValue);
	
	}

	public Date getDate ( 
		String columnName, 
		Calendar cal) 
		throws SQLException
	{	  
		return getDate (findColumn (columnName), cal);
	}


	//--------------------------------------------------------------------
	// getTime w/ Calendar Object.
	//--------------------------------------------------------------------

	public Time getTime ( 
		int columnIndex, 
		Calendar cal) 
		throws SQLException
	{ 
		checkOpen ();
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;
		long lValue = 0L;

		// Re-map column if necessary

		columnIndex = mapColumn (columnIndex);

		// If a pseudo columnIndex exists, return a null

		if (getPseudoCol (columnIndex) != null) {
			lastColumnNull = true;
			return null;
		}

		lValue = getDataLongTime (columnIndex, cal);

		// If the columnIndex is null, return a null object

		if (lValue == 0L) 
		{		 
			return null;
		}

		// Now convert to a Time object

		return new java.sql.Time(lValue);
	}

	public Time getTime ( 
		String columnName, 
		Calendar cal) 
		throws SQLException
	{	  
		return getTime (findColumn (columnName), cal); 
	}


	//--------------------------------------------------------------------
	// getTimestamp w/ Calendar Object.
	//--------------------------------------------------------------------

	public Timestamp getTimestamp ( 
		int columnIndex, 
		Calendar cal) 
		throws SQLException
	{ 
		checkOpen ();
		// Reset last warning message

		clearWarnings ();
		lastColumnNull = false;

		long lValue = 0L;

		// Re-map columnIndex if necessary

		columnIndex = mapColumn (columnIndex);

		// If a pseudo columnIndex exists, return a null

		if (getPseudoCol (columnIndex) != null) {
			lastColumnNull = true;
			return null;
		}

		lValue = getDataLongTimestamp (columnIndex, cal);

		// If the columnIndex is null, return a null object

		if (lValue == 0L) 
		{
			return null;
		}

		// Now convert to a Timestamp object
		else		    
			return new java.sql.Timestamp(lValue);
	}

	public Timestamp getTimestamp ( 
		String columnName, 
		Calendar cal) 
		throws SQLException
	{	 
		return getTimestamp (findColumn (columnName), cal); 
	}


	//====================================================================
	// Non-API methods
	//====================================================================

	//--------------------------------------------------------------------
	// setRowStatusPrt
	// sets the row status pointer to monitor updates within the rowSet.
	//--------------------------------------------------------------------

	protected void setRowStatusPtr()
		throws SQLException
	{
		checkOpen ();
		clearWarnings ();
			
		rowStatusArray = new int[rowSet + 1];
		
		pA = new long[2]; //4486684
		pA[0] = 0;
		pA[1] = 0;
		
		OdbcApi.SQLSetStmtAttrPtr (hStmt, 
				OdbcDef.SQL_ATTR_ROW_STATUS_PTR, rowStatusArray, 0, pA); //4486684

	}

	//--------------------------------------------------------------------
	// setRowArraySize
	// return true if rowSet Size was set to > 1 successfully.
	//--------------------------------------------------------------------

	protected boolean setRowArraySize()
	{
		int rowSize = 0;

	    
			try
			{							
				clearWarnings ();
				
				if (rowSet > 1)
				{

				    // the rowSet can not be biger than the
				    // number of rows in the resultSet.
				    if (numberOfRows < rowSet)
					rowSet = numberOfRows;

				    OdbcApi.SQLSetStmtAttr (hStmt, 
						    OdbcDef.SQL_ATTR_ROW_BIND_TYPE,
						    OdbcDef.SQL_ROW_BIND_BY_COLUMN, 0);

				    OdbcApi.SQLSetStmtAttr (hStmt,
						    OdbcDef.SQL_ATTR_ROW_ARRAY_SIZE, rowSet, 0);

				    rowSize = OdbcApi.SQLGetStmtAttr (hStmt,
						    OdbcDef.SQL_ATTR_ROW_ARRAY_SIZE);

				    if ( (rowSize > 1) && (rowSize < rowSet) )
				    {
					rowSet = rowSize;
					return true;
				    }		 

				}

			}
			catch (SQLException e)
			{
			       return false;
			}
		    
		// double check new settings
		if (rowSize == rowSet)
		{
	    	    return true;
		}
		else 
	    	    return false;
	}

	//--------------------------------------------------------------------
	// resetInsertRow
	// Clears the insert row for future use.
	//--------------------------------------------------------------------

	protected void resetInsertRow() throws SQLException
	{
		checkOpen ();
		int row = getRowIndex();	    
		
		if ( atInsertRow )
		{
		    // 4672508
		    for (int i = 0; i < numberOfCols; i++) {
		        boundCols[i].resetColumnToIgnoreData();
		    }
		}
	    
	}


	//--------------------------------------------------------------------
	// resetColumnState()
	// prepares the Column for next update. Change Ind. SQL_COLUMN_IGNORE
	//--------------------------------------------------------------------

	protected void resetColumnState()
		throws SQLException
	{
		checkOpen ();
		// free unbound columns before 
		// changing the length indicator.

		if (hStmt != OdbcDef.SQL_NULL_HSTMT) 
		{
			OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_UNBIND);		
		}

		for (int i = 0; i < numberOfCols; i++)
		{
		    boundCols[i].resetColumnToIgnoreData();
		}

	}

	//--------------------------------------------------------------------
	// bindCol
	// Given the SQL type and column #, Bind the column with the newly
	// stored array values and lengths.
	//--------------------------------------------------------------------

	protected void bindCol(int columnIndex, int sqlType) 
	    throws SQLException
	{
	    int scale = 0;
	    int rowBuffer = 0;
	    String strVal = null;
	    
	    //Bound array's values and lengths.
	    Object[] rowValues = boundCols[columnIndex - 1].getRowValues();
	    //4691886
	    byte[] rowLengths = boundCols[columnIndex - 1].getRowLengths();

	    if ( !blockCursor && atInsertRow )
	    {
		rowBuffer = 0;

		// move insert values to a valid row.
		// before adding the row.

		//4691886
		
		// 4672508
		/**
		 * Since SQL_COLUMN_IGNORE is to be used when we want to ignore
		 * the value of a column to be bound, which is the case when the
		 * data type is for example an auto increment column, the fix
		 * for 4627275 which uses SQL_NULL_DATA instead, is being removed. 
		 */

	    }

	    if (blockCursor)
	    {
		rowBuffer = currentBlockCell - 1;
	    }
	   
	    //Column properties.
	    Object x = boundCols[columnIndex - 1].getRowValue(rowBuffer);
	    int columnLen = boundCols[columnIndex - 1].getLength();
	    
	    if (columnLen < 0)
	    	  columnLen = getColumnLength (columnIndex);
	    //int theColumnLen = getColumnLength (columnIndex);
	    //if (columnLen < theColumnLen)
	    //	  columnLen = theColumnLen;

	    try {

		switch (sqlType)
		{
		    case Types.CHAR:
		    case Types.VARCHAR:
		    case Types.NUMERIC:
		    case Types.DECIMAL:

				if (sqlType == Types.NUMERIC || sqlType == Types.DECIMAL)
				{
				    if (x != null)
				    {
					Object decObj = (Object)x;
					String strObj = decObj.toString();
					columnLen = strObj.length();
					
					BigDecimal dec = new BigDecimal(strObj);
					int decimals = dec.scale();
					
					// conpensate for a decimal point.
					if (decimals <= 0)
					    columnLen++;

				    }

				}

				bindStringCol(columnIndex, sqlType, 
					      rowValues, rowLengths, columnLen);

			    break;

		    case Types.LONGVARCHAR:

				 // check if the Array contain Stream Data.
				int typeValue = ownerStatement.getTypeFromObject(x);

				if ( (typeValue == Types.LONGVARBINARY) || (typeValue == Types.NULL)  )
				{
					bindAtExecCol (columnIndex, sqlType, rowLengths);
				}
				else if ( typeValue == Types.BINARY )
				{
					bindBinaryCol(columnIndex, rowValues, rowLengths, columnLen);
				}
				break;


		    case Types.BIT:
		    case Types.TINYINT:
		    case Types.SMALLINT:
		    case Types.INTEGER:
				
				bindIntegerCol(columnIndex, rowValues, rowLengths);
				break;

		    case Types.DOUBLE:

				bindDoubleCol(columnIndex, rowValues, rowLengths);
				break;

		    case Types.BIGINT:
		    case Types.FLOAT:
		    case Types.REAL:
				
				bindFloatCol(columnIndex, rowValues, rowLengths);
				break;

		    case Types.DATE:

				bindDateCol(columnIndex, rowValues, rowLengths);
				break;

		    case Types.TIME:

				bindTimeCol(columnIndex, rowValues, rowLengths);
				break;

		    case Types.TIMESTAMP:

				bindTimestampCol(columnIndex, rowValues, rowLengths);
				break;


		    case Types.BINARY:
		    case Types.VARBINARY:

				bindBinaryCol(columnIndex, rowValues, rowLengths, columnLen);				    
				break;

		    case Types.LONGVARBINARY:

				bindAtExecCol(columnIndex, sqlType, rowLengths);
				break;

		}//end of Bind Array!

	    }
	    catch (SQLException se) 
	    {		
		throw new SQLException ("SQLBinCol (" + columnIndex + ") SQLType = " +  sqlType + ". " +  se.getMessage());
	    }

	}

	//--------------------------------------------------------------------
	// bindStringCol
	// Binds the column index with a String.
	//--------------------------------------------------------------------

	protected void bindStringCol(
	    int columnIndex,
	    int sqlType,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths,
	    int columnLen)
	    throws SQLException
	{
	    byte[] dataBuf = boundCols[columnIndex-1].allocBindDataBuffer((columnLen + 1) * (rowValues.length));
				
	    long[] strBuffers = new long[4];
	    strBuffers[0]=0;
	    strBuffers[1]=0;
	    strBuffers[2]=0;
	    strBuffers[3]=0;

	    OdbcApi.SQLBindColString (hStmt, columnIndex, 
					sqlType, rowValues, columnLen,
					rowLengths, dataBuf, strBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=strBuffers[0];
	    boundCols[columnIndex - 1].pA2=strBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=strBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=strBuffers[3]; // 4638528

	}

	//--------------------------------------------------------------------
	// bindIntegerCol
	// Binds the column index with an Integer.
	//--------------------------------------------------------------------

	protected void bindIntegerCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{
	    byte[] intDataBuf = boundCols[columnIndex-1].allocBindDataBuffer(4 * (rowValues.length));

	    long[] iBuffers = new long[4]; // 4638528
	    iBuffers[0]=0;
	    iBuffers[1]=0;
	    iBuffers[2]=0; // 4638528
	    iBuffers[3]=0; // 4638528

	    OdbcApi.SQLBindColInteger (
	    		hStmt, columnIndex, rowValues,
			rowLengths, intDataBuf, iBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=iBuffers[0];
	    boundCols[columnIndex - 1].pA2=iBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=iBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=iBuffers[3]; // 4638528	    

	}


	//--------------------------------------------------------------------
	// bindFloatCol
	// Binds the column index with a Float.
	//--------------------------------------------------------------------

	protected void bindFloatCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{
	    byte[] fDataBuf = boundCols[columnIndex-1].allocBindDataBuffer(8 * (rowValues.length));

	    long[] fBuffers = new long[4]; // 4638528
	    fBuffers[0]=0;
	    fBuffers[1]=0;
	    fBuffers[2]=0; // 4638528
	    fBuffers[3]=0; // 4638528
	    
	    OdbcApi.SQLBindColFloat (
	    		hStmt, columnIndex, rowValues,
	    		rowLengths, fDataBuf, fBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=fBuffers[0];
	    boundCols[columnIndex - 1].pA2=fBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=fBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=fBuffers[3]; // 4638528
	}

	//--------------------------------------------------------------------
	// bindDoubleCol
	// Binds the column index with a Double.
	//--------------------------------------------------------------------

	protected void bindDoubleCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{

	    byte[] dDataBuf = boundCols[columnIndex-1].allocBindDataBuffer(8 * (rowValues.length));

	    long[] dBuffers = new long[4]; // 4638528
	    dBuffers[0]=0;
	    dBuffers[1]=0;
	    dBuffers[2]=0; // 4638528
	    dBuffers[3]=0; // 4638528	

	    OdbcApi.SQLBindColDouble (
	    		hStmt, columnIndex, rowValues,
	    		rowLengths, dDataBuf, dBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=dBuffers[0];
	    boundCols[columnIndex - 1].pA2=dBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=dBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=dBuffers[3];	// 4638528    

	}

	//--------------------------------------------------------------------
	// bindDateCol
	// Binds the column index with a Date.
	//--------------------------------------------------------------------

	protected void bindDateCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{
	    // allocate a (10 * arraySize) byte array;
	    byte[] dtBuf = boundCols[columnIndex-1].allocBindDataBuffer(10 * (rowValues.length));

	    long[] dtBuffers = new long[4]; // 4638528
	    dtBuffers[0]=0;
	    dtBuffers[1]=0;
	    dtBuffers[2]=0; // 4638528
	    dtBuffers[3]=0; // 4638528
	    			    				
	    OdbcApi.SQLBindColDate (hStmt, columnIndex, rowValues, rowLengths, dtBuf, dtBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=dtBuffers[0];
	    boundCols[columnIndex - 1].pA2=dtBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=dtBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=dtBuffers[3]; // 4638528	    

	}

	//--------------------------------------------------------------------
	// bindTimeCol
	// Binds the column index with Time Object.
	//--------------------------------------------------------------------

	protected void bindTimeCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{
	    // allocate a ((8 + 1) * arraySize) byte array;
	    byte[] tmBuf = boundCols[columnIndex-1].allocBindDataBuffer(9 * (rowValues.length));

	    long[] tmBuffers = new long[4];// 4638528
	    tmBuffers[0]=0;
	    tmBuffers[1]=0;
	    tmBuffers[2]=0; // 4638528
	    tmBuffers[3]=0; // 4638528
	    				
	    OdbcApi.SQLBindColTime (hStmt, columnIndex, rowValues, rowLengths, tmBuf, tmBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=tmBuffers[0];
	    boundCols[columnIndex - 1].pA2=tmBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=tmBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=tmBuffers[3]; // 4638528
	}

	//--------------------------------------------------------------------
	// bindTimestampCol
	// Binds the column index with a Timestamp.
	//--------------------------------------------------------------------

	protected void bindTimestampCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths)
	    throws SQLException
	{
	    // allocate a ((29 + 1) * arraySize) byte array;
	    byte[] stmpBuf = boundCols[columnIndex-1].allocBindDataBuffer(30 * (rowValues.length));

	    long[] stmpBuffers = new long[4];// 4638528
	    stmpBuffers[0]=0;
	    stmpBuffers[1]=0;
	    stmpBuffers[2]=0; // 4638528
	    stmpBuffers[3]=0; // 4638528
	    				
	    OdbcApi.SQLBindColTimestamp (hStmt, columnIndex, rowValues, rowLengths, stmpBuf, stmpBuffers);

	    //save the native pointers from Garbage Collection
	    boundCols[columnIndex - 1].pA1=stmpBuffers[0];
	    boundCols[columnIndex - 1].pA2=stmpBuffers[1]; 
	    boundCols[columnIndex - 1].pC1=stmpBuffers[2]; // 4638528
	    boundCols[columnIndex - 1].pC2=stmpBuffers[3]; // 4638528
	}

	//--------------------------------------------------------------------
	// bindBinaryCol
	// Binds the column index with BINARY data.
	//--------------------------------------------------------------------

	protected void bindBinaryCol(
	    int columnIndex,
	    Object[] rowValues,
	    //4691886
	    byte[] rowLengths,
	    int columnLen)
	    throws SQLException
	{
	    
	    byte[] byteBuf = boundCols[columnIndex-1].allocBindDataBuffer((columnLen + 1) * (rowValues.length));

	    long bBuffers[]=new long[4];

	    bBuffers[0]=0;
	    bBuffers[1]=0;
	    bBuffers[2]=0;
	    bBuffers[3]=0;

		
	    OdbcApi.SQLBindColBinary (	hStmt, columnIndex,
					rowValues, rowLengths, 
					columnLen, byteBuf, bBuffers);

	    //Save the pointers from the trash
	    boundCols[columnIndex - 1].pA1=bBuffers[0];
	    boundCols[columnIndex - 1].pA2=bBuffers[1]; 
	    // Fix 4531124. Changing from pB to pC to effect ReleaseStoredIntegers.
	    boundCols[columnIndex - 1].pC1=bBuffers[2];
	    boundCols[columnIndex - 1].pC2=bBuffers[3]; 

	}

	//--------------------------------------------------------------------
	// bindAtExecCol
	// Binds the column index with LONGVARCHAR or LONGVARBINARY data.
	//--------------------------------------------------------------------

	protected void bindAtExecCol(
		int columnIndex,
		int sqlType,
		//4691886
		byte[] rowLengths)
		throws SQLException
	{			

		// Allocate a new buffer for the column data.  This buffer
		// will be returned by "SQLParamData" (it is set to the current
		// row, a 4-byte integer)
		byte dataBuf[] = boundCols[columnIndex-1].allocBindDataBuffer(4);
		
                // Bind the column with SQL_LEN_DATA_AT_EXEC

                long buffers[]=new long[4];

                buffers[0]=0;
                buffers[1]=0;
                buffers[2]=0;
                buffers[3]=0;

		OdbcApi.SQLBindColAtExec (hStmt, columnIndex,
					    sqlType, rowLengths, dataBuf, buffers);

		//save the native pointers from Garbage Collection
		boundCols[columnIndex - 1].pA1=buffers[0];
		boundCols[columnIndex - 1].pA2=buffers[1]; 
		boundCols[columnIndex - 1].pC1=buffers[2]; // 4638528
		boundCols[columnIndex - 1].pC2=buffers[3]; // 4638528
			
	}


	//--------------------------------------------------------------------
	// setPos
	// Given the column and attribute type, update the row values.
	//--------------------------------------------------------------------

	protected void setPos(int row, int action)
		throws SQLException
	{

		SQLWarning	warning = null;
		boolean needData = false;

		try 
		{
			// Reset last warning message
			clearWarnings ();

			needData = OdbcApi.SQLSetPos (hStmt, row,
							action,	OdbcDef.SQL_LOCK_NO_CHANGE);

			// Now loop while more data is needed (i.e. a data-at-
			// execution parameter was given).  For each row
			// that needs data, put the data from the input stream.

			// Get the row number that requires data
			int columnIndex = 0;

			while (needData) {

				int rowPos = getRowIndex();

				String driverName = OdbcApi.odbcDriverName;

				if ( blockCursor && (driverName.indexOf("(IV") == -1) )
				{
					columnIndex = OdbcApi.SQLParamDataInBlock(hStmt, rowPos);
				}
				else
					columnIndex = OdbcApi.SQLParamData (hStmt);
			
				// If the row index is -1, there is no
				// more data required

				if (columnIndex == -1) {
					needData = false;
				}
				else 
				{	
				    // Now we have the proper parameter.
				    // get the data from the stored input stream array 
				    // and set it as the InputStream to SQLPutData.

				    putColumnData (columnIndex);
				}
			}


		}
		catch (SQLWarning ex) {

			// Save pointer to warning and save it.
			warning = ex;
		}
		catch (SQLException se) {
			
		    // ignore exception.
		    throw new SQLException (se.getMessage());
		}

	}


	//--------------------------------------------------------------------
	// putColumnData
	// Put InputStream data for the current row being updated.  The
	// InputStream was bound using SQL_LEN_DATA_AT_EXEC(length) macro.
	//--------------------------------------------------------------------
	
	protected void putColumnData (
		int columnIndex)
		throws SQLException, JdbcOdbcSQLWarning
	{
		// We'll transfer up to maxLen at a time
		int	maxLen = JdbcOdbcLimits.MAX_PUT_DATA_LENGTH;
		int	bufLen;
		int	realLen;
		byte	buf[] = new byte[maxLen];
		boolean	endOfStream = false;

		// Sanity check the column index
		if ((columnIndex < 1) ||
		    (columnIndex > numberOfCols)) {

			if (OdbcApi.getTracer().isTracing ()) {
				OdbcApi.getTracer().trace ("Invalid index for putColumnData()");
			}
			return;
		}
		
		java.io.InputStream inputStream = null;
		
		// get the current row Position.
		int row = getRowIndex();

		// Get the information about the input stream
		try
		{
		    inputStream = (InputStream) boundCols[columnIndex - 1].getRowValue(row);
		}
		catch (Exception e)
		{
		    throw new SQLException ("Invalid data for columnIndex(" 
					    + columnIndex 
					    + "): "
					    + e.getMessage());
		}

		
		int inputStreamLen = boundCols[columnIndex - 1].getLength ();
				
		int inputStreamType = boundCols[columnIndex - 1].getStreamType ();


		// Loop while more data from the input stream

		while (!endOfStream) {

			// Read some data from the input stream

			try {
				if (OdbcApi.getTracer().isTracing ()) {
					OdbcApi.getTracer().trace ("Reading from input stream"); 
				}
				bufLen = inputStream.read (buf);
				if (OdbcApi.getTracer().isTracing ()) {
					OdbcApi.getTracer().trace ("Bytes read: " + bufLen);
				}
			}
			catch (java.io.IOException ex) {

				// If an I/O exception was generated, turn
				// it into a SQLException

				throw new SQLException (ex.getMessage ());
			}

			// -1 as the number of bytes read indicates that
			// there is no more data in the input stream

			if (bufLen == -1) {

				// Sanity check to ensure that all the data we said we
				// had was read.  If not, raise an exception

				if (inputStreamLen != 0) {
					throw new SQLException ("End of InputStream reached before satisfying length specified when InputStream was set");
				}
				endOfStream = true;
				break;
			}
			
			// If we got more bytes than necessary, truncate
			// the buffer by re-setting the buffer length.  Also,
			// indicate that we don't need to read any more.

			if (bufLen > inputStreamLen) {
				bufLen = inputStreamLen;
				endOfStream = true;
			}

			realLen = bufLen;
			
			// For UNICODE streams, strip off the high byte and set the
			// number of actual bytes present.  It is assumed that
			// there are 2 bytes present for every UNICODE character - if
			// not, then that's not our problem


/*
			// This has already been acomplished. in updateCharacterStream ();		
			if (inputStreamType == JdbcOdbcBoundParam.UNICODE) {
				realLen = bufLen / 2;

				for (int ii = 0; ii < realLen; ii++) {
					buf[ii] = buf[(ii * 2) + 1];
				}
			}
*/

			// Put the data

			try
			{

			    OdbcApi.SQLPutData (hStmt, buf, realLen);
			}
			catch (SQLWarning sw)
			{
			    setWarning(sw);
			}
			catch (SQLException se)
			{
			    //ignore exception for now.
			}

			// Decrement the number of bytes still needed

			inputStreamLen -= bufLen;

			if (OdbcApi.getTracer().isTracing ()) {
				OdbcApi.getTracer().trace ("" + inputStreamLen + " bytes remaining");
			}

			// If there is no more data to be read, exit loop

			if (inputStreamLen == 0) {
				endOfStream = true;
			}
		}
	}


	//--------------------------------------------------------------------
	// getColAttribute
	// Given the column and attribute type, return the attribute value
	//--------------------------------------------------------------------

	public int getColAttribute (
		int column,
		int type)
		throws SQLException
	{
		int	value = 0;

		// Reset last warning message

		clearWarnings ();

		try {
			value = OdbcApi.SQLColAttributes (hStmt,
					column, type);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			value = n.intValue ();

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		return value;
	}


	//--------------------------------------------------------------------
	// getMaxCharLen
	// Returns the length of the column as needed for a character
	// representation
	//--------------------------------------------------------------------

	protected int getMaxCharLen (
			int column)
			throws SQLException
	{
		int colType = getColumnType (column);
		int maxLen = getColumnLength (column);
		
		if (maxLen != OdbcDef.SQL_NULL_DATA) {

			// Adjust for certain data types

			switch (colType) {
			case Types.BINARY:
			case Types.VARBINARY:
			case Types.LONGVARBINARY:

				// Adjust binary types being displayed
				// as character.  Each binary byte takes
				// two characters to display

				maxLen *= 2;
				break;
			case Types.DATE:
				maxLen = 10;
				break;
			case Types.TIME:
				maxLen = 8;
				break;
			case Types.TIMESTAMP:
				maxLen = 29;
				break;
			case Types.NUMERIC:
			case Types.DECIMAL:
				maxLen += 2;
				break;
			case Types.BIT:
				maxLen = 1;
				break;
			case Types.TINYINT:
				maxLen = 4;
				break;
			case Types.SMALLINT:
				maxLen = 6;
				break;
			case Types.INTEGER:
				maxLen = 11;
				break;
			case Types.BIGINT:
				maxLen = 20;
				break;
			case Types.REAL:
				maxLen = 13;
				break;
			case Types.FLOAT:
			case Types.DOUBLE:
				maxLen = 22;
				break;
			}


			// Sanity check the length.  If invalid, set to max

			if ((maxLen <= 0) ||
			(maxLen > JdbcOdbcLimits.MAX_GET_DATA_LENGTH)) {
				maxLen = JdbcOdbcLimits.MAX_GET_DATA_LENGTH;
			}
		}
		return maxLen;
	}

	//--------------------------------------------------------------------
	// getMaxBinaryLen
	// Returns the length of the column as needed for a binary
	// representation
	//--------------------------------------------------------------------

	protected int getMaxBinaryLen (
			int column)
			throws SQLException
	{
		int maxLen = getColumnLength (column);

		if (maxLen != OdbcDef.SQL_NULL_DATA) {

			// Sanity check the length.  If invalid, set to max

			if ((maxLen <= 0) ||
			(maxLen > JdbcOdbcLimits.MAX_GET_DATA_LENGTH)) {
				maxLen = JdbcOdbcLimits.MAX_GET_DATA_LENGTH;
			}
		}
		return maxLen;
	}

	//--------------------------------------------------------------------
	// getDataDouble
	// Returns the data for the given column as a Double object.  Returns
	// null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public Double getDataDouble (
		int column)
		throws SQLException
	{
		Double n;
		lastColumnNull = false;
		try {
			n = OdbcApi.SQLGetDataDouble (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			n = (Double) ex.value;

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (n == null) {
			lastColumnNull = true;
		}
		return n;
	}

	//--------------------------------------------------------------------
	// getDataFloat
	// Returns the data for the given column as a Float object.  Returns
	// null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public Float getDataFloat (
		int column)
		throws SQLException
	{
		Float n;
		lastColumnNull = false;
		try {
			n = OdbcApi.SQLGetDataFloat (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			n = (Float) ex.value;

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (n == null) {
			lastColumnNull = true;
		}
		return n;
	}

	//--------------------------------------------------------------------
	// getDataInteger
	// Returns the data for the given column as an Integer object.  Returns
	// null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public Integer getDataInteger (
		int column)
		throws SQLException
	{

		Integer n;
		lastColumnNull = false;
		try {
			n = OdbcApi.SQLGetDataInteger (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			n = (Integer) ex.value;

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (n == null) {
			lastColumnNull = true;
		}
		else if (column == sqlTypeColumn) {
		
			// If we are fetching data that is a catalog SQL type column,
			// we need to convert it to a JDBC SQL type.

			n = new Integer (OdbcDef.odbcTypeToJdbc (n.intValue()));
		}
		return n;
	}

	//--------------------------------------------------------------------
	// getDataLong
	// Returns the data for the given column as a Long object.  Returns
	// null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public Long getDataLong (
		int column)
		throws SQLException
	{
		Long n = null;
		Double d = getDataDouble (column);
		
		if (d != null) {
			n = new Long (d.longValue ());
		}
		return n;
	}


	//--------------------------------------------------------------------
	// getDataString
	// Returns the data for the given column as an String object.  Returns
	// null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public String getDataString (
		int column,
		int maxLen,
		boolean trimSpaces)
                throws SQLException
	{
		String s;
		lastColumnNull = false;
		try {
			s = OdbcApi.SQLGetDataString (hStmt, column, maxLen,
						trimSpaces);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (s == null) {
			lastColumnNull = true;
		}
		else if (column == sqlTypeColumn) {
		
			// If we are fetching data that is a catalog SQL type column,
			// we need to convert it to a JDBC SQL type.  If an exception
			// is thrown during the conversion between a String a int,
			// ignore it and return the original string
			
			try {
				int colType = OdbcDef.odbcTypeToJdbc (
							(Integer.valueOf(s)).intValue());
				s = "" + colType;
			}
			catch (Exception ex) {
			}
		}
		return s;
	}

	//--------------------------------------------------------------------
	// getDataStringDate
	// Returns the data for the given column as an String Date object
	// (yyyy-mm-dd).  Returns null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public String getDataStringDate (
		int column)
		throws SQLException
	{
		String s;
		lastColumnNull = false;
		try {
			s = OdbcApi.SQLGetDataStringDate (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (s == null) {
			lastColumnNull = true;
		}
		return s;
	}

	//--------------------------------------------------------------------
	// getDataStringTime
	// Returns the data for the given column as an String Time object
	// (hh:mm:ss).  Returns null if the value for the column was NULL.
	//--------------------------------------------------------------------

	public String getDataStringTime (
		int column)
		throws SQLException
	{
		String s;
		lastColumnNull = false;
		try {
			s = OdbcApi.SQLGetDataStringTime (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (s == null) {
			lastColumnNull = true;
		}
		return s;
	}

	//--------------------------------------------------------------------
	// getDataStringTimestamp
	// Returns the data for the given column as an String Timestamp object
	// (yyyy-mm-dd hh:mm:ss).  Returns null if the value for the column
	// was NULL.
	//--------------------------------------------------------------------

	public String getDataStringTimestamp (
		int column)
		throws SQLException
	{
		String s;
		lastColumnNull = false;
		try {
			s = OdbcApi.SQLGetDataStringTimestamp (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		if (s == null) {
			lastColumnNull = true;
		}
		return s;
	}

	//--------------------------------------------------------------------
	// getDataLongDate
	// Returns the data for the given column as an long converted from a 
	// String (yyyy-mm-dd) Date object. Returns null if the value for the 
	// column was NULL.
	//--------------------------------------------------------------------

	public long getDataLongDate (
		int column,
		Calendar cal)
		throws SQLException
	{
		String s;
		lastColumnNull = false;
		java.util.Date tmpDate = null;
		long lDate = 0L;
		
		try {
			s = OdbcApi.SQLGetDataStringDate (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		
		// make the string into a Date object to be set with
		// the desire Calendar's timeZone/locale.
		// once set, return the long value of the new Date
		// in milliseconds back to getDate (idx, Calendar). 
		
		if (s != null)
		{
			  tmpDate = Date.valueOf(s);	
			  // Fix 4380653		    
			  lDate = utils.convertFromGMT(tmpDate, cal);		
		}
		else if (s == null) 
		{
			lastColumnNull = true;
		}
		
		
		return lDate;
	}

	//--------------------------------------------------------------------
	// getDataLongTime
	// Returns the data for the given column as an long converted from 
	// String (hh:mm:ss) Time object. Returns null if the value for the 
	// column was NULL.
	//--------------------------------------------------------------------

	public long getDataLongTime (
		int column, 
		Calendar cal) 
		throws SQLException
	{
		String s;
		lastColumnNull = false;		       
		java.util.Date tmpTime = null;
		
		long lTime = 0L;
		
		try {
			s = OdbcApi.SQLGetDataStringTime (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}

		if (s != null)
		{
			  tmpTime = Time.valueOf(s);
			  // Fix 4380653		    			  
			  lTime = utils.convertFromGMT(tmpTime ,cal);		
		}		 
		else if (s == null)
		{
			lastColumnNull = true;
		}
		return lTime;
	}


	
	//--------------------------------------------------------------------
	// getDataLongTimestamp
	// Returns the data for the given column as an long converted from
	// a String (yyyy-mm-dd hh:mm:ss) Timestamp object. Returns null if the
	// value for the column was NULL.
	//--------------------------------------------------------------------

	public long getDataLongTimestamp ( 
		int column, 
		Calendar cal) 
		throws SQLException
	{
		String s;
		lastColumnNull = false;
		java.util.Date tmpTS = null;
		
		long lTS = 0L;
		
		try {
			s = OdbcApi.SQLGetDataStringTimestamp (hStmt, column);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was 
			// saved off in the warning object.  Get it
			// back and set the return value.

			s = (String) ex.value;
			setWarning (JdbcOdbc.convertWarning (ex));
		}
		
		if (s != null)
		{
			  tmpTS = Timestamp.valueOf(s);     
			  // Fix 4380653		    			      					  
			  lTS = utils.convertFromGMT(tmpTS, cal);			
		}				 
		else if (s == null)
		{
			lastColumnNull = true;
		}
		
			    
		return lTS;
	}
	
	//--------------------------------------------------------------------
	// getColumnLength
	// Returns the length of the given column number.
	//--------------------------------------------------------------------

	public int getColumnLength (
		int column)
		throws SQLException
	{
		int colLen = -1;

		// As an optimization, we'll cache the length into
		// the column array.

		if ((column > 0) &&
		    (column <= numberOfCols)) {
		    
			// Get the column length from the column array

			colLen = boundCols[column - 1].getLength ();

		}

		// If we don't know the length, ask the data source

		if (colLen == -1) {
			colLen = getColAttribute (column, 
				OdbcDef.SQL_COLUMN_LENGTH);
			
			// Cache the length into the column array

			if ((column > 0) &&
				(column <= numberOfCols)) {
				boundCols[column - 1].setLength (colLen);
			}
		}
		return colLen;
	}

	//--------------------------------------------------------------------
	// getScale
	// Number of digits to right of decimal
	//--------------------------------------------------------------------

	public int getScale (
		int column)
		throws SQLException
	{
		int value;

		// If a pseudo column exists for this column, return 0

		if (getPseudoCol (column) != null) {
			lastColumnNull = true;
			value = 0;
		}
		else {
			value = getColAttribute (column,
				OdbcDef.SQL_COLUMN_SCALE);
		}       	
		return value;

	}

	//--------------------------------------------------------------------
	// getColumnType
	// Returns the Java SQL type of the given column number.
	//--------------------------------------------------------------------

	public int getColumnType (
		int column)
		throws SQLException
	{
		int colType = OdbcDef.SQL_TYPE_UNKNOWN;

		// As an optimization, we'll cache the type information into
		// the column array.

		if ((column > 0) &&
		    (column <= numberOfCols)) {
		    
			// Get the column type from the column array

			colType = boundCols[column - 1].getType ();

		}

		// If we don't know the type, ask the data source

		if (colType == OdbcDef.SQL_TYPE_UNKNOWN) {
			
			colType = getColAttribute (column, 
				OdbcDef.SQL_COLUMN_TYPE);
			
			// Convert it to the proper JDBC SQL type

			colType = OdbcDef.odbcTypeToJdbc (colType);
		
			// Cache the type into the column array
			if ((column > 0) &&
				(column <= numberOfCols)) {
				boundCols[column - 1].setType (colType);
			}
		}

		return colType;
	}

	//--------------------------------------------------------------------
	// setPseudoCols
	// Set the pseudo column information
	//--------------------------------------------------------------------

	public void setPseudoCols (
		int first,
		int last,
		JdbcOdbcPseudoCol pc[])
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("Setting pseudo columns, first=" +
				first + ", last=" + last);
		}
		firstPseudoCol = first;
		lastPseudoCol = last;
		pseudoCols = pc;
	}

	//--------------------------------------------------------------------
	// getPseudoCol
	// Given a column number, return the corresponding pseudo column,
	// null if one does not exist for this column
	//--------------------------------------------------------------------

	public JdbcOdbcPseudoCol getPseudoCol (
		int column)
	{
		JdbcOdbcPseudoCol pc = null;

		// If a pseudo column exists for this column, return
		// it from the array.  It is assumed that the pseudo
		// column array contains an element for each column between
		// (and including) the first and last pseudo column
		// specified.

		if ((column > 0) &&
		    (column >= firstPseudoCol) &&
		    (column <= lastPseudoCol)) {
			pc = pseudoCols[column - firstPseudoCol];
		}
		return pc;
	}

	//--------------------------------------------------------------------
	// setSQLTypeColumn
	// Some catalogs return SQL data types in ODBC format.  We need to
	// flag them so that they can be converted into Java SQL types.
	//--------------------------------------------------------------------

	public void setSQLTypeColumn (
		int column)
	{
		sqlTypeColumn = column;
	}
		
	//--------------------------------------------------------------------
	// setInputStream
	// Saves a reference to the newly allocated InputStream.  For each
	// InputStream that is created for the statement, a reference is
	// kept.  If the Statement is closed or the cursor is moved, the
	// InputStream is invalidated.  Once invalidated, if an application
	// attempts to read from the InputStream, an exception is raised.
	//--------------------------------------------------------------------

	protected void setInputStream (
		int column,
		JdbcOdbcInputStream inStream)
	{
		// Sanity check the column number 

		if ((column > 0) &&
		    (column <= numberOfCols)) {

			// Save the input stream in the bound column array

			boundCols[column - 1].setInputStream (inStream);
		}
	}

	//--------------------------------------------------------------------
	// closeInputStreams
	// Close (invalidate) any InputStreams that are currently in use
	// for the statement
	//--------------------------------------------------------------------

	protected void closeInputStreams ()
	{
		// Close all input streams

		for (int i = 0; i < numberOfCols; i++) {
			boundCols[i].closeInputStream ();
		}
	}

	//--------------------------------------------------------------------
	// setColumnMappings
	// In order to facilitate re-numbering/re-ordering result set columns,
	// an array of integers can be used.  If set, the length of the
	// integer array is the new number of columns in the result set, and
	// each of the elements re-maps the column number (i.e. element 0
	// of the array points to the first column number).  For example,
	// DatabaseMetaData.getTableTypes() needs to return only 1 column
	// in the result set.  The results from the ODBC call getTables returns
	// multiple columns, and the column needed is column number 4.  Thus,
	// once the result set is created:
	//
	//	int	map[] = new int[1];
	//	map[0] = 4;
	//	rs.setColumnMappings (map);
	//
	// This will set the number of result set columns to 1 (the size of
	// the mapping array), and column number 1 (the first element in the
	// array) will use column number 4 from the result set data.
	//--------------------------------------------------------------------

	public void setColumnMappings (
		int	map[])
	{
		colMappings = map;
	}

	//--------------------------------------------------------------------
	// mapColumn
	// Given a column number, map it to the corresponding column in
	// the result set.  If no column mappings exist, return the original
	// column number.  If the column number is out-of-range, return -1.
	//--------------------------------------------------------------------

	public int mapColumn (
		int	column)
	{
		int	map = column;

		if (colMappings != null) {

			// Validate column number

			if ((column > 0) &&
			    (column <= colMappings.length)) {
				map = colMappings[column - 1];
			}
			else {
				// Invalid column number
				map = -1;
			}
		}

		return map;
	}

	//--------------------------------------------------------------------
	// calculateRowCount
	// Get row count of result set by trying the following methods:
	// 1) SQLRowCount
	// 2) Move cursor the last row, then calll SQLGetStmtOption with 
	//    SQL_ROW_NUMBER
	// 3) Execute query "select count(*) ..."
	//--------------------------------------------------------------------

	protected void calculateRowCount ()
		throws SQLException
	{

		// First, try SQLRowCount
		try {
			numberOfRows = OdbcApi.SQLRowCount (hStmt);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			numberOfRows = n.intValue ();
		}

		if (numberOfRows > 0) return;

		// Then, try to move cursor to the last row and query the row number

		    try
		    {

			OdbcApi.SQLFetchScroll(
					hStmt,
					OdbcDef.SQL_FETCH_LAST,
					0);
					

			/* SQLGetStmtOption can return a pointer
			 * value in V3.0 but here it is a SQLINTEGER
			 */
			
			numberOfRows = (int)OdbcApi.SQLGetStmtOption(
					hStmt,
					OdbcDef.SQL_ROW_NUMBER);
					

		    }
		    catch (SQLException e)
		    {
			// 3 posible causes:
			// a.FetchScroll not supported. (bad)
			// b.posible empty ResultSet. (ok)
			// c.get option Row_Number not supported. (ok)

			// If an exception is thrown, ignore it,
			// but try reset to the beforeFirst row
			// in case of ("c") above.
		    }
		    finally
		    {
			// reset to the beforeFirst row.
			OdbcApi.SQLFetchScroll(
					hStmt,
					OdbcDef.SQL_FETCH_ABSOLUTE,
					0);
		    }

		if (numberOfRows > 0)
		{
		    // Now that we have the numberOfRows,
		    // set it back to the beforeFirst row.
		    OdbcApi.SQLFetchScroll(
				    hStmt,
				    OdbcDef.SQL_FETCH_ABSOLUTE,
				    0);

		    return;
		}

		// At last, execute "select count(*) ..." statement to get row count
		if (ownerStatement != null)
		{
		    Connection ownerConn = ownerStatement.getConnection ();		    

		    String oldSql = ownerStatement.getSql ();
		    
		    String newSQLstr = null;

		    String statementName = ownerStatement.getClass().getName();

		    int ParameterCount = 0;
		    
		    ParameterCount = ownerStatement.getParamCount();
		    
		    if (ParameterCount > 0)
		    {
			if ( statementName.indexOf("CallableStatement") > 0 )
			{						    
			    throw new SQLException("Unable to obtain result set row count. From " + oldSql);
			}
			else if ( statementName.indexOf("PreparedStatement") > 0 )
			{
			    //4672368
			    if(oldSql.toLowerCase().indexOf("select") == -1) {
			        throw new SQLException("Cannot obtain result set row count for " + oldSql);
			    }
			    newSQLstr = reWordAsCountQuery(oldSql);
			    
			    if (newSQLstr.indexOf("?") > 0)
			    {
				numberOfRows = parameterQuery(ownerConn.prepareStatement (newSQLstr));
			    }
			    else
				ParameterCount = 0;

			}
		    }
		    
		    if (statementName.indexOf("Statement") > 0 && (ParameterCount == 0))
		    {
			Statement stmt = ownerConn.createStatement ();

			if (oldSql != null && oldSql.startsWith("SELECT"))
			{			
			    if (newSQLstr == null)
				newSQLstr = reWordAsCountQuery(oldSql);

			    ResultSet rs = stmt.executeQuery (newSQLstr);
			    
			    rs.next ();		
			    
			    // here's what we need.
			    numberOfRows = rs.getInt (1);

			    // if a row count was achived w/o a count(*),
			    // then ther row count is the result of a 
			    // function that returns one row. However, 
			    // function queries are not considered updatable.
			    if ( (newSQLstr.indexOf ("COUNT(*)") < 0) && numberOfRows > 0 )
			    {
				    numberOfRows = 1;
				    setWarning (new SQLWarning ("ResultSet is not updatable."));			
			    }
			    
			}

			if (stmt != null) 
			    stmt.close();
		    }


		    
		}
		
		if (numberOfRows > 0)
			return;
		else
			setWarning (new SQLWarning ("Can not determine result set row count."));			
			//throw new SQLException("Can not determine result set row count.");
	}


	//--------------------------------------------------------------------
	// parameterQuery.
	// similar to reWordAsCountQuery. However, values for the statement
	// that created the ResultSet may be to big to recreate as a string.
	// There fore, values must be obtained from the bound parameters.
	//--------------------------------------------------------------------

	protected int parameterQuery(PreparedStatement paramStmt)
		throws SQLException
	{
		int count = 0;

		Object[] paramObjs = null;
		int[]	paramTypes = null;
	    			
		if (paramStmt != null)
		{
		   try
		   { 
	    		paramObjs  = ownerStatement.getObjects();
			paramTypes = ownerStatement.getObjectTypes();

			for (int i = 0; i < paramObjs.length; i++)
			{
			    paramStmt.setObject(i + 1, paramObjs[i], paramTypes[i]);
			}
		    }
		    catch (Exception e)
		    {
			throw new SQLException ("while calculating row count: " + e.getMessage());
		    }	    
		    ResultSet rs = paramStmt.executeQuery ();
				
		    rs.next ();		
				
		    // here's what we need.
		    count = rs.getInt (1);

		    paramStmt.close();
		}

		return count;

	}

	//--------------------------------------------------------------------
	// reWordAsCountQuery.
	// changes the SQL for the ResultSet into an count(*) query as an 
	// atempt to produce a rowCount (see calculateRowCount ()).
	//--------------------------------------------------------------------

	protected String reWordAsCountQuery(String SQLstr)
	{

			int function	= SQLstr.indexOf (" COUNT(*) ");
			int operation   = -1;
			int fromPos     = SQLstr.indexOf (" FROM ");

			// check if the query has a quote String before
			// the key word FROM.

			int startQuote	= SQLstr.indexOf ("'");
			int endQuote = -1;
			    
			if (startQuote > 0)
			{
			    endQuote	= SQLstr.indexOf ("'", startQuote + 2);				
			}

			// An SQL may have more than one occurence of the word
			// "FROM". So we ignore the String within the SQL in quotes 
			// prior to word "FROM" and replace the (column(s)/string) with
			// count(*).

			if ( (fromPos > startQuote) && (endQuote > fromPos) )
			{
			    fromPos = SQLstr.indexOf(" FROM ", endQuote);
			}
			    
			// re-check if the function is inside the quote.
			if ( (function > startQuote) && (endQuote > function) )
			      function = -1;

			// mark the index for the piece of sql
			// to trim of the count query.
			// e.g. "ORDER BY" nor "GROUP BY"
			// should not alter the row count.

			int trimOrder = -1;
			int trimGroup = -1;
			int trimForUp = -1;
			int trimUnion = -1;
			int trimpiece = -1;

			int whereClause = SQLstr.indexOf ("WHERE");			
			int clauseValue = -1;
			if (whereClause < fromPos)
			    whereClause = SQLstr.indexOf ("WHERE", whereClause + 2);	

			String trimKeyWords = "";
			
			if (trimpiece < 0)
			{
			    trimOrder = SQLstr.lastIndexOf("ORDER BY");
			    if (trimOrder > whereClause)
				trimpiece = trimOrder;

			    trimKeyWords = "ORDER BY";
			}
			if (trimpiece < 0)
			{
			    trimGroup = SQLstr.lastIndexOf("GROUP BY");
			    if ((trimGroup > whereClause) && (trimGroup > trimOrder))
				trimpiece = trimGroup;

			    trimKeyWords = "GROUP BY";
			}
			if (trimpiece < 0)
			{
			    trimForUp = SQLstr.lastIndexOf("FOR UPDATE");
			    if ((trimForUp > whereClause) && (trimForUp > trimGroup))
				trimpiece = trimForUp;

			    trimKeyWords = "FOR UPDATE";
			}
			if (trimpiece < 0)
			{
			    trimUnion = SQLstr.lastIndexOf("UNION");
			    if (trimUnion > whereClause && (trimUnion > trimForUp))
				trimpiece = trimUnion;

			    trimKeyWords = "UNION";
			}

			if (trimpiece > 0 )
			{
			    // make sure the piece to trim
			    // is not part of a where clause or 
			    // string value.

			    if (trimpiece > fromPos)
			    {			    
				if ( (whereClause > 0) && (whereClause > fromPos) )
				{

				    int startWhereQuote	= SQLstr.indexOf ("'", whereClause);
				    int endWhereQuote = -1;
					
				    if (startWhereQuote > 0)
				    {
					endWhereQuote	= SQLstr.indexOf ("'", startWhereQuote + 2);				
				    }

				    // search for the key words to trim if the first attempt
				    // fount the key words within a where clause value.
				    if ( (trimpiece > startWhereQuote) && (endQuote > trimpiece) )
				    {
					trimpiece = SQLstr.indexOf(trimKeyWords, endWhereQuote);
				    }
				    				
				    if (trimpiece > endWhereQuote)
					    SQLstr = SQLstr.substring(0, trimpiece);
				}
				else
					    SQLstr = SQLstr.substring(0, trimpiece);

			    } //skip trim piece.

			}
		    
			String functionTest = SQLstr.substring(0, fromPos);
			StringBuffer newSql = new StringBuffer (SQLstr);

			    
			// check for a funtion query.

			if ( function < 0 && operation < 0)
			{
			    operation = functionTest.lastIndexOf (")"); // end of the function.
			    
			    if ( operation > 0 )
			    {
				int  compute = SQLstr.indexOf(" ("); // start of the function
								
				// check for non-function.
				if ( compute > 0) 
				{
				    if ( (compute < startQuote) && (compute < endQuote) )
					operation = -1;
				}
				else if (compute < 0)
				{
					operation = -1;
					function = compute; // a database function w/o count.
				}

			    }

			}


			if ( operation > 0)
			{
			    	newSql.insert (6, " COUNT(*), ");
			}
			else if ((function < 0) && (fromPos > 0))
			{
				newSql.replace (6, fromPos, " COUNT(*) ");
			}

			return newSql.toString();
			
	}

	//--------------------------------------------------------------------
	// setCursorType.
	// sets the ResultSet's cursor Type. If the cursor was downgraded, 
	// the temporary type will be the ResultSet's cursor Type until 
	// the ResultSet or the owner statement gets closed.
	//--------------------------------------------------------------------

	protected void setCursorType()
		throws SQLException
	{
		// Reset last warning message

		clearWarnings ();

		try {
			long result = OdbcApi.SQLGetStmtOption (hStmt, OdbcDef.SQL_ATTR_CURSOR_TYPE);

			odbcCursorType = (short)result;
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			odbcCursorType = n.shortValue ();

			setWarning (JdbcOdbc.convertWarning (ex));
		}
	}

	//--------------------------------------------------------------------
	// checkOpen
	// Some statements that re-execute do not have their resultSet's 
	// garbage collected in time to throw an Exception if the resutlSet 
	// has already been closed. Each cursor movement now checks here to 
	// make sure that the resultSet is open.
	//--------------------------------------------------------------------

	protected void checkOpen()
		throws SQLException
	{
		if (closed) {
			throw new SQLException("ResultSet is closed");
		}
	}

	//====================================================================
	// Data attributes
	//====================================================================

	protected JdbcOdbc OdbcApi;		// ODBC API interface object

	protected long	hDbc;			// Database connection handle

	protected long	hStmt;			// Statement handle

	protected SQLWarning lastWarning;	// Last SQLWarning generated
						//  by an operation.

	protected boolean keepHSTMT;		// Flag indicating whether the
						//  statement handle should be
						//  kept upon close.  Another
						//  object may 'own' the 
						//  statement handle, and we
						//  don't want to destroy it.

	protected JdbcOdbcBoundCol boundCols[];	// An array of bound column
						//  objects.  See comments
						//  in initialize ()

	protected int	numberOfCols;		// Number of columns in the
						//  result set

	protected int   numResultCols;		// Cache for number of result
						//  columns

	protected int	firstPseudoCol;		// First pseudo column number

	protected int	lastPseudoCol;		// Last pseudo column number
	
	protected JdbcOdbcPseudoCol pseudoCols[];
						// An array of pseudo column
						//  objects.  See comments
						//  in JdbcOdbcPseudoCol

	protected int	colMappings[];		// An array used to map
						//  column numbers.  See 
						//  comments for
						//  setColumnMappings

	protected ResultSetMetaData rsmd;	// ResultSetMetaData object
						//  used for finding column
						//  names and mapping them to
						//  column numbers

	private java.util.Hashtable colNameToNum;
						// Hash table for mapping
						//  column names to numbers
	private java.util.Hashtable colNumToName;
						// Hash table for mapping
						//  column numbers to names

	private boolean lastColumnNull;		// true if the last column
						//  referenced by a getXXX
						//  function was null

	private boolean closed;			// true if the result set
						//  has been closed

	private int sqlTypeColumn;		// If a catalog result set
						//  has a SQL data type, we
						//  need to convert it to
						//  a JDBC SQL type

	private boolean freed;			// true if the statement
						//  has already been freed						
						
	private JdbcOdbcUtils utils = 		// Utility
	    new JdbcOdbcUtils();		// object

	private boolean ownInsertsAreVisible;   // Variables for checking the
						// visibility of updates
	private boolean ownDeletesAreVisible;	// 4628693, 4668340						

	protected JdbcOdbcStatement ownerStatement;
						// Keep a reference of our
						//  owning statement object

	protected int numberOfRows;		// Number of rows in result set

	protected int rowPosition;		// Current Row position of 
						//  scrollable cursor

	protected int lastRowPosition;		// row Position to go back to.

	protected int[] rowStatusArray;		// Row atatus array for 
						// updatable ResultSets.

	protected boolean atInsertRow;		// true when inserting rows.

	protected int lastForwardRecord;	// marks the last FORWARD_ONLY 
						// row position.
	
	protected int lastColumnData;		// marks the last column
						// where getXXX() methods where
						// performed.

	protected int rowSet;			// block-cursor size.

	protected boolean blockCursor;		// true if a block-cursor
						// is being used when scrolling.

	protected int fetchCount;		// count for block-cursor fetches

	protected int currentBlockCell;		// position within the block-cursor
	protected int lastBlockPosition;	// stores currentBlockCell during inserts.

	protected boolean moveUpBlock;		// true if pward direction
						// within block-cursor.
	protected boolean moveDownBlock;	// true if downward direction
						// within block-cursor.
	protected short odbcCursorType;

	protected boolean rowUpdated;		// true after a updateRow is executed.
						// (see consecutiveFetch)
						
	protected long[] pA; //4486684, pointers for native buffers
	
}
