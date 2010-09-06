/*
 * @(#)JdbcOdbcDriverInterface.java	1.29 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcDriverInterface.java
//
// Description: Definition of the JdbcOdbcDriver interface.  This is
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

public interface JdbcOdbcDriverInterface
	extends		java.sql.Driver
{
	//--------------------------------------------------------------------
	// allocConnection
	// Allocate a connection handle.  Once the handle is allocated, keep
	// it in our list if allocated connection handles.  This list will be
	// used to determine how many connection handles are active.  When
	// closing a connection handle (closeConnection), if the number of
	// handles is zero, the driver can be closed (i.e. the environment
	// handle can be free'ed
	//--------------------------------------------------------------------

	public long allocConnection (
		long env)
		throws SQLException;

	//--------------------------------------------------------------------
	// closeConnection
	// Close the given connection handle, and remove it from our list
	// of connection handles.  If it is the last connection handle in
	// the list, the environment can also be closed
	//--------------------------------------------------------------------

	public void closeConnection (
		long dbc)
		throws SQLException;

	//--------------------------------------------------------------------
	// disconnect
	// Close the given connection handle, disconnect
	//--------------------------------------------------------------------

	public void disconnect (
		long dbc)
		throws SQLException;

}
