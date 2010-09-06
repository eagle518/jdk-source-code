/*
 * @(#)JdbcOdbcSQLWarning.java	1.27 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcSQLWarning.java
//
// Description:	Extension of the SQLWarning class.  Typically, if a warning
//              is issued, there is valid data that needs to be returned
//              to the calling object.  Since the warning is caught as a
//              Java exception, the normal return value cannot be used.  This
//              class exetends the SQLWarning class and provides value 
//              holders in case of a warning.
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

import java.net.URL;
import java.net.MalformedURLException;
import java.sql.*;

public class JdbcOdbcSQLWarning
	extends		SQLWarning {

	public JdbcOdbcSQLWarning (String reason, String SQLState,
			int vendorCode)
	{
		super (reason, SQLState, vendorCode);
	}

	public JdbcOdbcSQLWarning (String reason, String SQLState)
	{
		super (reason, SQLState);
	}

	public JdbcOdbcSQLWarning (String reason)
	{
		super (reason);
	}

	public JdbcOdbcSQLWarning ()
	{
		super ();
	}

	//====================================================================
	// Data attributes
	//====================================================================

	Object value;				// Generic value object
}
