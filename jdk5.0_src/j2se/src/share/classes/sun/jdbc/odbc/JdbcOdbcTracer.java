/*
 * @(#)JdbcOdbcTracer.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc;

import java.io.PrintWriter;
import java.sql.*;

/**
 * Common tracer class for bridge. The intention of this class is to have
 * a concrete way of handling tracing. This can be used from SE part or EE
 * part of the bridge.
 *
 * @version	1.0, 05/04/02
 * @author	Amit Handa
 */ 
public class JdbcOdbcTracer {
    
    /** The printWriter object **/
    private PrintWriter outWriter;
 
    /**
     * Checks whether we have the tracing set or not
     * 
     * @return boolean yes if it is set, no otherwise
     */			
    public boolean isTracing ()	{
	if(outWriter != null){
	   return true;
	}  
	else if(DriverManager.getLogWriter() != null){
	    return (DriverManager.getLogWriter() != null);
	}
	else {
	    return false;
	}
    }

    /**
     * Writes the trace to the print writer
     *
     * @param String output to be printed
     */			
    public void trace (String outLine){
	if(outWriter != null){
    	    outWriter.println (outLine);
	    outWriter.flush();
	}
	else if(DriverManager.getLogWriter() != null){   
            DriverManager.getLogWriter().println(outLine);
	    DriverManager.getLogWriter().flush();
	}  
		//else do not write anything.
    }

    /**
     * Writes the trace to the print writer, 
     * also checks whether we have to check the next line or not.
     *
     * @param String output to be printed
     * @param boolean check whether we need to check the next line or not
     */			
    public void trace (String outLine, 	boolean advance){
        if(advance) {
	    trace (outLine);
	}

	// Sanity check to ensure we have a print stream
	if(outWriter != null){
 	    outWriter.println (outLine);
	    outWriter.flush();
	}
	else if(DriverManager.getLogWriter() != null){   
            DriverManager.getLogWriter().println (outLine);
	    DriverManager.getLogWriter().flush();
	}  
	//else do not write anything.
    }
	
    /**
     * Sets the PrinterWriter object 
     * 
     * @param PrintWriter object
     */			
    public void setWriter(java.io.PrintWriter paramWriter){
   	if(paramWriter != null){
	    outWriter = paramWriter;
	}
	else {
	    outWriter = null;
	}
			
    } 
    
    /**
     * Get the printwriter associated with this Tracer object.
     *
     * @return	printwriter.
     */
    public java.io.PrintWriter getWriter(){
        return outWriter;
    }

}      