/*
 * @(#)CommonDataSource.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import sun.jdbc.odbc.*;
import java.sql.*;
import javax.sql.*;
import javax.naming.*;

/**
 * Abstract class for datasource implementations in JDBC-ODBC bridge. 
 * <p>
 * This class contains all common behaviour of a datasource implementation.
 * Common datasource properties described in the Jdbc(tm) specification are get
 * and set in this class.
 * </p> 
 * <p>
 * Datasource parameters like port number and role name are not implemented,
 * since they are not applicable to ODBC. Also ODBC DSN name is get and set using
 * the methods <code> getDatabaseName </code>  and <code> setDatabaseName </code>. 
 * <code> getDataSourceName/setDataSourceName </code> refers to the name of this datasource.
 * </p>
 *
 * @see sun.jdbc.odbc.ee.ConnectionAttributes
 *
 * @version	1.0, 05/04/02
 * @author	Amit Handa
 */ 
public abstract class CommonDataSource implements javax.sql.DataSource, 
						  javax.naming.Referenceable,
						  java.io.Serializable {    
    /** Database Name connecting to **/
    private String strDbName;
    
    /** Name of the DSN connecting to **/
    private String strDSName;
    
    /** Description of the DB connectiong to **/
    private String strDesc;	

    /** User Id of the Database connectiong to  **/
    private String strUser;
    	
    /** Password of the Database connectiong to  **/
    private String strPasswd;

    /** Port no to make a TCP/IP connection  **/
    private int iPortNo;
    
    /** Role of User Id **/
    private String strRoleName;
    
    /** Character set **/
    private String strCharSet;
    
    /** Time in seconds for TimeOut of connection  **/
    private int iLoginTimeout;    
    
    /** Tracer object to get log writer  **/
    private transient JdbcOdbcTracer tracer = new JdbcOdbcTracer();
    
    /**
     * Gets the connection.
     * 
     * @return	Connection.
     * @throws SQLException	If there is any error while connecting.
     */
    public abstract Connection getConnection() throws SQLException;
    
    /**
     * Gets the connection specified by user-id and password.
     * 
     * @param	user		user id.
     * @param	password	Password
     * @return	Connection.
     * @throws SQLException	If there is any error while connecting.
     */
    public abstract Connection getConnection(String user, String password) throws SQLException;
    
    /**
     * Returns the reference required by JNDI.
     *
     * @return	Reference
     * @throws NamingException	If there is any error while getting the Reference.
     */
    public abstract Reference getReference() throws NamingException ;
    
    /**
     * Public constructor. JNDi may use this if the provider is using serialization 
     * as the underlying storage mechanism.
     */
    public CommonDataSource(){
    }
    
    /**
     * Gets the ConnectionAttributes for DataSource .
     *
     * @see sun.jdbc.odbc.ee.ConnectionAttributes
     * @return	Connection attributes.
     */    
    public ConnectionAttributes getAttributes() {
    	return new ConnectionAttributes(strDbName , strUser, strPasswd, strCharSet, iLoginTimeout);
    }

    /**
     * Sets the ODBC DSN name .The database applicable to ODBC is DSN.
     *
     * @param	paramDB	DSN name.
     */    
    public void setDatabaseName(String paramDb){
	strDbName = paramDb;
    }

    /**
     * Gets the ODBC DSN name. The database applicable to ODBC is DSN.
     *
     * @return	DSN Name
     */    
    public String getDatabaseName(){
	return strDbName;
    }
	
    /**
     * Sets the Data Source name. This is the name of this DataSource.
     *
     * @param	paramDSName DataSource Name.
     */    
    public void setDataSourceName(String paramDSName)  {
	strDSName = paramDSName;
    }

    /**
     * Gets the Data Source name.
     *
     * @return	Data Source Name
     */    
    public String getDataSourceName()  {
	return strDSName;
    }

    /**
     * Sets the Description for this DataSource.
     *
     * @param	paramDesc Description.
     */    
    public void setDescription(String paramDesc)  {
	strDesc = paramDesc;
    }

    /**
     * Gets the Description of the DataSource.
     *
     * @return	Description of Database
     */    
    public String getDescription() throws Exception{
	return strDesc;
    }	

    /**
     * Sets the Password of the Database for DataSource.
     *
     * @param	paramPasswd	Password.
     */    
    public void setPassword(String paramPasswd)  {
	strPasswd = paramPasswd;
    }
	
    /**
     * Gets the Password of the Database for DataSource.
     *
     * @return	Password
     */    
    public String getPassword()  {
	return strPasswd;
    }

    /**
     * This function is not implemented. If J2EE Application servers'
     * administration tools try to use this function, it will not have
     * any effect on Datasource.
     *
     * @param	paramPortNo	N/A
     */    
    public void setPortNumber(int paramPortNo)  {
	iPortNo = paramPortNo;
    }

    /**
     * This function is not implemented. 
     *
     * @return	N/A
     */    
    public int getPortNumber()  {
	return iPortNo;
    }

    /**
     * This function is not implemented. If J2EE Application servers'
     * administration tools try to use this function, it will not have
     * any effect on Datasource.
     *
     * @param	paramRole	N/A
     */    
    public void setRoleName(String paramRole)  {
	strRoleName = paramRole;
    }

    /**
     * This function is not implemented. 
     *
     * @return	N/A
     */    
    public String getRoleName()  {
	return strRoleName;
    }

    /**
     * Sets the Character set. This specifies the character encoding scheme
     * to be used in the connections. Possible values are available at
     * <link>
     * http://java.sun.com/products/jdk/1.1/intl/html/intlspec.doc7.html#20888
     * </link>
     * 
     * @param	paramCharSet	Characterset name.
     */    
    public void setCharSet(String paramCharSet)  {
	strCharSet = paramCharSet;
    }

    /**
     * Gets the character encoding scheme of this datasource.
     *
     * @return	charset.
     */    
    public String getCharSet()  {
	return strCharSet;
    }

    /**
     * Sets the User ID to be used in connection.
     *
     * @param	paramUser	UserID.
     */    
    public void setUser(String paramUser)  {
	strUser = paramUser;
    }

    /**
     * Gets the User ID.
     *
     * @return	User ID
     */    
    public String getUser()  {
	return strUser;
    }

    /**
     * Sets the Timeout in seconds for a connection to a Database. Default
     * value is zero and it specifies no timeout.
     *
     * @param	paramLoginTimeout	timeout in seconds.
     */    
    public void setLoginTimeout(int paramLoginTimeout)  {
	iLoginTimeout = paramLoginTimeout ;
    }

    /**
     * Gets the Timeout in seconds for a connection to a Database
     *
     * @return	Timeout in seconds
     */    
    public int getLoginTimeout()  {
	return iLoginTimeout;
    }

    /**
     * Sets the LogWriter. This method switches on the tracing in the datasource.
     *
     * @param	pwLogWriter	Printwriter for writing trace methods.
     */    
    public void setLogWriter(java.io.PrintWriter pwLogWriter) {
        SecurityManager sec = System.getSecurityManager();
        if (sec != null) {
            sec.checkPermission(new SQLPermission("setLog"));
        }
        
	if (tracer == null) {
            tracer = new JdbcOdbcTracer();
        }
	tracer.setWriter(pwLogWriter);
    }
	
    /**
     * Gets the LogWriter set in this datasource. This may return a null
     * if JNDI provider uses serialization to manage the datasource.
     *
     * @return	LogWriter object of type PrintWriter set in setLogWriter
     */    
    public java.io.PrintWriter getLogWriter()  {
    	if ( tracer == null) {
    	    return null;
    	} else {
    	    return tracer.getWriter();
    	}
    }

    /**
     * Gets the tracer object. Used internally in Bridge's datasource implementation.
     * This also may return a null if JNDI provider uses serialization to manage the datasource.
     *
     * @see sun.jdbc.odbc.JdbcOdbcTracer
     * @return	JdbcOdbcTracer object.
     */    
     public JdbcOdbcTracer getTracer(){
         if (tracer == null) {
            tracer = new JdbcOdbcTracer();
         }
         return tracer; 
     }    
}
 
