;
; @(#)Interceptors.mc	1.8 03/09/09
;
; Copyright 2003 Sun Microsystems, Inc. All rights reserved.
; SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
;
("com.sun.corba.se.impl.logging" "InterceptorsSystemException" INTERCEPTORS
    (
	(BAD_PARAM
	    (TYPE_OUT_OF_RANGE	     1 WARNING "Interceptor type {0} is out of range")
	    (NAME_NULL		     2 WARNING "Interceptor's name is null: use empty string for anonymous interceptors"))
	(BAD_INV_ORDER 
	    (RIR_INVALID_PRE_INIT    1 WARNING "resolve_initial_reference is invalid during pre_init")
	    (BAD_STATE1		     2 WARNING "Expected state {0}, but current state is {1}")
	    (BAD_STATE2		     3 WARNING "Expected state {0} or {1}, but current state is {2}"))
	(COMM_FAILURE 
	    (IOEXCEPTION_DURING_CANCEL_REQUEST 1 WARNING "IOException during cancel request"))
	(INTERNAL 
	    (EXCEPTION_WAS_NULL      1 WARNING "Exception was null")
	    (OBJECT_HAS_NO_DELEGATE  2 WARNING "Object has no delegate")
	    (DELEGATE_NOT_CLIENTSUB  3 WARNING "Delegate was not a ClientRequestDispatcher")
	    (OBJECT_NOT_OBJECTIMPL   4 WARNING "Object is not an ObjectImpl")
	    (EXCEPTION_INVALID       5 WARNING "Assertion failed: Interceptor set exception to UserException or ApplicationException")
	    (REPLY_STATUS_NOT_INIT    6 WARNING "Assertion failed: Reply status is initialized but not SYSTEM_EXCEPTION or LOCATION_FORWARD")
	    (EXCEPTION_IN_ARGUMENTS  7 WARNING "Exception in arguments")
	    (EXCEPTION_IN_EXCEPTIONS 8 WARNING "Exception in exceptions")
	    (EXCEPTION_IN_CONTEXTS   9 WARNING "Exception in contexts")
	    (EXCEPTION_WAS_NULL_2    10 WARNING "Another exception was null")
	    (SERVANT_INVALID         11 WARNING "Servant invalid")
	    (CANT_POP_ONLY_PICURRENT 12 WARNING "Can't pop only PICurrent")
	    (CANT_POP_ONLY_CURRENT_2 13 WARNING "Can't pop another PICurrent")
	    (PI_DSI_RESULT_IS_NULL   14 WARNING "DSI result is null")
	    (PI_DII_RESULT_IS_NULL   15 WARNING "DII result is null")
	    (EXCEPTION_UNAVAILABLE   16 WARNING "Exception is unavailable")
	    (CLIENT_INFO_STACK_NULL  17 WARNING "Assertion failed: client request info stack is null")
	    (SERVER_INFO_STACK_NULL  18 WARNING "Assertion failed: Server request info stack is null")
	    (MARK_AND_RESET_FAILED   19 WARNING "Mark and reset failed")
	    (SLOT_TABLE_INVARIANT    20 WARNING "currentIndex > tableContainer.size(): {0} > {1}")
	    (INTERCEPTOR_LIST_LOCKED  21 WARNING "InterceptorList is locked")
	    (SORT_SIZE_MISMATCH      22 WARNING "Invariant: sorted size + unsorted size == total size was violated"))
	(NO_IMPLEMENT 
	    (PI_ORB_NOT_POLICY_BASED 1 WARNING "Policies not implemented"))
	(OBJECT_NOT_EXIST
	    (ORBINITINFO_INVALID     1 FINE "ORBInitInfo object is only valid during ORB_init"))
	(UNKNOWN
	    (UNKNOWN_REQUEST_INVOKE  
	     1 FINE "Unknown request invocation error"))
	))

;;; End of file.
