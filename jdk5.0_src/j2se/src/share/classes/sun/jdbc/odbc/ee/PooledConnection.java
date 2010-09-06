/*
 * @(#)PooledConnection.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.sql.*;
import java.util.*;
import sun.jdbc.odbc.JdbcOdbcDriver;
import sun.jdbc.odbc.JdbcOdbcTracer;

/**
 * Basic <code> PooledConnection </code> implementation. It also serves as the <code> PooledObject </code> in the pool.
 *
 * @version	1.0, 05/04/02
 * @author	Amit Handa
 */ 

public class PooledConnection implements javax.sql.PooledConnection, PooledObject  {

    /** The user id  **/
    private String strUserId = null;

    /** The password petaining to user id  **/
    private String strPassword = null;

    /** The url to connect to a DB  **/
    private String strUrl = null;
    
    /** Charset of connection	**/
    private String strCharset;

    /** private int timeout	**/
    private int timeout;

    /** The url to connect to a DB  **/
    private Properties pr = null;
    
    /** This object is the javax.sql.Connection implementation for the connection pool **/
    private ConnectionHandler conHandler;
    
    /** Check whether the PooledConnection is available for (re)use **/
    private boolean isAvailableForUse = true;
    
    /** Mark for sweeping **/
    //private boolean isMarked = false;
    
    /** Handle the listeners through HashTable **/
    private Hashtable htListener;
        
    /** Time in mill seconds when this PooledConnection object was created **/
    private long time = 0;    
    
    /** Tracer object **/
    private JdbcOdbcTracer tracer = new JdbcOdbcTracer();    
        
    /** JdbcOdbcDriver **/
    private sun.jdbc.odbc.JdbcOdbcDriver driver = null;           
    
    /** state of this object **/
    private int state;
    
    /** ConnectionEventListener **/    
    private ConnectionEventListener listener;
    
    /**
     * Constructor of the pooled connection object.
     *
     * @param Properties object p which contain a username and a password.
     * @param	tracer	JdbcOdbcTracer object.
     * @throws SQLException if creation fails.
     */
     public PooledConnection(Properties p, JdbcOdbcTracer tracer) throws SQLException{
	 try{ 
	     this.tracer = tracer;
	     strUserId = (String) p.get("user");
	     strPassword = (String) p.get("password");
	     strUrl = (String) p.get("url");
	     strCharset = (String) p.get("charset");
	     timeout = Integer.parseInt((String) p.get("loginTimeout"));
	     pr = p;
	     time = System.currentTimeMillis();
	     htListener = new Hashtable();
	     driver = new sun.jdbc.odbc.JdbcOdbcDriver();
	     driver.setTimeOut(timeout);
	     driver.setWriter(tracer.getWriter());
	     tracer.trace(" PooledConnection Being created ...."+strUserId+":"+strPassword+":"+strUrl+":"+driver);
	     conHandler = (ConnectionHandler) driver.EEConnect(strUrl,p);
	     conHandler.setPooledObject(this);
	 }catch(SQLException sqe){
	     throw sqe;
	 }catch(Exception e){
	     throw new SQLException("Error in creating pooled connection" + e.getMessage());
	 } 
     }
     
    /**
     * Check whether the Pooled Object matches the same properties used for initialize
     * 
     * @param Properties object p which contain a username, password, url and charset.
     * @return true if matches Properties object else false.
     */
     public boolean isMatching(Properties p){
         return (pr.equals(p));
     }
    
    /**
     * Check whether the Pooled Object is ready for use or re-use
     * 
     * @return	Whether it is usable or not.
     */
     public boolean isUsable(){
          return isAvailableForUse; 
     }
     
    /**
     * Mark this object for sweeping
     */
     public void markForSweep(){
          state = MARKEDFORSWEEP;
     }
     
    /**
     * Check whether marked for sweep
     * 
     * @return true if already marked for sweeping.
     */
     public boolean isMarkedForSweep(){
     	return (state==MARKEDFORSWEEP);
     }
     
    /**
     * Mark this object as free for use.
     */
     public void markUsable(){
     	 isAvailableForUse = true;
     }          
     
    /**
     * Get the time of creation of this object
     *
     * @return the time in seconds of creation of this object
     */
     public long getCreatedTime(){
     	return time;
     }
     
    /**
     * Register the event listener to notify when an event occurs
     * 
     * @param	listener	A ConnectionEventListener object.
     */
     public void addConnectionEventListener(javax.sql.ConnectionEventListener listener){
     	 htListener.put(listener, "");
     	 if (listener instanceof ConnectionEventListener){
     	     this.listener = (ConnectionEventListener) listener;
     	 }
     }
     
    /**
     * Get the time of creation of this object
     *
     * @return the time in seconds of creation of this object
     * @throws SQLException If the underlying physical connection is not available or
     *		not free for use.
     */
     public Connection getConnection() throws SQLException{
     	 if (conHandler.getState() != conHandler.CLOSED 
     	  && conHandler.getState() != conHandler.NOTOPEN) {
     	     throw new SQLException("Connection is not available now!");
     	 }     	      	      	 
     	 if (state == MARKEDFORSWEEP){
     	     throw new SQLException("PooledConnection is not usable");
     	 }     	      	 
     	 if (state == CHECKEDIN) {
     	     listener.connectionCheckOut(new javax.sql.ConnectionEvent(this));
     	 }
     	 isAvailableForUse = false;     	 
     	 conHandler.setState(conHandler.OPEN);
         return conHandler ;
     }
     
    /**
     * Remove the event listener from the list of components
     * 
     * @param	listener	a ConnectionEventListener object.
     */
     public void removeConnectionEventListener(javax.sql.ConnectionEventListener listener){
         htListener.remove(listener);
     }

    /**
     * Call the close method of ConnectionHandler
     * 
     * @throws SQLException	In case an error occurs
     */
    public void close() throws SQLException{
	 try{
 	     isAvailableForUse = false;
 	     state = MARKEDFORSWEEP;
 	     conHandler.actualClose();
 	 }catch(SQLException sqle) {
 	     throw sqle;
 	 }catch(Exception ex) {
 	     throw new SQLException("Unexpected Exception : " + ex.getMessage());
 	 }
     }
     
    /**
     * Destroys the object.
     */   
    public void destroy(){
	try{
 	    isAvailableForUse = false;
	    state = MARKEDFORSWEEP;
 	    conHandler.destroy();
     	}catch(Exception ex) {
 	    // Do nothing. Consider that object is destroyed.
 	}    
    }                    
     
    /**
     * This initiates checkIn process.
     */
    public void connectionClosed(){
        Enumeration e = htListener.keys();
        while (e.hasMoreElements()){
            javax.sql.ConnectionEventListener cel = (javax.sql.ConnectionEventListener) e.nextElement();
            javax.sql.ConnectionEvent ce = new javax.sql.ConnectionEvent(this);
            cel.connectionClosed(ce);
        }
    }

    /**
     * This is called while error occurs and initiates checkIn process.
     *
     * @param ex	SQLException causing this error.
     */
    public void connectionErrorOccurred(SQLException ex){
        Enumeration e = htListener.keys();
        while (e.hasMoreElements()){
            javax.sql.ConnectionEventListener cel = (javax.sql.ConnectionEventListener) e.nextElement();
            javax.sql.ConnectionEvent ce = new javax.sql.ConnectionEvent(this,ex);
            cel.connectionErrorOccurred(ce);
        }
    }  

    /**
     * Marks this object as checked out.
     */
    public void checkedOut(){
    	if (state!=MARKEDFORSWEEP){    
            state = CHECKEDOUT;
        }
    }    

    /**
     * Marks this object as checked in
     */
    public void checkedIn(){
    	if (state!=MARKEDFORSWEEP){
            state = CHECKEDIN;
        }
    }    
}
