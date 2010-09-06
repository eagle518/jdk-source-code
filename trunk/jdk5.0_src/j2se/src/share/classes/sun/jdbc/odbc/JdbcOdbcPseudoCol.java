/*
 * @(#)JdbcOdbcPseudoCol.java	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcPseudoCol.java
//
// Description: Class for a pseudo result set column.  In some cases, 
//              the DatabaseMetaData class returns information about the
//              data source that has a fixed result set format (i.e. 
//              getColumns).  For some versions of ODBC drivers, some of
//              these columns do not exist.  This class represents one of
//              these columns.  If the ODBC driver does not support these
//              types of columns, they will be manufactured (using this
//              class) and returned to the Java application.  The column
//              label and type information will be stored, and the column
//              value for all rows will be null.
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

public class JdbcOdbcPseudoCol
	extends	JdbcOdbcObject {


	//--------------------------------------------------------------------
	// Constructors
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcPseudoCol (
		String columnLabel,
		int columnType,
		int columnLength)
	{
		colLabel = columnLabel;
		colType = columnType;
		colLength = columnLength;

		// Determine the display size of the column

		colDisplaySize = colLength;

		switch (colType) {
		case Types.BINARY:
		case Types.VARBINARY:
		case Types.LONGVARBINARY:

			// Each binary byte takes two characters to display

			colDisplaySize *= 2;
			break;
		case Types.DATE:
			colDisplaySize = 10;
			break;
		case Types.TIME:
			colDisplaySize = 8;
			break;
		case Types.TIMESTAMP:
			colDisplaySize = 29;
			break;
		case Types.NUMERIC:
		case Types.DECIMAL:
			colDisplaySize += 2;
			break;
		case Types.BIT:
			colDisplaySize = 1;
			break;
		case Types.TINYINT:
			colDisplaySize = 4;
			break;
		case Types.SMALLINT:
			colDisplaySize = 6;
			break;
		case Types.INTEGER:
			colDisplaySize = 11;
			break;
		case Types.BIGINT:
			colDisplaySize = 20;
			break;
		case Types.REAL:
			colDisplaySize = 13;
			break;
		case Types.FLOAT:
		case Types.DOUBLE:
			colDisplaySize = 22;
			break;
		}
	}

	//--------------------------------------------------------------------
	// getColumnLabel
	//--------------------------------------------------------------------

	public String getColumnLabel ()
	{
		return colLabel;
	}

	//--------------------------------------------------------------------
	// getColumnType
	//--------------------------------------------------------------------
	
	public int getColumnType ()
	{
		return colType;
	}

	//--------------------------------------------------------------------
	// getColumnLength
	//--------------------------------------------------------------------
	
	public int getColumnLength ()
	{
		return colLength;
	}

	//--------------------------------------------------------------------
	// getColumnDisplaySize
	//--------------------------------------------------------------------
	
	public int getColumnDisplaySize ()
	{
		return colDisplaySize;
	}


	//====================================================================
	// Data attributes
	//====================================================================
	
	protected String colLabel;		// Column label

	protected int colType;			// JDBC SQL type of the column

	protected int colLength;		// Length of column

	protected int colDisplaySize;		// Column display size

}
