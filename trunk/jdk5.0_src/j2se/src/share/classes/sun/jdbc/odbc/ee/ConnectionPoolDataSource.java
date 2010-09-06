/*
 * @(#)ConnectionPoolDataSource.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import sun.jdbc.odbc.JdbcOdbcDriver;
import java.util.Properties;
import java.sql.Connection;
import java.sql.SQLException;
import javax.sql.PooledConnection;
import javax.naming.Reference;
import javax.naming.StringRefAddr;
import javax.naming.NamingException;

/**
 * <code> ConnectionPoolDataSource </code> implementation for JDBC-ODBC bridge. 
 * <p>
 * This implementation encapsulates a <code> ConnectionPool </code>. <code> ConnectionPool </code>
 * is kept as separate class and is maintained by a <code> ConnectionPoolFactory </code>, 
 * to manage more that one connection pool. <code> ConnectionPool </code> 
 * Implementation serves as the interface to the application programs and application servers.
 * </p>
 * <p>
 * An application program can use by binding this as a <code> Datasource </code>.
 * <PRE>
 * .............................. 
 * InitialContext ctx = new InitialContext();
 * javax.sql.DataSource ds = (javax.sql.DataSource) ctx.lookup("jdbc/odbcPoolDB");
 * java.sql.Connection con1 = ds.getConnection();
 * java.sql.Connection con2 = ds.getConnection(user,password);
 * .............................. 
 * </PRE>
 * In the above code, jdbc/odbcPoolDB , will be bound in the JNDI after setting necessary 
 * properties.
 * </p>
 * <p>
 * An application program or an application server can use by binding this as a ConnectionPoolDatasource.
 * <PRE>
 * ..............................
 * InitialContext ctx = new InitialContext();
 * javax.sql.ConnectionPoolDataSource cpds = (javax.sql.ConnectionPoolDataSource) ctx.lookup("jdbc/odbcPoolDB");
 * javax.sql.PooledConnection pc1 = cpds.getPooledConnection();
 * java.sql.Connection con1 = pc1.getConnection();
 * javax.sql.PooledConnection pc2 = cpds.getPooledConnection(user,password);
 * java.sql.Connection con2 = pc2.getConnection();
 * .............................. 
 * </PRE>
 * In the above code, jdbc/odbcPoolDB , will be bound in the JNDI after setting necessary 
 * properties. This typically will be used by an application server to manage this ConnectionPoolDatasource.
 * </p> 
 * <p>
 * For both the scenarios mentioned above, ConnectionPoolDataSource may be bound in JNDI as follows.
 * ...............
 * sun.jdbc.odbc.ee.ConnectionPoolDataSource cpds = new sun.jdbc.odbc.ee.ConnectionPoolDataSource("jndiname");
 * <PRE>
 * cpds.setUser("user");
 * cpds.setPassword("password");
 * cpds.set.......
 * ...............
 * InitialContext ctx = new InitialContext();
 * ctx.bind("jndiname",cpds);
 * ...............
 * </PRE>
 * </p>
 *
 * @see ConnectionPool 
 * @see ConnectionPoolFactory
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class ConnectionPoolDataSource extends CommonDataSource implements javax.sql.ConnectionPoolDataSource{

    /** Maximum number of statements in the pool */
    private int maxStatements;
    
    /** Initial Size of connection pool **/
    private int initialPoolSize;
    
    /** Minimum Pool Size **/
    private int minPoolSize;
    
    /** Maximum Pool Size **/
    private int maxPoolSize;
    
    /** Maximum Idle Time **/
    private int maxIdleTime;
    
    /** property Cycle **/
    private int propertyCycle;
    
    /** Connection time out from pool **/
    private int timeoutFromPool;
    
    /** Maintenance Interval **/
    private int mInterval;
    
    /** boolean to indicate shutdown **/
    private boolean shutdown=false;    
    
    /**
     * Creates a ConnectionPoolDataSource.
     */
    public ConnectionPoolDataSource() {    
    }
        
    /**
     * Creates a ConnectionPoolDataSource.
     *
     * @param	dsn	DataSourceName
     */
    public ConnectionPoolDataSource(String dsn) {
        super.setDataSourceName(dsn);
    }
    
    /**
     * Get the connection from an available Pooled Connection.
     *
     * @return a Connection object.
     * @throws SQLException	if there is any problem in getting the connection from pool.
     */
    public Connection getConnection() throws SQLException {
    	return getPooledConnection().getConnection();
    }
    
    /**
     * Get the connection from an available Pooled Connection.
     *
     * @param user User id for connection.
     * @param password	Password for the connection.
     * @return a Connection object.
     * @throws SQLException	if there is any problem in getting the connection from pool.     
     */
    public Connection getConnection(String user, String password) throws SQLException {
    	return getPooledConnection(user,password).getConnection();
    }
    
    /**
     * Get a available PooledConnection.
     *
     * @return a PooledConnection object.
     * @throws SQLException	if there is any problem in getting the connection from pool.     
     */    
    public PooledConnection getPooledConnection() throws SQLException{
    	return (PooledConnection) getPool().checkOut();
    }    
    
    /**
     * Get a available PooledConnection for the given user-id and password.
     *
     * @return a PooledConnection object.
     * @throws SQLException	if there is any problem in getting the connection from pool.     
     */    
    public PooledConnection getPooledConnection(String user, String password) throws SQLException{
        Properties p = super.getAttributes().getProperties();
        p.put("user", user);
        p.put("password",password);
    	return (PooledConnection) getPool().checkOut(p);
    }
 
    /**
     * Set the no of statements Pool should keep open. This method is not implemented now.
     * This is provided only for forward compatibility.
     *
     * @param	noOfStatements	No of statements.
     * @throws	SQLException currently not thrown.
     */ 
    public void setMaxStatements(String noOfStatements) throws SQLException {
	// Do Nothing. This is not implemented now.
    }
    
    /**
     * Get the no of statements Pool keep open. This method is not implemented now.
     * This is provided only for forward compatibility.
     *
     * @return	No of statements.
     */    
    public int getMaxStatements() {
        // This is not implemented now
    	return maxStatements;
    }
    
    /**
     * Set the Initial Size of the pool. ConnectionPool will be started with this size.
     *
     * @param	poolSize	Initial Pool size.
     * @throws SQLException	If the value set is not a number or null.
     */
    public void setInitialPoolSize(String poolSize) throws SQLException {
        if (poolSize == null) {
            throw new SQLException("Initial pool size cannot be null");
        }
    	try{
    	   initialPoolSize = Integer.parseInt(poolSize.trim());
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Initial pool size is not a number ");
    	}    	
    }
    
    /**
     * Get the Initial Size of the pool.
     *
     * @return	Initial Pool size.
     */    
    public int getInitialPoolSize() {
    	return initialPoolSize;
    }
    
    /**
     * Set the Maximum Size of the pool. A value of zero indicates that there will not be any limit to 
     * the maximum pool size.
     *
     * @param	poolSize	Maximuml Pool size.
     * @throws SQLException	If the value set is not a number or null.     
     */
    public void setMaxPoolSize(String poolSize) throws SQLException {
        if (poolSize == null) {
            throw new SQLException("Max pool size cannot be null");
        }    
    	try{
    	    maxPoolSize = Integer.parseInt(poolSize.trim());
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Max pool size is not a number ");
    	}    	    	
    }
    
    /**
     * Get the Maximum Size of the pool.
     *
     * @return	Maximum Pool size.
     */    
    public int getMaxPoolSize() {
    	return maxPoolSize;
    }
    
    /**
     * Set the Minimum Size of the pool. A value of zero indicates that, connections will be created as
     * and when required.
     *
     * @param	poolSize	Minimum Pool size.
     * @throws SQLException	If the value set is not a number or null.     
     */
    public void setMinPoolSize(String poolSize) throws SQLException {
        if (poolSize == null) {
            throw new SQLException("Min pool size cannot be null");
        }    
    	try{
    	    minPoolSize = Integer.parseInt(poolSize.trim());
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Min pool size is not a number ");
    	}    	    	
    }
    
    /**
     * Get the Minimum Size of the pool.
     *
     * @return	Minimum Pool size.
     */    
    public int getMinPoolSize() {
    	return minPoolSize;
    }
    
    /**
     * Set the Maximum Idle time of a Connection. A connection in the pool can remain for maxIdleTime
     * without being used.
     *
     * @param	idleTime	Maximum idle time.
     * @throws SQLException	If the value set is not a number or null.     
     */
    public void setMaxIdleTime(String idleTime) throws SQLException {
        if (idleTime == null) {
            throw new SQLException("Idle time cannot be null");
        }    
    	try{
    	    maxIdleTime = Integer.parseInt(idleTime.trim());
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Max Idle time is not a number ");
    	}    	    	
    }
    
    /**
     * Get the Maximum Idle time of a Connection.
     *
     * @return	Maximum idle time.
     */
    public int getMaxIdleTime() {
    	return maxIdleTime;
    }    

    /**
     * Set the Property Cycle for the pool. This is not implemented now.
     * Currently the properties are set while next getPooledConnection is
     * executed.
     *
     * @param	cycle	Interval in seconds.
     */
    public void setPropertyCycle(String cycle) {
    	// Do nothing. This is unsupported now.
    }    
      
    /**
     * Get the Property Cycle for the pool. This is not implemented now.
     *
     * @return	Interval in seconds.
     */
    public int getPropertyCycle() {
        // This is unsupported now.
    	return propertyCycle;
    } 
    
    /**
     * Set the Timeout from the pool for a connection. This is the age of 
     * connection. Once a connection crosses this, and if it is not used,
     * Maintenance thread removes it from the pool.
     *
     * @param	time	Interval in seconds.
     * @throws SQLException	If the value set is not a number or null.     
     */
    public void setTimeoutFromPool(String time) throws SQLException {
        if (time == null) {
            throw new SQLException("timeout cannot be null");
        }            
    	try{
    	    timeoutFromPool = Integer.parseInt(time.trim());
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Timeout is not a number ");
    	}    	    	
    }    
      
    /**
     * Get the Timeout from the pool.
     *
     * @return	Timout.
     */
    public int getTimeoutFromPool() {
    	return timeoutFromPool;
    } 
    
    /**
     * Set the maintenance interval of the pool. This specifies, at what interval 
     * pool need to be maintained. Ideal time for this can be less than the 
     * lesser of maxIdleTime and timeoutFromPoool
     *
     * @param mInterval interval time.
     * @throws SQLException	If the value set is not a number or null.     
     */
    public void setMaintenanceInterval(String mInterval) throws SQLException {
        if (mInterval == null) {
            throw new SQLException("Maintenance interval cannot be null");
        }    
        try{
            this.mInterval = Integer.parseInt(mInterval.trim()); 
    	} catch(NumberFormatException nfe){
    	    throw new SQLException("Maintenance interval is not a number ");
    	}    	        
    }
    
    /**
     * Get the maintenance interval of the datasource.
     *
     * @return interval time in seconds.
     */
    public int getMaintenanceInterval() {
        return mInterval;
    }
    
    /**
     * Returns a reference object with all the information required to reconstruct the
     * datasource. This is used by JNDI.
     *
     * @return Reference object.
     */
    public Reference getReference() throws NamingException {
    	Reference ref = new Reference (this.getClass().getName(),
    					"sun.jdbc.odbc.ee.ObjectFactory", null);
    					
	ref.add(new StringRefAddr("databaseName", super.getDatabaseName()));
	ref.add(new StringRefAddr("dataSourceName", super.getDataSourceName()));

    	ConnectionAttributes attrib = super.getAttributes();    	

	ref.add(new StringRefAddr("user", attrib.getUser()));
	ref.add(new StringRefAddr("password", attrib.getPassword()));
	ref.add(new StringRefAddr("charSet", attrib.getCharSet()));    	    	
	ref.add(new StringRefAddr("loginTimeout", ""+super.getLoginTimeout()));	
	ref.add(new StringRefAddr("maxStatements", ""+maxStatements));
	ref.add(new StringRefAddr("initialPoolSize", ""+initialPoolSize));
	ref.add(new StringRefAddr("maxPoolSize", ""+maxPoolSize));
	ref.add(new StringRefAddr("minPoolSize", ""+minPoolSize));	
	ref.add(new StringRefAddr("maxIdleTime", ""+maxIdleTime));	
	ref.add(new StringRefAddr("propertyCycle", ""+propertyCycle));
	ref.add(new StringRefAddr("timeoutFromPool", ""+timeoutFromPool));	
	ref.add(new StringRefAddr("mInterval", ""+mInterval));
    	return ref;
    }      
    
    /**
     * ShutDown the pool associated with this Datasource. This may be called only while 
     * unbinding this datasource. Otherwise this can cause unexpected results.
     *
     * @param hotShutdown A boolean stating to specify hotshutdown or not
     */    
    public void shutDown(boolean hotShutdown) {
    	ConnectionPool pool = ConnectionPoolFactory.obtainConnectionPool(getDataSourceName());
    	pool.shutDown(hotShutdown);
    	shutdown = true;
    }
    
    /**
     * Get the pool.
     *
     * @return	ConnectionPool.
     * @throws SQLException	If pool is not initialised properly.
     */
    private ConnectionPool getPool() throws SQLException{
        if (shutdown) {
            throw new SQLException("Pool is shutdown!");
        }
        ConnectionPool pool = ConnectionPoolFactory.obtainConnectionPool(super.getDataSourceName());
	pool.setTracer(super.getTracer());
	
	PoolProperties pr = new PoolProperties();
	pr.set(PoolProperties.INITIALPOOLSIZE,initialPoolSize);
	pr.set(PoolProperties.MAXPOOLSIZE,maxPoolSize);
	pr.set(PoolProperties.MINPOOLSIZE,minPoolSize);
	pr.set(PoolProperties.MAXIDLETIME,maxIdleTime);
	pr.set(PoolProperties.TIMEOUTFROMPOOL,timeoutFromPool);
	pr.set(PoolProperties.MAINTENANCEINTERVAL,mInterval);
	
	pool.setProperties(pr);	     
	pool.setConnectionDetails(super.getAttributes().getProperties());
	pool.initializePool();   
	return pool;     
    }

   /**
    * Private serial version unique ID to ensure serialization
    * compatibility.
    */
    static final long serialVersionUID = 8730440750011279189L;
   
}
