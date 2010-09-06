/*
 * @(#)JdbcOdbcBoundParam.java	@(#)JdbcOdbcBoundParam.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcBoundParam.java
//
// Description: Class for a single bound parameter.  When a parameter is
//              bound for a prepared statement, a copy of the native data is
//              kept in this object.  This is so Java won't garbage collect
//              the data before we are done with it.  An array of these
//              objects are kept with the PreparedStatement object, one
//              for each parameter marker.
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

public class JdbcOdbcBoundParam
	extends	JdbcOdbcObject {

	//--------------------------------------------------------------------
	// initialize
	// Perform an necessary initialization
	//--------------------------------------------------------------------
	public void initialize ()
	{
		// Allocate storage for the length.  Note - the length is
		// stored in native format, and will have to be converted
		// to a Java int.  The jdbcodbc 'C' bridge provides an
		// interface to do this.

		paramLength = new byte[4];
	}

	//--------------------------------------------------------------------
	// allocBindDataBuffer
	// Allocates and returns a new bind data buffer of the specified
	// length
	//--------------------------------------------------------------------
	public byte[] allocBindDataBuffer (
		int bufLen)
	{
		//changed for a test suite fix
		// Reset the input stream, we are doing a new bind
		//4641016
		//setInputStream (null, 0);

		if (binaryData == null)
			binaryData = new byte[bufLen];
		else
		{
			//we are probably rebinding so reuse the previous bindDataBuffer
			//update the previous bindDataBuffer first
			return getBindDataBuffer();			
		}		
		return binaryData;
	}

	//--------------------------------------------------------------------
	// getBindDataBuffer
	// Returns the data buffer to be used when binding to a parameter
	//--------------------------------------------------------------------
	public byte[] getBindDataBuffer ()
	{		
	
		if (pA1!=0)
		{
			//System.out.println("Called Rel 1");
			JdbcOdbc.ReleaseStoredBytes (pA1, pA2);
			pA1 = 0;
			pA2 = 0;
		}
		if (pB1!=0)
		{
			//System.out.println("Called Rel 2");			
			JdbcOdbc.ReleaseStoredBytes (pB1, pB2);
			pB1 = 0;
			pB2 = 0;
		}
		if (pC1!=0)
		{
			JdbcOdbc.ReleaseStoredBytes (pC1, pC2);
			pC1 = 0;
			pC2 = 0;
		}
		if (pS1!=0)
		{
			JdbcOdbc.ReleaseStoredChars (pS1, pS2);
			pS1 = 0;
			pS2 = 0;
		}	
		return binaryData;
	}

	//--------------------------------------------------------------------
	// getBindLengthBuffer
	// Returns the length buffer to be used when binding to a parameter
	//--------------------------------------------------------------------
	public byte[] getBindLengthBuffer ()
	{		
		if (pA1!=0)
		{
			//System.out.println("Called Rel 1");
			JdbcOdbc.ReleaseStoredBytes (pA1, pA2);
			pA1 =0;
		}
		if (pB1!=0)
		{
			//System.out.println("Called Rel 2");
			JdbcOdbc.ReleaseStoredBytes (pB1, pB2);
			pB1 =0;
		}
		if (pC1!=0)
		{
			JdbcOdbc.ReleaseStoredBytes (pC1, pC2);
			pC1 = 0;
			pC2 = 0;
		}
		if (pS1!=0)
		{
			JdbcOdbc.ReleaseStoredChars (pS1, pS2);
			pS1 = 0;
			pS2 = 0;
		}
		return paramLength;
	}
	
	public void resetBindDataBuffer(byte dataBuf[])
	{
		binaryData = dataBuf;
	}
	//--------------------------------------------------------------------
	// setInputStream
	// Sets the input stream for the bound parameter
	//--------------------------------------------------------------------
	public void setInputStream (
		java.io.InputStream inputStream,
		int len)
	{
		paramInputStream = inputStream;
		paramInputStreamLen = len;		
	}

	//--------------------------------------------------------------------
	// getInputStream
	// Gets the input stream for the bound parameter
	//--------------------------------------------------------------------
	public java.io.InputStream getInputStream ()
	{
		return paramInputStream;
	}

	//--------------------------------------------------------------------
	// getInputStreamLen
	// Gets the input stream length for the bound parameter
	//--------------------------------------------------------------------
	public int getInputStreamLen ()
	{
		return paramInputStreamLen;
	}

	//--------------------------------------------------------------------
	// setSqlType
	// Sets the Java sql type used to register an OUT parameter
	//--------------------------------------------------------------------
	
	public void setSqlType (
		int type)
	{
		sqlType = type;
	}

	//--------------------------------------------------------------------
	// getSqlType
	// Gets the Java sql type used to register an OUT parameter
	//--------------------------------------------------------------------
	
	public int getSqlType ()
	{
		return sqlType;
	}

	//--------------------------------------------------------------------
	// setStreamType
	// Sets the input stream type used to register an OUT parameter
	//--------------------------------------------------------------------
	
	public void setStreamType (
		int type)
	{
		streamType = type;
	}

	//--------------------------------------------------------------------
	// getStreamType
	// Gets the input stream type used to register an OUT parameter
	//--------------------------------------------------------------------
	
	public int getStreamType ()
	{
		return streamType;
	}

	//--------------------------------------------------------------------
	// setOutputParameter
	// Sets the flag indicating if this is an OUTPUT parameter
	//--------------------------------------------------------------------
	
	public void setOutputParameter (
		boolean output)
	{
		outputParameter = output;
	}
	
	//--------------------------------------------------------------------
	// setInputParameter
	// Sets the flag indicating if this is an INPUT parameter
	//--------------------------------------------------------------------
	
	public void setInputParameter (
		boolean input)
	{
		inputParameter = input;
	}

	//--------------------------------------------------------------------
	// isOutputParameter
	// Gets the OUTPUT parameter flag
	//--------------------------------------------------------------------
	
	public boolean isOutputParameter ()
	{
		return outputParameter;
	}

	public boolean isInOutParameter()
	{
		return (inputParameter && outputParameter);
	}

	//====================================================================
	// Data attributes
	//====================================================================

	protected byte binaryData[];		       // Storage area to be used
								// when binding the parameter

	protected byte paramLength[];		       // Storage area to be used
								// for the bound length of the
								// parameter.  Note that this
								// data is in native format.

	protected java.io.InputStream paramInputStream;
								// When an input stream is 
								// bound to a parameter, the
								// input stream is saved
								// until needed.

	protected int paramInputStreamLen;	              // Length of input stream

	protected int sqlType;                          // Java SQL type used to
	                                                // register an OUT parameter

	protected int streamType;			       // Input stream type
								// (ASCII, BINARY, UNICODE)
		public final static short ASCII   = 1;
		public final static short UNICODE = 2;
		public final static short BINARY  = 3;

	protected boolean outputParameter;	// true for OUTPUT parameters
	protected boolean inputParameter = false; // true for INPUT parameters
	protected int scale = 0;

                                        
        protected long pA1=0;   //pointers
        protected long pA2=0;
        protected long pB1=0;
        protected long pB2=0;
        protected long pC1=0;
        protected long pC2=0;
        protected long pS1=0;
        protected long pS2=0;	// reserved for strings(UTFChars)
	protected int boundType;
	protected Object boundValue;
}
