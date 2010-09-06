/*
 * @(#)JdbcOdbcResultSetMetaData.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcResultSetMetaData.java
//
// Description: Impementation of the ResultSetMetaData interface class
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

public class JdbcOdbcResultSetMetaData
	extends		JdbcOdbcObject
	implements	java.sql.ResultSetMetaData {

	//====================================================================
	// Public methods
	//====================================================================

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcResultSetMetaData (
		JdbcOdbc odbcApi,
		JdbcOdbcResultSetInterface rs)
	{
		// Save a pointer to the ODBC api and the ResultSet object

		OdbcApi = odbcApi;
		resultSet = rs;

		// Set the connection handle

		hStmt = rs.getHSTMT ();
	}

	//--------------------------------------------------------------------
	// getColumnCount
	// The number of columns in the ResultSet.
	//--------------------------------------------------------------------

	public int getColumnCount ()
		throws SQLException
	{
		return resultSet.getColumnCount ();
	}

	//--------------------------------------------------------------------
	// isAutoIncrement
	// Is column automaticallty numbered, thus read-only.
	//--------------------------------------------------------------------

	public boolean isAutoIncrement (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isAutoIncrement (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			value = getColAttributeBoolean (column,
				OdbcDef.SQL_COLUMN_AUTO_INCREMENT);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isCaseSensitive
	// Does case matter?
	//--------------------------------------------------------------------

	public boolean isCaseSensitive (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isCaseSensitive (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			value = getColAttributeBoolean (column,
				OdbcDef.SQL_COLUMN_CASE_SENSITIVE);
		}
		return value;
	}


	//--------------------------------------------------------------------
	// isSearchable
	// Can column be used in where clause?
	//--------------------------------------------------------------------

	public boolean isSearchable (
		int column)
		throws SQLException
	{	
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isSearchable (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			int n = getColAttribute (column,
					OdbcDef.SQL_COLUMN_SEARCHABLE);
			value = (n != OdbcDef.SQL_UNSEARCHABLE);
		}
		return value;
	}


	//--------------------------------------------------------------------
	// isCurrency
	// Is it a cash value?
	//--------------------------------------------------------------------

	public boolean isCurrency (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isCurrency (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			value = getColAttributeBoolean (column,
				OdbcDef.SQL_COLUMN_MONEY);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isNullable
	// Can you put a null in this column?		
	//--------------------------------------------------------------------

	public int isNullable (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isNullable (" +
				column + ")");
		}
		int value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = columnNoNulls;
		}
		else {
			value = getColAttribute (column,
					OdbcDef.SQL_COLUMN_NULLABLE);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isSigned
	// True if column is signed number.
	//--------------------------------------------------------------------

	public boolean isSigned (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isSigned (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			value = (getColAttributeBoolean (column,
				OdbcDef.SQL_COLUMN_UNSIGNED) == false);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// getColumnDisplaySize
	// normal max width as chars
	//--------------------------------------------------------------------

	public int getColumnDisplaySize (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnDisplaySize (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		JdbcOdbcPseudoCol pc = resultSet.getPseudoCol (column);
		int value;

		// If a pseudo column exists for this column, get the
		// data from the pseudo column

		if (pc != null) {
			value = pc.getColumnDisplaySize ();
		}
		else {
			// No pseudo column, get the attribute

			value = getColAttribute (column,
				OdbcDef.SQL_COLUMN_DISPLAY_SIZE);
		}

		return value;
	}

	//--------------------------------------------------------------------
	// getColumnLabel
	// Suggested column title for use in printouts and displays
	//--------------------------------------------------------------------

	public String getColumnLabel (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnLabel (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		JdbcOdbcPseudoCol pc = resultSet.getPseudoCol (column);
		String value;

		// If a pseudo column exists for this column, get the
		// data from the pseudo column

		if (pc != null) {
			value = pc.getColumnLabel ();
		}
		else {
			// No pseudo column, get the attribute
			value =  getColAttributeString (column,
				OdbcDef.SQL_COLUMN_LABEL);
		}
				
		value = resultSet.mapColumnName(value, column); // sun's 4234318 fix.

		return value;
	}

	//--------------------------------------------------------------------
	// getColumnName
	// SQL name for column
	//--------------------------------------------------------------------

	public String getColumnName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnName (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		JdbcOdbcPseudoCol pc = resultSet.getPseudoCol (column);
		String value;

		// If a pseudo column exists for this column, get the
		// data from the pseudo column

		if (pc != null) {
			value = pc.getColumnLabel ();
		}
		else {
			// No pseudo column, get the attribute
			value = getColAttributeString (column,
				OdbcDef.SQL_COLUMN_NAME);
		}
	
		value = resultSet.mapColumnName(value, column); // sun's 4234318 fix.

		return value;
	}


	//--------------------------------------------------------------------
	// getSchemaName
	// Column's table's schema, or "" if not applicable.
	//--------------------------------------------------------------------

	public String getSchemaName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getSchemaName (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return the
		// same schema name as column 1

		if (resultSet.getPseudoCol (column) != null) {
			column = 1;
		}

		return getColAttributeString (column,
			OdbcDef.SQL_COLUMN_OWNER_NAME);
	}

	//--------------------------------------------------------------------
	// getPrecision
	// Number of decimal digits
	//--------------------------------------------------------------------

	public int getPrecision (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getPrecision (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		JdbcOdbcPseudoCol pc = resultSet.getPseudoCol (column);
		int value;

		// If a pseudo column exists for this column, get the
		// data from the pseudo column

		if (pc != null) {
			value = pc.getColumnDisplaySize () - 1;
		}
		else {
			// No pseudo column, get the attribute
			value = getColAttribute (column,
				OdbcDef.SQL_COLUMN_PRECISION);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// getScale
	// Number of digits to right of decimal
	//--------------------------------------------------------------------

	public int getScale (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getScale (" +
				column + ")");
		}
		return resultSet.getScale (column);
	}

	//--------------------------------------------------------------------
	// getTableName
	// name of column's table, or "" if not applicable.
	//--------------------------------------------------------------------

	public String getTableName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getTableName (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return the
		// same table name as column 1

		if (resultSet.getPseudoCol (column) != null) {
			column = 1;
		}

		return getColAttributeString (column,
			OdbcDef.SQL_COLUMN_TABLE_NAME);
	}


	//--------------------------------------------------------------------
	// getCatalogName
	// catalog of column's table, or "" if not applicable.
	//--------------------------------------------------------------------

	public String getCatalogName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getCatalogName (" +
				column + ")");
		}
		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return the
		// same catalog name as column 1

		if (resultSet.getPseudoCol (column) != null) {
			column = 1;
		}

		return getColAttributeString (column,
			OdbcDef.SQL_COLUMN_QUALIFIER_NAME);
	}

	//--------------------------------------------------------------------
	// getColumnType
	// SQL type, from java.sql.Types
	//--------------------------------------------------------------------

	public int getColumnType (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnType (" +
				column + ")");
		}
		JdbcOdbcPseudoCol pc = resultSet.getPseudoCol (column);
		int value;

		// If a pseudo column exists for this column, get the
		// data from the pseudo column

		if (pc != null) {
			value = pc.getColumnType () - 1;
		}
		else {

			// The result set already contains information about
			// the column.  Get the SQL type from here.

			value = resultSet.getColumnType (column);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// getColumnTypeName
	// SQL type name
	//--------------------------------------------------------------------

	public String getColumnTypeName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnTypeName (" +
				column + ")");
		}
		String value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return ""

		if (resultSet.getPseudoCol (column) != null) {
			value = "";
		}
		else {
			value = getColAttributeString (column,
				OdbcDef.SQL_COLUMN_TYPE_NAME);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isReadOnly
	// definitely not writable
	//--------------------------------------------------------------------

	public boolean isReadOnly (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isReadOnly (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return true

		if (resultSet.getPseudoCol (column) != null) {
			value = true;
		}
		else {
			int n = getColAttribute (column,
					OdbcDef.SQL_COLUMN_UPDATABLE);
			value = (n == OdbcDef.SQL_ATTR_READONLY);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isWritable
	// a write *may* succeed
	//--------------------------------------------------------------------

	public boolean isWritable (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isWritable (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false 

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			int n = getColAttribute (column,
					OdbcDef.SQL_COLUMN_UPDATABLE);
			value = (n == OdbcDef.SQL_ATTR_READWRITE_UNKNOWN);
		}
		return value;
	}

	//--------------------------------------------------------------------
	// isDefinitelyWritable
	// a write will succeed
	//--------------------------------------------------------------------
	
	public boolean isDefinitelyWritable (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.isDefinitelyWritable (" +
				column + ")");
		}
		boolean value;

		// Re-map column if necessary

		column = resultSet.mapColumn (column);

		// If a pseudo column exists for this column, return false 

		if (resultSet.getPseudoCol (column) != null) {
			value = false;
		}
		else {
			int n = getColAttribute (column,
					OdbcDef.SQL_COLUMN_UPDATABLE);
			value = (n == OdbcDef.SQL_ATTR_WRITE);
		}
		return value;
	}

	// New JDBC 2.0 API

	//--------------------------------------------------------------------
	// getColumnClassName (2.0)
	// Returns the fully-qualified name of the class in the Java
	// programming language that would be used by the method 
	// ResultSet.getObject to retrieve the value in the specified column. 
	// This is the class name used for custom mapping.
	//--------------------------------------------------------------------
	
	public String getColumnClassName (
		int column)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*ResultSetMetaData.getColumnClassName (" +
				column + ")");
		}

		String className = (new String()).getClass().getName();

		int sqlType = getColumnType(column);

		// Not implemented yet.
		// throw new UnsupportedOperationException();
		switch (sqlType) {


		case Types.NUMERIC:
		case Types.DECIMAL:
			className = (new java.math.BigDecimal(0)).getClass().getName ();
			break;

		case Types.BIT:			
			className = (new Boolean(false)).getClass().getName ();
			break;

		case Types.TINYINT:
			className = (new Byte("0")).getClass().getName ();
			break;

		case Types.SMALLINT:
			className = (new Short("0")).getClass().getName ();
			break;

		case Types.INTEGER:
			className = (new Integer(0)).getClass().getName ();
			break;

		case Types.BIGINT:
			className = (new Long(0)).getClass().getName ();
			break;

		case Types.REAL:
			className = (new Float(0)).getClass().getName ();
			break;

		case Types.FLOAT:
		case Types.DOUBLE:
			className = (new Double(0)).getClass().getName();
			break;
			
		case Types.BINARY:
		case Types.VARBINARY:
		case Types.LONGVARBINARY:
		    // Fixed for Sun BugID 4398863. Moved LONGVARBINARY so that it returned
		    // the same class as the other binary types.
		    // NOTE: this will return [B which is correct. Please see:
		    // http://java.sun.com/docs/books/jls/second_edition/html/arrays.doc.html#40879
			byte[] b = {};
			className = (b.getClass()).getName();
			break;

		case Types.DATE:
			className = (new java.sql.Date(123456)).getClass().getName ();
			break;

		case Types.TIME:
			className = (new java.sql.Time(123456)).getClass().getName ();
			break;

		case Types.TIMESTAMP:
			className = (new java.sql.Timestamp(123456)).getClass().getName ();
			break;


		}

		return className;



	}	

	//====================================================================
	// Protected methods
	//====================================================================

	protected int getColAttribute (
		int column,
		int type)
		throws SQLException
	{
		return resultSet.getColAttribute (column, type);
	}

	protected boolean getColAttributeBoolean (
		int column,
		int type)
		throws SQLException
	{
		int value = getColAttribute (column, type);
		boolean rc = false;
		
		// If the value is SQL_TRUE (1), return true

		if (value == OdbcDef.SQL_TRUE) {
			rc = true;
		}
		return rc;
	}

	protected String getColAttributeString (
		int column,
		int type)
		throws SQLException
	{
		String	value = "";

		// Reset last warning message

		resultSet.clearWarnings ();

		try {
			value = OdbcApi.SQLColAttributesString (hStmt,
					column, type);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			value = (String) ex.value;
			resultSet.setWarning (JdbcOdbc.convertWarning (ex));
		}
		return value.trim ();
	}


	//====================================================================
	// Data attributes
	//====================================================================

	protected JdbcOdbc OdbcApi;		// ODBC API interface object

	protected JdbcOdbcResultSetInterface resultSet;
						// Owning result set object
	
	protected long	hStmt;			// Statement handle
	
}
