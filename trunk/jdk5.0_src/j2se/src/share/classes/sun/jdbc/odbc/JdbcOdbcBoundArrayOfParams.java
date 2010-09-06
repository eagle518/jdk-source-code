/*
 * @(#)JdbcOdbcBoundArrayOfParams.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      JdbcOdbcBoundArrayOfParams.java
//
// Description: Class for a Batch Update bound parameters.  When a parameter is
//              bound for a prepared statement for Batch Updates, a copy of 
//		the Object's value and length is kept in this object.
//		This is so that Arrays of Parameters manipulation can 
//		be performed at one location.
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Dezi Siliezar.
//
// Date:        April, 1999
//
//----------------------------------------------------------------------------

package sun.jdbc.odbc;

import java.sql.*;
import java.util.Hashtable;
import java.util.Vector;

public class JdbcOdbcBoundArrayOfParams
	extends	JdbcOdbcObject {

	//--------------------------------------------------------------------
	// constructor
	// Calls initialization after setting the parameters count.
	//--------------------------------------------------------------------
	public JdbcOdbcBoundArrayOfParams(int paramSize)
	{

	    this.numParams = paramSize;

	    initialize();

	}

	//--------------------------------------------------------------------
	// initialize
	// Perform necessary initialization
	//--------------------------------------------------------------------
	public void initialize ()
	{
		// Allocate an array of bound parameter objects
		// and lengthIndicator for batch Update.
		// Then initialize lengthIndicator elements
		// indicating that no values have been set.
			
		storedParams = new Object[numParams];
		paramLenIdx = new int[numParams];
		hashedLenIdx = new java.util.Hashtable();
		
		// unknown # of rows in the batch at this point. 
		batchSize = 0;

		// When all parameters are properly
		// set and or reused through the PreparedStatement.setXXX
		// methods,the length indicator should be valid.
		for (int i = 0; i < numParams; i++) 
		{
			paramLenIdx[i] = -5;
		}	
	}


	//--------------------------------------------------------------------
	// storeValue
	// store the current set of parameter values/lengths.	
	//--------------------------------------------------------------------

	public void storeValue(int idx, Object x, int lenInd)
	{
	    //save value and length/ind. 	    
	    storedParams[idx] = x;
	    paramLenIdx[idx] = lenInd;

	}

	//--------------------------------------------------------------------
	// clearParameterSet
	// reset arrays for values and lengths to initialize state.	
	//--------------------------------------------------------------------

	public void clearParameterSet()
	{
	    if (storedParams != null)
	    {
		for (int i = 0; i < numParams; i++)
		{
		    storedParams[i] = new Object();
		    paramLenIdx[i] = -5;
		}
	    }
	}

	//--------------------------------------------------------------------
	// getStoredParameterSet
	// returns the current set of parameter in values Array	
	//--------------------------------------------------------------------

	public Object[] getStoredParameterSet()
	{
	    Object[] currentBatchSet = {};
	    
	    if (storedParams != null)
	    {
		currentBatchSet = new Object[numParams];

		try
		{
		    for (int i = 0; i < numParams; i++)
		    {				 
			currentBatchSet[i] = storedParams[i];
		    }			     
		}
		catch (ArrayIndexOutOfBoundsException e)
		{
		    System.out.println("exception: " + e.getMessage());
		    e.printStackTrace();
		}   
	    }
	    return currentBatchSet;
	}


	//--------------------------------------------------------------------
	// storeIndex
	// stores and lengths array Object in a HashTable for later retrival.	
	//--------------------------------------------------------------------
	public void storeRowIndex( int id, int[] x)
	{
	    hashedLenIdx.put( new Integer(id), x);
	}

	//--------------------------------------------------------------------
	// getStoredIndex
	// returns a lengths array Object with the given Object id.	
	//--------------------------------------------------------------------
	public int[] getStoredRowIndex( int id )
	{
	    return (int[]) hashedLenIdx.get( new Integer(id) );
	}

	//--------------------------------------------------------------------
	// clearStoredRowIndexs
	// clear the all the parameter lengths in the Hashtable.	
	//--------------------------------------------------------------------
	public void clearStoredRowIndexs()
	{
	    if (!hashedLenIdx.isEmpty())
	    {
		hashedLenIdx.clear();
	    }
	}

	//--------------------------------------------------------------------
	// getStoredIndexSet
	// returns the current set of parameter's length/indicator values 
	// from current length Array.
	//--------------------------------------------------------------------

	public int[] getStoredIndexSet()
	{
	    int[] currentBatchIdxSet = {};

	    if (paramLenIdx != null)
	    {
		currentBatchIdxSet = new int[numParams];

		try
		{
		    for (int i = 0; i < numParams; i++)
		    {				 
			currentBatchIdxSet[i] = paramLenIdx[i];

			if (currentBatchIdxSet[i] == -5)
			{
			    currentBatchIdxSet = new int[0];
			    return currentBatchIdxSet;
			}
		    }			     
		}
		catch (ArrayIndexOutOfBoundsException e)
		{
		    System.out.println("exception: " + e.getMessage());
		    e.printStackTrace();
		}   
	    }
	    return currentBatchIdxSet;
	}

	//--------------------------------------------------------------------
	// builtColumnWiseParameterSets
	// creates values and lengths 2D arrays. to hold parameters of the
	// batchList in a column-wise order.
	//--------------------------------------------------------------------

	public void builtColumWiseParameteSets( int numRows, Vector batchSqlVec )
	{
	    int paramIndicator[] = {};
	    Object params[] = {};

	    batchSize = numRows;

	    if (batchSqlVec.size() == batchSize)
	    {
		storedInputStreams	= new Object[batchSize][numParams];
		paramSets		= new Object[batchSize][numParams];
		paramLenIdxSets	= new    int[batchSize][numParams];

		//copy arrays into 2D array for column-wise order.
		for (int i = 0; i < batchSize; i++)
		{
		    paramIndicator = getStoredRowIndex(i);

		    params = (Object[]) batchSqlVec.elementAt(i);
						
		    int j = 0;

		    while (j < numParams)
		    {
			paramSets[i][j] = params[j];
			paramLenIdxSets[i][j] = paramIndicator[j];
			++j;
		    }
		}
	    }

	}

	//--------------------------------------------------------------------
	// getColumnWiseParamSet
	// returns a column of Objects from the 2D array of parameter values.
	//--------------------------------------------------------------------	

	public Object[] getColumnWiseParamSet(int colset)
	{
		Object[] columnSet = new Object[batchSize];

		if (paramSets != null)
		{
			for (int i = 0; i < batchSize; i++)
			{
				columnSet[i] = paramSets[i][colset - 1];
			}
		}

		return columnSet;
	}

	//--------------------------------------------------------------------
	// getColumnWiseIndexArray
	// builds length/indicator Column from the 2D array of parameter.
	// length or indicator also determines if the parameter value is NULL.
	//--------------------------------------------------------------------	

	public int[] getColumnWiseIndexArray(int colset)
	{

	    int[] columnIndex = new int[batchSize];
    
	    if (paramLenIdxSets != null)
	    {
		for (int i = 0; i < batchSize; i++)
		{
		    columnIndex[i] = paramLenIdxSets[i][colset - 1];
		}
	    }
			    
	    return columnIndex;

	}

	//--------------------------------------------------------------------
	// setInputStreamElements
	// stores an array of InputStream Objects into columwise Object array.
	//--------------------------------------------------------------------	

	public void setInputStreamElements(int columnIndex, Object[] ColumnOfStreams)
	{
	    if ((columnIndex >= 1) && (columnIndex <= numParams))
	    {	    
		if ( (storedInputStreams != null) && (ColumnOfStreams != null) )
		{
		    int j = 0;

		    while (j < batchSize)
		    {
			storedInputStreams[j][columnIndex - 1] = ColumnOfStreams[j];
			j++;
		    }
		}
	    }

	}

	//--------------------------------------------------------------------
	// getInputStreamElement
	// retrieves the Parameter InputStream for the proper Row when
	// SQL_NEED_DATA is returned during executeBatch.
	//--------------------------------------------------------------------	
	
	public java.io.InputStream getInputStreamElement(int columnIndex, int recordIndex)
	{

	    java.io.InputStream streamElement = null;

	    if ((columnIndex >= 1) && (columnIndex <= numParams))
	    {
		if ((recordIndex >= 1) && (recordIndex <= batchSize))
		    streamElement = (java.io.InputStream)storedInputStreams[recordIndex - 1][columnIndex - 1];
	    }

	    return streamElement;
	}

	//--------------------------------------------------------------------
	// getElementLength
	// retrieves the Parameter length for the proper Row when
	// SQL_NEED_DATA is returned during executeBatch..
	//--------------------------------------------------------------------	

	public int getElementLength( int column, int row )
	{
	
	   return paramLenIdxSets[row - 1][column - 1];

	}


	//====================================================================
	// Data attributes
	//====================================================================

	protected int numParams;		// Number of parameter markers
						// for the prepared statement

	protected Hashtable hashedLenIdx;	// Stores rowWise parameter indicators.
						// to determine nullability/lengths.

	protected Object[] storedParams;	// Temporary Array of Parameter Values
						// moved into batchSqlVec.

	protected int[] paramLenIdx;		// Temporary storage for Array members
						// lenght/indicator moved into hashedLenIdx.

	protected Object[][] storedInputStreams;// Temporary storage for Array of 
						// InputStreams used in executeBatch.

	protected Object[][] paramSets;		// Stores columWise parameter Values;
	protected int[][] paramLenIdxSets;	// Stores columWise parameter indicators.

	protected int batchSize;		// The # of rows for columWise parameter Sets.


}
