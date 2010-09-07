/*
 * @(#)DispatchClient.java	1.2 02/06/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.javascript.ocx.*;

/**
 *  Dispatch is the Java side companion to the IDispatch COM interface.
 */
public class DispatchClient
{
	private static final int EQUALS_FALSE = 0;	// native object is not equivalent
	private static final int EQUALS_TRUE = 1;	// native object is equivalent
	private static final int EQUALS_ERROR = -1; // error, can not compare

    private int dispPtr=0;
    private int threadId=0;

    public DispatchClient(int ptr, int tid) {
	dispPtr = ptr;
	threadId = tid;
    }

    public Object invoke(int handle, String methodName, int flag, Object args[]) {
	Object retVal;
	Object [] convertedArgs = null;
	
	//Convert the non primitive types into a DispatchImpl object
	if(args != null) {
	    convertedArgs = Utils.convertArgs(args, handle);
	}

	//invoke the method
	Object result = nativeInvoke(handle, methodName, flag, convertedArgs, dispPtr, threadId);
	if(result != null && result instanceof DispatchClient) {
	    retVal = new JSObject((DispatchClient)result);
	} else {
	    retVal = result;
	}
	
	return retVal; 
    }

    public void release(int handle) {
	nativeRelease(handle, dispPtr);
    }

	public boolean equals(int handle, Object other) {
		if(!(other instanceof DispatchClient)) 
			return false;

		DispatchClient dispOther = ((DispatchClient)other);
		int nativeRet = nativeEquals(handle, dispPtr, dispOther.dispPtr);
		if(EQUALS_ERROR == nativeRet)
			throw new RuntimeException("Unexpected native error");

		return (EQUALS_TRUE == nativeRet);
	}

    public int getDispatchWrapper(){
	return dispPtr;
    }

    public String getDispType(int handle) {
	return nativeGetDispType(handle, dispPtr);
    }




	private native int nativeEquals(int handle, int dispPtr1, int dispPtr2);
    private native void nativeRelease(int handle, int ptr);
    private native Object nativeInvoke( int handle, String methodName, int flag, 
					Object[] args, int ptr, int tid);
    private native String nativeGetDispType(int handle, int ptr);
}
