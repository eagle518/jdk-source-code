/*
 * @(#)OdbcDef.java	1.30 02/01/31
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      OdbcDef.java
//
// Description: Defines ODBC constants
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

import java.sql.Types;

public class OdbcDef {

	// SQL return codes

	public final static short SQL_SUCCESS = 0;
	public final static short SQL_SUCCESS_WITH_INFO = 1;
	public final static short SQL_NO_DATA = 100;
	public final static short SQL_NO_DATA_FOUND = 100;
	public final static short SQL_ERROR = -1;
	public final static short SQL_INVALID_HANDLE = -2;
	public final static short SQL_STILL_EXECUTING = 2;
	public final static short SQL_NEED_DATA = 99;

	// TRUE, FALSE
	
	public final static short SQL_TRUE  = 1;
	public final static short SQL_FALSE = 0;

	// C data types

	public final static short SQL_C_BINARY    = -2;
	public final static short SQL_C_BIT       = -7;
	public final static short SQL_C_BOOKMARK  = -18;
	public final static short SQL_C_CHAR      = 1;
	public final static short SQL_C_DATE      = 91;
	public final static short SQL_C_DEFAULT   = 99;
	public final static short SQL_C_DOUBLE	  = 8;
	public final static short SQL_C_FLOAT     = 7;
	public final static short SQL_C_LONG      = 4;
	public final static short SQL_C_SHORT	  = 5;
	public final static short SQL_C_SLONG     = -16;
	public final static short SQL_C_SSHORT    = -15;
	public final static short SQL_C_STINYINT  = -26;
	public final static short SQL_C_TIME      = 92;
	public final static short SQL_C_TIMESTAMP = 93;
	public final static short SQL_C_TINYINT   = -6;
	public final static short SQL_C_ULONG     = -18;
	public final static short SQL_C_USHORT    = -17;
	public final static short SQL_C_UTINYINT  = -28;
	public final static short SQL_C_SBIGINT   = -25;//4532162

	// SQL types
	public final static short SQL_CHAR	   = SQL_C_CHAR;
	public final static short SQL_DATE	   = 9;
	public final static short SQL_TIME	   = 10;
	public final static short SQL_TIMESTAMP	   = 11;
	public final static short SQL_TYPE_NULL    = 0;
	public final static short SQL_TYPE_UNKNOWN = 9999;

	// NULL handles

	public final static int	SQL_NULL_HENV = 0;
	public final static int	SQL_NULL_HDBC = 0;
	public final static int	SQL_NULL_HSTMT = 0;

	// Length indicator
	public final static int SQL_NULL_DATA	    = -1;
	public final static int SQL_DATA_AT_EXEC    = -2; 
	public final static int SQL_NTS		    = -3;
	public final static int SQL_NO_TOTAL	    = -4;
	public final static int SQL_DEFAULT_PARAM   = -5;	
	public final static int SQL_COLUMN_IGNORE   = -6;

	// SQLFreeStmt options
	public final static int SQL_CLOSE        = 0;
	public final static int SQL_DROP         = 1;
	public final static int SQL_UNBIND       = 2;
	public final static int SQL_RESET_PARAMS = 3;

	// Get/Set Connect options

	public final static short SQL_ACCESS_MODE = 101;
	   public final static int SQL_MODE_READ_WRITE = 0;
	   public final static int SQL_MODE_READ_ONLY  = 1;

	public final static short SQL_AUTOCOMMIT = 102;
	   public final static int SQL_AUTOCOMMIT_OFF = 0;
	   public final static int SQL_AUTOCOMMIT_ON  = 1;

	public final static short SQL_LOGIN_TIMEOUT = 103;
	public final static short SQL_TXN_ISOLATION = 108;
	public final static short SQL_CURRENT_QUALIFIER = 109;

	// GetInfo types

	public final static short SQL_ACTIVE_CONNECTIONS = 0;
	public final static short SQL_ACTIVE_STATEMENTS = 1;
	public final static short SQL_DRIVER_NAME = 6;
	public final static short SQL_DRIVER_VER = 7;
	public final static short SQL_ODBC_VER = 10;
	public final static short SQL_SEARCH_PATTERN_ESCAPE = 14;

	public final static short SQL_ODBC_SQL_CONFORMANCE = 15;
	   public final static short SQL_OSC_MINIMUM  = 0;
	   public final static short SQL_OSC_CORE     = 1;
	   public final static short SQL_OSC_EXTENDED = 2;

	public final static short SQL_DATABASE_NAME = 16;
	public final static short SQL_DBMS_NAME = 17;
	public final static short SQL_DBMS_VER = 18;
	public final static short SQL_ACCESSIBLE_TABLES = 19;
	public final static short SQL_ACCESSIBLE_PROCEDURES = 20;
	public final static short SQL_PROCEDURES = 21;

	public final static short SQL_CONCAT_NULL_BEHAVIOR = 22;
	   public final static short SQL_CB_NULL = 0; 

	public final static short SQL_CURSOR_COMMIT_BEHAVIOR = 23;
	public final static short SQL_CURSOR_ROLLBACK_BEHAVIOR = 24;
	   public final static short SQL_CB_DELETE   = 0;
	   public final static short SQL_CB_CLOSE    = 1;
	   public final static short SQL_CB_PRESERVE = 2;


	public final static short SQL_DATA_SOURCE_READ_ONLY = 25;
	public final static short SQL_DEFAULT_TXN_ISOLATION = 26;
	public final static short SQL_EXPRESSIONS_IN_ORDERBY = 27;

	public final static short SQL_IDENTIFIER_CASE = 28;
	   public final static short SQL_IC_UPPER     = 1;
	   public final static short SQL_IC_LOWER     = 2;
	   public final static short SQL_IC_SENSITIVE = 3;
	   public final static short SQL_IC_MIXED     = 4;

	public final static short SQL_IDENTIFIER_QUOTE_CHAR = 29;
	public final static short SQL_MAX_COLUMN_NAME_LEN = 30;
	public final static short SQL_MAX_CURSOR_NAME_LEN = 31;
	public final static short SQL_MAX_OWNER_NAME_LEN = 32;
	public final static short SQL_MAX_PROCEDURE_NAME_LEN = 33;
	public final static short SQL_MAX_QUALIFIER_NAME_LEN = 34;
	public final static short SQL_MAX_TABLE_NAME_LEN = 35;
	public final static short SQL_MULT_RESULT_SETS = 36;
	public final static short SQL_MULTIPLE_ACTIVE_TXN = 37;
	public final static short SQL_OUTER_JOINS = 38;
	public final static short SQL_OWNER_TERM = 39;
	public final static short SQL_PROCEDURE_TERM = 40;
	public final static short SQL_QUALIFIER_NAME_SEPARATOR = 41;
	public final static short SQL_QUALIFIER_TERM = 42;

	public final static short SQL_TXN_CAPABLE = 46;
	   public final static short SQL_TC_NONE        = 0;
	   public final static short SQL_TC_DML         = 1;
	   public final static short SQL_TC_ALL         = 2;
	   public final static short SQL_TC_DDL_COMMIT  = 3;
	   public final static short SQL_TC_DDL_IGNORE  = 4;

	public final static short SQL_USER_NAME = 47;

	public final static short SQL_CONVERT_FUNCTIONS = 48;
	   public final static int SQL_FN_CVT_CONVERT = 1;

	public final static short SQL_NUMERIC_FUNCTIONS = 49;
	   public final static int SQL_FN_NUM_ABS      = 0x00000001;
	   public final static int SQL_FN_NUM_ACOS     = 0x00000002;
	   public final static int SQL_FN_NUM_ASIN     = 0x00000004;
	   public final static int SQL_FN_NUM_ATAN     = 0x00000008;
	   public final static int SQL_FN_NUM_ATAN2    = 0x00000010;
	   public final static int SQL_FN_NUM_CEILING  = 0x00000020;
	   public final static int SQL_FN_NUM_COS      = 0x00000040;
	   public final static int SQL_FN_NUM_COT      = 0x00000080;
	   public final static int SQL_FN_NUM_EXP      = 0x00000100;
	   public final static int SQL_FN_NUM_FLOOR    = 0x00000200;
	   public final static int SQL_FN_NUM_LOG      = 0x00000400;
	   public final static int SQL_FN_NUM_MOD      = 0x00000800;
	   public final static int SQL_FN_NUM_SIGN     = 0x00001000;
	   public final static int SQL_FN_NUM_SIN      = 0x00002000;
	   public final static int SQL_FN_NUM_SQRT     = 0x00004000;
	   public final static int SQL_FN_NUM_TAN      = 0x00008000;
	   public final static int SQL_FN_NUM_PI       = 0x00010000;
	   public final static int SQL_FN_NUM_RAND     = 0x00020000;
	   public final static int SQL_FN_NUM_DEGREES  = 0x00040000;
	   public final static int SQL_FN_NUM_LOG10    = 0x00080000;
	   public final static int SQL_FN_NUM_POWER    = 0x00100000;
	   public final static int SQL_FN_NUM_RADIANS  = 0x00200000;
	   public final static int SQL_FN_NUM_ROUND    = 0x00400000;
	   public final static int SQL_FN_NUM_TRUNCATE = 0x00800000;

	public final static short SQL_STRING_FUNCTIONS = 50;
	   public final static int SQL_FN_STR_CONCAT     = 0x00000001;
	   public final static int SQL_FN_STR_INSERT     = 0x00000002;
	   public final static int SQL_FN_STR_LEFT       = 0x00000004;
	   public final static int SQL_FN_STR_LTRIM      = 0x00000008;
	   public final static int SQL_FN_STR_LENGTH     = 0x00000010;
	   public final static int SQL_FN_STR_LOCATE     = 0x00000020;
	   public final static int SQL_FN_STR_LCASE      = 0x00000040;
	   public final static int SQL_FN_STR_REPEAT     = 0x00000080;
	   public final static int SQL_FN_STR_REPLACE    = 0x00000100;
	   public final static int SQL_FN_STR_RIGHT      = 0x00000200;
	   public final static int SQL_FN_STR_RTRIM      = 0x00000400;
	   public final static int SQL_FN_STR_SUBSTRING  = 0x00000800;
	   public final static int SQL_FN_STR_UCASE      = 0x00001000;
	   public final static int SQL_FN_STR_ASCII      = 0x00002000;
	   public final static int SQL_FN_STR_CHAR       = 0x00004000;
	   public final static int SQL_FN_STR_DIFFERENCE = 0x00008000;
	   public final static int SQL_FN_STR_LOCATE_2   = 0x00010000;
	   public final static int SQL_FN_STR_SOUNDEX    = 0x00020000;
	   public final static int SQL_FN_STR_SPACE	 = 0x00040000;

	public final static short SQL_SYSTEM_FUNCTIONS = 51;
	   public final static int SQL_FN_SYS_USERNAME = 0x00000001;
	   public final static int SQL_FN_SYS_DBNAME   = 0x00000002;
	   public final static int SQL_FN_SYS_IFNULL   = 0x00000004;

	public final static short SQL_TIMEDATE_FUNCTIONS = 52;
	   public final static int SQL_FN_TD_NOW           = 0x00000001;
	   public final static int SQL_FN_TD_CURDATE       = 0x00000002;
	   public final static int SQL_FN_TD_DAYOFMONTH    = 0x00000004;
	   public final static int SQL_FN_TD_DAYOFWEEK     = 0x00000008;
	   public final static int SQL_FN_TD_DAYOFYEAR     = 0x00000010;
	   public final static int SQL_FN_TD_MONTH         = 0x00000020;
	   public final static int SQL_FN_TD_QUARTER       = 0x00000040;
	   public final static int SQL_FN_TD_WEEK          = 0x00000080;
	   public final static int SQL_FN_TD_YEAR          = 0x00000100;
	   public final static int SQL_FN_TD_CURTIME       = 0x00000200;
	   public final static int SQL_FN_TD_HOUR          = 0x00000400;
	   public final static int SQL_FN_TD_MINUTE        = 0x00000800;
	   public final static int SQL_FN_TD_SECOND        = 0x00001000;
	   public final static int SQL_FN_TD_TIMESTAMPADD  = 0x00002000;
	   public final static int SQL_FN_TD_TIMESTAMPDIFF = 0x00004000;
	   public final static int SQL_FN_TD_DAYNAME       = 0x00008000;
	   public final static int SQL_FN_TD_MONTHNAME     = 0x00010000;

	public final static short SQL_CONVERT_BIGINT = 53;
	public final static short SQL_CONVERT_BINARY = 54;
	public final static short SQL_CONVERT_BIT = 55;
	public final static short SQL_CONVERT_CHAR = 56;
	public final static short SQL_CONVERT_DATE = 57;
	public final static short SQL_CONVERT_DECIMAL = 58;
	public final static short SQL_CONVERT_DOUBLE = 59;
	public final static short SQL_CONVERT_FLOAT = 60;
	public final static short SQL_CONVERT_INTEGER = 61;
	public final static short SQL_CONVERT_LONGVARCHAR = 62;
	public final static short SQL_CONVERT_NUMERIC = 63;
	public final static short SQL_CONVERT_REAL = 64;
	public final static short SQL_CONVERT_SMALLINT = 65;
	public final static short SQL_CONVERT_TIME = 66;
	public final static short SQL_CONVERT_TIMESTAMP = 67;
	public final static short SQL_CONVERT_TINYINT = 68;
	public final static short SQL_CONVERT_VARBINARY = 69;
	public final static short SQL_CONVERT_VARCHAR = 70;
	public final static short SQL_CONVERT_LONGVARBINARY = 71;
	   public final static int SQL_CVT_CHAR          = 0x00000001;
	   public final static int SQL_CVT_NUMERIC       = 0x00000002;
	   public final static int SQL_CVT_DECIMAL       = 0x00000004;
	   public final static int SQL_CVT_INTEGER       = 0x00000008;
	   public final static int SQL_CVT_SMALLINT      = 0x00000010;
	   public final static int SQL_CVT_FLOAT         = 0x00000020;
	   public final static int SQL_CVT_REAL          = 0x00000040;
	   public final static int SQL_CVT_DOUBLE        = 0x00000080;
	   public final static int SQL_CVT_VARCHAR       = 0x00000100;
	   public final static int SQL_CVT_LONGVARCHAR   = 0x00000200;
	   public final static int SQL_CVT_BINARY        = 0x00000400;
	   public final static int SQL_CVT_VARBINARY     = 0x00000800;
	   public final static int SQL_CVT_BIT           = 0x00001000;
	   public final static int SQL_CVT_TINYINT       = 0x00002000;
	   public final static int SQL_CVT_BIGINT        = 0x00004000;
	   public final static int SQL_CVT_DATE          = 0x00008000;
	   public final static int SQL_CVT_TIME          = 0x00010000;
	   public final static int SQL_CVT_TIMESTAMP     = 0x00020000;
	   public final static int SQL_CVT_LONGVARBINARY = 0x00040000;


	public final static short SQL_TXN_ISOLATION_OPTION = 72;
	   public final static int SQL_TXN_READ_UNCOMMITTED  = 0x00000001;
	   public final static int SQL_TXN_READ_COMMITTED    = 0x00000002;
	   public final static int SQL_TXN_REPEATABLE_READ   = 0x00000004;
	   public final static int SQL_TXN_SERIALIZABLE      = 0x00000008;
	   public final static int SQL_TXN_VERSIONING        = 0x00000010;

	public final static short SQL_CORRELATION_NAME = 74;
	   public final static short SQL_CN_NONE      = 0;
	   public final static short SQL_CN_DIFFERENT = 1;
	   public final static short SQL_CN_ANY       = 2;

	public final static short SQL_ODBC_SQL_OPT_IEF = 73;

	public final static short SQL_NON_NULLABLE_COLUMNS = 75;
	   public final static short SQL_NNC_NULL     = 0;
	   public final static short SQL_NNC_NON_NULL = 1;

	public final static short SQL_POSITIONED_STATEMENTS = 80;
	   public final static int SQL_PS_POSITIONED_DELETE = 0x00000001;
	   public final static int SQL_PS_POSITIONED_UPDATE = 0x00000002;
	   public final static int SQL_PS_SELECT_FOR_UPDATE = 0x00000004;

	public final static short SQL_FILE_USAGE = 84;
	   public final static short SQL_FILE_TABLE     = 1;
	   public final static short SQL_FILE_QUALIFIER = 2;

	public final static short SQL_NULL_COLLATION = 85;
	   public final static short SQL_NC_HIGH  = 0;
	   public final static short SQL_NC_LOW   = 1;
	   public final static short SQL_NC_START = 2;
	   public final static short SQL_NC_END   = 4;

	public final static short SQL_ALTER_TABLE = 86;
	   public final static int SQL_AT_ADD_COLUMN  = 1;
	   public final static int SQL_AT_DROP_COLUMN = 2;

	public final static short SQL_COLUMN_ALIAS = 87;

	public final static short SQL_GROUP_BY = 88;
	   public final static short SQL_GB_NOT_SUPPORTED            = 0;
	   public final static short SQL_GB_GROUP_BY_EQUALS_SELECT   = 1;
	   public final static short SQL_GB_GROUP_BY_CONTAINS_SELECT = 2;
	   public final static short SQL_GB_NO_RELATION              = 3;

	public final static short SQL_KEYWORDS = 89;
	public final static short SQL_ORDER_BY_COLUMNS_IN_SELECT = 90;

	public final static short SQL_OWNER_USAGE = 91;
	   public final static int SQL_OU_DML_STATEMENTS       = 0x00000001;
	   public final static int SQL_OU_PROCEDURE_INVOCATION = 0x00000002;
	   public final static int SQL_OU_TABLE_DEFINITION     = 0x00000004;
	   public final static int SQL_OU_INDEX_DEFINITION     = 0x00000008;
	   public final static int SQL_OU_PRIVILEGE_DEFINITION = 0x00000010;

	public final static short SQL_QUALIFIER_USAGE = 92;
	   public final static int SQL_QU_DML_STATEMENTS       = 0x00000001;
	   public final static int SQL_QU_PROCEDURE_INVOCATION = 0x00000002;
	   public final static int SQL_QU_TABLE_DEFINITION     = 0x00000004;
	   public final static int SQL_QU_INDEX_DEFINITION     = 0x00000008;
	   public final static int SQL_QU_PRIVILEGE_DEFINITION = 0x00000010;

	public final static short SQL_QUOTED_IDENTIFIER_CASE = 93;
	public final static short SQL_SPECIAL_CHARACTERS = 94;

	public final static short SQL_SUBQUERIES = 95;
	   public final static int SQL_SQ_COMPARISON            = 0x00000001;
	   public final static int SQL_SQ_EXISTS                = 0x00000002;
	   public final static int SQL_SQ_IN                    = 0x00000004;
	   public final static int SQL_SQ_QUANTIFIED            = 0x00000008;
	   public final static int SQL_SQ_CORRELATED_SUBQUERIES = 0x00000010;

	public final static short SQL_UNION = 96;
	   public final static int SQL_U_UNION     = 1;
	   public final static int SQL_U_UNION_ALL = 2;

	public final static short SQL_MAX_COLUMNS_IN_GROUP_BY = 97;
	public final static short SQL_MAX_COLUMNS_IN_INDEX = 98;
	public final static short SQL_MAX_COLUMNS_IN_ORDER_BY = 99;
	public final static short SQL_MAX_COLUMNS_IN_SELECT = 100;
	public final static short SQL_MAX_COLUMNS_IN_TABLE = 101;
	public final static short SQL_MAX_INDEX_SIZE = 102;
	public final static short SQL_MAX_ROW_SIZE_INCLUDES_LONG = 103;
	public final static short SQL_MAX_ROW_SIZE = 104;
	public final static short SQL_MAX_STATEMENT_LEN = 105;
	public final static short SQL_MAX_TABLES_IN_SELECT = 106;
	public final static short SQL_MAX_USER_NAME_LEN = 107;
	public final static short SQL_MAX_CHAR_LITERAL_LEN = 108;
	public final static short SQL_MAX_BINARY_LITERAL_LEN = 112;
	public final static short SQL_LIKE_CLAUSE_ESCAPE = 113;

	public final static short SQL_QUALIFIER_LOCATION = 114;
	   public final static short SQL_QL_START = 1;
	   public final static short SQL_QL_END   = 2;

	//Info types needed for SQL BATCH support.
	
	public final static short SQL_BATCH_ROW_COUNT = 120;	     
	       public final static int SQL_BRC_PROCEDURES = 0x0000001;
	       public final static int SQL_BRC_EXPLICIT	  = 0x0000002;
	       public final static int SQL_BRC_ROLLED_UP  = 0x0000004;
	   
	// BS_SELECT intended for JDBC 2.0 (Only need BS_ROW_COUNT  2 and 8)

	public final static short SQL_BATCH_SUPPORT = 121;
	       public final static int SQL_BS_SELECT_EXPLICIT	 = 0x00000001;
	       public final static int SQL_BS_ROW_COUNT_EXPLICIT = 0x00000002;
	       public final static int SQL_BS_SELECT_PROC	 = 0x00000004;
	       public final static int SQL_BS_ROW_COUNT_PROC	 = 0x00000008;
	
	public final static short SQL_PARAM_ARRAY_ROW_COUNTS = 153;
	       public final static int SQL_PARC_BATCH = 1;
	       public final static int SQL_PARC_NO_BATCH = 2;
	   
		// PAS not intended for JDBC 2.0	
	public final static short SQL_PARAM_ARRAY_SELECTS = 154;
	       public final static int SQL_PAS_BATCH = 1;
	       public final static int SQL_PAS_NO_BATCH = 2;
	       public final static int SQL_PAS_NO_SELECT = 3;
	       
	// Batch Support.


	// SQLColAttributes

	public final static short SQL_COLUMN_NAME = 1;
	public final static short SQL_COLUMN_TYPE = 2;
	public final static short SQL_COLUMN_LENGTH = 3;
	public final static short SQL_COLUMN_PRECISION = 4;
	public final static short SQL_COLUMN_SCALE = 5;
	public final static short SQL_COLUMN_DISPLAY_SIZE = 6;

	public final static short SQL_COLUMN_NULLABLE = 7;
	   public final static short SQL_NULLABLE = 1;

	public final static short SQL_COLUMN_UNSIGNED = 8;
	public final static short SQL_COLUMN_MONEY = 9;

	public final static short SQL_COLUMN_UPDATABLE = 10;
	   public final static short SQL_ATTR_READONLY          = 0;
	   public final static short SQL_ATTR_WRITE             = 1;
	   public final static short SQL_ATTR_READWRITE_UNKNOWN = 2;

	public final static short SQL_COLUMN_AUTO_INCREMENT = 11;
	public final static short SQL_COLUMN_CASE_SENSITIVE = 12;

	public final static short SQL_COLUMN_SEARCHABLE = 13;
	   public final static short SQL_UNSEARCHABLE = 0;
	   public final static short SQL_SEARCHABLE = 3;

	public final static short SQL_COLUMN_TYPE_NAME = 14;
	public final static short SQL_COLUMN_TABLE_NAME = 15;
	public final static short SQL_COLUMN_OWNER_NAME = 16;
	public final static short SQL_COLUMN_QUALIFIER_NAME = 17;
	public final static short SQL_COLUMN_LABEL = 18;

	// SQL Get/Set StmtOption

	public final static short SQL_QUERY_TIMEOUT = 0;
	public final static short SQL_MAX_ROWS = 1;

	public final static short SQL_NOSCAN = 2;
	   public final static int SQL_NOSCAN_OFF = 0;
	   public final static int SQL_NOSCAN_ON = 1;

	public final static short SQL_MAX_LENGTH = 3;
	public final static short SQL_CONCURRENCY = 7;
	   public final static int SQL_CONCUR_READ_ONLY	= 1;
	   public final static int SQL_CONCUR_LOCK      = 2;
	   public final static int SQL_CONCUR_ROWVER    = 3;
	   public final static int SQL_CONCUR_VALUES    = 4;

	public final static short SQL_GET_BOOKMARK = 13;
	public final static short SQL_ROW_NUMBER = 14;

	public final static short SQL_ATTR_CURSOR_TYPE = 6;
	   public final static short SQL_CURSOR_FORWARD_ONLY  = 0;
	   public final static short SQL_CURSOR_KEYSET_DRIVEN = 1;
	   public final static short SQL_CURSOR_DYNAMIC       = 2;
	   public final static short SQL_CURSOR_STATIC        = 3;

	public final static short SQL_SCROLL_OPTIONS = 44;
	   public final static int SQL_SO_FORWARD_ONLY  = 0x00000001;
	   public final static int SQL_SO_KEYSET_DRIVEN = 0x00000002;
	   public final static int SQL_SO_DYNAMIC       = 0x00000004;
	   public final static int SQL_SO_MIXED         = 0x00000008;
	   public final static int SQL_SO_STATIC        = 0x00000010;

	public final static short SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1 = 146;
	public final static short SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2 = 147;
	public final static short SQL_STATIC_CURSOR_ATTRIBUTES1 = 167;
	public final static short SQL_STATIC_CURSOR_ATTRIBUTES2 = 168;
	public final static short SQL_KEYSET_CURSOR_ATTRIBUTES1 = 150;
	public final static short SQL_KEYSET_CURSOR_ATTRIBUTES2 = 151;
	public final static short SQL_DYNAMIC_CURSOR_ATTRIBUTES1 = 144;
	public final static short SQL_DYNAMIC_CURSOR_ATTRIBUTES2 = 145;
	   public final static int SQL_CA2_SENSITIVITY_ADDITIONS = 0x00000010;
	   public final static int SQL_CA2_SENSITIVITY_DELETIONS = 0x00000020;
	   public final static int SQL_CA2_SENSITIVITY_UPDATES = 0x00000040;


	// SQL Get/Set StmtAttr (to support Arrays of Parameters)
	// The first two are the same but for in different context.
	// Although the default is BIND_BY_COLUMN, we set it still
	// for both arrays of parameters and Resulset Rows.
	public final static int SQL_PARAM_BIND_BY_COLUMN    = 0;
	public final static int SQL_ROW_BIND_BY_COLUMN	    = 0;

	public final static int SQL_ATTR_QUERY_TIMEOUT	    = 0;
	public final static int SQL_ATTR_MAX_ROWS	    = 1;
	public final static int SQL_ATTR_NOSCAN		    = 2;
	public final static int SQL_ATTR_MAX_LENGTH	    = 3;
	public final static int SQL_ATTR_ASYNC_ENABLE	    = 4;

	public final static int SQL_ATTR_ROW_BIND_TYPE		    = 5;
	//public final static int SQL_ATTR_CURSOR_TYPE		    = 6; //see stmtOptions.
	public final static int SQL_ATTR_CONCURRENCY	    	    = 7;
	public final static int SQL_ATTR_KEYSET_SIZE		    = 8;
	public final static int SQL_ATTR_SIMULATE_CURSOR	    = 10;
	public final static int SQL_ATTR_RETRIEVE_DATA		    = 11;
	public final static int SQL_ATTR_USE_BOOKMARKS		    = 12;
	public final static int SQL_ATTR_ROW_NUMBER		    = 14;	  	/*GetStmtAttr*/
	public final static int SQL_ATTR_ENABLE_AUTO_IPD	    = 15;
	public final static int SQL_ATTR_FETCH_BOOKMARK_PTR	    = 16;
	public final static int SQL_ATTR_PARAM_BIND_OFFSET_PTR	    = 17;
	public final static int SQL_ATTR_PARAM_BIND_TYPE	    = 18;
	public final static int SQL_ATTR_PARAM_OPERATION_PTR	    = 19;
	public final static int SQL_ATTR_PARAM_STATUS_PTR	    = 20;
	public final static int SQL_ATTR_PARAMS_PROCESSED_PTR	    = 21;
	public final static int SQL_ATTR_PARAMSET_SIZE		    = 22;
	public final static int SQL_ATTR_ROW_BIND_OFFSET_PTR	    = 23;
	public final static int SQL_ATTR_ROW_OPERATION_PTR	    = 24;
	public final static int SQL_ATTR_ROW_STATUS_PTR		    = 25;
	public final static int SQL_ATTR_ROWS_FETCHED_PTR	    = 26;
	public final static int SQL_ATTR_ROW_ARRAY_SIZE		    = 27;

	// SQLFetchScroll

	public final static short SQL_FETCH_NEXT = 1;
	public final static short SQL_FETCH_FIRST = 2;
	public final static short SQL_FETCH_LAST = 3;
	public final static short SQL_FETCH_PRIOR = 4;
	public final static short SQL_FETCH_ABSOLUTE = 5;
	public final static short SQL_FETCH_RELATIVE = 6;
	
	// SQLSpecialColumns

	public final static short SQL_BEST_ROWID = 1;
	public final static short SQL_ROWVER = 2;

	// SQLGetTypeInfo

	public final static short SQL_ALL_TYPES = 0;

	// SQLTransact

	public final static short SQL_COMMIT   = 0;
	public final static short SQL_ROLLBACK = 1;


	//SQLBindCol and SQLSetPos

	public final static int SQL_LOCK_NO_CHANGE  = 0;
	public final static int SQL_LOCK_EXCLUSIVE  = 1;
	public final static int SQL_LOCK_UNLOCK	    = 2;


	public final static int SQL_ENTIRE_ROWSET   = 0;
	
	public final static int SQL_POSITION	= 0;
	public final static int SQL_REFRESH	= 1;
	public final static int SQL_UPDATE	= 2;
	public final static int SQL_DELETE	= 3;

	//SQLBulkOperations
	public final static int SQL_ADD		= 4;

	//SQL RowStatus after position Updates.
	public final static int SQL_ROW_SUCCESS	= 0;
	public final static int SQL_ROW_DELETED	= 1;
	public final static int SQL_ROW_UPDATED	= 2;
	public final static int SQL_ROW_NOROW	= 3;
	public final static int SQL_ROW_ADDED	= 4;
	public final static int SQL_ROW_ERROR	= 5;


	// SQLMoreResults status flags!
	public final static int RESULTS_NOT_SET	    = 1;
	public final static int HAS_MORE_RESULTS    = 2;
	public final static int NO_MORE_RESULTS	    = 3;


	//--------------------------------------------------------------------
	// odbcTypeToJdbc
	// Convert the ODBC SQL type to the correct JDBC type
	//--------------------------------------------------------------------
	public static int odbcTypeToJdbc (
		int odbcType)
	{
		// For the most part, JDBC types match ODBC types.  We'll
		// just convert the ones that we know are different

		int jdbcType = odbcType;
		
		switch (odbcType) {		
		case OdbcDef.SQL_DATE:
			jdbcType = Types.DATE;
			break;
		case OdbcDef.SQL_TIME:
			jdbcType = Types.TIME;
			break;
		case OdbcDef.SQL_TIMESTAMP:
			jdbcType = Types.TIMESTAMP;
			break;
		default:
			if (odbcType >= 0 && odbcType <= 8)
				jdbcType = odbcType;
			else if (odbcType == 12)
				jdbcType = odbcType;
			else if (odbcType <= -1 && odbcType >= -10)
				jdbcType = odbcType;
			else
				jdbcType = Types.OTHER;
		}

		return jdbcType;
	}

	//--------------------------------------------------------------------
	// jdbcTypeToOdbc
	// Convert the JDBC SQL type to the correct ODBC type
	//--------------------------------------------------------------------
	public static int jdbcTypeToOdbc (
		int jdbcType)
	{
		// For the most part, JDBC types match ODBC types.  We'll
		// just convert the ones that we know are different

		int odbcType = jdbcType;
		
		switch (jdbcType) {
		case Types.CHAR:
		case Types.VARCHAR:
			//commented out as fix for Oracle precision problems
			//odbcType = OdbcDef.SQL_CHAR;
			break;
		case Types.DATE:
			odbcType = OdbcDef.SQL_DATE;
			break;
		case Types.TIME:
			odbcType = OdbcDef.SQL_TIME;
			break;
		case Types.TIMESTAMP:
			odbcType = OdbcDef.SQL_TIMESTAMP;
			break;
		}

		return odbcType;
	}


	// bug 4412437
        public static int jdbcTypeToCType(int jdbcType)
        {
                switch(jdbcType)
                {
                        case Types.INTEGER:
                                return SQL_C_SLONG;

                        case Types.SMALLINT:
                                return SQL_C_SLONG;

                        case Types.TINYINT:
                                return SQL_C_SLONG;

                        case Types.BIT:
                                return SQL_C_SLONG;

                        case Types.BIGINT:
                                return SQL_C_SBIGINT;

                        case Types.REAL:
                                return SQL_C_FLOAT;

                        case Types.FLOAT:
                                return SQL_C_FLOAT;

                        case Types.DOUBLE:
                                return SQL_C_DOUBLE;

                        case Types.CHAR:
                        case Types.VARCHAR:
                                return SQL_C_CHAR;

                        default:
                                return 0;

                }
        }


	// INTERSOLV SQLSetConnectionOption extensions (1040 to 1139)
	public final static short SQL_LIC_FILE_NAME     = 1041;
	public final static short SQL_LIC_FILE_PASSWORD = 1042;
}

