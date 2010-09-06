;
; @(#)Activation.mc	1.6 03/06/28
;
; Copyright 2003 Sun Microsystems, Inc. All rights reserved.
; SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
;
("com.sun.corba.se.impl.logging" "ActivationSystemException" ACTIVATION
    (
	(INITIALIZE
	    (CANNOT_READ_REPOSITORY_DB 1 WARNING "Cannot read repository datastore")
	    (CANNOT_ADD_INITIAL_NAMING 2 WARNING "Cannot add initial naming"))
	(INTERNAL
	    (CANNOT_WRITE_REPOSITORY_DB 1 WARNING "Cannot write repository datastore")
	    (SERVER_NOT_EXPECTED_TO_REGISTER 3 WARNING "Server not expected to register")
	    (UNABLE_TO_START_PROCESS 4 WARNING "Unable to start server process")
	    (SERVER_NOT_RUNNING 6 WARNING "Server is not running"))
	(OBJECT_NOT_EXIST
	    (ERROR_IN_BAD_SERVER_ID_HANDLER 1 WARNING "Error in BadServerIdHandler"))))
