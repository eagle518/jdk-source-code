/*
 * @(#)JdbcOdbcConnectionInterface.java	1.26 00/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcConnectionInterface.java
//
// Description: Definition of the JdbcOdbcConnection interface.  This is
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
import java.util.Vector;

public interface JdbcOdbcConnectionInterface
	extends		java.sql.Connection
{
	//--------------------------------------------------------------------
	// getHDBC
	// Returns the connection handle
	//--------------------------------------------------------------------

	public long getHDBC ();

	//--------------------------------------------------------------------
	// getURL
	// Returns the URL for the connection
	//--------------------------------------------------------------------

	public String getURL ();

	//--------------------------------------------------------------------
	// getODBCVer
	// Queries the data source and gets the current ODBC version number.
	// The major version is returned as an int.  Returns -1 if unknown.
	//--------------------------------------------------------------------

	public int getODBCVer ();

	//--------------------------------------------------------------------
	// validateConnection
	// Validates that this connection is not closed.  If the connection
	// is closed, throw an exception
	//--------------------------------------------------------------------

	public void validateConnection ()
			throws SQLException;

	//--------------------------------------------------------------------
	// deregisterStatement
	// Each Statement object has a reference to it in the owning Connection
	// object.  This method will remove that reference
	//--------------------------------------------------------------------

	public void deregisterStatement (Statement stmt);
	//--------------------------------------------------------------------
	// setBatchVector
	// store a copy of Batch SQL with reference to the Statement object
	//--------------------------------------------------------------------
	public void setBatchVector(Vector batchVector, Statement stmt);


	//--------------------------------------------------------------------
	// getBatchVector
	// return the Batch SQL of the referenced Statement object
	//--------------------------------------------------------------------
	public Vector getBatchVector(Statement stmt);
	
		
	//--------------------------------------------------------------------
	// removeBatchVector
	// destroy Batch SQL for the given referenced statement Object.
	//--------------------------------------------------------------------
	public void removeBatchVector(Statement stmt);
	
	//--------------------------------------------------------------------
	// getBatchRowCountFlag
	// returns the row count Type if Batch Row Counts are supported.
	// or -1 if the individual row counts are not rupported.
	//--------------------------------------------------------------------
	public int getBatchRowCountFlag(int StmtType);
	
	public short getOdbcCursorType (int resultSetType);

	public int getOdbcCursorAttr2 (short odbcCursor)
		throws SQLException;
	
	public short getOdbcConcurrency (int resultSetConcurrency);

	//--------------------------------------------------------------------
	// getBestOdbcCursorType
	// Return the Best ODBC cusror type supported by the driver.
	//--------------------------------------------------------------------

	public short getBestOdbcCursorType ();
	
	public boolean isFreeStmtsFromConnectionOnly(); // 4524683

}
