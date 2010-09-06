/*
 * @(#)JdbcOdbcBatchUpdateException.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcBatchUpdateException.java
//
// Description:	Extension of the BatchUpdateException class.  
//		Typically, if a error occured in the Statement 
//		method executeBatch, there may still be valid 
//		valid data that needs to be returned to the calling object.
//		Since the Update error is caught as a Java exception, 
//		the normal return value cannot be used.  This
//              class exetends the SQLException class and provides value 
//              holder for Batch Updates that wher successfull before the
//		erro occured.
//
// Product:     JDBC-ODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Dezi Siliezar
//
// Date:        March, 1999
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.sql.*;

public class JdbcOdbcBatchUpdateException 
	extends BatchUpdateException {

	public JdbcOdbcBatchUpdateException (String reason, String SQLState, int vendorCode, int[] updateCounts)
	{
		super (reason, SQLState, vendorCode, updateCounts);
		this.exceptionCounts = updateCounts;
	}

	public JdbcOdbcBatchUpdateException (String reason, String SQLState, int[] updateCounts)
	{
		super (reason, SQLState, updateCounts);
		this.exceptionCounts = updateCounts;
	}

	public JdbcOdbcBatchUpdateException (String reason, int[] updateCounts)
	{
		super (reason, updateCounts);
		this.exceptionCounts = updateCounts;
	}

	public JdbcOdbcBatchUpdateException (int[] updateCounts)
	{
		super (updateCounts);
		this.exceptionCounts = updateCounts;
	}

	public JdbcOdbcBatchUpdateException ()
	{
		super ();
	}

	public int[] getUpdateCounts()
	{
		return exceptionCounts;
	}

	//====================================================================
	// Data attributes
	//====================================================================

	int [] exceptionCounts;
}
