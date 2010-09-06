/*
 * @(#)JdbcOdbcResultSetInterface.java	1.29 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcResultSetInterface.java
//
// Description: Definition of the JdbcOdbcResultSet interface.  This is
//              necessary to 'forward define' the class to eliminate
//              circular dependencies
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Karl Moss
//
// Date:        May, 1996
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.sql.*;

public interface JdbcOdbcResultSetInterface
	extends		java.sql.ResultSet
{

	//--------------------------------------------------------------------
	// getPseudoCol
	// Given a column number, return the corresponding pseudo column,
	// null if one does not exist for this column
	//--------------------------------------------------------------------

	public JdbcOdbcPseudoCol getPseudoCol (
		int column);

	//--------------------------------------------------------------------
	// mapColumn
	// Given a column number, map it to the corresponding column in
	// the result set.  If no column mappings exist, return the original
	// column number.  If the column number is out-of-range, return -1.
	//--------------------------------------------------------------------

	public int mapColumn (
		int	column);

	//--------------------------------------------------------------------
	// clearWarnings
	// Clears any warning information for the statement
	//--------------------------------------------------------------------

	public void clearWarnings()
			throws SQLException;

	//--------------------------------------------------------------------
	// getHSTMT
	// Returns the statement handle for the result set
	//--------------------------------------------------------------------
	
	public long getHSTMT ();

	//--------------------------------------------------------------------
	// getColumnCount
	// Return the number of columns in the ResultSet
	//--------------------------------------------------------------------

	public int getColumnCount ()
		throws SQLException;

	//--------------------------------------------------------------------
	// getScale
	// Number of digits to right of decimal
	//--------------------------------------------------------------------

	public int getScale (
		int column)
		throws SQLException;

	//--------------------------------------------------------------------
	// getColumnType
	// Returns the Java SQL type of the given column number.
	//--------------------------------------------------------------------

	public int getColumnType (
		int column)
		throws SQLException;

	//--------------------------------------------------------------------
	// getColAttribute
	// Given the column and attribute type, return the attribute value
	//--------------------------------------------------------------------

	public int getColAttribute (
		int column,
		int type)
		throws SQLException;

	//--------------------------------------------------------------------
	// setWarning
	// Sets the warning 
	//--------------------------------------------------------------------

	public void setWarning (
			SQLWarning ex)
			throws SQLException;
  
	//--------------------------------------------------------------------
	// mapColumnName // sun's 4234318 fix.
	// Given a column name, map it to the corresponding column rename in 
	// the result set. If no column rename exist, return the original column
	// name. If the column number is out-of-range, return original column name.
	//--------------------------------------------------------------------
	         
	public String mapColumnName (
			String columnName,
			int column);

}
