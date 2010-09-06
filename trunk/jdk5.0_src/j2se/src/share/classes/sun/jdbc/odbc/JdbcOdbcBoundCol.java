/*
 * @(#)JdbcOdbcBoundCol.java	@(#)JdbcOdbcBoundCol.java	1.32 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcBoundCol.java
//
// Description: Class for a result set column.  This class is used a place
//              holder for information needed for each result set column.
//              Currently, this includes columns that are using an InputStream
//              to return data from the result set.
//
//              An array of JdbcOdbcBoundCol objects is created for this
//              purpose.
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

import java.sql.*;

public class JdbcOdbcBoundCol
	extends	JdbcOdbcObject {

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcBoundCol ()
	{
		type = OdbcDef.SQL_TYPE_UNKNOWN;
		len = -1;
		isRenamed = false;
		aliasName = null;
	}	

	//--------------------------------------------------------------------
	// setInputStream
	// Sets the input stream object.
	//--------------------------------------------------------------------

	public void setInputStream (JdbcOdbcInputStream inStream)
	{
		inputStream = inStream;
	}

	//--------------------------------------------------------------------
	// closeInputStream
	// Closes (invalidates) the current input stream, and removes
	// the reference to the input stream
	//--------------------------------------------------------------------

	public void closeInputStream ()
	{
		if (inputStream != null) {
			inputStream.invalidate ();
			inputStream = null;
		}
	}

	//--------------------------------------------------------------------
	// setType
	// Sets the JDBC SQL type
	//--------------------------------------------------------------------

	public void setType (
		int javaType)
	{	
		type = javaType;
	}

	//--------------------------------------------------------------------
	// getType
	// Gets the JDBC SQL type
	//--------------------------------------------------------------------

	public int getType ()
	{	
		return type;
	}

	//--------------------------------------------------------------------
	// setLength
	// Sets the column length
	//--------------------------------------------------------------------

	public void setLength (
		int l)
	{	
		len = l;
	}

	//--------------------------------------------------------------------
	// getLength
	// Gets the column length
	//--------------------------------------------------------------------

	public int getLength ()
	{	
		return len;
	}

	//--------------------------------------------------------------------
	// setAliasName // sun's 4234318 fix.
	// Sets the column name to be used
	//--------------------------------------------------------------------
 
	public void setAliasName (
			String Name)
	{
		aliasName = Name;
		isRenamed = true;
	}
 
	//--------------------------------------------------------------------
	// mapAliasName // sun's 4234318 fix.
	// Maps the column name to alias name if isrenamed flag is used
	//--------------------------------------------------------------------
	public String mapAliasName (
			String columnName)
	{
		if (isRenamed == true)
			return aliasName;
		else
			return columnName;
	}
         	
	//--------------------------------------------------------------------
	// setColumnValue
	// Sets the input stream object.
	//--------------------------------------------------------------------
	public void setColumnValue (Object value, int lenInd)
	{

	    // Try a InputStream type first.
	    try
	    {
	    	if ((type == Types.LONGVARCHAR) || (type == Types.LONGVARBINARY) )
		{
		    if ( (java.io.InputStream)value != null ) 
			setInputStream((JdbcOdbcInputStream)value);
		    else
			colObj = value;
		}
		else 
		    colObj = value;

		    setLength(lenInd);
	    }
	    catch (Exception e) {}

	}


	//--------------------------------------------------------------------
	// getColumnValue
	// Sets the input stream object.
	//--------------------------------------------------------------------
	public Object getColumnValue ()
	{

	    // Try a InputStream type first.
	    if ((type == Types.LONGVARCHAR) || (type == Types.LONGVARBINARY) )
	    {
	    	if (inputStream != null) 
		    return inputStream;
		else 
		    return colObj;
	    }
	    else
		return colObj;

	}

	//--------------------------------------------------------------------
	// getInputStream
	// returns the input stream object.
	//--------------------------------------------------------------------

	public JdbcOdbcInputStream getInputStream ()
	{
		return inputStream;
	}

	//--------------------------------------------------------------------
	// initStagingArea
	// Initializes the column's Data storage area with the proper row size
	// including an extra row for inserts.
	//--------------------------------------------------------------------

	public void initStagingArea(int rowSet)
	{
	    this.rowSetSize = rowSet;		

	    columnWiseData = new Object[rowSetSize + 1];
	    
	    //4691886
	    columnWiseLength = new byte[(rowSetSize + 1) * (JdbcOdbcPlatform.getLengthBufferSize())];
	    //4691886
	    //4672508
	    // changing rowSetSize to rowSetSize+1
	    
	    /**
	     * Filling the length indicator byte array with the byte representation of
	     * the value OdbcDef.SQL_COLUMN_IGNORE.
	     */
	    
	    byte[] b = JdbcOdbcPlatform.convertIntToByteArray((int)OdbcDef.SQL_COLUMN_IGNORE);
	    
	    for (int i = 0; i < (rowSetSize+1) * b.length; i=i + b.length) {
		for(int j=0; j<b.length; j++) {
		    columnWiseLength[i+j] = b[j];
		}
	    }
	}

	//--------------------------------------------------------------------
	// resetColumnToIgnoreData()
	// reset Column length indicator to OdbcDef.SQL_COLUMN_IGNORE, so that
	// only the current row is updated or inserted w/o afecting the other 
	// rows within the rowSet.
	//--------------------------------------------------------------------

	public void resetColumnToIgnoreData()
	{
	    //4691886
	    // 4672508
	    // changing rowSetSize to rowSetSize+1
	    
	    /**
	     * Filling the length indicator byte array with the byte representation of
	     * the value OdbcDef.SQL_COLUMN_IGNORE.
	     */
	    byte[] b = JdbcOdbcPlatform.convertIntToByteArray((int)OdbcDef.SQL_COLUMN_IGNORE);
	    
	    for (int i = 0; i < (rowSetSize+1) * b.length; i=i + b.length) {
		for(int j=0; j<b.length; j++) {
		    columnWiseLength[i+j] = b[j];
		}
	    }	
	}

	//--------------------------------------------------------------------
	// setRowValue
	// sets the Data to be bound for later update with SQLSetPos with 
	// with it's corresponding length indicator.
	//--------------------------------------------------------------------

	public void setRowValues(int row, Object rowData, int length)
	{
	    columnWiseData[row] = rowData;
	    
	    // 4691886
	    
	    /**
	     * Setting the length indicator for the row "row" that has been passed
	     * as a parameter to this function in the length indicator byte array,
	     * columnWiseLength. This is achieved by copying the byte array
	     * representation of the parameter "length" that has been passed to
	     * this function from the position [row * (size of each length buffer)]
	     * to [row * (size of each length buffer) + (size of each length buffer)]
	     */
	    
	    byte[] b = JdbcOdbcPlatform.convertIntToByteArray(length);
	    
	    int j = row * b.length;
	    for (int i = j; i < j + b.length; i++) {
		columnWiseLength[i] = b[i-j];
	    }
	}

	//--------------------------------------------------------------------
	// getRowValue
	// returns an array of stored Object for a call to SQLBoundCol
	//--------------------------------------------------------------------

	public Object getRowValue(int row)
	{
	    return columnWiseData[row];
	}

	//--------------------------------------------------------------------
	// getRowLenInd
	// returns an array of length/Indicator for the call to SQLBounCol.
	//--------------------------------------------------------------------

	public int getRowLenInd(int row)
	{
	    return columnWiseLength[row];
	}

	//--------------------------------------------------------------------
	// getRowValue
	// returns an array of stored Object for a call to SQLBoundCol
	//--------------------------------------------------------------------

	public Object[] getRowValues()
	{
	    return columnWiseData;
	}

	//--------------------------------------------------------------------
	// getRowLenInd
	// returns an array of length/Indicator for the call to SQLBounCol.
	//--------------------------------------------------------------------

	//4691886
	public byte[] getRowLengths()
	{
	    return columnWiseLength;
	}

	//--------------------------------------------------------------------
	// allocBindDataBuffer
	// Allocates and returns a new bind data buffer of the specified
	// length
	//--------------------------------------------------------------------
	public byte[] allocBindDataBuffer (
		int bufLen)
	{
		binaryData = new byte[bufLen];

		// Reset the input stream, we are doing a new bind
		// setInputStream (null, 0);

		return binaryData;
	}

	//--------------------------------------------------------------------
	// setStreamType
	// Sets the IO Stream type
	//--------------------------------------------------------------------

	public void setStreamType (
		int ioType)
	{	
		streamType = ioType;
	}

	//--------------------------------------------------------------------
	// getStreamType
	// Gets the IO Stream type
	//--------------------------------------------------------------------

	public int getStreamType ()
	{	
		return streamType;
	}

	//====================================================================
	// Data attributes
	//====================================================================

	protected int type;			// JDBC SQL type for the
						//  column.  Initially set
						//  to SQL_TYPE_UNKNOWN

	protected int len;			// Length of the column.
						//  Initially set to -1
						//  indicating that it is 
						//  unknown.

	protected JdbcOdbcInputStream inputStream;
						// If an input stream is used
						//  to get data from a column,
						//  a reference to that object
						//  is kept here.  Once the
						//  Statement is closed or
						//  the cursor is moved (next)
						//  all of the input streams
						//  for the statement are
						//  closed (marked as invalid)
						//  and then the reference is
						//  removed.

	protected boolean isRenamed;		//Show whether alias name is to be used

	protected String aliasName;		//Store the alias name.		

	protected int rowSetSize;		// The numver of the rowSet block.

	protected Object colObj;		// reference to column value.

	protected Object columnWiseData[];	// Staging Area for ResultSet's
						// position updates.

	//4691886
	protected byte columnWiseLength[];	// Staging Area's length for the 
						// data used in position updates.
	protected byte[] binaryData;
	

	protected int streamType;		// Input stream type
						// (ASCII, BINARY, UNICODE)
	public final static short ASCII   = 1;
	public final static short UNICODE = 2;
	public final static short BINARY  = 3;

        protected long pA1=0;              //pointers
        protected long pA2=0;
        protected long pB1=0;
        protected long pB2=0;
        protected long pC1=0;
        protected long pC2=0;
        protected long pS1=0;
        protected long pS2=0;// reserved for strings(UTFChars)
}
