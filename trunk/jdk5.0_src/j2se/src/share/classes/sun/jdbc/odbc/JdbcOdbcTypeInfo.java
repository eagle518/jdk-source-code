/*
 * @(#)JdbcOdbcTypeInfo.java	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcTypeInfo.java
//
// Description: Class for a type info column.  This class is used a place
//              holder for information needed for each SQL type.
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Karl Moss
//
// Date:        June, 1996
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.sql.*;

public class JdbcOdbcTypeInfo
	extends	JdbcOdbcObject {

	//--------------------------------------------------------------------
	// setName
	// Sets the SQL type name
	//--------------------------------------------------------------------

	public void setName (
		String name)
	{	
		typeName = name;
	}

	//--------------------------------------------------------------------
	// getName
	// Gets the SQL type name
	//--------------------------------------------------------------------

	public String getName ()
	{	
		return typeName;
	}

	//--------------------------------------------------------------------
	// setPrec
	// Sets the precision
	//--------------------------------------------------------------------

	public void setPrec (
		int prec)
	{	
		precision = prec;
	}

	//--------------------------------------------------------------------
	// getPrec
	// Gets the precision
	//--------------------------------------------------------------------

	public int getPrec ()
	{	
		return precision;
	}

	//====================================================================
	// Data attributes
	//====================================================================

	String typeName;			// SQL type name

	int precision;				// Precision
}
