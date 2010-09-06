;
; @(#)Naming.mc	1.9 03/10/06
;
; Copyright 2003 Sun Microsystems, Inc. All rights reserved.
; SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
;
("com.sun.corba.se.impl.logging" "NamingSystemException" NAMING
    (
	(BAD_PARAM
	    (TRANSIENT_NAME_SERVER_BAD_PORT 0 WARNING "Port 0 is not a valid port in the transient name server")
	    (TRANSIENT_NAME_SERVER_BAD_HOST 1 WARNING "A null hostname is not a valid hostname in the transient name server")
	    (OBJECT_IS_NULL 2 WARNING "Invalid object reference passed in rebind or bind operation")
	    (INS_BAD_ADDRESS 3 WARNING "Bad host address in -ORBInitDef"))
	(UNKNOWN
	    (BIND_UPDATE_CONTEXT_FAILED 0 WARNING "Updated context failed for bind")
	    (BIND_FAILURE 1 WARNING "bind failure")
	    (RESOLVE_CONVERSION_FAILURE 2 WARNING "Resolve conversion failed")
	    (RESOLVE_FAILURE 3 WARNING "Resolve failure")
	    (UNBIND_FAILURE 4 WARNING "Unbind failure"))
	(INITIALIZE
	    (TRANS_NS_CANNOT_CREATE_INITIAL_NC_SYS 50 WARNING "SystemException in transient name service while initializing")
	    (TRANS_NS_CANNOT_CREATE_INITIAL_NC 51 WARNING "Java exception in transient name service while initializing"))
	(INTERNAL
	    (NAMING_CTX_REBIND_ALREADY_BOUND 0 WARNING "Unexpected AlreadyBound exception iun rebind")
	    (NAMING_CTX_REBINDCTX_ALREADY_BOUND 1 WARNING "Unexpected AlreadyBound exception in rebind_context")
	    (NAMING_CTX_BAD_BINDINGTYPE 2 WARNING "Bad binding type in internal binding implementation")
	    (NAMING_CTX_RESOLVE_CANNOT_NARROW_TO_CTX 3 WARNING "Object reference that is not CosNaming::NamingContext bound as a context")
	    (NAMING_CTX_BINDING_ITERATOR_CREATE 4 WARNING "Error in creating POA for BindingIterator")
	    (TRANS_NC_BIND_ALREADY_BOUND 100 WARNING "Bind implementation encountered a previous bind")
	    (TRANS_NC_LIST_GOT_EXC 101 WARNING "list operation caught an unexpected Java exception while creating list iterator")
	    (TRANS_NC_NEWCTX_GOT_EXC 102 WARNING "new_context operation caught an unexpected Java excpetion creating the NewContext servant")
	    (TRANS_NC_DESTROY_GOT_EXC 103 WARNING "Destroy operation caught a Java exception while disconnecting from ORB")
	    (INS_BAD_SCHEME_NAME 105 WARNING "Stringified object reference with unknown protocol specified")
	    (INS_BAD_SCHEME_SPECIFIC_PART 107 WARNING "Malformed URL in -ORBInitDef")
	    (INS_OTHER 108 WARNING "Malformed URL in -ORBInitDef"))))
