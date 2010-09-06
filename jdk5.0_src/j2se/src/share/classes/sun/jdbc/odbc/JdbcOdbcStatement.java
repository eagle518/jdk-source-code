/*
 * @(#)JdbcOdbcStatement.java	1.36 00/12/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcStatement.java
//
// Description: Impementation of the Statement interface class
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

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Vector;
import java.math.BigDecimal;
import java.sql.*;

public class JdbcOdbcStatement
	extends		JdbcOdbcObject
	implements	java.sql.Statement {

	//====================================================================
	// Public methods
	//====================================================================

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcStatement (
		JdbcOdbcConnectionInterface con)
	{
		OdbcApi = null;
		hDbc    = OdbcDef.SQL_NULL_HDBC;
		hStmt   = OdbcDef.SQL_NULL_HSTMT;
		lastWarning = null;
		myConnection = con;
		rsType = ResultSet.TYPE_FORWARD_ONLY;
		rsConcurrency = ResultSet.CONCUR_READ_ONLY;
		fetchDirection = ResultSet.FETCH_FORWARD;
		fetchSize = 1;

		batchRCFlag = -1;
		batchSupport = false;

		moreResults = OdbcDef.RESULTS_NOT_SET;
	}

	//--------------------------------------------------------------------
	// finalize
	// Perform any cleanup when this object is garbage collected
	//--------------------------------------------------------------------

	protected void finalize ()
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("Statement.finalize " + this);
		}

		try {
			closeCalledFromFinalize = true; //4524683
			close ();
		}
		catch (SQLException ex) {
			// If an exception is thrown, ignore it
		}
	}

	//--------------------------------------------------------------------
	// initialize
	// Initialize the Statement object.  Give the ODBC API interface
	// object, the connection handle, and optionally the statement
	// handle.  If no statement handle is given, one is created.
	//--------------------------------------------------------------------

	public void initialize (
		JdbcOdbc odbcApi,
		long hdbc,
		long hstmt,
		Hashtable info,
		int resultSetType,
		int resultSetConcurrency)
		throws SQLException
	{
		OdbcApi = odbcApi;
		hDbc = hdbc;
		hStmt = hstmt;
		rsType = resultSetType;
		rsConcurrency = resultSetConcurrency;
		typeInfo = info;

		// If BATCH_EXPLICIT supported,
		// we need to know if Row Counts
		// will be returned for individual statements.
		// If no Row counts returned for each statement, 
		// Batch Update is not supported and must emulate.
		batchRCFlag = myConnection.getBatchRowCountFlag(1);

		// double check if row counts are returned.
		if ( (batchRCFlag > 0) && (batchRCFlag == OdbcDef.SQL_BRC_EXPLICIT) )
		{
		    batchSupport = true;
		}
		else batchSupport = false;

		// Check if cursor and concurrency type are valid ResultSet Types
		
		if  ((rsType == ResultSet.TYPE_FORWARD_ONLY) ||
		     (rsType == ResultSet.TYPE_SCROLL_INSENSITIVE) ||
		     (rsType == ResultSet.TYPE_SCROLL_SENSITIVE))
		{
		    if  ((rsConcurrency == ResultSet.CONCUR_READ_ONLY) ||
			 (rsConcurrency == ResultSet.CONCUR_UPDATABLE)  )
		    {
			// do Nothing;
		    }
		    else
		    {
			    close();
			    throw new SQLException ("Invalid Concurrency Type.");
		    }
		}
		else
		{
			close();
			throw new SQLException ("Invalid Cursor Type.");
		}


		// Check if driver support the requested result set type.
		// If not, find an alternative cursor

		short odbcCursorType = myConnection.getOdbcCursorType (rsType);

		if (odbcCursorType == -1) {
			
			odbcCursorType = myConnection.getBestOdbcCursorType();

			if (odbcCursorType == -1) {
				throw new SQLException (
				    "The result set type is not supported.");
			}
			else				
			    setWarning (new SQLWarning (
				    "The result set type has been downgraded and changed."));

			// if the cursor is downgraded, 
			// reset the cursor Type to the best 
			// cursor supported. This temporary 
			// cursor will remain in effect until
			// statement closes.

			switch (odbcCursorType) 
			{
			    case OdbcDef.SQL_CURSOR_FORWARD_ONLY:
				    rsType = ResultSet.TYPE_FORWARD_ONLY;
				    break;
			    case OdbcDef.SQL_CURSOR_STATIC:
			    case OdbcDef.SQL_CURSOR_KEYSET_DRIVEN:
				    rsType = ResultSet.TYPE_SCROLL_INSENSITIVE;
				    break;
			}
						
		}

		// 4486195 -> most widely available concurrency in all
		// drivers
		if (rsConcurrency == ResultSet.CONCUR_UPDATABLE) {
		    odbcCursorType = OdbcDef.SQL_CURSOR_DYNAMIC;
		}

		// Set ODBC cursor type

		try {	
			OdbcApi.SQLSetStmtOption (hStmt, 
					OdbcDef.SQL_ATTR_CURSOR_TYPE,
					odbcCursorType);
		}
		catch (SQLWarning ex) {
			setWarning (ex);
		}
		//fix for Sun Bug id 4320025
		catch (SQLException se)
		{
			if (odbcCursorType != OdbcDef.SQL_CURSOR_FORWARD_ONLY)
			{
				se.fillInStackTrace();
				throw se;
			}
		}


		// Set ODBC concurrency

		short odbcConcurrency = 
			myConnection.getOdbcConcurrency (rsConcurrency);

		try {
			OdbcApi.SQLSetStmtOption (hStmt,
					OdbcDef.SQL_CONCURRENCY,
					odbcConcurrency);
		}
		catch (SQLWarning ex) {
			setWarning (ex);
		}
		//fix for Sun Bug id 4320025
		catch (SQLException se)
		{
			if (odbcConcurrency != OdbcDef.SQL_CONCUR_READ_ONLY)
			{
				se.fillInStackTrace();
				throw se;
			}
		}


	}


	//--------------------------------------------------------------------
	// executeQuery
	// executeQuery method executes the given SQL string which must
	// be a query that returns a single ResultSet.
	// (See also the more general "execute" below.)
	//--------------------------------------------------------------------

	public ResultSet executeQuery (
		String sql)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.executeQuery (" + sql + ")");
		}
		ResultSet rs = null;

		// Execute the statement.  If execute returns true, a result
		// set exists.

		if (execute (sql)) {			
			rs = getResultSet (false);			
		}
		else {
			// No ResultSet was produced.  Raise an exception

			throw new SQLException ("No ResultSet was produced");
		}

		if (batchOn)
		    clearBatch();

		return rs;
	}

	//--------------------------------------------------------------------
	// executeUpdate
	// This method execute a SQL statement that is known to be a simple 
	// database update (such as a SQL UPDATE, INSERT, DELETE, ...)
	// The result is the number of rows that have been modified. 
	// (See also the more general "execute" below.)
	//--------------------------------------------------------------------

	public int executeUpdate (
		String sql)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.executeUpdate (" + sql + ")");
		}
		int numRows = -1;

		// Execute the statement.  If execute returns false, a
		// row count exists.

		if (!execute (sql)) {
			numRows = getUpdateCount ();
		}
		else {

			// No update count was produced (a ResultSet was).  Raise
			// an exception

			throw new SQLException ("No row count was produced");
		}

		if (batchOn)
		    clearBatch();

		return numRows;
	}

	//--------------------------------------------------------------------
	// execute
	// The "execute" method execute a SQL statement and returns "true"
	// if the first result is a ResultSet, false otherwise.  You can
	// then use getResultSet or getUpdateCount to retrieve the result,
	// and getMoreResults to move to any subseuqnet result(s).
	//--------------------------------------------------------------------

	public synchronized boolean execute (
		String sql)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.execute (" + sql + ")");
		}
		boolean hasResultSet = false;
		SQLWarning	warning = null;

		// save the SQL for this statement
		setSql(sql);

		// Reset the statement handle and warning

		reset ();

		// Check for a 'FOR UPDATE' statement.  If present, change
		// the concurrency to lock

		lockIfNecessary (sql);

		// Call SQLExecDirect

		try {
			OdbcApi.SQLExecDirect (hStmt, sql);
		}
		catch (SQLWarning ex) {

			// Save pointer to warning and save with ResultSet
			// object once it is created.

			warning = ex;
		}
				
		// Now determine if there is a result set associated with
		// the SQL statement that was executed.  Get the column
		// count, and if it is not zero, there is a result set.

		if (getColumnCount () > 0) {
			hasResultSet = true;
		}

		if (batchOn)
		    clearBatch();

		return hasResultSet;
	}

	//--------------------------------------------------------------------
	// getResultSet
	// getResultSet returns the current result as a ResultSet.  It 
	// returns NULL if the current result is not a ResultSet.
	//--------------------------------------------------------------------
	public ResultSet getResultSet ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getResultSet");
		}
		//fix for bugid 4251419
		if (myResultSet != null)
			return myResultSet;
		myResultSet = getResultSet (true);

		return myResultSet;
	}

	//--------------------------------------------------------------------
	// getResultSet
	// getResultSet returns the current result as a ResultSet.  It 
	// returns NULL if the current result is not a ResultSet.
	//--------------------------------------------------------------------
	
	//public ResultSet getResultSet (
	public synchronized ResultSet getResultSet (
		boolean checkCount)
		throws SQLException
	{
		//fix for bugid 4251419
		if (myResultSet != null) {
		//if ((myResultSet != null) && (checkCount == true)) {
			// if resultset already retrieved,
			// throw exception to avoid sequence error
			
			throw new SQLException ("Invalid state for getResultSet");

		}

		JdbcOdbcResultSet rs = null;
		int numCols = 1;

		// If we already know we have result columns, checkCount
		// is false.  This is an optimization to prevent unneeded
		// calls to getColumnCount

		if (checkCount) {
			numCols = getColumnCount ();
		}

		// Only return a result set if there are result columns

		if (numCols > 0) {

			// Check if ODBC driver downgraded the cursor type for
			// this query

			if (rsType != ResultSet.TYPE_FORWARD_ONLY)
				checkCursorDowngrade ();

			rs = new JdbcOdbcResultSet ();
			rs.initialize (OdbcApi, hDbc, hStmt, true, this);

			// Save a copy of our last result set			
			//fix for bugid 4251419
			myResultSet = rs;
		}
		else {
			clearMyResultSet ();
		}
		return rs;
	}

	//--------------------------------------------------------------------
	// getUpdateCount
	// getUpdateCount returns the current result, which should be an update
	// count.   It returns -1 if there are no more results or if the
	// current result is a ResultSet.
	//--------------------------------------------------------------------
	public int getUpdateCount ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getUpdateCount");
		}
		int rowCount = -1;
		

		if (moreResults == OdbcDef.NO_MORE_RESULTS)
			return rowCount;

		// Only return a row count for SQL statements that did not
		// return a result set.

		if (getColumnCount () == 0) 
			rowCount = getRowCount ();
		
		return rowCount;
	}

	//--------------------------------------------------------------------
	// close
	// close frees up internal state associated with the statement.
	// You should normally call close when you are done with a statement.
	// N.B. any ResultSets associated with the Statement will become 
	// unusable when the Statement is closed.
	//--------------------------------------------------------------------

	public synchronized void close ()
		throws SQLException
	{
	
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.close");
		}
	

		// Close/clear our result set

		clearMyResultSet ();

		// Reset last warning message

		try {
			clearWarnings ();
			if (hStmt != OdbcDef.SQL_NULL_HSTMT) {
				//4524683
				if(closeCalledFromFinalize == true) {
					if( myConnection.isFreeStmtsFromConnectionOnly() == false) {
						OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);
					}
				}
				else {
					OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_DROP);	
				}
				hStmt = OdbcDef.SQL_NULL_HSTMT;
			}
		}
		catch (SQLException ex) {
			ex.printStackTrace();
			// If we get an error, ignore
		}

		// Remove this Statement object from the Connection object's
		// list

		myConnection.deregisterStatement (this);
	}

	//--------------------------------------------------------------------
	// reset
	// Frees up internal state associated with the statement before re-using
	// the statement
	//--------------------------------------------------------------------

	protected void reset ()
		throws SQLException
	{
		clearWarnings ();
		
		clearMyResultSet ();
		if (hStmt != OdbcDef.SQL_NULL_HSTMT) 
		{
			OdbcApi.SQLFreeStmt (hStmt, OdbcDef.SQL_CLOSE);			    
		}
				
	}


	//--------------------------------------------------------------------
	// getMoreResults
	// getMoreResults moves to the next result.  It returns true if 
	// this result is a ResultSet.  getMoreResults also implicitly
	// closes the current ResultSet.
	//--------------------------------------------------------------------

	public boolean getMoreResults ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getMoreResults");
		}
		//JdbcOdbcResultSet rs = null;
		SQLWarning	warning = null;
		boolean		hasResultSet = false;

		if (moreResults == OdbcDef.RESULTS_NOT_SET)
		    moreResults  = OdbcDef.NO_MORE_RESULTS;
		
		// clear previous warnings

		clearWarnings ();

		// Call SQLMoreResults

		try {
			if (OdbcApi.SQLMoreResults (hStmt))
				moreResults = OdbcDef.HAS_MORE_RESULTS;
			else
				moreResults = OdbcDef.NO_MORE_RESULTS;
		}
		catch (SQLWarning ex) {

			// Save pointer to warning and save with ResultSet
			// object once it is created.

			warning = ex;
		}
				
		// There are more results (it may not be a result set, though)

		if (moreResults == OdbcDef.HAS_MORE_RESULTS) {

			// Now determine if there is a result set associated
			// with the SQL statement that was executed.  Get the
			// column count, and if it is zero, there is not a
			// result set.

			if (getColumnCount () != 0) {
				hasResultSet = true;
			}
		}

		// Set the warning for the statement, if one was generated

		setWarning (warning);

		// Return the result set indicator

		return hasResultSet;
	}


	//--------------------------------------------------------------------
	// getMaxFieldSize
	// The maxFieldSize is a limit (in bytes) on how much data can be 
	// returned as part of a field.  If the limit is exceeded, the data
	// is truncated and a warning is added (see Statement.getWarnings).
	// You should normally not need to change the default limit.
	// Zero means no limit.
	//--------------------------------------------------------------------

	public int getMaxFieldSize ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getMaxFieldSize");
		}
		return getStmtOption (OdbcDef.SQL_MAX_LENGTH);
	}

	//--------------------------------------------------------------------
	// setMaxFieldSize
	//--------------------------------------------------------------------

	public void setMaxFieldSize (
		int max)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.setMaxFieldSize (" + max + ")");
		}
		OdbcApi.SQLSetStmtOption (hStmt, OdbcDef.SQL_MAX_LENGTH, max);
	}


	//--------------------------------------------------------------------
	// getMaxRows
	// The maxRows value is a limit on how many rows can be returned as
	// part of a ResutlSet.  You should normally not need to change
	// the default limit.  Zero means no limit.
	//--------------------------------------------------------------------

	public int getMaxRows ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getMaxRows");
		}
		return getStmtOption (OdbcDef.SQL_MAX_ROWS);
	}

	//--------------------------------------------------------------------
	// setMaxRows
	//--------------------------------------------------------------------

	public void setMaxRows (
		int max)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.setMaxRows (" + max + ")");
		}
		if (max < 0) { // bug 4495457
                        throw new SQLException("Invalid new max row limit");
                }
		
		OdbcApi.SQLSetStmtOption (hStmt, OdbcDef.SQL_MAX_ROWS, max);
	}


	//--------------------------------------------------------------------
	// setEscapeProcessing
	// if escape scanning is on (the default) the driver will do escape
	// substitution before sending the SQL to the database.
	//--------------------------------------------------------------------

	public void setEscapeProcessing (
		boolean enable)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.setEscapeProcessing (" +
				enable + ")");
		}
		int	value = OdbcDef.SQL_NOSCAN_OFF;
		if (!enable) {
			value = OdbcDef.SQL_NOSCAN_ON;
		}
		OdbcApi.SQLSetStmtOption (hStmt, OdbcDef.SQL_NOSCAN, value);
	}

	//--------------------------------------------------------------------
	// getQueryTimeout
	// The queryTimeout is how many seconds the driver will allow for an
	// SQL statement to execute before giving up.  Zero means no limit.
	//--------------------------------------------------------------------

	public int getQueryTimeout ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getQueryTimeout");
		}
		return getStmtOption (OdbcDef.SQL_QUERY_TIMEOUT);
	}

	//--------------------------------------------------------------------
	// setQueryTimeout
	//--------------------------------------------------------------------

	public void setQueryTimeout (
		int seconds)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.setQueryTimeout (" +
				seconds + ")");
		}
		OdbcApi.SQLSetStmtOption (hStmt, OdbcDef.SQL_QUERY_TIMEOUT,
				seconds);
	}


	//--------------------------------------------------------------------
	// cancel
	// Cancel can be used by one thread to cancel a statement that
	// is being executed by another thread.
	//--------------------------------------------------------------------

	public void cancel ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.cancel");
		}

		// Clear any warnings
		clearWarnings ();

		// Call SQLCancel

		try {
			OdbcApi.SQLCancel (hStmt);
		}
		catch (SQLWarning ex) {

			// Save last warning

			setWarning (ex);
		}
	}

	//--------------------------------------------------------------------
	// getWarnings will return any warning information related to
	// the current statement execution.  Note that SQLWarning may be
	// a chain.  If a statement is re-executed then warnings associated
	// with its previous execution will be automatically discarded.
	//--------------------------------------------------------------------

	public SQLWarning getWarnings ()
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.getWarnings");
		}
		return lastWarning;
	}

	public void clearWarnings ()
		throws SQLException
	{
		lastWarning = null;
	}

	//--------------------------------------------------------------------
	// setWarning
	// Sets the warning 
	//--------------------------------------------------------------------

	public void setWarning (SQLWarning ex)
	{
		lastWarning = ex;
	}

	//--------------------------------------------------------------------
	// setCursorName
	// Defines the SQL cursor name that will be used by
	// subsequent Statement execute methods.
	//--------------------------------------------------------------------

	public void setCursorName (
		String name)
		throws SQLException
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("*Statement.setCursorName " + name + ")");
		}
		OdbcApi.SQLSetCursorName (hStmt, name);
	}


	// New JDBC 2.0 API

	public void setFetchDirection (
		int direction)
		throws SQLException
	{
		// bug 4495457
                if ((direction == ResultSet.FETCH_FORWARD) ||
                    (direction == ResultSet.FETCH_REVERSE) ||
                    (direction == ResultSet.FETCH_UNKNOWN)) {
                        fetchDirection = direction;
                }
                else {
                        throw new SQLException("Invalid fetch direction");
                }
	}

	public int getFetchDirection()
		throws SQLException
	{
		return fetchDirection;
	}

	public void setFetchSize (
		int rows)
		throws SQLException
	{
		if ((0 <= rows) && (rows <= this.getMaxRows())) {
                       fetchSize = rows;
                }
                else { // bug 4495457
                       throw new SQLException("Invalid Fetch Size");
                }
	}

	public int getFetchSize()
		throws SQLException
	{
		return fetchSize;
	}

	public int getResultSetConcurrency()
		throws SQLException
	{
		return rsConcurrency;
	}

	public int getResultSetType()
		throws SQLException
	{
		return rsType;
	}

	public void addBatch (String sql) 
		throws SQLException
	{
		     
		if (OdbcApi.getTracer().isTracing ()){
			OdbcApi.getTracer().trace ("*Statement.addBatch (" + sql + ")");
		}

		try
		{
										
		    if (sql != null)
		    {		
			//get from storage and add to it.
			batchSqlVec = myConnection.getBatchVector(this);
		    
			if ( batchSqlVec == null )
			{
			    batchSqlVec = new Vector(5,10);
			}	     
							      
			batchSqlVec.addElement(sql);			       
				   
		        myConnection.setBatchVector(batchSqlVec, this);

			batchOn = true;
		    }

		}
		catch (Exception e)
		{
			e.printStackTrace();
		}	 
											    
	
	}

	public void clearBatch () 
	{
		if (OdbcApi.getTracer().isTracing ())
		{
			OdbcApi.getTracer().trace ("*Statement.clearBatch");
		}

	    try
	    {		
		if ( batchSqlVec != null )
		{
		   //remove from storage and destroy it.			   
			myConnection.removeBatchVector(this);			   
			batchSqlVec = null;
			batchOn = false;
		}	     
	    }
	    catch (Exception e)
	    {
		e.printStackTrace();
	    }	 														    
			
	}

	public int[] executeBatch () 
		throws BatchUpdateException
	{ 	
	    return executeBatchUpdate();
	}	

	protected int[] executeBatchUpdate()
	    throws BatchUpdateException
	{
	
	    int rowCounts[] = {}; // 4532169, 4639504
	    int rowCountsForException[] = null; // 4532169
	    int successCount = 0; // 4532169

		if (OdbcApi.getTracer().isTracing ())
		{
		    OdbcApi.getTracer().trace ("*Statement.executeBatch");
		}

	    if (!batchSupport) //Batch not Supported!
	    {
		return emulateBatchUpdate();
	    }
	    else
	    {

		batchSqlVec = (Vector) myConnection.getBatchVector(this);

		if (batchSqlVec != null)
		{

		    Enumeration enum_ = batchSqlVec.elements();

		    rowCounts = new int[batchSqlVec.size()];

		    int itemCount = rowCounts.length;

		    StringBuffer execBatchBuff = new StringBuffer();

		    // Loop while there are DDL or DML statements references
		    for (int i = 0 ; i < itemCount; i++)
		    {
			if (enum_.hasMoreElements ())
			{
			    String batchItem = (String) enum_.nextElement ();
			    /*
					// Some databases do not allow ";" delimiter
					// Syntax error on semicolon(";") character.
					if ( (batchItem.lastIndexOf(";") == batchItem.length()) ||
					     (i == (itemCount - 1)) )
					{			    
					    execBatchBuff.append(batchItem + "\n");			
					}
					else
					{			 
					    execBatchBuff.append(batchItem + ";\n");
					}
			    */				
			    execBatchBuff.append(batchItem + "\n");
			}
						
		    }//end for loop!			


		    try
		    {

		        if ( !execute(execBatchBuff.toString()) )
			{    
				// 4532169
				while(true)
				{
					int returnUpdates = getUpdateCount();
					
					if( returnUpdates == -1 )
					{
						break;
					}
					else
					{
						rowCounts[successCount++] = returnUpdates;
						getMoreResults();
					}
				}
				
				if( successCount < itemCount )
				{
					rowCountsForException = new int[successCount];
					for(int j = 0; j < successCount; j++)
					{
						rowCountsForException[j] = rowCounts[j];
					}
					
					clearBatch();
					throw new JdbcOdbcBatchUpdateException("SQL Attempt to produce a ResultSet from executeBatch", rowCountsForException);
				}
			}
			else
			{
			    clearBatch();
			    // 4532169
			    throw new JdbcOdbcBatchUpdateException("SQL Attempt to produce a ResultSet from executeBatch", null);
			}

		    }
		    catch (SQLException e) 
		    { 
			  clearBatch();
			  // 4532169
			  throw new JdbcOdbcBatchUpdateException(e.getMessage(), e.getSQLState(), rowCountsForException);
		    }//end try;

							
		}//end if!
		clearBatch ();


	    }//end Batch support!


	    return rowCounts;

	}

       //------------------------------------------------------
       // emulateBatchUpdate()
       // emulate batch Update if SQL batch is not supported for 
       // JdbcOdbcStatement.
       //------------------------------------------------------

	protected int[] emulateBatchUpdate()
	    throws BatchUpdateException
	{

	    int[] emulateBatchCount = {};
	    
		 										    
	    batchSqlVec = (Vector) myConnection.getBatchVector(this);
							
	    if (batchSqlVec != null)
	    {
		int[] exceptionCount = {};
		int successCount = 0;

		Enumeration enum_ = batchSqlVec.elements();
							
		emulateBatchCount = new int[batchSqlVec.size()];
								
		// Loop while there are DML statements references
		for (int i = 0 ; i < emulateBatchCount.length; i++) 
		{	 

		    if (enum_.hasMoreElements ())
		    {	

			String batchItem = (String) enum_.nextElement ();

			try
			{

			    if (!execute (batchItem))
			    {
				emulateBatchCount[i] = getUpdateCount ();
				successCount++;
			    }
			    else
			    {
			        // 4532169
			    	exceptionCount = new int[successCount];
				for (int j = 0; j <= i - 1; j++)
				{
				    exceptionCount[j] = emulateBatchCount[j];				    
				}					
				clearBatch();

				throw new JdbcOdbcBatchUpdateException("No row count was produced from executeBatch", exceptionCount);
			    }
			}
			catch(SQLException e)
			{
				// 4532169
				exceptionCount = new int[successCount];
				for (int j = 0; j <= i - 1; j++)
				{
				    exceptionCount[j] = emulateBatchCount[j];
				}
				clearBatch();

				throw new JdbcOdbcBatchUpdateException(e.getMessage(), e.getSQLState(), exceptionCount); 
			}


		    }
		}
		clearBatch();


	    }								 
												
	    return emulateBatchCount;

	}
	

	public Connection getConnection()
		throws SQLException
	{
		return myConnection;
	}

	public String getSql ()
	{
		return mySql;
	}

	//--------------------------------------------------------------------
	// setSql
	// sets the current SQL. Used to build a row count query if needed.
	//--------------------------------------------------------------------

	public void setSql (String sql)
	{
		mySql = sql.toUpperCase();
	}

	//--------------------------------------------------------------------
	// getObjects
	// dummy interface for PreparedStatements w/ scrollable ResultSet.
	//--------------------------------------------------------------------

	public Object[] getObjects()
	{
	    Object[] objs = {};

	    return objs;
	}

	//--------------------------------------------------------------------
	// getObjectTypes
	// dummy interface for PreparedStatements w/ scrollable ResultSet.
	//--------------------------------------------------------------------

	public int[] getObjectTypes()
	{
	    int[] types = {};

	    return types;
	}

	//--------------------------------------------------------------------
	// getParamCount
	// dummy interface for PreparedStatements w/ scrollable ResultSet.
	//--------------------------------------------------------------------

	public int getParamCount()
	{
	    return 0;
	}

	//--------------------------------------------------------------------
	// getBlockCursorSize
	// Return the prefered rowSet size for a scrollable block-curser.
	//--------------------------------------------------------------------

	public int getBlockCursorSize ()
	{
		return rsBlockSize;
	}


	//--------------------------------------------------------------------
	// setBlockCursorSize
	// Return the prefered rowSet size for a scrollable block-curser.
	//--------------------------------------------------------------------

	public void setBlockCursorSize (int rowSetSize)
	{
		rsBlockSize = rowSetSize;
	}

	//====================================================================
	// Protected methods
	//====================================================================

	//--------------------------------------------------------------------
	// getStmtOption
	// Invoke SQLGetStmtOption with the given option.
	//--------------------------------------------------------------------

	protected int getStmtOption (short fOption)
		throws SQLException
	{
		int	result = 0;

		// Reset last warning message

		clearWarnings ();

		try {
			result = (int)OdbcApi.SQLGetStmtOption (hStmt, fOption);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			result = n.intValue ();

			setWarning (JdbcOdbc.convertWarning (ex));
		}
		return result;
	}

	//--------------------------------------------------------------------
	// getColumnCount
	// Return the number of columns in the ResultSet
	//--------------------------------------------------------------------

	protected int getColumnCount ()
		throws SQLException
	{
		int	numCols = 0;

		try {
			numCols = OdbcApi.SQLNumResultCols (hStmt);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			numCols = n.intValue ();
		}
		return numCols;
	}

	//--------------------------------------------------------------------
	// getRowCount
	// The total number of rows returned by the query.  Note that on some
	// databases this method may be very expensive.
	//--------------------------------------------------------------------

	protected int getRowCount ()
		throws SQLException
	{
		int	numRows = 0;

		try {
			numRows = OdbcApi.SQLRowCount (hStmt);
		}
		catch (JdbcOdbcSQLWarning ex) {

			// If we got a warning, the return value was saved off
			// in the warning object.  Get it back and set the
			// return value.

			BigDecimal n = (BigDecimal) ex.value;
			numRows = n.intValue ();
		}
		return numRows;
	}


	//--------------------------------------------------------------------
	// lockIfNecessary
	// If the given SQL statement contains a 'FOR UPDATE' clause, change
	// the concurrency to lock so that the row can then be updated.  Returns
	// true if the concurrency has been changed
	//--------------------------------------------------------------------

	protected boolean lockIfNecessary (
		String sql)
		throws SQLException
	{
		boolean	rc = false;

		// First, convert the statement to upper case
		
		String sqlStatement = sql.toUpperCase ();

		// Now, look for the FOR UPDATE keywords.  If there is any extra white
		// space between the FOR and UPDATE, this will fail.

		int index = sqlStatement.indexOf (" FOR UPDATE");

		// We found it.  Change our concurrency level to ensure that the
		// row can be updated.

		if (index > 0) {
			if (OdbcApi.getTracer().isTracing ()) {
				OdbcApi.getTracer().trace ("Setting concurrency for update");
			}

			try {
				OdbcApi.SQLSetStmtOption (hStmt, OdbcDef.SQL_CONCURRENCY,
									OdbcDef.SQL_CONCUR_LOCK);
			}
			catch (SQLWarning warn) {

				// Catch any warnings and place on the warning stack
				setWarning (warn);
			}
			rc = true;
		}

		return rc;
	}

	//--------------------------------------------------------------------
	// getPrecision
	// Given a SQL type, return the maximum precision for the column.
	// Returns -1 if not known
	//--------------------------------------------------------------------

	protected int getPrecision (	
		int sqlType)
	{
		int prec = -1;
		JdbcOdbcTypeInfo info;

		if (typeInfo != null) {
			info = (JdbcOdbcTypeInfo) typeInfo.get (
						new Integer (sqlType));

			if (info != null) {
				prec = info.getPrec ();
			}
		}
		// 4532171
		if(sqlType == Types.BINARY && prec == -1) {
			prec = getPrecision(Types.VARBINARY);
		}
		return prec;
	}

	//--------------------------------------------------------------------
	// clearMyResultSet
	// If a ResultSet was created for this Statement, close it
	//--------------------------------------------------------------------

	protected synchronized void clearMyResultSet ()
		throws SQLException
	{
		if (myResultSet != null)
		{		    

		    if (hStmt != OdbcDef.SQL_NULL_HSTMT) 
		    {
				myResultSet.close ();
		    }
		    myResultSet = null;
		}
	}
                               
	//--------------------------------------------------------------------
	// checkCursorDowngrade
	// Call SQLGetInfo to get current ODBC cursor type. If it is not what
	// we expect, it means ODBC driver downgraded it for the current query.
	// Adjust JDBC result type accordingly.
	//--------------------------------------------------------------------

	protected void checkCursorDowngrade ()
		throws SQLException
	{

 		int cursorType = (int)OdbcApi.SQLGetStmtOption (
			hStmt, OdbcDef.SQL_ATTR_CURSOR_TYPE);

		if (cursorType != myConnection.getOdbcCursorType (rsType)) {

			if (cursorType == OdbcDef.SQL_CURSOR_FORWARD_ONLY)
				rsType = ResultSet.TYPE_FORWARD_ONLY;
			else
				rsType = ResultSet.TYPE_SCROLL_INSENSITIVE;
		}
		setWarning (new SQLWarning (
				"Result set type has been changed."));
	}


	//--------------------------------------------------------------------
	// getTypeFromObject
	// Given an object of unknown type, return the Java SQL type that
	// it can be casted to.
	//--------------------------------------------------------------------

	public static int getTypeFromObject (Object x)
	{
		if (x == null)
			return Types.NULL;
		// Try a String type
		if (x instanceof String)
			return Types.CHAR;
		// Try a Numeric type
		if (x instanceof BigDecimal)
			return Types.NUMERIC;
		// Try a Boolean type
		if (x instanceof Boolean)
			return Types.BIT;
		// Try a Byte type
		if (x instanceof Byte)
			return Types.TINYINT;
		// Try a Short type
		if (x instanceof Short)
			return Types.SMALLINT;
		// Try an Integer type
		if (x instanceof Integer)
			return Types.INTEGER;
		// Try a Long type
		if (x instanceof Long)
			return Types.BIGINT;
		// Try a Float type
		if (x instanceof Float)
			return Types.FLOAT;
		// Try a Double type
		if (x instanceof Double)
			return Types.DOUBLE;
		// Try a byte[] type
		if (x instanceof byte[])
			return Types.VARBINARY;
		// Try InputStream type
		if (x instanceof java.io.InputStream)
			return Types.LONGVARBINARY;
		// Try a reader type
		if (x instanceof java.io.Reader)
			return Types.LONGVARCHAR;
		// Try a Date type
		if (x instanceof java.sql.Date)
			return Types.DATE;
		// Try a Time type
		if (x instanceof java.sql.Time)
			return Types.TIME;
		// Try a Timestamp type
		if (x instanceof java.sql.Timestamp)
			return Types.TIMESTAMP;
		return Types.OTHER;		
	}

    //----------------------------------------------------------------
    // JDBC 3.0 API Changes
    //----------------------------------------------------------------
    
    public boolean getMoreResults(int current) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public ResultSet getGeneratedKeys() throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int executeUpdate(String sql, int flag) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int executeUpdate(String sql, int columnIndexes[]) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int executeUpdate(String sql, String columnNames[]) throws SQLException {
	throw new UnsupportedOperationException();
    }
    
    public boolean execute(String sql, int flag) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public boolean execute(String sql, int columnIndexes[]) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public boolean execute(String sql, String columnNames[]) throws SQLException {
	throw new UnsupportedOperationException();
    }

    public int getResultSetHoldability() throws SQLException {
	throw new UnsupportedOperationException();
    }


	//====================================================================
	// Data attributes
	//====================================================================

	protected JdbcOdbc OdbcApi;		// ODBC API interface object

	protected long	hDbc;			// Database connection handle

	protected long	hStmt;			// Statement handle

	protected SQLWarning lastWarning;	// Last SQLWarning generated
						//  by an operation

	protected Hashtable typeInfo;		// Hashtable containing an entry
						//  for each row returned by
						//  DatabaseMetaData.getTypeInfo.

	protected ResultSet myResultSet;	// The last ResultSet created
						//  for this Statement	

	protected JdbcOdbcConnectionInterface myConnection;
						// The owning Connection object

	protected int rsType;			// TYPE_FORWARD_ONLY
						// TYPE_SCROLL_SENSITIVE
						// TYPE_SCROLL_INSENSITIVE

	protected int rsConcurrency;		// CONCUR_READ_ONLY
						// CONCUR_UPDATABLE

	protected int fetchDirection;		// 0 - forward, 1 - reverse

	protected int fetchSize;		// 

	protected Vector batchSqlVec;		// Vector that holds all DML 
						// for SQL Batch Update.
	
	protected boolean batchSupport;		// Indicates if SQL Batch is
						// supported.
	
	protected int	batchRCFlag;		// Indicates what type of row
						// count is return when batch
						// is supported.

	protected String mySql;			// Saved SQL string

	protected boolean batchOn;		// indicates when there are
						// statemenst to be process
						// for BatchUpdate.

	protected int rsBlockSize;		// prefered odbcRowSetSize 
						// property value for
						// a scrollable block-cursor.

	protected int moreResults;		// Keeps MoreResults status flag;
	
	protected boolean closeCalledFromFinalize; //4524683
	

}
