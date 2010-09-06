/*
 * @(#)JdbcOdbcLimits.java	1.29 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcLimits.java
//
// Description: Maximum limits for various functions
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


public class JdbcOdbcLimits {

	// Maximum column width for a single getData

	public static final int MAX_GET_DATA_LENGTH = 32767;

	// Default buffer size when retrieving options/attributes/errors

	public static final int DEFAULT_BUFFER_LENGTH = 300;

	// Default precision when binding parameters
	
	public static final int DEFAULT_IN_PRECISION = 8000;

	// Maximum length of native SQL conversion

	public static final int DEFAULT_NATIVE_SQL_LENGTH = 1024;

	// Maximum length of PutData buffer

	public static final int MAX_PUT_DATA_LENGTH = 2000;

	// Maximum length of attribute strings returned by SQLBrowseConnect

	public static final int MAX_BROWSE_RESULT_LENGTH = 2000;

	// Default scale for out parameters

	public static final int DEFAULT_OUT_SCALE = 4;

	// Default resultSet's block-cursor size

	public static final int DEFAULT_ROW_SET = 10;


}


