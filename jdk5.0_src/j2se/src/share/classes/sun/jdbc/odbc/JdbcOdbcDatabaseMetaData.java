/*
 * @(#)JdbcOdbcDatabaseMetaData.java	1.36 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcDatabaseMetaData.java
//
// Description: Impementation of the DatabaseMetaData interface class
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

public class JdbcOdbcDatabaseMetaData
        extends         JdbcOdbcObject
        implements      DatabaseMetaData {

        //====================================================================
        // Public methods
        //====================================================================

        //--------------------------------------------------------------------
        // Constructor
        // Perform any necessary initialization.
        //--------------------------------------------------------------------

        public JdbcOdbcDatabaseMetaData (
                JdbcOdbc odbcApi,
                JdbcOdbcConnectionInterface con)
        {
                // Save a pointer to the ODBC api and the connection object

                OdbcApi = odbcApi;
                Con = con;

                // Set the connection handle

                hDbc = Con.getHDBC ();
        }


        //--------------------------------------------------------------------
        // allProceduresAreCallable
        // Can all the procedures returned by getProcedures be called by the
        // current user?
        //--------------------------------------------------------------------

        public boolean allProceduresAreCallable ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.allProceduresAreCallable");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_ACCESSIBLE_PROCEDURES);
        }

        //--------------------------------------------------------------------
        // allTablesAreSelectable
        // Can all the table returned by getTable be SELECTed by the current
        // user?
        //--------------------------------------------------------------------

        public boolean allTablesAreSelectable ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.allTablesAreSelectable");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_ACCESSIBLE_TABLES);
        }
                

        //--------------------------------------------------------------------
        // If possible return the url for the database.
        // This should return java null if we can't generate a url.
        //--------------------------------------------------------------------

        public String getURL ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getURL");
                }
                return Con.getURL ();
        }

        //--------------------------------------------------------------------
        // getUserName
        // get our user name as known to the database:
        //--------------------------------------------------------------------

        public String getUserName ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getUserName");
                }
                return getInfoString (OdbcDef.SQL_USER_NAME);
        }

        //--------------------------------------------------------------------
        // isReadOnly
        // is the connection in read-only mode?
        //--------------------------------------------------------------------

        public boolean isReadOnly ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.isReadOnly");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_DATA_SOURCE_READ_ONLY);
        }

        //--------------------------------------------------------------------
        // nullsAreSortedHigh
        // Nulls are sorted at the high end of the list
        //--------------------------------------------------------------------

        public boolean nullsAreSortedHigh ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.nullsAreSortedHigh");
                }
                int value = getInfoShort (OdbcDef.SQL_NULL_COLLATION);

                return (value == OdbcDef.SQL_NC_HIGH);
        }

        //--------------------------------------------------------------------
        // nullsAreSortedLow
        // Nulls are sorted at the low end of the list
        //--------------------------------------------------------------------

        public boolean nullsAreSortedLow ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.nullsAreSortedLow");
                }
                int value = getInfo (OdbcDef.SQL_NULL_COLLATION);

                return (value == OdbcDef.SQL_NC_LOW);
        }

        //--------------------------------------------------------------------
        // nullsAreSortedAtStart
        // Nulls are sorted at the start of the list, regardless of the sort
        // order
        //--------------------------------------------------------------------

        public boolean nullsAreSortedAtStart ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.nullsAreSortedAtStart");
                }
                int value = getInfo (OdbcDef.SQL_NULL_COLLATION);

                return (value == OdbcDef.SQL_NC_START);
        }

        //--------------------------------------------------------------------
        // nullsAreSortedAtEnd
        // Nulls are sorted at the end of the list, regardless of the sort
        // order
        //--------------------------------------------------------------------

        public boolean nullsAreSortedAtEnd ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.nullsAreSortedAtEnd");
                }
                int value = getInfo (OdbcDef.SQL_NULL_COLLATION);

                return (value == OdbcDef.SQL_NC_END);
        }

        //--------------------------------------------------------------------
        // getDatabaseProductName
        // Get the name of the underlying database product.
        //--------------------------------------------------------------------

        public String getDatabaseProductName ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getDatabaseProductName");
                }
                return getInfoString (OdbcDef.SQL_DBMS_NAME);
        }

        //--------------------------------------------------------------------
        // getDatabaseProductVersion
        // get the database product version.
        //--------------------------------------------------------------------

        public String getDatabaseProductVersion ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getDatabaseProductVersion");
                }
                return getInfoString (OdbcDef.SQL_DBMS_VER);
        }

        //--------------------------------------------------------------------
        // getDriverName
        // get the JDBC driver name.  We will return JDBC-ODBC Bridge (name)
        // where name is the ODBC driver name
        //--------------------------------------------------------------------

        public String getDriverName ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getDriverName");
                }
                return "JDBC-ODBC Bridge (" +
                        getInfoString (OdbcDef.SQL_DRIVER_NAME) + ")";
        }

        //--------------------------------------------------------------------
        // getDriverVersion
        // get the JDBC driver version as a String.  We will return the
        // major and minor version number of the driver, as well as
        // the version number of the ODBC driver.
        //--------------------------------------------------------------------

        public String getDriverVersion ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getDriverVersion");
                }
                int minor = getDriverMinorVersion ();
                String s = "";

                // Format the minor version to have 4 digits,
                // with leading 0's if necessary

                if (minor < 1000) s += "0";
                if (minor < 100)  s += "0";
                if (minor < 10)   s += "0";
                s += "" + minor;

                return "" + getDriverMajorVersion () +
                        "." + s + " (" +
                        getInfoString (OdbcDef.SQL_DRIVER_VER) + ")";
        }

        //--------------------------------------------------------------------
        // getDriverMajorVersion
        // Return the JDBC major version
        //--------------------------------------------------------------------

        public int getDriverMajorVersion ()
        {
                return JdbcOdbc.MajorVersion;
        }

        //--------------------------------------------------------------------
        // getDriverMinorVersion
        // Return the JDBC minor version
        //--------------------------------------------------------------------

        public int getDriverMinorVersion ()
        {
                return JdbcOdbc.MinorVersion;
        }

        //--------------------------------------------------------------------
        // usesLocalFiles
        // Does the database store each table in a local file?
        //--------------------------------------------------------------------

        public boolean usesLocalFiles ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.usesLocalFiles");
                }
                int value = getInfoShort (OdbcDef.SQL_FILE_USAGE);

                return (value == OdbcDef.SQL_FILE_QUALIFIER);
        }

        // Does the database use a file for each table?
        public boolean usesLocalFilePerTable ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.usesLocalFilePerTable");
                }
                int value = getInfoShort (OdbcDef.SQL_FILE_USAGE);

                return (value == OdbcDef.SQL_FILE_TABLE);
        }

        //--------------------------------------------------------------------
        // supportsMixedCaseIndentifiers
        // Does the database support mixed case unquoted SQL identifiers?
        //--------------------------------------------------------------------

        public boolean supportsMixedCaseIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsMixedCaseIdentifiers");
                }
                int value = getInfoShort (OdbcDef.SQL_IDENTIFIER_CASE);
/*
		// sun's 4234316 fix.
		// According to JDBC_ODBC spec., A JDBC Compliant Driver
		// will let this method return <false>.
		// Identifiers in SQL_IC_MIXED are not case-sensitive.
		// Identifiers in SQL_IC_SENSITIVE are case-sensitive.

                return ((value == OdbcDef.SQL_IC_MIXED) ||
                    (value == OdbcDef.SQL_IC_UPPER) ||
                    (value == OdbcDef.SQL_IC_LOWER));
*/
                return ( value == OdbcDef.SQL_IC_SENSITIVE );

        }

        //--------------------------------------------------------------------
        // storesUpperCaseIdentifiers
        // Are identifiers stored in upper case?
        //--------------------------------------------------------------------

        public boolean storesUpperCaseIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesUpperCaseIdentifiers");
                }
                int value = getInfoShort (OdbcDef.SQL_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_UPPER);
        }

        //--------------------------------------------------------------------
        // storesLowerCaseIdentifiers
        // Are identifiers stored in lower case?
        //--------------------------------------------------------------------

        public boolean storesLowerCaseIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesLowerCaseIdentifiers");
                }
                int value = getInfoShort (OdbcDef.SQL_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_LOWER);
        }

        //--------------------------------------------------------------------
        // storesMixedCaseIdentifiers
        // Are identifiers stored in mixed case?
        //--------------------------------------------------------------------

        public boolean storesMixedCaseIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesMixedCaseIdentifiers");
                }
                int value = getInfoShort (OdbcDef.SQL_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_MIXED);
        }

        //--------------------------------------------------------------------
        // supportsMixedCaseQuotedIdentifiers
        // Does the database support mixed case quoted SQL identifiers?
        //--------------------------------------------------------------------

        public boolean supportsMixedCaseQuotedIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsMixedCaseQuotedIdentifiers");
                }
                int value = getInfoShort(OdbcDef.SQL_QUOTED_IDENTIFIER_CASE);
/*		
		// sun's 4234316 fix.
		// According to JDBC_ODBC spec., A JDBC Compliant Driver
		// will let this method return <true>.
		// Identifiers in SQL_IC_MIXED are not case-sensitive.
		// Identifiers in SQL_IC_SENSITIVE are case-sensitive.

                return ((value == OdbcDef.SQL_IC_MIXED) ||
                    (value == OdbcDef.SQL_IC_UPPER) ||
                    (value == OdbcDef.SQL_IC_LOWER));
*/
                return ( value == OdbcDef.SQL_IC_SENSITIVE );

        }

        //--------------------------------------------------------------------
        // storesUpperCaseQuotedIdentifiers
        // Are quoted identifiers stored in upper case?
        //--------------------------------------------------------------------

        public boolean storesUpperCaseQuotedIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesUpperCaseQuotedIdentifiers");
                }
                int value = getInfoShort(OdbcDef.SQL_QUOTED_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_UPPER);
        }

        //--------------------------------------------------------------------
        // storesLowerCaseQuotedIdentifiers
        // Are quoted identifiers stored in lower case?
        //--------------------------------------------------------------------

        public boolean storesLowerCaseQuotedIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesLowerCaseQuotedIdentifiers");
                }
                int value = getInfoShort(OdbcDef.SQL_QUOTED_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_LOWER);
        }

        //--------------------------------------------------------------------
        // storesMixedCaseIdentifiers
        // Are quoted identifiers stored in mixed case?
        //--------------------------------------------------------------------

        public boolean storesMixedCaseQuotedIdentifiers ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.storesMixedCaseQuotedIdentifiers");
                }
                int value = getInfoShort(OdbcDef.SQL_QUOTED_IDENTIFIER_CASE);

                return (value == OdbcDef.SQL_IC_MIXED);
        }

        //--------------------------------------------------------------------
        // getIdentifierQuoteString
        // quote string used to surround quoted SQL identifiers?
        //--------------------------------------------------------------------

        public String getIdentifierQuoteString ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getIdentifierQuoteString");
                }
                return getInfoString (OdbcDef.SQL_IDENTIFIER_QUOTE_CHAR);
        }

        //--------------------------------------------------------------------
        // getSQLKeywords
        // get a comma separated list of all database specific SQL keywords.
        //--------------------------------------------------------------------

        public String getSQLKeywords ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getSQLKeywords");
                }
                return getInfoString (OdbcDef.SQL_KEYWORDS,
                                JdbcOdbcLimits.MAX_GET_DATA_LENGTH / 2);
        }

        //--------------------------------------------------------------------
        // getNumericFunctions
        // Return a comma separated list of math functions.
        //--------------------------------------------------------------------

        public String getNumericFunctions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getNumericFunctions");
                }

                // Bit mask
                int m;
                String list = "";

                m = getInfo (OdbcDef.SQL_NUMERIC_FUNCTIONS);

                // Create a list of stanard function names (i.e. not the
                // native function names)

                if ((m & OdbcDef.SQL_FN_NUM_ABS) != 0)      list+="ABS,";
                if ((m & OdbcDef.SQL_FN_NUM_ACOS) != 0)     list+="ACOS,";
                if ((m & OdbcDef.SQL_FN_NUM_ASIN) != 0)     list+="ASIN,";
                if ((m & OdbcDef.SQL_FN_NUM_ATAN) != 0)     list+="ATAN,";
                if ((m & OdbcDef.SQL_FN_NUM_ATAN2) != 0)    list+="ATAN2,";
                if ((m & OdbcDef.SQL_FN_NUM_CEILING) !=0)   list+="CEILING,";
                if ((m & OdbcDef.SQL_FN_NUM_COS) != 0)      list+="COS,";
                if ((m & OdbcDef.SQL_FN_NUM_COT) != 0)      list+="COT,";
                if ((m & OdbcDef.SQL_FN_NUM_DEGREES) !=0)   list+="DEGREES,";
                if ((m & OdbcDef.SQL_FN_NUM_EXP) != 0)      list+="EXP,";
                if ((m & OdbcDef.SQL_FN_NUM_FLOOR) != 0)    list+="FLOOR,";
                if ((m & OdbcDef.SQL_FN_NUM_LOG) != 0)      list+="LOG,";
                if ((m & OdbcDef.SQL_FN_NUM_LOG10) != 0)    list+="LOG10,";
                if ((m & OdbcDef.SQL_FN_NUM_MOD) != 0)      list+="MOD,";
                if ((m & OdbcDef.SQL_FN_NUM_PI) != 0)       list+="PI,";
                if ((m & OdbcDef.SQL_FN_NUM_POWER) != 0)    list+="POWER,";
                if ((m & OdbcDef.SQL_FN_NUM_RADIANS) !=0)   list+="RADIANS,";
                if ((m & OdbcDef.SQL_FN_NUM_RAND) != 0)     list+="RAND,";
                if ((m & OdbcDef.SQL_FN_NUM_ROUND) != 0)    list+="ROUND,";
                if ((m & OdbcDef.SQL_FN_NUM_SIGN) != 0)     list+="SIGN,";
                if ((m & OdbcDef.SQL_FN_NUM_SIN) != 0)      list+="SIN,";
                if ((m & OdbcDef.SQL_FN_NUM_SQRT) != 0)     list+="SQRT,";
                if ((m & OdbcDef.SQL_FN_NUM_TAN) != 0)      list+="TAN,";
                if ((m & OdbcDef.SQL_FN_NUM_TRUNCATE) != 0) list+="TRUNCATE,";

                // Since we always added a comma after each function name,
                // strip off the last one (if necessary)
                if (list.length () > 0) {
                        list = list.substring (0, list.length () - 1);
                }
                return list;
        }

        //--------------------------------------------------------------------
        // getStringFunctions
        // Return a comma separated list of string functions
        //--------------------------------------------------------------------

        public String getStringFunctions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getStringFunctions");
                }

                // Bit mask
                int m;
                String list = "";

                m = getInfo (OdbcDef.SQL_STRING_FUNCTIONS);

                // Create a list of stanard function names (i.e. not the
                // native function names)

                if ((m & OdbcDef.SQL_FN_STR_ASCII) != 0)     list+="ASCII,";
                if ((m & OdbcDef.SQL_FN_STR_CHAR) != 0)      list+="CHAR,";
                if ((m & OdbcDef.SQL_FN_STR_CONCAT) != 0)    list+="CONCAT,";
                if ((m & OdbcDef.SQL_FN_STR_DIFFERENCE)!=0)list+="DIFFERENCE,";
                if ((m & OdbcDef.SQL_FN_STR_INSERT) != 0)    list+="INSERT,";
                if ((m & OdbcDef.SQL_FN_STR_LCASE) != 0)     list+="LCASE,";
                if ((m & OdbcDef.SQL_FN_STR_LEFT) != 0)      list+="LEFT,";
                if ((m & OdbcDef.SQL_FN_STR_LENGTH) != 0)    list+="LENGTH,";
                if ((m & OdbcDef.SQL_FN_STR_LOCATE) != 0)    list+="LOCATE,";
                if ((m & OdbcDef.SQL_FN_STR_LOCATE_2) != 0)  list+="LOCATE_2,";
                if ((m & OdbcDef.SQL_FN_STR_LTRIM) != 0)     list+="LTRIM,";
                if ((m & OdbcDef.SQL_FN_STR_REPEAT) != 0)    list+="REPEAT,";
                if ((m & OdbcDef.SQL_FN_STR_REPLACE) != 0)   list+="REPLACE,";
                if ((m & OdbcDef.SQL_FN_STR_RIGHT) != 0)     list+="RIGHT,";
                if ((m & OdbcDef.SQL_FN_STR_RTRIM) != 0)     list+="RTRIM,";
                if ((m & OdbcDef.SQL_FN_STR_SOUNDEX) != 0)   list+="SOUNDEX,";
                if ((m & OdbcDef.SQL_FN_STR_SPACE) != 0)     list+="SPACE,";
                if ((m & OdbcDef.SQL_FN_STR_SUBSTRING)!=0)  list+="SUBSTRING,";
                if ((m & OdbcDef.SQL_FN_STR_UCASE) != 0)     list+="UCASE,";

                // Since we always added a comma after each function name,
                // strip off the last one (if necessary)
                if (list.length () > 0) {
                        list = list.substring (0, list.length () - 1);
                }
                return list;
        }

        //--------------------------------------------------------------------
        // getSystemFunctions
        // Return a comma separated list of system functions
        //--------------------------------------------------------------------

        public String getSystemFunctions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getSystemFunctions");
                }

                // Bit mask
                int m;
                String list = "";

                m = getInfo (OdbcDef.SQL_SYSTEM_FUNCTIONS);

                // Create a list of stanard function names (i.e. not the
                // native function names)

                if ((m & OdbcDef.SQL_FN_SYS_DBNAME) != 0)    list+="DBNAME,";
                if ((m & OdbcDef.SQL_FN_SYS_IFNULL) != 0)    list+="IFNULL,";
                if ((m & OdbcDef.SQL_FN_SYS_USERNAME) != 0)  list+="USERNAME,";

                // Since we always added a comma after each function name,
                // strip off the last one (if necessary)
                if (list.length () > 0) {
                        list = list.substring (0, list.length () - 1);
                }
                return list;
        }

        //--------------------------------------------------------------------
        // getTimeDateFunctions 
        // Returns a comma separated list of time and date functions
        //--------------------------------------------------------------------

        public String getTimeDateFunctions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getTimeDateFunctions");
                }

                // Bit mask
                int m;
                String list = "";

                m = getInfo (OdbcDef.SQL_TIMEDATE_FUNCTIONS);

                // Create a list of stanard function names (i.e. not the
                // native function names)

                if ((m & OdbcDef.SQL_FN_TD_CURDATE) != 0)  list+="CURDATE,";
                if ((m & OdbcDef.SQL_FN_TD_CURTIME) != 0)  list+="CURTIME,";
                if ((m & OdbcDef.SQL_FN_TD_DAYNAME) != 0)  list+="DAYNAME,";
                if ((m & OdbcDef.SQL_FN_TD_DAYOFMONTH)!=0) list+="DAYOFMONTH,";
                if ((m & OdbcDef.SQL_FN_TD_DAYOFWEEK) !=0) list+="DAYOFWEEK,";
                if ((m & OdbcDef.SQL_FN_TD_DAYOFYEAR) !=0) list+="DAYOFYEAR,";
                if ((m & OdbcDef.SQL_FN_TD_HOUR) != 0)     list+="HOUR,";
                if ((m & OdbcDef.SQL_FN_TD_MINUTE) != 0)   list+="MINUTE,";
                if ((m & OdbcDef.SQL_FN_TD_MONTH) != 0)    list+="MONTH,";
                if ((m & OdbcDef.SQL_FN_TD_MONTHNAME) !=0) list+="MONTHNAME,";
                if ((m & OdbcDef.SQL_FN_TD_NOW) != 0)      list+="NOW,";
                if ((m & OdbcDef.SQL_FN_TD_QUARTER) != 0)  list+="QUARTER,";
                if ((m & OdbcDef.SQL_FN_TD_SECOND) != 0)   list+="SECOND,";
                if ((m & OdbcDef.SQL_FN_TD_TIMESTAMPADD) != 0)
                                                list+="TIMESTAMPADD,";
                if ((m & OdbcDef.SQL_FN_TD_TIMESTAMPDIFF) != 0)
                                                list+="TIMESTAMPDIFF,";
                if ((m & OdbcDef.SQL_FN_TD_WEEK) != 0)     list+="WEEK,";
                if ((m & OdbcDef.SQL_FN_TD_YEAR) != 0)     list+="YEAR,";


                // Since we always added a comma after each function name,
                // strip off the last one (if necessary)
                if (list.length () > 0) {
                        list = list.substring (0, list.length () - 1);
                }
                return list;
        }

        //--------------------------------------------------------------------
        // getSearchStringEscape
        // This is the string that can be used to escape '_' or '%' in
        // the string pattern based seacrhes such as getTables.
        //--------------------------------------------------------------------

        public String getSearchStringEscape ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getSearchStringEscape");
                }
                return getInfoString (OdbcDef.SQL_SEARCH_PATTERN_ESCAPE);
        }

        //--------------------------------------------------------------------
        // getExtraNameCharacters
        // String of all the "extra" charactes tha can be used in
        // identifier names, i.e. those beyond a-z, 0-9 and _.
        //--------------------------------------------------------------------

        public String getExtraNameCharacters ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getExtraNameCharacters");
                }
                return getInfoString (OdbcDef.SQL_SPECIAL_CHARACTERS);
        }

        //--------------------------------------------------------------------
        // supportsAlterTableWithAddColumn
        // Do we support "ALTER TABLE" with add column?
        //--------------------------------------------------------------------

        public boolean supportsAlterTableWithAddColumn ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsAlterTableWithAddColumn");
                }
                int value = getInfo (OdbcDef.SQL_ALTER_TABLE);

                return ((value & OdbcDef.SQL_AT_ADD_COLUMN) > 0);
        }

        //--------------------------------------------------------------------
        // supportsAlterTableWithDropColumn
        // Do we support "ALTER TABLE" with drop column?
        //--------------------------------------------------------------------

        public boolean supportsAlterTableWithDropColumn ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsAlterTableWithDropColumn");
                }
                int value = getInfo (OdbcDef.SQL_ALTER_TABLE);

                return ((value & OdbcDef.SQL_AT_DROP_COLUMN) > 0);
        }

        //--------------------------------------------------------------------
        // supportsColumnAliasing
        // Do we support column aliasing?
        //--------------------------------------------------------------------

        public boolean supportsColumnAliasing ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsColumnAliasing");
                }
                return getInfoBooleanString (OdbcDef.SQL_COLUMN_ALIAS);
        }

        //--------------------------------------------------------------------
        // nullPlusNonNullIsNull
        // Are concatenations between NUL and non-NUL values NULL?
        //--------------------------------------------------------------------

        public boolean nullPlusNonNullIsNull ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.nullPlusNullIsNull");
                }
                int value = getInfoShort (OdbcDef.SQL_CONCAT_NULL_BEHAVIOR);

                return (value == OdbcDef.SQL_CB_NULL);
        }

        //--------------------------------------------------------------------
        // supportsConvert
        // Do we support the CONVERT function between SQL types?
        //--------------------------------------------------------------------

        public boolean supportsConvert () 
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsConvert");
                }
                int value = getInfo (OdbcDef.SQL_CONVERT_FUNCTIONS);

                return (value == OdbcDef.SQL_FN_CVT_CONVERT);
        }

        //--------------------------------------------------------------------
        // Do we support CONVERT betyween the given SQL types from
        // java.sql.Types
        //--------------------------------------------------------------------

        public boolean supportsConvert (
                int fromType,
                int toType)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsConvert (" +
                                fromType + "," + toType + ")");
                }

                short cvt = 0;
                boolean rc = false;
                int mask = 0;

                // Determine the convert from type

                switch (fromType) {
                case Types.BIT:
                        cvt = OdbcDef.SQL_CONVERT_BIT;
                        break;
                case Types.TINYINT:
                        cvt = OdbcDef.SQL_CONVERT_TINYINT;
                        break;
                case Types.SMALLINT:
                        cvt = OdbcDef.SQL_CONVERT_SMALLINT;
                        break;
                case Types.INTEGER:
                        cvt = OdbcDef.SQL_CONVERT_INTEGER;
                        break;
                case Types.BIGINT:
                        cvt = OdbcDef.SQL_CONVERT_BIGINT;
                        break;
                case Types.FLOAT:
                        cvt = OdbcDef.SQL_CONVERT_FLOAT;
                        break;
                case Types.REAL:
                        cvt = OdbcDef.SQL_CONVERT_REAL;
                        break;
                case Types.DOUBLE:
                        cvt = OdbcDef.SQL_CONVERT_DOUBLE;
                        break;
                case Types.NUMERIC:
                        cvt = OdbcDef.SQL_CONVERT_NUMERIC;
                        break;
                case Types.DECIMAL:
                        cvt = OdbcDef.SQL_CONVERT_DECIMAL;
                        break;
                case Types.CHAR:
                        cvt = OdbcDef.SQL_CONVERT_CHAR;
                        break;
                case Types.VARCHAR:
                        cvt = OdbcDef.SQL_CONVERT_VARCHAR;
                        break;
                case Types.LONGVARCHAR:
                        cvt = OdbcDef.SQL_CONVERT_LONGVARCHAR;
                        break;
                case Types.DATE:
                        cvt = OdbcDef.SQL_CONVERT_DATE;
                        break;
                case Types.TIME:
                        cvt = OdbcDef.SQL_CONVERT_TIME;
                        break;
                case Types.TIMESTAMP:
                        cvt = OdbcDef.SQL_CONVERT_TIMESTAMP;
                        break;
                case Types.BINARY:
                        cvt = OdbcDef.SQL_CONVERT_BINARY;
                        break;
                case Types.VARBINARY:
                        cvt = OdbcDef.SQL_CONVERT_VARBINARY;
                        break;
                case Types.LONGVARBINARY:
                        cvt = OdbcDef.SQL_CONVERT_LONGVARBINARY;
                        break;
                }

                // Get the bitmask for the conversions supported for this
                // type

                int value = getInfo (cvt);

                switch (toType) {
                case Types.BIT:
                        mask = OdbcDef.SQL_CVT_BIT;
                        break;
                case Types.TINYINT:
                        mask = OdbcDef.SQL_CVT_TINYINT;
                        break;
                case Types.SMALLINT:
                        mask = OdbcDef.SQL_CVT_SMALLINT;
                        break;
                case Types.INTEGER:
                        mask = OdbcDef.SQL_CVT_INTEGER;
                        break;
                case Types.BIGINT:
                        mask = OdbcDef.SQL_CVT_BIGINT;
                        break;
                case Types.FLOAT:
                        mask = OdbcDef.SQL_CVT_FLOAT;
                        break;
                case Types.REAL:
                        mask = OdbcDef.SQL_CVT_REAL;
                        break;
                case Types.DOUBLE:
                        mask = OdbcDef.SQL_CVT_DOUBLE;
                        break;
                case Types.NUMERIC:
                        mask = OdbcDef.SQL_CVT_NUMERIC;
                        break;
                case Types.DECIMAL:
                        mask = OdbcDef.SQL_CVT_DECIMAL;
                        break;
                case Types.CHAR:
                        mask = OdbcDef.SQL_CVT_CHAR;
                        break;
                case Types.VARCHAR:
                        mask = OdbcDef.SQL_CVT_VARCHAR;
                        break;
                case Types.LONGVARCHAR:
                        mask = OdbcDef.SQL_CVT_LONGVARCHAR;
                        break;
                case Types.DATE:
                        mask = OdbcDef.SQL_CVT_DATE;
                        break;
                case Types.TIME:
                        mask = OdbcDef.SQL_CVT_TIME;
                        break;
                case Types.TIMESTAMP:
                        mask = OdbcDef.SQL_CVT_TIMESTAMP;
                        break;
                case Types.BINARY:
                        mask = OdbcDef.SQL_CVT_BINARY;
                        break;
                case Types.VARBINARY:
                        mask = OdbcDef.SQL_CVT_VARBINARY;
                        break;
                case Types.LONGVARBINARY:
                        mask = OdbcDef.SQL_CVT_LONGVARBINARY;
                        break;
                }

                return ((value & mask) > 0);
        }

        //--------------------------------------------------------------------
        // supportsTableCorrelationNames
        // Do we support table correlation names?
        //--------------------------------------------------------------------

        public boolean supportsTableCorrelationNames ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsTableCorrelationNames");
                }
                int value = getInfoShort (OdbcDef.SQL_CORRELATION_NAME);

                return ((value == OdbcDef.SQL_CN_DIFFERENT) ||
                    (value == OdbcDef.SQL_CN_ANY));
        }

        //--------------------------------------------------------------------
        // supports DifferentTableCorrelationNames
        // If we do support table correlation names, are they restricted to be
        // different from the names of the tables?
        //--------------------------------------------------------------------

        public boolean supportsDifferentTableCorrelationNames ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsDifferentTableCorrelationNames");
                }
                int value = getInfoShort (OdbcDef.SQL_CORRELATION_NAME);

                return (value == OdbcDef.SQL_CN_DIFFERENT);
        }

        //--------------------------------------------------------------------
        // supportsExpressionsInOrderBy
        // Do we support expressions in "ORDER BY" lists?
        //--------------------------------------------------------------------

        public boolean supportsExpressionsInOrderBy ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsExpressionsInOrderBy");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_EXPRESSIONS_IN_ORDERBY);
        }

        //--------------------------------------------------------------------
        // supportsOrderByUnrelated
        // Can we use columns in "ORDER BY" not in the SELECT?
        //--------------------------------------------------------------------

        public boolean supportsOrderByUnrelated ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOrderByUnrelated");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_ORDER_BY_COLUMNS_IN_SELECT);
        }

        //--------------------------------------------------------------------
        // supportsGroupBy
        // Do we support some form of "GROUP BY" clause?
        //--------------------------------------------------------------------

        public boolean supportsGroupBy ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsGroupBy");
                }
                int value = getInfoShort (OdbcDef.SQL_GROUP_BY);

                return (value != OdbcDef.SQL_GB_NOT_SUPPORTED);
        }

        //--------------------------------------------------------------------
        // supportsGroupByUnrelated
        // Can a "GROUP BY" clause use columns not in the SELECT?
        //--------------------------------------------------------------------

        public boolean supportsGroupByUnrelated ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsGroupByUnrelated");
                }
                int value = getInfoShort (OdbcDef.SQL_GROUP_BY);

                return (value == OdbcDef.SQL_GB_NO_RELATION);
        }

        //--------------------------------------------------------------------
        // supportsGroupByBeyondSelect
        // Can a "GROUP BY" clauses add columns not in the SELECT?
        //--------------------------------------------------------------------

        public boolean supportsGroupByBeyondSelect ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsGroupByBeyondSelect");
                }
                int value = getInfoShort (OdbcDef.SQL_GROUP_BY);

                return (value == OdbcDef.SQL_GB_GROUP_BY_CONTAINS_SELECT);
        }

        //--------------------------------------------------------------------
        // supportsLikeEscapeClause
        // True if you can use escape character sin "LIKE" clauses.
        //--------------------------------------------------------------------

        public boolean supportsLikeEscapeClause ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsLikeEscapeClause");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_LIKE_CLAUSE_ESCAPE);
        }

        //--------------------------------------------------------------------
        // supportsMultipleResultSets
        // Do we support multiple ResultSets from a single execute?
        //--------------------------------------------------------------------

        public boolean supportsMultipleResultSets ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsMultipleResultSets");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_MULT_RESULT_SETS);
        }

        //--------------------------------------------------------------------
        // supportsMultipleTransactions
        // Can we have multiple transcations open at once (on different
        // connections)?
        //--------------------------------------------------------------------

        public boolean supportsMultipleTransactions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsMultipleTransactions");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_MULTIPLE_ACTIVE_TXN);
        }

        //--------------------------------------------------------------------
        // supportsNonNullableColumns
        // Are there some columns that don't take NULL values?
        //--------------------------------------------------------------------

        public boolean supportsNonNullableColumns ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsNonNullableColumns");
                }
                int value = getInfoShort (OdbcDef.SQL_NON_NULLABLE_COLUMNS);

                return (value == OdbcDef.SQL_NNC_NON_NULL);
        }


        //--------------------------------------------------------------------
        // supportsMinimumSQLGrammar
        //--------------------------------------------------------------------

        public boolean supportsMinimumSQLGrammar ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsMinimumSQLGrammar");
                }
                int value = getInfoShort (OdbcDef.SQL_ODBC_SQL_CONFORMANCE);

                // Minimum grammar is supported for minimum, core, and
                // extended

                return ((value == OdbcDef.SQL_OSC_MINIMUM) ||
                        (value == OdbcDef.SQL_OSC_CORE) ||
                        (value == OdbcDef.SQL_OSC_EXTENDED));
        }

        //--------------------------------------------------------------------
        // supportsCoreSQLGrammar
        //--------------------------------------------------------------------

        public boolean supportsCoreSQLGrammar ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCoreSQLGrammar");
                }
                int value = getInfoShort (OdbcDef.SQL_ODBC_SQL_CONFORMANCE);

                // Core grammar is supported for core and extended

                return ((value == OdbcDef.SQL_OSC_CORE) ||
                        (value == OdbcDef.SQL_OSC_EXTENDED));
        }

        //--------------------------------------------------------------------
        // supportsANSI92EntryLevelSQL
        // Is the ANSI92 entry level SQL grammar supported?
        //--------------------------------------------------------------------

        public boolean supportsANSI92EntryLevelSQL ()
                throws SQLException
        {
                return true;
        }

        //--------------------------------------------------------------------
        // supportsANSI92IntermediateLevelSQL
        // Is the ANSI92 intermediate SQL grammar supported?
        //--------------------------------------------------------------------

        public boolean supportsANSI92IntermediateSQL () 
                throws SQLException
        {
                return false;
        }

        //--------------------------------------------------------------------
        // supportsANSI92FullLevelSQL
        // Is the ANSI92 full SQL grammar supported?
        //--------------------------------------------------------------------

        public boolean supportsANSI92FullSQL ()
                throws SQLException
        {
                return false;
        }

        //--------------------------------------------------------------------
        // supportsExtendedSQLGrammar
        //--------------------------------------------------------------------

        public boolean supportsExtendedSQLGrammar ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsExtendedSQLGrammar");
                }
                int value = getInfoShort (OdbcDef.SQL_ODBC_SQL_CONFORMANCE);

                return (value == OdbcDef.SQL_OSC_EXTENDED);
        }

        //--------------------------------------------------------------------
        // supportsIntegrityEnhancementFacility
        // Extra SQL support for structual integrity?
        //--------------------------------------------------------------------

        public boolean supportsIntegrityEnhancementFacility ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsIntegrityEnhancementFacility");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_ODBC_SQL_OPT_IEF);
        }

        //--------------------------------------------------------------------
        // supportsOuterJoins
        // Supports some form of outer join
        //--------------------------------------------------------------------

        public boolean supportsOuterJoins ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOuterJoins");
                }
                String  value;

                value = getInfoString (OdbcDef.SQL_OUTER_JOINS);

                return (!value.equalsIgnoreCase ("N"));
        }

        //--------------------------------------------------------------------
        // supportsFullOuterJoins
        // Full nested outer joins
        //--------------------------------------------------------------------

        public boolean supportsFullOuterJoins ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsFullOuterJoins");
                }
                String  value;

                value = getInfoString (OdbcDef.SQL_OUTER_JOINS);

                return value.equalsIgnoreCase ("F");
        }

        //--------------------------------------------------------------------
        // supportsLimitedOuterJoins
        // Outer joins with limits
        //--------------------------------------------------------------------

        public boolean supportsLimitedOuterJoins ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsLimitedOuterJoins");
                }
                String  value;

                value = getInfoString (OdbcDef.SQL_OUTER_JOINS);

                return value.equalsIgnoreCase ("P");
        }

        //--------------------------------------------------------------------
        // getSchemaTerm
        // The database vendor's preferred term for "schema"
        //--------------------------------------------------------------------

        public String getSchemaTerm ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getSchemaTerm");
                }
                return getInfoString (OdbcDef.SQL_OWNER_TERM);
        }

        //--------------------------------------------------------------------
        // getProcedureTerm
        // The database vendor's preferred term for "procedure" 
        //--------------------------------------------------------------------

        public String getProcedureTerm ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getProcedureTerm");
                }
                return getInfoString (OdbcDef.SQL_PROCEDURE_TERM);
        }

        //--------------------------------------------------------------------
        // getCatalogTerm
        // The database vendor's preferred term for a "catalog"
        //--------------------------------------------------------------------

        public String getCatalogTerm ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getCatalogTerm");
                }
                return getInfoString (OdbcDef.SQL_QUALIFIER_TERM);
        }

        //--------------------------------------------------------------------
        // isCatalogAtStart
        // Does a catalog appear at the start of a qualified table name?
        // (Otherwise it appears at the end)
        //--------------------------------------------------------------------

        public boolean isCatalogAtStart ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.isCatalogAtStart");
                }
                int value = getInfoShort (OdbcDef.SQL_QUALIFIER_LOCATION);

                return (value == OdbcDef.SQL_QL_START);
        }

        //--------------------------------------------------------------------
        // getCatalogSeparator
        // Separator between catalog and table name.
        //--------------------------------------------------------------------

        public String getCatalogSeparator ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getCatalogSeparator");
                }
                return getInfoString (OdbcDef.SQL_QUALIFIER_NAME_SEPARATOR);
        }

        //--------------------------------------------------------------------
        // supportsSchemasInDataManipulation
        //--------------------------------------------------------------------

        public boolean supportsSchemasInDataManipulation ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSchemasInDataManipulation");
                }
                int value = getInfo (OdbcDef.SQL_OWNER_USAGE);

                return ((value & OdbcDef.SQL_OU_DML_STATEMENTS) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSchemasInProcedureCalls
        //--------------------------------------------------------------------

        public boolean supportsSchemasInProcedureCalls ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSchemasInProcedureCalls");
                }
                int value = getInfo (OdbcDef.SQL_OWNER_USAGE);

                return ((value & OdbcDef.SQL_OU_PROCEDURE_INVOCATION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSchemasInTableDefinitions
        //--------------------------------------------------------------------

        public boolean supportsSchemasInTableDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSchemasInTableDefinitions");
                }
                int value = getInfo (OdbcDef.SQL_OWNER_USAGE);

                return ((value & OdbcDef.SQL_OU_TABLE_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSchemasInIndexDefinitions
        //--------------------------------------------------------------------

        public boolean supportsSchemasInIndexDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSchemasInIndexDefintions");
                }
                int value = getInfo (OdbcDef.SQL_OWNER_USAGE);

                return ((value & OdbcDef.SQL_OU_INDEX_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSchemasInPrivilegeDefinitions
        //--------------------------------------------------------------------

        public boolean supportsSchemasInPrivilegeDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSchemasInPrivilegeDefintions");
                }
                int value = getInfo (OdbcDef.SQL_OWNER_USAGE);

                return ((value & OdbcDef.SQL_OU_PRIVILEGE_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCatalogsInDataManipulation
        //--------------------------------------------------------------------

        public boolean supportsCatalogsInDataManipulation ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCatalogsInDataManipulation");
                }
                int value = getInfo (OdbcDef.SQL_QUALIFIER_USAGE);

                return ((value & OdbcDef.SQL_QU_DML_STATEMENTS) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCatalogsInProcedureCalls
        //--------------------------------------------------------------------

        public boolean supportsCatalogsInProcedureCalls ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCatalogsInProcedureCalls");
                }
                int value = getInfo (OdbcDef.SQL_QUALIFIER_USAGE);

                return ((value & OdbcDef.SQL_QU_PROCEDURE_INVOCATION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCatalogsInTableDefinitions
        //--------------------------------------------------------------------

        public boolean supportsCatalogsInTableDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCatalogsInTableDefinitions");
                }
                int value = getInfo (OdbcDef.SQL_QUALIFIER_USAGE);

                return ((value & OdbcDef.SQL_QU_TABLE_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCatalogsInIndexDefinitions
        //--------------------------------------------------------------------

        public boolean supportsCatalogsInIndexDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCatalogsInIndexDefinitions");
                }
                int value = getInfo (OdbcDef.SQL_QUALIFIER_USAGE);

                return ((value & OdbcDef.SQL_QU_INDEX_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCatalogsInPrivilegeDefinitions
        //--------------------------------------------------------------------

        public boolean supportsCatalogsInPrivilegeDefinitions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCatalogsInPrivilegeDefintions");
                }
                int value = getInfo (OdbcDef.SQL_QUALIFIER_USAGE);

                return ((value & OdbcDef.SQL_QU_PRIVILEGE_DEFINITION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsPositionedDelete
        // Do we support positioned DELETE?
        //--------------------------------------------------------------------

        public boolean supportsPositionedDelete ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsPositionedDelete");
                }
                int value = getInfo (OdbcDef.SQL_POSITIONED_STATEMENTS);

                return ((value & OdbcDef.SQL_PS_POSITIONED_DELETE) > 0);
        }

        //--------------------------------------------------------------------
        // supportsPositionedUpdate
        // Do we support positioned UPDATE?
        //--------------------------------------------------------------------

        public boolean supportsPositionedUpdate ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsPositionedUpdate");
                }
                int value = getInfo (OdbcDef.SQL_POSITIONED_STATEMENTS);

                return ((value & OdbcDef.SQL_PS_POSITIONED_UPDATE) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSelectForUpdate
        // Do we support SELECT for UPDATE?
        //--------------------------------------------------------------------

        public boolean supportsSelectForUpdate ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSelectForUpdate");
                }
                int value = getInfo (OdbcDef.SQL_POSITIONED_STATEMENTS);

                return ((value & OdbcDef.SQL_PS_SELECT_FOR_UPDATE) > 0);
        }

        //--------------------------------------------------------------------
        // supportsStoredProcedures
        // Do we support stored procedures with a standard call syntax
        //--------------------------------------------------------------------

        public boolean supportsStoredProcedures ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsStoredProcedures");
                }
                return getInfoBooleanString (OdbcDef.SQL_PROCEDURES);
        }

        //--------------------------------------------------------------------
        // supportsSubqueriesInComparisions
        //--------------------------------------------------------------------

        public boolean supportsSubqueriesInComparisons ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSubqueriesInComparisions");
                }
                int value = getInfo (OdbcDef.SQL_SUBQUERIES);

                return ((value & OdbcDef.SQL_SQ_COMPARISON) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSubqueriesInExists
        //--------------------------------------------------------------------

        public boolean supportsSubqueriesInExists ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSubqueriesInExists");
                }
                int value = getInfo (OdbcDef.SQL_SUBQUERIES);

                return ((value & OdbcDef.SQL_SQ_EXISTS) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSubqueriesInIns
        //--------------------------------------------------------------------

        public boolean supportsSubqueriesInIns ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSubqueriesInIns");
                }
                int value = getInfo (OdbcDef.SQL_SUBQUERIES);

                return ((value & OdbcDef.SQL_SQ_IN) > 0);
        }

        //--------------------------------------------------------------------
        // supportsSubqueriesInQuantifieds
        //--------------------------------------------------------------------

        public boolean supportsSubqueriesInQuantifieds ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsSubqueriesInQuantifieds");
                }
                int value = getInfo (OdbcDef.SQL_SUBQUERIES);

                return ((value & OdbcDef.SQL_SQ_QUANTIFIED) > 0);
        }

        //--------------------------------------------------------------------
        // supportsCorrelatedSubqueries
        //--------------------------------------------------------------------

        public boolean supportsCorrelatedSubqueries ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsCorrelatedSubqueries");
                }
                int value = getInfo (OdbcDef.SQL_SUBQUERIES);

                return ((value & OdbcDef.SQL_SQ_CORRELATED_SUBQUERIES) > 0);
        }

        //--------------------------------------------------------------------
        // supportsUnion
        // Return true if the database supports the UNION clase
        //--------------------------------------------------------------------

        public boolean supportsUnion ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsUnion");
                }
                int value = getInfo (OdbcDef.SQL_UNION);

                return ((value & OdbcDef.SQL_U_UNION) > 0);
        }

        //--------------------------------------------------------------------
        // supportsUnionAll
        // Return true if the database supports the UNION clase with the
        // ALL keyword
        //--------------------------------------------------------------------

        public boolean supportsUnionAll ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsUnionAll");
                }
                int value = getInfo (OdbcDef.SQL_UNION);

                return ((value & OdbcDef.SQL_U_UNION_ALL) > 0);
        }

        //--------------------------------------------------------------------
        // Normally all open statememts, resultsets, etc, are closed when
        // either Statement.commit or Statement.rollback is called.
        // However some databases provide mechanisms for preserving state
        // across these methods.  The Connection.disableAutoCommit method
        // can be used to try to keep statements open, and the following
        // methods can be used to find what is supported:
        //--------------------------------------------------------------------

        public boolean supportsOpenCursorsAcrossCommit ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOpenCursorsAcrossCommit");
                }
                int value = getInfoShort(OdbcDef.SQL_CURSOR_COMMIT_BEHAVIOR);

                return (value == OdbcDef.SQL_CB_PRESERVE);
        }

        public boolean supportsOpenCursorsAcrossRollback ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOpenCursorsAcrossRollback");
                }
                int value = getInfoShort (
                                OdbcDef.SQL_CURSOR_ROLLBACK_BEHAVIOR);

                return (value == OdbcDef.SQL_CB_PRESERVE);
        }

        public boolean supportsOpenStatementsAcrossCommit ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOpenStatementsAcrossCommit");
                }
                int value = getInfoShort(OdbcDef.SQL_CURSOR_COMMIT_BEHAVIOR);

                return ((value == OdbcDef.SQL_CB_PRESERVE) ||
                        (value == OdbcDef.SQL_CB_CLOSE));
        }

        public boolean supportsOpenStatementsAcrossRollback ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsOpenStatementsAcrossRollback");
                }
                int value = getInfoShort (
                                OdbcDef.SQL_CURSOR_ROLLBACK_BEHAVIOR);

                return ((value == OdbcDef.SQL_CB_PRESERVE) ||
                        (value == OdbcDef.SQL_CB_CLOSE));
        }

        
        //--------------------------------------------------------------------
        // getMaxBinaryLiteralLength
        // How many hex characters can you have in an inline binary literal?
        //--------------------------------------------------------------------

        public int getMaxBinaryLiteralLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxBinaryLiteralLength");
                }
                return getInfo (OdbcDef.SQL_MAX_BINARY_LITERAL_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxCharLiteralLength
        // What's the max length for a character literal?
        //--------------------------------------------------------------------

        public int getMaxCharLiteralLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxCharLiteralLength");
                }
                return getInfo (OdbcDef.SQL_MAX_CHAR_LITERAL_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxColumnNameLength
        // What's the limit on column name length?
        //--------------------------------------------------------------------

        public int getMaxColumnNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_COLUMN_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxColumnsInGroupBy
        // maximum numer of columns in a "GROUP BY" clause.
        //--------------------------------------------------------------------

        public int getMaxColumnsInGroupBy ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnsInGroupBy");
                }
                return getInfoShort(OdbcDef.SQL_MAX_COLUMNS_IN_GROUP_BY);
        }

        //--------------------------------------------------------------------
        // getMaxColumnsInIndex
        // maximum number of columns allowed in an index
        //--------------------------------------------------------------------

        public int getMaxColumnsInIndex ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnsInIndex");
                }
                return getInfoShort (OdbcDef.SQL_MAX_COLUMNS_IN_INDEX);
        }

        //--------------------------------------------------------------------
        // getMaxColumnsInOrderBy
        // maximum number of columns in an "ORDER BY" clause
        //--------------------------------------------------------------------

        public int getMaxColumnsInOrderBy ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnsInOrderBy");
                }
                return getInfoShort(OdbcDef.SQL_MAX_COLUMNS_IN_ORDER_BY);
        }

        //--------------------------------------------------------------------
        // getMaxColumnsInSelect
        // maximum number of columns in a "SELECT" list
        //--------------------------------------------------------------------

        public int getMaxColumnsInSelect ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnsInSeleted");
                }
                return getInfoShort (OdbcDef.SQL_MAX_COLUMNS_IN_SELECT);
        }

        //--------------------------------------------------------------------
        // getMaxColumnsInTable
        // maximum number of columns in a table
        //--------------------------------------------------------------------

        public int getMaxColumnsInTable ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxColumnsInTable");
                }
                return getInfoShort (OdbcDef.SQL_MAX_COLUMNS_IN_TABLE);
        }

        //--------------------------------------------------------------------
        // getMaxConnections
        // How many active connections can we have at a time to this database?
        //--------------------------------------------------------------------

        public int getMaxConnections ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxConnections");
                }
                return getInfoShort (OdbcDef.SQL_ACTIVE_CONNECTIONS);
        }

        //--------------------------------------------------------------------
        // getMaxCursorNameLength
        // maximum cursor name length
        //--------------------------------------------------------------------

        public int getMaxCursorNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxCursorNameLength");
                }
                return (int) getInfo (OdbcDef.SQL_MAX_CURSOR_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxIndexLength
        // maximum size of an index (in bytes)  
        //--------------------------------------------------------------------

        public int getMaxIndexLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxIndexLength");
                }
                return getInfo (OdbcDef.SQL_MAX_INDEX_SIZE);
        }

        //--------------------------------------------------------------------
        // getMaxSchemaNameLength
        // The maximum size allowed for a schema name.
        //--------------------------------------------------------------------

        public int getMaxSchemaNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxSchemaNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_OWNER_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxProcedureNameLength
        // The maximum length of a procedure name
        //--------------------------------------------------------------------

        public int getMaxProcedureNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxProcedureNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_PROCEDURE_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxCatalogNameLength
        // The maximum length of a catalog name.
        //--------------------------------------------------------------------

        public int getMaxCatalogNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxCatalogNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_QUALIFIER_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxRowSize
        // The maximum length of a single row
        //--------------------------------------------------------------------

        public int getMaxRowSize ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxRowSize");
                }
                return getInfo (OdbcDef.SQL_MAX_ROW_SIZE);
        }

        //--------------------------------------------------------------------
        // doesMaxRowSizeIncludeBlobs
        // And for the pernickity did getmaxRowSize() include LONGVARCHAR and
        // LONGVARBINARY blobs?
        //--------------------------------------------------------------------

        public boolean doesMaxRowSizeIncludeBlobs ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.doesMaxRowSizeIncludeBlobs");
                }
                return getInfoBooleanString (
                        OdbcDef.SQL_MAX_ROW_SIZE_INCLUDES_LONG);
        }

        //--------------------------------------------------------------------
        // getMaxStatementLength
        // maximum length of a SQL statement
        //--------------------------------------------------------------------

        public int getMaxStatementLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxStatementLength");
                }
                return getInfo (OdbcDef.SQL_MAX_STATEMENT_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxStatements
        // How many active statements can we have at a time to this database?
        //--------------------------------------------------------------------

        public int getMaxStatements ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxStatements");
                }
                return getInfoShort (OdbcDef.SQL_ACTIVE_STATEMENTS);
        }

        //--------------------------------------------------------------------
        // getMaxTableNameLength
        // maximum length of a table name.
        //--------------------------------------------------------------------

        public int getMaxTableNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxTableNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_TABLE_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getMaxTablesInSelect
        // maximum number of tables in a SELECT.
        //--------------------------------------------------------------------

        public int getMaxTablesInSelect ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxTablesInSelect");
                }
                return getInfoShort (OdbcDef.SQL_MAX_TABLES_IN_SELECT);
        }

        //--------------------------------------------------------------------
        // getMaxUserNameLength
        // maximum length of a user name
        //--------------------------------------------------------------------

        public int getMaxUserNameLength ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getMaxUserNameLength");
                }
                return getInfoShort (OdbcDef.SQL_MAX_USER_NAME_LEN);
        }

        //--------------------------------------------------------------------
        // getDefaultTransactionIsolation
        // Get the databases's default transaction isolation level.
        // The value are defined in java.sql.Connection.
        // These values happen to be the same as defined for ODBC, but
        // we will still map them
        //--------------------------------------------------------------------

        public int getDefaultTransactionIsolation ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getDefaultTransactionIsolation");
                }
                int value = getInfo (OdbcDef.SQL_DEFAULT_TXN_ISOLATION);
                int jdbcValue = Connection.TRANSACTION_NONE;

                // Map ODBC value to JDBC value
                
                switch (value) {
                case OdbcDef.SQL_TXN_READ_UNCOMMITTED:
                        jdbcValue = Connection.TRANSACTION_READ_UNCOMMITTED;
                        break;
                case OdbcDef.SQL_TXN_READ_COMMITTED:
                        jdbcValue = Connection.TRANSACTION_READ_COMMITTED;
                        break;
                case OdbcDef.SQL_TXN_REPEATABLE_READ:
                        jdbcValue = Connection.TRANSACTION_REPEATABLE_READ;
                        break;
                case OdbcDef.SQL_TXN_SERIALIZABLE:
                        jdbcValue = Connection.TRANSACTION_SERIALIZABLE;
                        break;
                }
                return jdbcValue;
        }

        //--------------------------------------------------------------------
        // supportsTransactions
        // if supportsTranscations returns false, then the databade doesn't 
        // support transactions.  This means that beginTransaction and commit
        // are noops; and the isolation level is TRNSACTION_NONE.
        //--------------------------------------------------------------------

        public boolean supportsTransactions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsTransactions");
                }
                int value = getInfoShort (OdbcDef.SQL_TXN_CAPABLE);

                return (value != OdbcDef.SQL_TC_NONE);
        }

        //--------------------------------------------------------------------
        // supportsTransactionIsolationLevel
        // Check if the database supports a given transaction isolation level.
        // The values are defined in java.sql.Connection.
        //--------------------------------------------------------------------

        public boolean supportsTransactionIsolationLevel (
                int level)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsTransactionIsolationLevel ("
                                + level + ")");
                }
                // Special case of TRANSACTION_NONE, check if transactions
                // are supported.

                if (level == Connection.TRANSACTION_NONE) {
                        return (supportsTransactions () == false);
                }

                int value = getInfo (OdbcDef.SQL_TXN_ISOLATION_OPTION);
                boolean supported = false;

                switch (level) {
                case Connection.TRANSACTION_READ_UNCOMMITTED:
                        supported = (value &
                                OdbcDef.SQL_TXN_READ_UNCOMMITTED) > 0;
                        break;
                case Connection.TRANSACTION_READ_COMMITTED:
                        supported = (value &
                                OdbcDef.SQL_TXN_READ_COMMITTED) > 0;
                        break;
                case Connection.TRANSACTION_REPEATABLE_READ:
                        supported = (value &
                                OdbcDef.SQL_TXN_REPEATABLE_READ) > 0;
                        break;
                case Connection.TRANSACTION_SERIALIZABLE:
                        supported = (value &
                                OdbcDef.SQL_TXN_SERIALIZABLE) > 0;
                        break;
                }
                return supported;
        }

        //--------------------------------------------------------------------
        // supportsDataDefinitionAndDataManipulationTransactions
        // Methdos specifying whether you can do data definition stamements as
        // parts of transctions and what happens if you do.
        //--------------------------------------------------------------------

        public boolean supportsDataDefinitionAndDataManipulationTransactions ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsDataDefinitionAndDataManipulationTransactions");
                }
                int value = getInfoShort (OdbcDef.SQL_TXN_CAPABLE);

                return ((value & OdbcDef.SQL_TC_ALL) > 0);
        }

        public boolean supportsDataManipulationTransactionsOnly ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsDataManipulationTransactionsOnly");
                }
                int value = getInfoShort (OdbcDef.SQL_TXN_CAPABLE);

                return ((value & OdbcDef.SQL_TC_DML) > 0);
        }

        public boolean dataDefinitionCausesTransactionCommit()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.dataDefintionCausesTransactionCommit");
                }
                int value = getInfoShort (OdbcDef.SQL_TXN_CAPABLE);

                return ((value & OdbcDef.SQL_TC_DDL_COMMIT) > 0);
        }

        public boolean dataDefinitionIgnoredInTransactions()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.dataDefintionIgnoredInTransactions");
                }
                int value = getInfoShort (OdbcDef.SQL_TXN_CAPABLE);

                return ((value & OdbcDef.SQL_TC_DDL_IGNORE) > 0);
        }


        //--------------------------------------------------------------------
        // getProcedures
        // Returns a ResultSet that contains a row for each
        // matching stored procedure in the database.
        // Each row in the ResultSet contains the following fields:
        //      Column(1) String => procedure catalog (may be NULL)
        //      Column(2) String => procedure schema (may be NULL)
        //      Column(3) String => procedure name
        //      Column(7) String => explanatory comment on the procedure
        //      Column(8) short  => kind of procedure:
//      int procedureResultUnknown      = 0;    // May returns a result.
//      int procedureNoResult           = 1;    // Does not return a result
//      int procedureReturnsResult      = 2;    // Returns a result.
        // The output can be limited by specifying catalog, schema, and
        // procedure name patterns that must be matched.
        // The results are sorted by columns 1, 2, and 3.
        //--------------------------------------------------------------------

        public ResultSet getProcedures(
                String catalog,
                String schemaPattern,
                String procedureNamePattern)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getProcedures (" +
                                catalog + "," + schemaPattern + "," +
                                procedureNamePattern + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLProcedures
                try {
                        OdbcApi.SQLProcedures (hStmt, catalog, schemaPattern,
                                procedureNamePattern);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getProcedureColumns
        // Returns a ResultSet that contains rows
        // describing the parameters and result columns from matching stored
        // procedures in the database.
        // Each row in the ResultSet contains the following fields:
        //      Column(1)  String => procedure catalog (may be NULL)
        //      Column(2)  String => procedure schema (may be NULL)
        //      Column(3)  String => procedure name
        //      Column(4)  String => column/parameter name 
        //      Column(5)  Short  => kind of column/parameter:
//      int procedureColumnUnknown      = 0;    // nobody knows
//      int procedureColumnIn           = 1;    // INT parameter
//      int procedureColumnInOut        = 2;    // INOUT parameter
//      int procedureColumnResult       = 3;    // result column in ResultSet
//      int procedureColumnOut          = 4;    // OUT parameter
//      int procedureColumnReturn       = 5;    // return value of function
        //      Column(6)  short  => SQL type from java.sql.Types
        //      Column(7)  String => SQL type name
        //      Column(8)  int    => precision
        //      Column(10) short  => scale
        //      Column(11) short  => radix
        //      Column(12) boolean => can it contain null?
        //      Column(13) String => comment describing parameter/column
        // The results are sorted by columns 1, 2, 3, and 5.
        //--------------------------------------------------------------------
        
        public ResultSet getProcedureColumns(
                String catalog,
                String schemaPattern,
                String procedureNamePattern, 
                String columnNamePattern) 
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getProcedureColumns (" +
                                catalog + "," + schemaPattern + "," +
                                procedureNamePattern + "," +
                                columnNamePattern + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLProcedureColumns
                try {
                        OdbcApi.SQLProcedureColumns (hStmt, catalog,
                                schemaPattern, procedureNamePattern,
                                columnNamePattern);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

/*
                if (Con.getODBCVer () == 2) {
                        rs.setSQLTypeColumn (6);
                }
*/
		// sun's 4234318 fix.
		if (Con.getODBCVer () >= 2) {
			rs.setSQLTypeColumn (6);
 
			//rename some columns in order to be JDBC compilant.
			rs.setAliasColumnName("PRECISION", 8);
			rs.setAliasColumnName("LENGTH", 9);
			rs.setAliasColumnName("SCALE", 10);
			rs.setAliasColumnName("RADIX", 11);
		}

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getTables
        // Returns a ResultSet that contains a row for each matching
        // table in the database
        // Each row in the ResultSet contains the following fields:
        //      Column(1) String => table catalog (may be NULL)
        //      Column(2) String => table schema
        //      Column(3) String => table name
        //      Column(4) String => table type.  Typical types are "TABLE",
        //                      "VIEW", "SYSTEM TABLE", "GLOBAL TEMPORARY", 
        //                      "LOCAL TEMPORARY", "ALIAS", "SYNONYM".
        //      Column(5) String => explanatory comment on the table
        // The output can be limited by specifying catalog, schema, and
        // procedures name patterns that must be matched.  If the table types
        // array is non-null then only tables of the given types will be 
        // returned.  The results are sorted by columns 4, 1, 2, and 3.
        //--------------------------------------------------------------------
        
        public ResultSet getTables(
                String catalog,
                String schemaPattern,
                String tableNamePattern,
                String types[])
                throws SQLException
        {
                long hStmt;
                JdbcOdbcResultSet rs = null;
                String tableTypes = null;
                SQLWarning      warning = null;

                // Convert the types into a comma separated list
                if (types != null) {
                        tableTypes = "";
                        short   i = 0;;
                        for (i = 0; i < types.length; i++) {
                                String s = types[i];
                                if (i > 0) {
                                        // Add a comma to separate
                                        tableTypes += ",";
                                }
                                tableTypes += s;
                        }
                }

                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getTables (" +
                                catalog + "," + schemaPattern + "," +
                                tableNamePattern + "," + tableTypes + ")");
                }

                // Allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);


                // Call SQLTables
                try {
                        OdbcApi.SQLTables (hStmt, catalog, schemaPattern,
                                tableNamePattern, tableTypes);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getSchemas
        // Returns a ResultSet containing a row for each schema.
        // Each row in the ResultSet contains a single String field which is
        // the schema username.
        // The results are sorted by string value.
        //--------------------------------------------------------------------

        public ResultSet getSchemas ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getSchemas");
                }

                JdbcOdbcResultSet rs;

                rs = (JdbcOdbcResultSet) getTables ("", "%", "", null);

                // Now set the column mappings (see comments for 
                // setColumnMappings for more information)

                int map[] = new int[1];
                map[0] = 2;
                rs.setColumnMappings (map);
                return rs;
        }

        //--------------------------------------------------------------------
        // getCatalogs
        // Returns a ResultSet containing a row for each "catalog"
        // in the database. Each row in the ResultSet contains a single String
        // field which is the catalog name.
        // The results are sorted by string value.
        //--------------------------------------------------------------------

        public ResultSet getCatalogs ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getCatalogs");
                }
                JdbcOdbcResultSet rs;

                rs = (JdbcOdbcResultSet) getTables ("%", "", "", null);
                // Now set the column mappings (see comments for 
                // setColumnMappings for more information)

                int map[] = new int[1];
                map[0] = 1;
                rs.setColumnMappings (map);
                return rs;
        }

        //--------------------------------------------------------------------
        // getTableTypes
        // Returns a ResultSet containing a row for each table
        // type in the database. Each row in the ResultSet contains a single
        // String field which is the table type.
        // The results are sorted by string value.
        //--------------------------------------------------------------------

        public ResultSet getTableTypes ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getTableTypes");
                }
                String types[];
                JdbcOdbcResultSet rs;

                // Make a single entry String array with the only element
                // containing "%"

                types = new String[1];
                types[0] = "%";

                //4514392
		rs = (JdbcOdbcResultSet) getTables (null, null, "%", null);

                // Now set the column mappings (see comments for 
                // setColumnMappings for more information)

                int map[] = new int[1];
                map[0] = 4;
                rs.setColumnMappings (map);
                return rs;
        }

        //--------------------------------------------------------------------
        // getColumns
        // Returns a ResultSet containing a row for each column in
        // the matching table(s).
        // The ResultSet is order by catalog, schema, and table name.
        // Each row in the ResultSet contains:
        //      Column(1)  String => table catalog (may be NULL)
        //      Column(2)  String => table schema
        //      Column(3)  String => table name
        //      Column(4)  String => column name
        //      Column(5)  short   => SQL type from java.sql.Types
        //      Column(6)  String => SQL type name
        //      Column(7)  int    => column size.  For char or date types this
        //                           is the maximum number of characters, for
        //                           numeric or decimal types this is
        //                           precision.
        //      Column(8)  is not used.
        //      Column(9)  int    => scale
        //      Column(10) int    => Radix (typicaly either 10 or 2)
        //      Column(11) boolean=> can column contain NULL?
        //      Column(12) String => comment describing column (may be NULL)
        //      Column(13) String => default value (may be NULL)
        //  Column(14) SQL_DATA_TYPE int => unused
        //      Column(15) SQL_DATETIME_SUB int => unused
        //      Column(16) CHAR_OCTET_LENGTH int => for char types the 
        //                              maximum number of bytes in the column
        //      Column(17) ORDINAL_POSITION int => index of column in table 
        //                              (starting at 1)
        //      Column(18) IS_NULLABLE String => "NO" means column definitely 
        //                              does not allow NULL values; "YES" means the column might 
        //                              allow NULL values.  An empty string means nobody knows.
        //
        // The results are sorted by columns 1, 2, and 3.
        //--------------------------------------------------------------------

        public ResultSet getColumns(
                String catalog,
                String schemaPattern,
                String tableNamePattern,
                String columnNamePattern)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getColumns (" +
                                catalog + "," + schemaPattern + "," +
                                tableNamePattern + "," +
                                columnNamePattern + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLColumns
                try {
                        OdbcApi.SQLColumns (hStmt, catalog, schemaPattern,
                                tableNamePattern, columnNamePattern);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                        
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // If this is an ODBC 2.x version driver, we need to
                // manufacture some additional result set columns
                // (JdbcOdbcPseudoCol)

                if (Con.getODBCVer () == 2) {
                        JdbcOdbcPseudoCol pc[] = new JdbcOdbcPseudoCol[6];

                        pc[0] = new JdbcOdbcPseudoCol ("COLUMN_DEF",
                                        Types.VARCHAR, 254);
                        pc[1] = new JdbcOdbcPseudoCol ("SQL_DATA_TYPE",
                                        Types.SMALLINT, 0);
                        pc[2] = new JdbcOdbcPseudoCol ("SQL_DATETIME_SUB",
                                        Types.SMALLINT, 0);
                        pc[3] = new JdbcOdbcPseudoCol ("CHAR_OCTET_LENGTH",
                                        Types.INTEGER, 0);
                        pc[4] = new JdbcOdbcPseudoCol ("ORDINAL_POSITION",
                                        Types.INTEGER, 0);
                        pc[5] = new JdbcOdbcPseudoCol ("IS_NULLABLE",
                                        Types.VARCHAR, 254);

                        // Set the new pseudo columns in the result set,
                        // supplying the first and last pseudo column
                        // numbers

                        rs.setPseudoCols (13, 18, pc);
                        rs.setSQLTypeColumn (5);
                }
		else if (Con.getODBCVer () >= 3) {  // sun's 4234318 fix.
			rs.setSQLTypeColumn (5);
			rs.setAliasColumnName ("SQL_DATETIME_SUB", 15);
		}

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getColumnPrivileges
        // Returns a ResultSet containing a row for each
        // set of access rights which matches the given pattern for column
        // names. Each row in the ResultSet contains:
        //      Column(1) String => table catalog (may be NULL)
        //      Column(2) String => table schema
        //      Column(3) String => table name
        //      Column(4) String => column name
        //      Column(5) String => grantor of access
        //      Column(6) String => grantee of access
        //      Column(7) String => name of access (SELECT, INSERT, UPDATE,
        //                          REFRENCES, ...)
        //      Column(8) String => grantable ("YES" if it can be granted to
        //                          others)
        // The results are sorted by columns 1, 2, 3, 4, and 7.
        //--------------------------------------------------------------------

        public ResultSet getColumnPrivileges (
                String catalog,
                String schema,
                String table,
                String columnNamePattern)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getColumnPrivileges (" +
                                catalog + "," + schema + "," + table + "," +
                                columnNamePattern + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLColumnPrivileges
                try {
                        OdbcApi.SQLColumnPrivileges (hStmt, catalog,
                                schema, table, columnNamePattern);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getTablePrivileges
        // Returns a ResulSet containing a row for each set
        // of access rights associated with matching tables.
        // Each row in the ResultSet contains:
        //      Column(1) String => table catalog (may be NULL)
        //      Column(2) String => table schema
        //      Column(3) String => table name
        //      Column(4) String => grantor of access
        //      Column(5) String => grantee of access
        //      Column(6) String => name of access (SELECT, INSERT, UPDATE,
        //                          REFRENCES, ...)
        //      Column(7) String => grantable ("YES" if it can be granted to
        //                          others)
        // The results are sorted by columns 1, 2, 3, and 7.
        //--------------------------------------------------------------------

        public ResultSet getTablePrivileges (
                String catalog,
                String schemaPattern,
                String tableNamePattern)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getTablePrivileges (" +
                                catalog + "," + schemaPattern + "," +
                                tableNamePattern + ")");
                }

                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLTablePrivileges
                try {
                        OdbcApi.SQLTablePrivileges (hStmt, catalog,
                                schemaPattern, tableNamePattern);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getBestRowIdentifier
        // Returns the optimal set of columns that can be
        // used to select rows in the given table.
        // The input and output scope values are one of
//      int bestRowTemporary   = 0;     // => very temporary, while using row
//      int bestRowTransaction = 1;     // => valid for remainder of current
                                        //    transaction
//      int bestRowSession     = 2;     // => valid for remainder of current
                                        //    session
        // The "nullable" argument specifies whether to return columns that
        // can take null values.
        // Each row in the ResultSet contains:
        //      Column(1) short  => actual scope of result (see above)
        //      Column(2) String => column name
        //      Column(3) short  => SQL data type from java.sql.Types
        //      Column(4) String => SQL type name
        //      Column(5) int    => precision
        //      Column(7) short  => scale
        //      Column(8) short  => is this a psuedo column:
//      int bestRowUnknown      = 0;
//      int bestRowNotPseudo    = 1;
//      int bestRowPseudo       = 2;
        // The results are sorted by column 1.
        //--------------------------------------------------------------------

        public ResultSet getBestRowIdentifier (
                String catalog,
                String schema,
                String table,
                int scope,
                boolean nullable)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getBestRowIdentifier (" +
                                catalog + "," + schema + "," + table + "," +
                                scope + "," + nullable + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLSpecialColumns
                try {
                        OdbcApi.SQLSpecialColumns (hStmt,
                                OdbcDef.SQL_BEST_ROWID, catalog,
                                schema, table, scope, nullable);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);
		//if (Con.getODBCVer () == 2) {
                if (Con.getODBCVer () >= 2) { // sun's 4234318 fix.
                        rs.setSQLTypeColumn (3);
                }

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getVersionColumns
        // Returns the set of column names where the column
        // is automaticaly updated when ever any field in a row changes.
        // Each row in the ResultSet contains:
        //      Column(1) is not used
        //      Column(2) String => column name
        //      Column(3) short  => SQL data type from java.sql.Types
        //      Column(4) String => SQL type name
        //      Column(5) int    => precision
        //      Column(7) short  => scale
        //      Column(8) short  => is this a psuedo column?
//      int versionColumnUnknown        = 0;
//      int versionColumnNotPseudo      = 1;
//      int versionColumnPseudo         = 2;
        // The results are unsorted.
        //--------------------------------------------------------------------

        public ResultSet getVersionColumns (
                String catalog,
                String schema,
                String table)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getVersionColumns (" +
                                catalog + "," + schema + "," + table + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLSpecialColumns
                try {
                        OdbcApi.SQLSpecialColumns (hStmt,
                                OdbcDef.SQL_ROWVER, catalog,
                                schema, table, bestRowTemporary, false);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);
		//if (Con.getODBCVer () == 2) {
                if (Con.getODBCVer () >= 2) { // sun's 4234318 fix.
                        rs.setSQLTypeColumn (3);
                }

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getPrimaryKeys
        // Returns infomation about the primary keys for
        // the given table.
        // Each row in the ResultSet contains:
        //      Column(1) String => table catalog (may be NULL)
        //      Column(2) String => table schema
        //      Column(3) String => table name
        //      Column(4) String => column name 
        //      Column(5) short  => sequence number within primary key
        //      Column(6) String => primary key name
        // The results are sorted by columns 1, 2, 3, and 5.
        //--------------------------------------------------------------------

        public ResultSet getPrimaryKeys (
                String catalog,
                String schema,
                String table)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getPrimaryKeys (" +
                                catalog + "," + schema + "," + table + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLPrimaryKeys
                try {
                        OdbcApi.SQLPrimaryKeys (hStmt,
                                catalog, schema, table);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getImportedKeys
        // Returns information about the foreign keys in the
        // given table, and the primary keys to which they refer.  See also
        // getExportedKeys and getCrossReference.
        //
        // Each row in the ResultSet contains:
        //      Column(1)  String => primary key table catalog
        //      Column(2)  String => primary key table schema
        //      Column(3)  String => primary key table name
        //      Column(4)  String => primary key column name
        //      Column(5)  String => foreign key table catalog
        //      Column(6)  String => foreign key table schema
        //      Column(7)  String => foreign key table name
        //      Column(8)  String => foreign key table name
        //      Column(9)  String => column sequence number within primary key
        //      Column(10) short  => Update rule.  What happens to foreign key
        //                              when primary is updated (may be NULL):
//      int importedKeyCascade  = 0;
//      int importedKeyRestrict = 1;
//      int importedKeySetNull  = 2;
        //      Column(11) short  => Delete rule.  What happens to the foreign
        //                              key when primary is deleted.  Values
        //                              are the same as for the update rule.
        //      Column(12) String => foreign key name (may be NULL)
        //      Column(13) String => primary key name (may be NULL)
        //  Column(14) short  => DEFERRABILITY; can the evaluation of the
        //                              foreign key contraints be deferred until commit
        // The results are sorted by columns 1, 2, 3, and 9.
        //--------------------------------------------------------------------

        public ResultSet getImportedKeys (
                String catalog,
                String schema,
                String table)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getImportedKeys (" +
                                catalog + "," + schema + "," + table + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLForeignKeys
                try {
                        OdbcApi.SQLForeignKeys (hStmt,
                                null, null, null,
                                catalog, schema, table);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // If this is an ODBC 2.x version driver, we need to
                // manufacture some additional result set columns
                // (JdbcOdbcPseudoCol)
		//4532168
                if (Con.getODBCVer () >= 2) {
                        JdbcOdbcPseudoCol pc[] = new JdbcOdbcPseudoCol[1];

                        pc[0] = new JdbcOdbcPseudoCol ("DEFERRABILITY",
                                        Types.SMALLINT, 0);

                        // Set the new pseudo columns in the result set,
                        // supplying the first and last pseudo column
                        // numbers

                        rs.setPseudoCols (14, 14, pc);
                }

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getExportedKeys
        // Returns information about foreign keys in other
        // tables which reference the primary key in this table.  See also
        // getImportedKeys and getCrossReference.
        //
        // The columns in the ResultSet ares as for getImportedKeys.
        // The results are sorted by columns 5, 6, 7, and 9.
        //--------------------------------------------------------------------

        public ResultSet getExportedKeys ( 
                String catalog,
                String schema,
                String table)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getExportedKeys (" +
                                catalog + "," + schema + "," + table + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLForeignKeys
                try {
                        OdbcApi.SQLForeignKeys (hStmt,
                                catalog, schema, table,
                                null, null, null);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // If this is an ODBC 2.x version driver, we need to
                // manufacture some additional result set columns
                // (JdbcOdbcPseudoCol)
		//4532168
                if (Con.getODBCVer () >= 2) {
                        JdbcOdbcPseudoCol pc[] = new JdbcOdbcPseudoCol[1];

                        pc[0] = new JdbcOdbcPseudoCol ("DEFERRABILITY",
                                        Types.SMALLINT, 0);

                        // Set the new pseudo columns in the result set,
                        // supplying the first and last pseudo column
                        // numbers

                        rs.setPseudoCols (14, 14, pc);
                }

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getCrossReference
        // Returns information about the foreign key in
        // foreignTable which references the primary key for primaryTable.
        // See also getExportedKeys and getImportedKeys.
        //
        // The columns in the ResultSet ares as for getImportedKeys.
        // The results should be either zero or one rows and are not sorted.
        //--------------------------------------------------------------------

        public ResultSet getCrossReference (
                String primaryCatalog,
                String primarySchema,
                String primaryTable,
                String foreignCatalog,
                String foreignSchema,
                String foreignTable)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getCrossReference (" +
                                primaryCatalog + "," + primarySchema +
                                "," + primaryTable + "," +
                                foreignCatalog + "," + foreignSchema +
                                "," + foreignTable + ")");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLForeignKeys
                try {
                        OdbcApi.SQLForeignKeys (hStmt,
                                primaryCatalog, primarySchema,
                                primaryTable, foreignCatalog,
                                foreignSchema, foreignTable);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // If this is an ODBC 2.x version driver, we need to
                // manufacture some additional result set columns
                // (JdbcOdbcPseudoCol)
		//4532168
                if (Con.getODBCVer () >= 2) {
                        JdbcOdbcPseudoCol pc[] = new JdbcOdbcPseudoCol[1];

                        pc[0] = new JdbcOdbcPseudoCol ("DEFERRABILITY",
                                        Types.SMALLINT, 0);

                        // Set the new pseudo columns in the result set,
                        // supplying the first and last pseudo column
                        // numbers

                        rs.setPseudoCols (14, 14, pc);
                }

                // Return the result set

                return rs;
        }


        // New JDBC 2.0 API

        //--------------------------------------------------------------------
        // supportsResultSetType
        // Returns true if the cursor type is supported.
        //--------------------------------------------------------------------
        public boolean supportsResultSetType (
                int type)
                throws SQLException
        { 

		short supportedType = getConnectionSupportType(type);

		switch (type) 
		{

		    case ResultSet.TYPE_FORWARD_ONLY:
			    return ( supportedType == OdbcDef.SQL_CURSOR_FORWARD_ONLY );
		    case ResultSet.TYPE_SCROLL_INSENSITIVE:
			    return ( supportedType == OdbcDef.SQL_CURSOR_STATIC ||
				     supportedType == OdbcDef.SQL_CURSOR_KEYSET_DRIVEN );
		    case ResultSet.TYPE_SCROLL_SENSITIVE:

			    if ( supportedType == OdbcDef.SQL_CURSOR_KEYSET_DRIVEN )
			    {
				int cursorAttrs = Con.getOdbcCursorAttr2 (supportedType);

			    	if ((cursorAttrs & OdbcDef.SQL_CA2_SENSITIVITY_UPDATES) != 0)
				    return true;
				else 
				    return false;
			    }
			    else 
				return ( supportedType == OdbcDef.SQL_CURSOR_DYNAMIC );

		    default:
			    return false;
		}

	}

        //--------------------------------------------------------------------
        // supportsResultSetConcurrency
        // Returns true if the cursor and concurrency types are supported.
        //--------------------------------------------------------------------

        public boolean supportsResultSetConcurrency (
                int type,
                int concurrency)
                throws SQLException
        { 


	    if ( supportsResultSetType(type) )
	    {
		short supportedConCurr = Con.getOdbcConcurrency (concurrency);

		switch (concurrency)
		{
		    case ResultSet.CONCUR_READ_ONLY:
			    return (supportedConCurr == OdbcDef.SQL_CONCUR_READ_ONLY);
		    case ResultSet.CONCUR_UPDATABLE:
			    if ( type != ResultSet.TYPE_FORWARD_ONLY )
				return (supportedConCurr == OdbcDef.SQL_CONCUR_LOCK);
			    else
				return false;
		    default:
			    return false;
		}

	    }
	    else return false;
	    		    
	}

        //--------------------------------------------------------------------
        // ownUpdatesAreVisible
        // Returns true if the ResultSet's own updates are visible.
        //--------------------------------------------------------------------
        public boolean ownUpdatesAreVisible (
                int type)
                throws SQLException
        { 

	    if ( type != ResultSet.TYPE_FORWARD_ONLY )
	    {
		return updatesAreDetected(type);
	    }
	    else
		return false;
	
	}

        //--------------------------------------------------------------------
        // ownDeletesAreVisible
        // Returns true if the ResultSet's own deletes are visible.
        //--------------------------------------------------------------------
        public boolean ownDeletesAreVisible(
                int type)
                throws SQLException
        { 
	    if ( type != ResultSet.TYPE_FORWARD_ONLY )
	    {
		return deletesAreDetected(type);
	    }
	    else
		return false;	 
	}

        //--------------------------------------------------------------------
        // ownInsertsAreVisible
        // Returns true if the ResultSet's own inserts are visible.
        //--------------------------------------------------------------------
        public boolean ownInsertsAreVisible(
                int type)
                throws SQLException
        { 
	    if ( type != ResultSet.TYPE_FORWARD_ONLY )
	    {
		return insertsAreDetected(type);
	    }
	    else
		return false;	 
	}

        //--------------------------------------------------------------------
        // othersUpdatesAreVisible
        // Returns true if updates made by others are visible.
        //--------------------------------------------------------------------

        public boolean othersUpdatesAreVisible(
                int type)
                throws SQLException
        {	
	    if ( type == ResultSet.TYPE_SCROLL_SENSITIVE )
	    {
		return updatesAreDetected(type);
	    }
	    else
		return false;	 
	}

        //--------------------------------------------------------------------
        // othersDeletesAreVisible
        // Returns true if deletes made by others are visible.
        //--------------------------------------------------------------------

        public boolean othersDeletesAreVisible(
                int type)
                throws SQLException
        {	
	    if ( type == ResultSet.TYPE_SCROLL_SENSITIVE )
	    {
		return deletesAreDetected(type);
	    }
	    else
		return false;	 
	}

        //--------------------------------------------------------------------
        // othersInsertsAreVisible
        // Returns true if inserts made by others are visible.
        //--------------------------------------------------------------------

        public boolean othersInsertsAreVisible(
                int type)
                throws SQLException
        { 
	    if ( type == ResultSet.TYPE_SCROLL_SENSITIVE )
	    {
		return insertsAreDetected(type);
	    }
	    else
		return false;
	 }


        //--------------------------------------------------------------------
        // updatesAreDetected
        // Returns true if updates are detected.
        //--------------------------------------------------------------------
        public boolean updatesAreDetected(
                int type)
                throws SQLException
        { 
		short attrName = getCursorAttribute(type);

		if ( attrName > 0 )
		{
			try
			{
			    int visibleUpdates = OdbcApi.SQLGetInfo (hDbc, attrName);

			    return ( (visibleUpdates & OdbcDef.SQL_CA2_SENSITIVITY_UPDATES) > 0);
			}
			catch (SQLException e)
			{
			    // ignore exception.
			    return false;
			}
		}
		else		
		    return false;	
	}

        //--------------------------------------------------------------------
        // deletesAreDetected
        // Returns true if deletes are detected.
        //--------------------------------------------------------------------
        public boolean deletesAreDetected(
                int type)
                throws SQLException
        { 
		short attrName = getCursorAttribute(type);

		if ( attrName > 0 )
		{
			try
			{
			    int visibleUpdates = OdbcApi.SQLGetInfo (hDbc, attrName);

			    return ( (visibleUpdates & OdbcDef.SQL_CA2_SENSITIVITY_DELETIONS) > 0);
			}
			catch (SQLException e)
			{
			    // ignore exception.
			    return false;
			}
		}
		else		
		    return false;		
	}


        //--------------------------------------------------------------------
        // insertsAreDetected
        // Returns true if inserts are detected.
        //--------------------------------------------------------------------
        public boolean insertsAreDetected(
                int type)
                throws SQLException
        { 
		short attrName = getCursorAttribute(type);

		if ( attrName > 0 )
		{
			try
			{
			    int visibleUpdates = OdbcApi.SQLGetInfo (hDbc, attrName);

			    return ( (visibleUpdates & OdbcDef.SQL_CA2_SENSITIVITY_ADDITIONS) > 0);
			}
			catch (SQLException e)
			{
			    // ignore exception.
			    return false;
			}
		}
		else		
		    return false;		
	
	}


        //--------------------------------------------------------------------
        // supportsBatchUpdates
        // Returns true if batch Updates are supported.
        //--------------------------------------------------------------------
        public boolean supportsBatchUpdates()
                throws SQLException
        {                 
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.supportsBatchUpdates");
                }

		//assume nothing is supported before checking
                int batchSupport = 0;

		//local flags for Statement support.
		boolean explicitBatchSupport = false;
		boolean callBatchSupport = false;
		boolean prepareBatchSupport = false;
		
		try 
		{
			// Are Statements and Procedures supported?
			batchSupport = OdbcApi.SQLGetInfo (hDbc, OdbcDef.SQL_BATCH_SUPPORT);
			
			if ( (batchSupport & OdbcDef.SQL_BS_ROW_COUNT_EXPLICIT) > 0)		      
			{		 
			    explicitBatchSupport = true;
			}


			// Can obtain row counts from Prepared/Callable when executing 
			// them with array of parameters?
			batchSupport = OdbcApi.SQLGetInfo(hDbc, OdbcDef.SQL_PARAM_ARRAY_ROW_COUNTS);

			if ( (batchSupport & OdbcDef.SQL_PARC_BATCH) > 0)		      
			{		 
			    prepareBatchSupport = true;
			}


		}
		catch (SQLException e)
		{

		    //handle exception for non-batch supported Drivers
		    // assume nothing is supported if getInfo 
		    // Exception occurred during checkBatchUpdateSupport.
		   explicitBatchSupport = false;
		   prepareBatchSupport = false;
		}

		if ( (explicitBatchSupport == true) && (prepareBatchSupport == true) )
		{    
		    return true;
		}
		else
		{
		    // JDBC Bridge2 Driver supports it. 
		    // even if ODBC Driver does not.
		    return true;
		}
        }


        public ResultSet getUDTs(
                String catalog,
                String schemaPattern,
                String typeNamePattern,
                int[] types)
                throws SQLException
	{
		throw new UnsupportedOperationException();
	}


        public Connection getConnection()
                throws SQLException
	{
		if (Con != null && hDbc > 0)
		{
		    return Con;
		}
		else 
		    return null;
	}


        //--------------------------------------------------------------------
        // getTypeInfo
        // Returns information on all the standard SQL types
        // supported by the target database.
        //
        // The columns in the ResultSet are:
        //      Column (1)  String => Type name
        //      Column (2)  int    => SQL data type from java.sql.Types
        //      Column (3)  int    => maximum precision
        //      Column (4)  String => prefix used to quote a literal
        //      Column (5)  String => suffix used to quote a literal
        //      Column (6)  String => parameters used in creating the type
        //      Column (7)  boolean=> can you use NULL for this type?
        //      Column (8)  boolean=> is it case sensitive?
        //      Column (9)  boolean=> can you use "WHERE" based on this type:
//      int typeUnSearchable   = 0;     // No support
//      int typeSearchLikeOnly = 1;     // Only supported with WHERE .. LIKE
//      int typeSearchNotLike  = 2;     // Supported except for WHERE .. LIKE
//      int typeSearchable     = 3;     // Supported for all WHERE ..
        //      Column (10) boolean=> is it unsigned?
        //      Column (11) boolean=> can it be a money value?
        //      Column (12) boolean=> can it be used for an auto-increment
        //                            value?
        //      Column (13) String => localized version of type name
        //      Column (14) short  => minimum scale supported
        //      Column (15) short  => maximum scale supported
        //
        // The results are sorted by columns 2 and 1.
        //--------------------------------------------------------------------
        
        public ResultSet getTypeInfo ()
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getTypeInfo");
                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLGetTypeInfo
                try {
                        OdbcApi.SQLGetTypeInfo (hStmt,
                                OdbcDef.SQL_ALL_TYPES);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // If this is an ODBC 2.x version driver, we need to
                // manufacture some additional result set columns
                // (JdbcOdbcPseudoCol)

                if (Con.getODBCVer () == 2) {
                        JdbcOdbcPseudoCol pc[] = new JdbcOdbcPseudoCol[5];

                        pc[0] = new JdbcOdbcPseudoCol ("SQL_DATA_TYPE",
                                        Types.SMALLINT, 0);
                        pc[1] = new JdbcOdbcPseudoCol ("SQL_DATETIME_SUB",
                                        Types.SMALLINT, 0);
                        pc[2] = new JdbcOdbcPseudoCol ("NUM_PREC_RADIX",
                                        Types.SMALLINT, 0);

                        // Set the new pseudo columns in the result set,
                        // supplying the first and last pseudo column
                        // numbers

                        rs.setPseudoCols (16, 18, pc);
                        rs.setSQLTypeColumn (2);
                }
		else if (Con.getODBCVer () >= 3) {  // sun's 4234318 fix.
			rs.setSQLTypeColumn (2);
		}

		if (Con.getODBCVer () >= 2) {	// sun's 4234318 fix.
			//rename some columns in order to be JDBC compilant.
			rs.setAliasColumnName("PRECISION", 3);
			rs.setAliasColumnName("AUTO_INCREMENT", 12);
		}

                // Return the result set

                return rs;
        }

        //--------------------------------------------------------------------
        // getIndexInfo
        // Returns information about indexes for a given table.
        // If the "unique" argument is true then the results describes only
        // the indexes that are restricted to unique values.  If the 
        // "approximate" argument is true then the driver may use cached
        // and protentially out of date values for some ResultSet fields.
        //
        // The columns in the ResultSet are:
        //      Column (1)  String => catalog (may be NULL)
        //      Column (2)  String => schema (may be NULL)
        //      Column (3)  String => table name
        //      Column (4)  short  => Is this index always unique?
        //      Column (5)  String => index catalog (may be NULL)
        //      Column (6)  String => index name
        //      Column (7)  short  => index type:
//      short tableIndexStatistic = 0;
//      short tableIndexClustered = 1;
//      short tableIndexHashed    = 2;
//      short tableIndexOther     = 3;
        //      Column (8)  short  => column sequence number within index
        //      Column (9)  String => column name
        //      Column (10) String => column sort sequence, "A" => ascending,
        //                              "D" => descending, may be NULL. 
        //      Column (11) int  => If type == tableIndexStatisic then this
        //                      is the number of rows in the table, otherwise
        //                     it is the number of unique valies in the index.
        //                      may be NULL.
        //      Column (12) String => If type == tableIndexStatisic then this
        //                      is the number fo pages used for the table,
        //                      otherwise it is the mnumber of pages used for
        //                      the current index.  May be NULL.
        //      Column (13) String => Filter condition, if any.  (may be NULL)
        //
        // The results are sorted by columns 4, 5, 6, and 8.
        //--------------------------------------------------------------------

        public ResultSet getIndexInfo (
                String catalog,
                String schema,
                String table,
                boolean unique,
                boolean approximate)
                throws SQLException
        {
                if (OdbcApi.getTracer().isTracing ()) {
                        OdbcApi.getTracer().trace ("*DatabaseMetaData.getIndexInfo (" +
                                catalog + "," + schema + "," + table +
                                unique + "," + approximate + ")");

                }
                long hStmt;
                JdbcOdbcResultSet rs = null;
                SQLWarning      warning = null;

                // First, allocate a statement handle for the operation

                hStmt = OdbcApi.SQLAllocStmt (hDbc);

                // Call SQLStatistic
                try {
                        OdbcApi.SQLStatistics (hStmt,
                                catalog, schema, table,
                                unique, approximate);
                }
                catch (SQLWarning ex) {

                        // Save pointer to warning and save with ResultSet
                        // object once it is created.

                        warning = ex;
                }
                catch (SQLException ex) {

                        // If we got an exception, we need to clean up
                        // the statement handle we allocated, then
                        // re-throw the exception

                        OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);

                        throw ex;
                }
                                
                // Now, create a new ResultSet object, and initialize it

                rs = new JdbcOdbcResultSet ();
                rs.initialize (OdbcApi, hDbc, hStmt, false, null);
                rs.setWarning (warning);

                // Return the result set

                return rs;
        }


        //--------------------------------------------------------------------
        // validateConnection
        // Validates that this connection is not closed.  If the connection
        // is closed, an exception is thrown
        //--------------------------------------------------------------------

        protected void validateConnection ()
                        throws SQLException
        {
                Con.validateConnection ();
        }

        //====================================================================
        // Protected methods
        //====================================================================

        //--------------------------------------------------------------------
        // getInfo
        // Returns the SQLGetInfo numeric value for the given info type
        //--------------------------------------------------------------------

        protected int getInfo (
                short infoType)
                throws SQLException
        {
                // Validate that the connection is valid

                validateConnection ();

                return OdbcApi.SQLGetInfo (hDbc, infoType);
        }

        //--------------------------------------------------------------------
        // getInfoShort
        // Returns the SQLGetInfo short numeric value for the given info type
        //--------------------------------------------------------------------

        protected int getInfoShort (
                short infoType)
                throws SQLException
        {
                // Validate that the connection is valid

                validateConnection ();

                return OdbcApi.SQLGetInfoShort (hDbc, infoType);
        }

        //--------------------------------------------------------------------
        // getInfoBooleanString
        // Returns true if the SQLGetInfo for the given info type is a
        // string containing "Y"
        //--------------------------------------------------------------------

        protected boolean getInfoBooleanString (
                short infoType)
                throws SQLException
        {
                String  value;

                // Validate that the connection is valid

                validateConnection ();

                value = OdbcApi.SQLGetInfoString (hDbc, infoType);

                return value.equalsIgnoreCase ("Y");
        }

        //--------------------------------------------------------------------
        // getInfoString
        // Returns the SQLGetInfo string for the given info type
        //--------------------------------------------------------------------

        protected String getInfoString (
                short infoType)
                throws SQLException
        {
                // Validate that the connection is valid

                validateConnection ();

                return OdbcApi.SQLGetInfoString (hDbc, infoType);
        }

        //--------------------------------------------------------------------
        // getInfoString
        // Optional interface to supply size of return buffer.
        // Returns the SQLGetInfo string for the given info type
        //--------------------------------------------------------------------

        protected String getInfoString (
                short infoType,
                int buffSize)
                throws SQLException
        {
                // Validate that the connection is valid

                validateConnection ();

                return OdbcApi.SQLGetInfoString (hDbc, infoType, buffSize);
        }


        //--------------------------------------------------------------------
        // getConnectionSupportType
        // Return the best ODBC cursor Type supported by the connection.
        //--------------------------------------------------------------------

	protected short getConnectionSupportType(int type)
                throws SQLException
	{
		short supportedType = Con.getOdbcCursorType (type);

		if (supportedType == -1) 
		{			
		    supportedType = Con.getBestOdbcCursorType();		    
		}
		return supportedType;
	}

        //--------------------------------------------------------------------
        // getCursorAttribute
        // Returns ODBC Attribute value for the given supported-cursor-type.
        //--------------------------------------------------------------------

	protected short getCursorAttribute(int type)
                throws SQLException
	{
		short attrName = 0;
		
		if ( supportsResultSetType(type) )
		{
			short attrValue = getConnectionSupportType(type);
			
			switch (attrValue) 
			{
			    case OdbcDef.SQL_CURSOR_KEYSET_DRIVEN:
				    attrName = OdbcDef.SQL_KEYSET_CURSOR_ATTRIBUTES2;
				    break;
			    case OdbcDef.SQL_CURSOR_DYNAMIC:
				    attrName = OdbcDef.SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
				    break;
			    case OdbcDef.SQL_CURSOR_STATIC:
				    attrName = OdbcDef.SQL_STATIC_CURSOR_ATTRIBUTES2;
				    break;
			}
			return attrName;
		}
		else		
		    return attrName;
	}

    //------------------------------------------------------------------
    // JDBC 3.0 API Changes
    //------------------------------------------------------------------

    public boolean supportsSavepoints() throws SQLException {
	return false;
    }

    public boolean supportsNamedParameters() throws SQLException {
	return false;
    }

    public boolean supportsMultipleOpenResults() throws SQLException {
	return false;
    }

    public boolean supportsGetGeneratedKeys() throws SQLException {
	return false;
    }

    public ResultSet getSuperTypes(String catalog, String schemaPattern, 
			    String typeNamePattern) throws SQLException {
	throw new UnsupportedOperationException();
    }
    
    public ResultSet getSuperTables(String catalog, String schemaPattern,
				    String tableNamePattern) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public ResultSet getAttributes(String catalog, String schemaPattern,
			    String typeNamePattern, String attributeNamePattern) 
	throws SQLException {
	throw new UnsupportedOperationException();
    }


    public boolean supportsResultSetHoldability(int holdability) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int getResultSetHoldability() throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int getDatabaseMajorVersion() throws SQLException { 
	throw new UnsupportedOperationException ();
    }

    public int getDatabaseMinorVersion() throws SQLException {
	throw new UnsupportedOperationException ();
    }

    public  int getSQLStateType() throws SQLException {
	return java.sql.DatabaseMetaData.sqlStateXOpen;
    }
    
    public int getJDBCMajorVersion() throws SQLException {
	return 2;
    }

    public int getJDBCMinorVersion() throws SQLException {
	return 0;
    }

    public boolean locatorsUpdateCopy() throws SQLException {
	throw new UnsupportedOperationException();
    }

    public boolean supportsStatementPooling() throws SQLException {
	throw new UnsupportedOperationException();
    }

        //====================================================================
        // Data attributes
        //====================================================================

        protected JdbcOdbc OdbcApi;             // ODBC API interface object

        protected JdbcOdbcConnectionInterface Con;
                                                // Owning connection object

        protected long   hDbc;                   // Database connection handle        

}
