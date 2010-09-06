/*
 * @(#)JdbcOdbcDriver.java	1.36 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcDriver.java
//
// Description: Impementation of the Driver interface class
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
import java.net.URL;
import java.net.MalformedURLException;
import java.sql.*;
import sun.jdbc.odbc.ee.*;

public class JdbcOdbcDriver
	extends		JdbcOdbcObject
	implements	JdbcOdbcDriverInterface
{

	//--------------------------------------------------------------------
	// Static method to be executed when the class is loaded.
	//--------------------------------------------------------------------


	static
	{		
		JdbcOdbcTracer tracer1 = new JdbcOdbcTracer();
		if (tracer1.isTracing ()) {
			tracer1.trace ("JdbcOdbcDriver class loaded");
		}
		
		JdbcOdbcDriver driver = new JdbcOdbcDriver ();

		// Attempt to register the driver

		try {
			DriverManager.registerDriver (driver);
		}
		catch (SQLException ex) {
			if (tracer1.isTracing ()) {
				tracer1.trace ("Unable to register driver");
			}  
		}
	}

	//====================================================================
	// Public methods
	//====================================================================

	//--------------------------------------------------------------------
	// Constructor
	// Perform any necessary initialization.
	//--------------------------------------------------------------------

	public JdbcOdbcDriver ()
	{

		//if an instace is created, where these
		//values have been set to meaningful values,
		//a second instance will destroy them.
		//OdbcApi = null;
		//hEnv = 0;
		//hDbc = 0;

		// Allocate a new connection list
		if (connectionList == null) { // Bug 4641013
		    connectionList = new Hashtable ();
		}
		nativePrefix = "";
			
	}

	//--------------------------------------------------------------------
	// finalize
	// Perform any cleanup when this object is garbage collected
	//--------------------------------------------------------------------

	protected synchronized void finalize ()
	{
		if (OdbcApi.getTracer().isTracing ()) {
			OdbcApi.getTracer().trace ("Driver.finalize");
		}

		try {
			// If we had previously allocated a connection handle,
			// close it now

			if (hDbc != 0) {
				disconnect (hDbc);
				closeConnection (hDbc);
				hDbc = 0;
			}
		}
		catch (SQLException ex) {
			// If an exception is thrown, ignore it
		}
	}


	//--------------------------------------------------------------------
	// connect
	// Parse the url, and verify that we can handle it.  If so,
	// allocate an environment and connection handle, then attempt
	// to connect to the odbc driver given in the url.
	//--------------------------------------------------------------------

	public synchronized Connection connect(
		String url,
		java.util.Properties info)
		throws SQLException
	{
		int iTimOut; // RFE 4641013

		if ( tracer.isTracing() ) {
			tracer.trace ("*Driver.connect (" + url + ")");
		}
	
		// If this driver cannot accept the URL, do not allow the
		// connection

		if (!acceptsURL (url)) {
			return null;
		}

		// If we had previously allocated a connection handle
		// (for getPropertyInfo), close it now

		if (hDbc != 0) {
			disconnect (hDbc);
			closeConnection (hDbc);
			hDbc = 0;
		}

		// If we can't initialize properly, return null to indicate
		// that this driver cannot be used.

		if (initialize () == false) {
			return null;
		}

		// Create the connection object.  Pass in our ODBC API
		// object and the environment handle

		JdbcOdbcConnection con = new JdbcOdbcConnection (OdbcApi,
				hEnv, this);

		// RFE 4641013
		if(getTimeOut() > 0 ){
		   iTimOut = getTimeOut();
		}
		else{
		     iTimOut = DriverManager.getLoginTimeout();
		} 
		
		// Now, attempt to initialize it by connecting to the
		// data source.  Supply the datasource name and the info
		// table, as well as the login timeout value.				
		con.initialize (getSubName (url), info,	iTimOut);

		// Set the URL for the connection

		con.setURL (url);

		return con;
	}
	

	//--------------------------------------------------------------------
	// RFE 4641013
	// EEconnect
	// Parse the url, and verify that we can handle it.  If so,
	// allocate an environment and connection handle, then attempt
	// to connect to the odbc driver given in the url. This will be called from 
	// EE part of jdbc-odbc bridge
	//--------------------------------------------------------------------

	public synchronized Connection EEConnect(
		String url,
		java.util.Properties info)
		throws SQLException
	{
		int iTimOut;
		if (tracer.isTracing ()) {
			tracer.trace ("*Driver.connect (" + url + ")");
		}

		// If this driver cannot accept the URL, do not allow the
		// connection

		if (!acceptsURL (url)) {
			return null;
		}

		// If we had previously allocated a connection handle
		// (for getPropertyInfo), close it now

		if (hDbc != 0) {
			disconnect (hDbc);
			closeConnection (hDbc);
			hDbc = 0;
		}

		// If we can't initialize properly, return null to indicate
		// that this driver cannot be used.

		if (initialize () == false) {
			return null;
		}

		// Create the connection object.  Pass in our ODBC API
		// object and the environment handle

		ConnectionHandler con = new ConnectionHandler (OdbcApi, hEnv, this);

		// RFE 4641013
		if(getTimeOut() > 0 ){
		   iTimOut = getTimeOut();
		}
		else{
		     iTimOut = DriverManager.getLoginTimeout();
		} 
		
		// Now, attempt to initialize it by connecting to the
		// data source.  Supply the datasource name and the info
		// table, as well as the login timeout value.				
		con.initialize (getSubName (url), info,	iTimOut);

		// Set the URL for the connection

		con.setURL (url);

		return con;
	}

	//--------------------------------------------------------------
	// RFE 4641013
	// getTimeOut
	// This returns the timeout set by the EE part of Jdbc-Odbc bridge. 
	//--------------------------------------------------------------
	public int getTimeOut(){
		return iTimeOut;
	}
	
	//--------------------------------------------------------------
	// RFE 4641013
	// setTimeOut
	// Sets the logintimout to be used in connection initialize. This
	// will be called by EE part of Jdbc-Odbc bridge.
	//--------------------------------------------------------------	
	public void setTimeOut(int paramTimeOut){
		iTimeOut = paramTimeOut;
	}
	
	//--------------------------------------------------------------
	// RFE 4641013
	// getWriter
	// This returns the writer set by the EE part of Jdbc-Odbc bridge. 
	//--------------------------------------------------------------	
	public java.io.PrintWriter getWriter(){
		return outWriter;
	}
	
	//--------------------------------------------------------------
	// RFE 4641013
	// setWriter
	// Datasource /Connectionpool datasource uses this method to set the
	// printwriter. 
	//--------------------------------------------------------------		
	public void setWriter(java.io.PrintWriter paramWriter){
		outWriter = paramWriter;
		tracer.setWriter(outWriter);
	}
	
	//--------------------------------------------------------------------
	// acceptsURL
	// Returns true if the driver thinks that it can open a connection
	// to the given URL.  Typically drivers will return true if they
	// understand the sun-protocol specified in the URL and false
	// otherwise.
	//--------------------------------------------------------------------

	public boolean acceptsURL (
		String url)
		throws SQLException
	{
		boolean rc = false;

		// If this is a known URL, check our security.  If we are
		// not in a secure environment, we can't support this URL

		if (knownURL (url)) {
			if (trusted ()) {
				rc = true;
			}
		}

		return rc;
	}

	//--------------------------------------------------------------------
	// getPropertyInfo
	// The getPropertyInfo method is intended to allow a generic GUI
	// tool to discover what properties it should prompt a human for
	// in order to get enough information to connect to a database.
	// Note that depending on the values the human has supplied so far,
	// additional values may become necessary, so it may be necessary
	// to iterate though several calls to getPropertyInfo.
	//--------------------------------------------------------------------

	public DriverPropertyInfo[] getPropertyInfo (
		String url,
		java.util.Properties info)
		throws SQLException
	{
		// Fix 4772103
		if (tracer.isTracing ()) {
			tracer.trace ("*Driver.getPropertyInfo (" + url + ")");
		}

		// If this driver cannot accept the URL, do not allow the
		// property info call

		if (!acceptsURL (url)) {
			return null;
		}

		// If we can't initialize properly, return null to indicate
		// that this driver cannot be used.

		if (initialize () == false) {
			return null;
		}

		// Convert the properties into a connection string

		String connectionString = makeConnectionString (info);

		// Perform a browse connect to find the required/optional
		// attribute strings
		
		String attr = "";

		attr = getConnectionAttributes (getSubName (url),
						connectionString);

		// Get a Hashtable if DriverPropertyInfo objects

		Hashtable attrTable = getAttributeProperties (attr);

		DriverPropertyInfo propertyInfo[] = 
				new DriverPropertyInfo[attrTable.size ()];

		// Convert the Hashtable into an array

		for (int i = 0; i < attrTable.size (); i++) {
			propertyInfo[i] = (DriverPropertyInfo) 
						attrTable.get (new Integer(i));
		}

		return propertyInfo;
	}

	//--------------------------------------------------------------------
	// Return the major version of the driver
	//--------------------------------------------------------------------

	public int getMajorVersion()
	{
		return JdbcOdbc.MajorVersion;
	}

	//--------------------------------------------------------------------
	// Return the minor version of the driver
	//--------------------------------------------------------------------

	public int getMinorVersion()
	{
		return JdbcOdbc.MinorVersion;
	}

	//--------------------------------------------------------------------
	// jdbcCompliant
	// Report whether the Driver is a genuine JDBC COMPLIANT (tm) driver.
	// A driver may only report "true" here if it passes the JDBC
	// compliance tests, otherwise it is required to return false.
	//--------------------------------------------------------------------
	
	public boolean jdbcCompliant ()
	{
		return true;
	}

	//====================================================================
	// Private methods
	//====================================================================

	//--------------------------------------------------------------------
	// initialize
	// Perform any required initialization.  Returns true if initialized
	// properly.
	//--------------------------------------------------------------------
	private boolean initialize ()
		throws SQLException
	{
		boolean rc = true;

		// Create a new JDBC -> ODBC bridge object.  This object is
		// used for every ODBC call.  This object will attempt the
		// JDBC to ODBC bridge library (native 'C' code).

		if (OdbcApi == null) {
			try {
				OdbcApi = new JdbcOdbc (tracer, nativePrefix);
				tracer = OdbcApi.getTracer();
				// Set OS file encoding.
				OdbcApi.charSet = (String) java.security.AccessController.doPrivileged(
				    new sun.security.action.GetPropertyAction("file.encoding"));
			}
			catch (Exception ex) {
				if (OdbcApi.getTracer().isTracing ()) {
					OdbcApi.getTracer().trace ("Unable to load JdbcOdbc library");
				}
				rc = false;
			}
		}
		
		// RFE 4641013
		if(getWriter() != null){
		   OdbcApi.getTracer().setWriter(getWriter());
		}
		
		// Allocate an environment handle for the driver

		if (hEnv == 0) {
			try {
				hEnv = OdbcApi.SQLAllocEnv ();
			}
			catch (Exception ex) {
				if (OdbcApi.getTracer().isTracing ()) {
					OdbcApi.getTracer().trace ("Unable to allocate environment");
				}
				rc = false;
			}
		}		

		return rc;
	}

	//--------------------------------------------------------------------
	// knownURL
	// Checks the protocol and subprotocol of the given url.  The protocol
	// must be 'jdbc' and the subprotocol must be 'odbc'
	//--------------------------------------------------------------------
	private boolean knownURL (String url)
	{
		String	s;

		// Make sure we can handle the protocol

		s = getProtocol (url);

		// The protocol must be 'jdbc'

		if (!s.equalsIgnoreCase ("jdbc")) {
			return false;
		}

		// Now check for the subprotocol of 'odbc'

		s = getSubProtocol (url);
		if (!s.equalsIgnoreCase ("odbc")) {
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------------
	// getProtocol
	// Returns the protocol string of the given url.  For example, if
	// a url of 'jdbc:odbc:datasource' is given, the protocol returned
	// is 'jdbc'
	//--------------------------------------------------------------------
	public static String getProtocol (String url)
	{
		String	protocol = "";
		int	index;

		// Find the first occurance of a colon ':'

		index = url.indexOf (':');

		if (index >= 0) {
			protocol = url.substring (0, index);
		}
		return protocol;
	}

	//--------------------------------------------------------------------
	// getSubProtocol
	// Returns the subprotocol string of the given url.  For example, if
	// a url of 'jdbc:odbc:datasource' is given, the subprotocol returned
	// is 'odbc'
	//--------------------------------------------------------------------
	public static String getSubProtocol (String url)
	{
		String	subprotocol = "";
		int	index;

		// Find the first occurance of a colon ':'

		index = url.indexOf (':');

		if (index >= 0) {
			int index2;

			// Find the next occurance of a colon ':'

			index2 = url.indexOf (':', index + 1);

			if (index2 >= 0) {
				subprotocol = url.substring (index+1, index2);
			}
		}
		return subprotocol;
	}

	//--------------------------------------------------------------------
	// getSubName
	// Returns the subname string of the given url.  For example, if
	// a url of 'jdbc:odbc:datasource' is given, the subname returned
	// is 'datasource'.  If there is any other data passed in after the
	// subname, it is also returned.
	//--------------------------------------------------------------------
	public static String getSubName (String url)
	{
		String	subname = "";
		int	index;

		// Find the first occurance of a colon ':'

		index = url.indexOf (':');

		if (index >= 0) {
			int index2;

			// Find the next occurance of a colon ':'

			index2 = url.indexOf (':', index + 1);

			if (index2 >= 0) {

				// Return the rest of the string

				subname = url.substring (index2 + 1);
			}
		}
		return subname;
	}

	//--------------------------------------------------------------------
	// trusted
	// Return true if the Driver is being called from a trusted
	// application/applet
	//--------------------------------------------------------------------

	private boolean trusted ()
	{
		boolean rc = false;

		if (tracer.isTracing ()) {
			tracer.trace ("JDBC to ODBC Bridge: Checking security");
		}
	
		SecurityManager security = System.getSecurityManager ();
		if (security != null) {

			// First, let's see if we are running inside Netscape
			// Navigator.  If we have the security to do so, and
			// we are using Netscape, then allow the bridge to be used.
			// Currently, only an applet that is loaded locally can access
			// this information.

			try {
				String browser; 

				browser = (String) java.security.AccessController.doPrivileged(
				    new sun.security.action.GetPropertyAction ("browser"));

				if (browser != null) {
					if (browser.equalsIgnoreCase ("Netscape Navigator")) {

						// Set the prefix for the native library

						nativePrefix = "Netscape_";
						return true;
					}
				}
			}
			catch (Exception ex) {
				// Do nothing with the exception
			}

			try {
				security.checkWrite ("JdbcOdbcSecurityCheck");
				rc = true;
			}
			catch (SecurityException ex) {
				if (tracer.isTracing ()) {
					tracer.trace ("Security check failed: " +
						ex.getMessage ());
				}
				rc = false;
			}
		}
		else {
			if (tracer.isTracing ()) {
				tracer.trace ("No SecurityManager present, assuming trusted application/applet");
			}
			rc = true;
		}
rc=true;
		return rc;
	}

	//--------------------------------------------------------------------
	// getConnectionAttributes
	// Use SQLBrowseConnect to retrieve the required/optional connection
	// string attributes
	//--------------------------------------------------------------------
	public String getConnectionAttributes (
		String dataSource,
		String options)
		throws SQLException
	{
		String dsn = "DSN=" + dataSource + options;

		// First, allocate a connection handle if we haven't done
		// so already

		if (hDbc == 0) {
			hDbc = allocConnection (hEnv);
		}

		// Now use SQLBrowseConnect to return a list of the required
		// and optional connection string attributes.

		String attr = OdbcApi.SQLBrowseConnect (hDbc, dsn);

		// If SQLBrowseConnect actually did connect, there
		// are no attributes to return.  Return an empty string and
		// disconnect

		if (attr == null) {
			attr = "";

			// Close the connection handle

			disconnect (hDbc);
			closeConnection (hDbc);
			hDbc = 0;
		}

		return attr;
	}

	//--------------------------------------------------------------------
	// getAttributeProperties
	// Given a property attribute string return a Hashtable containing
	// DriverPropertyInfo objects.
	//
	// The format of the attribute string is:
	//
	//  [*]short name:long name=value list;
	// 
	// Where * indicates an optional parameter, and value list
	// is either a ?, or a list of valid options
	//--------------------------------------------------------------------

	public java.util.Hashtable getAttributeProperties (
		String attributes)
	{
		String s;
		int i = 0;
		int j = 0;
		int k;
		int colonPos;
		int equalPos;
		int count = 0;
		DriverPropertyInfo info;
		boolean required;
		String shortName;
		String longName;
		String choices[];
		String currentValue;

		// Hash table to keep property information

		Hashtable names = new java.util.Hashtable ();

		int attrLen = attributes.length ();

		while (i < attrLen) {

			required = true;
			shortName = null;
			longName = null;
			choices = null;
			currentValue = null;

			// Find beginning of next attribute
			j = attributes.indexOf (";", i);

			if (j < 0) {
				// Prevent StringIndexOutOfBoundsException
				// j = attrLen + 1;				
				j = attrLen;
			}

			s = attributes.substring (i, j);

			k = 0;
			colonPos = s.indexOf (":", 0);
			equalPos = s.indexOf ("=", 0);
			
			// If the string starts with a '*', skip
			// to the next character

			if (s.startsWith ("*")) {
				required = false;
				k++;
			}
				
			// Save the short name

			if (colonPos > 0) {
				shortName = s.substring (k, colonPos);
			}

			// Save the long name

			if ((colonPos > 0) &&
			    (equalPos > 0)) {
				longName = s.substring (
						colonPos+1, equalPos);
			}

			// Save the value

			if (equalPos > 0) {
				currentValue = s.substring 
					(equalPos+1);
				if (currentValue.equals("?")) {
					currentValue = null;	
				}
			}

			// Determine if a possible list of values was given,
			// or the current value
			
			if (currentValue != null) {
				if (currentValue.startsWith ("{")) {
					choices = listToArray (currentValue);
					currentValue = null;
				}
			}

			// Create our DriverPropertyInfo entry

			info = new DriverPropertyInfo (shortName,
						currentValue);
			info.description = longName;
			info.required = required;
			info.choices = choices;
			
			// Save the attribute in the hash table

			names.put (new Integer (count), info);
			count++;

			// Position to beginning of next attribute
			i = j + 1;
		}

		return names;
	}

	//--------------------------------------------------------------------
	// makeConnectionString
	// Given a Properties list, create a connection string with the
	// short name and value
	//--------------------------------------------------------------------
	protected static String makeConnectionString (
		java.util.Properties info)
	{
		String s = "";

		// Create an enumeration of all the properties

		java.util.Enumeration e = info.propertyNames ();

		// Now, loop through the list until there are no more
		// properties.

		String propertyName;
		String propertyValue;

		// Try to replace the default encoding with the preffered charSet value
		OdbcApi.charSet = info.getProperty ("charSet", 
		    (String) java.security.AccessController.doPrivileged(
		    new sun.security.action.GetPropertyAction("file.encoding")));

		while (e.hasMoreElements ()) {	
			propertyName = (String) e.nextElement ();
			propertyValue = info.getProperty (propertyName);

			// Change 'user' and 'password' to the proper 
			// element names

			if (propertyName.equalsIgnoreCase ("user")) {
				propertyName = "UID";
			}

			if (propertyName.equalsIgnoreCase ("password")) {
				propertyName = "PWD";
			}

			// Now that we have the short name and value, format
			// the connection string attribute

			if (propertyValue != null) {
				s += ";" + propertyName + "=" + propertyValue;
			}
		}

		return s;
	}

	//--------------------------------------------------------------------
	// listToArray
	// Converst the given list of options into a string array.  The list
	// has the format:
	//	{choice1,choice2,choice3,...}
	//--------------------------------------------------------------------

	protected static String[] listToArray (
		String list)
	{
		String choices[] = null;
		String s;
		Hashtable t = new Hashtable ();
		int count = 0;
		int i = 1;
		int j = 1;
		int listLen = list.length ();

		if (list.startsWith ("{") == false) {
			return null;
		}

		if (list.endsWith ("}") == false) {
			return null;
		}
		
		// Convert the list into a Hashtable

		while (i < listLen) {
			// Find the next comma

			j = list.indexOf (",", i);

			// No comma found, it's the end

			if (j < 0) {
				j = listLen - 1;
			}

			s = list.substring (i, j);
			t.put (new Integer (count), s);
			count++;

			i = j + 1;
		}

		// Now convert the Hashtable into an array

		choices = new String[count];

		for (i = 0; i < count; i++) {
			s = (String) t.get (new Integer (i));
			choices[i] = s;
		}

		return choices;
	}

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
		throws SQLException
	{
		long handle = 0;

		// Allocate the connection handle
		handle = OdbcApi.SQLAllocConnect (env);

		// Add the connection to the list
		connectionList.put (new Long (handle),
				new Long (env));

		return handle;
	}

	//--------------------------------------------------------------------
	// closeConnection
	// Close the given connection handle, and remove it from our list
	// of connection handles.  If it is the last connection handle in
	// the list, the environment can also be closed
	//--------------------------------------------------------------------

	public void closeConnection (
		long dbc)
		throws SQLException
	{
		// Now that we are done with the connection, free the
		// handle

		OdbcApi.SQLFreeConnect (dbc);
		
		// Get the connection handle
		Long env = (Long) connectionList.remove (
				new Long (dbc));

		// If there are no more connections in the list, free the
		// environment

		if (connectionList.size () == 0) {
			if (hEnv != 0) {
				OdbcApi.SQLFreeEnv (hEnv);
				hEnv = 0;
			}
		}
	}

	//--------------------------------------------------------------------
	// disconnect
	// Close the given connection handle, disconnect
	//--------------------------------------------------------------------

	public void disconnect (
		long dbc)
		throws SQLException
	{
		// Disconnect. 

		OdbcApi.SQLDisconnect (dbc);
	}

	//====================================================================
	// Data attributes
	//====================================================================

	protected static JdbcOdbc OdbcApi;	// ODBC API interface object

	protected static long	hEnv;		// Environment handle

	protected static long	hDbc;		// Connection handle, used
						//  for getPropertyInfo

	protected static Hashtable connectionList;
						// List of active connections
	
	protected int iTimeOut; // RFE 4641013
	
	// Prefix to be used when loading our native library

	protected static String nativePrefix;
	
	protected java.io.PrintWriter outWriter; // RFE 4641013
	
	protected JdbcOdbcTracer tracer = new JdbcOdbcTracer(); // RFE 4641013
	
}

//----------------------------------------------------------------------------
// JdbcOdbcDriverAttribute
// Class that contains a single driver attribute.  This includes the short
// name, long name, and value.  This class is package-private
//----------------------------------------------------------------------------

class JdbcOdbcDriverAttribute
{
	String shortName;
	String longName;
	String selections;
}
