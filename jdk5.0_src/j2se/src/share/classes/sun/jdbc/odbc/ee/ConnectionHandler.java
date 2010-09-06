/*
 * @(#)ConnectionHandler.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.sql.*;
import javax.sql.*;
import sun.jdbc.odbc.*;

/**
 * This is the <code>java.sql.Connection</code> implementation used in 
 * Connectionpool implementation in the bridge. 
 * Each <code> ConnectionHandler </code> is held in <code> PooledConnection </code>.
 * <code> ConnectionHandler </code> maintains different states so that it can be effectively
 * managed. Also, it works in conjuction with <code> ConnectionEventListener </code> to have 
 * an effective checkIn/checkOut mechanism for the connection pool.
 *
 * @see java.sql.Connection 
 * @version	1.0, 05/04/02
 * @author	Amit Handa
 */
public class ConnectionHandler extends sun.jdbc.odbc.JdbcOdbcConnection{
  
    /** The url to connect to a DB  **/
    private sun.jdbc.odbc.JdbcOdbcConnection con ;

    /** Tracer object **/
    private JdbcOdbcTracer tracer = new JdbcOdbcTracer();
            
    /** Just after the creation of a connection handler the state will be NOTOPEN. **/
    final int NOTOPEN = 0;
    
    /** 
     * When a PooledConnection returns it's underlying connection handler 
     * to the application program state changes to OPEN. 
     * It maintains this state while in use.
     */
    final int OPEN    = 1;
    
    /** 
     * Multi-thread consideration make this state necessary. 
     * Just marks the handler as being closed.
     */    
    final int CLOSING = 2;
        
    /** Once the state is closed, handler is ready for reuse.**/
    final int CLOSED  = 3;
    
    /** Marks the handler as being destroyed **/
    final int DESTROYING = 4;
    
    /** Marks the handler as destroyed **/    
    final int DESTROYED = 5;
    
    /** State of the connection **/
    private int state = NOTOPEN;
        
    /** Pooled connection object **/
    private PooledObject jpo;    
        
    /**
     * Constructor. Creates a Connection Handler class, to be used in pooled connection.
     *
     * @param	odbcApi	A jdbcodbc object.
     * @param	env	Environment handle.
     * @param	driver  Driver.
     */
    public ConnectionHandler(JdbcOdbc odbcApi,long env, JdbcOdbcDriverInterface driver){
        super(odbcApi,env,driver);     
        tracer = odbcApi.getTracer();	   
    }
	
    /**
     * Close all resources with the Connection Handle. Changes the state to CLOSED at the end of this.
     * It also initates the close/error ConnectionEvent.
     */
     public synchronized void close(){
         if (state != OPEN) {
             // It is not required to be closed!
             return;
         }
         state = CLOSING;
         try{
             if (tracer.isTracing ()) {
		   tracer.trace ("*Releasing all resources to this connection Connection.close");
	     }
    	     // Close any statement objects that we have a reference to

	     super.setFreeStmtsFromConnectionOnly();
	     super.closeAllStatements ();
	     super.setFreeStmtsFromAnyWhere();	     
             if (tracer.isTracing ()) {
		 tracer.trace ("*Releasing all resources to this connection Connection.close");
	     }	
	     jpo.markUsable();
    	     state = CLOSED;
    	     ((PooledConnection)jpo).connectionClosed();
	 }catch(Exception e){	     
	     tracer.trace("Error occured while closing the connection "+ this + " " + e.getMessage());
    	     ((PooledConnection)jpo).connectionErrorOccurred(new SQLException(e.getMessage()));	     	     
	 }       
    }  
    
    /**
     * Checks whether the connection is closed or not
     *
     * @return boolean indicating whether the connection is closed or not.     
     */
    public boolean isClosed() throws SQLException{
        return (state != OPEN);
    }
  
    /**
     * Actually close the physical connection here. Changes the state of the handler to DESTROYED.
     * This also initates close event for putting it back to the pool for managing pool.
     *
     * @throws SQLException	If there is any error while closing the physical connection.
     */
    public synchronized void actualClose() throws SQLException{
         if (state == DESTROYING || state == DESTROYED) {
             // Already one thread is destroying this connection. so return.
             return;
         }
         if (state == OPEN) {
             jpo.markForSweep();
             this.close();
         }
         state = DESTROYING;
         try{         
             if (tracer.isTracing ()) {
		    tracer.trace ("*Actual Connection.close");
             }
	     super.close();
	     state = DESTROYED;
	 }catch(SQLException e){
	     state = DESTROYED;
	     tracer.trace("Error occured while closing the connection "+ this + " " + e.getMessage());	     
	     throw e;
	 }catch(Exception e){
	     state = DESTROYED;	     
	     tracer.trace("Error occured while closing the connection "+ this + " " + e.getMessage());	     
	     throw new SQLException("Unexpected exception:"+e.getMessage());	 
	 }
    }

    /**
     * This destroys the object. This doesnot bother about the pool management. Kills the object
     * immediately.
     *
     * @throws SQLException	If there is any error while destroying the object.
     */
    public void destroy() throws SQLException{
         if (state == DESTROYING || state == DESTROYED) {
             // Already one thread is destroying this connection. so return.
             return;
         }
         state = DESTROYING;
         try{         
             if (tracer.isTracing ()) {
		    tracer.trace ("*ConnectionHandler.destroy");
             }
	     super.close();
	     state = DESTROYED;
	 }catch(SQLException e){
	     state = DESTROYED;
	     tracer.trace("Error occured while closing the connection "+ this + " " + e.getMessage());
	     throw e;
	 }catch(Exception e){
	     state = DESTROYED;
	     tracer.trace("Error occured while closing the connection "+ this + " " + e.getMessage());	     
	     throw new SQLException("Unexpected exception:"+e.getMessage());	 
	 }
    }
    
    /** 
     * Get the state of the connection.
     *
     * @return State.
     */
    public int getState(){
        return state;
    }
    
    /**
     * Set the state of connection.
     *
     * @param state	State to be set.
     */
    public void setState(int state){
        this.state = state;
    }    	            
             
    /**
     * Set PooledObject which created this connectionhandler.
     *
     * @param jpo PooledObject which actually creates this connection.
     */
    public void setPooledObject(PooledObject jpo) {
        this.jpo = jpo;
    }
    
    /**
     * Finalise calls destroy to close the physical connection.
     */
    public void finalize(){
        tracer.trace("Connectionhandler Finalize....");
        try{
            destroy();
        }catch(Exception e){
            // Do nothing.
        }
    }
    
}  