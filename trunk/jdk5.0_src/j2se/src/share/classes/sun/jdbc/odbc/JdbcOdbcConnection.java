/*
 * @(#)JdbcOdbcConnection.java	1.31 00/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcConnection.java
//
// Description: Impementation of the Connection interface class
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

import java.util.Map;
import java.util.Hashtable;
import java.util.Vector;
import java.sql.*;
import sun.io.*;

public class JdbcOdbcConnection
	extends		JdbcOdbcObject
	implements	JdbcOdbcConnectionInterface
{

	//====================================================================
	// Public methods
	//====================================================================

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------
	
	public JdbcOdbcConnection (
		JdbcOdbc odbcApi,
		long env,
		JdbcOdbcDriverInterface driver)
	{
		// Save the ODBC API object, and our environment handle

		OdbcApi = odbcApi;
		
		// RFE 4641013.
		tracer = OdbcApi.getTracer();
		
		myDriver = driver;
		hEnv = env;

		// Init the connection handle, URL

		hDbc = 0;
		URL = null;

		// Init the warning list
		
		lastWarning = null;

		// Default the connection to closed
		
		closed = true;
		freeStmtsFromConnectionOnly = false; // 4524683
	}

	//--------------------------------------------------------------------
	// finalize
	// Perform any cleanup when this object is garbage collected
	//--------------------------------------------------------------------

	protected void finalize ()
	{
		if (tracer.isTracing ()) {
			tracer.trace ("Connection.finalize " + this);
		}

		try {
			close ();
		}
		catch (SQLException ex) {
			// If an exception is thrown, ignore it
		}
	}

	//--------------------------------------------------------------------
	// initialize
	// Finish initializing the connection object.  This includes 
	// allocating a connection handle, and performing the connect.
	//--------------------------------------------------------------------

	public void initialize (
		String dsn, 
		java.util.Properties info,
		int timeout)
		throws SQLException
	{
		String	uid;
		String	pwd;
		String	connectString = "";
		String	rowSize;

		// use to parse properties UID and PWD into connectString
		// also look for odbcRowSetSize property to pass to Statements.
		String  connectProps	= "";
		String  userTok		= null;
		String  passwordTok	= null;
		String	rowSizeTok	= null;
		String	currentToken	= ""; // token monitor.

		String licenseFileName;
		String licenseFilePassword;
	
		// If we don't have a connection handle for this object,
		// allocate one

		if (closed) {
			hDbc = myDriver.allocConnection (hEnv);
		}

		// If a login timeout was supplied, set the new login
		// timeout value
		if (timeout > 0) {
			setLoginTimeout (timeout);
		}
		

		// Get the user's preffered rowSetSize proverty.
		rowSize = info.getProperty ("odbcRowSetSize");
		if (rowSize != null)
		    setResultSetBlockSize( rowSize );

		//try to override the default charSet value
		OdbcApi.charSet = info.getProperty ("charSet", System.getProperty("file.encoding"));

		// Get INTERSOLV License File Name and Password
		licenseFileName = info.getProperty ("licfile", "");
		licenseFilePassword = info.getProperty ("licpwd", "");

		// Now build the connection string		
		// Get the username and password from the Properties table
		// If not such Properties, null is spected.
		uid = info.getProperty ("user", "");
		pwd = info.getProperty ("password", "");
		//uid = info.getProperty ("user");
		//pwd = info.getProperty ("password");
						
		//connectString = "DSN=" + dsn;
		//String tokenizedProps = "DSN=" + dsn;
		String tokenizedProps = null;
		if (dsn.indexOf("DRIVER") != -1 || dsn.indexOf("Driver") != -1 ||
			dsn.indexOf("driver") != -1)
			tokenizedProps = dsn;
		else
			tokenizedProps = "DSN=" + dsn;
		
		// Some ODBC drivers fail when user/password 
		// key words are used in the URL connection string
		// parsing the string to change user and password to
		// UID and PWD respectively.
		java.util.StringTokenizer st = new java.util.StringTokenizer(tokenizedProps, ";", false);

		if (st.countTokens() > 1)
		{
		    int tokcount = 0;

		    while (st.hasMoreTokens()) 
		    {
			tokcount++;
			String token = st.nextToken();

			if ( token.startsWith("user") )
			{
			    userTok = token;
			}
			else if ( token.startsWith("password") )
			{
			    passwordTok = token;
			}
			else if ( token.startsWith("odbcRowSetSize") )
			{
			    rowSizeTok = token;
			}
			else 
			{
			    if (tokcount > 1)
			    {
				connectProps += ";" + token;
			    }
			    else connectProps += token;
			}
		    }
		}
		else 
		{
			connectProps = tokenizedProps;
		}
		
		// done tokenizing!
		// user and password are not in the
		// connect string at this point.
		connectString = connectProps;

		
		// Property values have precedence 
		// over the Url's string values
		try
		{

		    if ( (uid.equals("")) && (userTok != null) )
			//if ( (uid == null) && (userTok != null) ) 
		    {
			    currentToken = userTok;

			    String newUid = userTok.substring(4);
				if (!newUid.equals(""))
				{
					//token takes precedence over the 
					//UID=value key-value pair of the Url String
					if (connectString.indexOf("UID=") == -1) 
						connectString += ";UID" + newUid;
					else 
					{
						int uidIndex = connectString.indexOf("UID=");
						int seperatorIndex = connectString.indexOf(";", uidIndex);
						StringBuffer strBuf = new StringBuffer(connectString);
						strBuf.replace(uidIndex, seperatorIndex, "UID="+newUid);
						connectString = strBuf.toString();
					}
				}
		    }
			else 
			{
				if (!uid.equals(""))
				{
					//property value takes precedence over the 
					//UID=value key-value pair of the Url String
					if (connectString.indexOf("UID=") == -1)
						connectString += ";UID=" + uid;
					else
					{
						int uidIndex = connectString.indexOf("UID=");
						int seperatorIndex = connectString.indexOf(";", uidIndex);
						StringBuffer strBuf = new StringBuffer(connectString);
						strBuf.replace(uidIndex, seperatorIndex, "UID="+uid);
						connectString = strBuf.toString();
					}
				}
			}

		    
		    if ( (pwd.equals("")) && (passwordTok != null) )
			//if ( (pwd == null) && (passwordTok != null) ) 
		    {
			    currentToken = passwordTok;

			    String newPwd = passwordTok.substring(8);
				//token takes precedence over the 
				//PWD=value key-value pair of the Url String
			    if (connectString.indexOf("UID=") != -1)
				{
					if (connectString.indexOf("PWD=") == -1)
						connectString += ";PWD" + newPwd;
					else
					{
						int pwdIndex = connectString.indexOf("PWD=");
						int seperatorIndex = connectString.indexOf(";", pwdIndex);
						StringBuffer strBuf = new StringBuffer(connectString);
						strBuf.replace(pwdIndex, seperatorIndex, "PWD="+newPwd);
						connectString = strBuf.toString();
					}
				}
		    }
		    else 
			{
				if (!pwd.equals(""))
				{
					//property value takes precedence over the
					//PWD=value key-value pair of the Url String
					if (connectString.indexOf("UID=") != -1)
					{
						if (connectString.indexOf("PWD=") == -1)
							connectString += ";PWD=" + pwd;
						else
						{
							int pwdIndex = connectString.indexOf("PWD=");
							int seperatorIndex = connectString.indexOf(";", pwdIndex);
							StringBuffer strBuf = new StringBuffer(connectString);
							strBuf.replace(pwdIndex, seperatorIndex, "PWD="+pwd);
							connectString = strBuf.toString();
						}
					}
				}
			}

		    if ( (rowSize == null) && (rowSizeTok != null) )
		    {
			    currentToken = rowSizeTok;

			    String newRowSize = rowSizeTok.substring(15);

			    setResultSetBlockSize(newRowSize);
		    }
		}
		catch (java.lang.StringIndexOutOfBoundsException be)
		{
		    throw new SQLException ("invalid property values [" + currentToken + "]");
		}


		// Perform the connect.  If we get a SQLWarning, save it;
		// the application will have to poll for it

		try {
			OdbcApi.SQLDriverConnect (hDbc, connectString);
		}
		catch (SQLWarning ex) {
			lastWarning = ex;
		}
		catch (SQLException ex) {

			// If an exception was raised, gracefully close
			// down the connection.  This includes freeing the
			// connection handle

			myDriver.closeConnection (hDbc);

			// Re-throw the exception

			throw ex;
		}

		// We are now open!

		closed = false;

		// Set closed driver license file and password

		if (licenseFileName != null) {
			//setLicenseFile(licenseFileName);
		}

		if (licenseFilePassword != null) {
			//setLicensePassword(licenseFilePassword);
		}

		// Create a new Hashtable to store each of our Statement objects

		statements = new java.util.WeakHashMap (); //4524683

		// Create a new Hashtable to store batch Vectors for each of our Statement objects

		batchStatements = new Hashtable ();
		
		DatabaseMetaData dma = getMetaData ();

		OdbcApi.odbcDriverName = dma.getDriverName () + " " + dma.getDriverVersion ();

		// If tracing, tracer.trace the driver name and version
		if (tracer.isTracing()) {
			//DatabaseMetaData dma = getMetaData ();
	
			tracer.trace("Driver name:    " + dma.getDriverName ());
			tracer.trace("Driver version: " + dma.getDriverVersion ());
		}
		else
		    dma = null;

		// Now that the connection has been established, get some default
		// information.  This includes building a Hashtable with all of the
		// SQL type information

		buildTypeInfo ();

		checkScrollCursorSupport ();
		
		checkBatchUpdateSupport();
	}

	//--------------------------------------------------------------------
	// createStatement
	// Returns an SQL Statement object
	//--------------------------------------------------------------------

	public Statement createStatement ()
		throws SQLException 
	{
		return createStatement (
				ResultSet.TYPE_FORWARD_ONLY,
				ResultSet.CONCUR_READ_ONLY);

	}

	public Statement createStatement(
		int resultSetType,
		int resultSetConcurrency)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.createStatement");
		}

		// First, allocate a statement handle for the operation

		long hStmt = OdbcApi.SQLAllocStmt (hDbc);

		// Create a new Statement object

		JdbcOdbcStatement stmt = new JdbcOdbcStatement (this);

		// Initialize the object.  This will also allocate a
		// statement handle.

		stmt.initialize (OdbcApi, hDbc, hStmt, null,
				resultSetType, resultSetConcurrency);

		stmt.setBlockCursorSize (rsBlockSize);

		// Keep a reference of this statement object

		registerStatement (stmt);

		return stmt;
	}

	//--------------------------------------------------------------------
	// prepareStatement
	// prepareStatement creates a precompiled SQL PreparedStatement object.
	//--------------------------------------------------------------------
	
	public PreparedStatement prepareStatement (
		String sql)
		throws SQLException
	{
		return prepareStatement (sql,
				ResultSet.TYPE_FORWARD_ONLY,
				ResultSet.CONCUR_READ_ONLY);
	}
	
	public PreparedStatement prepareStatement (
		String sql,
		int resultSetType,
		int resultSetConcurrency)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.prepareStatement (" + sql + ")");
		}

		long hStmt;
		JdbcOdbcPreparedStatement ps = null;
		SQLWarning	warning = null;


		// First, allocate a statement handle for the operation

		hStmt = OdbcApi.SQLAllocStmt (hDbc);

		// Now, create a new PreparedStatement object, and initialize

		ps = new JdbcOdbcPreparedStatement (this);
		ps.initialize (OdbcApi, hDbc, hStmt, typeInfo,
				resultSetType, resultSetConcurrency);

		// Call SQLPrepare
		try {
			OdbcApi.SQLPrepare (hStmt, sql);
		}
		catch (SQLWarning ex) {

			// Save pointer to warning and save with
			// PreparedStatement object once it is created.

			warning = ex;
		}
		catch (SQLException ex) {

			// If we got an exception, we need to clean up
			// the statement handle we allocated, then
			// re-throw the exception

			//OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);
			ps.close();
			throw ex;
		}

		ps.initBoundParam ();
						
		ps.setWarning (warning);

		ps.setBlockCursorSize (rsBlockSize);

		// save the SQL for this statement
		// replace the parameters with setXXX().
		ps.setSql(sql);

		// Keep a reference of this statement object

		registerStatement (ps);

		// Return the prepared statement

		return ps;
	}

	//--------------------------------------------------------------------
	// prepareCall
	// prepareCall create a pre-compiled SQL statement that is
	// a call on a stored procedure.
	//--------------------------------------------------------------------

	public CallableStatement prepareCall (
		String sql)
		throws SQLException
	{
		return prepareCall (sql,
				ResultSet.TYPE_FORWARD_ONLY,
				ResultSet.CONCUR_READ_ONLY);
	}			

	public CallableStatement prepareCall(
		String sql,
		int resultSetType,
		int resultSetConcurrency)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.prepareCall (" + sql + ")");
		}

		long hStmt;
		JdbcOdbcCallableStatement cs = null;
		SQLWarning	warning = null;

		// First, allocate a statement handle for the operation

		hStmt = OdbcApi.SQLAllocStmt (hDbc);

		// Now, create a new CallableStatement object, and initialize

		cs = new JdbcOdbcCallableStatement (this);
		cs.initialize (OdbcApi, hDbc, hStmt, typeInfo,
				resultSetType, resultSetConcurrency);

		// Call SQLPrepare
		try {
			OdbcApi.SQLPrepare (hStmt, sql);
		}
		catch (SQLWarning ex) {

			// Save pointer to warning and save with
			// PreparedStatement object once it is created.

			warning = ex;
		}
		catch (SQLException ex) {

			// If we got an exception, we need to clean up
			// the statement handle we allocated, then
			// re-throw the exception

			//OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);
			cs.close();
			throw ex;
		}

		cs.initBoundParam ();
				
		cs.setWarning (warning);

		cs.setBlockCursorSize (rsBlockSize);

		// save the SQL for this statement
		// replace the parameters with setXXX().
		cs.setSql(sql);

		// Keep a reference of this statement object

		registerStatement (cs);

		// Return the callable statement

		return cs;
	}

						
	//--------------------------------------------------------------------
	// nativeSQL
	// Convert the given generic SQL statement to the drivers native SQL.
	//--------------------------------------------------------------------

	public String nativeSQL (
		String query)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.nativeSQL (" + query + ")");
		}

		String nativeQuery;

		try {
			nativeQuery = OdbcApi.SQLNativeSql (hDbc, query);
		}
		catch (SQLException ex) {

			// If an exception is thrown, simply return the
			// original string
			nativeQuery = query;
		}
		return nativeQuery;
	}

	//--------------------------------------------------------------------
	// setAutoCommit
	// If "autoCommit" is true, then all subsequent SQL statements will
	// be executed and committed as individual transactions.  Otherwise
	// (if "autoCommit" is false) then subsequent SQL statements will
	// all be part of the same transaction , which must be expicitly
	// committed with either a "commit" or "rollback" call.
  	// By default new connections are initialized with autoCommit "true".
	//--------------------------------------------------------------------
	
	public void setAutoCommit(
		boolean enableAutoCommit)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setAutoCommit (" + 
				enableAutoCommit + ")");
		}

		int	mode = OdbcDef.SQL_AUTOCOMMIT_ON;

		// Validate that the connection is valid

		validateConnection ();

		if (!enableAutoCommit) {
			mode = OdbcDef.SQL_AUTOCOMMIT_OFF;
		}

		OdbcApi.SQLSetConnectOption (hDbc, OdbcDef.SQL_AUTOCOMMIT,
				mode);
	}

	//--------------------------------------------------------------------
	// getAutoCommit
	// Test whether "autoCommit" is true.
	//--------------------------------------------------------------------

	public boolean getAutoCommit()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.getAutoCommit");
		}

		int	mode;
		boolean rc = false;

		// Validate that the connection is valid

		validateConnection ();

		mode = (int)OdbcApi.SQLGetConnectOption (hDbc,
				OdbcDef.SQL_AUTOCOMMIT);

		if (mode == OdbcDef.SQL_AUTOCOMMIT_ON) {
			rc = true;
		}
		return rc;
	}


	//--------------------------------------------------------------------
	// commit
	// You can use commit or rollback to commit or abort a transaction.
	// Note that by default all active Statement or ResultSet objects
	// associated with a Connection will be closed when it is committed
	// or aborted.
	//--------------------------------------------------------------------
	
	public void commit ()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.commit");
		}
		// Validate that the connection is valid
		validateConnection ();
		OdbcApi.SQLTransact (hEnv, hDbc, OdbcDef.SQL_COMMIT);
	}

	//--------------------------------------------------------------------
	// rollback
	//--------------------------------------------------------------------

	public void rollback ()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.rollback");
		}
		// Validate that the connection is valid
		validateConnection ();
		OdbcApi.SQLTransact (hEnv, hDbc, OdbcDef.SQL_ROLLBACK);
	}

	//--------------------------------------------------------------------
	// close
	// close frees up various state associated with the connection.
	// This includes freeing the connection handle, and disconnecting.
	//--------------------------------------------------------------------

	public void close () throws SQLException
	{

		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.close");
		}

		// Close any statement objects that we still have a reference to
		// RFE 4641013
		setFreeStmtsFromConnectionOnly(); 
		closeAllStatements ();

		// If we still have a connection handle, close it

		if (!closed) {
			myDriver.disconnect (hDbc);
			myDriver.closeConnection (hDbc);
		}

		closed = true;
		URL = null;
	}
	
	// 4524683
	public boolean isFreeStmtsFromConnectionOnly() {
		return freeStmtsFromConnectionOnly;
	}

	//--------------------------------------------------------------------
	// RFE 4641013.
	// freeStmtsFromConnectionOnly
	// Changes the status to make sure that statements will be freed from
	// Connection only. This is used by connection pool implementation.
	//--------------------------------------------------------------------
	public void setFreeStmtsFromConnectionOnly(){
	     freeStmtsFromConnectionOnly = true; 
	}
	
	//--------------------------------------------------------------------
	// RFE 4641013.
	// freeStmtsFromAnyWhere
	// Changes the status to make sure that statements can be freed from
	// anywhere. This is used by connection pool implementation.
	//--------------------------------------------------------------------	
	public void setFreeStmtsFromAnyWhere(){
	     freeStmtsFromConnectionOnly = false; 
	}		 
	
	//--------------------------------------------------------------------
	// isClosed
	// isClosed returns true if the connection is closed, which can
	// occur either due to an explicit call on "close" or due to
	// some fatal error on the connection.
	//--------------------------------------------------------------------

	public boolean isClosed() throws SQLException {
		return closed;
	}


	// You can obtain a DatabaseMetaData object to get information 
	// about the target database.
	public DatabaseMetaData getMetaData()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.getMetaData");
		}

		JdbcOdbcDatabaseMetaData metaData;

		// Validate that the connection is valid

		validateConnection ();

		// Create the new MetaData object, and return it
		
		metaData = new JdbcOdbcDatabaseMetaData (OdbcApi, this);

		return metaData;
	}

	//--------------------------------------------------------------------
	// setReadOnly
	// You can put a connection in readonly mode as a hint to enable 
	// database optimizations.  Note that setReadOnly cannot be called
	// while in the middle of a transaction.
	//--------------------------------------------------------------------

	public void setReadOnly (
		boolean readOnly)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setReadOnly (" + readOnly + ")");
		}

		int	mode = OdbcDef.SQL_MODE_READ_WRITE;

		// Validate that the connection is valid

		validateConnection ();

		if (readOnly) {
			mode = OdbcDef.SQL_MODE_READ_ONLY;
		}

		// Since this is just a hint for ODBC, we'll catch any exceptions

		try {
			OdbcApi.SQLSetConnectOption (hDbc, OdbcDef.SQL_ACCESS_MODE,
							mode);
		}
		catch (SQLException ex) {
			if (tracer.isTracing()) {
				tracer.trace ("setReadOnly exception ignored");
			}
		}
	}

	//--------------------------------------------------------------------
	// isReadOnly
	// Returns TRUE if the connection is in read-only mode
	//--------------------------------------------------------------------

	public boolean isReadOnly()
		throws SQLException 
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.isReadOnly");
		}

		int	mode;
		boolean rc = false;

		// Validate that the connection is valid

		validateConnection ();

		mode = (int)OdbcApi.SQLGetConnectOption (hDbc,
				OdbcDef.SQL_ACCESS_MODE);

		if (mode == OdbcDef.SQL_MODE_READ_ONLY) {
			rc = true;
		}
		return rc;
	}

	//--------------------------------------------------------------------
	// The "catalog" selects a database.
	//--------------------------------------------------------------------

	public void setCatalog(
		String catalog)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setCatalog (" + catalog + ")");
		}

		// Validate that the connection is valid

		validateConnection ();

		OdbcApi.SQLSetConnectOption (hDbc,
				OdbcDef.SQL_CURRENT_QUALIFIER, catalog);
	}

	//--------------------------------------------------------------------
	// getCatalog
	// Gets the current catalog for the connection
	//--------------------------------------------------------------------

	public String getCatalog()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.getCatalog");
		}

		// Validate that the connection is valid

		validateConnection ();

		return OdbcApi.SQLGetInfoString (hDbc, 
				OdbcDef.SQL_DATABASE_NAME);
	}

	//--------------------------------------------------------------------
	// setTransactionIsolation
	// You can call the following method to try to change the transaction
	// isolation level on a newly opened connection, using one of the 
	// TRANSACTION_* values.  Use the DatabaseMetaData class to find what
   	// isolation levels are supported by the current database.
 	// Note that setTransactionIsolation cannot be called while in the
	// middle of a transaction.
	//--------------------------------------------------------------------

	public void setTransactionIsolation (
		int level)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setTransactionIsolation (" +
				level + ")");
		}

		// Validate that the connection is valid

		validateConnection ();

		switch (level) {
		case TRANSACTION_NONE:

			// No transactions, turn on auto-commit

			setAutoCommit (true);
			break;
		case TRANSACTION_READ_UNCOMMITTED:
			setAutoCommit (false);
			OdbcApi.SQLSetConnectOption (hDbc, 
				OdbcDef.SQL_TXN_ISOLATION,
				OdbcDef.SQL_TXN_READ_UNCOMMITTED);
			break;
		case TRANSACTION_READ_COMMITTED:
			setAutoCommit (false);
			OdbcApi.SQLSetConnectOption (hDbc, 
				OdbcDef.SQL_TXN_ISOLATION,
				OdbcDef.SQL_TXN_READ_COMMITTED);
			break;
		case TRANSACTION_REPEATABLE_READ:
			setAutoCommit (false);
			OdbcApi.SQLSetConnectOption (hDbc, 
				OdbcDef.SQL_TXN_ISOLATION,
				OdbcDef.SQL_TXN_REPEATABLE_READ);
			break;
		case TRANSACTION_SERIALIZABLE:
			setAutoCommit (false);
			OdbcApi.SQLSetConnectOption (hDbc, 
				OdbcDef.SQL_TXN_ISOLATION,
				OdbcDef.SQL_TXN_SERIALIZABLE);
			break;
		default:

			// Unknown option.  Call the driver with the option,
			// if it is invalid the driver will let us know

			setAutoCommit (false);
			OdbcApi.SQLSetConnectOption (hDbc,
				OdbcDef.SQL_TXN_ISOLATION, level); 
			break;
		}
	}

	//--------------------------------------------------------------------
	// setLicenseFile
	// The following method will set the License File for INTERSOLV's
	// closed drivers.
	//--------------------------------------------------------------------

	public void setLicenseFile (
		String licenseFileName)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setLicenseFile (" +
				licenseFileName + ")");
		}

		// Validate that the connection is valid

		// validateConnection ();

		OdbcApi.SQLSetConnectOption (hDbc,
			OdbcDef.SQL_LIC_FILE_NAME,
			licenseFileName);

	}

	//--------------------------------------------------------------------
	// setLicensePassword
	// The following method will set the password for INTERSOLV's
	// closed drivers.
	//--------------------------------------------------------------------

	public void setLicensePassword (
		String password)
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.setPassword (" +
				password + ")");
		}

		// Validate that the connection is valid

		// validateConnection ();

		OdbcApi.SQLSetConnectOption (hDbc,
			OdbcDef.SQL_LIC_FILE_PASSWORD,
			password);

	}

	//--------------------------------------------------------------------
	// getTransactionIsolation
	//--------------------------------------------------------------------

	public int getTransactionIsolation ()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.getTransactionIsolation");
		}

		int isolation = TRANSACTION_NONE;
		int SQLIsolation;

		// Validate that the connection is valid

		validateConnection ();

		SQLIsolation = (int)OdbcApi.SQLGetConnectOption (hDbc,
				OdbcDef.SQL_TXN_ISOLATION);

		// SQLGetConnectOption with SQL_TXN_ISOLATION actually
		// returns a bitmask, but we'll check for equality

		switch (SQLIsolation) {
		case OdbcDef.SQL_TXN_READ_UNCOMMITTED:
			isolation = TRANSACTION_READ_UNCOMMITTED;
			break;
		case OdbcDef.SQL_TXN_READ_COMMITTED:
			isolation = TRANSACTION_READ_COMMITTED;
			break;
		case OdbcDef.SQL_TXN_REPEATABLE_READ:
			isolation = TRANSACTION_REPEATABLE_READ;
			break;
		case OdbcDef.SQL_TXN_SERIALIZABLE:
			isolation = TRANSACTION_SERIALIZABLE;
			break;
		default:
			isolation = SQLIsolation;
		}
		return isolation;
	}

	//--------------------------------------------------------------------
	// getWarnings
	// Return any warning information related to the current connection.
	// Note that SQLWarning may be a chain.  Returns null if no warnings
	// exist.
	//--------------------------------------------------------------------

	public SQLWarning getWarnings()
			throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("*Connection.getWarnings");
		}
		return lastWarning;
	}

	//--------------------------------------------------------------------
	// clearWarnings
	// Clears any warning information for the connection
	//--------------------------------------------------------------------

	public void clearWarnings()
			throws SQLException
	{
		lastWarning = null;
	}

	//--------------------------------------------------------------------
	// validateConnection
	// Validates that this connection is not closed.  If the connection
	// is closed, throw an exception
	//--------------------------------------------------------------------

	public void validateConnection ()
			throws SQLException
	{
		//RFE 4641013.
		if (closed) {
			throw new SQLException ("Connection is closed");
		}
	}

	//--------------------------------------------------------------------
	// getHDBC
	// Returns the connection handle
	//--------------------------------------------------------------------
	public long getHDBC ()
	{
		return hDbc;
	}

	//--------------------------------------------------------------------
	// setURL
	// Sets the URL for the connection
	//--------------------------------------------------------------------
	public void setURL (
		String url)
	{
		URL = url;
	}

	//--------------------------------------------------------------------
	// getURL
	// Returns the URL for the connection
	//--------------------------------------------------------------------
	public String getURL ()
	{
		return URL;
	}

	//--------------------------------------------------------------------
	// setLoginTimeout
	// Sets the login timeout for the connection
	//--------------------------------------------------------------------

	protected void setLoginTimeout(
		int seconds)
		throws SQLException
	{
		OdbcApi.SQLSetConnectOption (hDbc, OdbcDef.SQL_LOGIN_TIMEOUT,
				seconds);
	}


	//--------------------------------------------------------------------
	// getODBCVer
	// Queries the data source and gets the current ODBC version number.
	// The major version is returned as an int.  Returns -1 if unknown.
	//--------------------------------------------------------------------

	public int getODBCVer ()
	{
		if (odbcVer == 0) {

			String	s;

			try {
				s =  OdbcApi.SQLGetInfoString (hDbc, 
					OdbcDef.SQL_ODBC_VER);
			}
			catch (SQLException ex) {
				s = "-1";
			}

			// Convert first two characters of version (major
			// version) to an integer value.

			Integer i = new Integer (s.substring (0, 2));

			// Cache the value

			odbcVer = i.intValue ();
		}

		return odbcVer;
	}


	// New JDBC 2.0 API

	protected void checkBatchUpdateSupport()
	{

		//assume nothing is supported before checking
		batchInStatements = -1;
		batchInProcedures = -1;
		batchInPrepares = -1;

		// GetInfo results for batch Support and Row Counts.
		int batchSupport = -1;
		int batchRCType = -1;

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

			if ( (batchSupport & OdbcDef.SQL_BS_ROW_COUNT_PROC) > 0 )
			{
			    callBatchSupport = true;
			} 

			// behavior of the driver with respect to the availability of 
			// row counts. 
			batchRCType = OdbcApi.SQLGetInfo (hDbc, OdbcDef.SQL_BATCH_ROW_COUNT);
			
			// if row counts are rolled-up. Batch can only
			// get row counts for individual estatements.
			// through emulation.
			if ( (batchRCType & OdbcDef.SQL_BRC_ROLLED_UP) > 0)
			{
				batchInStatements = OdbcDef.SQL_BRC_ROLLED_UP;
				batchInProcedures = OdbcDef.SQL_BRC_ROLLED_UP;			
			}
			else
			{
			    // Are row counts available for explicit Statements			    
			    if (explicitBatchSupport)
			    {
				if ( (batchRCType & OdbcDef.SQL_BRC_EXPLICIT) > 0)
				{			
				    batchInStatements = OdbcDef.SQL_BRC_EXPLICIT;
				}
			    }
			    
			    // Are row counts available for procedures			    			    
			    if (callBatchSupport)
			    {
				if ( (batchRCType & OdbcDef.SQL_BRC_PROCEDURES) > 0)
				{
				    batchInProcedures = OdbcDef.SQL_BRC_PROCEDURES;
				}
			    }
			}


			// Can obtain row counts from Prepared/Callable when executing 
			// them with array of parameters?
			batchSupport = OdbcApi.SQLGetInfo(hDbc, OdbcDef.SQL_PARAM_ARRAY_ROW_COUNTS);

			if ( (batchSupport & OdbcDef.SQL_PARC_BATCH) > 0)		      
			{		 
			    prepareBatchSupport = true;

			    batchInPrepares = OdbcDef.SQL_PARC_BATCH;
			}

		}
		catch (SQLException e)
		{
		    // assume nothing is supported if getInfo 
		    // Exception occurred during checkBatchUpdateSupport.
		    batchInStatements = -1;
		    batchInProcedures = -1;
		    batchInPrepares = -1;

		    //handle exception for non-batch supported Drivers
		    //e.printStackTrace();
		}
		
	}


	public int getBatchRowCountFlag(int StmtType)
	{

	    switch (StmtType)
	    {
		case 1:
		    return batchInStatements;
		case 2:
		    return batchInPrepares;
		case 3:
		    return batchInProcedures;
	    }

	    return -1;

	}

	//--------------------------------------------------------------------
	// checkScrollCursorSupport
	// Check what result set types that data source can support. Assume
	// forward only type is alwasys supported.
	//--------------------------------------------------------------------

	public void checkScrollCursorSupport ()
		throws SQLException
	{
		short odbcCursor = -1;
		int cursorAttrs = 0;

		// Get supported cursor types from data source
		int scrollOpts = OdbcApi.SQLGetInfo (
			hDbc,
			OdbcDef.SQL_SCROLL_OPTIONS);

		// Initalize to -1 to indicate no support
		rsTypeFO = -1;
		rsTypeSI = -1;
		rsTypeSS = -1;

		// Get Odbc cursor for TYPE_FORWARD_ONLY
		if ((scrollOpts & OdbcDef.SQL_SO_FORWARD_ONLY) != 0)
			rsTypeFO = OdbcDef.SQL_CURSOR_FORWARD_ONLY;

		// Get Odbc cursor for TYPE_SCROLL_INSENSITIVE
		if ((scrollOpts & OdbcDef.SQL_SO_STATIC) != 0)
			rsTypeSI = OdbcDef.SQL_CURSOR_STATIC;

		// Get Odbc cursor for TYPE_SCROLL_INSENSITIVE. Check each scrollable
		// cursor and pick the one that support update visibility.
		if ((scrollOpts & OdbcDef.SQL_SO_STATIC) != 0) {
			odbcCursor = OdbcDef.SQL_CURSOR_STATIC;
			cursorAttrs = getOdbcCursorAttr2 (odbcCursor);
			if ((cursorAttrs & OdbcDef.SQL_CA2_SENSITIVITY_UPDATES)!= 0)
				rsTypeSS = odbcCursor;
		}
		if ((scrollOpts & OdbcDef.SQL_SO_KEYSET_DRIVEN) != 0 ||
		    (scrollOpts & OdbcDef.SQL_SO_MIXED) != 0) {
			odbcCursor = OdbcDef.SQL_CURSOR_KEYSET_DRIVEN;
			cursorAttrs = getOdbcCursorAttr2 (odbcCursor);
			if ((cursorAttrs & OdbcDef.SQL_CA2_SENSITIVITY_UPDATES) != 0)
				rsTypeSS = odbcCursor;
			else
				rsTypeSI = odbcCursor;
		}
		if ((scrollOpts & OdbcDef.SQL_SO_DYNAMIC) != 0) {
			odbcCursor = OdbcDef.SQL_CURSOR_DYNAMIC;
			cursorAttrs = getOdbcCursorAttr2 (odbcCursor);
			if ((cursorAttrs & OdbcDef.SQL_CA2_SENSITIVITY_UPDATES) != 0)
				rsTypeSS = odbcCursor;
		}

		rsTypeBest = odbcCursor;

		if (rsTypeBest == -1)
			rsTypeBest = rsTypeSS;
		if (rsTypeBest == -1)
			rsTypeBest = rsTypeSI;
		if (rsTypeBest == -1)
			rsTypeBest = rsTypeFO;
	}

	//--------------------------------------------------------------------
	// getBestOdbcCursorType
	// Return the Best ODBC cusror type supported by the driver.
	//--------------------------------------------------------------------

	public short getBestOdbcCursorType ()
	{
		return rsTypeBest;
	}


	//--------------------------------------------------------------------
	// getOdbcCursorType
	// Return the corresponding ODBC cusror type for the given
	// JDBC result set type
	//--------------------------------------------------------------------

	public short getOdbcCursorType (
		int resultSetType)
	{
		short cursor = -1;

		switch (resultSetType) {
		    case ResultSet.TYPE_FORWARD_ONLY:
			    cursor = rsTypeFO;
			    break;
		    case ResultSet.TYPE_SCROLL_INSENSITIVE:
			    cursor = rsTypeSI;
			    break;
		    case ResultSet.TYPE_SCROLL_SENSITIVE:
			    cursor = rsTypeSS;
			    break;
		}

		return cursor;
	}

	//--------------------------------------------------------------------
	// getOdbcConcurrency
	// Return the corresponding ODBC concurrency for the given
	// JDBC result set concurrency
	//--------------------------------------------------------------------

	public short getOdbcConcurrency (
		int resultSetConcurrency)
	{
		switch (resultSetConcurrency) {
		case ResultSet.CONCUR_READ_ONLY:
			return OdbcDef.SQL_CONCUR_READ_ONLY;
		case ResultSet.CONCUR_UPDATABLE:
			return OdbcDef.SQL_CONCUR_LOCK;
		}
		return OdbcDef.SQL_CONCUR_READ_ONLY;
	}

	//--------------------------------------------------------------------
	// getOdbcCursorAttr2
	// Given an ODBC cursor CCC, call SQLGetInfo with
	// SQL_CCC_CURSOR_ATTRIBUTES2
	//--------------------------------------------------------------------

	public int getOdbcCursorAttr2 (
		short odbcCursor)
		throws SQLException
	{
		short attrName = 0;

		switch (odbcCursor) {
		case OdbcDef.SQL_CURSOR_FORWARD_ONLY:
			attrName = OdbcDef.SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2;
			break;
		case OdbcDef.SQL_CURSOR_STATIC:
			attrName = OdbcDef.SQL_STATIC_CURSOR_ATTRIBUTES2;
			break;
		case OdbcDef.SQL_CURSOR_KEYSET_DRIVEN:
			attrName = OdbcDef.SQL_KEYSET_CURSOR_ATTRIBUTES2;
			break;
		case OdbcDef.SQL_CURSOR_DYNAMIC:
			attrName = OdbcDef.SQL_DYNAMIC_CURSOR_ATTRIBUTES2;
			break;
		}
		
		try
		{
		    return OdbcApi.SQLGetInfo (hDbc, attrName);
		}
		catch (SQLException e)
		{
		    //e.printStackTrace();
		    return 0;
		}

	}

	public Map<String,Class<?>> getTypeMap()
               throws SQLException
	{
		throw new UnsupportedOperationException();
	}

	public void setTypeMap(
		Map<String,Class<?>> map)
                throws SQLException
	{
		throw new UnsupportedOperationException();
	}

	//--------------------------------------------------------------------
	// buildTypeInfo
	// Builds the typeInfo Hashtable.  This table will be keyed by
	// the SQL type, and contain information about each type
	//--------------------------------------------------------------------

	protected void buildTypeInfo ()
		throws SQLException
	{
		typeInfo = new Hashtable ();

		// Get a result set containing all of the type information

		if (tracer.isTracing ()) {
			tracer.trace ("Caching SQL type information");
		}

		ResultSet rs = (getMetaData ()).getTypeInfo ();

		// Information for a single SQL type

		JdbcOdbcTypeInfo info;
		int sqlType;

		// Loop on the result set until we reach end of file

		boolean more = rs.next ();
		String name;
		int prec;

		while (more) {
			name = rs.getString (1);
			sqlType = rs.getInt (2);

			// Now that we have the type info, save it
			// in the Hashtable if we don't already have an
			// entry for this SQL type.

			if (typeInfo.get (new Integer (sqlType)) == null) {
				info = new JdbcOdbcTypeInfo ();
				info.setName (name);
				info.setPrec (rs.getInt (3));

				typeInfo.put (new Integer (sqlType), info);
			}
			more = rs.next ();
		}

		// Close the result set/statement.

		rs.close ();
	}

	//--------------------------------------------------------------------
	// registerStatement
	// Keep a reference of the given Statement object
	//--------------------------------------------------------------------

	protected void registerStatement (
		Statement stmt)
	{
		if (tracer.isTracing ()) {
			tracer.trace ("Registering Statement " + stmt);
		}

		// We use a Hashtable to keep the list.  The Statement object
		// is the key.  There is no need to store a meaningful value.

		statements.put (stmt, "");
	}

	//--------------------------------------------------------------------
	// deregisterStatement
	// Each Statement object has a reference to it in the owning Connection
	// object.  This method will remove that reference
	//--------------------------------------------------------------------

	public void deregisterStatement (
		Statement stmt)
	{
		if (statements.get (stmt) != null) {
			if (tracer.isTracing ()) {
				tracer.trace ("deregistering Statement " + stmt);
			}
			statements.remove (stmt);
		}
	}

	//--------------------------------------------------------------------
	// closeAllStatements
	// Close any statements that are still referenced.
	//--------------------------------------------------------------------

	public synchronized void closeAllStatements ()
		throws SQLException
	{
		if (tracer.isTracing ()) {
			tracer.trace ("" + statements.size () + " Statement(s) to close");
		}
	        
		if (statements.size () == 0) {
			return;
		}
	
		//4524683
		java.util.Set stmtSet = statements.keySet ();
		java.util.Iterator stmtSetIterator= stmtSet.iterator();
		
		Statement stmt;

		// Loop while there are still Statements references
		
		//4524683
		while (stmtSetIterator.hasNext ()) {

			// Get the statement reference

			try {
				stmt = (Statement) stmtSetIterator.next ();
					
				// Close the Statement.  This will also call our
				// deregisterStatement method to remove it from the statement
				// list
				
				stmt.close ();
			} catch(Exception e) {
				stmtSet = statements.keySet ();
				stmtSetIterator= stmtSet.iterator();
			}

		}
		
	
		//garbage collect any Batch Vector reference.
		batchStatements = null;
	}


	//--------------------------------------------------------------------
	// setBatchVector
	// store a copy of Batch SQL with reference to the Statement object
	//--------------------------------------------------------------------
	public synchronized void setBatchVector(Vector batchVector, Statement stmt)
	{
		int islot = -1;
	
		if (tracer.isTracing ()) {
			tracer.trace ("setBatchVector " + stmt);
		}

		// We use a Hashtable to keep the list.	 The Statement object
		// is the key.	There is no need to store a meaningful value.
		
		batchStatements.put (stmt, batchVector);
	
	}

	//--------------------------------------------------------------------
	// getBatchVector
	// return the Batch SQL of the referenced Statement object
	//--------------------------------------------------------------------
	public Vector getBatchVector(Statement stmt)
	{
		if (tracer.isTracing ()) 
		{
			tracer.trace ("getBatchVector " + stmt);
		}
					
		return (Vector)batchStatements.get (stmt);
		
	}

	//--------------------------------------------------------------------
	// removeBatchVector
	// destroy Batch SQL for the given referenced statement Object.
	//--------------------------------------------------------------------
	public synchronized void removeBatchVector(Statement stmt)
	{
		if (tracer.isTracing ()) 
		{
			tracer.trace ("removeBatchVector " + stmt);
		}
					
		batchStatements.remove (stmt);
	
	}

	//--------------------------------------------------------------------
	// setResultSetBlockSize
	// sets the connecition's odbcRowSetSize property value.
	// if this property is not set, the default is used.
	//--------------------------------------------------------------------

	protected void setResultSetBlockSize(String rowSetSize)
		throws SQLException
	{
		rsBlockSize = JdbcOdbcLimits.DEFAULT_ROW_SET;

		if ( rowSetSize != null )
		{
		    rowSetSize.trim();
		
		    if (!rowSetSize.equals(""))
		    {
			try
			{
			    int blockSize = (new Integer(rowSetSize)).intValue();

			    if (blockSize > 0)
				rsBlockSize = blockSize;
			}
			catch (NumberFormatException nfe) 
			{
			    throw new SQLException ("invalid property value: [odbcRowSetSize=" + rowSetSize + "]"); 
			}
		    }

		}		
	}

    //-------------------------------------------------------------------
    // JDBC 3.0 API Changes
    //-------------------------------------------------------------------

 
    public void setHoldability(int holdability) throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public int getHoldability() throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public Savepoint setSavepoint() throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public Savepoint setSavepoint(String name) throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public void rollback(Savepoint savepoint) throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public Statement createStatement(int resultSetType, int resultSetConcurrency, 
			      int resultSetHoldability) throws SQLException {
	throw new UnsupportedOperationException();
    }


    public PreparedStatement prepareStatement(String sql, int resultSetType, 
				       int resultSetConcurrency, int resultSetHoldability)
	throws SQLException {
	throw new UnsupportedOperationException();
    }

    public void releaseSavepoint(Savepoint savepoint) throws SQLException {
	throw new UnsupportedOperationException();
    }



    public CallableStatement prepareCall(String sql, int resultSetType, 
				  int resultSetConcurrency, 
				  int resultSetHoldability) throws SQLException {
	throw new UnsupportedOperationException();
    }



    public PreparedStatement prepareStatement(String sql, int flag)
	throws SQLException {
	throw new UnsupportedOperationException();
    }

 
    public PreparedStatement prepareStatement(String sql, int columnIndexes[])
	throws SQLException {
	throw new UnsupportedOperationException();
    }

   
    public PreparedStatement prepareStatement(String sql, String columnNames[])
       throws SQLException {
	throw new UnsupportedOperationException();
    }



	//====================================================================
	// Data attributes
	//====================================================================

	protected JdbcOdbc OdbcApi;		// ODBC API interface object
	
	protected JdbcOdbcDriverInterface myDriver;
						// Pointer to the owning
						//  driver object

	protected long	hEnv;			// Environment handle

	protected long	hDbc;			// Database connection handle

	protected SQLWarning	lastWarning;	// Last SQLWarning generated by
						//  an operation

	protected boolean	closed;			// Flag indicating whether the
						//  connection has been closed.

	protected String URL;			// URL of connection

	protected int odbcVer;			// ODBC version number (i.e. 2)

	protected Hashtable typeInfo;		// Hashtable containing an entry
						//  for each row returned by
						//  DatabaseMetaData.getTypeInfo.

	// 4524683.
	public java.util.WeakHashMap statements;// Hashtable containing a list
						//  of all the Statement objects
						//  for this Connection

	protected Hashtable batchStatements;	// Hashtable containing Vectors
						//  of all batchStatement objects
						//  for this Connection

	protected short rsTypeFO;		// The corresponding ODBC cursor
						//  for TYPE_FORWARD_ONLY
	protected short rsTypeSI;		// For TYPE_SCROLL_SENSITIVE
	protected short rsTypeSS;		// For TYPE_SCROLL_SENSITIVE
	protected short rsTypeBest;		// The best scrollable cursor
						// supported by ODBC driver

	protected int rsBlockSize;		// prefered property value for
						// a scrollable block-cursor.

	protected int batchInStatements;
	protected int batchInProcedures;
	protected int batchInPrepares;
	private boolean freeStmtsFromConnectionOnly; // 4524683
	
	// RFE 4641013.
	protected JdbcOdbcTracer tracer = new JdbcOdbcTracer();	// Keeps all tracing  
						// through this object 

}

